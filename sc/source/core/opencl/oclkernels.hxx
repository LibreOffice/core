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

/////////////////////////////////////////////
const char *kernel_src = KERNEL(
__kernel void hello(__global uint *buffer)

{
    size_t idx = get_global_id(0);
    buffer[idx]=idx;
}

__kernel void oclformula(__global float *data,
                       const uint type)
{
    const unsigned int i = get_global_id(0);

    switch (type)
    {
        case 0:          //MAX
        {
            //printf("%i ?%f>%f\n",i,data[2*i],data[2*i+1]);
            if(data[2*i]>data[2*i+1])
                data[i] = data[2*i];
            else
                data[i] = data[2*i+1];
            break;
        }
        case 1:          //MIN
        {
            //printf("%d ?%d<%d\n",i,data[2*i],data[2*i+1]);
            if(data[2*i]<data[2*i+1])
                data[i] = data[2*i];
            else
                data[i] = data[2*i+1];
            break;
        }
        case 2:          //SUM
        case 3:          //AVG
        {
            //printf("%d %d+%d\n",i,data[2*i],data[2*i+1]);
            data[i] = data[2*i] + data[2*i+1];
            break;
        }
        default:
            break;

    }
}


__kernel void oclSignedAdd(__global float *ltData,__global float *rtData,__global float *otData)
{
    const unsigned int id = get_global_id(0);
    otData[id] = ltData[id] + rtData[id];
}


__kernel void oclSignedSub(__global float *ltData,__global float *rtData,__global float *otData)
{
    const unsigned int id = get_global_id(0);
    otData[id] = ltData[id] - rtData[id];

}

__kernel void oclSignedMul(__global float *ltData,__global float *rtData,__global float *otData)
{
    int id = get_global_id(0);
    otData[id] =ltData[id] * rtData[id];
}

__kernel void oclSignedDiv(__global float *ltData,__global float *rtData,__global float *otData)
{
    const unsigned int id = get_global_id(0);
    otData[id] = ltData[id] / rtData[id];
}

__kernel void oclFormulaMin(__global float *input,__global int *start,__global int *end,__global float *output)
{
    const unsigned int id = get_global_id(0);
    int i=0;
    unsigned int startFlag = start[id];
    unsigned int endFlag = end[id];
    float min = input[startFlag];
    for(i=startFlag;i<=endFlag;i++)
    {
        if(input[i]<min)
            min = input[i];
    }
    output[id] = min;

}

__kernel void oclFormulaMax(__global float *input,__global int *start,__global int *end,__global float *output)
{
    const unsigned int id = get_global_id(0);
    int i=0;
    unsigned int startFlag = start[id];
    unsigned int endFlag = end[id];
    float max = input[startFlag];
    for(i=startFlag;i<=endFlag;i++)
    {
        if(input[i]>max)
            max = input[i];
    }
    output[id] = max;

}
//Sum
__kernel void oclFormulaSum(__global float *input,__global int *start,__global int *end,__global float *output)
{
    const unsigned int nId = get_global_id(0);
    float fSum = 0.0f;
    for(int i = start[nId]; i<=end[nId]; i++)
        fSum += input[i];
    output[nId] = fSum ;
}
//Count
__kernel void oclFormulaCount(__global int *start,__global int *end,__global float *output)
{
    const unsigned int nId = get_global_id(0);
    output[nId] = end[nId] - start[nId] + 1;
}

__kernel void oclFormulaAverage(__global float *input,__global int *start,__global int *end,__global float *output)
{
    const unsigned int id = get_global_id(0);
    int i=0;
    float sum=0;
    for(i = start[id];i<=end[id];i++)
        sum += input[i];
    output[id] = sum / (end[id]-start[id]+1);
}

//Sumproduct
__kernel void oclFormulaSumproduct(__global float *firstCol,__global float *secondCol,__global int *start,__global int *end,__global float *output)
{
    const int nId = get_global_id(0);
    int nCount     = start[nId] - end[nId] + 1;
    int nStartA  = start[nId*2];
    int nStartB  = start[nId*2+1];
    for(int i = 0; i<nCount; i++)
        output[nId] += firstCol[nStartA+i]*secondCol[nStartB+i];
}

__kernel void oclFormulaMinverse(__global float *data,
                       const uint type)
{

}

// Double precision is a requirement of spreadsheets
// cl_khr_fp64: Khronos extension
// cl_amd_fp64: AMD extension
\n#if 0 \n
\n#if defined(cl_khr_fp64) \n
\n#pragma OPENCL EXTENSION cl_khr_fp64 : enable \n
\n#elif defined(cl_amd_fp64) \n
\n#pragma OPENCL EXTENSION cl_amd_fp64 : enable \n
\n#endif \n
\ntypedef double fp_t; \n
\n#else \n
\ntypedef float fp_t; \n
\n#endif \n

__kernel void oclAverageDelta(__global fp_t *values, __global fp_t *subtract, uint start, uint end, __global fp_t *output)
{
    const unsigned int id = get_global_id(0);

    // Average
    fp_t fSum = 0.0f;
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
    float fMaxVal = values[start];
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
    float fMinVal = values[start];
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
