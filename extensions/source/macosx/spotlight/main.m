/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
//
//  main.m
//  SpotlightTester
//
//  Created by Florian Heckl on 10.07.07.
//
//==============================================================================
//
//    DO NOT MODIFY THE CONTENTS OF THIS FILE
//
//    This file contains the generic CFPlug-in code necessary for your importer
//    To complete your importer implement the function in GetMetadataForFile.c
//
//==============================================================================

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPlugInCOM.h>
#include <CoreServices/CoreServices.h>

#include "GetMetadataForFile.h"

//    constants


#define PLUGIN_ID "A3FCC88D-B9A6-4364-8B93-92123C8A2D18"

//
// Below is the generic glue code for all plug-ins.
//
// You should not have to modify this code aside from changing
// names if you decide to change the names defined in the Info.plist
//


//    typedefs

// The layout for an instance of MetaDataImporterPlugIn
typedef struct
{
    MDImporterInterfaceStruct *conduitInterface;
    CFUUIDRef                 factoryID;
    UInt32                    refCount;
} MetadataImporterPluginType;

//    prototypes
//    Forward declaration for the IUnknown implementation.
//

MetadataImporterPluginType  *AllocMetadataImporterPluginType(CFUUIDRef inFactoryID);
void                      DeallocMetadataImporterPluginType(MetadataImporterPluginType *thisInstance);
HRESULT                   MetadataImporterQueryInterface(void *thisInstance,REFIID iid,LPVOID *ppv);
ULONG                     MetadataImporterPluginAddRef(void *thisInstance);
ULONG                     MetadataImporterPluginRelease(void *thisInstance);
//    testInterfaceFtbl    definition
//    The TestInterface function table.
//

static MDImporterInterfaceStruct testInterfaceFtbl = {
    NULL,
    MetadataImporterQueryInterface,
    MetadataImporterPluginAddRef,
    MetadataImporterPluginRelease,
    GetMetadataForFile
};


//    AllocMetadataImporterPluginType
//    Utility function that allocates a new instance.
//      You can do some initial setup for the importer here if you wish
//      like allocating globals etc...
//
MetadataImporterPluginType *AllocMetadataImporterPluginType(CFUUIDRef inFactoryID)
{
    MetadataImporterPluginType *theNewInstance;

    theNewInstance = (MetadataImporterPluginType *)malloc(sizeof(MetadataImporterPluginType));
    memset(theNewInstance,0,sizeof(MetadataImporterPluginType));

        /* Point to the function table */
    theNewInstance->conduitInterface = &testInterfaceFtbl;

        /*  Retain and keep an open instance refcount for each factory. */
    theNewInstance->factoryID = CFRetain(inFactoryID);
    CFPlugInAddInstanceForFactory(inFactoryID);

        /* This function returns the IUnknown interface so set the refCount to one. */
    theNewInstance->refCount = 1;
    return theNewInstance;
}

//    DeallocSpotlightTesterMDImporterPluginType
//    Utility function that deallocates the instance when
//    the refCount goes to zero.
//      In the current implementation importer interfaces are never deallocated
//      but implement this as this might change in the future
//
void DeallocMetadataImporterPluginType(MetadataImporterPluginType *thisInstance)
{
    CFUUIDRef theFactoryID;

    theFactoryID = thisInstance->factoryID;
    free(thisInstance);
    if (theFactoryID){
        CFPlugInRemoveInstanceForFactory(theFactoryID);
        CFRelease(theFactoryID);
    }
}

//    MetadataImporterQueryInterface
//    Implementation of the IUnknown QueryInterface function.
//
HRESULT MetadataImporterQueryInterface(void *thisInstance,REFIID iid,LPVOID *ppv)
{
    CFUUIDRef interfaceID;

    interfaceID = CFUUIDCreateFromUUIDBytes(kCFAllocatorDefault,iid);

    if (CFEqual(interfaceID,kMDImporterInterfaceID)){
            /* If the Right interface was requested, bump the ref count,
             * set the ppv parameter equal to the instance, and
             * return good status.
             */
        ((MetadataImporterPluginType*)thisInstance)->conduitInterface->AddRef(thisInstance);
        *ppv = thisInstance;
        CFRelease(interfaceID);
        return S_OK;
    }else{
        if (CFEqual(interfaceID,IUnknownUUID)){
                /* If the IUnknown interface was requested, same as above. */
            ((MetadataImporterPluginType*)thisInstance )->conduitInterface->AddRef(thisInstance);
            *ppv = thisInstance;
            CFRelease(interfaceID);
            return S_OK;
        }else{
                /* Requested interface unknown, bail with error. */
            *ppv = NULL;
            CFRelease(interfaceID);
            return E_NOINTERFACE;
        }
    }
}

//    MetadataImporterPluginAddRef
//    Implementation of reference counting for this type. Whenever an interface
//    is requested, bump the refCount for the instance. NOTE: returning the
//    refcount is a convention but is not required so don't rely on it.
//
ULONG MetadataImporterPluginAddRef(void *thisInstance)
{
    ((MetadataImporterPluginType *)thisInstance )->refCount += 1;
    return ((MetadataImporterPluginType*) thisInstance)->refCount;
}

// SampleCMPluginRelease
//    When an interface is released, decrement the refCount.
//    If the refCount goes to zero, deallocate the instance.
//
ULONG MetadataImporterPluginRelease(void *thisInstance)
{
    ((MetadataImporterPluginType*)thisInstance)->refCount -= 1;
    if (((MetadataImporterPluginType*)thisInstance)->refCount == 0){
        DeallocMetadataImporterPluginType((MetadataImporterPluginType*)thisInstance );
        return 0;
    }else{
        return ((MetadataImporterPluginType*) thisInstance )->refCount;
    }
}

//    SpotlightTesterMDImporterPluginFactory
//    Implementation of the factory function for this type.
//
__attribute__ ((visibility("default")))
void *
MetadataImporterPluginFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    (void) allocator; /* unused */
    MetadataImporterPluginType *result;
    CFUUIDRef                 uuid;

        /* If correct type is being requested, allocate an
         * instance of TestType and return the IUnknown interface.
         */
    if (CFEqual(typeID,kMDImporterTypeID)){
        uuid = CFUUIDCreateFromString(kCFAllocatorDefault,CFSTR(PLUGIN_ID));
        result = AllocMetadataImporterPluginType(uuid);
        CFRelease(uuid);
        return result;
    }
        /* If the requested type is incorrect, return NULL. */
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
