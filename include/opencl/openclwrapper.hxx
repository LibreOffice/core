/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_CORE_OPENCL_OPENCLWRAPPER_HXX
#define INCLUDED_SC_SOURCE_CORE_OPENCL_OPENCLWRAPPER_HXX

#include <config_features.h>

#include <cassert>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <clew/clew.h>

#include <sal/detail/log.h>
#include <opencl/opencldllapi.h>
#include <opencl/platforminfo.hxx>
#include <osl/file.hxx>
#include <rtl/string.hxx>


#define CHECK_OPENCL(status,name)    \
if( status != CL_SUCCESS )    \
{    \
    printf ("OpenCL error code is %d at " SAL_DETAIL_WHERE " when %s .\n", status, name);    \
    return false;    \
}

#define MAX_CLFILE_NUM 50

#include <cstdio>

namespace opencl {

struct KernelEnv
{
    cl_context mpkContext;
    cl_command_queue mpkCmdQueue;
    cl_program mpkProgram;
};

struct GPUEnv
{
    //share vb in all modules in hb library
    cl_platform_id mpPlatformID;
    cl_device_type mDevType;
    cl_context mpContext;
    cl_device_id *mpArryDevsID;
    cl_device_id mpDevID;
    cl_command_queue mpCmdQueue;
    cl_program mpArryPrograms[MAX_CLFILE_NUM]; //one program object maps one kernel source file
    int mnIsUserCreated; // 1: created , 0:no create and needed to create by opencl wrapper
    bool mnKhrFp64Flag;
    bool mnAmdFp64Flag;
};

extern OPENCL_DLLPUBLIC GPUEnv gpuEnv;
OPENCL_DLLPUBLIC bool generatBinFromKernelSource( cl_program program, const char * clFileName );
OPENCL_DLLPUBLIC bool buildProgramFromBinary(const char* buildOption, GPUEnv* gpuEnv, const char* filename, int idx);
OPENCL_DLLPUBLIC void setKernelEnv( KernelEnv *envInfo );
OPENCL_DLLPUBLIC const std::vector<OpenCLPlatformInfo>& fillOpenCLInfo();

/**
 * Used to set or switch between OpenCL devices.
 *
 * @param pDeviceId the id of the opencl device of type cl_device_id, NULL means use software calculation
 * @param bAutoSelect use the algorithm to select the best OpenCL device
 *
 * @return returns true if there is a valid opencl device that has been set up
 */
OPENCL_DLLPUBLIC bool switchOpenCLDevice(const OUString* pDeviceId, bool bAutoSelect,
                                         bool bForceEvaluation);

OPENCL_DLLPUBLIC void getOpenCLDeviceInfo(size_t& rDeviceId, size_t& rPlatformId);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
