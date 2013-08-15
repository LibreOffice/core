/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>
#include "formulagroup.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include "formula/vectortoken.hxx"
#include "scmatrix.hxx"

#include "openclwrapper.hxx"

namespace sc {

// A single public entry point for a factory function:
namespace opencl {
    extern sc::FormulaGroupInterpreter *createFormulaGroupInterpreter();
}

/////time test dbg
double getTimeDiff(const TimeValue& t1, const TimeValue& t2)
{
    double tv1 = t1.Seconds;
    double tv2 = t2.Seconds;
    tv1 += t1.Nanosec / 1000000000.0;
    tv2 += t2.Nanosec / 1000000000.0;

    return tv1 - tv2;
}//dbg-t
TimeValue aTimeBefore, aTimeAfter;
///////////////////////////////////////

class FormulaGroupInterpreterOpenCL : public FormulaGroupInterpreterSoftware
{
public:
    FormulaGroupInterpreterOpenCL() :
        FormulaGroupInterpreterSoftware()
    {
        OclCalc::InitEnv();
    }
    virtual ~FormulaGroupInterpreterOpenCL()
    {
        OclCalc::ReleaseOpenclRunEnv();
    }

    virtual ScMatrixRef inverseMatrix(const ScMatrix& rMat);
    virtual bool interpret(ScDocument& rDoc, const ScAddress& rTopPos,
                           const ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode);
};

ScMatrixRef FormulaGroupInterpreterOpenCL::inverseMatrix(const ScMatrix& rMat)
{
    SCSIZE nC, nR;
    rMat.GetDimensions(nC, nR);
    if (nC != nR || nC == 0)
        // Input matrix must be square. Return an empty matrix on failure and
        // the caller will calculate it via CPU.
        return ScMatrixRef();

    // This vector will contain a series of doubles from the first column to
    // the last, chained together in a single array.
    std::vector<double> aDoubles;
    rMat.GetDoubleArray(aDoubles);
    float * fpOclMatrixSrc = NULL;
    float * fpOclMatrixDst = NULL;
    double * dpOclMatrixSrc = NULL;
    double * dpOclMatrixDst = NULL;
    uint nMatrixSize = nC * nR;
    static OclCalc aOclCalc;
    if ( aOclCalc.GetOpenclState() )
    {
        if ( aOclCalc.gpuEnv.mnKhrFp64Flag == 1 || aOclCalc.gpuEnv.mnAmdFp64Flag == 1 )
        {
            aOclCalc.CreateBuffer64Bits( dpOclMatrixSrc, dpOclMatrixDst, nMatrixSize );
            for ( uint i = 0; i < nC; i++ )
                for ( uint j = 0; j < nR; j++ )
                    dpOclMatrixSrc[i*nC+j] = aDoubles[j*nR+i];
            aOclCalc.OclHostMatrixInverse64Bits( "oclFormulaMtxInv", dpOclMatrixSrc, dpOclMatrixDst,aDoubles, nR );
        }
        else
        {
            aOclCalc.CreateBuffer32Bits( fpOclMatrixSrc, fpOclMatrixDst, nMatrixSize );
            for ( uint i = 0; i < nC; i++ )
                for ( uint j = 0; j < nR; j++ )
                    fpOclMatrixSrc[i*nC+j] = (float) aDoubles[j*nR+i];
            aOclCalc.OclHostMatrixInverse32Bits( "oclFormulaMtxInv", fpOclMatrixSrc, fpOclMatrixDst, aDoubles, nR );
        }
    }

    // TODO: Inverse this matrix and put the result back into xInv. Right now,
    // I'll just put the original, non-inversed matrix values back, just to
    // demonstrate how to put the values back after inversion.  There are two
    // ways to put the values back (depending on what the GPU output is).
    ScMatrixRef xInv(new ScMatrix(nR, nR, 0.0));

#if 0
    // One way is to put the whole value as one array. This method assumes
    // that the array size equals column x row, and is oriented column-wise.
    // This method is slightly more efficient than the second, but I wouldn't
    // expect too much of a difference.
    xInv->PutDouble(&aDoubles[0], aDoubles.size(), 0, 0);
#else
    // Another way is to put the values one column at a time.
    const double* p = &aDoubles[0];
    for (SCSIZE i = 0; i < nC; ++i)
    {
        xInv->PutDouble(p, nR, i, 0);
        p += nR;
    }
#endif

    return xInv;
}

bool FormulaGroupInterpreterOpenCL::interpret(ScDocument& rDoc, const ScAddress& rTopPos,
                                              const ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode)
{
    generateRPNCode(rDoc, rTopPos, rCode);

    size_t rowSize = xGroup->mnLength;
    fprintf(stderr,"rowSize at begin is ...%ld.\n",(long)rowSize);
    // The row quantity can be gotten from p2->GetArrayLength()
    uint nCount1 = 0, nCount2 = 0, nCount3 = 0;
    int nOclOp = 0;
    double *rResult = NULL; // Point to the output data from GPU
    rResult = (double *)malloc(sizeof(double) * rowSize*2);// For 2 columns(B,C)
    if(NULL==rResult)
    {
        printf("malloc err\n");
        return false;
    }
    memset(rResult,0,rowSize);
    float * fpOclSrcData = NULL; // Point to the input data from CPU
    double * dpOclSrcData = NULL;
    uint * npOclStartPos = NULL; // The first position for calculation,for example,the A1 in (=MAX(A1:A100))
    uint * npOclEndPos   = NULL; // The last position for calculation,for example, the A100 in (=MAX(A1:A100))
    float * fpLeftData   = NULL; // Left input for binary operator(+,-,*,/),for example,(=leftData+rightData)
    float * fpRightData  = NULL; // Right input for binary operator(+,-,*,/),for example,(=leftData/rightData)
                                 // The rightData can't be zero for "/"
    double * dpLeftData = NULL;
    double * dpRightData = NULL;

    float * fpSaveData=NULL;            //It is a temp pointer point the preparing memory;
    float * fpSumProMergeLfData = NULL; //It merge the more col to one col is the left operator
    float * fpSumProMergeRtData = NULL; //It merge the more col to one col is the right operator
    double * dpSaveData=NULL;
    double * dpSumProMergeLfData = NULL;
    double * dpSumProMergeRtData = NULL;
    uint * npSumSize=NULL;      //It is a array to save the matix sizt(col *row)
    int nSumproductSize=0;      //It is the merge array size
    bool aIsAlloc=false;        //It is a flag to judge the fpSumProMergeLfData existed
    unsigned int nCountMatix=0; //It is a count to save the calculate times
    static OclCalc ocl_calc;
    bool isSumProduct=false;
    if(ocl_calc.GetOpenclState())
    {
        // Don't know how large the size will be applied previously, so create them as the rowSize or 65536
        // Don't know which formulae will be used previously, so create buffers for different formulae used probably
        if(ocl_calc.gpuEnv.mnKhrFp64Flag==1 || ocl_calc.gpuEnv.mnAmdFp64Flag==1)
        {
            ocl_calc.CreateBuffer64Bits(dpOclSrcData,npOclStartPos,npOclEndPos,rowSize);
            ocl_calc.CreateBuffer64Bits(dpLeftData,dpRightData,rowSize);
        }
        else
        {
            ocl_calc.CreateBuffer32Bits(fpOclSrcData,npOclStartPos,npOclEndPos,rowSize);
            ocl_calc.CreateBuffer32Bits(fpLeftData,fpRightData,rowSize);
        }
        //printf("pptrr is %d,%d,%d\n",fpOclSrcData,npOclStartPos,npOclEndPos);
    }
///////////////////////////////////////////////////////////////////////////////////////////

    // Until we implement group calculation for real, decompose the group into
    // individual formula token arrays for individual calculation.
    ScAddress aTmpPos = rTopPos;
    for (sal_Int32 i = 0; i < xGroup->mnLength; ++i)
    {
        aTmpPos.SetRow(xGroup->mnStart + i);
        ScTokenArray aCode2;
        for (const formula::FormulaToken* p = rCode.First(); p; p = rCode.Next())
        {
            switch (p->GetType())
            {
                case formula::svSingleVectorRef:
                {
                    const formula::SingleVectorRefToken* p2 = static_cast<const formula::SingleVectorRefToken*>(p);
                    const double* pArray = p2->GetArray();
                    aCode2.AddDouble(static_cast<size_t>(i) < p2->GetArrayLength() ? pArray[i] : 0.0);
                }
                break;
                case formula::svDoubleVectorRef:
                {
                    const formula::DoubleVectorRefToken* p2 = static_cast<const formula::DoubleVectorRefToken*>(p);
                    const std::vector<const double*>& rArrays = p2->GetArrays();
                    size_t nColSize = rArrays.size();
                    size_t nRowStart = p2->IsStartFixed() ? 0 : i;
                    size_t nRowEnd = p2->GetRefRowSize() - 1;
                    if (!p2->IsEndFixed())
                        nRowEnd += i;
                    size_t nRowSize = nRowEnd - nRowStart + 1;
                    //store the a matix`s rowsize and colsize,use it to calculate the matix`s size
                    ocl_calc.nFormulaRowSize = nRowSize;
                    ocl_calc.nFormulaColSize = nColSize;
                    ScMatrixRef pMat(new ScMatrix(nColSize, nRowSize, 0.0));
                    if(ocl_calc.GetOpenclState())
                    {
                        npOclStartPos[i] = nRowStart; // record the start position
                        npOclEndPos[i]   = nRowEnd;   // record the end position
                    }
                    int nTempOpcode;
                    const formula::FormulaToken* pTemp = p;
                    pTemp=aCode2.Next();
                    nTempOpcode=pTemp->GetOpCode();
                    while(1)
                    {
                        nTempOpcode=pTemp->GetOpCode();
                        if(nTempOpcode!=ocOpen && nTempOpcode!=ocPush)
                            break;
                         pTemp=aCode2.Next();
                    }
                    if((!aIsAlloc) && (ocl_calc.GetOpenclState())&& (nTempOpcode == ocSumProduct))
                    {
                        //nColSize * rowSize is the data size , but except the the head of data will use less the nRowSize
                        //the other all use nRowSize times . and it must aligen so add nRowSize-1.
                        nSumproductSize = nRowSize+nColSize * rowSize*nRowSize-1;
                        if(ocl_calc.gpuEnv.mnKhrFp64Flag==1 || ocl_calc.gpuEnv.mnAmdFp64Flag==1)
                            ocl_calc.CreateBuffer64Bits(dpSumProMergeLfData,dpSumProMergeRtData,npSumSize,nSumproductSize,rowSize);
                        else
                            ocl_calc.CreateBuffer32Bits(fpSumProMergeLfData,fpSumProMergeRtData,npSumSize,nSumproductSize,rowSize);
                        aIsAlloc = true;
                        isSumProduct=true;
                    }
                    if(isSumProduct)
                    {
                        if(ocl_calc.gpuEnv.mnKhrFp64Flag==1 || ocl_calc.gpuEnv.mnAmdFp64Flag==1)
                        {
                            if(nCountMatix%2==0)
                                dpSaveData = dpSumProMergeLfData;
                            else
                                dpSaveData = dpSumProMergeRtData;
                        }
                        else
                        {
                            if(nCountMatix%2==0)
                                fpSaveData = fpSumProMergeLfData;
                            else
                                fpSaveData = fpSumProMergeRtData;
                        }
                    }
                    for (size_t nCol = 0; nCol < nColSize; ++nCol)
                    {
                        const double* pArray = rArrays[nCol];
                        if( NULL==pArray )
                        {
                            fprintf(stderr,"Error: pArray is NULL!\n");
                            free(rResult);
                            return false;
                        }
                        if(ocl_calc.GetOpenclState())
                        {
                            for( size_t u=nRowStart; u<=nRowEnd; u++ )
                            {
                                if(ocl_calc.gpuEnv.mnKhrFp64Flag==1 || ocl_calc.gpuEnv.mnAmdFp64Flag==1)
                                {
                                    dpOclSrcData[u] = pArray[u];
                                    //fprintf(stderr,"dpOclSrcData[%d] is %f.\n",u,dpOclSrcData[u]);
                                    if(isSumProduct)
                                        dpSaveData[u+nRowSize*nCol + nRowStart* nColSize * nRowSize-nRowStart] = pArray[u];
                                }
                                else
                                {
                                    // Many video cards can't support double type in kernel, so need transfer the double to float
                                    fpOclSrcData[u] = (float)pArray[u];
                                    //fprintf(stderr,"fpOclSrcData[%d] is %f.\n",u,fpOclSrcData[u]);
                                    if(isSumProduct)
                                        fpSaveData[u+nRowSize*nCol + nRowStart* nColSize * nRowSize-nRowStart] = (float)pArray[u];
                                }
                            }
                        }

                        for (size_t nRow = 0; nRow < nRowSize; ++nRow)
                        {
                            if (nRowStart + nRow < p2->GetArrayLength())
                            {
                                double fVal = pArray[nRowStart+nRow];
                                pMat->PutDouble(fVal, nCol, nRow);
                            }
                        }
                    }

                    ScMatrixToken aTok(pMat);
                    aCode2.AddToken(aTok);
                    if(isSumProduct)
                    {
                        npSumSize[nCountMatix/2] =nRowSize*nColSize;
                        nCountMatix++;
                    }
                }
                break;
                default:
                    aCode2.AddToken(*p);
            }
        }

        ScFormulaCell* pDest = rDoc.GetFormulaCell(aTmpPos);
        if (!pDest)
        {
            free(rResult);
            return false;
        }
        if(ocl_calc.GetOpenclState())
        {
            const formula::FormulaToken *pCur = aCode2.First();
            aCode2.Reset();
            while( ( pCur = aCode2.Next() ) != NULL )
            {
                OpCode eOp = pCur->GetOpCode();
                if(eOp==0)
                {
                    if(ocl_calc.gpuEnv.mnKhrFp64Flag==1 || ocl_calc.gpuEnv.mnAmdFp64Flag==1)
                    {
                        if(nCount3%2==0)
                            dpLeftData[nCount1++] = pCur->GetDouble();
                        else
                            dpRightData[nCount2++] = pCur->GetDouble();
                        nCount3++;
                    }
                    else
                    {
                        if(nCount3%2==0)
                            fpLeftData[nCount1++] = (float)pCur->GetDouble();
                        else
                            fpRightData[nCount2++] = (float)pCur->GetDouble();
                        nCount3++;
                    }
                }
                else if( eOp!=ocOpen && eOp!=ocClose &&eOp != ocSep)
                    nOclOp = eOp;

//              if(count1>0){//dbg
//                  fprintf(stderr,"leftData is %f.\n",fpLeftData[count1-1]);
//                  count1--;
//              }
//              if(count2>0){//dbg
//                  fprintf(stderr,"rightData is %f.\n",fpRightData[count2-1]);
//                  count2--;
//              }
            }
        }

        if(!getenv("SC_GPU")||!ocl_calc.GetOpenclState())
        {
            //fprintf(stderr,"ccCPU flow...\n\n");
            generateRPNCode(rDoc, aTmpPos, aCode2);
            ScInterpreter aInterpreter(pDest, &rDoc, aTmpPos, aCode2);
            aInterpreter.Interpret();
            pDest->SetResultToken(aInterpreter.GetResultToken().get());
            pDest->ResetDirty();
            pDest->SetChanged(true);
        }
    } // for loop end (xGroup->mnLength)

    // For GPU calculation
    if(getenv("SC_GPU")&&ocl_calc.GetOpenclState())
    {
        fprintf(stderr,"ggGPU flow...\n\n");
        printf(" oclOp is... %d\n",nOclOp);
        osl_getSystemTime(&aTimeBefore); //timer
        if(ocl_calc.gpuEnv.mnKhrFp64Flag==1 || ocl_calc.gpuEnv.mnAmdFp64Flag==1)
        {
            fprintf(stderr,"ggGPU double precision flow...\n\n");
            //double precision
            switch(nOclOp)
            {
                case ocAdd:
                    ocl_calc.OclHostArithmeticOperator64Bits("oclSignedAdd",dpLeftData,dpRightData,rResult,nCount1);
                    break;
                case ocSub:
                    ocl_calc.OclHostArithmeticOperator64Bits("oclSignedSub",dpLeftData,dpRightData,rResult,nCount1);
                    break;
                case ocMul:
                    ocl_calc.OclHostArithmeticOperator64Bits("oclSignedMul",dpLeftData,dpRightData,rResult,nCount1);
                    break;
                case ocDiv:
                    ocl_calc.OclHostArithmeticOperator64Bits("oclSignedDiv",dpLeftData,dpRightData,rResult,nCount1);
                    break;
                case ocMax:
                    ocl_calc.OclHostFormulaStatistics64Bits("oclFormulaMax",dpOclSrcData,npOclStartPos,npOclEndPos,rResult,rowSize);
                    break;
                case ocMin:
                    ocl_calc.OclHostFormulaStatistics64Bits("oclFormulaMin",dpOclSrcData,npOclStartPos,npOclEndPos,rResult,rowSize);
                    break;
                case ocAverage:
                    ocl_calc.OclHostFormulaStatistics64Bits("oclFormulaAverage",dpOclSrcData,npOclStartPos,npOclEndPos,rResult,rowSize);
                    break;
                case ocSum:
                    ocl_calc.OclHostFormulaStatistics64Bits("oclFormulaSum",dpOclSrcData,npOclStartPos,npOclEndPos,rResult,rowSize);
                    break;
                case ocCount:
                    ocl_calc.OclHostFormulaCount64Bits(npOclStartPos,npOclEndPos,rResult,rowSize);
                    break;
                case ocSumProduct:
                    ocl_calc.OclHostFormulaSumProduct64Bits(dpSumProMergeLfData,dpSumProMergeRtData,npSumSize,rResult,rowSize);
                    break;
                default:
                    fprintf(stderr,"No OpenCL function for this calculation.\n");
                    break;
              }
        }
        else
        {
            fprintf(stderr,"ggGPU float precision flow...\n\n");
            //float precision
            switch(nOclOp)
            {
                case ocAdd:
                    ocl_calc.OclHostArithmeticOperator32Bits("oclSignedAdd",fpLeftData,fpRightData,rResult,nCount1);
                    break;
                case ocSub:
                    ocl_calc.OclHostArithmeticOperator32Bits("oclSignedSub",fpLeftData,fpRightData,rResult,nCount1);
                    break;
                case ocMul:
                    ocl_calc.OclHostArithmeticOperator32Bits("oclSignedMul",fpLeftData,fpRightData,rResult,nCount1);
                    break;
                case ocDiv:
                    ocl_calc.OclHostArithmeticOperator32Bits("oclSignedDiv",fpLeftData,fpRightData,rResult,nCount1);
                    break;
                case ocMax:
                    ocl_calc.OclHostFormulaStatistics32Bits("oclFormulaMax",fpOclSrcData,npOclStartPos,npOclEndPos,rResult,rowSize);
                    break;
                case ocMin:
                    ocl_calc.OclHostFormulaStatistics32Bits("oclFormulaMin",fpOclSrcData,npOclStartPos,npOclEndPos,rResult,rowSize);
                    break;
                case ocAverage:
                    ocl_calc.OclHostFormulaStatistics32Bits("oclFormulaAverage",fpOclSrcData,npOclStartPos,npOclEndPos,rResult,rowSize);
                    break;
                case ocSum:
                    ocl_calc.OclHostFormulaStatistics32Bits("oclFormulaSum",fpOclSrcData,npOclStartPos,npOclEndPos,rResult,rowSize);
                    break;
                case ocCount:
                    ocl_calc.OclHostFormulaCount32Bits(npOclStartPos,npOclEndPos,rResult,rowSize);
                    break;
                case ocSumProduct:
                    ocl_calc.OclHostFormulaSumProduct32Bits(fpSumProMergeLfData,fpSumProMergeRtData,npSumSize,rResult,rowSize);
                    break;
                default:
                    fprintf(stderr,"No OpenCL function for this calculation.\n");
                    break;
              }
        }

        /////////////////////////////////////////////////////
        osl_getSystemTime(&aTimeAfter);
        double diff = getTimeDiff(aTimeAfter, aTimeBefore);
        //if (diff >= 1.0)
        {
            fprintf(stderr,"OpenCL,diff...%f.\n",diff);
        }
/////////////////////////////////////////////////////

//rResult[i];
//           for(sal_Int32 i = 0; i < rowSize; ++i){//dbg output results
//               fprintf(stderr,"After GPU,rRsults[%d] is ...%f\n",i,rResult[i]);
//           }

        // Insert the double data, in rResult[i] back into the document
        rDoc.SetFormulaResults(rTopPos, rResult, xGroup->mnLength);
    }

    free(rResult);

    return true;
}

/// Special case of formula compiler for groundwatering
class FormulaGroupInterpreterGroundwater : public FormulaGroupInterpreterSoftware
{
public:
    FormulaGroupInterpreterGroundwater() :
        FormulaGroupInterpreterSoftware()
    {
        fprintf(stderr,"\n\n ***** Groundwater Backend *****\n\n\n");
        OclCalc::InitEnv();
    }
    virtual ~FormulaGroupInterpreterGroundwater()
    {
        OclCalc::ReleaseOpenclRunEnv();
    }

