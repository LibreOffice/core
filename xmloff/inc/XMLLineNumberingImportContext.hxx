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
#if 1

#include <xmloff/xmlstyle.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
} } }


enum LineNumberingToken
{
    XML_TOK_LINENUMBERING_STYLE_NAME,
    XML_TOK_LINENUMBERING_NUMBER_LINES,
    XML_TOK_LINENUMBERING_COUNT_EMPTY_LINES,
    XML_TOK_LINENUMBERING_COUNT_IN_TEXT_BOXES,
    XML_TOK_LINENUMBERING_RESTART_NUMBERING,
    XML_TOK_LINENUMBERING_OFFSET,
    XML_TOK_LINENUMBERING_NUM_FORMAT,
    XML_TOK_LINENUMBERING_NUM_LETTER_SYNC,
    XML_TOK_LINENUMBERING_NUMBER_POSITION,
    XML_TOK_LINENUMBERING_INCREMENT
//  XML_TOK_LINENUMBERING_LINENUMBERING_CONFIGURATION,
//  XML_TOK_LINENUMBERING_INCREMENT,
//  XML_TOK_LINENUMBERING_LINENUMBERING_SEPARATOR,
};


/** import <text:linenumbering-configuration> elements */
class XMLLineNumberingImportContext : public SvXMLStyleContext
{
    const ::rtl::OUString sCharStyleName;
    const ::rtl::OUString sCountEmptyLines;
    const ::rtl::OUString sCountLinesInFrames;
    const ::rtl::OUString sDistance;
    const ::rtl::OUString sInterval;
    const ::rtl::OUString sSeparatorText;
    const ::rtl::OUString sNumberPosition;
    const ::rtl::OUString sNumberingType;
    const ::rtl::OUString sIsOn;
    const ::rtl::OUString sRestartAtEachPage;
    const ::rtl::OUString sSeparatorInterval;

    ::rtl::OUString sStyleName;
    ::rtl::OUString sNumFormat;
    ::rtl::OUString sNumLetterSync;
    ::rtl::OUString sSeparator;
    sal_Int32 nOffset;
    sal_Int16 nNumberPosition;
    sal_Int16 nIncrement;
    sal_Int16 nSeparatorIncrement;
    sal_Bool bNumberLines;
    sal_Bool bCountEmptyLines;
    sal_Bool bCountInFloatingFrames;
    sal_Bool bRestartNumbering;

public:

    TYPEINFO();

    XMLLineNumberingImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    ~XMLLineNumberingImportContext();

    // to be used by child context: set separator info
    void SetSeparatorText(const ::rtl::OUString& sText);
    void SetSeparatorIncrement(sal_Int16 nIncr);

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    void ProcessAttribute(
        enum LineNumberingToken eToken,
        ::rtl::OUString sValue);

    virtual void CreateAndInsert(sal_Bool bOverwrite);

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );

    void ProcessAttribute(
        const ::rtl::OUString sLocalName,
        const ::rtl::OUString sValue);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
