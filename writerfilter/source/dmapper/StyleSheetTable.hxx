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
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_STYLESHEETTABLE_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_STYLESHEETTABLE_HXX

#include <memory>
#include "TblStylePrHandler.hxx"

#include <DomainMapper.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include "PropertyMap.hxx"
#include "FontTable.hxx"
#include "LoggedResources.hxx"

namespace com{ namespace sun { namespace star { namespace text{
    class XTextDocument;
}}}}


namespace writerfilter {
namespace dmapper
{


enum StyleType
{
    STYLE_TYPE_UNKNOWN,
    STYLE_TYPE_PARA,
    STYLE_TYPE_CHAR,
    STYLE_TYPE_TABLE,
    STYLE_TYPE_LIST
};

struct StyleSheetTable_Impl;
class StyleSheetEntry
{
    std::vector<css::beans::PropertyValue> m_aInteropGrabBag;
public:
    OUString sStyleIdentifierI;
    OUString sStyleIdentifierD;
    bool            bIsDefaultStyle;
    bool            bInvalidHeight;
    bool            bHasUPE; //universal property expansion
    StyleType       nStyleTypeCode; //sgc
    OUString sBaseStyleIdentifier;
    OUString sNextStyleIdentifier;
    OUString sStyleName;
    OUString sStyleName1;
    PropertyMapPtr  pProperties;
    OUString sConvertedStyleName;
    std::vector<css::beans::PropertyValue> aLatentStyles; ///< Attributes of latentStyles
    std::vector<css::beans::PropertyValue> aLsdExceptions; ///< List of lsdException attribute lists
    bool           bAutoRedefine; ///< Writer calls this auto-update.

    void AppendInteropGrabBag(const css::beans::PropertyValue& rValue);
    css::beans::PropertyValue GetInteropGrabBag(); ///< Used for table styles, has a name.
    css::beans::PropertyValues GetInteropGrabBagSeq(); ///< Used for existing styles, just a list of properties.

    StyleSheetEntry();
    virtual ~StyleSheetEntry();
};

typedef std::shared_ptr<StyleSheetEntry> StyleSheetEntryPtr;

class DomainMapper;
class StyleSheetTable :
        public LoggedProperties,
        public LoggedTable
{
    std::unique_ptr<StyleSheetTable_Impl> m_pImpl;

public:
    StyleSheetTable(DomainMapper& rDMapper, css::uno::Reference<css::text::XTextDocument> const& xTextDocument, bool bIsNewDoc);
    virtual ~StyleSheetTable() override;

    void ApplyStyleSheets( const FontTablePtr& rFontTable );
    const StyleSheetEntryPtr FindStyleSheetByISTD(const OUString& sIndex);
    const StyleSheetEntryPtr FindStyleSheetByStyleName(const OUString& rIndex);
    const StyleSheetEntryPtr FindStyleSheetByConvertedStyleName(const OUString& rIndex);
    const StyleSheetEntryPtr FindDefaultParaStyle();
    // returns the parent of the one with the given name - if empty the parent of the current style sheet is returned
    const StyleSheetEntryPtr FindParentStyleSheet(const OUString& sBaseStyle);

    OUString ConvertStyleName( const OUString& rWWName, bool bExtendedSearch = false );

    OUString getOrCreateCharStyle( PropertyValueVector_t& rCharProperties, bool bAlwaysCreate );

    /// Returns the default character properties.
    PropertyMapPtr GetDefaultCharProps();

private:
    // Properties
    virtual void lcl_attribute(Id Name, Value & val) override;
    virtual void lcl_sprm(Sprm & sprm) override;

    // Table
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref) override;

    void applyDefaults(bool bParaProperties);
};
typedef std::shared_ptr< StyleSheetTable >    StyleSheetTablePtr;


class TableStyleSheetEntry :
    public StyleSheetEntry
{
private:
    typedef std::map<TblStyleType, PropertyMapPtr> TblStylePrs;

    TblStylePrs m_aStyles;

public:
    // Adds a new tblStylePr to the table style entry. This method
    // fixes some possible properties conflicts, like borders ones.
    void AddTblStylePr( TblStyleType nType, const PropertyMapPtr& pProps );

    // Gets all the properties
    //     + corresponding to the mask,
    //     + from the parent styles

    // @param mask      mask describing which properties to return
    PropertyMapPtr GetProperties( sal_Int32 nMask);

    TableStyleSheetEntry( StyleSheetEntry& aEntry );
    virtual ~TableStyleSheetEntry( ) override;

protected:
    PropertyMapPtr GetLocalPropertiesFromMask( sal_Int32 nMask );
};


}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
