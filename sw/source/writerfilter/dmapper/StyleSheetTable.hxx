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

#include <memory>
#include <rtl/ref.hxx>
#include "TblStylePrHandler.hxx"

#include "DomainMapper.hxx"
#include <com/sun/star/beans/PropertyValues.hpp>
#include "PropertyMap.hxx"
#include "FontTable.hxx"
#include "LoggedResources.hxx"

class SwXTextDocument;
namespace com::sun::star::text { class XTextDocument; }


namespace writerfilter::dmapper
{


enum StyleType
{
    STYLE_TYPE_UNKNOWN,
    STYLE_TYPE_PARA,
    STYLE_TYPE_CHAR,
    STYLE_TYPE_TABLE,
    STYLE_TYPE_LIST
};
class StyleSheetTable;
typedef tools::SvRef<StyleSheetTable> StyleSheetTablePtr;

struct StyleSheetTable_Impl;
class StyleSheetEntry : public virtual SvRefBase
{
    std::vector<css::beans::PropertyValue> m_aInteropGrabBag;
public:
    OUString m_sStyleIdentifierD;   // WW8 name
    bool            m_bIsDefaultStyle;
    bool            m_bAssignedAsChapterNumbering;
    bool            m_bInvalidHeight;
    bool            m_bHasUPE; //universal property expansion
    StyleType       m_nStyleTypeCode; //sgc
    OUString m_sBaseStyleIdentifier;
    OUString m_sNextStyleIdentifier;
    OUString m_sLinkStyleIdentifier;
    OUString m_sStyleName;
    const tools::SvRef<StyleSheetPropertyMap> m_pProperties;
    OUString m_sConvertedStyleName;
    std::vector<css::beans::PropertyValue> m_aLatentStyles; ///< Attributes of latentStyles
    std::vector<css::beans::PropertyValue> m_aLsdExceptions; ///< List of lsdException attribute lists
    bool           m_bAutoRedefine; ///< Writer calls this auto-update.

    void AppendInteropGrabBag(const css::beans::PropertyValue& rValue);
    css::beans::PropertyValue GetInteropGrabBag(); ///< Used for table styles, has a name.
    css::beans::PropertyValues GetInteropGrabBagSeq() const; ///< Used for existing styles, just a list of properties.

    // Get all properties, merged with the all of the parent's properties
    PropertyMapPtr GetMergedInheritedProperties(const StyleSheetTablePtr& pStyleSheetTable);

    StyleSheetEntry();
    StyleSheetEntry(StyleSheetEntry const&) = default;
    virtual ~StyleSheetEntry() override;
};

typedef tools::SvRef<StyleSheetEntry> StyleSheetEntryPtr;

class DomainMapper;
class StyleSheetTable :
        public LoggedProperties,
        public LoggedTable
{
    std::unique_ptr<StyleSheetTable_Impl> m_pImpl;

public:
    StyleSheetTable(DomainMapper& rDMapper, rtl::Reference<SwXTextDocument> const& xTextDocument, bool bIsNewDoc);
    virtual ~StyleSheetTable() override;

    void ReApplyInheritedOutlineLevelFromChapterNumbering();
    void ApplyNumberingStyleNameToParaStyles();
    void ApplyStyleSheets( const FontTablePtr& rFontTable );
    StyleSheetEntryPtr FindStyleSheetByISTD(const OUString& sIndex);
    StyleSheetEntryPtr FindStyleSheetByConvertedStyleName(std::u16string_view rIndex);
    StyleSheetEntryPtr FindDefaultParaStyle();

    OUString ConvertStyleName( const OUString& rWWName, bool bExtendedSearch = false );
    OUString CloneTOCStyle(FontTablePtr const& rFontTable, StyleSheetEntryPtr const pStyle, OUString const& rName);
    void ApplyClonedTOCStyles();

    OUString getOrCreateCharStyle( PropertyValueVector_t& rCharProperties, bool bAlwaysCreate );

    void SetDefaultParaProps(PropertyIds eId, const css::uno::Any& rAny);
    PropertyMapPtr const & GetDefaultParaProps() const;
    /// Returns the default character properties.
    PropertyMapPtr const & GetDefaultCharProps() const;

    const StyleSheetEntryPtr & GetCurrentEntry() const;

private:
    // Properties
    virtual void lcl_attribute(Id Name, Value & val) override;
    virtual void lcl_sprm(Sprm & sprm) override;

    // Table
    virtual void lcl_entry(const writerfilter::Reference<Properties>::Pointer_t& ref) override;

    void applyDefaults(bool bParaProperties);

    void ApplyStyleSheetsImpl(const FontTablePtr& rFontTable, std::vector<StyleSheetEntryPtr> const& rEntries);
};


class TableStyleSheetEntry :
    public StyleSheetEntry
{
public:
    // Adds a new tblStylePr to the table style entry. This method
    // fixes some possible properties conflicts, like borders ones.
    void AddTblStylePr( TblStyleType nType, const PropertyMapPtr& pProps );

    // Gets all the properties
    //     + corresponding to the mask,
    //     + from the parent styles

    // @param mask      mask describing which properties to return
    PropertyMapPtr GetProperties( sal_Int32 nMask);

    TableStyleSheetEntry( StyleSheetEntry const & aEntry );
    virtual ~TableStyleSheetEntry( ) override;

private:
    typedef std::map<TblStyleType, PropertyMapPtr> TblStylePrs;
    TblStylePrs m_aStyles;
    PropertyMapPtr GetLocalPropertiesFromMask( sal_Int32 nMask );
};


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
