/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OPENCL_INC_OPENCL_DEVICE_SELECTION_H
#define INCLUDED_OPENCL_INC_OPENCL_DEVICE_SELECTION_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <clew/clew.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlstring.h>
#include <tools/stream.hxx>
#include <rtl/math.hxx>

#include <vector>

enum ds_status
{
    DS_SUCCESS = 0
    ,DS_INVALID_PROFILE = 1000
    ,DS_MEMORY_ERROR
    , DS_INVALID_PERF_EVALUATOR_TYPE
    , DS_INVALID_PERF_EVALUATOR
    , DS_PERF_EVALUATOR_ERROR
    , DS_FILE_ERROR
    , DS_UNKNOWN_DEVICE_TYPE
    , DS_PROFILE_FILE_ERROR
    , DS_SCORE_SERIALIZER_ERROR
    , DS_SCORE_DESERIALIZER_ERROR
};

// device type
enum class DeviceType
{
    None,
    NativeCPU,
    OpenCLDevice
};

struct ds_device
{
    DeviceType eType;
    cl_device_id aDeviceID;

    OString sPlatformName;
    OString sPlatformVendor;
    OString sPlatformVersion;
    OString sPlatformProfile;
    OString sPlatformExtensions;

    OString sDeviceName;
    OString sDeviceVendor;
    OString sDeviceVersion;
    OString sDriverVersion;
    OString sDeviceType;
    OString sDeviceExtensions;
    OString sDeviceOpenCLVersion;

    bool bDeviceAvailable;
    bool bDeviceCompilerAvailable;
    bool bDeviceLinkerAvailable;

    double fTime;   // small time means faster device
    bool   bErrors; // were there any opencl errors
};

struct ds_profile
{
    unsigned int  numDevices;
    std::vector<ds_device> devices;
    OString version;

    ds_profile(OString& inVersion)
        : numDevices(0)
        , version(inVersion)
    {}
};

inline OString getPlatformInfoString(cl_platform_id aPlatformId, cl_platform_info aPlatformInfo)
{
    std::vector<char> temporary(2048, 0);
    clGetPlatformInfo(aPlatformId, aPlatformInfo, temporary.size(), temporary.data(), nullptr);
    return OString(temporary.data());
}

inline OString getDeviceInfoString(cl_device_id aDeviceId, cl_device_info aDeviceInfo)
{
    std::vector<char> temporary(2048, 0);
    clGetDeviceInfo(aDeviceId, aDeviceInfo, temporary.size(), temporary.data(), nullptr);
    return OString(temporary.data());
}

inline OString getDeviceType(cl_device_id aDeviceId)
{
    OString sType = "";
    cl_device_type aDeviceType;
    clGetDeviceInfo(aDeviceId, CL_DEVICE_TYPE, sizeof(aDeviceType), &aDeviceType, nullptr);
    if (aDeviceType & CL_DEVICE_TYPE_CPU)
        sType += "cpu ";
    if (aDeviceType & CL_DEVICE_TYPE_GPU)
        sType += "gpu ";
    if (aDeviceType & CL_DEVICE_TYPE_ACCELERATOR)
        sType += "accelerator ";
    if (aDeviceType & CL_DEVICE_TYPE_CUSTOM)
        sType += "custom ";
    if (aDeviceType & CL_DEVICE_TYPE_DEFAULT)
        sType += "default ";
    return sType;
}

inline bool getDeviceInfoBool(cl_device_id aDeviceId, cl_device_info aDeviceInfo)
{
    cl_bool bCLBool;
    clGetDeviceInfo(aDeviceId, aDeviceInfo, sizeof(bCLBool), &bCLBool, nullptr);
    return bCLBool == CL_TRUE;
}

