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

/*  disable impl_loadOnDemand function of BaseContainer for certain
    functions, where it the feature "impl_loadItemOnDemand() of class FilterCache
    can be used instead of loadAll()!*/
// #define LOAD_IMPLICIT


/** @short  used to identify a some generic item properties against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
inline constexpr OUStringLiteral PROPNAME_NAME = u"Name";

/** @short  used to identify a type item property against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
inline constexpr OUStringLiteral PROPNAME_UINAME = u"UIName";
inline constexpr OUStringLiteral PROPNAME_UINAMES = u"UINames";
inline constexpr OUStringLiteral PROPNAME_PREFERRED = u"Preferred";
inline constexpr OUStringLiteral PROPNAME_PREFERREDFILTER = u"PreferredFilter";
inline constexpr OUStringLiteral PROPNAME_DETECTSERVICE = u"DetectService";
inline constexpr OUStringLiteral PROPNAME_MEDIATYPE = u"MediaType";
inline constexpr OUStringLiteral PROPNAME_CLIPBOARDFORMAT = u"ClipboardFormat";
inline constexpr OUStringLiteral PROPNAME_URLPATTERN = u"URLPattern";
inline constexpr OUStringLiteral PROPNAME_EXTENSIONS = u"Extensions";

/** @short  used to identify a filter item property against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
inline constexpr OUStringLiteral PROPNAME_TYPE = u"Type";
inline constexpr OUStringLiteral PROPNAME_DOCUMENTSERVICE = u"DocumentService";
inline constexpr OUStringLiteral PROPNAME_FILTERSERVICE = u"FilterService";
inline constexpr OUStringLiteral PROPNAME_UICOMPONENT = u"UIComponent";
inline constexpr OUStringLiteral PROPNAME_FLAGS = u"Flags";
inline constexpr OUStringLiteral PROPNAME_USERDATA = u"UserData";
inline constexpr OUStringLiteral PROPNAME_TEMPLATENAME = u"TemplateName";
inline constexpr OUStringLiteral PROPNAME_FILEFORMATVERSION = u"FileFormatVersion";
inline constexpr OUStringLiteral PROPNAME_EXPORTEXTENSION = u"ExportExtension";
inline constexpr OUStringLiteral PROPNAME_ENABLED = u"Enabled";

/** @short  used to identify a frame loader or detect service item
            property against the configuration API and can be used
            at all name containers (based on this filtercache) too.
 */
inline constexpr OUStringLiteral PROPNAME_TYPES = u"Types";

/** @short  used to identify the list of sorted filters for a specific
            office module
 */
inline constexpr OUStringLiteral PROPNAME_SORTEDFILTERLIST = u"SortedFilterList";

/** @short  implicit properties. which are used at the container interface only.
 */
inline constexpr OUStringLiteral PROPNAME_FINALIZED = u"Finalized";
inline constexpr OUStringLiteral PROPNAME_MANDATORY = u"Mandatory";

/** @short  used to identify a set of items against the configuration API. */
inline constexpr OUStringLiteral CFGSET_TYPES = u"Types";
inline constexpr OUStringLiteral CFGSET_FILTERS = u"Filters";
inline constexpr OUStringLiteral CFGSET_FRAMELOADERS = u"FrameLoaders";
inline constexpr OUStringLiteral CFGSET_CONTENTHANDLERS = u"ContentHandlers";

/** @short  used to address some configuration keys directly.

    @descr  Such direct keys should be used with function
            FilterCache::impl_getDirectCFGValue() only!

    @TODO   define these direct keys ...
 */
inline constexpr OUStringLiteral CFGDIRECTKEY_OFFICELOCALE = u"/org.openoffice.Setup/L10N/ooLocale";
inline constexpr OUStringLiteral CFGDIRECTKEY_DEFAULTFRAMELOADER = u"/org.openoffice.TypeDetection.Misc/Defaults/DefaultFrameLoader";
#define  CFGDIRECTKEY_PRODUCTNAME           "/org.openoffice.Setup/Product/ooName"

// Note that these flag bits have parallel names in
// comphelper/inc/comphelper/documentconstants.hxx . See that file for
// documentation on their meaning.

