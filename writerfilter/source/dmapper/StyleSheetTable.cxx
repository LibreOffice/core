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
#include <StyleSheetTable.hxx>
#include "util.hxx"
#include <NumberingManager.hxx>
#include <ConversionHelper.hxx>
#include <TblStylePrHandler.hxx>
#include <BorderHandler.hxx>
#include <LatentStyleHandler.hxx>
#include <ooxml/resourceids.hxx>
#include <vector>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <map>
#include <set>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <comphelper/string.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;

namespace writerfilter {
namespace dmapper
{

typedef ::std::map< OUString, OUString> StringPairMap_t;


StyleSheetEntry::StyleSheetEntry() :
        sStyleIdentifierI()
        ,sStyleIdentifierD()
        ,bIsDefaultStyle(false)
        ,bInvalidHeight(false)
        ,bHasUPE(false)
        ,nStyleTypeCode(STYLE_TYPE_UNKNOWN)
        ,sBaseStyleIdentifier()
        ,sNextStyleIdentifier()
        ,pProperties(new StyleSheetPropertyMap)
        ,bAutoRedefine(false)
{
}

StyleSheetEntry::~StyleSheetEntry()
{
}

TableStyleSheetEntry::TableStyleSheetEntry( StyleSheetEntry& rEntry, StyleSheetTable* pStyles ):
    StyleSheetEntry( ),
    m_pStyleSheet( pStyles )
{

    bIsDefaultStyle = rEntry.bIsDefaultStyle;
    bInvalidHeight = rEntry.bInvalidHeight;
    bHasUPE = rEntry.bHasUPE;
    nStyleTypeCode = STYLE_TYPE_TABLE;
    sBaseStyleIdentifier = rEntry.sBaseStyleIdentifier;
    sNextStyleIdentifier = rEntry.sNextStyleIdentifier;
    sStyleName = rEntry.sStyleName;
    sStyleName1 = rEntry.sStyleName1;
    sStyleIdentifierI = rEntry.sStyleIdentifierI;
    sStyleIdentifierD = rEntry.sStyleIdentifierD;

    m_nColBandSize = 1;
    m_nRowBandSize = 1;
}

TableStyleSheetEntry::~TableStyleSheetEntry( )
{
    m_pStyleSheet = nullptr;
}

void TableStyleSheetEntry::AddTblStylePr( TblStyleType nType, PropertyMapPtr pProps )
{
    static const int nTypesProps = 4;
    static const TblStyleType pTypesToFix[nTypesProps] =
    {
        TBL_STYLE_FIRSTROW,
        TBL_STYLE_LASTROW,
        TBL_STYLE_FIRSTCOL,
        TBL_STYLE_LASTCOL
    };

    static const PropertyIds pPropsToCheck[nTypesProps] =
    {
        PROP_BOTTOM_BORDER,
        PROP_TOP_BORDER,
        PROP_RIGHT_BORDER,
        PROP_LEFT_BORDER
    };

    for (int i=0; i < nTypesProps; ++i )
    {
        if ( nType == pTypesToFix[i] )
        {
            PropertyIds nChecked = pPropsToCheck[i];
            boost::optional<PropertyMap::Property> pChecked = pProps->getProperty(nChecked);

            PropertyIds nInsideProp = ( i < 2 ) ? META_PROP_HORIZONTAL_BORDER : META_PROP_VERTICAL_BORDER;
            boost::optional<PropertyMap::Property> pInside = pProps->getProperty(nInsideProp);

            if ( pChecked && pProps )
            {
                // In this case, remove the inside border
                pProps->Erase( nInsideProp );
            }

            break;
        }
    }

    // Append the tblStylePr
    m_aStyles[nType] = pProps;
}

PropertyMapPtr TableStyleSheetEntry::GetProperties( sal_Int32 nMask, StyleSheetEntryDequePtr pStack )
{
    PropertyMapPtr pProps( new PropertyMap );

    // First get the parent properties
    StyleSheetEntryPtr pEntry = m_pStyleSheet->FindParentStyleSheet( sBaseStyleIdentifier );

    if ( pEntry.get( ) )
    {
        if (pStack.get() == nullptr)
            pStack.reset(new StyleSheetEntryDeque());

        StyleSheetEntryDeque::const_iterator aIt = find(pStack->begin(), pStack->end(), pEntry);

        if (aIt != pStack->end())
        {
            pStack->push_back(pEntry);

            TableStyleSheetEntry* pParent = static_cast<TableStyleSheetEntry *>( pEntry.get( ) );
            pProps->InsertProps(pParent->GetProperties(nMask));

            pStack->pop_back();
        }
    }

    // And finally get the mask ones
    pProps->InsertProps(GetLocalPropertiesFromMask(nMask));

    return pProps;
}

beans::PropertyValues StyleSheetEntry::GetInteropGrabBagSeq()
{
    return comphelper::containerToSequence(m_aInteropGrabBag);
}

beans::PropertyValue StyleSheetEntry::GetInteropGrabBag()
{
    beans::PropertyValue aRet;
    aRet.Name = sStyleIdentifierI;

    beans::PropertyValues aSeq = GetInteropGrabBagSeq();
    aRet.Value = uno::makeAny(aSeq);
    return aRet;
}

void StyleSheetEntry::AppendInteropGrabBag(const beans::PropertyValue& rValue)
{
    m_aInteropGrabBag.push_back(rValue);
}

void lcl_mergeProps( PropertyMapPtr pToFill,  PropertyMapPtr pToAdd, TblStyleType nStyleId )
{
    static const PropertyIds pPropsToCheck[] =
    {
        PROP_BOTTOM_BORDER,
        PROP_TOP_BORDER,
        PROP_RIGHT_BORDER,
        PROP_LEFT_BORDER,
    };

    bool pRemoveInside[] =
    {
        ( nStyleId == TBL_STYLE_FIRSTROW ),
        ( nStyleId == TBL_STYLE_LASTROW ),
        ( nStyleId == TBL_STYLE_LASTCOL ),
        ( nStyleId == TBL_STYLE_FIRSTCOL )
    };

    for ( unsigned i = 0 ; i != sizeof(pPropsToCheck) / sizeof(PropertyIds); i++ )
    {
        PropertyIds nId = pPropsToCheck[i];
        boost::optional<PropertyMap::Property> pProp = pToAdd->getProperty(nId);

        if ( pProp )
        {
            if ( pRemoveInside[i] )
            {
                // Remove the insideH and insideV depending on the cell pos
                PropertyIds nInsideProp = ( i < 2 ) ? META_PROP_HORIZONTAL_BORDER : META_PROP_VERTICAL_BORDER;
                pToFill->Erase(nInsideProp);
            }
        }
    }

    pToFill->InsertProps(pToAdd);
}

PropertyMapPtr TableStyleSheetEntry::GetLocalPropertiesFromMask( sal_Int32 nMask )
{
    // Order from right to left
    struct TblStyleTypeAndMask {
        sal_Int32       mask;
        TblStyleType    type;
    };

    static const TblStyleTypeAndMask aOrderedStyleTable[] =
    {
        { 0x010, TBL_STYLE_BAND2HORZ },
        { 0x020, TBL_STYLE_BAND1HORZ },
        { 0x040, TBL_STYLE_BAND2VERT },
        { 0x080, TBL_STYLE_BAND1VERT },
        { 0x100, TBL_STYLE_LASTCOL  },
        { 0x200, TBL_STYLE_FIRSTCOL },
        { 0x400, TBL_STYLE_LASTROW  },
        { 0x800, TBL_STYLE_FIRSTROW },
        { 0x001, TBL_STYLE_SWCELL },
        { 0x002, TBL_STYLE_SECELL },
        { 0x004, TBL_STYLE_NWCELL },
        { 0x008, TBL_STYLE_NECELL }
    };

    // Get the properties applying according to the mask
    PropertyMapPtr pProps( new PropertyMap( ) );
    for (size_t i = 0; i < sizeof(aOrderedStyleTable)/sizeof(aOrderedStyleTable[0]); ++i)
    {
        TblStylePrs::iterator pIt = m_aStyles.find( aOrderedStyleTable[ i ].type );
        if ( ( nMask & aOrderedStyleTable[ i ].mask ) && ( pIt != m_aStyles.end( ) ) )
            lcl_mergeProps( pProps, pIt->second, aOrderedStyleTable[ i ].type );
    }
    return pProps;
}


struct ListCharStylePropertyMap_t
{
    OUString         sCharStyleName;
    PropertyValueVector_t   aPropertyValues;

