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

#ifndef _XMLOFF_XMLSECTIONIMPORTCONTEXT_HXX_
#define _XMLOFF_XMLSECTIONIMPORTCONTEXT_HXX_

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

namespace com { namespace sun { namespace star {
    namespace text { class XTextRange;  }
    namespace beans { class XPropertySet; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace rtl { class OUString; }


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

    const OUString sTextSection;
    const OUString sIndexHeaderSection;
    const OUString sCondition;
    const OUString sIsVisible;
    const OUString sProtectionKey;
    const OUString sIsProtected;
    const OUString sIsCurrentlyVisible;
    const OUString sEmpty;

    OUString sXmlId;
    OUString sStyleName;
    OUString sName;
    OUString sCond;
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
        const OUString& rLocalName );

    ~XMLSectionImportContext();

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual void EndElement();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );

    void ProcessAttributes(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
