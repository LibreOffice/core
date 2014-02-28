/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_folders.h>

#include "openclwrapper.hxx"

#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/digest.h>
#include <rtl/bootstrap.hxx>
#include <boost/scoped_array.hpp>

#include "sal/config.h"
#include <osl/file.hxx>
#include "opencl_device.hxx"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#ifdef WIN32
#include <windows.h>
#define OPENCL_DLL_NAME "OpenCL.dll"
#elif defined(MACOSX)
#define OPENCL_DLL_NAME NULL
#else
#define OPENCL_DLL_NAME "libOpenCL.so"
#endif

#define DEVICE_NAME_LENGTH 1024
#define DRIVER_VERSION_LENGTH 1024
#define PLATFORM_VERSION_LENGTH 1024

using namespace std;

namespace sc { namespace opencl {

GPUEnv OpenclDevice::gpuEnv;
bool OpenclDevice::bIsInited = false;

namespace {

OString generateMD5(const void* pData, size_t length)
{
    sal_uInt8 pBuffer[RTL_DIGEST_LENGTH_MD5];
    rtlDigestError aError = rtl_digest_MD5(pData, length,
            pBuffer, RTL_DIGEST_LENGTH_MD5);
    SAL_WARN_IF(aError != rtl_Digest_E_None, "sc", "md5 generation failed");

    OStringBuffer aBuffer;
    const char* pString = "0123456789ABCDEF";
    for(size_t i = 0; i < RTL_DIGEST_LENGTH_MD5; ++i)
    {
        sal_uInt8 val = pBuffer[i];
        aBuffer.append(pString[val/16]);
        aBuffer.append(pString[val%16]);
    }
    return aBuffer.makeStringAndClear();
}

OString getCacheFolder()
{
    OUString url("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/cache/");
    rtl::Bootstrap::expandMacros(url);

    osl::Directory::create(url);

    return rtl::OUStringToOString(url, RTL_TEXTENCODING_UTF8);
}

void clearCache()
{
#if 0
    // We used to delete all files that did not end with the hash of
    // the static kernel source string from oclkernels.hxx. But as
    // those static kernels were not used for anything, it was
    // pointless, that hash never changed. The static kernels are now
    // removed, their hash is not part of the .bin file names any
    // more.  So there is little this function can do until we come up
    // with some other way to figure out which cached .bin files are
    // "current".
    OUString aCacheDirURL(rtl::OStringToOUString(OpenclDevice::maCacheFolder, RTL_TEXTENCODING_UTF8));
    osl::Directory aCacheDir(aCacheDirURL);
    osl::FileBase::RC status = aCacheDir.open();
    if(status != osl::FileBase::E_None)
        return;

    osl::DirectoryItem aItem;
    while(osl::FileBase::E_None == aCacheDir.getNextItem(aItem))
    {
        osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileName|osl_FileStatus_Mask_FileURL);
        status = aItem.getFileStatus(aFileStatus);
        if(status != osl::FileBase::E_None)
            continue;

        OUString aFileName = aFileStatus.getFileName();
        if(aFileName.endsWith(".bin"))
        {
            if ( file is in some way obsolete )
            {
                // delete the file
                OUString aFileUrl = aFileStatus.getFileURL();
                osl::File::remove(aFileUrl);
            }
        }
    }
#endif
}

}

OString OpenclDevice::maCacheFolder = getCacheFolder();

void OpenclDevice::registOpenclKernel()
{
    if ( !gpuEnv.mnIsUserCreated )
        memset( &gpuEnv, 0, sizeof(gpuEnv) );
}

void OpenclDevice::setKernelEnv( KernelEnv *envInfo )
{
    envInfo->mpkContext = gpuEnv.mpContext;
    envInfo->mpkCmdQueue = gpuEnv.mpCmdQueue;
    envInfo->mpkProgram = gpuEnv.mpArryPrograms[0];
}

namespace {

OString createFileName(cl_device_id deviceId, const char* clFileName)
{
    OString fileName(clFileName);
    sal_Int32 nIndex = fileName.lastIndexOf(".cl");
    if(nIndex > 0)
        fileName = fileName.copy(0, nIndex);

    char deviceName[DEVICE_NAME_LENGTH] = {0};
    clGetDeviceInfo(deviceId, CL_DEVICE_NAME,
            sizeof(deviceName), deviceName, NULL);

    char driverVersion[DRIVER_VERSION_LENGTH] = {0};
    clGetDeviceInfo(deviceId, CL_DRIVER_VERSION,
            sizeof(driverVersion), driverVersion, NULL);

    cl_platform_id platformId;
    clGetDeviceInfo(deviceId, CL_DEVICE_PLATFORM,
            sizeof(platformId), &platformId, NULL);

    char platformVersion[PLATFORM_VERSION_LENGTH] = {0};
    clGetPlatformInfo(platformId, CL_PLATFORM_VERSION, sizeof(platformVersion),
            platformVersion, NULL);

    // create hash for deviceName + driver version + platform version
    OString aString = OString(deviceName) + driverVersion + platformVersion;
    OString aHash = generateMD5(aString.getStr(), aString.getLength());

    return OpenclDevice::maCacheFolder + fileName + "-" +
        aHash + ".bin";
}

}

std::vector<boost::shared_ptr<osl::File> > OpenclDevice::binaryGenerated( const char * clFileName, cl_context context )
{
    size_t numDevices=0;

    std::vector<boost::shared_ptr<osl::File> > aGeneratedFiles;
    cl_int clStatus = clGetContextInfo( context, CL_CONTEXT_DEVICES,
            0, NULL, &numDevices );
    numDevices /= sizeof(numDevices);

    if(clStatus != CL_SUCCESS)
        return aGeneratedFiles;


    // grab the handles to all of the devices in the context.
    boost::scoped_array<cl_device_id> mpArryDevsID(new cl_device_id[numDevices]);
    clStatus = clGetContextInfo( context, CL_CONTEXT_DEVICES,
            sizeof( cl_device_id ) * numDevices, mpArryDevsID.get(), NULL );

    if(clStatus != CL_SUCCESS)
        return aGeneratedFiles;

    for ( size_t i = 0; i < numDevices; i++ )
    {
        if ( mpArryDevsID[i] != 0 )
        {
            OString fileName = createFileName(gpuEnv.mpArryDevsID[i], clFileName);
            osl::File* pNewFile = new osl::File(rtl::OStringToOUString(fileName, RTL_TEXTENCODING_UTF8));
            if(pNewFile->open(osl_File_OpenFlag_Read) == osl::FileBase::E_None)
            {
                aGeneratedFiles.push_back(boost::shared_ptr<osl::File>(pNewFile));
                SAL_INFO("sc.opencl", "Opening binary file '" << fileName << "' for reading: success");
            }
            else
            {
                SAL_INFO("sc.opencl", "Opening binary file '" << fileName << "' for reading: FAIL");
                delete pNewFile;
                break;
            }
        }
    }

    return aGeneratedFiles;
}

bool OpenclDevice::writeBinaryToFile( const OString& rFileName, const char* binary, size_t numBytes )
{
    clearCache();
    osl::File file(rtl::OStringToOUString(rFileName, RTL_TEXTENCODING_UTF8));
    osl::FileBase::RC status = file.open(
            osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );

    if(status != osl::FileBase::E_None)
        return false;

    sal_uInt64 nBytesWritten = 0;
    file.write( binary, numBytes, nBytesWritten );

    assert(numBytes == nBytesWritten);

    return true;
}

bool OpenclDevice::generatBinFromKernelSource( cl_program program, const char * clFileName )
{
    cl_uint numDevices;

    cl_int clStatus = clGetProgramInfo( program, CL_PROGRAM_NUM_DEVICES,
                   sizeof(numDevices), &numDevices, NULL );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    std::vector<cl_device_id> mpArryDevsID(numDevices);
    /* grab the handles to all of the devices in the program. */
    clStatus = clGetProgramInfo( program, CL_PROGRAM_DEVICES,
                   sizeof(cl_device_id) * numDevices, &mpArryDevsID[0], NULL );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    /* figure out the sizes of each of the binaries. */
    std::vector<size_t> binarySizes(numDevices);

    clStatus = clGetProgramInfo( program, CL_PROGRAM_BINARY_SIZES,
                   sizeof(size_t) * numDevices, &binarySizes[0], NULL );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    /* copy over all of the generated binaries. */
    boost::scoped_array<char*> binaries(new char*[numDevices]);

    for ( size_t i = 0; i < numDevices; i++ )
    {
        if ( binarySizes[i] != 0 )
        {
            binaries[i] = new char[binarySizes[i]];
        }
        else
        {
            binaries[i] = NULL;
        }
    }

    clStatus = clGetProgramInfo( program, CL_PROGRAM_BINARIES,
                   sizeof(char *) * numDevices, binaries.get(), NULL );
    CHECK_OPENCL(clStatus,"clGetProgramInfo");

    /* dump out each binary into its own separate file. */
    for ( size_t i = 0; i < numDevices; i++ )
    {

        if ( binarySizes[i] != 0 )
        {
            OString fileName = createFileName(mpArryDevsID[i], clFileName);
            if ( !writeBinaryToFile( fileName,
                        binaries[i], binarySizes[i] ) )
                SAL_INFO("sc.opencl", "Writing binary file '" << fileName << "': FAIL");
            else
                SAL_INFO("sc.opencl", "Writing binary file '" << fileName << "': success");
        }
    }

    // Release all resouces and memory
    for ( size_t i = 0; i < numDevices; i++ )
    {
        delete[] binaries[i];
    }

    return true;
}

bool OpenclDevice::initOpenclAttr( OpenCLEnv * env )
{
    if ( gpuEnv.mnIsUserCreated )
        return true;

    gpuEnv.mpContext = env->mpOclContext;
    gpuEnv.mpPlatformID = env->mpOclPlatformID;
    gpuEnv.mpDevID = env->mpOclDevsID;
    gpuEnv.mpCmdQueue = env->mpOclCmdQueue;

    gpuEnv.mnIsUserCreated = 1;

    return false;
}

void OpenclDevice::releaseOpenclEnv( GPUEnv *gpuInfo )
{
    if ( !bIsInited )
    {
        return;
    }

    if ( gpuEnv.mpCmdQueue )
    {
        clReleaseCommandQueue( gpuEnv.mpCmdQueue );
        gpuEnv.mpCmdQueue = NULL;
    }
    if ( gpuEnv.mpContext )
    {
        clReleaseContext( gpuEnv.mpContext );
        gpuEnv.mpContext = NULL;
    }
    bIsInited = false;
    gpuInfo->mnIsUserCreated = 0;
    free( gpuInfo->mpArryDevsID );

    return;
}

namespace {

bool buildProgram(const char* buildOption, GPUEnv* gpuInfo, int idx)
{
    cl_int clStatus;
    //char options[512];
    // create a cl program executable for all the devices specified
    if (!gpuInfo->mnIsUserCreated)
    {
        clStatus = clBuildProgram(gpuInfo->mpArryPrograms[idx], 1, gpuInfo->mpArryDevsID,
                       buildOption, NULL, NULL);
    }
    else
    {
        clStatus = clBuildProgram(gpuInfo->mpArryPrograms[idx], 1, &(gpuInfo->mpDevID),
                       buildOption, NULL, NULL);
    }

    if ( clStatus != CL_SUCCESS )
    {
        size_t length;
        if ( !gpuInfo->mnIsUserCreated )
        {
            clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpArryDevsID[0],
                           CL_PROGRAM_BUILD_LOG, 0, NULL, &length );
        }
        else
        {
            clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpDevID,
                           CL_PROGRAM_BUILD_LOG, 0, NULL, &length);
        }
        if ( clStatus != CL_SUCCESS )
        {
            return false;
        }

