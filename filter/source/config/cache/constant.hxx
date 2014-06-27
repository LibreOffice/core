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
#ifndef INCLUDED_FILTER_SOURCE_CONFIG_CACHE_CONSTANT_HXX
#define INCLUDED_FILTER_SOURCE_CONFIG_CACHE_CONSTANT_HXX

#include "macros.hxx"

/*  disable impl_loadOnDemand function of BaseContainer for certain
    functions, where it the feature "impl_loadItemOnDemand() of class FilterCache
    can be used instead of loadAll()!*/
// #define LOAD_IMPLICIT


/** @short  used to identify a some generic item properties against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
#define  PROPNAME_NAME  "Name"

/** @short  used to identify a type item property against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
#define  PROPNAME_UINAME            "UIName"
#define  PROPNAME_UINAMES           "UINames"
#define  PROPNAME_PREFERRED         "Preferred"
#define  PROPNAME_PREFERREDFILTER   "PreferredFilter"
#define  PROPNAME_DETECTSERVICE     "DetectService"
#define  PROPNAME_MEDIATYPE         "MediaType"
#define  PROPNAME_CLIPBOARDFORMAT   "ClipboardFormat"
#define  PROPNAME_URLPATTERN        "URLPattern"
#define  PROPNAME_EXTENSIONS        "Extensions"

/** @short  used to identify a filter item property against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
#define  PROPNAME_TYPE              "Type"
#define  PROPNAME_DOCUMENTSERVICE   "DocumentService"
#define  PROPNAME_FILTERSERVICE     "FilterService"
#define  PROPNAME_UICOMPONENT       "UIComponent"
#define  PROPNAME_FLAGS             "Flags"
#define  PROPNAME_USERDATA          "UserData"
#define  PROPNAME_TEMPLATENAME      "TemplateName"
#define  PROPNAME_FILEFORMATVERSION "FileFormatVersion"
#define  PROPNAME_EXPORTEXTENSION   "ExportExtension"

/** @short  used to identify a frame loader or detect service item
            property against the configuration API and can be used
            at all name containers (based on this filtercache) too.
 */
#define  PROPNAME_TYPES     "Types"

/** @short  used to identify the list of sorted filters for a specific
            office module
 */
#define  PROPNAME_SORTEDFILTERLIST     "SortedFilterList"

/** @short  implicit properties. which are used at the container interface only.
 */
#define  PROPNAME_FINALIZED "Finalized"
#define  PROPNAME_MANDATORY "Mandatory"

/** @short  used to identify a set of items against the configuration API. */
#define  CFGSET_TYPES               "Types"
#define  CFGSET_FILTERS             "Filters"
#define  CFGSET_FRAMELOADERS        "FrameLoaders"
#define  CFGSET_CONTENTHANDLERS     "ContentHandlers"

/** @short  used to address some configuration keys directly.

    @descr  Such direct keys should be used with function
            FilterCache::impl_getDirectCFGValue() only!

    @TODO   define these direct keys ...
 */
#define  CFGDIRECTKEY_OFFICELOCALE          "/org.openoffice.Setup/L10N/ooLocale"
#define  CFGDIRECTKEY_DEFAULTFRAMELOADER    "/org.openoffice.TypeDetection.Misc/Defaults/DefaultFrameLoader"
#define  CFGDIRECTKEY_OFFICELOCALE          "/org.openoffice.Setup/L10N/ooLocale"
#define  CFGDIRECTKEY_PRODUCTNAME           "/org.openoffice.Setup/Product/ooName"

// Note that these flag bits have parallel names in
// comphelper/inc/comphelper/documentconstants.hxx . See that file for
// documentation on their meaning.

/** @short  names of filter flags, sorted in alphabetical order */
#define  FLAGNAME_3RDPARTYFILTER    "3RDPARTYFILTER"
#define  FLAGNAME_ALIEN             "ALIEN"
#define  FLAGNAME_ASYNCHRON         "ASYNCHRON"
#define  FLAGNAME_BROWSERPREFERRED  "BROWSERPREFERRED"
#define  FLAGNAME_CONSULTSERVICE    "CONSULTSERVICE"
#define  FLAGNAME_DEFAULT           "DEFAULT"
#define  FLAGNAME_ENCRYPTION        "ENCRYPTION"
#define  FLAGNAME_EXPORT            "EXPORT"
#define  FLAGNAME_IMPORT            "IMPORT"
#define  FLAGNAME_INTERNAL          "INTERNAL"
#define  FLAGNAME_NOTINCHOOSER      "NOTINCHOOSER"
#define  FLAGNAME_NOTINFILEDIALOG   "NOTINFILEDIALOG"
#define  FLAGNAME_NOTINSTALLED      "NOTINSTALLED"
#define  FLAGNAME_OWN               "OWN"
#define  FLAGNAME_PACKED            "PACKED"
#define  FLAGNAME_PASSWORDTOMODIFY  "PASSWORDTOMODIFY"
#define  FLAGNAME_PREFERRED         "PREFERRED"
#define  FLAGNAME_STARTPRESENTATION "STARTPRESENTATION"
#define  FLAGNAME_READONLY          "READONLY"
#define  FLAGNAME_SUPPORTSSELECTION "SUPPORTSSELECTION"
#define  FLAGNAME_TEMPLATE          "TEMPLATE"
#define  FLAGNAME_TEMPLATEPATH      "TEMPLATEPATH"
#define  FLAGNAME_USESOPTIONS       "USESOPTIONS"
#define  FLAGNAME_COMBINED          "COMBINED"

