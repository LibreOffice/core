/*************************************************************************
 *
 *  $RCSfile: constant.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-20 09:28:48 $
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
 *  This library is free software you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License") You may not use this file
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
#define  PROPNAME_NAME  _FILTER_CONFIG_FROM_ASCII_("Name")

/** @short  used to identify a type item property against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
#define  PROPNAME_UINAME            _FILTER_CONFIG_FROM_ASCII_("UIName"          )
#define  PROPNAME_UINAMES           _FILTER_CONFIG_FROM_ASCII_("UINames"         )
#define  PROPNAME_PREFERRED         _FILTER_CONFIG_FROM_ASCII_("Preferred"       )
#define  PROPNAME_PREFERREDFILTER   _FILTER_CONFIG_FROM_ASCII_("PreferredFilter" )
#define  PROPNAME_DETECTSERVICE     _FILTER_CONFIG_FROM_ASCII_("DetectService"   )
#define  PROPNAME_MEDIATYPE         _FILTER_CONFIG_FROM_ASCII_("MediaType"       )
#define  PROPNAME_CLIPBOARDFORMAT   _FILTER_CONFIG_FROM_ASCII_("ClipboardFormat" )
#define  PROPNAME_URLPATTERN        _FILTER_CONFIG_FROM_ASCII_("URLPattern"      )
#define  PROPNAME_EXTENSIONS        _FILTER_CONFIG_FROM_ASCII_("Extensions"      )

/** @short  used to identify a filter item property against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
#define  PROPNAME_TYPE              _FILTER_CONFIG_FROM_ASCII_("Type"             )
#define  PROPNAME_DOCUMENTSERVICE   _FILTER_CONFIG_FROM_ASCII_("DocumentService"  )
#define  PROPNAME_FILTERSERVICE     _FILTER_CONFIG_FROM_ASCII_("FilterService"    )
#define  PROPNAME_UICOMPONENT       _FILTER_CONFIG_FROM_ASCII_("UIComponent"      )
#define  PROPNAME_FLAGS             _FILTER_CONFIG_FROM_ASCII_("Flags"            )
#define  PROPNAME_USERDATA          _FILTER_CONFIG_FROM_ASCII_("UserData"         )
#define  PROPNAME_TEMPLATENAME      _FILTER_CONFIG_FROM_ASCII_("TemplateName"     )
#define  PROPNAME_FILEFORMATVERSION _FILTER_CONFIG_FROM_ASCII_("FileFormatVersion")

/** @short  used to identify a frame loader or detect service item
            property against the configuration API and can be used
            at all name containers (based on this filtercache) too.
 */
#define  PROPNAME_TYPES     _FILTER_CONFIG_FROM_ASCII_("Types")

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
#define  DOCUMENTSERVICE_CHART          _FILTER_CONFIG_FROM_ASCII_("com.sun.star.chart.ChartDocument"              )
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
