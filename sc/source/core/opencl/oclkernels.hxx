/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _OCL_KERNEL_H_
#define _OCL_KERNEL_H_

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
    otData[id] =ltData[id] * rtData[id];
}

__kernel void oclSignedDiv(__global fp_t *ltData,__global fp_t *rtData,__global fp_t *otData)
{
    const unsigned int id = get_global_id(0);
    otData[id] = ltData[id] / rtData[id];
}

__kernel void oclFormulaMin(__global fp_t *input,__global int *start,__global int *end,__global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    unsigned int startFlag = start[id];
    unsigned int endFlag = end[id];
    fp_t min = input[startFlag];
    for(int i=startFlag;i<=endFlag;i++)
    {
        if(input[i]<min)
            min = input[i];
    }
    output[id] = min;
}

__kernel void oclFormulaMax(__global fp_t *input,__global int *start,__global int *end,__global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    unsigned int startFlag = start[id];
    unsigned int endFlag = end[id];
    fp_t max = input[startFlag];
    for(int i=startFlag;i<=endFlag;i++)
    {
        if(input[i]>max)
            max = input[i];
    }
    output[id] = max;
}
//Sum
__kernel void oclFormulaSum(__global fp_t *input,__global int *start,__global int *end,__global fp_t *output)
{
    const unsigned int nId = get_global_id(0);
    fp_t fSum = 0.0;
    for(int i = start[nId]; i<=end[nId]; i++)
        fSum += input[i];
    output[nId] = fSum ;
}
//Count
__kernel void oclFormulaCount(__global int *start,__global int *end,__global fp_t *output)
{
    const unsigned int nId = get_global_id(0);
    output[nId] = end[nId] - start[nId] + 1;
}

__kernel void oclFormulaAverage(__global fp_t *input,__global int *start,__global int *end,__global fp_t *output)
{
    const unsigned int id = get_global_id(0);
    fp_t sum=0.0;
    for(int i = start[id];i<=end[id];i++)
        sum += input[i];
    output[id] = sum / (end[id]-start[id]+1);
}

//Sumproduct
__kernel void oclFormulaSumproduct(__global fp_t *firstCol,__global int* npSumSize,__global fp_t *output,uint nMatixSize)
{
    const unsigned int id = get_global_id(0);
    unsigned int nSumSize = npSumSize[id];
    fp_t fSum = 0.0;
    for(int i=0;i<nSumSize;i++)
        fSum += firstCol[i + nMatixSize * id];
    output[id] = fSum;
}

__kernel void oclFormulaMinverse(__global fp_t *data, const uint type)
{

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
    for(int i=start+1;i < end;i++)
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
    for(int i=start+1;i < end;i++)
    {
        if(values[i]<fMinVal)
            fMinVal = values[i];
    }

    // Subtract & output
    output[id] = fMinVal - subtract[id];
}


);

#endif // USE_EXTERNAL_KERNEL
#endif //_OCL_KERNEL_H_
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
