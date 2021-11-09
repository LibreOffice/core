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
constexpr OUStringLiteral PROPNAME_NAME = u"Name";

/** @short  used to identify a type item property against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
constexpr OUStringLiteral PROPNAME_UINAME = u"UIName";
constexpr OUStringLiteral PROPNAME_UINAMES = u"UINames";
constexpr OUStringLiteral PROPNAME_PREFERRED = u"Preferred";
constexpr OUStringLiteral PROPNAME_PREFERREDFILTER = u"PreferredFilter";
constexpr OUStringLiteral PROPNAME_DETECTSERVICE = u"DetectService";
constexpr OUStringLiteral PROPNAME_MEDIATYPE = u"MediaType";
constexpr OUStringLiteral PROPNAME_CLIPBOARDFORMAT = u"ClipboardFormat";
constexpr OUStringLiteral PROPNAME_URLPATTERN = u"URLPattern";
constexpr OUStringLiteral PROPNAME_EXTENSIONS = u"Extensions";

/** @short  used to identify a filter item property against the
            configuration API and can be used at all name containers
            (based on this filtercache) too.
 */
constexpr OUStringLiteral PROPNAME_TYPE = u"Type";
constexpr OUStringLiteral PROPNAME_DOCUMENTSERVICE = u"DocumentService";
constexpr OUStringLiteral PROPNAME_FILTERSERVICE = u"FilterService";
constexpr OUStringLiteral PROPNAME_UICOMPONENT = u"UIComponent";
constexpr OUStringLiteral PROPNAME_FLAGS = u"Flags";
constexpr OUStringLiteral PROPNAME_USERDATA = u"UserData";
constexpr OUStringLiteral PROPNAME_TEMPLATENAME = u"TemplateName";
constexpr OUStringLiteral PROPNAME_FILEFORMATVERSION = u"FileFormatVersion";
#define  PROPNAME_EXPORTEXTENSION   "ExportExtension"
#define  PROPNAME_ENABLED           "Enabled"

/** @short  used to identify a frame loader or detect service item
            property against the configuration API and can be used
            at all name containers (based on this filtercache) too.
 */
constexpr OUStringLiteral PROPNAME_TYPES = u"Types";

/** @short  used to identify the list of sorted filters for a specific
            office module
 */
constexpr OUStringLiteral PROPNAME_SORTEDFILTERLIST = u"SortedFilterList";

/** @short  implicit properties. which are used at the container interface only.
 */
constexpr OUStringLiteral PROPNAME_FINALIZED = u"Finalized";
constexpr OUStringLiteral PROPNAME_MANDATORY = u"Mandatory";

/** @short  used to identify a set of items against the configuration API. */
constexpr OUStringLiteral CFGSET_TYPES = u"Types";
constexpr OUStringLiteral CFGSET_FILTERS = u"Filters";
constexpr OUStringLiteral CFGSET_FRAMELOADERS = u"FrameLoaders";
constexpr OUStringLiteral CFGSET_CONTENTHANDLERS = u"ContentHandlers";

/** @short  used to address some configuration keys directly.

    @descr  Such direct keys should be used with function
            FilterCache::impl_getDirectCFGValue() only!

    @TODO   define these direct keys ...
 */
constexpr OUStringLiteral CFGDIRECTKEY_OFFICELOCALE = u"/org.openoffice.Setup/L10N/ooLocale";
constexpr OUStringLiteral CFGDIRECTKEY_DEFAULTFRAMELOADER = u"/org.openoffice.TypeDetection.Misc/Defaults/DefaultFrameLoader";
#define  CFGDIRECTKEY_PRODUCTNAME           "/org.openoffice.Setup/Product/ooName"

// Note that these flag bits have parallel names in
// comphelper/inc/comphelper/documentconstants.hxx . See that file for
// documentation on their meaning.

/** @short  names of filter flags, sorted in alphabetical order */
#define  FLAGNAME_3RDPARTYFILTER    "3RDPARTYFILTER"
#define  FLAGNAME_ALIEN             "ALIEN"
#define  FLAGNAME_CONSULTSERVICE    "CONSULTSERVICE"
#define  FLAGNAME_DEFAULT           "DEFAULT"
#define  FLAGNAME_ENCRYPTION        "ENCRYPTION"
#define  FLAGNAME_EXPORT            "EXPORT"
#define  FLAGNAME_GPGENCRYPTION     "GPGENCRYPTION"
#define  FLAGNAME_IMPORT            "IMPORT"
#define  FLAGNAME_INTERNAL          "INTERNAL"
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
#define  FLAGNAME_COMBINED          "COMBINED"
#define FLAGNAME_SUPPORTSSIGNING "SUPPORTSSIGNING"
#define FLAGNAME_EXOTIC "EXOTIC"

/** @short  some uno service names.
 */
constexpr OUStringLiteral SERVICE_CONFIGURATIONUPDATEACCESS = u"com.sun.star.configuration.ConfigurationUpdateAccess";
constexpr OUStringLiteral SERVICE_CONFIGURATIONACCESS = u"com.sun.star.configuration.ConfigurationAccess";

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
constexpr OUStringLiteral QUERY_IDENTIFIER_MATCHBYDOCUMENTSERVICE = u"matchByDocumentService";
constexpr OUStringLiteral QUERY_IDENTIFIER_GETPREFERREDFILTERFORTYPE = u"getDefaultFilterForType";
constexpr OUStringLiteral QUERY_IDENTIFIER_GET_SORTED_FILTERLIST = u"getSortedFilterList()";

constexpr OUStringLiteral QUERY_PARAM_IFLAGS = u"iflags";
constexpr OUStringLiteral QUERY_PARAM_EFLAGS = u"eflags";
constexpr OUStringLiteral QUERY_PARAM_MODULE = u"module";
#define  QUERY_PARAM_DEFAULTFIRST                   "default_first"
#define  QUERY_CONSTVALUE_ALL                       "all"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
