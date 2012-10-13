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
#ifndef _FILTER_CONFIG_CONSTANT_HXX_
#define _FILTER_CONFIG_CONSTANT_HXX_

#include "macros.hxx"

/*  disable impl_loadOnDemand function of BaseContainer for certain
    functions, where it the feature "impl_loadItemOnDemand() of class FilterCache
    can be used instead of loadAll()!*/
// #define LOAD_IMPLICIT

//_______________________________________________

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
#define  PROPNAME_SORTEDFILTERLIST     _FILTER_CONFIG_FROM_ASCII_("SortedFilterList")

/** @short  implicit properties. which are used at the container interface only.
 */
#define  PROPNAME_FINALIZED _FILTER_CONFIG_FROM_ASCII_("Finalized")
#define  PROPNAME_MANDATORY _FILTER_CONFIG_FROM_ASCII_("Mandatory")

/** @short  used to identify a set of items against the configuration API. */
#define  CFGSET_TYPES               _FILTER_CONFIG_FROM_ASCII_("Types"          )
#define  CFGSET_FILTERS             _FILTER_CONFIG_FROM_ASCII_("Filters"        )
#define  CFGSET_FRAMELOADERS        _FILTER_CONFIG_FROM_ASCII_("FrameLoaders"   )
#define  CFGSET_CONTENTHANDLERS     _FILTER_CONFIG_FROM_ASCII_("ContentHandlers")

/** @short  used to address some configuration keys directly.

    @descr  Such direct keys should be used with function
            FilterCache::impl_getDirectCFGValue() only!

    @TODO   define these direct keys ...
 */
#define  CFGDIRECTKEY_OFFICELOCALE          _FILTER_CONFIG_FROM_ASCII_("/org.openoffice.Setup/L10N/ooLocale"                           )
#define  CFGDIRECTKEY_DEFAULTFRAMELOADER    _FILTER_CONFIG_FROM_ASCII_("/org.openoffice.TypeDetection.Misc/Defaults/DefaultFrameLoader")
#define  CFGDIRECTKEY_OFFICELOCALE          _FILTER_CONFIG_FROM_ASCII_("/org.openoffice.Setup/L10N/ooLocale"                           )
#define  CFGDIRECTKEY_FORMATNAME            _FILTER_CONFIG_FROM_ASCII_("/org.openoffice.Setup/Product/ooXMLFileFormatName"             )
#define  CFGDIRECTKEY_FORMATVERSION         _FILTER_CONFIG_FROM_ASCII_("/org.openoffice.Setup/Product/ooXMLFileFormatVersion"          )
#define  CFGDIRECTKEY_PRODUCTNAME           _FILTER_CONFIG_FROM_ASCII_("/org.openoffice.Setup/Product/ooName"                          )

// Note that these flag bits have parallel names in
// comphelper/inc/comphelper/documentconstants.hxx . See that file for
// documentation on their meaning.