        boost::scoped_array<char> buildLog(new char[length]);
        if ( !gpuInfo->mnIsUserCreated )
        {
            clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpArryDevsID[0],
                           CL_PROGRAM_BUILD_LOG, length, buildLog.get(), &length );
        }
        else
        {
            clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpDevID,
                           CL_PROGRAM_BUILD_LOG, length, buildLog.get(), &length );
        }
        if ( clStatus != CL_SUCCESS )
        {
            return false;
        }

        OString aBuildLogFileURL = OpenclDevice::maCacheFolder + "kernel-build.log";
        osl::File aBuildLogFile(rtl::OStringToOUString(aBuildLogFileURL, RTL_TEXTENCODING_UTF8));
        osl::FileBase::RC status = aBuildLogFile.open(
                osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );

        if(status != osl::FileBase::E_None)
            return false;

        sal_uInt64 nBytesWritten = 0;
        aBuildLogFile.write( buildLog.get(), length, nBytesWritten );

        return false;
    }

    return true;
}

}

bool OpenclDevice::buildProgramFromBinary(const char* buildOption, GPUEnv* gpuInfo, const char* filename, int idx)
{
    size_t numDevices;
    cl_int clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES,
            0, NULL, &numDevices );
    numDevices /= sizeof(numDevices);
    CHECK_OPENCL( clStatus, "clGetContextInfo" );

    std::vector<boost::shared_ptr<osl::File> > aGeneratedFiles = binaryGenerated(
            filename, gpuInfo->mpContext );

    if (aGeneratedFiles.size() == numDevices)
    {
        boost::scoped_array<size_t> length(new size_t[numDevices]);
        boost::scoped_array<unsigned char*> pBinary(new unsigned char*[numDevices]);
        for(size_t i = 0; i < numDevices; ++i)
        {
            sal_uInt64 nSize;
            aGeneratedFiles[i]->getSize(nSize);
            unsigned char* binary = new unsigned char[nSize];
            sal_uInt64 nBytesRead;
            aGeneratedFiles[i]->read(binary, nSize, nBytesRead);
            if(nSize != nBytesRead)
                assert(false);

            length[i] = nBytesRead;

            pBinary[i] = binary;
        }

        // grab the handles to all of the devices in the context.
        boost::scoped_array<cl_device_id> mpArryDevsID(new cl_device_id[numDevices]);
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES,
                       sizeof( cl_device_id ) * numDevices, mpArryDevsID.get(), NULL );

        if(clStatus != CL_SUCCESS)
        {
            for(size_t i = 0; i < numDevices; ++i)
            {
                delete[] pBinary[i];
            }
            return false;
        }

        cl_int binary_status;

        gpuInfo->mpArryPrograms[idx] = clCreateProgramWithBinary( gpuInfo->mpContext,numDevices,
                                           mpArryDevsID.get(), length.get(), (const unsigned char**) pBinary.get(),
                                           &binary_status, &clStatus );
        if(clStatus != CL_SUCCESS)
        {
            // something went wrong, fall back to compiling from source
            return false;
        }
        for(size_t i = 0; i < numDevices; ++i)
        {
            delete[] pBinary[i];
        }
    }

    if ( !gpuInfo->mpArryPrograms[idx] )
    {
        return false;
    }
    return buildProgram(buildOption, gpuInfo, idx);
}

