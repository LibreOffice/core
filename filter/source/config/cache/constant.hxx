/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _FILTER_CONFIG_CONSTANT_HXX_
#define _FILTER_CONFIG_CONSTANT_HXX_

#include "macros.hxx"

namespace filter { namespace config {
extern rtl::OUString pFilterStrings[];
} }
#ifndef PROPNAME_IMPL_DECL
#  define PROPNAME_DECL(index, str) (pFilterStrings[(index)])
#else
#  define PROPNAME_DECL(index, str) pFilterStrings[(index)] = _FILTER_CONFIG_FROM_ASCII_(str)
#endif

/*  disable impl_loadOnDemand function of BaseContainer for certain
    functions, where it the feature "impl_loadItemOnDemand() of class FilterCache
    can be used instead of loadAll()!*/
// #define LOAD_IMPLICIT

//_______________________________________________

/** @short  used to identify a some generic item properties against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
#define  PROPNAME_NAME  PROPNAME_DECL(0, "Name")

/** @short  used to identify a type item property against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
#define  PROPNAME_UINAME            PROPNAME_DECL(1, "UIName"          )
#define  PROPNAME_UINAMES           PROPNAME_DECL(2, "UINames"         )
#define  PROPNAME_PREFERRED         PROPNAME_DECL(3, "Preferred"       )
#define  PROPNAME_PREFERREDFILTER   PROPNAME_DECL(4, "PreferredFilter" )
#define  PROPNAME_DETECTSERVICE     PROPNAME_DECL(5, "DetectService"   )
#define  PROPNAME_MEDIATYPE         PROPNAME_DECL(6, "MediaType"       )
#define  PROPNAME_CLIPBOARDFORMAT   PROPNAME_DECL(7, "ClipboardFormat" )
#define  PROPNAME_URLPATTERN        PROPNAME_DECL(8, "URLPattern"      )
#define  PROPNAME_EXTENSIONS        PROPNAME_DECL(9, "Extensions"      )

/** @short  used to identify a filter item property against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
#define  PROPNAME_TYPE              PROPNAME_DECL(10, "Type"             )
#define  PROPNAME_DOCUMENTSERVICE   PROPNAME_DECL(11, "DocumentService"  )
#define  PROPNAME_FILTERSERVICE     PROPNAME_DECL(12, "FilterService"    )
#define  PROPNAME_UICOMPONENT       PROPNAME_DECL(13, "UIComponent"      )
#define  PROPNAME_FLAGS             PROPNAME_DECL(14, "Flags"            )
#define  PROPNAME_USERDATA          PROPNAME_DECL(15, "UserData"         )
#define  PROPNAME_TEMPLATENAME      PROPNAME_DECL(16, "TemplateName"     )
#define  PROPNAME_FILEFORMATVERSION PROPNAME_DECL(17, "FileFormatVersion")

/** @short  used to identify a frame loader or detect service item
            property against the configuration API and can be used
            at all name containers (based on this filtercache) too.
 */
#define  PROPNAME_TYPES     PROPNAME_DECL(18, "Types")

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

/** @short  used to adress some configuration keys directly.

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

/** @short  names of filter flags. */
#define  FLAGNAME_3RDPARTYFILTER    _FILTER_CONFIG_FROM_ASCII_("3RDPARTYFILTER"   )
#define  FLAGNAME_ALIEN             _FILTER_CONFIG_FROM_ASCII_("ALIEN"            )
#define  FLAGNAME_ASYNCHRON         _FILTER_CONFIG_FROM_ASCII_("ASYNCHRON"        )
#define  FLAGNAME_BROWSERPREFERRED  _FILTER_CONFIG_FROM_ASCII_("BROWSERPREFERRED" )
#define  FLAGNAME_CONSULTSERVICE    _FILTER_CONFIG_FROM_ASCII_("CONSULTSERVICE"   )
#define  FLAGNAME_DEFAULT           _FILTER_CONFIG_FROM_ASCII_("DEFAULT"          )
#define  FLAGNAME_EXPORT            _FILTER_CONFIG_FROM_ASCII_("EXPORT"           )
#define  FLAGNAME_IMPORT            _FILTER_CONFIG_FROM_ASCII_("IMPORT"           )
#define  FLAGNAME_INTERNAL          _FILTER_CONFIG_FROM_ASCII_("INTERNAL"         )
#define  FLAGNAME_NOTINCHOOSER      _FILTER_CONFIG_FROM_ASCII_("NOTINCHOOSER"     )
#define  FLAGNAME_NOTINFILEDIALOG   _FILTER_CONFIG_FROM_ASCII_("NOTINFILEDIALOG"  )
#define  FLAGNAME_NOTINSTALLED      _FILTER_CONFIG_FROM_ASCII_("NOTINSTALLED"     )
#define  FLAGNAME_OWN               _FILTER_CONFIG_FROM_ASCII_("OWN"              )
#define  FLAGNAME_PACKED            _FILTER_CONFIG_FROM_ASCII_("PACKED"           )
#define  FLAGNAME_PREFERRED         _FILTER_CONFIG_FROM_ASCII_("PREFERRED"        )
#define  FLAGNAME_READONLY          _FILTER_CONFIG_FROM_ASCII_("READONLY"         )
#define  FLAGNAME_SILENTEXPORT      _FILTER_CONFIG_FROM_ASCII_("SILENTEXPORT"     )
#define  FLAGNAME_SUPPORTSSELECTION _FILTER_CONFIG_FROM_ASCII_("SUPPORTSSELECTION")
#define  FLAGNAME_TEMPLATE          _FILTER_CONFIG_FROM_ASCII_("TEMPLATE"         )
#define  FLAGNAME_TEMPLATEPATH      _FILTER_CONFIG_FROM_ASCII_("TEMPLATEPATH"     )
#define  FLAGNAME_USESOPTIONS       _FILTER_CONFIG_FROM_ASCII_("USESOPTIONS"      )
#define  FLAGNAME_COMBINED          _FILTER_CONFIG_FROM_ASCII_("COMBINED"         )