    ListCharStylePropertyMap_t(const OUString& rCharStyleName, const PropertyValueVector_t& rPropertyValues):
        sCharStyleName( rCharStyleName ),
        aPropertyValues( rPropertyValues )
        {}
};
typedef std::vector< ListCharStylePropertyMap_t > ListCharStylePropertyVector_t;


struct StyleSheetTable_Impl
{
    DomainMapper&                           m_rDMapper;
    uno::Reference< text::XTextDocument>    m_xTextDocument;
    uno::Reference< beans::XPropertySet>    m_xTextDefaults;
    std::vector< StyleSheetEntryPtr >       m_aStyleSheetEntries;
    StyleSheetEntryPtr                      m_pCurrentEntry;
    PropertyMapPtr                          m_pDefaultParaProps, m_pDefaultCharProps;
    PropertyMapPtr                          m_pCurrentProps;
    StringPairMap_t                         m_aStyleNameMap;
    /// Style names which should not be used without a " (user)" suffix.
    std::set<OUString>                      m_aReservedStyleNames;
    ListCharStylePropertyVector_t           m_aListCharStylePropertyVector;
    bool                                    m_bIsNewDoc;

    StyleSheetTable_Impl(DomainMapper& rDMapper, uno::Reference< text::XTextDocument> const& xTextDocument, bool bIsNewDoc);

    OUString HasListCharStyle( const PropertyValueVector_t& rCharProperties );

