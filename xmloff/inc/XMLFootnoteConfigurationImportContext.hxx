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

#ifndef INCLUDED_XMLOFF_INC_XMLFOOTNOTECONFIGURATIONIMPORTCONTEXT_HXX
#define INCLUDED_XMLOFF_INC_XMLFOOTNOTECONFIGURATIONIMPORTCONTEXT_HXX

#include <xmloff/xmlstyle.hxx>

namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Reference; }
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }
class SvXMLImport;

/// import footnote and endnote configuration elements
class XMLFootnoteConfigurationImportContext : public SvXMLStyleContext
{
    const OUString sPropertyAnchorCharStyleName;
    const OUString sPropertyCharStyleName;
    const OUString sPropertyNumberingType;
    const OUString sPropertyPageStyleName;
    const OUString sPropertyParagraphStyleName;
    const OUString sPropertyPrefix;
    const OUString sPropertyStartAt;
    const OUString sPropertySuffix;
    const OUString sPropertyPositionEndOfDoc;
    const OUString sPropertyFootnoteCounting;
    const OUString sPropertyEndNotice;
    const OUString sPropertyBeginNotice;

    OUString sCitationStyle;
    OUString sAnchorStyle;
    OUString sDefaultStyle;
    OUString sPageStyle;
    OUString sPrefix;
    OUString sSuffix;
    OUString sNumFormat;
    OUString sNumSync;
    OUString sBeginNotice;
    OUString sEndNotice;

    SvXMLTokenMap* pAttrTokenMap;

    sal_Int16 nOffset;
    sal_Int16 nNumbering;
    bool bPosition;
    bool bIsEndnote;

public:


    XMLFootnoteConfigurationImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const css::uno::Reference< css::xml::sax::XAttributeList> & xAttrList);

    virtual ~XMLFootnoteConfigurationImportContext();

    /// parse attributes
    virtual void StartElement(
        const css::uno::Reference< css::xml::sax::XAttributeList> & xAttrList ) override;

    /// for footnotes, also parse begin and end notices
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList> & xAttrList ) override;

    /// get token map for attributes
    const SvXMLTokenMap& GetFtnConfigAttrTokenMap();

    /// set configuration at document; calls ProcessSettings
    /* Move code from <CreateAndInsertLate(..)> to <Finish(..)>, because
       at this time all styles it references have been set. (#i40579#)
    */
    virtual void Finish( bool bOverwrite) override;

    /// set configuration at document
    void ProcessSettings(
        const css::uno::Reference< css::beans::XPropertySet> & rConfig);

    /// for helper class: set begin notice
    void SetBeginNotice( const OUString& sText);

    /// for helper class: set end notice
    void SetEndNotice( const OUString& sText);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
