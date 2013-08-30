/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_OCLKERNELS_HXX
#define SC_OCLKERNELS_HXX

#ifndef USE_EXTERNAL_KERNEL
#define KERNEL( ... )# __VA_ARGS__
// Double precision is a default of spreadsheets
// cl_khr_fp64: Khronos extension
// cl_amd_fp64: AMD extension
// use build option outside to define fp_t
/////////////////////////////////////////////
const char *kernel_src = KERNEL(
\n#ifdef KHR_DP_EXTENSION\n
\n#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n
\n#elif AMD_DP_EXTENSION\n
\n#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n
\n#else\n
\n#endif\n
inline fp_t oclAverage( const uint id,__global fp_t *values,__global uint *startArray,__global uint *endArray)
{
    uint start = startArray[id];
    uint end = endArray[id];
    fp_t fSum = 0.0;
    fp_t zero[16] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
    fp_t16 vSum=vload16(0,zero);
    fp_t16 ptr;
    __global fp_t *p = values;
    p+= start;

    for(int i = 0; i < (end - start + 1)/16; ++i)
    {
        ptr=vload16(0,p);
        vSum += ptr;
        p+=16;
    }
    int lastData = (end-start+1)%16;
    for(int i = 0; i <lastData; i++)
    {
        fSum += *p;
        p+=1;
    }
    vSum.s01234567 = vSum.s01234567+vSum.s89abcdef;
    vSum.s0123 = vSum.s0123+vSum.s4567;
    vSum.s01 = vSum.s01+vSum.s23;
    vSum.s0 = vSum.s0+vSum.s1;
    fSum = vSum.s0+fSum;
    fp_t fVal = fSum/(end-start+1);
    return fVal;
}
inline fp_t oclMax( const uint id,__global fp_t *values,__global uint *startArray,__global uint *endArray)
{
    uint start = startArray[id];
    uint end = endArray[id];
    fp_t fMax = values[start];
    fp_t zero[16] = {fMax,fMax,fMax,fMax,fMax,fMax,fMax,fMax,fMax,fMax,fMax,fMax,fMax,fMax,fMax,fMax};
    fp_t16 vMax=vload16(0,zero);
    //Max
    fp_t16 ptr;
    __global fp_t *p = values;
    p+= start;
    for(int i = 0; i < (end - start + 1)/16; ++i)
    {
        ptr=vload16(0,p);
        vMax = fmax(vMax,ptr);
        p+=16;
    }
    int lastData = (end-start+1)%16;
    for(int i = 0; i <lastData; i++)
    {
        fMax = fmax(fMax,*p);
        p+=1;
    }
    vMax.s01234567 = fmax(vMax.s01234567, vMax.s89abcdef);
    vMax.s0123 = fmax(vMax.s0123, vMax.s4567);
    vMax.s01 = fmax(vMax.s01, vMax.s23);
    vMax.s0 = fmax(vMax.s0, vMax.s1);
    fMax = fmax(vMax.s0, fMax);
    return fMax;
}
inline fp_t oclMin( const uint id,__global fp_t *values,__global uint *startArray,__global uint *endArray)
{
    uint start = startArray[id];
    uint end = endArray[id];
    fp_t fMin = values[start];
    fp_t zero[16] = {fMin,fMin,fMin,fMin,fMin,fMin,fMin,fMin,fMin,fMin,fMin,fMin,fMin,fMin,fMin,fMin};
    fp_t16 vMin=vload16(0,zero);
    //Min
    fp_t16 ptr;
    __global fp_t *p = values;
    p+= start;
    for(int i = 0; i < (end - start + 1)/16; ++i)
    {
        ptr=vload16(0,p);
        vMin = fmin(vMin,ptr);
        p+=16;
    }
    int lastData = (end-start+1)%16;
    for(int i = 0; i <lastData; i++)
    {
        fMin = fmin(fMin,*p);
        p+=1;
    }
    vMin.s01234567 = fmin(vMin.s01234567, vMin.s89abcdef);
    vMin.s0123 = fmin(vMin.s0123, vMin.s4567);
    vMin.s01 = fmin(vMin.s01, vMin.s23);
    vMin.s0 = fmin(vMin.s0, vMin.s1);
    fMin = fmin(vMin.s0, fMin);
    return fMin;
}

__kernel void oclSignedAdd(__global fp_t *ltData,__global fp_t *rtData,__global fp_t *otData)
{
    const unsigned int id = get_global_id(0);
    otData[id] = ltData[id] + rtData[id];
}

__kernel void oclSignedSub(__global fp_t *ltData,__global fp_t *rtData,__global fp_t *otData)
{
    const unsigned int id = get_global_id(0);
    otData[id] = ltData[id] - rtData[id];
}

__kernel void oclSignedMul(__global fp_t *ltData,__global fp_t *rtData,__global fp_t *otData)
{
    int id = get_global_id(0);
    otData[id] = ltData[id] * rtData[id];
}

__kernel void oclSignedDiv(__global fp_t *ltData,__global fp_t *rtData,__global fp_t *otData)
{
    const unsigned int id = get_global_id(0);
    fp_t divisor = rtData[id];
    if ( divisor != 0 )
        otData[id] = ltData[id] / divisor;
    else
        otData[id] = 0.0;
}

__kernel void oclFormulaMin(__global fp_t *input,__global uint *start,__global uint *end,__global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fVal = oclMin(id,input,start,end);
    output[id] = fVal ;
}

__kernel void oclFormulaMax(__global fp_t *input,__global uint *start,__global uint *end,__global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fVal = oclMax(id,input,start,end);
    output[id] = fVal ;
}
//Sum
__kernel void oclFormulaSum(__global fp_t *input,__global uint *start,__global uint *end,__global fp_t *output)
{
    const unsigned int nId = get_global_id(0);
    fp_t fSum = 0.0;
    for(int i = start[nId]; i<=end[nId]; i++)
        fSum += input[i];
    output[nId] = fSum ;
}
//Count
__kernel void oclFormulaCount(__global uint *start,__global uint *end,__global fp_t *output)
{
    const unsigned int nId = get_global_id(0);
    output[nId] = end[nId] - start[nId] + 1;
}

__kernel void oclFormulaAverage(__global fp_t *input,__global uint *start,__global uint *end,__global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fVal = oclAverage(id,input,start,end);
    output[id] = fVal ;

}
//Sumproduct
__kernel void oclFormulaSumproduct(__global fp_t *firstCol,__global uint* npSumSize,__global fp_t *output,uint nMatixSize)
{
    const unsigned int id = get_global_id(0);
    unsigned int nSumSize = npSumSize[id];
    fp_t fSum = 0.0;
    for(int i=0;i<nSumSize;i++)
        fSum += firstCol[i + nMatixSize * id];
    output[id] = fSum;
}

__kernel void oclAverageDelta(__global fp_t *values, __global fp_t *subtract, uint start, uint end, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);

    // Average
    fp_t fSum = 0.0;
    for(int i = start; i < end; i++)
        fSum += values[i];
    fp_t fVal = fSum/(end-start);

    // Subtract & output
    output[id] = fVal - subtract[id];
}