    /// Appends the given key-value pair to the list of latent style properties of the current entry.
    void AppendLatentStyleProperty(const OUString& aName, Value& rValue);
    /// Sets all properties of xStyle back to default.
    void SetPropertiesToDefault(const uno::Reference<style::XStyle>& xStyle);
};


StyleSheetTable_Impl::StyleSheetTable_Impl(DomainMapper& rDMapper,
        uno::Reference< text::XTextDocument> const& xTextDocument,
        bool const bIsNewDoc)
    :
            m_rDMapper( rDMapper ),
            m_xTextDocument( xTextDocument ),
            m_pCurrentEntry(),
            m_pDefaultParaProps(new PropertyMap),
            m_pDefaultCharProps(new PropertyMap),
            m_bIsNewDoc(bIsNewDoc)
{
    //set font height default to 10pt
    uno::Any aVal = uno::makeAny( double(10.) );
    m_pDefaultCharProps->Insert( PROP_CHAR_HEIGHT, aVal );
    m_pDefaultCharProps->Insert( PROP_CHAR_HEIGHT_ASIAN, aVal );
    m_pDefaultCharProps->Insert( PROP_CHAR_HEIGHT_COMPLEX, aVal );
}


OUString StyleSheetTable_Impl::HasListCharStyle( const PropertyValueVector_t& rPropValues )
{
    OUString sRet;
    ListCharStylePropertyVector_t::const_iterator aListVectorIter = m_aListCharStylePropertyVector.begin();
    while( aListVectorIter != m_aListCharStylePropertyVector.end() )
    {
        //if size is identical
        if( aListVectorIter->aPropertyValues.size() == rPropValues.size() )
        {
            bool bBreak = false;
            //then search for all contained properties
            PropertyValueVector_t::const_iterator aList1Iter = rPropValues.begin();
            while( aList1Iter != rPropValues.end() && !bBreak)
            {
                //find the property
                bool bElementFound = false;
                PropertyValueVector_t::const_iterator aList2Iter = aListVectorIter->aPropertyValues.begin();
                while( aList2Iter != aListVectorIter->aPropertyValues.end() && !bBreak )
                {
                    if( aList2Iter->Name == aList1Iter->Name )
                    {
                        bElementFound = true;
                        if( aList2Iter->Value != aList1Iter->Value )
                            bBreak = true;
                        break;
                    }
                    ++aList2Iter;
                }
                //set break flag if property hasn't been found
                if(!bElementFound )
                {
                    bBreak = true;
                    break;
                }
                ++aList1Iter;
            }
            if( !bBreak )
                return aListVectorIter->sCharStyleName;
        }
        ++aListVectorIter;
    }
    return sRet;
}

void StyleSheetTable_Impl::AppendLatentStyleProperty(const OUString& aName, Value& rValue)
{
    beans::PropertyValue aValue;
    aValue.Name = aName;
    aValue.Value <<= rValue.getString();
    m_pCurrentEntry->aLatentStyles.push_back(aValue);
}

void StyleSheetTable_Impl::SetPropertiesToDefault(const uno::Reference<style::XStyle>& xStyle)
{
    // See if the existing style has any non-default properties. If so, reset them back to default.
    uno::Reference<beans::XPropertySet> xPropertySet(xStyle, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySetInfo> xPropertySetInfo = xPropertySet->getPropertySetInfo();
    uno::Sequence<beans::Property> aProperties = xPropertySetInfo->getProperties();
    std::vector<OUString> aPropertyNames;
    for (sal_Int32 i = 0; i < aProperties.getLength(); ++i)
    {
        aPropertyNames.push_back(aProperties[i].Name);
    }

    uno::Reference<beans::XPropertyState> xPropertyState(xStyle, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyState> aStates = xPropertyState->getPropertyStates(comphelper::containerToSequence(aPropertyNames));
    for (sal_Int32 i = 0; i < aStates.getLength(); ++i)
    {
        if (aStates[i] == beans::PropertyState_DIRECT_VALUE)
        {
            try
            {
                xPropertyState->setPropertyToDefault(aPropertyNames[i]);
            }
            catch(const uno::Exception& rException)
            {
                SAL_INFO("writerfilter", "setPropertyToDefault(" << aPropertyNames[i] << ") failed: " << rException.Message);
            }
        }
    }
}

StyleSheetTable::StyleSheetTable(DomainMapper& rDMapper,
        uno::Reference< text::XTextDocument> const& xTextDocument,
        bool const bIsNewDoc)
: LoggedProperties("StyleSheetTable")
, LoggedTable("StyleSheetTable")
, m_pImpl( new StyleSheetTable_Impl(rDMapper, xTextDocument, bIsNewDoc) )
{
}


StyleSheetTable::~StyleSheetTable()
{
}

PropertyMapPtr StyleSheetTable::GetDefaultCharProps()
{
    return m_pImpl->m_pDefaultCharProps;
}

void StyleSheetTable::lcl_attribute(Id Name, Value & val)
{
    OSL_ENSURE( m_pImpl->m_pCurrentEntry, "current entry has to be set here");
    if(!m_pImpl->m_pCurrentEntry)
        return ;
    int nIntValue = val.getInt();
    OUString sValue = val.getString();

    // The default type is paragraph, and it needs to be processed first,
    // because the NS_ooxml::LN_CT_Style_type handling may set m_pImpl->m_pCurrentEntry
    // to point to a different object.
    if( m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_UNKNOWN )
    {
        if( Name != NS_ooxml::LN_CT_Style_type )
            m_pImpl->m_pCurrentEntry->nStyleTypeCode = STYLE_TYPE_PARA;
    }
    switch(Name)
    {
        case NS_ooxml::LN_CT_Style_type:
        {
            SAL_WARN_IF( m_pImpl->m_pCurrentEntry->nStyleTypeCode != STYLE_TYPE_UNKNOWN,
                "writerfilter", "Style type needs to be processed first" );
            StyleType nType(STYLE_TYPE_UNKNOWN);
            switch (nIntValue)
            {
                case NS_ooxml::LN_Value_ST_StyleType_paragraph:
                    nType = STYLE_TYPE_PARA;
                    break;
                case NS_ooxml::LN_Value_ST_StyleType_character:
                    nType = STYLE_TYPE_CHAR;
                    break;
                case NS_ooxml::LN_Value_ST_StyleType_table:
                    nType = STYLE_TYPE_TABLE;
                    break;
                case NS_ooxml::LN_Value_ST_StyleType_numbering:
                    nType = STYLE_TYPE_LIST;
                    break;
                default:
                    SAL_WARN("writerfilter", "unknown LN_CT_Style_type " << nType);
                    SAL_FALLTHROUGH;
                case 0: // explicit unknown set by tokenizer
                    break;

            }
            if ( nType == STYLE_TYPE_TABLE )
            {
                StyleSheetEntryPtr pEntry = m_pImpl->m_pCurrentEntry;
                std::shared_ptr<TableStyleSheetEntry> pTableEntry( new TableStyleSheetEntry( *pEntry.get( ), this ) );
                m_pImpl->m_pCurrentEntry = pTableEntry;
            }
            else
                m_pImpl->m_pCurrentEntry->nStyleTypeCode = nType;
        }
        break;
        case NS_ooxml::LN_CT_Style_default:
            m_pImpl->m_pCurrentEntry->bIsDefaultStyle = (nIntValue != 0);
            if (m_pImpl->m_pCurrentEntry->nStyleTypeCode != STYLE_TYPE_UNKNOWN)
            {
                beans::PropertyValue aValue;
                aValue.Name = "default";
                aValue.Value = uno::makeAny(m_pImpl->m_pCurrentEntry->bIsDefaultStyle);
                m_pImpl->m_pCurrentEntry->AppendInteropGrabBag(aValue);
            }
        break;
        case NS_ooxml::LN_CT_Style_customStyle:
            if (m_pImpl->m_pCurrentEntry->nStyleTypeCode != STYLE_TYPE_UNKNOWN)
            {
                beans::PropertyValue aValue;
                aValue.Name = "customStyle";
                aValue.Value = uno::makeAny(nIntValue != 0);
                m_pImpl->m_pCurrentEntry->AppendInteropGrabBag(aValue);
            }
        break;
        case NS_ooxml::LN_CT_Style_styleId:
            m_pImpl->m_pCurrentEntry->sStyleIdentifierI = sValue;
            m_pImpl->m_pCurrentEntry->sStyleIdentifierD = sValue;
            if(m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_TABLE)
            {
                TableStyleSheetEntry* pTableEntry = static_cast<TableStyleSheetEntry *>(m_pImpl->m_pCurrentEntry.get());
                beans::PropertyValue aValue;
                aValue.Name = "styleId";
                aValue.Value = uno::makeAny(sValue);
                pTableEntry->AppendInteropGrabBag(aValue);
            }
        break;
        case NS_ooxml::LN_CT_TblWidth_w:
            if (StyleSheetPropertyMap* pMap = dynamic_cast< StyleSheetPropertyMap* >( m_pImpl->m_pCurrentEntry->pProperties.get() ))
                pMap->SetCT_TblWidth_w( nIntValue );
        break;
        case NS_ooxml::LN_CT_TblWidth_type:
            if (StyleSheetPropertyMap* pMap = dynamic_cast< StyleSheetPropertyMap* >( m_pImpl->m_pCurrentEntry->pProperties.get() ))
                pMap->SetCT_TblWidth_type( nIntValue );
        break;
        case NS_ooxml::LN_CT_LatentStyles_defQFormat:
            m_pImpl->AppendLatentStyleProperty("defQFormat", val);
        break;
        case NS_ooxml::LN_CT_LatentStyles_defUnhideWhenUsed:
            m_pImpl->AppendLatentStyleProperty("defUnhideWhenUsed", val);
        break;
        case NS_ooxml::LN_CT_LatentStyles_defSemiHidden:
            m_pImpl->AppendLatentStyleProperty("defSemiHidden", val);
        break;
        case NS_ooxml::LN_CT_LatentStyles_count:
            m_pImpl->AppendLatentStyleProperty("count", val);
        break;
        case NS_ooxml::LN_CT_LatentStyles_defUIPriority:
            m_pImpl->AppendLatentStyleProperty("defUIPriority", val);
        break;
        case NS_ooxml::LN_CT_LatentStyles_defLockedState:
            m_pImpl->AppendLatentStyleProperty("defLockedState", val);
        break;
        default:
        {
#ifdef DEBUG_WRITERFILTER
            TagLogger::getInstance().element("unhandled");
#endif
        }
        break;
    }
}


void StyleSheetTable::lcl_sprm(Sprm & rSprm)
{
    sal_uInt32 nSprmId = rSprm.getId();
    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue.get() ? pValue->getInt() : 0;
    OUString sStringValue = pValue.get() ? pValue->getString() : OUString();

    switch(nSprmId)
    {
        case NS_ooxml::LN_CT_Style_name:
            //this is only a UI name!
            m_pImpl->m_pCurrentEntry->sStyleName = sStringValue;
            m_pImpl->m_pCurrentEntry->sStyleName1 = sStringValue;
            if(m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_TABLE)
            {
                TableStyleSheetEntry* pTableEntry = static_cast<TableStyleSheetEntry *>(m_pImpl->m_pCurrentEntry.get());
                beans::PropertyValue aValue;
                aValue.Name = "name";
                aValue.Value = uno::makeAny(sStringValue);
                pTableEntry->AppendInteropGrabBag(aValue);
            }
            break;
        case NS_ooxml::LN_CT_Style_basedOn:
            m_pImpl->m_pCurrentEntry->sBaseStyleIdentifier = sStringValue;
            if(m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_TABLE)
            {
                TableStyleSheetEntry* pTableEntry = static_cast<TableStyleSheetEntry *>(m_pImpl->m_pCurrentEntry.get());
                beans::PropertyValue aValue;
                aValue.Name = "basedOn";
                aValue.Value = uno::makeAny(sStringValue);
                pTableEntry->AppendInteropGrabBag(aValue);
            }
            break;
        case NS_ooxml::LN_CT_Style_next:
            m_pImpl->m_pCurrentEntry->sNextStyleIdentifier = sStringValue;
            break;
        case NS_ooxml::LN_CT_Style_aliases:
        case NS_ooxml::LN_CT_Style_hidden:
        case NS_ooxml::LN_CT_Style_personal:
        case NS_ooxml::LN_CT_Style_personalCompose:
        case NS_ooxml::LN_CT_Style_personalReply:
        break;
        case NS_ooxml::LN_CT_Style_autoRedefine:
        m_pImpl->m_pCurrentEntry->bAutoRedefine = nIntValue;
        break;
        case NS_ooxml::LN_CT_Style_tcPr:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get() && m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_TABLE)
            {
                TblStylePrHandlerPtr pTblStylePrHandler(new TblStylePrHandler(m_pImpl->m_rDMapper));
                pProperties->resolve(*pTblStylePrHandler);
                StyleSheetEntry* pEntry = m_pImpl->m_pCurrentEntry.get();
                TableStyleSheetEntry& rTableEntry = dynamic_cast<TableStyleSheetEntry&>(*pEntry);
                rTableEntry.AppendInteropGrabBag(pTblStylePrHandler->getInteropGrabBag("tcPr"));

                // This is a <w:tcPr> directly under <w:style>, so it affects the whole table.
                rTableEntry.pProperties->InsertProps(pTblStylePrHandler->getProperties());
            }
        }
        break;
        case NS_ooxml::LN_CT_Style_trPr:
        break;
        case NS_ooxml::LN_CT_Style_rsid:
        case NS_ooxml::LN_CT_Style_qFormat:
        case NS_ooxml::LN_CT_Style_semiHidden:
        case NS_ooxml::LN_CT_Style_unhideWhenUsed:
        case NS_ooxml::LN_CT_Style_uiPriority:
        case NS_ooxml::LN_CT_Style_link:
        case NS_ooxml::LN_CT_Style_locked:
            if (m_pImpl->m_pCurrentEntry->nStyleTypeCode != STYLE_TYPE_UNKNOWN)
            {
                StyleSheetEntryPtr pEntry = m_pImpl->m_pCurrentEntry;
                beans::PropertyValue aValue;
                switch (nSprmId)
                {
                case NS_ooxml::LN_CT_Style_rsid:
                {
                    // We want the rsid as a hex string, but always with the length of 8.
                    OUStringBuffer aBuf = OUString::number(nIntValue, 16);
                    OUStringBuffer aStr;
                    comphelper::string::padToLength(aStr, 8 - aBuf.getLength(), '0');
                    aStr.append(aBuf.getStr());

                    aValue.Name = "rsid";
                    aValue.Value = uno::makeAny(aStr.makeStringAndClear());
                }
                break;
                case NS_ooxml::LN_CT_Style_qFormat:
                    aValue.Name = "qFormat";
                break;
                case NS_ooxml::LN_CT_Style_semiHidden:
                    aValue.Name = "semiHidden";
                break;
                case NS_ooxml::LN_CT_Style_unhideWhenUsed:
                    aValue.Name = "unhideWhenUsed";
                break;
                case NS_ooxml::LN_CT_Style_uiPriority:
                {
                    aValue.Name = "uiPriority";
                    aValue.Value = uno::makeAny(OUString::number(nIntValue));
                }
                break;
                case NS_ooxml::LN_CT_Style_link:
                {
                    aValue.Name = "link";
                    aValue.Value = uno::makeAny(sStringValue);
                }
                break;
                case NS_ooxml::LN_CT_Style_locked:
                    aValue.Name = "locked";
                break;
                }
                pEntry->AppendInteropGrabBag(aValue);
            }
        break;
        case NS_ooxml::LN_CT_Style_tblPr: //contains table properties
        case NS_ooxml::LN_CT_Style_tblStylePr: //contains  to table properties
        case NS_ooxml::LN_CT_TblPrBase_tblInd: //table properties - at least width value and type
        case NS_ooxml::LN_EG_RPrBase_rFonts: //table fonts
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                TblStylePrHandlerPtr pTblStylePrHandler( new TblStylePrHandler( m_pImpl->m_rDMapper ) );
                pProperties->resolve( *pTblStylePrHandler );

                // Add the properties to the table style
                TblStyleType nType = pTblStylePrHandler->getType( );
                PropertyMapPtr pProps = pTblStylePrHandler->getProperties( );
                StyleSheetEntry *  pEntry = m_pImpl->m_pCurrentEntry.get();

                TableStyleSheetEntry * pTableEntry = dynamic_cast<TableStyleSheetEntry*>( pEntry );
                if (nType == TBL_STYLE_UNKNOWN)
                {
                    pEntry->pProperties->InsertProps(pProps);
                }
                else
                {
                    if (pTableEntry != nullptr)
                        pTableEntry->AddTblStylePr( nType, pProps );
                }

                if (nSprmId == NS_ooxml::LN_CT_Style_tblPr)
                {
                    if (pTableEntry != nullptr)
                        pTableEntry->AppendInteropGrabBag(pTblStylePrHandler->getInteropGrabBag("tblPr"));
                }
                else if (nSprmId == NS_ooxml::LN_CT_Style_tblStylePr)
                {
                    pTblStylePrHandler->appendInteropGrabBag("type", pTblStylePrHandler->getTypeString());
                    if (pTableEntry != nullptr)
                        pTableEntry->AppendInteropGrabBag(pTblStylePrHandler->getInteropGrabBag("tblStylePr"));
                }
            }
            break;
        }
        case NS_ooxml::LN_CT_PPrDefault_pPr:
        case NS_ooxml::LN_CT_DocDefaults_pPrDefault:
            m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pDefaultParaProps );
            resolveSprmProps( m_pImpl->m_rDMapper, rSprm );
            m_pImpl->m_rDMapper.PopStyleSheetProperties();
            applyDefaults( true );
        break;
        case NS_ooxml::LN_CT_RPrDefault_rPr:
        case NS_ooxml::LN_CT_DocDefaults_rPrDefault:
            m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pDefaultCharProps );
            resolveSprmProps( m_pImpl->m_rDMapper, rSprm );
            m_pImpl->m_rDMapper.PopStyleSheetProperties();
            applyDefaults( false );
        break;
        case NS_ooxml::LN_CT_TblPrBase_jc:     //table alignment - row properties!
             m_pImpl->m_pCurrentEntry->pProperties->Insert( PROP_HORI_ORIENT,
                uno::makeAny( ConversionHelper::convertTableJustification( nIntValue )));
        break;
        case NS_ooxml::LN_CT_TrPrBase_jc:     //table alignment - row properties!
            if (StyleSheetPropertyMap* pMap = dynamic_cast< StyleSheetPropertyMap* >( m_pImpl->m_pCurrentEntry->pProperties.get() ))
                pMap->SetCT_TrPrBase_jc(nIntValue);
        break;
        case NS_ooxml::LN_CT_TblPrBase_tblBorders: //table borders, might be defined in table style
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                BorderHandlerPtr pBorderHandler( new BorderHandler(m_pImpl->m_rDMapper.IsOOXMLImport()) );
                pProperties->resolve(*pBorderHandler);
                m_pImpl->m_pCurrentEntry->pProperties->InsertProps(
                        pBorderHandler->getProperties());
            }
        }
        break;
        case NS_ooxml::LN_CT_TblPrBase_tblStyleRowBandSize:
        case NS_ooxml::LN_CT_TblPrBase_tblStyleColBandSize:
        {
            StyleSheetEntry* pEntry = m_pImpl->m_pCurrentEntry.get( );
            TableStyleSheetEntry *pTEntry = static_cast<TableStyleSheetEntry*>( pEntry );
            if ( pTEntry )
            {
                if ( nSprmId == NS_ooxml::LN_CT_TblPrBase_tblStyleRowBandSize )
                    pTEntry->m_nRowBandSize = nIntValue;
                else
                    pTEntry->m_nColBandSize = nIntValue;
            }
        }
        break;
        case NS_ooxml::LN_CT_TblPrBase_tblCellMar:
            //no cell margins in styles
        break;
        case NS_ooxml::LN_CT_LatentStyles_lsdException:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if (pProperties.get())
            {
                std::shared_ptr<LatentStyleHandler> pLatentStyleHandler(new LatentStyleHandler());
                pProperties->resolve(*pLatentStyleHandler);
                beans::PropertyValue aValue;
                aValue.Name = "lsdException";
                aValue.Value = uno::makeAny(comphelper::containerToSequence(pLatentStyleHandler->getAttributes()));
                m_pImpl->m_pCurrentEntry->aLsdExceptions.push_back(aValue);
            }
        }
        break;
        case NS_ooxml::LN_CT_Style_pPr:
            // no break
        case NS_ooxml::LN_CT_Style_rPr:
            // no break
        default:
            {
                if (!m_pImpl->m_pCurrentEntry)
                    break;

                std::shared_ptr<TablePropertiesHandler> pTblHandler(new TablePropertiesHandler());
                pTblHandler->SetProperties( m_pImpl->m_pCurrentEntry->pProperties );
                if ( !pTblHandler->sprm( rSprm ) )
                {
                    m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pCurrentEntry->pProperties );

                    PropertyMapPtr pProps(new PropertyMap());
                    bool bTableStyleRunProps = m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_TABLE && nSprmId == NS_ooxml::LN_CT_Style_rPr;
                    if (bTableStyleRunProps)
                        m_pImpl->m_rDMapper.setInTableStyleRunProps(true);
                    if (m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_TABLE)
                    {
                        if (nSprmId == NS_ooxml::LN_CT_Style_pPr)
                            m_pImpl->m_rDMapper.enableInteropGrabBag("pPr");
                        else if (nSprmId == NS_ooxml::LN_CT_Style_rPr)
                            m_pImpl->m_rDMapper.enableInteropGrabBag("rPr");
                    }
                    m_pImpl->m_rDMapper.sprmWithProps( rSprm, pProps );
                    if (m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_TABLE)
                    {
                        if (nSprmId == NS_ooxml::LN_CT_Style_pPr || nSprmId == NS_ooxml::LN_CT_Style_rPr)
                        {
                            TableStyleSheetEntry* pTableEntry = static_cast<TableStyleSheetEntry *>(m_pImpl->m_pCurrentEntry.get());
                            pTableEntry->AppendInteropGrabBag(m_pImpl->m_rDMapper.getInteropGrabBag());
                        }
                    }
                    if (bTableStyleRunProps)
                        m_pImpl->m_rDMapper.setInTableStyleRunProps(false);

                    m_pImpl->m_pCurrentEntry->pProperties->InsertProps(pProps);

                    m_pImpl->m_rDMapper.PopStyleSheetProperties( );

                    if (m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_PARA && m_pImpl->m_pCurrentEntry->bIsDefaultStyle)
                    {
                        // The current style is the default paragraph style.
                        PropertyMapPtr pProperties = m_pImpl->m_pCurrentEntry->pProperties;
                        if (pProperties->isSet(PROP_CHAR_HEIGHT) && !m_pImpl->m_pDefaultParaProps->isSet(PROP_CHAR_HEIGHT))
                        {
                            // We provide a character height value, but a document-level default wasn't set.
                            if (m_pImpl->m_xTextDefaults.is())
                            {
                                m_pImpl->m_xTextDefaults->setPropertyValue("CharHeight", pProperties->getProperty(PROP_CHAR_HEIGHT)->second);
                            }
                        }
                    }
                }
            }
            break;
}
}