/** @short  names of filter flags, sorted in alphabetical order */
#define  FLAGNAME_3RDPARTYFILTER    _FILTER_CONFIG_FROM_ASCII_("3RDPARTYFILTER"   )
#define  FLAGNAME_ALIEN             _FILTER_CONFIG_FROM_ASCII_("ALIEN"            )
#define  FLAGNAME_ASYNCHRON         _FILTER_CONFIG_FROM_ASCII_("ASYNCHRON"        )
#define  FLAGNAME_BROWSERPREFERRED  _FILTER_CONFIG_FROM_ASCII_("BROWSERPREFERRED" )
#define  FLAGNAME_CONSULTSERVICE    _FILTER_CONFIG_FROM_ASCII_("CONSULTSERVICE"   )
#define  FLAGNAME_DEFAULT           _FILTER_CONFIG_FROM_ASCII_("DEFAULT"          )
#define  FLAGNAME_ENCRYPTION        _FILTER_CONFIG_FROM_ASCII_("ENCRYPTION"       )
#define  FLAGNAME_EXPORT            _FILTER_CONFIG_FROM_ASCII_("EXPORT"           )
#define  FLAGNAME_IMPORT            _FILTER_CONFIG_FROM_ASCII_("IMPORT"           )
#define  FLAGNAME_INTERNAL          _FILTER_CONFIG_FROM_ASCII_("INTERNAL"         )
#define  FLAGNAME_NOTINCHOOSER      _FILTER_CONFIG_FROM_ASCII_("NOTINCHOOSER"     )
#define  FLAGNAME_NOTINFILEDIALOG   _FILTER_CONFIG_FROM_ASCII_("NOTINFILEDIALOG"  )
#define  FLAGNAME_NOTINSTALLED      _FILTER_CONFIG_FROM_ASCII_("NOTINSTALLED"     )
#define  FLAGNAME_OWN               _FILTER_CONFIG_FROM_ASCII_("OWN"              )
#define  FLAGNAME_PACKED            _FILTER_CONFIG_FROM_ASCII_("PACKED"           )
#define  FLAGNAME_PASSWORDTOMODIFY  _FILTER_CONFIG_FROM_ASCII_("PASSWORDTOMODIFY" )
#define  FLAGNAME_PREFERRED         _FILTER_CONFIG_FROM_ASCII_("PREFERRED"        )
#define  FLAGNAME_STARTPRESENTATION _FILTER_CONFIG_FROM_ASCII_("STARTPRESENTATION")
#define  FLAGNAME_READONLY          _FILTER_CONFIG_FROM_ASCII_("READONLY"         )
#define  FLAGNAME_SUPPORTSSELECTION _FILTER_CONFIG_FROM_ASCII_("SUPPORTSSELECTION")
#define  FLAGNAME_TEMPLATE          _FILTER_CONFIG_FROM_ASCII_("TEMPLATE"         )
#define  FLAGNAME_TEMPLATEPATH      _FILTER_CONFIG_FROM_ASCII_("TEMPLATEPATH"     )
#define  FLAGNAME_USESOPTIONS       _FILTER_CONFIG_FROM_ASCII_("USESOPTIONS"      )
#define  FLAGNAME_COMBINED          _FILTER_CONFIG_FROM_ASCII_("COMBINED"         )

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
#define  SERVICE_CONFIGURATIONUPDATEACCESS  _FILTER_CONFIG_FROM_ASCII_("com.sun.star.configuration.ConfigurationUpdateAccess" )
#define  SERVICE_CONFIGURATIONACCESS        _FILTER_CONFIG_FROM_ASCII_("com.sun.star.configuration.ConfigurationAccess"       )
#define  SERVICE_FILTERCONFIGREFRESH        _FILTER_CONFIG_FROM_ASCII_("com.sun.star.document.FilterConfigRefresh"            )

/** @short  some configuration paths.
 */
#define  CFGPACKAGE_TD_TYPES           _FILTER_CONFIG_FROM_ASCII_("/org.openoffice.TypeDetection.Types" )
#define  CFGPACKAGE_TD_FILTERS         _FILTER_CONFIG_FROM_ASCII_("/org.openoffice.TypeDetection.Filter")
#define  CFGPACKAGE_TD_OTHERS          _FILTER_CONFIG_FROM_ASCII_("/org.openoffice.TypeDetection.Misc"  )
#define  CFGPACKAGE_TD_UISORT          _FILTER_CONFIG_FROM_ASCII_("/org.openoffice.TypeDetection.UISort/ModuleDependendFilterOrder")
#define  CFGPACKAGE_TD_OLD             _FILTER_CONFIG_FROM_ASCII_("/org.openoffice.Office.TypeDetection")
#define  CFGPACKAGE_OOO_MODULES        _FILTER_CONFIG_FROM_ASCII_("/org.openoffice.Setup/Office/Factories")

/** @short  some default values.
 */
#define  DEFAULT_OFFICELOCALE       _FILTER_CONFIG_FROM_ASCII_("en-US")
#define  DEFAULT_FORMATNAME         _FILTER_CONFIG_FROM_ASCII_("LibreOffice")
#define  DEFAULT_FORMATVERSION      _FILTER_CONFIG_FROM_ASCII_("1.0")

/** @short  used for the queries of the FilterFactory service.
 */
#define  QUERY_IDENTIFIER_MATCHBYDOCUMENTSERVICE    _FILTER_CONFIG_FROM_ASCII_("matchByDocumentService" )
#define  QUERY_IDENTIFIER_GETPREFERREDFILTERFORTYPE _FILTER_CONFIG_FROM_ASCII_("getDefaultFilterForType")
#define  QUERY_IDENTIFIER_GET_SORTED_FILTERLIST     _FILTER_CONFIG_FROM_ASCII_("getSortedFilterList()"  )

#define  QUERY_PARAM_IFLAGS                         _FILTER_CONFIG_FROM_ASCII_("iflags")
#define  QUERY_PARAM_EFLAGS                         _FILTER_CONFIG_FROM_ASCII_("eflags")
#define  QUERY_PARAM_MODULE                         _FILTER_CONFIG_FROM_ASCII_("module")
#define  QUERY_PARAM_DEFAULTFIRST                   _FILTER_CONFIG_FROM_ASCII_("default_first")
#define  QUERY_CONSTVALUE_ALL                       _FILTER_CONFIG_FROM_ASCII_("all")

#endif // _FILTER_CONFIG_CONSTANT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
