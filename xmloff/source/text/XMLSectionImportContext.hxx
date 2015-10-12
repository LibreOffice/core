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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLSECTIONIMPORTCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLSECTIONIMPORTCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

namespace com { namespace sun { namespace star {
    namespace text { class XTextRange;  }
    namespace beans { class XPropertySet; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }


/**
 * Import text sections.
 *
 * This context may *also* be used for index header sections. The
 * differentiates its behaviour based on GetLocalName().
 */
class XMLSectionImportContext : public SvXMLImportContext
{
    /// start position; ranges acquired via getStart(),getEnd() don't move
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

    OUString sXmlId;
    OUString sStyleName;
    OUString sName;
    OUString sCond;
    ::com::sun::star::uno::Sequence<sal_Int8> aSequence;
    bool bProtect;
    bool bCondOK;
    bool bIsVisible;
    bool bValid;
    bool bSequenceOK;
    bool bIsCurrentlyVisible;
    bool bIsCurrentlyVisibleOK;

    bool bHasContent;

public:

    TYPEINFO_OVERRIDE();

    XMLSectionImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName );

    virtual ~XMLSectionImportContext();

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList) override;

    virtual void EndElement() override;

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList ) override;

    void ProcessAttributes(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