void StyleSheetTable::lcl_entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
    //create a new style entry
    OSL_ENSURE( !m_pImpl->m_pCurrentEntry, "current entry has to be NULL here");
    StyleSheetEntryPtr pNewEntry( new StyleSheetEntry );
    m_pImpl->m_pCurrentEntry = pNewEntry;
    m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pCurrentEntry->pProperties );
    ref->resolve(*this);
    //append it to the table
    m_pImpl->m_rDMapper.PopStyleSheetProperties();
    if( !m_pImpl->m_rDMapper.IsOOXMLImport() || !m_pImpl->m_pCurrentEntry->sStyleName.isEmpty())
    {
        m_pImpl->m_pCurrentEntry->sConvertedStyleName = ConvertStyleName( m_pImpl->m_pCurrentEntry->sStyleName );
        m_pImpl->m_aStyleSheetEntries.push_back( m_pImpl->m_pCurrentEntry );
    }
    else
    {
        //TODO: this entry contains the default settings - they have to be added to the settings
    }

    if (!m_pImpl->m_pCurrentEntry->aLatentStyles.empty())
    {
        // We have latent styles for this entry, then process them.
        std::vector<beans::PropertyValue>& rLatentStyles = m_pImpl->m_pCurrentEntry->aLatentStyles;

        if (!m_pImpl->m_pCurrentEntry->aLsdExceptions.empty())
        {
            std::vector<beans::PropertyValue>& rLsdExceptions = m_pImpl->m_pCurrentEntry->aLsdExceptions;
            beans::PropertyValue aValue;
            aValue.Name = "lsdExceptions";
            aValue.Value = uno::makeAny( comphelper::containerToSequence(rLsdExceptions) );
            rLatentStyles.push_back(aValue);
        }

        uno::Sequence<beans::PropertyValue> aLatentStyles( comphelper::containerToSequence(rLatentStyles) );

        // We can put all latent style info directly to the document interop
        // grab bag, as we can be sure that only a single style entry has
        // latent style info.
        uno::Reference<beans::XPropertySet> xPropertySet(m_pImpl->m_xTextDocument, uno::UNO_QUERY);
        auto aGrabBag = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(xPropertySet->getPropertyValue("InteropGrabBag").get< uno::Sequence<beans::PropertyValue> >());
        beans::PropertyValue aValue;
        aValue.Name = "latentStyles";
        aValue.Value = uno::makeAny(aLatentStyles);
        aGrabBag.push_back(aValue);
        xPropertySet->setPropertyValue("InteropGrabBag", uno::makeAny(comphelper::containerToSequence(aGrabBag)));
    }

    StyleSheetEntryPtr pEmptyEntry;
    m_pImpl->m_pCurrentEntry = pEmptyEntry;
}
/*-------------------------------------------------------------------------
    sorting helper
  -----------------------------------------------------------------------*/