bool OpenclDevice::initOpenclRunEnv( int argc )
{
    if ( MAX_CLKERNEL_NUM <= 0 )
    {
        return true;
    }
    if ( ( argc > MAX_CLFILE_NUM ) || ( argc < 0 ) )
        return true;

    if ( !bIsInited )
    {
        registOpenclKernel();
        //initialize devices, context, command_queue
        bool status = initOpenclRunEnv( &gpuEnv );
        if ( status )
        {
            return true;
        }
        //initialize program, kernelName, kernelCount
        if( getenv( "SC_FLOAT" ) )
        {
            gpuEnv.mnKhrFp64Flag = false;
            gpuEnv.mnAmdFp64Flag = false;
        }
        if( gpuEnv.mnKhrFp64Flag )
        {
            SAL_INFO("sc.opencl", "Use Khr double");
        }
        else if( gpuEnv.mnAmdFp64Flag )
        {
            SAL_INFO("sc.opencl", "Use AMD double type");
        }
        else
        {
            SAL_INFO("sc.opencl", "USE float type");
        }
        bIsInited = true;
    }
    return false;
}

namespace {

void checkDeviceForDoubleSupport(cl_device_id deviceId, bool& bKhrFp64, bool& bAmdFp64)
{
    bKhrFp64 = false;
    bAmdFp64 = false;

    // Check device extensions for double type
    size_t aDevExtInfoSize = 0;

    cl_uint clStatus = clGetDeviceInfo( deviceId, CL_DEVICE_EXTENSIONS, 0, NULL, &aDevExtInfoSize );
    if( clStatus != CL_SUCCESS )
        return;

    boost::scoped_array<char> pExtInfo(new char[aDevExtInfoSize]);

    clStatus = clGetDeviceInfo( deviceId, CL_DEVICE_EXTENSIONS,
                   sizeof(char) * aDevExtInfoSize, pExtInfo.get(), NULL);

    if( clStatus != CL_SUCCESS )
        return;

    if ( strstr( pExtInfo.get(), "cl_khr_fp64" ) )
    {
        bKhrFp64 = true;
    }
    else
    {
        // Check if cl_amd_fp64 extension is supported
        if ( strstr( pExtInfo.get(), "cl_amd_fp64" ) )
            bAmdFp64 = true;
    }
}

}

