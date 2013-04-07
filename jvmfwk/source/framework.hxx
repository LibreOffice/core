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
#ifndef INCLUDED_JVMFWK_LOCAL_FRAMEWORK_HXX
#define INCLUDED_JVMFWK_LOCAL_FRAMEWORK_HXX

#include "rtl/ustring.hxx"
#include "rtl/byteseq.hxx"
#include "jvmfwk/framework.h"
#include "jvmfwk/vendorplugin.h"

/** typedefs for functions from vendorplugin.h
 */
typedef javaPluginError (*jfw_plugin_getAllJavaInfos_ptr)(
    rtl_uString * sVendor,
    rtl_uString * sMinVersion,
    rtl_uString * sMaxVersion,
    rtl_uString * * arExcludeList,
    sal_Int32  nLenList,
    JavaInfo*** parJavaInfo,
    sal_Int32 *nLenInfoList);

typedef javaPluginError (*jfw_plugin_getJavaInfoByPath_ptr)(
    rtl_uString * sPath,
    rtl_uString * sVendor,
    rtl_uString * sMinVersion,
    rtl_uString * sMaxVersion,
    rtl_uString * * arExcludeList,
    sal_Int32  nLenList,
    JavaInfo** ppInfo);

/** starts a Java Virtual Machine.
    <p>
    The function shall ensure, that the VM does not abort the process
    during instantiation.
    </p>
 */
typedef javaPluginError (*jfw_plugin_startJavaVirtualMachine_ptr)(
    const JavaInfo *info,
    const JavaVMOption* options,
    sal_Int32 cOptions,
    JavaVM ** ppVM,
    JNIEnv ** ppEnv);

typedef javaPluginError (*jfw_plugin_existJRE_ptr)(
    const JavaInfo *info,
    sal_Bool *exist);


namespace jfw
{

class CJavaInfo
{
    static JavaInfo * copyJavaInfo(const JavaInfo * pInfo);

    enum _transfer_ownership {TRANSFER};
    /*Attaching the pointer to this class. The argument pInfo must not
    be freed afterwards.
    */
    CJavaInfo(::JavaInfo * info, _transfer_ownership);

public:
    ::JavaInfo * pInfo;



    CJavaInfo();
    CJavaInfo(const CJavaInfo &);
    ~CJavaInfo();
    CJavaInfo& operator =(const ::JavaInfo* info);
    CJavaInfo & operator = (const CJavaInfo& info);

    /* The returned class takes ownership of the argument info. info
    must not been freed afterwards.
    */
    static CJavaInfo createWrapper(::JavaInfo* info);
    /*Attaching the pointer to this class. The argument pInfo must not
    be freed afterwards.
    */
    void attach(::JavaInfo* pInfo);
    ::JavaInfo * detach();
    const ::JavaInfo* operator ->() const;
    operator ::JavaInfo* ();
    operator ::JavaInfo const * () const;
    ::JavaInfo* cloneJavaInfo() const;

    OUString getVendor() const;
    OUString getLocation() const;
    sal_uInt64 getFeatures() const;
};

class FrameworkException
{
public:

    FrameworkException(javaFrameworkError err, const OString& msg):
        errorCode(err), message(msg)
        {
        }
    javaFrameworkError errorCode;
    OString message;
};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
