/*************************************************************************
 *
 *  $RCSfile: framework.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:54:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#if !defined INCLUDED_JVMFWK_LOCAL_FRAMEWORK_HXX
#define INCLUDED_JVMFWK_LOCAL_FRAMEWORK_HXX
#include "rtl/ustring.hxx"
#include "rtl/byteseq.hxx"
#include "jvmfwk/framework.h"
#include "jvmfwk/vendorplugin.h"


//#define NS_JAVA_FRAMEWORK "http://openoffice.org/2004/java/framework/1.0"
//#define NS_SCHEMA_INSTANCE "http://www.w3.org/2001/XMLSchema-instance"

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
    CJavaInfo(const CJavaInfo &);
    CJavaInfo& operator = (const CJavaInfo&);

    static JavaInfo * copyJavaInfo(const JavaInfo * pInfo);
public:
    ::JavaInfo * pInfo;

    CJavaInfo();
    CJavaInfo(const ::JavaInfo* pInfo);
    ~CJavaInfo();
    CJavaInfo& operator =(const ::JavaInfo* info);
    const ::JavaInfo* operator ->() const;
    ::JavaInfo** operator & ();
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

}
#endif
