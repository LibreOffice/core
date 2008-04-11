/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: filter.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_CONSTANT_FILTER_HXX_
#include <constant/filter.hxx>
#endif

namespace framework{
    namespace constant{

const ::rtl::OUString Filter::PROP_NAME                                  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"           ));
const ::rtl::OUString Filter::PROP_TYPE                                  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Type"           ));
const ::rtl::OUString Filter::PROP_DOCUMENTSERVICE                       = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DocumentService"));
const ::rtl::OUString Filter::PROP_FILTERSERVICE                         = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FilterService"  ));
const ::rtl::OUString Filter::PROP_UICOMPONENT                           = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UIComponent"    ));
const ::rtl::OUString Filter::PROP_FLAGS                                 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Flags"          ));
const ::rtl::OUString Filter::PROP_USERDATA                              = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UserData"       ));
const ::rtl::OUString Filter::PROP_TEMPLATENAME                          = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TemplateName"   ));

const ::rtl::OUString Filter::QUERY_GET_DEFAULT_FILTER_FOR_TYPE          = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getDefaultFilterForType"));
const ::rtl::OUString Filter::QUERY_ALL                                  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_query_all"             ));
const ::rtl::OUString Filter::QUERY_WRITER                               = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_query_writer"          ));
const ::rtl::OUString Filter::QUERY_WEB                                  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_query_web"             ));
const ::rtl::OUString Filter::QUERY_GLOBAL                               = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_query_global"          ));
const ::rtl::OUString Filter::QUERY_CHART                                = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_query_chart"           ));
const ::rtl::OUString Filter::QUERY_CALC                                 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_query_calc"            ));
const ::rtl::OUString Filter::QUERY_IMPRESS                              = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_query_impress"         ));
const ::rtl::OUString Filter::QUERY_DRAW                                 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_query_draw"            ));
const ::rtl::OUString Filter::QUERY_MATH                                 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_query_math"            ));

const ::rtl::OUString Filter::QUERYPARAM_IFLAGS                          = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("iflags"        ));
const ::rtl::OUString Filter::QUERYPARAM_EFLAGS                          = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("eflags"        ));
const ::rtl::OUString Filter::QUERYPARAM_SORT_PROP                       = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sort_prop"     ));
const ::rtl::OUString Filter::QUERYPARAM_DESCENDING                      = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("descending"    ));
const ::rtl::OUString Filter::QUERYPARAM_USE_ORDER                       = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("use_order"     ));
const ::rtl::OUString Filter::QUERYPARAM_DEFAULT_FIRST                   = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("default_first" ));
const ::rtl::OUString Filter::QUERYPARAM_CASE_SENSITIVE                  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("case_sensitive"));
const ::rtl::OUString Filter::QUERYPARAMVALUE_SORT_PROP_NAME             = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("name"          ));
const ::rtl::OUString Filter::QUERYPARAMVALUE_SORT_PROP_UINAME           = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("uiname"        ));

const ::rtl::OUString Filter::FLAGNAME_IMPORT                            = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Import"           ));
const ::rtl::OUString Filter::FLAGNAME_EXPORT                            = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Export"           ));
const ::rtl::OUString Filter::FLAGNAME_TEMPLATE                          = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Template"         ));
const ::rtl::OUString Filter::FLAGNAME_INTERNAL                          = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Internal"         ));
const ::rtl::OUString Filter::FLAGNAME_TEMPLATEPATH                      = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TemplatePath"     ));
const ::rtl::OUString Filter::FLAGNAME_OWN                               = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Own"              ));
const ::rtl::OUString Filter::FLAGNAME_ALIEN                             = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Alien"            ));
const ::rtl::OUString Filter::FLAGNAME_USESOPTIONS                       = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UsesOptions"      ));
const ::rtl::OUString Filter::FLAGNAME_DEFAULT                           = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Default"          ));
const ::rtl::OUString Filter::FLAGNAME_EXECUTABLE                        = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Executable"       ));
const ::rtl::OUString Filter::FLAGNAME_SUPPORTSSELECTION                 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SupportsSelection"));
const ::rtl::OUString Filter::FLAGNAME_MAPTOAPPPLUG                      = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MapToAppPlug"     ));
const ::rtl::OUString Filter::FLAGNAME_NOTINFILEDIALOG                   = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NotInFileDialog"  ));
const ::rtl::OUString Filter::FLAGNAME_NOTINCHOOSER                      = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NotInChooser"     ));
const ::rtl::OUString Filter::FLAGNAME_ASYNCHRON                         = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Asynchron"        ));
const ::rtl::OUString Filter::FLAGNAME_CREATOR                           = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Creator"          ));
const ::rtl::OUString Filter::FLAGNAME_READONLY                          = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Readonly"         ));
const ::rtl::OUString Filter::FLAGNAME_NOTINSTALLED                      = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NotInstalled"     ));
const ::rtl::OUString Filter::FLAGNAME_CONSULTSERVICE                    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ConsultService"   ));
const ::rtl::OUString Filter::FLAGNAME_3RDPARTYFILTER                    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("3rdPartyFilter"   ));
const ::rtl::OUString Filter::FLAGNAME_PACKED                            = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Packed"           ));
const ::rtl::OUString Filter::FLAGNAME_SILENTEXPORT                      = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SilentExport"     ));
const ::rtl::OUString Filter::FLAGNAME_BROWSERPREFERED                   = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BrowserPrefered"  ));
const ::rtl::OUString Filter::FLAGNAME_PREFERED                          = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Prefered"         ));

