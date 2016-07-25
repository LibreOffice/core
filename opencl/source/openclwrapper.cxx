/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_folders.h>

#include "opencl_device.hxx"

#include <comphelper/string.hxx>
#include <opencl/openclconfig.hxx>
#include <opencl/openclwrapper.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/digest.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/log.hxx>
#include <opencl/OpenCLZone.hxx>

#include <memory>
#include <unicode/regex.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmath>

#ifdef _WIN32
#include <prewin.h>
#include <postwin.h>
#define OPENCL_DLL_NAME "OpenCL.dll"
#elif defined(MACOSX)
#define OPENCL_DLL_NAME nullptr
#else
#define OPENCL_DLL_NAME "libOpenCL.so.1"
#endif

#ifdef _WIN32_WINNT_WINBLUE
#include <VersionHelpers.h>
#endif

#define DEVICE_NAME_LENGTH 1024
#define DRIVER_VERSION_LENGTH 1024
#define PLATFORM_VERSION_LENGTH 1024

#define CHECK_OPENCL(status,name) \
if( status != CL_SUCCESS )  \
{ \
    SAL_WARN( "opencl", "OpenCL error code " << status << " at " SAL_DETAIL_WHERE " from " name ); \
    return false; \
}

using namespace std;

namespace opencl {

GPUEnv gpuEnv;
sal_uInt64 kernelFailures = 0;

namespace {

bool bIsInited = false;

OString generateMD5(const void* pData, size_t length)
{
    sal_uInt8 pBuffer[RTL_DIGEST_LENGTH_MD5];
    rtlDigestError aError = rtl_digest_MD5(pData, length,
            pBuffer, RTL_DIGEST_LENGTH_MD5);
    SAL_WARN_IF(aError != rtl_Digest_E_None, "opencl", "md5 generation failed");

    OStringBuffer aBuffer;
    const char* pString = "0123456789ABCDEF";
    for(sal_uInt8 val : pBuffer)
    {
        aBuffer.append(pString[val/16]);
        aBuffer.append(pString[val%16]);
    }
    return aBuffer.makeStringAndClear();
}

OString getCacheFolder()
{
    static OString aCacheFolder;

    if (aCacheFolder.isEmpty())
    {
        OUString url("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/cache/");
        rtl::Bootstrap::expandMacros(url);

        osl::Directory::create(url);

        aCacheFolder = rtl::OUStringToOString(url, RTL_TEXTENCODING_UTF8);
    }
    return aCacheFolder;
}

}

void setKernelEnv( KernelEnv *envInfo )
{
    envInfo->mpkContext = gpuEnv.mpContext;
    envInfo->mpkProgram = gpuEnv.mpArryPrograms[0];

    assert(gpuEnv.mnCmdQueuePos < OPENCL_CMDQUEUE_SIZE);
    envInfo->mpkCmdQueue = gpuEnv.mpCmdQueue[gpuEnv.mnCmdQueuePos];
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
            sizeof(deviceName), deviceName, nullptr);

    char driverVersion[DRIVER_VERSION_LENGTH] = {0};
    clGetDeviceInfo(deviceId, CL_DRIVER_VERSION,
            sizeof(driverVersion), driverVersion, nullptr);

    cl_platform_id platformId;
    clGetDeviceInfo(deviceId, CL_DEVICE_PLATFORM,
            sizeof(platformId), &platformId, nullptr);

    char platformVersion[PLATFORM_VERSION_LENGTH] = {0};
    clGetPlatformInfo(platformId, CL_PLATFORM_VERSION, sizeof(platformVersion),
            platformVersion, nullptr);

    // create hash for deviceName + driver version + platform version
    OString aString = OString(deviceName) + driverVersion + platformVersion;
    OString aHash = generateMD5(aString.getStr(), aString.getLength());

    return getCacheFolder() + fileName + "-" + aHash + ".bin";
}