/** @short  values of filter flags */
#define  FLAGVAL_3RDPARTYFILTER    0x00080000 // 524288
#define  FLAGVAL_ALIEN             0x00000040 // 64
#define  FLAGVAL_ALL               0xffffffff // 4294967295
#define  FLAGVAL_ASYNCHRON         0x00004000 // 16384
#define  FLAGVAL_BROWSERPREFERRED  0x00400000 // 4194304
#define  FLAGVAL_CONSULTSERVICE    0x00040000 // 262144
#define  FLAGVAL_DEFAULT           0x00000100 // 256
#define  FLAGVAL_EXPORT            0x00000002 // 2
#define  FLAGVAL_IMPORT            0x00000001 // 1
#define  FLAGVAL_INTERNAL          0x00000008 // 8
#define  FLAGVAL_NOTINCHOOSER      0x00002000 // 8192
#define  FLAGVAL_NOTINFILEDIALOG   0x00001000 // 4096
#define  FLAGVAL_NOTINSTALLED      0x00020000 // 131072
#define  FLAGVAL_OWN               0x00000020 // 32
#define  FLAGVAL_PACKED            0x00100000 // 1048576
#define  FLAGVAL_PREFERRED         0x10000000 // 268435456
#define  FLAGVAL_READONLY          0x00010000 // 65536
#define  FLAGVAL_SILENTEXPORT      0x00200000 // 2097152
#define  FLAGVAL_SUPPORTSSELECTION 0x00000400 // 1024
#define  FLAGVAL_TEMPLATE          0x00000004 // 4
#define  FLAGVAL_TEMPLATEPATH      0x00000010 // 16
#define  FLAGVAL_USESOPTIONS       0x00000080 // 128
#define  FLAGVAL_COMBINED          0x00800000 // 8388608

/** @short  uno service names of our document services
            provided by our application modules.
 */
#define  DOCUMENTSERVICE_WRITER         _FILTER_CONFIG_FROM_ASCII_("com.sun.star.text.TextDocument"                )
#define  DOCUMENTSERVICE_WRITER_WEB     _FILTER_CONFIG_FROM_ASCII_("com.sun.star.text.WebDocument"                 )
#define  DOCUMENTSERVICE_WRITER_GLOBAL  _FILTER_CONFIG_FROM_ASCII_("com.sun.star.text.GlobalDocument"              )
#define  DOCUMENTSERVICE_CALC           _FILTER_CONFIG_FROM_ASCII_("com.sun.star.sheet.SpreadsheetDocument"        )
#define  DOCUMENTSERVICE_DRAW           _FILTER_CONFIG_FROM_ASCII_("com.sun.star.drawing.DrawingDocument"          )
#define  DOCUMENTSERVICE_IMPRESS        _FILTER_CONFIG_FROM_ASCII_("com.sun.star.presentation.PresentationDocument")
#define  DOCUMENTSERVICE_CHART          _FILTER_CONFIG_FROM_ASCII_("com.sun.star.chart2.ChartDocument"              )
#define  DOCUMENTSERVICE_MATH           _FILTER_CONFIG_FROM_ASCII_("com.sun.star.formula.FormulaProperties"        )

/** @short  some uno service names.
 */
#define  SERVICE_CONFIGURATIONPROVIDER      _FILTER_CONFIG_FROM_ASCII_("com.sun.star.configuration.ConfigurationProvider"     )
#define  SERVICE_CONFIGURATIONUPDATEACCESS  _FILTER_CONFIG_FROM_ASCII_("com.sun.star.configuration.ConfigurationUpdateAccess" )
#define  SERVICE_CONFIGURATIONACCESS        _FILTER_CONFIG_FROM_ASCII_("com.sun.star.configuration.ConfigurationAccess"       )
#define  SERVICE_URLTRANSFORMER             _FILTER_CONFIG_FROM_ASCII_("com.sun.star.util.URLTransformer"                     )
#define  SERVICE_FILTERCONFIGREFRESH        _FILTER_CONFIG_FROM_ASCII_("com.sun.star.document.FilterConfigRefresh"            )

/** @short  some configuration pathes.
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
#define  DEFAULT_FORMATNAME         _FILTER_CONFIG_FROM_ASCII_("OpenOffice")
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
