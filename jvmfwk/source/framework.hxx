/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: framework.hxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#if !defined INCLUDED_JVMFWK_LOCAL_FRAMEWORK_HXX
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
    explicit CJavaInfo(const ::JavaInfo* pInfo);
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
//    ::JavaInfo** operator & ();
    operator ::JavaInfo* ();
    operator ::JavaInfo const * () const;
    ::JavaInfo* cloneJavaInfo() const;

    rtl::OUString getVendor() const;
    rtl::OUString getLocation() const;
    rtl::OUString getVersion() const;
    sal_uInt64 getFeatures() const;
    sal_uInt64 getRequirements() const;
    rtl::ByteSequence getVendorData() const;
};

class FrameworkException
{
public:

    FrameworkException(javaFrameworkError err, const rtl::OString& msg):
        errorCode(err), message(msg)
        {
        }
    javaFrameworkError errorCode;
    rtl::OString message;
};
}
#endif
