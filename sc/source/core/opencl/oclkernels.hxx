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

__kernel void oclFormulaSum(__global float *data,
					   const uint type)
{

}

__kernel void oclFormulaCount(__global float *data,
					   const uint type)
{

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


__kernel void oclFormulaSumproduct(__global float *data,
					   const uint type)
{

}

__kernel void oclFormulaMinverse(__global float *data,
					   const uint type)
{

}

);

#endif // USE_EXTERNAL_KERNEL
#endif //_OCL_KERNEL_H_
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
