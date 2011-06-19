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


#ifndef _XMLOFF_XMLTEXTMARKIMPORTCONTEXT_HXX
#define _XMLOFF_XMLTEXTMARKIMPORTCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>


namespace com { namespace sun { namespace star {
    namespace text {
        class XTextRange;
        class XTextContent;
    }
    namespace xml { namespace sax {
        class XAttributeList;
    } }
} } }
namespace rtl {
    class OUString;
}
class XMLTextImportHelper;

class XMLFieldParamImportContext : public SvXMLImportContext
{
    XMLTextImportHelper& rHelper;
public:
    XMLFieldParamImportContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rHlp,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName );

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);
};


/**
 * import bookmarks and reference marks
 * ( <bookmark>, <bookmark-start>, <bookmark-end>,
 *   <reference>, <reference-start>, <reference-end> )
 *
 * All elements are handled by the same class due to their similarities.
 */
class XMLTextMarkImportContext : public SvXMLImportContext
{

    XMLTextImportHelper & m_rHelper;
    ::rtl::OUString m_sBookmarkName;
    ::rtl::OUString m_sFieldName;
    ::rtl::OUString m_sXmlId;
    // RDFa
    bool m_bHaveAbout;
    ::rtl::OUString m_sAbout;
    ::rtl::OUString m_sProperty;
    ::rtl::OUString m_sContent;
    ::rtl::OUString m_sDatatype;

public:

    TYPEINFO();

    XMLTextMarkImportContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rHlp,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName );

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);
    virtual void EndElement();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

public:
    static ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XTextContent > CreateAndInsertMark(
        SvXMLImport& rImport,
        const ::rtl::OUString& sServiceName,
        const ::rtl::OUString& sMarkName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XTextRange> & rRange,
        const ::rtl::OUString& i_rXmlId = ::rtl::OUString());

    sal_Bool FindName(
        SvXMLImport& rImport,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
