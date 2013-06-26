/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formulagroup.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include "formula/vectortoken.hxx"

#ifdef ENABLE_OPENCL
#include "openclwrapper.hxx"
#endif

namespace sc {

FormulaGroupInterpreter::FormulaGroupInterpreter(
    ScDocument& rDoc, const ScAddress& rTopPos, const ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode) :
    mrDoc(rDoc), maTopPos(rTopPos), mxGroup(xGroup), mrCode(rCode) {}

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

bool FormulaGroupInterpreter::interpret()
{
#ifdef ENABLE_OPENCL //dbg
    size_t rowSize = mxGroup->mnLength, srcSize = 0;
    fprintf(stderr,"rowSize at begin is ...%ld.\n",rowSize);
    int *rangeStart =NULL; // The first position for calculation,for example,the A1 in (=MAX(A1:A100))
    int *rangeEnd = NULL; // The last position for calculation,for example, the A100 in (=MAX(A1:A100))
    // The row quantity can be gotten from p2->GetArrayLength()
    int count1 =0,count2 =0,count3=0;
    int oclOp=0;
    double *srcData = NULL; // Point to the input data from CPU
    double *rResult=NULL; // Point to the output data from GPU
    double *leftData=NULL; // Left input for binary operator(+,-,*,/),for example,(=leftData+rightData)
    double *rightData=NULL; // Right input for binary operator(+,-,*,/),for example,(=leftData/rightData)
                            // The rightData can't be zero for "/"

    leftData  = (double *)malloc(sizeof(double) * rowSize);
    rightData = (double *)malloc(sizeof(double) * rowSize);
    rResult   = (double *)malloc(sizeof(double) * rowSize*2);// For 2 columns(B,C)
    srcData = (double *)calloc(rowSize,sizeof(double));

    rangeStart =(int *)malloc(sizeof(int) * rowSize);
    rangeEnd   =(int *)malloc(sizeof(int) * rowSize);

    memset(rResult,0,rowSize);
    if(NULL==leftData||NULL==rightData||
           NULL==rResult||NULL==rangeStart||NULL==rangeEnd)
    {
        printf("malloc err\n");
        return false;
    }
    // printf("rowSize is %d.\n",rowsize);
#endif
    // Until we implement group calculation for real, decompose the group into
    // individual formula token arrays for individual calculation.
    ScAddress aTmpPos = maTopPos;
    for (sal_Int32 i = 0; i < mxGroup->mnLength; ++i)
    {
        aTmpPos.SetRow(mxGroup->mnStart + i);
        ScTokenArray aCode2;
        for (const formula::FormulaToken* p = mrCode.First(); p; p = mrCode.Next())
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
                    ScMatrixRef pMat(new ScMatrix(nColSize, nRowSize, 0.0));
#ifdef ENABLE_OPENCL
                    //srcSize = rowSize+nRowSize-rowSize%nRowSize;//align as nRowSize
                    //srcData = (double *)calloc(srcSize,sizeof(double));
                    rangeStart[i] = nRowStart;//record the start position
                    rangeEnd[i] = nRowEnd;//record the end position
#endif
                    for (size_t nCol = 0; nCol < nColSize; ++nCol)
                    {
                        const double* pArray = rArrays[nCol];
#ifdef ENABLE_OPENCL
                        //printf("pArray is %p.\n",pArray);
                        if( NULL==pArray )
                        {
                            fprintf(stderr,"Error: pArray is NULL!\n");
                            return false;
                        }
                        //fprintf(stderr,"(rowSize+nRowSize-1) is %d.\n",rowSize+nRowSize-1);
                        for( size_t u=0; u<rowSize; u++ )
                        {
                            srcData[u] = pArray[u];// note:rowSize<=srcSize
                            //fprintf(stderr,"srcData[%d] is %f.\n",u,srcData[u]);
                        }
#endif
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
                }
                break;
                default:
                    aCode2.AddToken(*p);
            }
        }