bool OpenclDevice::initOpenclRunEnv( GPUEnv *gpuInfo )
{
    size_t length;
    cl_int clStatus;
    cl_uint numPlatforms, numDevices;
    cl_platform_id *platforms;

    // Have a look at the available platforms.

    if ( !gpuInfo->mnIsUserCreated )
    {
        clStatus = clGetPlatformIDs( 0, NULL, &numPlatforms );
        CHECK_OPENCL(clStatus, "clGetPlatformIDs");
        gpuInfo->mpPlatformID = NULL;

        if ( 0 < numPlatforms )
        {
            char platformName[256];
            platforms = (cl_platform_id*) malloc( numPlatforms * sizeof( cl_platform_id ) );
            if (!platforms)
            {
                return true;
            }
            clStatus = clGetPlatformIDs( numPlatforms, platforms, NULL );
            CHECK_OPENCL(clStatus, "clGetPlatformIDs");

            for ( unsigned int i = 0; i < numPlatforms; i++ )
            {
                clStatus = clGetPlatformInfo( platforms[i], CL_PLATFORM_VENDOR,
                    sizeof( platformName ), platformName, NULL );

                if ( clStatus != CL_SUCCESS )
                {
                    break;
                }
                gpuInfo->mpPlatformID = platforms[i];

                //if (!strcmp(platformName, "Intel(R) Coporation"))
                //if( !strcmp( platformName, "Advanced Micro Devices, Inc." ))
                {
                    gpuInfo->mpPlatformID = platforms[i];
                    if ( getenv("SC_OPENCLCPU") )
                    {
                        clStatus = clGetDeviceIDs(gpuInfo->mpPlatformID, // platform
                                                  CL_DEVICE_TYPE_CPU,    // device_type for CPU device
                                                  0,                     // num_entries
                                                  NULL,                  // devices
                                                  &numDevices);
                    }
                    else
                    {
                          clStatus = clGetDeviceIDs(gpuInfo->mpPlatformID, // platform
                                                  CL_DEVICE_TYPE_GPU,      // device_type for GPU device
                                                  0,                       // num_entries
                                                  NULL,                    // devices
                                                  &numDevices);
                    }
                    if ( clStatus != CL_SUCCESS )
                        continue;

                    if ( numDevices )
                        break;
                }
            }
            free( platforms );
            if ( clStatus != CL_SUCCESS )
                return true;
        }
        if ( NULL == gpuInfo->mpPlatformID )
            return true;

        // Use available platform.
        cl_context_properties cps[3];
        cps[0] = CL_CONTEXT_PLATFORM;
        cps[1] = (cl_context_properties) gpuInfo->mpPlatformID;
        cps[2] = 0;
        // Set device type for OpenCL
        if ( getenv("SC_OPENCLCPU") )
        {
            gpuInfo->mDevType = CL_DEVICE_TYPE_CPU;
        }
        else
        {
            gpuInfo->mDevType = CL_DEVICE_TYPE_GPU;
        }
        gpuInfo->mpContext = clCreateContextFromType( cps, gpuInfo->mDevType, NULL, NULL, &clStatus );

        if ( ( gpuInfo->mpContext == (cl_context) NULL) || ( clStatus != CL_SUCCESS ) )
        {
            gpuInfo->mDevType = CL_DEVICE_TYPE_CPU;
            gpuInfo->mpContext = clCreateContextFromType( cps, gpuInfo->mDevType, NULL, NULL, &clStatus );
        }
        if ( ( gpuInfo->mpContext == (cl_context) NULL) || ( clStatus != CL_SUCCESS ) )
        {
            gpuInfo->mDevType = CL_DEVICE_TYPE_DEFAULT;
            gpuInfo->mpContext = clCreateContextFromType( cps, gpuInfo->mDevType, NULL, NULL, &clStatus );
        }
        if ( ( gpuInfo->mpContext == (cl_context) NULL) || ( clStatus != CL_SUCCESS ) )
            return true;
        // Detect OpenCL devices.
        // First, get the size of device list data
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES, 0, NULL, &length );
        if ( ( clStatus != CL_SUCCESS ) || ( length == 0 ) )
            return true;
        // Now allocate memory for device list based on the size we got earlier
        gpuInfo->mpArryDevsID = (cl_device_id*) malloc( length );
        if ( gpuInfo->mpArryDevsID == (cl_device_id*) NULL )
            return true;
        // Now, get the device list data
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES, length,
                       gpuInfo->mpArryDevsID, NULL );
        CHECK_OPENCL(clStatus, "clGetContextInfo");

        // Create OpenCL command queue.
        gpuInfo->mpCmdQueue = clCreateCommandQueue( gpuInfo->mpContext, gpuInfo->mpArryDevsID[0], 0, &clStatus );

        CHECK_OPENCL(clStatus, "clCreateCommandQueue");
    }
    bool bKhrFp64 = false;
    bool bAmdFp64 = false;

    checkDeviceForDoubleSupport(gpuInfo->mpArryDevsID[0], bKhrFp64, bAmdFp64);

    gpuInfo->mnKhrFp64Flag = bKhrFp64;
    gpuInfo->mnAmdFp64Flag = bAmdFp64;

    return false;
}

