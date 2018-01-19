#ifndef GRB_ALGORITHM_BFS_HPP
#define GRB_ALGORITHM_BFS_HPP

#include "graphblas/algorithm/testBfs.hpp"

namespace graphblas
{
namespace algorithm
{
  // Use float for now for both v and A
  Info bfs( Vector<float>*       v,
            const Matrix<float>* A, 
            Index                s,
		        Descriptor*          desc,
            bool                 transpose=false )
  {
    Index n;
    CHECK( A->nrows( &n ) );

    // Visited vector (use float for now)
    CHECK( v->fill(-1.f) );

    // Frontier vectors (use float for now)
    Vector<float> q1(n);
    Vector<float> q2(n);

    Desc_value desc_value;
    CHECK( desc->get(GrB_MXVMODE, &desc_value) );
    if( desc_value==GrB_PULLONLY )
    {
      CHECK( q1.fill(0.f) );
      CHECK( q1.setElement(1.f,s) );
    }
    else
    {
      std::vector<Index> indices(1,s);
      std::vector<float>  values(1,1.f);
      CHECK( q1.build(&indices, &values, 1, GrB_NULL) );
    }

    // Semiring
    /*BinaryOp GrB_LOR(  logical_or() );
    BinaryOp GrB_LAND( logical_and() );
    Monoid   GrB_Lor( GrB_LOR, false );
    Semiring GrB_Boolean( GrB_Lor, GrB_LAND );*/
		BinaryOp<float,float,float> GrB_PLUS_FP32;
		GrB_PLUS_FP32.nnew( plus<float>() );
		BinaryOp<float,float,float> GrB_TIMES_FP32;
		GrB_TIMES_FP32.nnew( multiplies<float>() );
		Monoid  <float> GrB_FP32Add;
		GrB_FP32Add.nnew( GrB_PLUS_FP32, 0.f );
		Semiring<float,float,float> GrB_FP32AddMul;
		GrB_FP32AddMul.nnew( GrB_FP32Add, GrB_TIMES_FP32 );

    float d    = 0;
    float succ = 0.f;
    CpuTimer cpu_tight;
    if( desc->descriptor_.timing_>0 )
      cpu_tight.Start();
    do
    {
      if( desc->descriptor_.debug() )
      {
        std::cout << "Iteration " << d << ":\n";
        v->print();
        q1.print();
      }
      if( desc->descriptor_.timing_==2 )
      {
        cpu_tight.Stop();
        std::cout << d-1 << ", " << cpu_tight.ElapsedMillis() << "\n";
        cpu_tight.Start();
      }
      assign<float,float>(v, &q1, GrB_NULL, d, GrB_ALL, n, desc);
      CHECK( desc->toggle(GrB_MASK) );
      if( transpose )
        mxv<float,float,float>(&q2, v, GrB_NULL, &GrB_FP32AddMul, A, &q1, desc);
      else
        vxm<float,float,float>(&q2, v, GrB_NULL, &GrB_FP32AddMul, &q1, A, desc);
      CHECK( desc->toggle(GrB_MASK) );
      CHECK( q2.swap(&q1) );
      reduce<float,float>(&succ, GrB_NULL, &GrB_FP32Add, &q1, desc);

      if( desc->descriptor_.debug() )
        std::cout << "succ: " << succ << " " << (int)succ << std::endl;
      d++;
    } while( succ>0 );
    if( desc->descriptor_.timing_>0 )
    {
      cpu_tight.Stop();
      std::cout << "tight, " << cpu_tight.ElapsedMillis() << ", \n";
    }

    return GrB_SUCCESS;
  }

  template <typename T, typename a>
  Info bfsCpu( Index        source,
               Matrix<a>*   A,
               T*           h_bfs_cpu,
							 Index        depth,
               bool         transpose=false )
  {
		Index* reference_check_preds = NULL;

    if( transpose )
		  SimpleReferenceBfs<T>( A->matrix_.nrows_, A->matrix_.sparse_.h_cscColPtr_,
          A->matrix_.sparse_.h_cscRowInd_, h_bfs_cpu, reference_check_preds, 
          source, depth);
    else
		  SimpleReferenceBfs<T>( A->matrix_.nrows_, A->matrix_.sparse_.h_csrRowPtr_,
          A->matrix_.sparse_.h_csrColInd_, h_bfs_cpu, reference_check_preds, 
          source, depth);

		//print_array(h_bfsResultCPU, m);
		return GrB_SUCCESS; 
	}

}  // algorithm
}  // graphblas

#endif  // GRB_ALGORITHM_BFS_HPP
