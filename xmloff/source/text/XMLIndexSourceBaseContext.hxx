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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXSOURCEBASECONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXSOURCEBASECONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }

enum IndexSourceParamEnum
{
    XML_TOK_INDEXSOURCE_OUTLINE_LEVEL,
    XML_TOK_INDEXSOURCE_USE_INDEX_MARKS,
    XML_TOK_INDEXSOURCE_INDEX_SCOPE,
    XML_TOK_INDEXSOURCE_RELATIVE_TABS,
    XML_TOK_INDEXSOURCE_USE_OTHER_OBJECTS,
    XML_TOK_INDEXSOURCE_USE_SHEET,
    XML_TOK_INDEXSOURCE_USE_CHART,
    XML_TOK_INDEXSOURCE_USE_DRAW,
    XML_TOK_INDEXSOURCE_USE_IMAGE,
    XML_TOK_INDEXSOURCE_USE_MATH,
    XML_TOK_INDEXSOURCE_MAIN_ENTRY_STYLE,
    XML_TOK_INDEXSOURCE_IGNORE_CASE,
    XML_TOK_INDEXSOURCE_SEPARATORS,
    XML_TOK_INDEXSOURCE_COMBINE_ENTRIES,
    XML_TOK_INDEXSOURCE_COMBINE_WITH_DASH,
    XML_TOK_INDEXSOURCE_KEYS_AS_ENTRIES,
    XML_TOK_INDEXSOURCE_COMBINE_WITH_PP,
    XML_TOK_INDEXSOURCE_CAPITALIZE,
    XML_TOK_INDEXSOURCE_USE_OBJECTS,
    XML_TOK_INDEXSOURCE_USE_GRAPHICS,
    XML_TOK_INDEXSOURCE_USE_TABLES,
    XML_TOK_INDEXSOURCE_USE_FRAMES,
    XML_TOK_INDEXSOURCE_COPY_OUTLINE_LEVELS,
    XML_TOK_INDEXSOURCE_USE_CAPTION,
    XML_TOK_INDEXSOURCE_SEQUENCE_NAME,
    XML_TOK_INDEXSOURCE_SEQUENCE_FORMAT,
    XML_TOK_INDEXSOURCE_COMMA_SEPARATED,
    XML_TOK_INDEXSOURCE_USE_INDEX_SOURCE_STYLES,
    XML_TOK_INDEXSOURCE_SORT_ALGORITHM,
    XML_TOK_INDEXSOURCE_RFC_LANGUAGE_TAG,
    XML_TOK_INDEXSOURCE_LANGUAGE,
    XML_TOK_INDEXSOURCE_SCRIPT,
    XML_TOK_INDEXSOURCE_COUNTRY,
    XML_TOK_INDEXSOURCE_USER_INDEX_NAME,
    XML_TOK_INDEXSOURCE_USE_OUTLINE_LEVEL
};


/**
 * Superclass for index source elements
 */
class XMLIndexSourceBaseContext : public SvXMLImportContext
{
    const OUString sCreateFromChapter;
    const OUString sIsRelativeTabstops;

    bool bUseLevelFormats;

    bool bChapterIndex;     /// chapter-wise or document index?
    bool bRelativeTabs;     /// tab stops relative to margin or indent?

protected:

    /// property set of index; must be accessible to subclasses
    css::uno::Reference<css::beans::XPropertySet> & rIndexPropertySet;

public:


    XMLIndexSourceBaseContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        css::uno::Reference<css::beans::XPropertySet> & rPropSet,
        bool bLevelFormats);

    virtual ~XMLIndexSourceBaseContext();

protected:

    virtual void StartElement(
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList) override;

    virtual void ProcessAttribute(
        enum IndexSourceParamEnum eParam,
        const OUString& rValue);

    virtual void EndElement() override;

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
