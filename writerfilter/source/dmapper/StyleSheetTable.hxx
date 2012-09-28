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
#ifndef INCLUDED_STYLESHEETTABLE_HXX
#define INCLUDED_STYLESHEETTABLE_HXX

#include "TblStylePrHandler.hxx"

#include <WriterFilterDllApi.hxx>
#include <dmapper/DomainMapper.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <PropertyMap.hxx>
#include <FontTable.hxx>
#include <resourcemodel/LoggedResources.hxx>

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
    STYLE_LIST
};

struct StyleSheetTable_Impl;
class StyleSheetEntry
{
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

    StyleSheetEntry();
    virtual ~StyleSheetEntry();
};

typedef boost::shared_ptr<StyleSheetEntry> StyleSheetEntryPtr;
typedef ::std::deque<StyleSheetEntryPtr> StyleSheetEntryDeque;
typedef boost::shared_ptr<StyleSheetEntryDeque> StyleSheetEntryDequePtr;

class DomainMapper;
class StyleSheetTable :
        public LoggedProperties,
        public LoggedTable
{
    StyleSheetTable_Impl   *m_pImpl;

public:
    StyleSheetTable( DomainMapper& rDMapper,
                        ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextDocument> xTextDocument );
    virtual ~StyleSheetTable();

    void ApplyStyleSheets( FontTablePtr rFontTable );
    const StyleSheetEntryPtr FindStyleSheetByISTD(const OUString& sIndex);
    const StyleSheetEntryPtr FindStyleSheetByStyleName(const OUString& rIndex);
    const StyleSheetEntryPtr FindStyleSheetByConvertedStyleName(const OUString& rIndex);
    // returns the parent of the one with the given name - if empty the parent of the current style sheet is returned
    const StyleSheetEntryPtr FindParentStyleSheet(OUString sBaseStyle);

    OUString ConvertStyleName( const OUString& rWWName, bool bExtendedSearch = false );
    OUString GetStyleIdFromIndex(const sal_uInt32 sti);

    OUString getOrCreateCharStyle( PropertyValueVector_t& rCharProperties );

    /// Returns the default character properties.
    PropertyMapPtr GetDefaultCharProps();

private:
    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

    // Table
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

    void resolveAttributeProperties(Value & val);
    void applyDefaults(bool bParaProperties);
};
typedef boost::shared_ptr< StyleSheetTable >    StyleSheetTablePtr;


class WRITERFILTER_DLLPRIVATE TableStyleSheetEntry :
    public StyleSheetEntry
{
private:
    typedef std::map<TblStyleType, PropertyMapPtr> TblStylePrs;

    StyleSheetTable* m_pStyleSheet;
    TblStylePrs m_aStyles;

public:

    short m_nColBandSize;
    short m_nRowBandSize;

    // Adds a new tblStylePr to the table style entry. This method
    // fixes some possible properties conflicts, like borders ones.
    void AddTblStylePr( TblStyleType nType, PropertyMapPtr pProps );

    // Gets all the properties
    //     + corresponding to the mask,
    //     + from the parent styles
    //
    // @param mask      mask describing which properties to return
    // @param pStack    already processed StyleSheetEntries
    PropertyMapPtr GetProperties( sal_Int32 nMask, StyleSheetEntryDequePtr pStack = StyleSheetEntryDequePtr());

    TableStyleSheetEntry( StyleSheetEntry& aEntry, StyleSheetTable* pStyles );
    virtual ~TableStyleSheetEntry( );

protected:
    PropertyMapPtr GetLocalPropertiesFromMask( sal_Int32 nMask );
};
typedef boost::shared_ptr<TableStyleSheetEntry> TableStyleSheetEntryPtr;

}}

#endif //

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