/** @short  values of filter flags, sorted based on value */
#define  FLAGVAL_IMPORT            0x00000001 // 1
#define  FLAGVAL_EXPORT            0x00000002 // 2
#define  FLAGVAL_TEMPLATE          0x00000004 // 4
#define  FLAGVAL_INTERNAL          0x00000008 // 8
#define  FLAGVAL_TEMPLATEPATH      0x00000010 // 16
#define  FLAGVAL_OWN               0x00000020 // 32
#define  FLAGVAL_ALIEN             0x00000040 // 64
#define  FLAGVAL_USESOPTIONS       0x00000080 // 128
#define  FLAGVAL_DEFAULT           0x00000100 // 256
#define  FLAGVAL_SUPPORTSSELECTION 0x00000400 // 1024
#define  FLAGVAL_NOTINFILEDIALOG   0x00001000 // 4096
#define  FLAGVAL_NOTINCHOOSER      0x00002000 // 8192
#define  FLAGVAL_ASYNCHRON         0x00004000 // 16384
#define  FLAGVAL_READONLY          0x00010000 // 65536
#define  FLAGVAL_NOTINSTALLED      0x00020000 // 131072
#define  FLAGVAL_CONSULTSERVICE    0x00040000 // 262144
#define  FLAGVAL_3RDPARTYFILTER    0x00080000 // 524288
#define  FLAGVAL_PACKED            0x00100000 // 1048576
#define  FLAGVAL_BROWSERPREFERRED  0x00400000 // 4194304
#define  FLAGVAL_COMBINED          0x00800000 // 8388608
#define  FLAGVAL_ENCRYPTION        0x01000000 // 16777216
#define  FLAGVAL_PASSWORDTOMODIFY  0x02000000 // 33554432
#define  FLAGVAL_PREFERRED         0x10000000 // 268435456
#define  FLAGVAL_STARTPRESENTATION 0x20000000 // 268435456
#define  FLAGVAL_ALL               0xffffffff // 4294967295

/** @short  some uno service names.
 */
#define  SERVICE_CONFIGURATIONUPDATEACCESS  "com.sun.star.configuration.ConfigurationUpdateAccess"
#define  SERVICE_CONFIGURATIONACCESS        "com.sun.star.configuration.ConfigurationAccess"

/** @short  some configuration paths.
 */
#define  CFGPACKAGE_TD_TYPES           "/org.openoffice.TypeDetection.Types"
#define  CFGPACKAGE_TD_FILTERS         "/org.openoffice.TypeDetection.Filter"
#define  CFGPACKAGE_TD_OTHERS          "/org.openoffice.TypeDetection.Misc"
#define  CFGPACKAGE_TD_OLD             "/org.openoffice.Office.TypeDetection"

/** @short  some default values.
 */
#define  DEFAULT_OFFICELOCALE       "en-US"

/** @short  used for the queries of the FilterFactory service.
 */
#define  QUERY_IDENTIFIER_MATCHBYDOCUMENTSERVICE    "matchByDocumentService"
#define  QUERY_IDENTIFIER_GETPREFERREDFILTERFORTYPE "getDefaultFilterForType"
#define  QUERY_IDENTIFIER_GET_SORTED_FILTERLIST     "getSortedFilterList()"

#define  QUERY_PARAM_IFLAGS                         "iflags"
#define  QUERY_PARAM_EFLAGS                         "eflags"
#define  QUERY_PARAM_MODULE                         "module"
#define  QUERY_PARAM_DEFAULTFIRST                   "default_first"
#define  QUERY_CONSTVALUE_ALL                       "all"

#endif // INCLUDED_FILTER_SOURCE_CONFIG_CACHE_CONSTANT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