namespace {

// based on crashes and hanging during kernel compilation
bool checkForKnownBadCompilers(const OpenclDeviceInfo& rInfo)
{

    struct {
        const char* pVendorName; const char* pDriverVersion;
    } aBadOpenCLCompilers[] = {
        { "Intel(R) Corporation", "9.17.10.2884" }
    };

    for(size_t i = 0; i < SAL_N_ELEMENTS(aBadOpenCLCompilers); ++i)
    {
        if(rInfo.maVendor == OUString::createFromAscii(aBadOpenCLCompilers[i].pVendorName) &&
                rInfo.maDriver == OUString::createFromAscii(aBadOpenCLCompilers[i].pDriverVersion))
            return true;
    }

    return false;
}

void createDeviceInfo(cl_device_id aDeviceId, OpenclPlatformInfo& rPlatformInfo)
{
    OpenclDeviceInfo aDeviceInfo;
    aDeviceInfo.device = aDeviceId;

    char pName[DEVICE_NAME_LENGTH];
    cl_int nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_NAME, DEVICE_NAME_LENGTH, pName, NULL);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.maName = OUString::createFromAscii(pName);

    char pVendor[DEVICE_NAME_LENGTH];
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_VENDOR, DEVICE_NAME_LENGTH, pVendor, NULL);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.maVendor = OUString::createFromAscii(pVendor);

    cl_ulong nMemSize;
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(nMemSize), &nMemSize, NULL);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.mnMemory = nMemSize;

    cl_uint nClockFrequency;
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(nClockFrequency), &nClockFrequency, NULL);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.mnFrequency = nClockFrequency;

    cl_uint nComputeUnits;
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(nComputeUnits), &nComputeUnits, NULL);
    if(nState != CL_SUCCESS)
        return;

    char pDriver[DEVICE_NAME_LENGTH];
    nState = clGetDeviceInfo(aDeviceId, CL_DRIVER_VERSION, DEVICE_NAME_LENGTH, pDriver, NULL);

    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.maDriver = OUString::createFromAscii(pDriver);

    bool bKhrFp64 = false;
    bool bAmdFp64 = false;
    checkDeviceForDoubleSupport(aDeviceId, bKhrFp64, bAmdFp64);

    // only list devices that support double
    if(!bKhrFp64 && !bAmdFp64)
        return;

    aDeviceInfo.mnComputeUnits = nComputeUnits;

    if(!checkForKnownBadCompilers(aDeviceInfo))
        rPlatformInfo.maDevices.push_back(aDeviceInfo);
}