std::vector<std::shared_ptr<osl::File> > binaryGenerated( const char * clFileName, cl_context context )
{
    size_t numDevices=0;

    std::vector<std::shared_ptr<osl::File> > aGeneratedFiles;
    cl_int clStatus = clGetContextInfo( context, CL_CONTEXT_DEVICES,
            0, nullptr, &numDevices );
    numDevices /= sizeof(numDevices);

    if(clStatus != CL_SUCCESS)
        return aGeneratedFiles;

    assert(numDevices == 1);

    // grab the handle to the device in the context.
    cl_device_id pDevID;
    clStatus = clGetContextInfo( context, CL_CONTEXT_DEVICES,
            sizeof( cl_device_id ), &pDevID, nullptr );

    if(clStatus != CL_SUCCESS)
        return aGeneratedFiles;

    assert(pDevID == gpuEnv.mpDevID);

    OString fileName = createFileName(gpuEnv.mpDevID, clFileName);
    osl::File* pNewFile = new osl::File(rtl::OStringToOUString(fileName, RTL_TEXTENCODING_UTF8));
    if(pNewFile->open(osl_File_OpenFlag_Read) == osl::FileBase::E_None)
    {
        aGeneratedFiles.push_back(std::shared_ptr<osl::File>(pNewFile));
        SAL_INFO("opencl.file", "Opening binary file '" << fileName << "' for reading: success");
    }
    else
    {
        SAL_INFO("opencl.file", "Opening binary file '" << fileName << "' for reading: FAIL");
        delete pNewFile;
    }

    return aGeneratedFiles;
}

bool writeBinaryToFile( const OString& rFileName, const char* binary, size_t numBytes )
{
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

}

bool generatBinFromKernelSource( cl_program program, const char * clFileName )
{
    cl_uint numDevices;

    cl_int clStatus = clGetProgramInfo( program, CL_PROGRAM_NUM_DEVICES,
                   sizeof(numDevices), &numDevices, nullptr );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    assert(numDevices == 1);

    cl_device_id pDevID;
    /* grab the handle to the device in the program. */
    clStatus = clGetProgramInfo( program, CL_PROGRAM_DEVICES,
                   sizeof(cl_device_id), &pDevID, nullptr );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    /* figure out the size of the binary. */
    size_t binarySize;

    clStatus = clGetProgramInfo( program, CL_PROGRAM_BINARY_SIZES,
                   sizeof(size_t), &binarySize, nullptr );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    /* copy over the generated binary. */
    if ( binarySize != 0 )
    {
        std::unique_ptr<char[]> binary(new char[binarySize]);
        clStatus = clGetProgramInfo( program, CL_PROGRAM_BINARIES,
                                     sizeof(char *), &binary, nullptr );
        CHECK_OPENCL(clStatus,"clGetProgramInfo");

        OString fileName = createFileName(pDevID, clFileName);
        if ( !writeBinaryToFile( fileName,
                                 binary.get(), binarySize ) )
            SAL_INFO("opencl.file", "Writing binary file '" << fileName << "': FAIL");
        else
            SAL_INFO("opencl.file", "Writing binary file '" << fileName << "': success");
    }
    return true;
}

namespace {

struct OpenCLEnv
{
    cl_platform_id mpOclPlatformID;
    cl_context mpOclContext;
    cl_device_id mpOclDevsID;
    cl_command_queue mpOclCmdQueue[OPENCL_CMDQUEUE_SIZE];
};

bool initOpenCLAttr( OpenCLEnv * env )
{
    if ( gpuEnv.mnIsUserCreated )
        return true;

    gpuEnv.mpContext = env->mpOclContext;
    gpuEnv.mpPlatformID = env->mpOclPlatformID;
    gpuEnv.mpDevID = env->mpOclDevsID;

    gpuEnv.mnIsUserCreated = 1;

    for (int i = 0; i < OPENCL_CMDQUEUE_SIZE; ++i)
        gpuEnv.mpCmdQueue[i] = env->mpOclCmdQueue[i];

    gpuEnv.mnCmdQueuePos = 0; // default to 0.

    return false;
}

}

