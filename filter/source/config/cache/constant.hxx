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
#pragma once

#include <rtl/ustring.hxx>

/** @short  used to identify a some generic item properties against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
inline constexpr OUString PROPNAME_NAME = u"Name"_ustr;

/** @short  used to identify a type item property against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
inline constexpr OUString PROPNAME_UINAME = u"UIName"_ustr;
inline constexpr OUString PROPNAME_UINAMES = u"UINames"_ustr;
inline constexpr OUString PROPNAME_PREFERRED = u"Preferred"_ustr;
inline constexpr OUString PROPNAME_PREFERREDFILTER = u"PreferredFilter"_ustr;
inline constexpr OUString PROPNAME_DETECTSERVICE = u"DetectService"_ustr;
inline constexpr OUString PROPNAME_MEDIATYPE = u"MediaType"_ustr;
inline constexpr OUString PROPNAME_CLIPBOARDFORMAT = u"ClipboardFormat"_ustr;
inline constexpr OUString PROPNAME_URLPATTERN = u"URLPattern"_ustr;
inline constexpr OUString PROPNAME_EXTENSIONS = u"Extensions"_ustr;

/** @short  used to identify a filter item property against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
inline constexpr OUString PROPNAME_TYPE = u"Type"_ustr;
inline constexpr OUString PROPNAME_DOCUMENTSERVICE = u"DocumentService"_ustr;
inline constexpr OUString PROPNAME_FILTERSERVICE = u"FilterService"_ustr;
inline constexpr OUString PROPNAME_UICOMPONENT = u"UIComponent"_ustr;
inline constexpr OUString PROPNAME_FLAGS = u"Flags"_ustr;
inline constexpr OUString PROPNAME_USERDATA = u"UserData"_ustr;
inline constexpr OUString PROPNAME_TEMPLATENAME = u"TemplateName"_ustr;
inline constexpr OUString PROPNAME_FILEFORMATVERSION = u"FileFormatVersion"_ustr;
inline constexpr OUString PROPNAME_EXPORTEXTENSION = u"ExportExtension"_ustr;
inline constexpr OUString PROPNAME_ENABLED = u"Enabled"_ustr;

/** @short  used to identify a frame loader or detect service item
            property against the configuration API and can be used
            at all name containers (based on this filtercache) too.
 */
inline constexpr OUString PROPNAME_TYPES = u"Types"_ustr;

/** @short  used to identify the list of sorted filters for a specific
            office module
 */
inline constexpr OUString PROPNAME_SORTEDFILTERLIST = u"SortedFilterList"_ustr;

/** @short  implicit properties. which are used at the container interface only.
 */
inline constexpr OUString PROPNAME_FINALIZED = u"Finalized"_ustr;
inline constexpr OUString PROPNAME_MANDATORY = u"Mandatory"_ustr;

/** @short  used to identify a set of items against the configuration API. */
inline constexpr OUString CFGSET_TYPES = u"Types"_ustr;
inline constexpr OUString CFGSET_FILTERS = u"Filters"_ustr;
inline constexpr OUString CFGSET_FRAMELOADERS = u"FrameLoaders"_ustr;
inline constexpr OUString CFGSET_CONTENTHANDLERS = u"ContentHandlers"_ustr;

/** @short  used to address some configuration keys directly.

    @descr  Such direct keys should be used with function
            FilterCache::impl_getDirectCFGValue() only!

    @TODO   define these direct keys ...
 */
inline constexpr OUString CFGDIRECTKEY_OFFICELOCALE = u"/org.openoffice.Setup/L10N/ooLocale"_ustr;
inline constexpr OUString CFGDIRECTKEY_DEFAULTFRAMELOADER = u"/org.openoffice.TypeDetection.Misc/Defaults/DefaultFrameLoader"_ustr;

// Note that these flag bits have parallel names in
// comphelper/inc/comphelper/documentconstants.hxx . See that file for
// documentation on their meaning.