bool createPlatformInfo(cl_platform_id nPlatformId, OpenclPlatformInfo& rPlatformInfo)
{
    rPlatformInfo.platform = nPlatformId;
    char pName[64];
    cl_int nState = clGetPlatformInfo(nPlatformId, CL_PLATFORM_NAME, 64,
             pName, NULL);
    if(nState != CL_SUCCESS)
        return false;
    rPlatformInfo.maName = OUString::createFromAscii(pName);

    char pVendor[64];
    nState = clGetPlatformInfo(nPlatformId, CL_PLATFORM_VENDOR, 64,
             pVendor, NULL);
    if(nState != CL_SUCCESS)
        return false;

    rPlatformInfo.maVendor = OUString::createFromAscii(pName);

    cl_uint nDevices;
    nState = clGetDeviceIDs(nPlatformId, CL_DEVICE_TYPE_ALL, 0, NULL, &nDevices);
    if(nState != CL_SUCCESS)
        return false;

    // memory leak that does not matter
    // memory is stored in static variable that lives through the whole program
    cl_device_id* pDevices = new cl_device_id[nDevices];
    nState = clGetDeviceIDs(nPlatformId, CL_DEVICE_TYPE_ALL, nDevices, pDevices, NULL);
    if(nState != CL_SUCCESS)
        return false;

    for(size_t i = 0; i < nDevices; ++i)
    {
        createDeviceInfo(pDevices[i], rPlatformInfo);
    }

    return true;
}

}