void releaseOpenCLEnv( GPUEnv *gpuInfo )
{
    OpenCLZone zone;

    if ( !bIsInited )
    {
        return;
    }

    for (_cl_command_queue* & i : gpuEnv.mpCmdQueue)
    {
        if (i)
        {
            clReleaseCommandQueue(i);
            i = nullptr;
        }
    }
    gpuEnv.mnCmdQueuePos = 0;

    if ( gpuEnv.mpContext )
    {
        clReleaseContext( gpuEnv.mpContext );
        gpuEnv.mpContext = nullptr;
    }
    bIsInited = false;
    gpuInfo->mnIsUserCreated = 0;

    return;
}

namespace {

bool buildProgram(const char* buildOption, GPUEnv* gpuInfo, int idx)
{
    cl_int clStatus;
    //char options[512];
    // create a cl program executable for all the devices specified
    clStatus = clBuildProgram(gpuInfo->mpArryPrograms[idx], 1, &gpuInfo->mpDevID,
                              buildOption, nullptr, nullptr);

    if ( clStatus != CL_SUCCESS )
    {
        size_t length;
        clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpDevID,
                                          CL_PROGRAM_BUILD_LOG, 0, nullptr, &length);
        if ( clStatus != CL_SUCCESS )
        {
            return false;
        }

        std::unique_ptr<char[]> buildLog(new char[length]);
        clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpDevID,
                                          CL_PROGRAM_BUILD_LOG, length, buildLog.get(), &length );
        if ( clStatus != CL_SUCCESS )
        {
            return false;
        }

        OString aBuildLogFileURL = getCacheFolder() + "kernel-build.log";
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

bool buildProgramFromBinary(const char* buildOption, GPUEnv* gpuInfo, const char* filename, int idx)
{
    size_t numDevices;
    cl_int clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES,
            0, nullptr, &numDevices );
    numDevices /= sizeof(numDevices);
    CHECK_OPENCL( clStatus, "clGetContextInfo" );

    std::vector<std::shared_ptr<osl::File> > aGeneratedFiles = binaryGenerated(
            filename, gpuInfo->mpContext );

    if (aGeneratedFiles.size() == numDevices)
    {
        std::unique_ptr<size_t[]> length(new size_t[numDevices]);
        std::unique_ptr<unsigned char*[]> pBinary(new unsigned char*[numDevices]);
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
        std::unique_ptr<cl_device_id[]> pArryDevsID(new cl_device_id[numDevices]);
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES,
                       sizeof( cl_device_id ) * numDevices, pArryDevsID.get(), nullptr );

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
                                           pArryDevsID.get(), length.get(), const_cast<const unsigned char**>(pBinary.get()),
                                           &binary_status, &clStatus );
        if(clStatus != CL_SUCCESS)
        {
            // something went wrong, fall back to compiling from source
            return false;
        }
        SAL_INFO("opencl", "Created program " << gpuInfo->mpArryPrograms[idx] << " from binary");
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