inline ds_status initDSProfile(std::unique_ptr<ds_profile>& rProfile, OString rVersion)
{
    int numDevices;
    cl_uint numPlatforms;
    std::vector<cl_platform_id> platforms;
    std::vector<cl_device_id> devices;

    unsigned int next;
    unsigned int i;

    rProfile = std::unique_ptr<ds_profile>(new ds_profile(rVersion));

    clGetPlatformIDs(0, NULL, &numPlatforms);
    if (numPlatforms != 0)
    {
        platforms.resize(numPlatforms);
        clGetPlatformIDs(numPlatforms, platforms.data(), NULL);
    }

    numDevices = 0;
    for (i = 0; i < (unsigned int)numPlatforms; i++)
    {
        cl_uint num = 0;
        cl_int err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num);
        if (err != CL_SUCCESS)
        {
            /* we want to catch at least the case when the call returns
             * CL_DEVICE_NOT_FOUND (i.e. no devices), because some platforms
             * don't set num to 0 in this case; but in fact this is a good
             * thing to do for _any_ error returned by the call
             */
            num = 0;
        }
        numDevices += num;
    }
    if (numDevices != 0)
    {
        devices.resize(numDevices);
    }

    rProfile->numDevices = numDevices + 1;     // +1 to numDevices to include the native CPU
    rProfile->devices.resize(rProfile->numDevices);

    next = 0;
    for (i = 0; i < (unsigned int)numPlatforms; i++)
    {
        cl_uint num = 0;
        unsigned j;

        OString sPlatformProfile = getPlatformInfoString(platforms[i], CL_PLATFORM_PROFILE);
        OString sPlatformVersion = getPlatformInfoString(platforms[i], CL_PLATFORM_VERSION);
        OString sPlatformName    = getPlatformInfoString(platforms[i], CL_PLATFORM_NAME);
        OString sPlatformVendor  = getPlatformInfoString(platforms[i], CL_PLATFORM_VENDOR);
        OString sPlatformExts    = getPlatformInfoString(platforms[i], CL_PLATFORM_EXTENSIONS);

        cl_int err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, numDevices, devices.data(), &num);
        if (err != CL_SUCCESS)
        {
            /* we want to catch at least the case when the call returns
             * CL_DEVICE_NOT_FOUND (i.e. no devices), because some platforms
             * don't set num to 0 in this case; but in fact this is a good
             * thing to do for _any_ error returned by the call
             */
            num = 0;
        }
        for (j = 0; j < num; j++, next++)
        {
            cl_device_id aDeviceID = devices[j];

            ds_device& rDevice = rProfile->devices[next];
            rDevice.eType = DeviceType::OpenCLDevice;
            rDevice.aDeviceID = aDeviceID;

            rDevice.sPlatformName         = sPlatformName;
            rDevice.sPlatformVendor       = sPlatformVendor;
            rDevice.sPlatformVersion      = sPlatformVersion;
            rDevice.sPlatformProfile      = sPlatformProfile;
            rDevice.sPlatformExtensions   = sPlatformExts;

            rDevice.sDeviceName       = getDeviceInfoString(aDeviceID, CL_DEVICE_NAME);
            rDevice.sDeviceVendor     = getDeviceInfoString(aDeviceID, CL_DEVICE_VENDOR);
            rDevice.sDeviceVersion    = getDeviceInfoString(aDeviceID, CL_DEVICE_VERSION);
            rDevice.sDriverVersion    = getDeviceInfoString(aDeviceID, CL_DRIVER_VERSION);
            rDevice.sDeviceType       = getDeviceType(aDeviceID);
            rDevice.sDeviceExtensions = getDeviceInfoString(aDeviceID, CL_DEVICE_EXTENSIONS);
            rDevice.sDeviceOpenCLVersion = getDeviceInfoString(aDeviceID, CL_DEVICE_OPENCL_C_VERSION);

            rDevice.bDeviceAvailable         = getDeviceInfoBool(aDeviceID, CL_DEVICE_AVAILABLE);
            rDevice.bDeviceCompilerAvailable = getDeviceInfoBool(aDeviceID, CL_DEVICE_COMPILER_AVAILABLE);
            rDevice.bDeviceLinkerAvailable   = getDeviceInfoBool(aDeviceID, CL_DEVICE_LINKER_AVAILABLE);
        }
    }
    rProfile->devices[next].eType = DeviceType::NativeCPU;

    return DS_SUCCESS;
}

// Pointer to a function that calculates the score of a device (ex: device->score)
// update the data size of score. The encoding and the format of the score data
// is implementation defined. The function should return DS_SUCCESS if there's no error to be reported.
typedef ds_status(* ds_perf_evaluator)(ds_device* device, void* data);

typedef enum {
    DS_EVALUATE_ALL
    , DS_EVALUATE_NEW_ONLY
} ds_evaluation_type;

inline ds_status profileDevices(ds_profile* profile, const ds_evaluation_type type,
                                ds_perf_evaluator evaluator, void* evaluatorData, unsigned int* numUpdates)
{
    ds_status status = DS_SUCCESS;
    unsigned int i;
    unsigned int updates = 0;

    if (profile == NULL)
    {
        return DS_INVALID_PROFILE;
    }
    if (evaluator == NULL)
    {
        return DS_INVALID_PERF_EVALUATOR;
    }

    for (i = 0; i < profile->numDevices; i++)
    {
        ds_status evaluatorStatus;

        switch (type)
        {
            case DS_EVALUATE_NEW_ONLY:
                //if (profile->devices[i].score != NULL) break;
                //  else fall through
            case DS_EVALUATE_ALL:
                evaluatorStatus = evaluator(profile->devices.data() + i, evaluatorData);
                if (evaluatorStatus != DS_SUCCESS)
                {
                    status = evaluatorStatus;
                    return status;
                }
                updates++;
                break;
            default:
                return DS_INVALID_PERF_EVALUATOR_TYPE;
                break;
        };
    }
    if (numUpdates) *numUpdates = updates;
    return status;
}

