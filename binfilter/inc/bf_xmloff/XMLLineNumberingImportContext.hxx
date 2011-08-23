/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XMLOFF_XMLLINENUMBERINGIMPORTCONTEXT_HXX_
#define _XMLOFF_XMLLINENUMBERINGIMPORTCONTEXT_HXX_

#include "xmlstyle.hxx"

#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace binfilter {


enum LineNumberingToken 
{
    XML_TOK_LINENUMBERING_STYLE_NAME,
    XML_TOK_LINENUMBERING_NUMBER_LINES,
    XML_TOK_LINENUMBERING_COUNT_EMPTY_LINES,
    XML_TOK_LINENUMBERING_COUNT_IN_FLOATING_FRAMES,
    XML_TOK_LINENUMBERING_RESTART_NUMBERING,
    XML_TOK_LINENUMBERING_OFFSET,
    XML_TOK_LINENUMBERING_NUM_FORMAT,
    XML_TOK_LINENUMBERING_NUM_LETTER_SYNC,
    XML_TOK_LINENUMBERING_NUMBER_POSITION,
    XML_TOK_LINENUMBERING_INCREMENT
//	XML_TOK_LINENUMBERING_LINENUMBERING_CONFIGURATION,
//	XML_TOK_LINENUMBERING_INCREMENT,
//	XML_TOK_LINENUMBERING_LINENUMBERING_SEPARATOR,
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

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