namespace {

void checkDeviceForDoubleSupport(cl_device_id deviceId, bool& bKhrFp64, bool& bAmdFp64)
{
    OpenCLZone zone;

    bKhrFp64 = false;
    bAmdFp64 = false;

    // Check device extensions for double type
    size_t aDevExtInfoSize = 0;

    cl_uint clStatus = clGetDeviceInfo( deviceId, CL_DEVICE_EXTENSIONS, 0, nullptr, &aDevExtInfoSize );
    if( clStatus != CL_SUCCESS )
        return;

    std::unique_ptr<char[]> pExtInfo(new char[aDevExtInfoSize]);

    clStatus = clGetDeviceInfo( deviceId, CL_DEVICE_EXTENSIONS,
                   sizeof(char) * aDevExtInfoSize, pExtInfo.get(), nullptr);

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

bool initOpenCLRunEnv( GPUEnv *gpuInfo )
{
    OpenCLZone zone;
    cl_uint nPreferredVectorWidthFloat;
    char pName[64];

    bool bKhrFp64 = false;
    bool bAmdFp64 = false;

    checkDeviceForDoubleSupport(gpuInfo->mpDevID, bKhrFp64, bAmdFp64);

    gpuInfo->mnKhrFp64Flag = bKhrFp64;
    gpuInfo->mnAmdFp64Flag = bAmdFp64;

    gpuInfo->mbNeedsTDRAvoidance = false;

    clGetDeviceInfo(gpuInfo->mpDevID, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(cl_uint),
                    &nPreferredVectorWidthFloat, nullptr);
    clGetPlatformInfo(gpuInfo->mpPlatformID, CL_PLATFORM_NAME, 64,
             pName, nullptr);

    bool bIsNotWinOrIsWin8OrGreater = true;

// the Win32 SDK 8.1 deprecates GetVersionEx()
#ifdef _WIN32_WINNT_WINBLUE
    bIsNotWinOrIsWin8OrGreater = IsWindows8OrGreater();
#elif defined (_WIN32)
    OSVERSIONINFO aVersionInfo;
    memset( &aVersionInfo, 0, sizeof(aVersionInfo) );
    aVersionInfo.dwOSVersionInfoSize = sizeof( aVersionInfo );
    if (GetVersionEx( &aVersionInfo ))
    {
        // Windows 7 or lower?
        if (aVersionInfo.dwMajorVersion < 6 ||
           (aVersionInfo.dwMajorVersion == 6 && aVersionInfo.dwMinorVersion < 2))
            bIsNotWinOrIsWin8OrGreater = false;
    }
#endif

    // Heuristic: Certain old low-end OpenCL implementations don't
    // work for us with too large group lengths. Looking at the preferred
    // float vector width seems to be a way to detect these devices, except
    // the non-working NVIDIA cards on Windows older than version 8.
    gpuInfo->mbNeedsTDRAvoidance = ( nPreferredVectorWidthFloat == 4 ) ||
        ( !bIsNotWinOrIsWin8OrGreater &&
          OUString::createFromAscii(pName).indexOf("NVIDIA") > -1 );
    return false;
}

bool initOpenCLRunEnv( int argc )
{
    if ( ( argc > MAX_CLFILE_NUM ) || ( argc < 0 ) )
        return true;

    if ( !bIsInited )
    {
        if ( !gpuEnv.mnIsUserCreated )
            memset( &gpuEnv, 0, sizeof(gpuEnv) );

        //initialize devices, context, command_queue
        bool status = initOpenCLRunEnv( &gpuEnv );
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
            SAL_INFO("opencl", "Use Khr double");
        }
        else if( gpuEnv.mnAmdFp64Flag )
        {
            SAL_INFO("opencl", "Use AMD double type");
        }
        else
        {
            SAL_INFO("opencl", "USE float type");
        }
        bIsInited = true;
    }
    return false;
}

// based on crashes and hanging during kernel compilation
void createDeviceInfo(cl_device_id aDeviceId, OpenCLPlatformInfo& rPlatformInfo)
{
    OpenCLDeviceInfo aDeviceInfo;
    aDeviceInfo.device = aDeviceId;

    char pName[DEVICE_NAME_LENGTH];
    cl_int nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_NAME, DEVICE_NAME_LENGTH, pName, nullptr);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.maName = OUString::createFromAscii(pName);