size_t getOpenCLPlatformCount()
{
    int status = clewInit(OPENCL_DLL_NAME);
    if (status < 0)
        return 0;

    cl_uint nPlatforms;
    cl_int nState = clGetPlatformIDs(0, NULL, &nPlatforms);

    if (nState != CL_SUCCESS)
        return 0;

    return nPlatforms;
}

const std::vector<OpenclPlatformInfo>& fillOpenCLInfo()
{
    static std::vector<OpenclPlatformInfo> aPlatforms;
    if(!aPlatforms.empty())
        return aPlatforms;

    int status = clewInit(OPENCL_DLL_NAME);
    if (status < 0)
        return aPlatforms;

    cl_uint nPlatforms;
    cl_int nState = clGetPlatformIDs(0, NULL, &nPlatforms);

    if(nState != CL_SUCCESS)
        return aPlatforms;

    // memory leak that does not matter,
    // memory is stored in static instance aPlatforms
    cl_platform_id* pPlatforms = new cl_platform_id[nPlatforms];
    nState = clGetPlatformIDs(nPlatforms, pPlatforms, NULL);

    if(nState != CL_SUCCESS)
        return aPlatforms;

    for(size_t i = 0; i < nPlatforms; ++i)
    {
        OpenclPlatformInfo aPlatformInfo;
        if(createPlatformInfo(pPlatforms[i], aPlatformInfo))
            aPlatforms.push_back(aPlatformInfo);
    }

    return aPlatforms;
}

namespace {

cl_device_id findDeviceIdByDeviceString(const OUString& rString, const std::vector<OpenclPlatformInfo>& rPlatforms)
{
    std::vector<OpenclPlatformInfo>::const_iterator it = rPlatforms.begin(), itEnd = rPlatforms.end();
    for(; it != itEnd; ++it)
    {
        std::vector<OpenclDeviceInfo>::const_iterator itr = it->maDevices.begin(), itrEnd = it->maDevices.end();
        for(; itr != itrEnd; ++itr)
        {
            OUString aDeviceId = it->maVendor + " " + itr->maName;
            if(rString == aDeviceId)
            {
                return static_cast<cl_device_id>(itr->device);
            }
        }
    }

    return NULL;
}

void findDeviceInfoFromDeviceId(cl_device_id aDeviceId, size_t& rDeviceId, size_t& rPlatformId)
{
    cl_platform_id platformId;
    cl_int nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_PLATFORM,
            sizeof(platformId), &platformId, NULL);