/** @short  names of filter flags, sorted in alphabetical order */
inline constexpr OUStringLiteral FLAGNAME_3RDPARTYFILTER = u"3RDPARTYFILTER";
inline constexpr OUStringLiteral FLAGNAME_ALIEN = u"ALIEN";
inline constexpr OUStringLiteral FLAGNAME_CONSULTSERVICE = u"CONSULTSERVICE";
inline constexpr OUStringLiteral FLAGNAME_DEFAULT = u"DEFAULT";
inline constexpr OUStringLiteral FLAGNAME_ENCRYPTION = u"ENCRYPTION";
inline constexpr OUStringLiteral FLAGNAME_EXPORT = u"EXPORT";
inline constexpr OUStringLiteral FLAGNAME_GPGENCRYPTION = u"GPGENCRYPTION";
inline constexpr OUStringLiteral FLAGNAME_IMPORT = u"IMPORT";
inline constexpr OUStringLiteral FLAGNAME_INTERNAL = u"INTERNAL";
inline constexpr OUStringLiteral FLAGNAME_NOTINFILEDIALOG = u"NOTINFILEDIALOG";
inline constexpr OUStringLiteral FLAGNAME_NOTINSTALLED = u"NOTINSTALLED";
inline constexpr OUStringLiteral FLAGNAME_OWN = u"OWN";
inline constexpr OUStringLiteral FLAGNAME_PACKED = u"PACKED";
inline constexpr OUStringLiteral FLAGNAME_PASSWORDTOMODIFY = u"PASSWORDTOMODIFY";
inline constexpr OUStringLiteral FLAGNAME_PREFERRED = u"PREFERRED";
inline constexpr OUStringLiteral FLAGNAME_STARTPRESENTATION = u"STARTPRESENTATION";
inline constexpr OUStringLiteral FLAGNAME_READONLY = u"READONLY";
inline constexpr OUStringLiteral FLAGNAME_SUPPORTSSELECTION = u"SUPPORTSSELECTION";
inline constexpr OUStringLiteral FLAGNAME_TEMPLATE = u"TEMPLATE";
inline constexpr OUStringLiteral FLAGNAME_TEMPLATEPATH = u"TEMPLATEPATH";
inline constexpr OUStringLiteral FLAGNAME_COMBINED = u"COMBINED";
inline constexpr OUStringLiteral FLAGNAME_SUPPORTSSIGNING = u"SUPPORTSSIGNING";
inline constexpr OUStringLiteral FLAGNAME_EXOTIC = u"EXOTIC";

/** @short  some uno service names.
 */
inline constexpr OUStringLiteral SERVICE_CONFIGURATIONUPDATEACCESS = u"com.sun.star.configuration.ConfigurationUpdateAccess";
inline constexpr OUStringLiteral SERVICE_CONFIGURATIONACCESS = u"com.sun.star.configuration.ConfigurationAccess";

/** @short  some configuration paths.
 */
inline constexpr OUStringLiteral CFGPACKAGE_TD_TYPES = u"/org.openoffice.TypeDetection.Types";
inline constexpr OUStringLiteral CFGPACKAGE_TD_FILTERS = u"/org.openoffice.TypeDetection.Filter";
inline constexpr OUStringLiteral CFGPACKAGE_TD_OTHERS = u"/org.openoffice.TypeDetection.Misc";
inline constexpr OUStringLiteral CFGPACKAGE_TD_OLD = u"/org.openoffice.Office.TypeDetection";

/** @short  some default values.
 */
inline constexpr OUStringLiteral DEFAULT_OFFICELOCALE = u"en-US";

/** @short  used for the queries of the FilterFactory service.
 */
inline constexpr OUStringLiteral QUERY_IDENTIFIER_MATCHBYDOCUMENTSERVICE = u"matchByDocumentService";
inline constexpr OUStringLiteral QUERY_IDENTIFIER_GETPREFERREDFILTERFORTYPE = u"getDefaultFilterForType";
inline constexpr OUStringLiteral QUERY_IDENTIFIER_GET_SORTED_FILTERLIST = u"getSortedFilterList()";

inline constexpr OUStringLiteral QUERY_PARAM_IFLAGS = u"iflags";
inline constexpr OUStringLiteral QUERY_PARAM_EFLAGS = u"eflags";
inline constexpr OUStringLiteral QUERY_PARAM_MODULE = u"module";
#define  QUERY_PARAM_DEFAULTFIRST                   "default_first"
#define  QUERY_CONSTVALUE_ALL                       "all"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