        ScFormulaCell* pDest = mrDoc.GetFormulaCell(aTmpPos);
        if (!pDest)
            return false;

#ifdef ENABLE_OPENCL
        const formula::FormulaToken *pCur = aCode2.First();
        aCode2.Reset();
        while( ( pCur = aCode2.Next() ) != NULL )
        {
            OpCode eOp = pCur->GetOpCode();
            if(eOp==0)
            {
                  if(count3%2==0)
                    leftData[count1++] = pCur->GetDouble();
                   else
                    rightData[count2++] = pCur->GetDouble();
                count3++;
               }
               else if( eOp!=ocOpen && eOp!=ocClose )
                oclOp = eOp;

//            if(count1>0){//dbg
//                fprintf(stderr,"leftData is %f.\n",leftData[count1-1]);
//                count1--;
//            }
//            if(count2>0){//dbg
//                fprintf(stderr,"rightData is %f.\n",rightData[count2-1]);
//                count2--;
//            }
        }
#endif
        if(getenv("SC_FORMULAGROUP")&&(!getenv("SC_GPU"))){
            fprintf(stderr,"ccCPU flow...\n\n");
            ScCompiler aComp(&mrDoc, aTmpPos, aCode2);
            aComp.SetGrammar(mrDoc.GetGrammar());
            aComp.CompileTokenArray(); // Create RPN token array.
            ScInterpreter aInterpreter(pDest, &mrDoc, aTmpPos, aCode2);
            aInterpreter.Interpret();
            pDest->SetResultToken(aInterpreter.GetResultToken().get());
            pDest->ResetDirty();
            pDest->SetChanged(true);
        }
    } // for loop end (mxGroup->mnLength)
    // For GPU calculation
#ifdef ENABLE_OPENCL //dbg: Using "export SC_FORMULAGROUP=1;export SC_GPU=1" to open if{} in terminal
    if(getenv("SC_FORMULAGROUP")&&(getenv("SC_GPU"))){
            fprintf(stderr,"ggGPU flow...\n\n");
            printf(" oclOp is... %d\n",oclOp);
osl_getSystemTime(&aTimeBefore);//timer
            static OclCalc ocl_calc;
            switch(oclOp)
            {
                case ocAdd:
                       ocl_calc.OclHostSignedAdd(leftData,rightData,rResult,count1);
                    break;
                case ocSub:
                    ocl_calc.OclHostSignedSub(leftData,rightData,rResult,count1);
                    break;
                case ocMul:
                    ocl_calc.OclHostSignedMul(leftData,rightData,rResult,count1);
                    break;
                case ocDiv:
                    ocl_calc.OclHostSignedDiv(leftData,rightData,rResult,count1);
                    break;
                case ocMax:
                    ocl_calc.OclHostFormulaMax(srcData,rangeStart,rangeEnd,rResult,rowSize);
                    break;
                case ocMin:
                    ocl_calc.OclHostFormulaMin(srcData,rangeStart,rangeEnd,rResult,rowSize);
                    break;
                case ocAverage:
                    ocl_calc.OclHostFormulaAverage(srcData,rangeStart,rangeEnd,rResult,rowSize);
                    break;
                default:
                    fprintf(stderr,"No OpenCL function for this calculation.\n");
                    break;
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
//            for(sal_Int32 i = 0; i < rowSize; ++i){//dbg output results
//                fprintf(stderr,"After GPU,rRsults[%d] is ...%f\n",i,rResult[i]);
//            }

// We want to stuff the double data, which in rResult[i] from GPU calculated well, to UI view for users
                for (sal_Int32 i = 0; i < mxGroup->mnLength; ++i)
                    {
                    ScFormulaCell* pDestx = mrDoc.GetFormulaCell(aTmpPos);
                    if (!pDestx)
                        return false;
                    formula::FormulaTokenRef xResult = new formula::FormulaDoubleToken(rResult[i]);
                    pDestx->SetResultToken(xResult.get());
                    pDestx->ResetDirty();
                    pDestx->SetChanged(true);
                    aTmpPos.SetRow(mxGroup->mnStart + i + 1);
                 }
        }

        if(leftData)
            free(leftData);
        if(rightData)
            free(rightData);
        if(rangeStart)
            free(rangeStart);
        if(rangeEnd)
            free(rangeEnd);
        if(rResult)
            free(rResult);
        if(srcData)
            free(srcData);

if(getenv("SC_GPUSAMPLE")){
    //fprintf(stderr,"FormulaGroupInterpreter::interpret(),iniflag...%d\n",ocl_calc.GetOpenclState());
    //ocl_calc.OclTest();//opencl test sample for debug
}
#endif

    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