__kernel void oclMaxDelta(__global fp_t *values, __global fp_t *subtract, uint start, uint end, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);

    // Max
    fp_t fMaxVal = values[start];
    for ( int i = start + 1; i < end; i++ )
    {
        if(values[i]>fMaxVal)
            fMaxVal = values[i];
    }

    // Subtract & output
    output[id] = fMaxVal - subtract[id];
}

__kernel void oclMinDelta(__global fp_t *values, __global fp_t *subtract, uint start, uint end, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);

    // Min
    fp_t fMinVal = values[start];
    for ( int i = start + 1; i < end; i++ )
    {
        if(values[i]<fMinVal)
            fMinVal = values[i];
    }

    // Subtract & output
    output[id] = fMinVal - subtract[id];
}

__kernel void oclSubDelta( fp_t ltData, __global fp_t *rtData, __global fp_t *outData )
{
    const unsigned int id = get_global_id(0);
    outData[id] = ltData - rtData[id];
}

__kernel void oclFormulaMtxInv(__global fp_t * fpMatrixInput, __global fp_t * fpP, int nOffset, int nMax)
{
    //get the global id of the workitem
    int nId = get_global_id(0);
    int nDimension = get_global_size(0);
    fp_t dMovebuffer;
    dMovebuffer = fpMatrixInput[nOffset*nDimension+nId];
    fpMatrixInput[nOffset*nDimension+nId] = fpMatrixInput[nMax*nDimension+nId];
    fpMatrixInput[nMax*nDimension+nId] = dMovebuffer;

    dMovebuffer = fpP[nOffset*nDimension+nId];
    fpP[nOffset*nDimension+nId] = fpP[nMax*nDimension+nId];
    fpP[nMax*nDimension+nId] = dMovebuffer;
}
__kernel void oclMatrixSolve(__global fp_t * fpMatrixInput,__global fp_t * fpMatrixOutput,__global fp_t * fpP,__global fp_t * fpY,__global uint* npDim)
{
    int nId = get_global_id(0);
    int nDimension = npDim[nId];
    fp_t fsum = 0.0;
    for ( int yi=0; yi < nDimension; yi++ )
    {
        fsum = 0.0;
        for ( int yj=0; yj < nDimension; yj++ )
        {
            fsum += fpMatrixInput[yi*nDimension+yj] * fpY[nId+yj*nDimension];
        }

        fpY[nId+yi*nDimension] = fpP[yi*nDimension+nId] - fsum;
    }
    for ( int xi = nDimension - 1; xi >= 0; xi-- )
    {
        fsum = 0.0;
        for ( int xj = 0; xj < nDimension; xj++ )
        {
            fsum += fpMatrixInput[xi*nDimension+xj] * fpMatrixOutput[nId+nDimension*xj];
        }
        fpMatrixOutput[nId+xi*nDimension] = (fpY[xi*nDimension+nId] - fsum) / fpMatrixInput[xi*nDimension+xi];
    }
}

