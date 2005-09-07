/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: framework.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:34:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
