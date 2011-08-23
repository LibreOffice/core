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

#ifndef _XMLOFF_XMLSECTIONIMPORTCONTEXT_HXX_
#define _XMLOFF_XMLSECTIONIMPORTCONTEXT_HXX_

#include "xmlictxt.hxx"

#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/uno/Sequence.h>

namespace com { namespace sun { namespace star {
    namespace text { class XTextRange;	}
    namespace beans { class XPropertySet; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace rtl {	class OUString; }
namespace binfilter {
class XMLTextImportHelper;


/**
 * Import text sections.
 *
 * This context may *also* be used for index header sections. The 
 * differentiates its behaviour based on GetLocalName().
 */
class XMLSectionImportContext : public SvXMLImportContext
{
    /// start position; ranges aquired via getStart(),getEnd() don't move
    ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XTextRange> xStartRange;

    /// end position
    ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XTextRange> xEndRange;

    /// TextSection (as XPropertySet) for passing down to data source elements
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> xSectionPropertySet;

    const ::rtl::OUString sTextSection;
    const ::rtl::OUString sIndexHeaderSection;
    const ::rtl::OUString sCondition;
    const ::rtl::OUString sIsVisible;
    const ::rtl::OUString sProtectionKey;
    const ::rtl::OUString sIsProtected;
    const ::rtl::OUString sIsCurrentlyVisible;
    const ::rtl::OUString sEmpty;

    ::rtl::OUString sStyleName;
    ::rtl::OUString sName;
    ::rtl::OUString sCond;
    ::com::sun::star::uno::Sequence<sal_Int8> aSequence;
    sal_Bool bProtect;
    sal_Bool bCondOK;
    sal_Bool bIsVisible;
    sal_Bool bValid;
    sal_Bool bSequenceOK;
    sal_Bool bIsCurrentlyVisible;
    sal_Bool bIsCurrentlyVisibleOK;

    sal_Bool bHasContent;

public:

    TYPEINFO();

    XMLSectionImportContext(
        SvXMLImport& rImport, 
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName );

    ~XMLSectionImportContext();

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual void EndElement();

    virtual SvXMLImportContext *CreateChildContext( 
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );

    void ProcessAttributes(
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