const sal_Int32 Filter::FLAGVALUE_IMPORT                                 = 0x00000001L;     // 1
const sal_Int32 Filter::FLAGVALUE_EXPORT                                 = 0x00000002L;     // 2
const sal_Int32 Filter::FLAGVALUE_TEMPLATE                               = 0x00000004L;     // 4
const sal_Int32 Filter::FLAGVALUE_INTERNAL                               = 0x00000008L;     // 8
const sal_Int32 Filter::FLAGVALUE_TEMPLATEPATH                           = 0x00000010L;     // 16
const sal_Int32 Filter::FLAGVALUE_OWN                                    = 0x00000020L;     // 32
const sal_Int32 Filter::FLAGVALUE_ALIEN                                  = 0x00000040L;     // 64
const sal_Int32 Filter::FLAGVALUE_USESOPTIONS                            = 0x00000080L;     // 128
const sal_Int32 Filter::FLAGVALUE_DEFAULT                                = 0x00000100L;     // 256
const sal_Int32 Filter::FLAGVALUE_EXECUTABLE                             = 0x00000200L;     // 512
const sal_Int32 Filter::FLAGVALUE_SUPPORTSSELECTION                      = 0x00000400L;     // 1024
const sal_Int32 Filter::FLAGVALUE_MAPTOAPPPLUG                           = 0x00000800L;     // 2048
const sal_Int32 Filter::FLAGVALUE_NOTINFILEDIALOG                        = 0x00001000L;     // 4096
const sal_Int32 Filter::FLAGVALUE_NOTINCHOOSER                           = 0x00002000L;     // 8192
const sal_Int32 Filter::FLAGVALUE_ASYNCHRON                              = 0x00004000L;     // 16384
const sal_Int32 Filter::FLAGVALUE_CREATOR                                = 0x00008000L;     // 32768
const sal_Int32 Filter::FLAGVALUE_READONLY                               = 0x00010000L;     // 65536
const sal_Int32 Filter::FLAGVALUE_NOTINSTALLED                           = 0x00020000L;     // 131072
const sal_Int32 Filter::FLAGVALUE_CONSULTSERVICE                         = 0x00040000L;     // 262144
const sal_Int32 Filter::FLAGVALUE_3RDPARTYFILTER                         = 0x00080000L;     // 524288
const sal_Int32 Filter::FLAGVALUE_PACKED                                 = 0x00100000L;     // 1048576
const sal_Int32 Filter::FLAGVALUE_SILENTEXPORT                           = 0x00200000L;     // 2097152
const sal_Int32 Filter::FLAGVALUE_BROWSERPREFERED                        = 0x00400000L;     // 4194304
const sal_Int32 Filter::FLAGVALUE_PREFERED                               = 0x10000000L;     // 268435456

/*-----------------------------------------------
    06.08.2003 09:47
-----------------------------------------------*/
sal_Bool Filter::areFlagsSet(sal_Int32 nField, sal_Int32 nMask)
{
    return ((nField & nMask) == nMask);
}

/*-----------------------------------------------
    06.08.2003 09:48
-----------------------------------------------*/
void Filter::addFlags(sal_Int32& nField, sal_Int32 nFlags)
{
    nField |= nFlags;
}

/*-----------------------------------------------
    06.08.2003 09:48
-----------------------------------------------*/
void Filter::removeFlags(sal_Int32& nField, sal_Int32 nFlags)
{
    nField &= ~nFlags;
}

    } // namespace constant
} // namespace framework
