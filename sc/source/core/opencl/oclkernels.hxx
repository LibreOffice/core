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
//Add kernel here
//use \n ... \n to define macro
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

__kernel void oclFormulaMin(__global float *data,
                            const uint type)
{

}

__kernel void oclFormulaMax(__global float *data,
                            const uint type)
{

}

__kernel void oclFormulaSum(__global float *data,
                            const uint type)
{

}

__kernel void oclFormulaCount(__global float *data,
                              const uint type)
{

}

__kernel void oclFormulaAverage(__global float *data,
                                const uint type)
{

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