    virtual ScMatrixRef inverseMatrix(const ScMatrix& /* rMat */) { return ScMatrixRef(); }
    virtual bool interpretCL(ScDocument& rDoc, const ScAddress& rTopPos,
                             const ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode);
    virtual bool interpret(ScDocument& rDoc, const ScAddress& rTopPos,
                           const ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode);
};

#define RETURN_IF_FAIL(a,b) do { if (!(a)) { fprintf (stderr,b"\n"); return false; } } while (0)

#include "compiler.hxx"

bool FormulaGroupInterpreterGroundwater::interpretCL(ScDocument& rDoc, const ScAddress& rTopPos,
                                                     const ScFormulaCellGroupRef& xGroup,
                                                     ScTokenArray& rCode)
{
    generateRPNCode(rDoc, rTopPos, rCode);
    double delta = 0.0;
    // Inputs: both of length xGroup->mnLength
    OpCode eOp = ocNone; // type of operation: ocAverage, ocMax, ocMin
    const double *pArrayToSubtractOneElementFrom = NULL;
    const double *pGroundWaterDataArray = NULL;

    const formula::FormulaToken* p = rCode.FirstRPN();
    if ( p->GetType() == formula::svDouble && !getenv("SC_LCPU") )
    {
        delta = p->GetDouble();
        eOp = ocSub;
    }
    else
    {
        RETURN_IF_FAIL(p != NULL && p->GetOpCode() == ocPush && p->GetType() == formula::svDoubleVectorRef, "double vector ref expected");
        // Get the range reference vector.
        const formula::DoubleVectorRefToken* pDvr = static_cast<const formula::DoubleVectorRefToken*>(p);
        const std::vector<const double*>& rArrays = pDvr->GetArrays();
        RETURN_IF_FAIL(rArrays.size() == 1, "unexpectedly large double ref array");
        RETURN_IF_FAIL(pDvr->GetArrayLength() == (size_t)xGroup->mnLength, "wrong double ref length");
        RETURN_IF_FAIL(pDvr->IsStartFixed() && pDvr->IsEndFixed(), "non-fixed ranges )");
        pGroundWaterDataArray = rArrays[0];

        // Function:
        p = rCode.NextRPN();
        RETURN_IF_FAIL(p != NULL, "no operator");
        eOp = p->GetOpCode();
        RETURN_IF_FAIL(eOp == ocAverage || eOp == ocMax || eOp == ocMin, "unexpected opcode - expected either average, max, or min");
    }

    p = rCode.NextRPN();
    RETURN_IF_FAIL(p != NULL && p->GetOpCode() == ocPush && p->GetType() == formula::svSingleVectorRef, "single vector ref expected");

    // Get the single reference vector.
    const formula::SingleVectorRefToken* pSvr = static_cast<const formula::SingleVectorRefToken*>(p);
    pArrayToSubtractOneElementFrom = pSvr->GetArray();
    RETURN_IF_FAIL(pSvr->GetArrayLength() == (size_t)xGroup->mnLength, "wrong single ref length");

    p = rCode.NextRPN();
    RETURN_IF_FAIL(p != NULL && p->GetOpCode() == ocSub, "missing subtract opcode");

    p = rCode.NextRPN();
    RETURN_IF_FAIL(p == NULL, "there should be no more token");

    static OclCalc ocl_calc;

    // Here we have all the data we need to dispatch our openCL kernel [ I hope ]
    // so for:
    //   =AVERAGE(L$6:L$7701) - L6
    // we would get:
    //   eOp => ocAverage
    //   pGroundWaterDataArray => contains L$5:L$7701
    //   pArrayToSubtractOneElementFrom => contains L$5:L$7701 (ie. a copy)
    //   length of this array -> xGroup->mnLength

    fprintf (stderr, "Calculate !");

    double *pResult = ocl_calc.OclSimpleDeltaOperation( eOp, pGroundWaterDataArray,
                                                        pArrayToSubtractOneElementFrom,
                                                        (size_t) xGroup->mnLength, delta );
    RETURN_IF_FAIL(pResult != NULL, "buffer alloc / calculaton failed");

    // Insert the double data, in rResult[i] back into the document
    rDoc.SetFormulaResults(rTopPos, pResult, xGroup->mnLength);

    delete [] pResult;

    SAL_DEBUG ("exit cleanly !");
    return true;
}

bool FormulaGroupInterpreterGroundwater::interpret(ScDocument& rDoc, const ScAddress& rTopPos,
                                                   const ScFormulaCellGroupRef& xGroup,
                                                   ScTokenArray& rCode)
{
    bool bComplete = interpretCL(rDoc, rTopPos, xGroup, rCode);
    if (!bComplete) // fallback to the (potentially) faster S/W formula group interpreter
        return FormulaGroupInterpreterSoftware::interpret(rDoc, rTopPos, xGroup, rCode);
    else
        return true;
}

namespace opencl {
    sc::FormulaGroupInterpreter *createFormulaGroupInterpreter()
    {
        if (getenv("SC_SOFTWARE"))
        {
            fprintf(stderr, "Create S/W interp\n");
            return new sc::FormulaGroupInterpreterSoftware();
        }
        if (getenv("SC_GROUNDWATER"))
            return new sc::FormulaGroupInterpreterGroundwater();
        else
            return new sc::FormulaGroupInterpreterOpenCL();
    }
} // namespace opencl

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