    if(nState != CL_SUCCESS)
        return;

    const std::vector<OpenclPlatformInfo>& rPlatforms = fillOpenCLInfo();
    for(size_t i = 0; i < rPlatforms.size(); ++i)
    {
        cl_platform_id platId = static_cast<cl_platform_id>(rPlatforms[i].platform);
        if(platId != platformId)
            continue;

        for(size_t j = 0; j < rPlatforms[i].maDevices.size(); ++j)
        {
            cl_device_id id = static_cast<cl_device_id>(rPlatforms[i].maDevices[j].device);
            if(id == aDeviceId)
            {
                rDeviceId = j;
                rPlatformId = i;
                return;
            }
        }
    }
}

}

bool switchOpenclDevice(const OUString* pDevice, bool bAutoSelect, bool bForceEvaluation)
{
    if(fillOpenCLInfo().empty())
        return false;

    cl_device_id pDeviceId = NULL;
    if(pDevice)
        pDeviceId = findDeviceIdByDeviceString(*pDevice, fillOpenCLInfo());

    if(!pDeviceId || bAutoSelect)
    {
        int status = clewInit(OPENCL_DLL_NAME);
        if (status < 0)
            return false;

        OUString url("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/cache/");
        rtl::Bootstrap::expandMacros(url);
        OUString path;
        osl::FileBase::getSystemPathFromFileURL(url,path);
        OString dsFileName = rtl::OUStringToOString(path, RTL_TEXTENCODING_UTF8);
        ds_device pSelectedDevice = sc::OpenCLDevice::getDeviceSelection(dsFileName.getStr(), bForceEvaluation);
        pDeviceId = pSelectedDevice.oclDeviceID;

    }

    if(OpenclDevice::gpuEnv.mpDevID == pDeviceId)
    {
        // we don't need to change anything
        // still the same device
        return pDeviceId != NULL;
    }

    cl_platform_id platformId;
    cl_int nState = clGetDeviceInfo(pDeviceId, CL_DEVICE_PLATFORM,
            sizeof(platformId), &platformId, NULL);

    cl_context_properties cps[3];
    cps[0] = CL_CONTEXT_PLATFORM;
    cps[1] = (cl_context_properties) platformId;
    cps[2] = 0;
    cl_context context = clCreateContext( cps, 1, &pDeviceId, NULL, NULL, &nState );

    if(nState != CL_SUCCESS || context == NULL)
    {
        if(context != NULL)
            clReleaseContext(context);

        SAL_WARN("sc", "failed to set/switch opencl device");
        return false;
    }

    cl_command_queue command_queue = clCreateCommandQueue(
            context, pDeviceId, 0, &nState);

    if(command_queue == NULL || nState != CL_SUCCESS)
    {
        if(command_queue != NULL)
            clReleaseCommandQueue(command_queue);

        clReleaseContext(context);
        SAL_WARN("sc", "failed to set/switch opencl device");
        return false;
    }

    OpenclDevice::releaseOpenclEnv(&OpenclDevice::gpuEnv);
    OpenCLEnv env;
    env.mpOclPlatformID = platformId;
    env.mpOclContext = context;
    env.mpOclDevsID = pDeviceId;
    env.mpOclCmdQueue = command_queue;
    OpenclDevice::initOpenclAttr(&env);

    // why do we need this at all?
    OpenclDevice::gpuEnv.mpArryDevsID = (cl_device_id*) malloc( sizeof(cl_device_id) );
    OpenclDevice::gpuEnv.mpArryDevsID[0] = pDeviceId;
    return !OpenclDevice::initOpenclRunEnv(0);
}

void getOpenCLDeviceInfo(size_t& rDeviceId, size_t& rPlatformId)
{
    int status = clewInit(OPENCL_DLL_NAME);
    if (status < 0)
        return;

    cl_device_id id = OpenclDevice::gpuEnv.mpDevID;
    findDeviceInfoFromDeviceId(id, rDeviceId, rPlatformId);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