namespace
{

/**
 * XmlWriter writes a XML to a SvStream. It uses libxml2 for writing but hides
 * all the internal libxml2 workings and uses types that are native for LO
 * development.
 *
 * The codepage used for XML is always "utf-8" and the output is indented so it
 * is easier to read.
 *
 * TODO: move to common code
 */
class XmlWriter
{
private:
    SvStream* mpStream;
    xmlTextWriterPtr mpWriter;

    static int funcWriteCallback(void* pContext, const char* sBuffer, int nLen)
    {
        SvStream* pStream = static_cast<SvStream*>(pContext);
        return static_cast<int>(pStream->Write(sBuffer, nLen));
    }

    static int funcCloseCallback(void* pContext)
    {
        SvStream* pStream = static_cast<SvStream*>(pContext);
        pStream->Flush();
        return 0; // 0 or -1 in case of error
    }

public:

    XmlWriter(SvStream* pStream)
        : mpStream(pStream)
        , mpWriter(nullptr)
    {
    }

    ~XmlWriter()
    {
        if (mpWriter != nullptr)
            endDocument();
    }

    bool startDocument()
    {
        xmlOutputBufferPtr xmlOutBuffer = xmlOutputBufferCreateIO(funcWriteCallback, funcCloseCallback, mpStream, nullptr);
        mpWriter = xmlNewTextWriter(xmlOutBuffer);
        if (mpWriter == nullptr)
            return false;
        xmlTextWriterSetIndent(mpWriter, 1);
        xmlTextWriterStartDocument(mpWriter, nullptr, "UTF-8", nullptr);
        return true;
    }

    void endDocument()
    {
        xmlTextWriterEndDocument(mpWriter);
        xmlFreeTextWriter(mpWriter);
        mpWriter = nullptr;
    }

    void startElement(const OString& sName)
    {
        xmlChar* xmlName = xmlCharStrdup(sName.getStr());
        xmlTextWriterStartElement(mpWriter, xmlName);
        xmlFree(xmlName);
    }

    void endElement()
    {
        xmlTextWriterEndElement(mpWriter);
    }

    void content(const OString& sValue)
    {
        xmlChar* xmlValue = xmlCharStrdup(sValue.getStr());
        xmlTextWriterWriteString(mpWriter, xmlValue);
        xmlFree(xmlValue);
    }
};

/**
 * XmlWalker main purpose is to make it easier for walking the
 * parsed XML DOM tree.
 *
 * It hides all the libxml2 and C -isms and makes the useage more
 * confortable from LO developer point of view.
 *
 * TODO: move to common code
 */
class XmlWalker
{
private:
    xmlDocPtr mpDocPtr;
    xmlNodePtr mpRoot;
    xmlNodePtr mpCurrent;

    std::vector<xmlNodePtr> mpStack;

public:
    XmlWalker()
    {}

    ~XmlWalker()
    {
        xmlFreeDoc(mpDocPtr);
    }

    bool open(SvStream* pStream)
    {
        sal_Size nSize = pStream->remainingSize();
        std::vector<sal_uInt8> aBuffer(nSize + 1);
        pStream->Read(aBuffer.data(), nSize);
        aBuffer[nSize] = 0;
        mpDocPtr = xmlParseDoc(reinterpret_cast<xmlChar*>(aBuffer.data()));
        if (mpDocPtr == nullptr)
            return false;
        mpRoot = xmlDocGetRootElement(mpDocPtr);
        mpCurrent = mpRoot;
        mpStack.push_back(mpCurrent);
        return true;
    }

    OString name()
    {
        return OString(reinterpret_cast<const char*>(mpCurrent->name));
    }

    OString content()
    {
        OString aContent;
        if (mpCurrent->xmlChildrenNode != nullptr)
        {
            xmlChar* pContent = xmlNodeListGetString(mpDocPtr, mpCurrent->xmlChildrenNode, 1);
            aContent = OString(reinterpret_cast<const char*>(pContent));
            xmlFree(pContent);
        }
        return aContent;
    }

    void children()
    {
        mpStack.push_back(mpCurrent);
        mpCurrent = mpCurrent->xmlChildrenNode;
    }

    void parent()
    {
        mpCurrent = mpStack.back();
        mpStack.pop_back();
    }

    void next()
    {
        mpCurrent = mpCurrent->next;
    }

    bool isValid()
    {
        return mpCurrent != nullptr;
    }
};

} // end anonymous namespace