/** @short  names of filter flags, sorted in alphabetical order */
inline constexpr OUString FLAGNAME_3RDPARTYFILTER = u"3RDPARTYFILTER"_ustr;
inline constexpr OUString FLAGNAME_ALIEN = u"ALIEN"_ustr;
inline constexpr OUString FLAGNAME_CONSULTSERVICE = u"CONSULTSERVICE"_ustr;
inline constexpr OUString FLAGNAME_DEFAULT = u"DEFAULT"_ustr;
inline constexpr OUString FLAGNAME_ENCRYPTION = u"ENCRYPTION"_ustr;
inline constexpr OUString FLAGNAME_EXPORT = u"EXPORT"_ustr;
inline constexpr OUString FLAGNAME_GPGENCRYPTION = u"GPGENCRYPTION"_ustr;
inline constexpr OUString FLAGNAME_IMPORT = u"IMPORT"_ustr;
inline constexpr OUString FLAGNAME_INTERNAL = u"INTERNAL"_ustr;
inline constexpr OUString FLAGNAME_NOTINFILEDIALOG = u"NOTINFILEDIALOG"_ustr;
inline constexpr OUString FLAGNAME_NOTINSTALLED = u"NOTINSTALLED"_ustr;
inline constexpr OUString FLAGNAME_OWN = u"OWN"_ustr;
inline constexpr OUString FLAGNAME_PACKED = u"PACKED"_ustr;
inline constexpr OUString FLAGNAME_PASSWORDTOMODIFY = u"PASSWORDTOMODIFY"_ustr;
inline constexpr OUString FLAGNAME_PREFERRED = u"PREFERRED"_ustr;
inline constexpr OUString FLAGNAME_STARTPRESENTATION = u"STARTPRESENTATION"_ustr;
inline constexpr OUString FLAGNAME_READONLY = u"READONLY"_ustr;
inline constexpr OUString FLAGNAME_SUPPORTSSELECTION = u"SUPPORTSSELECTION"_ustr;
inline constexpr OUString FLAGNAME_TEMPLATE = u"TEMPLATE"_ustr;
inline constexpr OUString FLAGNAME_TEMPLATEPATH = u"TEMPLATEPATH"_ustr;
inline constexpr OUString FLAGNAME_COMBINED = u"COMBINED"_ustr;
inline constexpr OUString FLAGNAME_SUPPORTSSIGNING = u"SUPPORTSSIGNING"_ustr;
inline constexpr OUString FLAGNAME_EXOTIC = u"EXOTIC"_ustr;

/** @short  some uno service names.
 */
inline constexpr OUString SERVICE_CONFIGURATIONUPDATEACCESS = u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr;
inline constexpr OUString SERVICE_CONFIGURATIONACCESS = u"com.sun.star.configuration.ConfigurationAccess"_ustr;

/** @short  some configuration paths.
 */
inline constexpr OUString CFGPACKAGE_TD_TYPES = u"/org.openoffice.TypeDetection.Types"_ustr;
inline constexpr OUString CFGPACKAGE_TD_FILTERS = u"/org.openoffice.TypeDetection.Filter"_ustr;
inline constexpr OUString CFGPACKAGE_TD_OTHERS = u"/org.openoffice.TypeDetection.Misc"_ustr;
inline constexpr OUString CFGPACKAGE_TD_OLD = u"/org.openoffice.Office.TypeDetection"_ustr;

/** @short  some default values.
 */
inline constexpr OUString DEFAULT_OFFICELOCALE = u"en-US"_ustr;

/** @short  used for the queries of the FilterFactory service.
 */
inline constexpr OUString QUERY_IDENTIFIER_MATCHBYDOCUMENTSERVICE = u"matchByDocumentService"_ustr;
inline constexpr OUString QUERY_IDENTIFIER_GETPREFERREDFILTERFORTYPE = u"getDefaultFilterForType"_ustr;
inline constexpr OUString QUERY_IDENTIFIER_GET_SORTED_FILTERLIST = u"getSortedFilterList()"_ustr;

inline constexpr OUString QUERY_PARAM_IFLAGS = u"iflags"_ustr;
inline constexpr OUString QUERY_PARAM_EFLAGS = u"eflags"_ustr;
inline constexpr OUString QUERY_PARAM_MODULE = u"module"_ustr;
#define  QUERY_CONSTVALUE_ALL                       "all"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
