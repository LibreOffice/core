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
#include <com/sun/star/beans/PropertyValues.hpp>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }
template<typename EnumT> struct SvXMLEnumMapEntry;


// constants for the XMLIndexTemplateContext constructor

// TOC and user defined index:
extern const SvXMLEnumMapEntry<sal_uInt16> aSvLevelNameTOCMap[];
extern const sal_Char* aLevelStylePropNameTOCMap[];
extern const bool aAllowedTokenTypesTOC[];
extern const bool aAllowedTokenTypesUser[];

// alphabetical index:
extern const SvXMLEnumMapEntry<sal_uInt16> aLevelNameAlphaMap[];
extern const sal_Char* aLevelStylePropNameAlphaMap[];
extern const bool aAllowedTokenTypesAlpha[];

// bibliography:
extern const SvXMLEnumMapEntry<sal_uInt16> aLevelNameBibliographyMap[];
extern const sal_Char* aLevelStylePropNameBibliographyMap[];
extern const bool aAllowedTokenTypesBibliography[];

// table, illustration and object tables:
extern const SvXMLEnumMapEntry<sal_uInt16>* aLevelNameTableMap; // NULL: no outline-level
extern const sal_Char* aLevelStylePropNameTableMap[];
extern const bool aIllustrationAllowedTokenTypesTable[];
extern const bool aAllowedTokenTypesTable[];


/**
 * Import index entry templates
 */
class XMLIndexTemplateContext : public SvXMLImportContext
{
    // pick up PropertyValues to be turned into a sequence.
    ::std::vector< css::beans::PropertyValues > aValueVector;

    OUString sStyleName;

    const SvXMLEnumMapEntry<sal_uInt16>* pOutlineLevelNameMap;
    enum ::xmloff::token::XMLTokenEnum eOutlineLevelAttrName;
    const sal_Char** pOutlineLevelStylePropMap;
    const bool* pAllowedTokenTypesMap;

    sal_Int32 nOutlineLevel;
    bool bStyleNameOK;
    bool bOutlineLevelOK;
    bool bTOC;

    // PropertySet of current index
    css::uno::Reference<css::beans::XPropertySet> & rPropertySet;

public:
    template<typename EnumT>
    XMLIndexTemplateContext(
        SvXMLImport& rImport,
        css::uno::Reference<css::beans::XPropertySet> & rPropSet,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const SvXMLEnumMapEntry<EnumT>* aLevelNameMap,
        enum ::xmloff::token::XMLTokenEnum eLevelAttrName,
        const sal_Char** aLevelStylePropNameMap,
        const bool* aAllowedTokenTypes,
        bool bTOC_=false)
        : XMLIndexTemplateContext(rImport,rPropSet,nPrfx,rLocalName,
                reinterpret_cast<const SvXMLEnumMapEntry<sal_uInt16>*>(aLevelNameMap),
                eLevelAttrName, aLevelStylePropNameMap, aAllowedTokenTypes, bTOC_) {}
    XMLIndexTemplateContext(
        SvXMLImport& rImport,
        css::uno::Reference<css::beans::XPropertySet> & rPropSet,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const SvXMLEnumMapEntry<sal_uInt16>* aLevelNameMap,
        enum ::xmloff::token::XMLTokenEnum eLevelAttrName,
        const sal_Char** aLevelStylePropNameMap,
        const bool* aAllowedTokenTypes,
        bool bTOC);

    virtual ~XMLIndexTemplateContext() override;

    /** add template; to be called by child template entry contexts */
    void addTemplateEntry(
        const css::beans::PropertyValues& aValues);

protected:

    virtual void StartElement(
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList) override;

    virtual void EndElement() override;

    virtual SvXMLImportContextRef CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