    char pVendor[DEVICE_NAME_LENGTH];
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_VENDOR, DEVICE_NAME_LENGTH, pVendor, nullptr);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.maVendor = OUString::createFromAscii(pVendor);

    cl_ulong nMemSize;
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(nMemSize), &nMemSize, nullptr);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.mnMemory = nMemSize;

    cl_uint nClockFrequency;
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(nClockFrequency), &nClockFrequency, nullptr);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.mnFrequency = nClockFrequency;

    cl_uint nComputeUnits;
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(nComputeUnits), &nComputeUnits, nullptr);
    if(nState != CL_SUCCESS)
        return;

    char pDriver[DEVICE_NAME_LENGTH];
    nState = clGetDeviceInfo(aDeviceId, CL_DRIVER_VERSION, DEVICE_NAME_LENGTH, pDriver, nullptr);

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

    if(!OpenCLConfig::get().checkImplementation(rPlatformInfo, aDeviceInfo))
        rPlatformInfo.maDevices.push_back(aDeviceInfo);
}

bool createPlatformInfo(cl_platform_id nPlatformId, OpenCLPlatformInfo& rPlatformInfo)
{
    rPlatformInfo.platform = nPlatformId;
    char pName[64];
    cl_int nState = clGetPlatformInfo(nPlatformId, CL_PLATFORM_NAME, 64,
             pName, nullptr);
    if(nState != CL_SUCCESS)
        return false;
    rPlatformInfo.maName = OUString::createFromAscii(pName);

    char pVendor[64];
    nState = clGetPlatformInfo(nPlatformId, CL_PLATFORM_VENDOR, 64,
             pVendor, nullptr);
    if(nState != CL_SUCCESS)
        return false;

    rPlatformInfo.maVendor = OUString::createFromAscii(pVendor);

    cl_uint nDevices;
    nState = clGetDeviceIDs(nPlatformId, CL_DEVICE_TYPE_ALL, 0, nullptr, &nDevices);
    if(nState != CL_SUCCESS)
        return false;

    // memory leak that does not matter
    // memory is stored in static variable that lives through the whole program
    cl_device_id* pDevices = new cl_device_id[nDevices];
    nState = clGetDeviceIDs(nPlatformId, CL_DEVICE_TYPE_ALL, nDevices, pDevices, nullptr);
    if(nState != CL_SUCCESS)
        return false;

    for(size_t i = 0; i < nDevices; ++i)
    {
        createDeviceInfo(pDevices[i], rPlatformInfo);
    }

    return true;
}

}

const std::vector<OpenCLPlatformInfo>& fillOpenCLInfo()
{
    static std::vector<OpenCLPlatformInfo> aPlatforms;
    if(!aPlatforms.empty())
        return aPlatforms;

    int status = clewInit(OPENCL_DLL_NAME);
    if (status < 0)
        return aPlatforms;

    cl_uint nPlatforms;
    cl_int nState = clGetPlatformIDs(0, nullptr, &nPlatforms);

    if(nState != CL_SUCCESS)
        return aPlatforms;

    // memory leak that does not matter,
    // memory is stored in static instance aPlatforms
    cl_platform_id* pPlatforms = new cl_platform_id[nPlatforms];
    nState = clGetPlatformIDs(nPlatforms, pPlatforms, nullptr);

    if(nState != CL_SUCCESS)
        return aPlatforms;

    for(size_t i = 0; i < nPlatforms; ++i)
    {
        OpenCLPlatformInfo aPlatformInfo;
        if(createPlatformInfo(pPlatforms[i], aPlatformInfo))
            aPlatforms.push_back(aPlatformInfo);
    }

    return aPlatforms;
}

namespace {

cl_device_id findDeviceIdByDeviceString(const OUString& rString, const std::vector<OpenCLPlatformInfo>& rPlatforms)
{
    std::vector<OpenCLPlatformInfo>::const_iterator it = rPlatforms.begin(), itEnd = rPlatforms.end();
    for(; it != itEnd; ++it)
    {
        std::vector<OpenCLDeviceInfo>::const_iterator itr = it->maDevices.begin(), itrEnd = it->maDevices.end();
        for(; itr != itrEnd; ++itr)
        {
            OUString aDeviceId = it->maVendor + " " + itr->maName;
            if(rString == aDeviceId)
            {
                return static_cast<cl_device_id>(itr->device);
            }
        }
    }

    return nullptr;
}

void findDeviceInfoFromDeviceId(cl_device_id aDeviceId, size_t& rDeviceId, size_t& rPlatformId)
{
    cl_platform_id platformId;
    cl_int nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_PLATFORM,
            sizeof(platformId), &platformId, nullptr);