inline ds_status writeProfile(const OUString& rStreamName, ds_profile* pProfile)
{
    ds_status eStatus = DS_SUCCESS;

    if (pProfile == nullptr)
        return DS_INVALID_PROFILE;
    if (rStreamName.isEmpty())
        return DS_INVALID_PROFILE;

    std::unique_ptr<SvStream> pStream;
    pStream.reset(new SvFileStream(rStreamName, STREAM_STD_READWRITE | StreamMode::TRUNC));

    XmlWriter aXmlWriter(pStream.get());

    if (aXmlWriter.startDocument() == false)
        return DS_FILE_ERROR;

    aXmlWriter.startElement("profile");

    aXmlWriter.startElement("version");
    aXmlWriter.content(OString(pProfile->version));
    aXmlWriter.endElement();

    for (size_t i = 0; i < pProfile->numDevices && eStatus == DS_SUCCESS; i++)
    {
        aXmlWriter.startElement("device");

        switch(pProfile->devices[i].eType)
        {
            case DeviceType::NativeCPU:
                aXmlWriter.startElement("type");
                aXmlWriter.content("native");
                aXmlWriter.endElement();
                break;
            case DeviceType::OpenCLDevice:
                aXmlWriter.startElement("type");
                aXmlWriter.content("opencl");
                aXmlWriter.endElement();

                aXmlWriter.startElement("name");
                aXmlWriter.content(OString(pProfile->devices[i].sDeviceName));
                aXmlWriter.endElement();

                aXmlWriter.startElement("driver");
                aXmlWriter.content(OString(pProfile->devices[i].sDriverVersion));
                aXmlWriter.endElement();
                break;
            default:
                break;
        }

        aXmlWriter.startElement("time");
        if (pProfile->devices[i].fTime == DBL_MAX)
            aXmlWriter.content("max");
        else
            aXmlWriter.content(OString::number(pProfile->devices[i].fTime));
        aXmlWriter.endElement();

        aXmlWriter.startElement("errors");
        aXmlWriter.content(pProfile->devices[i].bErrors ? "true" : "false");
        aXmlWriter.endElement();

        aXmlWriter.endElement();
    }

    aXmlWriter.endElement();
    aXmlWriter.endDocument();

    return eStatus;
}

inline ds_status readProfile(const OUString& rStreamName, ds_profile* pProfile)
{
    ds_status eStatus = DS_SUCCESS;

    if (rStreamName.isEmpty())
        return DS_INVALID_PROFILE;

    std::unique_ptr<SvStream> pStream;
    pStream.reset(new SvFileStream(rStreamName, StreamMode::READ));
    XmlWalker aWalker;

    if (!aWalker.open(pStream.get()))
        return DS_FILE_ERROR;

    if (aWalker.name() == "profile")
    {
        aWalker.children();
        while (aWalker.isValid())
        {
            if (aWalker.name() == "version")
            {
                if (aWalker.content() != pProfile->version)
                    return DS_PROFILE_FILE_ERROR;
            }
            else if (aWalker.name() == "device")
            {
                aWalker.children();

                DeviceType eDeviceType = DeviceType::None;
                OString sName;
                OString sVersion;
                double fTime = -1.0;
                bool bErrors = true;

                while (aWalker.isValid())
                {
                    if (aWalker.name() == "type")
                    {
                        OString sContent = aWalker.content();
                        if (sContent == "native")
                            eDeviceType = DeviceType::NativeCPU;
                        else if (sContent == "opencl")
                            eDeviceType = DeviceType::OpenCLDevice;
                        else
                            return DS_PROFILE_FILE_ERROR;
                    }
                    else if (aWalker.name() == "name")
                    {
                        sName = aWalker.content();
                    }
                    else if (aWalker.name() == "driver")
                    {
                        sVersion = aWalker.content();
                    }
                    else if (aWalker.name() == "time")
                    {
                        if (aWalker.content() == "max")
                            fTime = DBL_MAX;
                        else
                            fTime = aWalker.content().toDouble();
                    }
                    else if (aWalker.name() == "errors")
                    {
                        bErrors = (aWalker.content() == "true");
                    }

                    aWalker.next();
                }

                if (fTime < 0.0)
                    return DS_PROFILE_FILE_ERROR;

                for (unsigned int i = 0; i < pProfile->numDevices; i++)
                {
                    // type matches? either both are DS_DEVICE_OPENCL_DEVICE or DS_DEVICE_NATIVE_CPU
                    if (pProfile->devices[i].eType == eDeviceType)
                    {
                        // is DS_DEVICE_NATIVE_CPU or name + version matches?
                        if (eDeviceType == DeviceType::NativeCPU ||
                                (sName == OString(pProfile->devices[i].sDeviceName) &&
                                 sVersion == OString(pProfile->devices[i].sDriverVersion)))
                        {
                            pProfile->devices[i].fTime = fTime;
                            pProfile->devices[i].bErrors = bErrors;
                        }
                    }
                }

                aWalker.parent();
            }
            aWalker.next();
        }
        aWalker.parent();
    }

    return eStatus;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
