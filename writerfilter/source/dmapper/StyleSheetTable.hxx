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
#ifndef INCLUDED_STYLESHEETTABLE_HXX
#define INCLUDED_STYLESHEETTABLE_HXX

#include "TblStylePrHandler.hxx"

#include <WriterFilterDllApi.hxx>
#include <dmapper/DomainMapper.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <PropertyMap.hxx>
#include <FontTable.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>

#ifdef DEBUG_DOMAINMAPPER
#include <resourcemodel/TagLogger.hxx>
#endif

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
    ::rtl::OUString sStyleIdentifierI;
    ::rtl::OUString sStyleIdentifierD;
    bool            bIsDefaultStyle;
    bool            bInvalidHeight;
    bool            bHasUPE; //universal property expansion
    StyleType       nStyleTypeCode; //sgc
    ::rtl::OUString sBaseStyleIdentifier;
    ::rtl::OUString sNextStyleIdentifier;
    ::rtl::OUString sStyleName;
    ::rtl::OUString sStyleName1;
    PropertyMapPtr  pProperties;
    ::rtl::OUString sConvertedStyleName;
    
#ifdef DEBUG_DOMAINMAPPER
    virtual XMLTag::Pointer_t toTag();
#endif
    
    StyleSheetEntry();
    virtual ~StyleSheetEntry();
};

typedef boost::shared_ptr<StyleSheetEntry> StyleSheetEntryPtr;

class DomainMapper;
class StyleSheetTable :
        public Properties,
        public Table
{
    StyleSheetTable_Impl   *m_pImpl;

public:
    StyleSheetTable( DomainMapper& rDMapper,
                        ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextDocument> xTextDocument );
    virtual ~StyleSheetTable();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    // Table
    virtual void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

    void ApplyStyleSheets( FontTablePtr rFontTable );
    const StyleSheetEntryPtr FindStyleSheetByISTD(const ::rtl::OUString& sIndex);
    const StyleSheetEntryPtr FindStyleSheetByStyleName(const ::rtl::OUString& rIndex);
    const StyleSheetEntryPtr FindStyleSheetByConvertedStyleName(const ::rtl::OUString& rIndex);
    // returns the parent of the one with the given name - if empty the parent of the current style sheet is returned
    const StyleSheetEntryPtr FindParentStyleSheet(::rtl::OUString sBaseStyle);

    ::rtl::OUString ConvertStyleName( const ::rtl::OUString& rWWName, bool bExtendedSearch = false );
    ::rtl::OUString GetStyleIdFromIndex(const sal_uInt32 sti);

    ::rtl::OUString getOrCreateCharStyle( PropertyValueVector_t& rCharProperties );

private:
    void resolveAttributeProperties(Value & val);
    void resolveSprmProps(Sprm & sprm_);
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
    PropertyMapPtr GetProperties( sal_Int32 nMask );
    
#ifdef DEBUG_DOMAINMAPPER
    virtual XMLTag::Pointer_t toTag();
#endif

    TableStyleSheetEntry( StyleSheetEntry& aEntry, StyleSheetTable* pStyles );
    virtual ~TableStyleSheetEntry( );

protected:
    PropertyMapPtr GetLocalPropertiesFromMask( sal_Int32 nMask );
};
typedef boost::shared_ptr<TableStyleSheetEntry> TableStyleSheetEntryPtr;

}}

#endif //