    if(nState != CL_SUCCESS)
        return;

    const std::vector<OpenCLPlatformInfo>& rPlatforms = fillOpenCLInfo();
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

bool switchOpenCLDevice(const OUString* pDevice, bool bAutoSelect, bool bForceEvaluation, OUString& rOutSelectedDeviceVersionIDString)
{
    if(fillOpenCLInfo().empty() || getenv("SAL_DISABLE_OPENCL"))
        return false;

    cl_device_id pDeviceId = nullptr;
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
        ds_device aSelectedDevice = getDeviceSelection(path, bForceEvaluation);
        if ( aSelectedDevice.eType != DeviceType::OpenCLDevice)
            return false;
        pDeviceId = aSelectedDevice.aDeviceID;
    }

    if(gpuEnv.mpDevID == pDeviceId)
    {
        // we don't need to change anything
        // still the same device
        return pDeviceId != nullptr;
    }

    cl_context context;
    cl_platform_id platformId;
    cl_command_queue command_queue[OPENCL_CMDQUEUE_SIZE];

    {
        OpenCLZone zone;
        cl_int nState = clGetDeviceInfo(pDeviceId, CL_DEVICE_PLATFORM,
                                        sizeof(platformId), &platformId, nullptr);

        cl_context_properties cps[3];
        cps[0] = CL_CONTEXT_PLATFORM;
        cps[1] = reinterpret_cast<cl_context_properties>(platformId);
        cps[2] = 0;
        context = clCreateContext( cps, 1, &pDeviceId, nullptr, nullptr, &nState );
        if (nState != CL_SUCCESS)
            SAL_WARN("opencl", "clCreateContext failed: " << errorString(nState));

        if(nState != CL_SUCCESS || context == nullptr)
        {
            if(context != nullptr)
                clReleaseContext(context);

            SAL_WARN("opencl", "failed to set/switch opencl device");
            return false;
        }
        SAL_INFO("opencl", "Created context " << context << " for platform " << platformId << ", device " << pDeviceId);

        for (int i = 0; i < OPENCL_CMDQUEUE_SIZE; ++i)
        {
            command_queue[i] = clCreateCommandQueue(
                context, pDeviceId, 0, &nState);
            if (nState != CL_SUCCESS)
                SAL_WARN("opencl", "clCreateCommandQueue failed: " << errorString(nState));

            if (command_queue[i] == nullptr || nState != CL_SUCCESS)
            {
                // Release all command queues created so far.
                for (int j = 0; j <= i; ++j)
                {
                    if (command_queue[j])
                    {
                        clReleaseCommandQueue(command_queue[j]);
                        command_queue[j] = nullptr;
                    }
                }

                clReleaseContext(context);
                SAL_WARN("opencl", "failed to set/switch opencl device");
                return false;
            }

            SAL_INFO("opencl", "Created command queue " << command_queue[i] << " for context " << context);
        }

        OString sDeviceID = getDeviceInfoString(pDeviceId, CL_DEVICE_VENDOR) + " " + getDeviceInfoString(pDeviceId, CL_DRIVER_VERSION);
        rOutSelectedDeviceVersionIDString = OStringToOUString(sDeviceID, RTL_TEXTENCODING_UTF8);
    }

    setOpenCLCmdQueuePosition(0); // Call this just to avoid the method being deleted from unused function deleter.

