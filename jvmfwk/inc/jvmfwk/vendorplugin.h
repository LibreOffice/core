/*************************************************************************
 *
 *  $RCSfile: vendorplugin.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jl $ $Date: 2004-05-07 14:49:40 $
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

#if !defined INCLUDED_JVMFWK_VENDORPLUGIN_H
#define INCLUDED_JVMFWK_VENDORPLUGIN_H

#include "jvmfwk/framework.h"
#include "rtl/ustring.h"
#include "jni.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
   This library is dynamically loaded and unloaded. Therefore do not
   keep global variables.
 */

typedef enum
{
    JFW_PLUGIN_E_NONE,
    JFW_PLUGIN_E_ERROR,
    JFW_PLUGIN_E_INVALID_ARG,
    JFW_PLUGIN_E_WRONG_VERSION_FORMAT,
    JFW_PLUGIN_E_FAILED_VERSION,
    JFW_PLUGIN_E_NO_JRE,
    JFW_PLUGIN_E_WRONG_VENDOR,
    JFW_PLUGIN_E_VM_CREATION_FAILED
} javaPluginError;



/** obtains information about Java installations of which all have the same
    vendor.
    <p>
    In case an error occurred parJavaInfo does not to be freed.
    </p>
    The array parJavaInfo must be freed by the caller with rtl_freeMemory.
    @return
    JFW_PLUGIN_E_NONE,
    JFW_PLUGIN_E_ERROR,
    JFW_PLUGIN_E_INVALID_ARG,
    JFW_PLUGIN_E_WRONG_VERSION_FORMAT
 */
javaPluginError getAllJavaInfos(
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString * * arExcludeList,
    sal_Int32  nLenList,
    JavaInfo*** parJavaInfo,
    sal_Int32 *nLenInfoList);

/**
   @return
   JFW_PLUGIN_E_NONE
    JFW_PLUGIN_E_ERROR
    JFW_PLUGIN_E_INVALID_ARG
    JFW_PLUGIN_E_WRONG_VERSION_FORMAT
    JFW_PLUGIN_E_FAILED_VERSION
    JFW_PLUGIN_E_NO_JRE
 */
javaPluginError getJavaInfoByPath(
    rtl_uString *path,
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString * *arExcludeList,
    sal_Int32  nLenList,
    JavaInfo ** ppInfo);

/** starts a Java Virtual Machine.
    <p>
    The function shall ensure, that the VM does not abort the process
    during instantiation.</p>
    @param
    JFW_PLUGIN_E_NONE,
    JFW_PLUGIN_E_ERROR,
    JFW_PLUGIN_E_WRONG_VENDOR
    JFW_PLUGIN_E_INVALID_ARG,
    JFW_PLUGIN_E_VM_CREATION_FAILED

 */
javaPluginError startJavaVirtualMachine(
    const JavaInfo *info,
    const JavaVMOption* options,
    sal_Int32 cOptions,
    JavaVM ** ppVM,
    JNIEnv ** ppEnv);



#ifdef __cplusplus
}
#endif


#endif
