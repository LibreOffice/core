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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXTEMPLATECONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXTEMPLATECONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmltoken.hxx>

#include <vector>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValues.hpp>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }
struct SvXMLEnumMapEntry;


// constants for the XMLIndexTemplateContext constructor

// TOC and user defined index:
extern const SvXMLEnumMapEntry aSvLevelNameTOCMap[];
extern const sal_Char* aLevelStylePropNameTOCMap[];
extern const sal_Bool aAllowedTokenTypesTOC[];
extern const sal_Bool aAllowedTokenTypesUser[];

// alphabetical index:
extern const SvXMLEnumMapEntry aLevelNameAlphaMap[];
extern const sal_Char* aLevelStylePropNameAlphaMap[];
extern const sal_Bool aAllowedTokenTypesAlpha[];

// bibliography:
extern const SvXMLEnumMapEntry aLevelNameBibliographyMap[];
extern const sal_Char* aLevelStylePropNameBibliographyMap[];
extern const sal_Bool aAllowedTokenTypesBibliography[];

// table, illustration and object tables:
extern const SvXMLEnumMapEntry* aLevelNameTableMap; // NULL: no outline-level
extern const sal_Char* aLevelStylePropNameTableMap[];
extern const sal_Bool aAllowedTokenTypesTable[];


/**
 * Import index entry templates
 */
class XMLIndexTemplateContext : public SvXMLImportContext
{
    // pick up PropertyValues to be turned into a sequence.
    ::std::vector< css::beans::PropertyValues > aValueVector;

    OUString sStyleName;

    const SvXMLEnumMapEntry* pOutlineLevelNameMap;
    enum ::xmloff::token::XMLTokenEnum eOutlineLevelAttrName;
    const sal_Char** pOutlineLevelStylePropMap;
    const sal_Bool* pAllowedTokenTypesMap;

    sal_Int32 nOutlineLevel;
    bool bStyleNameOK;
    bool bOutlineLevelOK;
    bool bTOC;

    // PropertySet of current index
    css::uno::Reference<css::beans::XPropertySet> & rPropertySet;

public:

    // constants made available to other contexts (template entry
    // contexts, in particular)
    const OUString sTokenEntryNumber;
    const OUString sTokenEntryText;
    const OUString sTokenTabStop;
    const OUString sTokenText;
    const OUString sTokenPageNumber;
    const OUString sTokenChapterInfo;
    const OUString sTokenHyperlinkStart;
    const OUString sTokenHyperlinkEnd;
    const OUString sTokenBibliographyDataField;

    const OUString sCharacterStyleName;
    const OUString sTokenType;
    const OUString sText;
    const OUString sTabStopRightAligned;
    const OUString sTabStopPosition;
    const OUString sTabStopFillCharacter;
    const OUString sBibliographyDataField;
    const OUString sChapterFormat;
    const OUString sChapterLevel;//i53420

    const OUString sLevelFormat;



    XMLIndexTemplateContext(
        SvXMLImport& rImport,
        css::uno::Reference<css::beans::XPropertySet> & rPropSet,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const SvXMLEnumMapEntry* aLevelNameMap,
        enum ::xmloff::token::XMLTokenEnum eLevelAttrName,
        const sal_Char** aLevelStylePropNameMap,
        const sal_Bool* aAllowedTokenTypes,
        bool bTOC=false);

    virtual ~XMLIndexTemplateContext();

    /** add template; to be called by child template entry contexts */
    void addTemplateEntry(
        const css::beans::PropertyValues& aValues);

protected:

    virtual void StartElement(
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList) override;

    virtual void EndElement() override;

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