    releaseOpenCLEnv(&gpuEnv);
    OpenCLEnv env;
    env.mpOclPlatformID = platformId;
    env.mpOclContext = context;
    env.mpOclDevsID = pDeviceId;

    for (int i = 0; i < OPENCL_CMDQUEUE_SIZE; ++i)
        env.mpOclCmdQueue[i] = command_queue[i];

    initOpenCLAttr(&env);

    return !initOpenCLRunEnv(0);
}

void getOpenCLDeviceInfo(size_t& rDeviceId, size_t& rPlatformId)
{
    int status = clewInit(OPENCL_DLL_NAME);
    if (status < 0)
        return;

    cl_device_id id = gpuEnv.mpDevID;
    findDeviceInfoFromDeviceId(id, rDeviceId, rPlatformId);
}

void setOpenCLCmdQueuePosition( int nPos )
{
    if (nPos < 0 || nPos >= OPENCL_CMDQUEUE_SIZE)
        // Out of range. Ignore this.
        return;

    gpuEnv.mnCmdQueuePos = nPos;
}

const char* errorString(cl_int nError)
{
#define CASE(val) case CL_##val: return #val
    switch (nError)
    {
        CASE(SUCCESS);
        CASE(DEVICE_NOT_FOUND);
        CASE(DEVICE_NOT_AVAILABLE);
        CASE(COMPILER_NOT_AVAILABLE);
        CASE(MEM_OBJECT_ALLOCATION_FAILURE);
        CASE(OUT_OF_RESOURCES);
        CASE(OUT_OF_HOST_MEMORY);
        CASE(PROFILING_INFO_NOT_AVAILABLE);
        CASE(MEM_COPY_OVERLAP);
        CASE(IMAGE_FORMAT_MISMATCH);
        CASE(IMAGE_FORMAT_NOT_SUPPORTED);
        CASE(BUILD_PROGRAM_FAILURE);
        CASE(MAP_FAILURE);
        CASE(INVALID_VALUE);
        CASE(INVALID_DEVICE_TYPE);
        CASE(INVALID_PLATFORM);
        CASE(INVALID_DEVICE);
        CASE(INVALID_CONTEXT);
        CASE(INVALID_QUEUE_PROPERTIES);
        CASE(INVALID_COMMAND_QUEUE);
        CASE(INVALID_HOST_PTR);
        CASE(INVALID_MEM_OBJECT);
        CASE(INVALID_IMAGE_FORMAT_DESCRIPTOR);
        CASE(INVALID_IMAGE_SIZE);
        CASE(INVALID_SAMPLER);
        CASE(INVALID_BINARY);
        CASE(INVALID_BUILD_OPTIONS);
        CASE(INVALID_PROGRAM);
        CASE(INVALID_PROGRAM_EXECUTABLE);
        CASE(INVALID_KERNEL_NAME);
        CASE(INVALID_KERNEL_DEFINITION);
        CASE(INVALID_KERNEL);
        CASE(INVALID_ARG_INDEX);
        CASE(INVALID_ARG_VALUE);
        CASE(INVALID_ARG_SIZE);
        CASE(INVALID_KERNEL_ARGS);
        CASE(INVALID_WORK_DIMENSION);
        CASE(INVALID_WORK_GROUP_SIZE);
        CASE(INVALID_WORK_ITEM_SIZE);
        CASE(INVALID_GLOBAL_OFFSET);
        CASE(INVALID_EVENT_WAIT_LIST);
        CASE(INVALID_EVENT);
        CASE(INVALID_OPERATION);
        CASE(INVALID_GL_OBJECT);
        CASE(INVALID_BUFFER_SIZE);
        CASE(INVALID_MIP_LEVEL);
        CASE(INVALID_GLOBAL_WORK_SIZE);
        default:
            return "Unknown OpenCL error code";
    }
#undef CASE
}

bool GPUEnv::isOpenCLEnabled()
{
    return gpuEnv.mpDevID && gpuEnv.mpContext;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