class PropValVector
{
    std::vector<beans::PropertyValue> m_aValues;
public:
    PropValVector(){}

    void Insert(const beans::PropertyValue& rVal);
    uno::Sequence< uno::Any > getValues();
    uno::Sequence< OUString > getNames();
};

void PropValVector::Insert(const beans::PropertyValue& rVal)
{
    auto aIt = m_aValues.begin();
    while (aIt != m_aValues.end())
    {
        if (aIt->Name > rVal.Name)
        {
            m_aValues.insert( aIt, rVal );
            return;
        }
        ++aIt;
    }
    m_aValues.push_back(rVal);
}

uno::Sequence< uno::Any > PropValVector::getValues()
{
    std::vector<uno::Any> aRet;
    std::transform(m_aValues.begin(), m_aValues.end(), std::back_inserter(aRet), [](const beans::PropertyValue& rValue) { return rValue.Value; });
    return comphelper::containerToSequence(aRet);
}

uno::Sequence< OUString > PropValVector::getNames()
{
    std::vector<OUString> aRet;
    std::transform(m_aValues.begin(), m_aValues.end(), std::back_inserter(aRet), [](const beans::PropertyValue& rValue) { return rValue.Name; });
    return comphelper::containerToSequence(aRet);
}

void StyleSheetTable::ApplyStyleSheets( FontTablePtr rFontTable )
{
    try
    {
        uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
        uno::Reference< lang::XMultiServiceFactory > xDocFactory( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
        uno::Reference<container::XNameContainer> xCharStyles;
        uno::Reference<container::XNameContainer> xParaStyles;
        uno::Reference<container::XNameContainer> xNumberingStyles;

        xStyleFamilies->getByName(getPropertyName( PROP_CHARACTER_STYLES )) >>= xCharStyles;
        xStyleFamilies->getByName(getPropertyName( PROP_PARAGRAPH_STYLES )) >>= xParaStyles;
        xStyleFamilies->getByName("NumberingStyles") >>= xNumberingStyles;
        if(xCharStyles.is() && xParaStyles.is())
        {
            std::vector<beans::PropertyValue> aTableStylesVec;
            std::vector< StyleSheetEntryPtr >::iterator aIt = m_pImpl->m_aStyleSheetEntries.begin();
            while( aIt != m_pImpl->m_aStyleSheetEntries.end() )
            {
                StyleSheetEntryPtr pEntry = *aIt;
                if( pEntry->nStyleTypeCode == STYLE_TYPE_CHAR || pEntry->nStyleTypeCode == STYLE_TYPE_PARA || pEntry->nStyleTypeCode == STYLE_TYPE_LIST )
                {
                    bool bParaStyle = pEntry->nStyleTypeCode == STYLE_TYPE_PARA;
                    bool bListStyle = pEntry->nStyleTypeCode == STYLE_TYPE_LIST;
                    bool bInsert = false;
                    uno::Reference< container::XNameContainer > xStyles = bParaStyle ? xParaStyles : (bListStyle ? xNumberingStyles : xCharStyles);
                    uno::Reference< style::XStyle > xStyle;
                    OUString sConvertedStyleName = ConvertStyleName( pEntry->sStyleName );

                    if(xStyles->hasByName( sConvertedStyleName ))
                    {
                        // When pasting, don't update existing styles.
                        if (!m_pImpl->m_bIsNewDoc)
                        {
                            ++aIt;
                            continue;
                        }
                        xStyles->getByName( sConvertedStyleName ) >>= xStyle;

                        // Standard is handled already in applyDefaults().
                        if (sConvertedStyleName != "Standard")
                            m_pImpl->SetPropertiesToDefault(xStyle);
                    }
                    else
                    {
                        bInsert = true;
                        xStyle.set(xDocFactory->createInstance(
                                     bParaStyle ?
                                        getPropertyName( PROP_SERVICE_PARA_STYLE ) :
                                        (bListStyle ? OUString("com.sun.star.style.NumberingStyle") : getPropertyName( PROP_SERVICE_CHAR_STYLE ))),
                                   uno::UNO_QUERY_THROW);

                        // Numbering styles have to be inserted early, as e.g. the NumberingRules property is only available after insertion.
                        if (bListStyle)
                        {
                            xStyles->insertByName( sConvertedStyleName, uno::makeAny( xStyle ) );
                            xStyle.set(xStyles->getByName(sConvertedStyleName), uno::UNO_QUERY_THROW);

                            StyleSheetPropertyMap* pPropertyMap = dynamic_cast<StyleSheetPropertyMap*>(pEntry->pProperties.get());
                            if (pPropertyMap && pPropertyMap->GetListId() == -1)
                            {
                                // No properties? Word default is 'none', Writer one is 'arabic', handle this.
                                uno::Reference<beans::XPropertySet> xPropertySet(xStyle, uno::UNO_QUERY_THROW);
                                uno::Reference<container::XIndexReplace> xNumberingRules;
                                xPropertySet->getPropertyValue("NumberingRules") >>= xNumberingRules;
                                uno::Reference<container::XIndexAccess> xIndexAccess(xNumberingRules, uno::UNO_QUERY_THROW);
                                for (sal_Int32 i = 0; i < xIndexAccess->getCount(); ++i)
                                {
                                    uno::Sequence< beans::PropertyValue > aLvlProps(1);
                                    aLvlProps[0].Name = "NumberingType";
                                    aLvlProps[0].Value <<= style::NumberingType::NUMBER_NONE;
                                    xNumberingRules->replaceByIndex(i, uno::makeAny(aLvlProps));
                                    xPropertySet->setPropertyValue("NumberingRules", uno::makeAny(xNumberingRules));
                                }
                            }
                        }
                    }
                    if( !pEntry->sBaseStyleIdentifier.isEmpty() )
                    {
                        try
                        {
                            //TODO: Handle cases where a paragraph <> character style relation is needed
                            StyleSheetEntryPtr pParent = FindStyleSheetByISTD( pEntry->sBaseStyleIdentifier );
                            // Writer core doesn't support numbering styles having a parent style, it seems
                            if (pParent.get() != nullptr && !bListStyle)
                                xStyle->setParentStyle(ConvertStyleName( pParent->sStyleName ));
                        }
                        catch( const uno::RuntimeException& )
                        {
                            OSL_FAIL( "Styles parent could not be set");
                        }
                    }
                    else if( bParaStyle )
                    {
                        //now it's time to set the default parameters - for paragraph styles
                        //Fonts: Western first entry in font table
                        //CJK: second entry
                        //CTL: third entry, if it exists

                        sal_uInt32 nFontCount = rFontTable->size();
                        if( !m_pImpl->m_rDMapper.IsOOXMLImport() && nFontCount > 2 )
                        {
                            uno::Any aTwoHundredFortyTwip = uno::makeAny(12.);

                            // font size to 240 twip (12 pts) for all if not set
                            pEntry->pProperties->Insert(PROP_CHAR_HEIGHT, aTwoHundredFortyTwip, false);

                            // western font not already set -> apply first font
                            const FontEntry::Pointer_t pWesternFontEntry(rFontTable->getFontEntry( 0 ));
                            OUString sWesternFontName = pWesternFontEntry->sFontName;
                            pEntry->pProperties->Insert(PROP_CHAR_FONT_NAME, uno::makeAny( sWesternFontName ), false);

                            // CJK  ... apply second font
                            const FontEntry::Pointer_t pCJKFontEntry(rFontTable->getFontEntry( 2 ));
                            pEntry->pProperties->Insert(PROP_CHAR_FONT_NAME_ASIAN, uno::makeAny( pCJKFontEntry->sFontName ), false);
                            pEntry->pProperties->Insert(PROP_CHAR_HEIGHT_ASIAN, aTwoHundredFortyTwip, false);

                            // CTL  ... apply third font, if available
                            if( nFontCount > 3 )
                            {
                                const FontEntry::Pointer_t pCTLFontEntry(rFontTable->getFontEntry( 3 ));
                                pEntry->pProperties->Insert(PROP_CHAR_FONT_NAME_COMPLEX, uno::makeAny( pCTLFontEntry->sFontName ), false);
                                pEntry->pProperties->Insert(PROP_CHAR_HEIGHT_COMPLEX, aTwoHundredFortyTwip, false);
                            }
                        }

                        // Widow/Orphan -> set both to two if not already set
                        uno::Any aTwo = uno::makeAny(sal_Int8(2));
                        pEntry->pProperties->Insert(PROP_PARA_WIDOWS, aTwo, false);
                        pEntry->pProperties->Insert(PROP_PARA_ORPHANS, aTwo, false);

                        // Left-to-right direction if not already set
                        pEntry->pProperties->Insert(PROP_WRITING_MODE, uno::makeAny( sal_Int16(text::WritingMode_LR_TB) ), false);
                        // Left alignment if not already set
                        pEntry->pProperties->Insert(PROP_PARA_ADJUST, uno::makeAny( sal_Int16(style::ParagraphAdjust_LEFT) ), false);
                    }

                    auto aPropValues = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(pEntry->pProperties->GetPropertyValues());
                    bool bAddFollowStyle = false;
                    if(bParaStyle && !pEntry->sNextStyleIdentifier.isEmpty() )
                    {
                        bAddFollowStyle = true;
                    }

                    // remove Left/RightMargin values from TOX heading styles
                    if( bParaStyle )
                    {
                        // Set the outline levels
                        const StyleSheetPropertyMap* pStyleSheetProperties = dynamic_cast<const StyleSheetPropertyMap*>(pEntry ? pEntry->pProperties.get() : nullptr);
                        if ( pStyleSheetProperties )
                        {
                            beans::PropertyValue aLvlVal( getPropertyName( PROP_OUTLINE_LEVEL ), 0,
                                    uno::makeAny( sal_Int16( pStyleSheetProperties->GetOutlineLevel( ) + 1 ) ),
                                    beans::PropertyState_DIRECT_VALUE );
                            aPropValues.push_back(aLvlVal);
                        }

                        uno::Reference< beans::XPropertyState >xState( xStyle, uno::UNO_QUERY_THROW );
                        if( sConvertedStyleName == "Contents Heading" ||
                            sConvertedStyleName == "User Index Heading" ||
                            sConvertedStyleName == "Index Heading" )
                        {
                            //left margin is set to NULL by default
                            uno::Reference< beans::XPropertyState >xState1( xStyle, uno::UNO_QUERY_THROW );
                            xState1->setPropertyToDefault(getPropertyName( PROP_PARA_LEFT_MARGIN ));
                        }
                        else if ( sConvertedStyleName == "Text body" )
                            xState->setPropertyToDefault(getPropertyName( PROP_PARA_BOTTOM_MARGIN ));
                        else if( sConvertedStyleName == "Heading 1" ||
                                sConvertedStyleName == "Heading 2" ||
                                sConvertedStyleName == "Heading 3" ||
                                sConvertedStyleName == "Heading 4" ||
                                sConvertedStyleName == "Heading 5" ||
                                sConvertedStyleName == "Heading 6" ||
                                sConvertedStyleName == "Heading 7" ||
                                sConvertedStyleName == "Heading 8" ||
                                sConvertedStyleName == "Heading 9" )
                        {
                            xState->setPropertyToDefault(getPropertyName( PROP_CHAR_WEIGHT ));
                            xState->setPropertyToDefault(getPropertyName( PROP_CHAR_WEIGHT_ASIAN ));
                            xState->setPropertyToDefault(getPropertyName( PROP_CHAR_WEIGHT_COMPLEX ));
                            xState->setPropertyToDefault(getPropertyName( PROP_CHAR_POSTURE ));
                            xState->setPropertyToDefault(getPropertyName( PROP_CHAR_POSTURE_ASIAN ));
                            xState->setPropertyToDefault(getPropertyName( PROP_CHAR_POSTURE_COMPLEX ));
                            xState->setPropertyToDefault(getPropertyName( PROP_CHAR_PROP_HEIGHT        ));
                            xState->setPropertyToDefault(getPropertyName( PROP_CHAR_PROP_HEIGHT_ASIAN  ));
                            xState->setPropertyToDefault(getPropertyName( PROP_CHAR_PROP_HEIGHT_COMPLEX));

                        }
                    }

                    if(bAddFollowStyle || !aPropValues.empty())
                    {
                        PropValVector aSortedPropVals;
                        for (const beans::PropertyValue& rValue : aPropValues)
                        {
                            // Don't add the style name properties
                            bool bIsParaStyleName = rValue.Name == "ParaStyleName";
                            bool bIsCharStyleName = rValue.Name == "CharStyleName";
                            if ( !bIsParaStyleName && !bIsCharStyleName )
                            {
                                aSortedPropVals.Insert(rValue);
                            }
                        }
                        if(bAddFollowStyle)
                        {
                            //find the name of the Next style
                            std::vector< StyleSheetEntryPtr >::iterator it = m_pImpl->m_aStyleSheetEntries.begin();
                            for (; it != m_pImpl->m_aStyleSheetEntries.end(); ++it)
                            {
                                if (!(*it)->sStyleName.isEmpty() && (*it)->sStyleIdentifierD == pEntry->sNextStyleIdentifier)
                                {
                                    beans::PropertyValue aNew;
                                    aNew.Name = "FollowStyle";
                                    aNew.Value = uno::makeAny(ConvertStyleName((*it)->sStyleIdentifierD));
                                    aSortedPropVals.Insert(aNew);
                                    break;
                                }
                            }
                        }

                        try
                        {
                            uno::Reference< beans::XMultiPropertySet > xMultiPropertySet( xStyle, uno::UNO_QUERY_THROW);
                            xMultiPropertySet->setPropertyValues( aSortedPropVals.getNames(), aSortedPropVals.getValues() );
                        }
                        catch( const lang::WrappedTargetException& rWrapped)
                        {
                            (void) rWrapped;
#ifdef DEBUG_WRITERFILTER
                            OUString aMessage("StyleSheetTable::ApplyStyleSheets: Some style properties could not be set");
                            beans::UnknownPropertyException aUnknownPropertyException;

                            if (rWrapped.TargetException >>= aUnknownPropertyException)
                                aMessage += ": " + aUnknownPropertyException.Message;

                            SAL_WARN("writerfilter", aMessage);
#endif
                        }
                        catch( const uno::Exception& )
                        {
                            OSL_FAIL( "Some style properties could not be set");
                        }
                    }
                    // Numbering style got inserted earlier.
                    if(bInsert && !bListStyle)
                    {
                        xStyles->insertByName( sConvertedStyleName, uno::makeAny( xStyle) );
                    }

                    beans::PropertyValues aGrabBag = pEntry->GetInteropGrabBagSeq();
                    uno::Reference<beans::XPropertySet> xPropertySet(xStyle, uno::UNO_QUERY);
                    if (aGrabBag.hasElements())
                    {
                        xPropertySet->setPropertyValue("StyleInteropGrabBag", uno::makeAny(aGrabBag));
                    }

                    // List styles don't support automatic update.
                    if (pEntry->bAutoRedefine && !bListStyle)
                        xPropertySet->setPropertyValue("IsAutoUpdate", uno::makeAny(sal_True));
                }
                else if(pEntry->nStyleTypeCode == STYLE_TYPE_TABLE)
                {
                    // If this is a table style, save its contents as-is for roundtrip purposes.
                    TableStyleSheetEntry* pTableEntry = static_cast<TableStyleSheetEntry *>(pEntry.get());
                    aTableStylesVec.push_back(pTableEntry->GetInteropGrabBag());
                }
                ++aIt;
            }

            if (!aTableStylesVec.empty())
            {
                // If we had any table styles, add a new document-level InteropGrabBag entry for them.
                uno::Reference<beans::XPropertySet> xPropertySet(m_pImpl->m_xTextDocument, uno::UNO_QUERY);
                uno::Any aAny = xPropertySet->getPropertyValue("InteropGrabBag");
                auto aGrabBag = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(aAny.get< uno::Sequence<beans::PropertyValue> >());
                beans::PropertyValue aValue;
                aValue.Name = "tableStyles";
                aValue.Value = uno::makeAny(comphelper::containerToSequence(aTableStylesVec));
                aGrabBag.push_back(aValue);
                xPropertySet->setPropertyValue("InteropGrabBag", uno::makeAny(comphelper::containerToSequence(aGrabBag)));
            }
        }
    }
    catch( const uno::Exception& rException )
    {
        SAL_WARN("writerfilter", "Styles could not be imported completely: " << rException.Message);
    }
}


const StyleSheetEntryPtr StyleSheetTable::FindStyleSheetByISTD(const OUString& sIndex)
{
    StyleSheetEntryPtr pRet;
    for( size_t nPos = 0; nPos < m_pImpl->m_aStyleSheetEntries.size(); ++nPos )
    {
        if( m_pImpl->m_aStyleSheetEntries[nPos]->sStyleIdentifierD == sIndex)
        {
            pRet = m_pImpl->m_aStyleSheetEntries[nPos];
            break;
        }
    }
    return pRet;
}


const StyleSheetEntryPtr StyleSheetTable::FindStyleSheetByStyleName(const OUString& sIndex)
{
    StyleSheetEntryPtr pRet;
    for( size_t nPos = 0; nPos < m_pImpl->m_aStyleSheetEntries.size(); ++nPos )
    {
        if( m_pImpl->m_aStyleSheetEntries[nPos]->sStyleName == sIndex)
        {
            pRet = m_pImpl->m_aStyleSheetEntries[nPos];
            break;
        }
    }
    return pRet;
}


const StyleSheetEntryPtr StyleSheetTable::FindStyleSheetByConvertedStyleName(const OUString& sIndex)
{
    StyleSheetEntryPtr pRet;
    for( size_t nPos = 0; nPos < m_pImpl->m_aStyleSheetEntries.size(); ++nPos )
    {
        if( m_pImpl->m_aStyleSheetEntries[nPos]->sConvertedStyleName == sIndex)
        {
            pRet = m_pImpl->m_aStyleSheetEntries[nPos];
            break;
        }
    }
    return pRet;
}


const StyleSheetEntryPtr StyleSheetTable::FindDefaultParaStyle()
{
    StyleSheetEntryPtr pRet;
    for (size_t i = 0; i < m_pImpl->m_aStyleSheetEntries.size(); ++i)
    {
        StyleSheetEntryPtr pEntry = m_pImpl->m_aStyleSheetEntries[i];
        if (pEntry->bIsDefaultStyle && pEntry->nStyleTypeCode == STYLE_TYPE_PARA)
        {
            pRet = pEntry;
            break;
        }
    }
    return pRet;
}

const StyleSheetEntryPtr StyleSheetTable::FindParentStyleSheet(const OUString& _sBaseStyle)
{
    if( _sBaseStyle.isEmpty() )
    {
        StyleSheetEntryPtr pEmptyPtr;
        return pEmptyPtr;
    }
    OUString sBaseStyle = _sBaseStyle;
    if( m_pImpl->m_pCurrentEntry)
        sBaseStyle = m_pImpl->m_pCurrentEntry->sBaseStyleIdentifier;

    return FindStyleSheetByISTD( sBaseStyle );
}


static const sal_Char* const aStyleNamePairs[] =
{
    "Normal",                     "Standard",
    "heading 1",                  "Heading 1",
    "heading 2",                  "Heading 2",
    "heading 3",                  "Heading 3",
    "heading 4",                  "Heading 4",
    "heading 5",                  "Heading 5",
    "heading 6",                  "Heading 6",
    "heading 7",                  "Heading 7",
    "heading 8",                  "Heading 8",
    "heading 9",                  "Heading 9",
    "Heading1",                   "Heading 1",
    "Heading2",                   "Heading 2",
    "Heading3",                   "Heading 3",
    "Heading4",                   "Heading 4",
    "Heading5",                   "Heading 5",
    "Heading6",                   "Heading 6",
    "Heading7",                   "Heading 7",
    "Heading8",                   "Heading 8",
    "Heading9",                   "Heading 9",
    "Heading 1",                  "Heading 1",
    "Heading 2",                  "Heading 2",
    "Heading 3",                  "Heading 3",
    "Heading 4",                  "Heading 4",
    "Heading 5",                  "Heading 5",
    "Heading 6",                  "Heading 6",
    "Heading 7",                  "Heading 7",
    "Heading 8",                  "Heading 8",
    "Heading 9",                  "Heading 9",
    "Index 1",                   "Index 1",
    "Index 2",                   "Index 2",
    "Index 3",                   "Index 3",
    "Index 4",                   "",
    "Index 5",                   "",
    "Index 6",                   "",
    "Index 7",                   "",
    "Index 8",                   "",
    "Index 9",                   "",
    "TOC 1",                     "Contents 1",
    "TOC 2",                     "Contents 2",
    "TOC 3",                     "Contents 3",
    "TOC 4",                     "Contents 4",
    "TOC 5",                     "Contents 5",
    "TOC 6",                     "Contents 6",
    "TOC 7",                     "Contents 7",
    "TOC 8",                     "Contents 8",
    "TOC 9",                     "Contents 9",
    "TOCHeading",                "Contents Heading",
    "toc 1",                     "Contents 1",
    "toc 2",                     "Contents 2",
    "toc 3",                     "Contents 3",
    "toc 4",                     "Contents 4",
    "toc 5",                     "Contents 5",
    "toc 6",                     "Contents 6",
    "toc 7",                     "Contents 7",
    "toc 8",                     "Contents 8",
    "toc 9",                     "Contents 9",
    "TOC1",                     "Contents 1",
    "TOC2",                     "Contents 2",
    "TOC3",                     "Contents 3",
    "TOC4",                     "Contents 4",
    "TOC5",                     "Contents 5",
    "TOC6",                     "Contents 6",
    "TOC7",                     "Contents 7",
    "TOC8",                     "Contents 8",
    "TOC9",                     "Contents 9",
    "Normal Indent",             "",
    "Footnote Text",             "Footnote",
    "Annotation Text",           "",
    "Header",                    "Header",
    "header",                    "Header",
    "Footer",                    "Footer",
    "footer",                    "Footer",
    "Index Heading",             "Index Heading",
    "Caption",                   "",
    "Table of Figures",          "",
    "Envelope Address",          "Addressee",
    "Envelope Return",           "Sender",
    "Footnote Reference",        "Footnote anchor",
    "Annotation Reference",      "",
    "Line Number",               "Line numbering",
    "Page Number",               "Page Number",
    "Endnote Reference",         "Endnote anchor",
    "Endnote Text",              "Endnote Symbol",
    "Table of Authorities",      "",
    "Macro Text",                "",
    "TOA Heading",               "",
    "List",                      "List",
    "List 2",                    "",
    "List 3",                    "",
    "List 4",                    "",
    "List 5",                    "",
    "List Bullet",               "",
    "List Bullet 2",             "",
    "List Bullet 3",             "",
    "List Bullet 4",             "",
    "List Bullet 5",             "",
    "List Number",               "",
    "List Number 2",             "",
    "List Number 3",             "",
    "List Number 4",             "",
    "List Number 5",             "",
    "Title",                     "Title",
    "Closing",                   "",
    "Signature",                 "Signature",
    "Default Paragraph Font",    "",
    "DefaultParagraphFont",      "Default Paragraph Font",
    "Body Text",                 "Text body",
    "BodyText",                  "Text body",
    "BodyTextIndentItalic",     "Text body indent italic",
    "Body Text Indent",          "Text body indent",
    "BodyTextIndent",           "Text body indent",
    "BodyTextIndent2",          "Text body indent2",
    "List Continue",             "",
    "List Continue 2",           "",
    "List Continue 3",           "",
    "List Continue 4",           "",
    "List Continue 5",           "",
    "Message Header",            "",
    "Subtitle",                  "Subtitle",
    "Salutation",                "",
    "Date",                      "",
    "Body Text First Indent",    "Body Text Indent",
    "Body Text First Indent 2",  "",
    "Note Heading",              "",
    "Body Text 2",               "",
    "Body Text 3",               "",
    "Body Text Indent 2",        "",
    "Body Text Indent 3",        "",
    "Block Text",                "",
    "Hyperlink",                 "Internet link",
    "Followed Hyperlink",        "Visited Internet Link",
    "Emphasis",                  "Emphasis",
    "Document Map",              "",
    "Plain Text",                "",
    "NoList",                   "No List",
    "AbstractHeading",          "Abstract Heading",
    "AbstractBody",             "Abstract Body",
    "PageNumber",               "page number"
    "TableNormal",              "Normal Table",
    "DocumentMap",              "Document Map"
};


OUString StyleSheetTable::ConvertStyleName( const OUString& rWWName, bool bExtendedSearch)
{
    OUString sRet( rWWName );
    if( bExtendedSearch )
    {
        //search for the rWWName in the IdentifierD of the existing styles and convert the sStyleName member
        std::vector< StyleSheetEntryPtr >::iterator aIt = m_pImpl->m_aStyleSheetEntries.begin();
        //TODO: performance issue - put styles list into a map sorted by its sStyleIdentifierD members
        while( aIt != m_pImpl->m_aStyleSheetEntries.end() )
        {
            if( rWWName == ( *aIt )->sStyleIdentifierD )
                sRet = ( *aIt )->sStyleName;
            ++aIt;
        }
    }
    if(m_pImpl->m_aStyleNameMap.empty())
    {
        for( sal_uInt32 nPair = 0; nPair < sizeof(aStyleNamePairs) / sizeof( sal_Char*) / 2; ++nPair)
        {
            OUString aFrom = OUString::createFromAscii(aStyleNamePairs[2 * nPair]);
            OUString aTo = OUString::createFromAscii(aStyleNamePairs[2 * nPair + 1]);
            if (!aTo.isEmpty())
            {
                m_pImpl->m_aStyleNameMap.insert( StringPairMap_t::value_type(aFrom, aTo));
                m_pImpl->m_aReservedStyleNames.insert(aTo);
            }
        }
    }
    StringPairMap_t::iterator aIt = m_pImpl->m_aStyleNameMap.find( sRet );
    bool bConverted = false;
    if (aIt != m_pImpl->m_aStyleNameMap.end())
    {
        bConverted = true;
        sRet = aIt->second;
    }

    if (!bConverted)
    {
        // SwStyleNameMapper doc says: If the UI style name equals a
        // programmatic name, then it must append " (user)" to the end.
        std::set<OUString>::iterator aReservedIt = m_pImpl->m_aReservedStyleNames.find(sRet);
        if (aReservedIt != m_pImpl->m_aReservedStyleNames.end())
            sRet += " (user)";
    }
    return sRet;
}

void StyleSheetTable::applyDefaults(bool bParaProperties)
{
    try{
        if(!m_pImpl->m_xTextDefaults.is())
        {
            m_pImpl->m_xTextDefaults.set(
                m_pImpl->m_rDMapper.GetTextFactory()->createInstance("com.sun.star.text.Defaults"),
                uno::UNO_QUERY_THROW );
        }
        if( bParaProperties && m_pImpl->m_pDefaultParaProps.get())
        {
            uno::Reference<style::XStyleFamiliesSupplier> xStylesSupplier(m_pImpl->m_xTextDocument, uno::UNO_QUERY);
            uno::Reference<container::XNameAccess> xStyleFamilies = xStylesSupplier->getStyleFamilies();
            uno::Reference<container::XNameAccess> xParagraphStyles;
            xStyleFamilies->getByName("ParagraphStyles") >>= xParagraphStyles;
            uno::Reference<beans::XPropertySet> xStandard;
            xParagraphStyles->getByName("Standard") >>= xStandard;

            uno::Reference<style::XStyle> xStyle(xStandard, uno::UNO_QUERY);
            m_pImpl->SetPropertiesToDefault(xStyle);

            uno::Sequence< beans::PropertyValue > aPropValues = m_pImpl->m_pDefaultParaProps->GetPropertyValues();
            for( sal_Int32 i = 0; i < aPropValues.getLength(); ++i )
            {
                try
                {
                    xStandard->setPropertyValue(aPropValues[i].Name, aPropValues[i].Value);
                }
                catch( const uno::Exception& )
                {
                    OSL_FAIL( "setPropertyValue exception");
                }
            }
        }
        if( !bParaProperties && m_pImpl->m_pDefaultCharProps.get())
        {
            uno::Sequence< beans::PropertyValue > aPropValues = m_pImpl->m_pDefaultCharProps->GetPropertyValues();
            for( sal_Int32 i = 0; i < aPropValues.getLength(); ++i )
            {
                try
                {
                    m_pImpl->m_xTextDefaults->setPropertyValue( aPropValues[i].Name, aPropValues[i].Value );
                }
                catch( const uno::Exception& )
                {
                    OSL_FAIL( "setPropertyValue exception");
                }
            }
        }
    }
    catch( const uno::Exception& )
    {
    }
}


OUString StyleSheetTable::getOrCreateCharStyle( PropertyValueVector_t& rCharProperties, bool bAlwaysCreate )
{
    //find out if any of the styles already has the required properties then return its name
    OUString sListLabel = m_pImpl->HasListCharStyle(rCharProperties);
    // Don't try to reuse an existing character style if requested.
    if( !sListLabel.isEmpty() && !bAlwaysCreate)
        return sListLabel;
    const char cListLabel[] = "ListLabel ";
    uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
    uno::Reference<container::XNameContainer> xCharStyles;
    xStyleFamilies->getByName("CharacterStyles") >>= xCharStyles;
    //search for all character styles with the name sListLabel + <index>
    sal_Int32 nStyleFound = 0;
    uno::Sequence< OUString > aStyleNames = xCharStyles->getElementNames();
    const OUString* pStyleNames = aStyleNames.getConstArray();
    for( sal_Int32 nStyle = 0; nStyle < aStyleNames.getLength(); ++nStyle )
    {
        OUString sSuffix;
        if( pStyleNames[nStyle].startsWith( cListLabel, &sSuffix ) )
        {
            sal_Int32 nSuffix = sSuffix.toInt32();
            if( nSuffix > 0 )
            {
                if( nSuffix > nStyleFound )
                    nStyleFound = nSuffix;
            }
        }
    }
    sListLabel = cListLabel + OUString::number( ++nStyleFound );
    //create a new one otherwise
    uno::Reference< lang::XMultiServiceFactory > xDocFactory( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
    try
    {
        uno::Reference< style::XStyle > xStyle( xDocFactory->createInstance(
            getPropertyName( PROP_SERVICE_CHAR_STYLE )), uno::UNO_QUERY_THROW);
        uno::Reference< beans::XPropertySet > xStyleProps(xStyle, uno::UNO_QUERY_THROW );
        PropertyValueVector_t::const_iterator aCharPropIter = rCharProperties.begin();
        while( aCharPropIter != rCharProperties.end())
        {
            try
            {
                xStyleProps->setPropertyValue( aCharPropIter->Name, aCharPropIter->Value );
            }
            catch( const uno::Exception& )
            {
                OSL_FAIL( "Exception in StyleSheetTable::getOrCreateCharStyle - Style::setPropertyValue");
            }
            ++aCharPropIter;
        }
        xCharStyles->insertByName( sListLabel, uno::makeAny( xStyle) );
        m_pImpl->m_aListCharStylePropertyVector.push_back( ListCharStylePropertyMap_t( sListLabel, rCharProperties ));
    }
    catch( const uno::Exception& )
    {
        OSL_FAIL( "Exception in StyleSheetTable::getOrCreateCharStyle");
    }

    return sListLabel;
}

}//namespace dmapper
}//namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