__kernel void oclAverageAdd(__global fp_t *values,__global fp_t *addend, __global uint *startArray, __global uint *endArray, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fVal = oclAverage(id,values,startArray,endArray);
    output[id] = fVal + addend[id];
}

__kernel void oclAverageSub(__global fp_t *values,__global fp_t *subtract, __global uint *startArray, __global uint *endArray, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fVal = oclAverage(id,values,startArray,endArray);
    output[id] = fVal - subtract[id];
}

__kernel void oclAverageMul(__global fp_t *values,__global fp_t *multiplier, __global uint *startArray, __global uint *endArray, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fVal = oclAverage(id,values,startArray,endArray);
    output[id] = fVal * multiplier[id];
}
__kernel void oclAverageDiv(__global fp_t *values,__global fp_t *div, __global uint *startArray, __global uint *endArray, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fVal = oclAverage(id,values,startArray,endArray);
    fp_t divisor = div[id];
    if ( divisor != 0 )
        output[id] = fVal / divisor;
    else
        output[id] = 0.0;
}

__kernel void oclMinAdd(__global fp_t *values, __global fp_t *addend, __global uint *startArray, __global uint *endArray, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fMin = oclMin(id,values,startArray,endArray);
    output[id] = fMin + addend[id];
}

__kernel void oclMinSub(__global fp_t *values, __global fp_t *subtract, __global uint *startArray, __global uint *endArray, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fMin = oclMin(id,values,startArray,endArray);
    output[id] = fMin - subtract[id];
}
__kernel void oclMinMul(__global fp_t *values, __global fp_t *multiplier, __global uint *startArray, __global uint *endArray, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fMin = oclMin(id,values,startArray,endArray);
    output[id] = fMin * multiplier[id];
}
__kernel void oclMinDiv(__global fp_t *values, __global fp_t *div, __global uint *startArray, __global uint *endArray, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fMin = oclMin(id,values,startArray,endArray);
    fp_t divisor = div[id];
    if ( divisor != 0 )
        output[id] = fMin / divisor;
    else
        output[id] = 0.0;
}
__kernel void oclMaxAdd(__global fp_t *values, __global fp_t *addend, __global uint *startArray, __global uint *endArray, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fMax = oclMax(id,values,startArray,endArray);
    output[id] = fMax + addend[id];
}

__kernel void oclMaxSub(__global fp_t *values, __global fp_t *subtract, __global uint *startArray, __global uint *endArray, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fMax = oclMax(id,values,startArray,endArray);
    output[id] = fMax - subtract[id];
}
__kernel void oclMaxMul(__global fp_t *values, __global fp_t *multiplier, __global uint *startArray, __global uint *endArray, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fMax = oclMax(id,values,startArray,endArray);
    output[id] = fMax * multiplier[id];
}
__kernel void oclMaxDiv(__global fp_t *values, __global fp_t *div, __global uint *startArray, __global uint *endArray, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t fMax = oclMax(id,values,startArray,endArray);
    fp_t divisor = div[id];
    if ( divisor != 0 )
        output[id] = fMax / divisor;
    else
        output[id] = 0.0;
}

__kernel void oclSub( fp_t ltData, __global fp_t *rtData, __global fp_t *outData )
{
    const unsigned int id = get_global_id(0);
    outData[id] = ltData - rtData[id];
}
);

#endif // USE_EXTERNAL_KERNEL
#endif //_OCL_KERNEL_H_
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
