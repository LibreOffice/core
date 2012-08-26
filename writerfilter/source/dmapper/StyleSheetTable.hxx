/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    const StyleSheetEntryPtr FindStyleSheetByISTD(const ::rtl::OUString& sIndex);
    const StyleSheetEntryPtr FindStyleSheetByStyleName(const ::rtl::OUString& rIndex);
    const StyleSheetEntryPtr FindStyleSheetByConvertedStyleName(const ::rtl::OUString& rIndex);
    // returns the parent of the one with the given name - if empty the parent of the current style sheet is returned
    const StyleSheetEntryPtr FindParentStyleSheet(::rtl::OUString sBaseStyle);

    ::rtl::OUString ConvertStyleName( const ::rtl::OUString& rWWName, bool bExtendedSearch = false );
    ::rtl::OUString GetStyleIdFromIndex(const sal_uInt32 sti);

    ::rtl::OUString getOrCreateCharStyle( PropertyValueVector_t& rCharProperties );

private:
    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

    // Table
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

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
    //
    // @param mask      mask describing which properties to return
    // @param pStack    already processed StyleSheetEntries
    PropertyMapPtr GetProperties( sal_Int32 nMask, StyleSheetEntryDequePtr pStack = StyleSheetEntryDequePtr());

    TableStyleSheetEntry( StyleSheetEntry& aEntry, StyleSheetTable* pStyles );
    virtual ~TableStyleSheetEntry( );

protected:
    PropertyMapPtr GetLocalPropertiesFromMask( const sal_Int32 nMask );
    void           MergePropertiesFromMask(const short nBit, const sal_Int32 nMask,
                                           const TblStyleType nStyleId, PropertyMapPtr pToFill);
};
typedef boost::shared_ptr<TableStyleSheetEntry> TableStyleSheetEntryPtr;

}}

#endif //
