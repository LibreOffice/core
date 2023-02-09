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
#include "StyleSheetTable.hxx"
#include "util.hxx"
#include "ConversionHelper.hxx"
#include "TblStylePrHandler.hxx"
#include "TagLogger.hxx"
#include "BorderHandler.hxx"
#include "LatentStyleHandler.hxx"
#include <ooxml/resourceids.hxx>
#include <utility>
#include <vector>
#include <iterator>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <map>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/string.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/sorted_vector.hxx>

using namespace ::com::sun::star;

namespace writerfilter::dmapper
{

StyleSheetEntry::StyleSheetEntry() :
        bIsDefaultStyle(false)
        ,bAssignedAsChapterNumbering(false)
        ,bInvalidHeight(false)
        ,bHasUPE(false)
        ,nStyleTypeCode(STYLE_TYPE_UNKNOWN)
        ,pProperties(new StyleSheetPropertyMap)
        ,bAutoRedefine(false)
{
}

StyleSheetEntry::~StyleSheetEntry()
{
}

TableStyleSheetEntry::TableStyleSheetEntry( StyleSheetEntry const & rEntry )
{
    bIsDefaultStyle = rEntry.bIsDefaultStyle;
    bInvalidHeight = rEntry.bInvalidHeight;
    bHasUPE = rEntry.bHasUPE;
    nStyleTypeCode = STYLE_TYPE_TABLE;
    sBaseStyleIdentifier = rEntry.sBaseStyleIdentifier;
    sNextStyleIdentifier = rEntry.sNextStyleIdentifier;
    sLinkStyleIdentifier = rEntry.sLinkStyleIdentifier;
    sStyleName = rEntry.sStyleName;
    sStyleIdentifierD = rEntry.sStyleIdentifierD;
}

TableStyleSheetEntry::~TableStyleSheetEntry( )
{
}

void TableStyleSheetEntry::AddTblStylePr( TblStyleType nType, const PropertyMapPtr& pProps )
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
            std::optional<PropertyMap::Property> pChecked = pProps->getProperty(nChecked);

            PropertyIds nInsideProp = ( i < 2 ) ? META_PROP_HORIZONTAL_BORDER : META_PROP_VERTICAL_BORDER;
            std::optional<PropertyMap::Property> pInside = pProps->getProperty(nInsideProp);

            if ( pChecked && pInside )
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

PropertyMapPtr TableStyleSheetEntry::GetProperties( sal_Int32 nMask )
{
    PropertyMapPtr pProps( new PropertyMap );

    // And finally get the mask ones
    pProps->InsertProps(GetLocalPropertiesFromMask(nMask));

    return pProps;
}

beans::PropertyValues StyleSheetEntry::GetInteropGrabBagSeq() const
{
    return comphelper::containerToSequence(m_aInteropGrabBag);
}

beans::PropertyValue StyleSheetEntry::GetInteropGrabBag()
{
    beans::PropertyValue aRet;
    aRet.Name = sStyleIdentifierD;

    beans::PropertyValues aSeq = GetInteropGrabBagSeq();
    aRet.Value <<= aSeq;
    return aRet;
}

void StyleSheetEntry::AppendInteropGrabBag(const beans::PropertyValue& rValue)
{
    m_aInteropGrabBag.push_back(rValue);
}

PropertyMapPtr StyleSheetEntry::GetMergedInheritedProperties(const StyleSheetTablePtr& pStyleSheetTable)
{
    PropertyMapPtr pRet;
    if ( pStyleSheetTable && !sBaseStyleIdentifier.isEmpty() && sBaseStyleIdentifier != sStyleIdentifierD )
    {
        const StyleSheetEntryPtr pParentStyleSheet = pStyleSheetTable->FindStyleSheetByISTD(sBaseStyleIdentifier);
        if ( pParentStyleSheet )
            pRet = pParentStyleSheet->GetMergedInheritedProperties(pStyleSheetTable);
    }

    if ( !pRet )
        pRet = new PropertyMap;

    pRet->InsertProps(pProperties.get());

    return pRet;
}

static void lcl_mergeProps( const PropertyMapPtr& pToFill, const PropertyMapPtr& pToAdd, TblStyleType nStyleId )
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

    for ( unsigned i = 0 ; i != SAL_N_ELEMENTS(pPropsToCheck); i++ )
    {
        PropertyIds nId = pPropsToCheck[i];
        std::optional<PropertyMap::Property> pProp = pToAdd->getProperty(nId);

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
    for (const TblStyleTypeAndMask & i : aOrderedStyleTable)
    {
        TblStylePrs::iterator pIt = m_aStyles.find( i.type );
        if ( ( nMask & i.mask ) && ( pIt != m_aStyles.end( ) ) )
            lcl_mergeProps( pProps, pIt->second, i.type );
    }
    return pProps;
}

namespace {

struct ListCharStylePropertyMap_t
{
    OUString         sCharStyleName;
    PropertyValueVector_t   aPropertyValues;

    ListCharStylePropertyMap_t(OUString _sCharStyleName, PropertyValueVector_t&& rPropertyValues):
        sCharStyleName(std::move( _sCharStyleName )),
        aPropertyValues( std::move(rPropertyValues) )
        {}
};

}

struct StyleSheetTable_Impl
{
    DomainMapper&                           m_rDMapper;
    uno::Reference< text::XTextDocument>    m_xTextDocument;
    uno::Reference< beans::XPropertySet>    m_xTextDefaults;
    std::vector< StyleSheetEntryPtr >       m_aStyleSheetEntries;
    std::map< OUString, StyleSheetEntryPtr > m_aStyleSheetEntriesMap;
    std::map<OUString, OUString>            m_ClonedTOCStylesMap;
    StyleSheetEntryPtr                      m_pCurrentEntry;
    PropertyMapPtr                          m_pDefaultParaProps, m_pDefaultCharProps;
    OUString                                m_sDefaultParaStyleName; //WW8 name
    std::vector< ListCharStylePropertyMap_t > m_aListCharStylePropertyVector;
    bool                                    m_bHasImportedDefaultParaProps;
    bool                                    m_bIsNewDoc;

    StyleSheetTable_Impl(DomainMapper& rDMapper, uno::Reference< text::XTextDocument> xTextDocument, bool bIsNewDoc);

    OUString HasListCharStyle( const PropertyValueVector_t& rCharProperties );

    /// Appends the given key-value pair to the list of latent style properties of the current entry.
    void AppendLatentStyleProperty(const OUString& aName, Value const & rValue);
    /// Sets all properties of xStyle back to default.
    static void SetPropertiesToDefault(const uno::Reference<style::XStyle>& xStyle);
    void ApplyClonedTOCStylesToXText(uno::Reference<text::XText> const& xText);
};


StyleSheetTable_Impl::StyleSheetTable_Impl(DomainMapper& rDMapper,
        uno::Reference< text::XTextDocument> xTextDocument,
        bool const bIsNewDoc)
    :       m_rDMapper( rDMapper ),
            m_xTextDocument(std::move( xTextDocument )),
            m_pDefaultParaProps(new PropertyMap),
            m_pDefaultCharProps(new PropertyMap),
            m_sDefaultParaStyleName("Normal"),
            m_bHasImportedDefaultParaProps(false),
            m_bIsNewDoc(bIsNewDoc)
{
    //set font height default to 10pt
    uno::Any aVal( 10.0 );
    m_pDefaultCharProps->Insert( PROP_CHAR_HEIGHT, aVal );
    m_pDefaultCharProps->Insert( PROP_CHAR_HEIGHT_ASIAN, aVal );
    m_pDefaultCharProps->Insert( PROP_CHAR_HEIGHT_COMPLEX, aVal );

    // See SwDoc::RemoveAllFormatLanguageDependencies(), internal filters
    // disable kerning by default, do the same here.
    m_pDefaultCharProps->Insert(PROP_CHAR_AUTO_KERNING, uno::Any(false));
}


OUString StyleSheetTable_Impl::HasListCharStyle( const PropertyValueVector_t& rPropValues )
{
    for( const auto& rListVector : m_aListCharStylePropertyVector )
    {
        const auto& rPropertyValues = rListVector.aPropertyValues;
        //if size is identical
        if( rPropertyValues.size() == rPropValues.size() )
        {
            bool bBreak = false;
            //then search for all contained properties
            for( const auto& rPropVal1 : rPropValues)
            {
                //find the property
                auto aListIter = std::find_if(rPropertyValues.begin(), rPropertyValues.end(),
                    [&rPropVal1](const css::beans::PropertyValue& rPropVal2) { return rPropVal2.Name == rPropVal1.Name; });
                //set break flag if property hasn't been found
                bBreak = (aListIter == rPropertyValues.end()) || (aListIter->Value != rPropVal1.Value);
                if( bBreak )
                    break;
            }
            if( !bBreak )
                return rListVector.sCharStyleName;
        }
    }
    return OUString();
}

void StyleSheetTable_Impl::AppendLatentStyleProperty(const OUString& aName, Value const & rValue)
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
    const uno::Sequence<beans::Property> aProperties = xPropertySetInfo->getProperties();
    std::vector<OUString> aPropertyNames;
    aPropertyNames.reserve(aProperties.getLength());
    std::transform(aProperties.begin(), aProperties.end(), std::back_inserter(aPropertyNames),
        [](const beans::Property& rProp) { return rProp.Name; });

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
            catch(const uno::Exception&)
            {
                TOOLS_INFO_EXCEPTION("writerfilter", "setPropertyToDefault(" << aPropertyNames[i] << ") failed");
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

void StyleSheetTable::SetDefaultParaProps(PropertyIds eId, const css::uno::Any& rAny)
{
    m_pImpl->m_pDefaultParaProps->Insert(eId, rAny, /*bOverwrite=*/false, NO_GRAB_BAG, /*bDocDefault=*/true);
}

PropertyMapPtr const & StyleSheetTable::GetDefaultParaProps() const
{
    return m_pImpl->m_pDefaultParaProps;
}

PropertyMapPtr const & StyleSheetTable::GetDefaultCharProps() const
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
                    [[fallthrough]];
                case 0: // explicit unknown set by tokenizer
                    break;

            }
            if ( nType == STYLE_TYPE_TABLE )
            {
                StyleSheetEntryPtr pEntry = m_pImpl->m_pCurrentEntry;
                tools::SvRef<TableStyleSheetEntry> pTableEntry( new TableStyleSheetEntry( *pEntry ) );
                m_pImpl->m_pCurrentEntry = pTableEntry.get();
            }
            else
                m_pImpl->m_pCurrentEntry->nStyleTypeCode = nType;
        }
        break;
        case NS_ooxml::LN_CT_Style_default:
            m_pImpl->m_pCurrentEntry->bIsDefaultStyle = (nIntValue != 0);

            if (m_pImpl->m_pCurrentEntry->nStyleTypeCode != STYLE_TYPE_UNKNOWN)
            {
                // "If this attribute is specified by multiple styles, then the last instance shall be used."
                if (m_pImpl->m_pCurrentEntry->bIsDefaultStyle
                    && m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_PARA
                    && !m_pImpl->m_pCurrentEntry->sStyleIdentifierD.isEmpty())
                {
                    m_pImpl->m_sDefaultParaStyleName = m_pImpl->m_pCurrentEntry->sStyleIdentifierD;
                }

                beans::PropertyValue aValue;
                aValue.Name = "default";
                aValue.Value <<= m_pImpl->m_pCurrentEntry->bIsDefaultStyle;
                m_pImpl->m_pCurrentEntry->AppendInteropGrabBag(aValue);
            }
        break;
        case NS_ooxml::LN_CT_Style_customStyle:
            if (m_pImpl->m_pCurrentEntry->nStyleTypeCode != STYLE_TYPE_UNKNOWN)
            {
                beans::PropertyValue aValue;
                aValue.Name = "customStyle";
                aValue.Value <<= (nIntValue != 0);
                m_pImpl->m_pCurrentEntry->AppendInteropGrabBag(aValue);
            }
        break;
        case NS_ooxml::LN_CT_Style_styleId:
            m_pImpl->m_pCurrentEntry->sStyleIdentifierD = sValue;
            if(m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_TABLE)
            {
                TableStyleSheetEntry* pTableEntry = static_cast<TableStyleSheetEntry *>(m_pImpl->m_pCurrentEntry.get());
                beans::PropertyValue aValue;
                aValue.Name = "styleId";
                aValue.Value <<= sValue;
                pTableEntry->AppendInteropGrabBag(aValue);
            }
        break;
        case NS_ooxml::LN_CT_TblWidth_w:
        break;
        case NS_ooxml::LN_CT_TblWidth_type:
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
#ifdef DBG_UTIL
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
    sal_Int32 nIntValue = pValue ? pValue->getInt() : 0;
    OUString sStringValue = pValue ? pValue->getString() : OUString();

    switch(nSprmId)
    {
        case NS_ooxml::LN_CT_Style_name:
            //this is only a UI name!
            m_pImpl->m_pCurrentEntry->sStyleName = sStringValue;
            if(m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_TABLE)
            {
                TableStyleSheetEntry* pTableEntry = static_cast<TableStyleSheetEntry *>(m_pImpl->m_pCurrentEntry.get());
                beans::PropertyValue aValue;
                aValue.Name = "name";
                aValue.Value <<= sStringValue;
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
                aValue.Value <<= sStringValue;
                pTableEntry->AppendInteropGrabBag(aValue);
            }
            break;
        case NS_ooxml::LN_CT_Style_link:
            m_pImpl->m_pCurrentEntry->sLinkStyleIdentifier = sStringValue;
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
            if( pProperties && m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_TABLE)
            {
                auto pTblStylePrHandler = std::make_shared<TblStylePrHandler>(m_pImpl->m_rDMapper);
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
                    aValue.Value <<= aStr.makeStringAndClear();
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
                    aValue.Value <<= OUString::number(nIntValue);
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
            if( pProperties )
            {
                auto pTblStylePrHandler = std::make_shared<TblStylePrHandler>( m_pImpl->m_rDMapper );
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
            if (nSprmId == NS_ooxml::LN_CT_DocDefaults_pPrDefault)
                m_pImpl->m_rDMapper.SetDocDefaultsImport(true);

            m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pDefaultParaProps );
            resolveSprmProps( m_pImpl->m_rDMapper, rSprm );
            if ( nSprmId == NS_ooxml::LN_CT_DocDefaults_pPrDefault && m_pImpl->m_pDefaultParaProps &&
                !m_pImpl->m_pDefaultParaProps->isSet( PROP_PARA_TOP_MARGIN ) )
            {
                SetDefaultParaProps( PROP_PARA_TOP_MARGIN, uno::Any( sal_Int32(0) ) );
            }
            m_pImpl->m_rDMapper.PopStyleSheetProperties();
            applyDefaults( true );
            m_pImpl->m_bHasImportedDefaultParaProps = true;
            if (nSprmId == NS_ooxml::LN_CT_DocDefaults_pPrDefault)
                m_pImpl->m_rDMapper.SetDocDefaultsImport(false);
        break;
        case NS_ooxml::LN_CT_RPrDefault_rPr:
        case NS_ooxml::LN_CT_DocDefaults_rPrDefault:
            if (nSprmId == NS_ooxml::LN_CT_DocDefaults_rPrDefault)
                m_pImpl->m_rDMapper.SetDocDefaultsImport(true);

            m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pDefaultCharProps );
            resolveSprmProps( m_pImpl->m_rDMapper, rSprm );
            m_pImpl->m_rDMapper.PopStyleSheetProperties();
            applyDefaults( false );
            if (nSprmId == NS_ooxml::LN_CT_DocDefaults_rPrDefault)
                m_pImpl->m_rDMapper.SetDocDefaultsImport(false);
        break;
        case NS_ooxml::LN_CT_TblPrBase_jc:     //table alignment - row properties!
             m_pImpl->m_pCurrentEntry->pProperties->Insert( PROP_HORI_ORIENT,
                uno::Any( ConversionHelper::convertTableJustification( nIntValue )));
        break;
        case NS_ooxml::LN_CT_TrPrBase_jc:     //table alignment - row properties!
        break;
        case NS_ooxml::LN_CT_TblPrBase_tblBorders: //table borders, might be defined in table style
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties )
            {
                auto pBorderHandler = std::make_shared<BorderHandler>(m_pImpl->m_rDMapper.IsOOXMLImport());
                pProperties->resolve(*pBorderHandler);
                m_pImpl->m_pCurrentEntry->pProperties->InsertProps(
                        pBorderHandler->getProperties());
            }
        }
        break;
        case NS_ooxml::LN_CT_TblPrBase_tblStyleRowBandSize:
        case NS_ooxml::LN_CT_TblPrBase_tblStyleColBandSize:
        break;
        case NS_ooxml::LN_CT_TblPrBase_tblCellMar:
            //no cell margins in styles
        break;
        case NS_ooxml::LN_CT_LatentStyles_lsdException:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if (pProperties)
            {
                tools::SvRef<LatentStyleHandler> pLatentStyleHandler(new LatentStyleHandler());
                pProperties->resolve(*pLatentStyleHandler);
                beans::PropertyValue aValue;
                aValue.Name = "lsdException";
                aValue.Value <<= comphelper::containerToSequence(pLatentStyleHandler->getAttributes());
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

                tools::SvRef<TablePropertiesHandler> pTblHandler(new TablePropertiesHandler());
                pTblHandler->SetProperties( m_pImpl->m_pCurrentEntry->pProperties.get() );
                if ( !pTblHandler->sprm( rSprm ) )
                {
                    m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pCurrentEntry->pProperties.get() );

                    PropertyMapPtr pProps(new PropertyMap());
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

                    m_pImpl->m_pCurrentEntry->pProperties->InsertProps(pProps);

                    m_pImpl->m_rDMapper.PopStyleSheetProperties( );
                }
            }
            break;
}
}


void StyleSheetTable::lcl_entry(writerfilter::Reference<Properties>::Pointer_t ref)
{
    //create a new style entry
    OSL_ENSURE( !m_pImpl->m_pCurrentEntry, "current entry has to be NULL here");
    StyleSheetEntryPtr pNewEntry( new StyleSheetEntry );
    m_pImpl->m_pCurrentEntry = pNewEntry;
    m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pCurrentEntry->pProperties.get() );
    ref->resolve(*this);
    //append it to the table
    m_pImpl->m_rDMapper.PopStyleSheetProperties();
    if( !m_pImpl->m_rDMapper.IsOOXMLImport() || !m_pImpl->m_pCurrentEntry->sStyleName.isEmpty())
    {
        m_pImpl->m_pCurrentEntry->sConvertedStyleName = ConvertStyleName( m_pImpl->m_pCurrentEntry->sStyleName );
        m_pImpl->m_aStyleSheetEntries.push_back( m_pImpl->m_pCurrentEntry );
        m_pImpl->m_aStyleSheetEntriesMap.emplace( m_pImpl->m_pCurrentEntry->sStyleIdentifierD, m_pImpl->m_pCurrentEntry );
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
            aValue.Value <<= comphelper::containerToSequence(rLsdExceptions);
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
        aValue.Value <<= aLatentStyles;
        aGrabBag.push_back(aValue);
        xPropertySet->setPropertyValue("InteropGrabBag", uno::Any(comphelper::containerToSequence(aGrabBag)));
    }

    StyleSheetEntryPtr pEmptyEntry;
    m_pImpl->m_pCurrentEntry = pEmptyEntry;
}
/*-------------------------------------------------------------------------
    sorting helper
  -----------------------------------------------------------------------*/
namespace {

class PropValVector
{
    std::vector<beans::PropertyValue> m_aValues;
public:
    PropValVector(){}

    void Insert(const beans::PropertyValue& rVal);
    uno::Sequence< uno::Any > getValues();
    uno::Sequence< OUString > getNames();
    const std::vector<beans::PropertyValue>& getProperties() const { return m_aValues; };
};

}

void PropValVector::Insert(const beans::PropertyValue& rVal)
{
    auto aIt = std::find_if(m_aValues.begin(), m_aValues.end(),
        [&rVal](beans::PropertyValue& rPropVal) { return rPropVal.Name > rVal.Name; });
    if (aIt != m_aValues.end())
    {
        m_aValues.insert( aIt, rVal );
        return;
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

void StyleSheetTable::ApplyNumberingStyleNameToParaStyles()
{
    try
    {
        uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
        uno::Reference< lang::XMultiServiceFactory > xDocFactory( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
        uno::Reference<container::XNameContainer> xParaStyles;
        xStyleFamilies->getByName(getPropertyName( PROP_PARAGRAPH_STYLES )) >>= xParaStyles;

        if ( !xParaStyles.is() )
            return;

        for ( const auto& pEntry : m_pImpl->m_aStyleSheetEntries )
        {
            StyleSheetPropertyMap* pStyleSheetProperties = nullptr;
            if ( pEntry->nStyleTypeCode == STYLE_TYPE_PARA && (pStyleSheetProperties = pEntry->pProperties.get()) )
            {
                // ListId 0 means turn off numbering - to cancel inheritance - so make sure that can be set.
                if (pStyleSheetProperties->props().GetListId() > -1)
                {
                    uno::Reference< style::XStyle > xStyle;
                    xParaStyles->getByName( ConvertStyleName(pEntry->sStyleName) ) >>= xStyle;

                    if ( !xStyle.is() )
                        break;

                    uno::Reference<beans::XPropertySet> xPropertySet( xStyle, uno::UNO_QUERY_THROW );
                    const OUString sNumberingStyleName = m_pImpl->m_rDMapper.GetListStyleName( pStyleSheetProperties->props().GetListId() );
                    if ( !sNumberingStyleName.isEmpty()
                         || !pStyleSheetProperties->props().GetListId() )
                        xPropertySet->setPropertyValue( getPropertyName(PROP_NUMBERING_STYLE_NAME), uno::Any(sNumberingStyleName) );

                    // Word 2010+ (not Word 2003, and Word 2007 is completely broken)
                    // does something rather strange. It does not allow two paragraph styles
                    // to share the same listLevel on a numbering rule.
                    // Consider this style to just be body level if already used previously.
                    m_pImpl->m_rDMapper.ValidateListLevel(pEntry->sStyleIdentifierD);
                }
            }
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("writerfilter", "Failed applying numbering style name to Paragraph styles");
    }
}

/* Counteract the destructive tendencies of LibreOffice's Chapter Numbering
 *
 * Any assignment to Chapter Numbering will erase the numbering-like properties of inherited styles.
 * So go through the list of styles and any that inherit from a Chapter Numbering style
 * should have the Outline Level reapplied.
 */
void StyleSheetTable::ReApplyInheritedOutlineLevelFromChapterNumbering()
{
    try
    {
        uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier(m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW);
        uno::Reference< lang::XMultiServiceFactory > xDocFactory(m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW);
        uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
        uno::Reference<container::XNameContainer> xParaStyles;
        xStyleFamilies->getByName(getPropertyName(PROP_PARAGRAPH_STYLES)) >>= xParaStyles;

        if (!xParaStyles.is())
            return;

        for (const auto& pEntry : m_pImpl->m_aStyleSheetEntries)
        {
            if (pEntry->nStyleTypeCode != STYLE_TYPE_PARA || pEntry->sBaseStyleIdentifier.isEmpty())
                continue;

            sal_Int16 nOutlineLevel = pEntry->pProperties->GetOutlineLevel();
            if (nOutlineLevel != -1)
                continue;

            StyleSheetEntryPtr pParent = FindStyleSheetByISTD(pEntry->sBaseStyleIdentifier);
            if (!pParent || !pParent->bAssignedAsChapterNumbering)
                continue;

            nOutlineLevel = pParent->pProperties->GetOutlineLevel();
            assert(nOutlineLevel >= WW_OUTLINE_MIN && nOutlineLevel < WW_OUTLINE_MAX);

            // convert MS level to LO equivalent outline level
            ++nOutlineLevel;

            uno::Reference< style::XStyle > xStyle;
            xParaStyles->getByName(pEntry->sConvertedStyleName) >>= xStyle;
            if ( !xStyle.is() )
                break;

            uno::Reference<beans::XPropertySet> xPropertySet( xStyle, uno::UNO_QUERY_THROW );
            xPropertySet->setPropertyValue(getPropertyName(PROP_OUTLINE_LEVEL), uno::Any(nOutlineLevel));
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("writerfilter", "Failed applying outlineLevel to Paragraph styles");
    }
}

void StyleSheetTable_Impl::ApplyClonedTOCStylesToXText(uno::Reference<text::XText> const& xText)
{
    uno::Reference<container::XEnumerationAccess> const xEA(xText, uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumeration> const xParaEnum(xEA->createEnumeration());

    while (xParaEnum->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> const xElem(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
        if (xElem->supportsService(u"com.sun.star.text.Paragraph"))
        {
            uno::Reference<beans::XPropertySet> const xPara(xElem, uno::UNO_QUERY_THROW);
            OUString styleName;
            if (xPara->getPropertyValue(u"ParaStyleName") >>= styleName)
            {
                auto const it(m_ClonedTOCStylesMap.find(styleName));
                if (it != m_ClonedTOCStylesMap.end())
                {
                    xPara->setPropertyValue(u"ParaStyleName", uno::Any(it->second));
                }
            }
        }
        else if (xElem->supportsService(u"com.sun.star.text.TextTable"))
        {
            uno::Reference<text::XTextTable> const xTable(xElem, uno::UNO_QUERY_THROW);
            uno::Sequence<OUString> const cells(xTable->getCellNames());
            for (OUString const& rCell : cells)
            {
                uno::Reference<text::XText> const xCell(xTable->getCellByName(rCell), uno::UNO_QUERY_THROW);
                ApplyClonedTOCStylesToXText(xCell);
            }
        }
    }
}

/**
 Replace the applied en-US Word built-in styles that were referenced from
 TOC fields (also STYLEREF and likely AUTOTEXTLIST) with the localised clones.

 With the style cloned, and the clone referenced, the ToX should work in
 Writer and also, when exported to DOCX, in Word.
 */
void StyleSheetTable::ApplyClonedTOCStyles()
{
    if (m_pImpl->m_ClonedTOCStylesMap.empty()
        || !m_pImpl->m_bIsNewDoc) // avoid modifying pre-existing content
    {
        return;
    }
    SAL_INFO("writerfilter.dmapper", "Applying cloned styles to make TOC work");
    // ignore header / footer, irrelevant for ToX
    // text frames
    uno::Reference<text::XTextFramesSupplier> const xDocTFS(m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumerationAccess> const xFrames(xDocTFS->getTextFrames(), uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumeration> const xFramesEnum(xFrames->createEnumeration());
    while (xFramesEnum->hasMoreElements())
    {
        uno::Reference<text::XText> const xFrame(xFramesEnum->nextElement(), uno::UNO_QUERY_THROW);
        m_pImpl->ApplyClonedTOCStylesToXText(xFrame);
    }
    // body
    uno::Reference<text::XText> const xBody(m_pImpl->m_xTextDocument->getText());
    m_pImpl->ApplyClonedTOCStylesToXText(xBody);
}

void StyleSheetTable::CloneTOCStyle(FontTablePtr const& rFontTable, StyleSheetEntryPtr const pStyle, OUString const& rNewName)
{
    StyleSheetEntryPtr const pClone(new StyleSheetEntry(*pStyle));
    pClone->sStyleIdentifierD = rNewName;
    pClone->sStyleName = rNewName;
    pClone->sConvertedStyleName = ConvertStyleName(rNewName);
    m_pImpl->m_aStyleSheetEntries.push_back(pClone);
    // add it so it will be found if referenced from another TOC
    m_pImpl->m_aStyleSheetEntriesMap.emplace(rNewName, pClone);
    m_pImpl->m_ClonedTOCStylesMap.emplace(pStyle->sStyleName, rNewName);
    std::vector<StyleSheetEntryPtr> const styles{ pClone };
    return ApplyStyleSheetsImpl(rFontTable, styles);
}

void StyleSheetTable::ApplyStyleSheets( const FontTablePtr& rFontTable )
{
    return ApplyStyleSheetsImpl(rFontTable, m_pImpl->m_aStyleSheetEntries);
}

void StyleSheetTable::ApplyStyleSheetsImpl(const FontTablePtr& rFontTable, std::vector<StyleSheetEntryPtr> const& rEntries)
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
            std::vector< ::std::pair<OUString, uno::Reference<style::XStyle>> > aMissingParent;
            std::vector< ::std::pair<OUString, uno::Reference<style::XStyle>> > aMissingFollow;
            std::vector<std::pair<OUString, uno::Reference<style::XStyle>>> aMissingLink;
            std::vector<beans::PropertyValue> aTableStylesVec;
            for (auto& pEntry : rEntries)
            {
                if( pEntry->nStyleTypeCode == STYLE_TYPE_UNKNOWN && !pEntry->sStyleName.isEmpty() )
                    pEntry->nStyleTypeCode = STYLE_TYPE_PARA; // unspecified style types are considered paragraph styles

                if( pEntry->nStyleTypeCode == STYLE_TYPE_CHAR || pEntry->nStyleTypeCode == STYLE_TYPE_PARA || pEntry->nStyleTypeCode == STYLE_TYPE_LIST )
                {
                    bool bParaStyle = pEntry->nStyleTypeCode == STYLE_TYPE_PARA;
                    bool bCharStyle = pEntry->nStyleTypeCode == STYLE_TYPE_CHAR;
                    bool bListStyle = pEntry->nStyleTypeCode == STYLE_TYPE_LIST;
                    bool bInsert = false;
                    uno::Reference< container::XNameContainer > xStyles = bParaStyle ? xParaStyles : (bListStyle ? xNumberingStyles : xCharStyles);
                    uno::Reference< style::XStyle > xStyle;
                    const OUString sConvertedStyleName = ConvertStyleName( pEntry->sStyleName );

                    if(xStyles->hasByName( sConvertedStyleName ))
                    {
                        // When pasting, don't update existing styles.
                        if (!m_pImpl->m_bIsNewDoc)
                        {
                            continue;
                        }
                        xStyles->getByName( sConvertedStyleName ) >>= xStyle;

                        {
                            StyleSheetTable_Impl::SetPropertiesToDefault(xStyle);

                            // resolve import conflicts with built-in styles (only if defaults have been defined)
                            if ( m_pImpl->m_bHasImportedDefaultParaProps
                                && pEntry->sBaseStyleIdentifier.isEmpty()   //imported style has no inheritance
                                && !xStyle->getParentStyle().isEmpty() )    //built-in style has a default inheritance
                            {
                                xStyle->setParentStyle( "" );
                            }
                        }
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
                            xStyles->insertByName( sConvertedStyleName, uno::Any( xStyle ) );
                            xStyle.set(xStyles->getByName(sConvertedStyleName), uno::UNO_QUERY_THROW);

                            StyleSheetPropertyMap* pPropertyMap = pEntry->pProperties.get();
                            if (pPropertyMap && pPropertyMap->props().GetListId() == -1)
                            {
                                // No properties? Word default is 'none', Writer one is 'arabic', handle this.
                                uno::Reference<beans::XPropertySet> xPropertySet(xStyle, uno::UNO_QUERY_THROW);
                                uno::Reference<container::XIndexReplace> xNumberingRules;
                                xPropertySet->getPropertyValue("NumberingRules") >>= xNumberingRules;
                                uno::Reference<container::XIndexAccess> xIndexAccess(xNumberingRules, uno::UNO_QUERY_THROW);
                                for (sal_Int32 i = 0; i < xIndexAccess->getCount(); ++i)
                                {
                                    uno::Sequence< beans::PropertyValue > aLvlProps{
                                        comphelper::makePropertyValue(
                                            "NumberingType", style::NumberingType::NUMBER_NONE)
                                    };
                                    xNumberingRules->replaceByIndex(i, uno::Any(aLvlProps));
                                    xPropertySet->setPropertyValue("NumberingRules", uno::Any(xNumberingRules));
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
                            if (pParent && !bListStyle)
                            {
                                const OUString sParentStyleName = ConvertStyleName( pParent->sStyleName );
                                if ( !sParentStyleName.isEmpty() && !xStyles->hasByName( sParentStyleName ) )
                                    aMissingParent.emplace_back( sParentStyleName, xStyle );
                                else
                                    xStyle->setParentStyle( sParentStyleName );
                            }
                        }
                        catch( const uno::RuntimeException& )
                        {
                            OSL_FAIL( "Styles parent could not be set");
                        }
                    }
                    else if( bParaStyle )
                    {
                        // Paragraph styles that don't inherit from some parent need to apply the DocDefaults
                        pEntry->pProperties->InsertProps( m_pImpl->m_pDefaultParaProps, /*bOverwrite=*/false );

                        //now it's time to set the default parameters - for paragraph styles
                        //Fonts: Western first entry in font table
                        //CJK: second entry
                        //CTL: third entry, if it exists

                        sal_uInt32 nFontCount = rFontTable->size();
                        if( !m_pImpl->m_rDMapper.IsOOXMLImport() && nFontCount > 2 )
                        {
                            uno::Any aTwoHundredFortyTwip(12.);

                            // font size to 240 twip (12 pts) for all if not set
                            pEntry->pProperties->Insert(PROP_CHAR_HEIGHT, aTwoHundredFortyTwip, false);

                            // western font not already set -> apply first font
                            const FontEntry::Pointer_t pWesternFontEntry(rFontTable->getFontEntry( 0 ));
                            OUString sWesternFontName = pWesternFontEntry->sFontName;
                            pEntry->pProperties->Insert(PROP_CHAR_FONT_NAME, uno::Any( sWesternFontName ), false);

                            // CJK  ... apply second font
                            const FontEntry::Pointer_t pCJKFontEntry(rFontTable->getFontEntry( 2 ));
                            pEntry->pProperties->Insert(PROP_CHAR_FONT_NAME_ASIAN, uno::Any( pCJKFontEntry->sFontName ), false);
                            pEntry->pProperties->Insert(PROP_CHAR_HEIGHT_ASIAN, aTwoHundredFortyTwip, false);

                            // CTL  ... apply third font, if available
                            if( nFontCount > 3 )
                            {
                                const FontEntry::Pointer_t pCTLFontEntry(rFontTable->getFontEntry( 3 ));
                                pEntry->pProperties->Insert(PROP_CHAR_FONT_NAME_COMPLEX, uno::Any( pCTLFontEntry->sFontName ), false);
                                pEntry->pProperties->Insert(PROP_CHAR_HEIGHT_COMPLEX, aTwoHundredFortyTwip, false);
                            }
                        }
                    }

                    auto aPropValues = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(pEntry->pProperties->GetPropertyValues());

                    if (bParaStyle || bCharStyle)
                    {
                        // delay adding LinkStyle property: all styles need to be created first
                        if (!pEntry->sLinkStyleIdentifier.isEmpty())
                        {
                            StyleSheetEntryPtr pLinkStyle
                                = FindStyleSheetByISTD(pEntry->sLinkStyleIdentifier);
                            if (pLinkStyle && !pLinkStyle->sStyleName.isEmpty())
                                aMissingLink.emplace_back(ConvertStyleName(pLinkStyle->sStyleName),
                                                          xStyle);
                        }
                    }

                    if( bParaStyle )
                    {
                        // delay adding FollowStyle property: all styles need to be created first
                        if ( !pEntry->sNextStyleIdentifier.isEmpty() )
                        {
                            StyleSheetEntryPtr pFollowStyle = FindStyleSheetByISTD( pEntry->sNextStyleIdentifier );
                            if ( pFollowStyle && !pFollowStyle->sStyleName.isEmpty() )
                                aMissingFollow.emplace_back( ConvertStyleName( pFollowStyle->sStyleName ), xStyle );
                        }

                        // Set the outline levels
                        StyleSheetPropertyMap* pStyleSheetProperties = pEntry ? pEntry->pProperties.get() : nullptr;

                        if ( pStyleSheetProperties )
                        {
                            sal_Int16 nLvl = pStyleSheetProperties->GetOutlineLevel();
                            // convert MS body Level (9) to LO body level (0) and equivalent outline levels
                            if (nLvl != -1)
                            {
                                if (nLvl == WW_OUTLINE_MAX)
                                    nLvl = 0;
                                else
                                    ++nLvl;

                                beans::PropertyValue aLvlVal(getPropertyName(PROP_OUTLINE_LEVEL), 0,
                                                             uno::Any(nLvl),
                                                             beans::PropertyState_DIRECT_VALUE);
                                aPropValues.push_back(aLvlVal);
                            }
                        }

                        uno::Reference< beans::XPropertyState >xState( xStyle, uno::UNO_QUERY_THROW );
                        if( sConvertedStyleName == "Contents Heading" ||
                            sConvertedStyleName == "User Index Heading" ||
                            sConvertedStyleName == "Index Heading" )
                        {
                            // remove Left/RightMargin values from TOX heading styles
                            //left margin is set to NULL by default
                            xState->setPropertyToDefault(getPropertyName( PROP_PARA_LEFT_MARGIN ));
                        }
                        else if ( sConvertedStyleName == "Text body" )
                            xState->setPropertyToDefault(getPropertyName( PROP_PARA_BOTTOM_MARGIN ));
                        else if ( sConvertedStyleName == "Heading 1" ||
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

                    if ( !aPropValues.empty() )
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

                        try
                        {
                            uno::Reference< beans::XMultiPropertySet > xMultiPropertySet( xStyle, uno::UNO_QUERY_THROW);
                            try
                            {
                                xMultiPropertySet->setPropertyValues( aSortedPropVals.getNames(), aSortedPropVals.getValues() );
                            }
                            catch ( const uno::Exception& )
                            {
                                uno::Reference<beans::XPropertySet> xPropertySet(xStyle, uno::UNO_QUERY_THROW);
                                for ( const beans::PropertyValue& rValue : aSortedPropVals.getProperties() )
                                {
                                    try
                                    {
                                       xPropertySet->setPropertyValue( rValue.Name, rValue.Value );
                                    }
                                    catch ( const uno::Exception& )
                                    {
                                        SAL_WARN( "writerfilter", "StyleSheetTable::ApplyStyleSheets could not set property " << rValue.Name );
                                    }
                                }
                            }
                            // Duplicate MSWord's single footnote reference into Footnote Characters and Footnote anchor
                            if( pEntry->sStyleName.equalsIgnoreAsciiCase("footnote reference")
                                || pEntry->sStyleName.equalsIgnoreAsciiCase("endnote reference") )
                            {
                                uno::Reference< style::XStyle > xCopyStyle;
                                if( pEntry->sStyleName.equalsIgnoreAsciiCase("footnote reference") )
                                    xStyles->getByName( "Footnote anchor" ) >>= xCopyStyle;
                                else
                                    xStyles->getByName( "Endnote anchor" ) >>= xCopyStyle;

                                xMultiPropertySet.set( xCopyStyle, uno::UNO_QUERY_THROW);
                                xMultiPropertySet->setPropertyValues( aSortedPropVals.getNames(), aSortedPropVals.getValues() );
                            }
                        }
                        catch( const lang::WrappedTargetException& rWrapped)
                        {
#ifdef DBG_UTIL
                            OUString aMessage("StyleSheetTable::ApplyStyleSheets: Some style properties could not be set");
                            beans::UnknownPropertyException aUnknownPropertyException;

                            if (rWrapped.TargetException >>= aUnknownPropertyException)
                                aMessage += ": " + aUnknownPropertyException.Message;

                            SAL_WARN("writerfilter", aMessage);
#else
                            (void) rWrapped;
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
                        const OUString sParentStyle = xStyle->getParentStyle();
                        if( !sParentStyle.isEmpty() && !xStyles->hasByName( sParentStyle ) )
                            aMissingParent.emplace_back( sParentStyle, xStyle );

                        xStyles->insertByName( sConvertedStyleName, uno::Any( xStyle) );
                    }

                    beans::PropertyValues aGrabBag = pEntry->GetInteropGrabBagSeq();
                    uno::Reference<beans::XPropertySet> xPropertySet(xStyle, uno::UNO_QUERY);
                    if (aGrabBag.hasElements())
                    {
                        xPropertySet->setPropertyValue("StyleInteropGrabBag", uno::Any(aGrabBag));
                    }

                    // Only paragraph styles support automatic updates.
                    if (pEntry->bAutoRedefine && bParaStyle)
                        xPropertySet->setPropertyValue("IsAutoUpdate", uno::Any(true));
                }
                else if(pEntry->nStyleTypeCode == STYLE_TYPE_TABLE)
                {
                    // If this is a table style, save its contents as-is for roundtrip purposes.
                    TableStyleSheetEntry* pTableEntry = static_cast<TableStyleSheetEntry *>(pEntry.get());
                    aTableStylesVec.push_back(pTableEntry->GetInteropGrabBag());

                    // if DocDefaults exist, MS Word includes these in the table style definition.
                    pEntry->pProperties->InsertProps( m_pImpl->m_pDefaultCharProps, /*bOverwrite=*/false );
                    pEntry->pProperties->InsertProps( m_pImpl->m_pDefaultParaProps, /*bOverwrite=*/false );
                }
            }

            // Update the styles that were created before their parents or next-styles
            for( auto const & iter : aMissingParent )
            {
                iter.second->setParentStyle( iter.first );
            }

            for( auto const & iter : aMissingFollow )
            {
                try
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(iter.second, uno::UNO_QUERY);
                    xPropertySet->setPropertyValue( "FollowStyle", uno::Any(iter.first) );
                }
                catch( uno::Exception & ) {}
            }

            // Update the styles that were created before their linked styles.
            for (auto const& rLinked : aMissingLink)
            {
                try
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(rLinked.second,
                                                                     uno::UNO_QUERY);
                    xPropertySet->setPropertyValue("LinkStyle", uno::Any(rLinked.first));
                }
                catch (uno::Exception&)
                {
                    TOOLS_WARN_EXCEPTION(
                        "writerfilter",
                        "StyleSheetTable::ApplyStyleSheets: failed to set LinkStyle");
                }
            }

            if (!aTableStylesVec.empty())
            {
                // If we had any table styles, add a new document-level InteropGrabBag entry for them.
                uno::Reference<beans::XPropertySet> xPropertySet(m_pImpl->m_xTextDocument, uno::UNO_QUERY);
                uno::Any aAny = xPropertySet->getPropertyValue("InteropGrabBag");
                auto aGrabBag = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(aAny.get< uno::Sequence<beans::PropertyValue> >());
                beans::PropertyValue aValue;
                aValue.Name = "tableStyles";
                aValue.Value <<= comphelper::containerToSequence(aTableStylesVec);
                aGrabBag.push_back(aValue);
                xPropertySet->setPropertyValue("InteropGrabBag", uno::Any(comphelper::containerToSequence(aGrabBag)));
            }
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("writerfilter", "Styles could not be imported completely");
    }
}


StyleSheetEntryPtr StyleSheetTable::FindStyleSheetByISTD(const OUString& sIndex)
{
    auto findIt = m_pImpl->m_aStyleSheetEntriesMap.find(sIndex);
    if (findIt != m_pImpl->m_aStyleSheetEntriesMap.end())
        return findIt->second;
    return StyleSheetEntryPtr();
}


StyleSheetEntryPtr StyleSheetTable::FindStyleSheetByConvertedStyleName(std::u16string_view sIndex)
{
    StyleSheetEntryPtr pRet;
    for(const StyleSheetEntryPtr & rpEntry : m_pImpl->m_aStyleSheetEntries)
    {
        if( rpEntry->sConvertedStyleName == sIndex)
        {
            pRet = rpEntry;
            break;
        }
    }
    return pRet;
}


StyleSheetEntryPtr StyleSheetTable::FindDefaultParaStyle()
{
    return FindStyleSheetByISTD( m_pImpl->m_sDefaultParaStyleName );
}

const StyleSheetEntryPtr & StyleSheetTable::GetCurrentEntry() const
{
    return m_pImpl->m_pCurrentEntry;
}

OUString StyleSheetTable::ConvertStyleName( const OUString& rWWName, bool bExtendedSearch)
{
    OUString sRet( rWWName );
    if( bExtendedSearch )
    {
        //search for the rWWName in the IdentifierD of the existing styles and convert the sStyleName member
        auto findIt = m_pImpl->m_aStyleSheetEntriesMap.find(rWWName);
        if (findIt != m_pImpl->m_aStyleSheetEntriesMap.end())
        {
            if (!findIt->second->sConvertedStyleName.isEmpty())
                return findIt->second->sConvertedStyleName;
            sRet = findIt->second->sStyleName;
        }
    }

    // create a map only once
    // This maps Word's special style manes to Writer's (the opposite to what MSWordStyles::GetWWId
    // and ww::GetEnglishNameFromSti do on export). The mapping gives a Writer's style name, which
    // will point to a style with specific RES_POOL* in its m_nPoolFormatId. Then on export, the
    // pool format id will map to a ww::sti enum value, and finally to a Word style name. Keep this
    // part in sync with the export functions mentioned above!
    // In addition to "standard" names, some case variations are handled here; and also there are
    // a number of strange mappings like "BodyTextIndentItalic" -> "Text body indent italic", which
    // map something unused in Word to something unused in Writer :-/
    static const std::map< OUString, OUString> StyleNameMap {
        { "Normal", "Standard" }, // RES_POOLCOLL_STANDARD
        { "heading 1", "Heading 1" }, // RES_POOLCOLL_HEADLINE1
        { "heading 2", "Heading 2" }, // RES_POOLCOLL_HEADLINE2
        { "heading 3", "Heading 3" }, // RES_POOLCOLL_HEADLINE3
        { "heading 4", "Heading 4" }, // RES_POOLCOLL_HEADLINE4
        { "heading 5", "Heading 5" }, // RES_POOLCOLL_HEADLINE5
        { "heading 6", "Heading 6" }, // RES_POOLCOLL_HEADLINE6
        { "heading 7", "Heading 7" }, // RES_POOLCOLL_HEADLINE7
        { "heading 8", "Heading 8" }, // RES_POOLCOLL_HEADLINE8
        { "heading 9", "Heading 9" }, // RES_POOLCOLL_HEADLINE9
        { "Heading 1", "Heading 1" }, // RES_POOLCOLL_HEADLINE1
        { "Heading 2", "Heading 2" }, // RES_POOLCOLL_HEADLINE2
        { "Heading 3", "Heading 3" }, // RES_POOLCOLL_HEADLINE3
        { "Heading 4", "Heading 4" }, // RES_POOLCOLL_HEADLINE4
        { "Heading 5", "Heading 5" }, // RES_POOLCOLL_HEADLINE5
        { "Heading 6", "Heading 6" }, // RES_POOLCOLL_HEADLINE6
        { "Heading 7", "Heading 7" }, // RES_POOLCOLL_HEADLINE7
        { "Heading 8", "Heading 8" }, // RES_POOLCOLL_HEADLINE8
        { "Heading 9", "Heading 9" }, // RES_POOLCOLL_HEADLINE9
        { "Index 1", "Index 1" }, // RES_POOLCOLL_TOX_IDX1
        { "Index 2", "Index 2" }, // RES_POOLCOLL_TOX_IDX2
        { "Index 3", "Index 3" }, // RES_POOLCOLL_TOX_IDX3
//        { "Index 4", "" },
//        { "Index 5", "" },
//        { "Index 6", "" },
//        { "Index 7", "" },
//        { "Index 8", "" },
//        { "Index 9", "" },
        { "TOC 1", "Contents 1" }, // RES_POOLCOLL_TOX_CNTNT1
        { "TOC 2", "Contents 2" }, // RES_POOLCOLL_TOX_CNTNT2
        { "TOC 3", "Contents 3" }, // RES_POOLCOLL_TOX_CNTNT3
        { "TOC 4", "Contents 4" }, // RES_POOLCOLL_TOX_CNTNT4
        { "TOC 5", "Contents 5" }, // RES_POOLCOLL_TOX_CNTNT5
        { "TOC 6", "Contents 6" }, // RES_POOLCOLL_TOX_CNTNT6
        { "TOC 7", "Contents 7" }, // RES_POOLCOLL_TOX_CNTNT7
        { "TOC 8", "Contents 8" }, // RES_POOLCOLL_TOX_CNTNT8
        { "TOC 9", "Contents 9" }, // RES_POOLCOLL_TOX_CNTNT9
        { "TOC Heading", "Contents Heading" }, // RES_POOLCOLL_TOX_CNTNTH
        { "TOCHeading", "Contents Heading" }, // RES_POOLCOLL_TOX_CNTNTH
        { "toc 1", "Contents 1" }, // RES_POOLCOLL_TOX_CNTNT1
        { "toc 2", "Contents 2" }, // RES_POOLCOLL_TOX_CNTNT2
        { "toc 3", "Contents 3" }, // RES_POOLCOLL_TOX_CNTNT3
        { "toc 4", "Contents 4" }, // RES_POOLCOLL_TOX_CNTNT4
        { "toc 5", "Contents 5" }, // RES_POOLCOLL_TOX_CNTNT5
        { "toc 6", "Contents 6" }, // RES_POOLCOLL_TOX_CNTNT6
        { "toc 7", "Contents 7" }, // RES_POOLCOLL_TOX_CNTNT7
        { "toc 8", "Contents 8" }, // RES_POOLCOLL_TOX_CNTNT8
        { "toc 9", "Contents 9" }, // RES_POOLCOLL_TOX_CNTNT9
        { "TOC1", "Contents 1" }, // RES_POOLCOLL_TOX_CNTNT1
        { "TOC2", "Contents 2" }, // RES_POOLCOLL_TOX_CNTNT2
        { "TOC3", "Contents 3" }, // RES_POOLCOLL_TOX_CNTNT3
        { "TOC4", "Contents 4" }, // RES_POOLCOLL_TOX_CNTNT4
        { "TOC5", "Contents 5" }, // RES_POOLCOLL_TOX_CNTNT5
        { "TOC6", "Contents 6" }, // RES_POOLCOLL_TOX_CNTNT6
        { "TOC7", "Contents 7" }, // RES_POOLCOLL_TOX_CNTNT7
        { "TOC8", "Contents 8" }, // RES_POOLCOLL_TOX_CNTNT8
        { "TOC9", "Contents 9" }, // RES_POOLCOLL_TOX_CNTNT9
//        { "Normal Indent", "" },
        { "footnote text", "Footnote" }, // RES_POOLCOLL_FOOTNOTE
        { "Footnote Text", "Footnote" }, // RES_POOLCOLL_FOOTNOTE
        { "Annotation Text", "Marginalia" }, // RES_POOLCOLL_MARGINAL
        { "Header", "Header" }, // RES_POOLCOLL_HEADER
        { "header", "Header" }, // RES_POOLCOLL_HEADER
        { "Footer", "Footer" }, // RES_POOLCOLL_FOOTER
        { "footer", "Footer" }, // RES_POOLCOLL_FOOTER
        { "Index Heading", "Index Heading" }, // RES_POOLCOLL_TOX_IDXH
        { "Caption", "Caption" }, // RES_POOLCOLL_LABEL
        { "Table of Figures", "Drawing" }, // RES_POOLCOLL_LABEL_DRAWING
        { "Envelope Address", "Addressee" }, // RES_POOLCOLL_ENVELOPE_ADDRESS
        { "Envelope Return", "Sender" }, // RES_POOLCOLL_SEND_ADDRESS
        { "footnote reference", "Footnote Symbol" }, // RES_POOLCHR_FOOTNOTE; tdf#82173
        { "Footnote Reference", "Footnote Symbol" }, // RES_POOLCHR_FOOTNOTE; tdf#82173
//        { "Annotation Reference", "" },
        { "Line Number", "Line numbering" }, // RES_POOLCHR_LINENUM
        { "Page Number", "Page Number" }, // RES_POOLCHR_PAGENO
        { "endnote reference", "Endnote Symbol" }, // RES_POOLCHR_ENDNOTE; tdf#82173
        { "Endnote Reference", "Endnote Symbol" }, // RES_POOLCHR_ENDNOTE; tdf#82173
        { "endnote text", "Endnote" }, // RES_POOLCOLL_ENDNOTE
        { "Endnote Text", "Endnote" }, // RES_POOLCOLL_ENDNOTE
        { "Table of Authorities", "Bibliography Heading" }, // RES_POOLCOLL_TOX_AUTHORITIESH
//        { "Macro Text", "" },
//        { "TOA Heading", "" },
        { "List", "List" }, // RES_POOLCOLL_NUMBER_BULLET_BASE
//        { "List 2", "" },
//        { "List 3", "" },
//        { "List 4", "" },
//        { "List 5", "" },
        { "List Bullet", "List 1" }, // RES_POOLCOLL_BULLET_LEVEL1
        { "List Bullet 2", "List 2" }, // RES_POOLCOLL_BULLET_LEVEL2
        { "List Bullet 3", "List 3" }, // RES_POOLCOLL_BULLET_LEVEL3
        { "List Bullet 4", "List 4" }, // RES_POOLCOLL_BULLET_LEVEL4
        { "List Bullet 5", "List 5" }, // RES_POOLCOLL_BULLET_LEVEL5
        { "List Number", "Numbering 1" }, // RES_POOLCOLL_NUM_LEVEL1
        { "List Number 2", "Numbering 2" }, // RES_POOLCOLL_NUM_LEVEL2
        { "List Number 3", "Numbering 3" }, // RES_POOLCOLL_NUM_LEVEL3
        { "List Number 4", "Numbering 4" }, // RES_POOLCOLL_NUM_LEVEL4
        { "List Number 5", "Numbering 5" }, // RES_POOLCOLL_NUM_LEVEL5
        { "Title", "Title" }, // RES_POOLCOLL_DOC_TITLE
        { "Closing", "Appendix" }, // RES_POOLCOLL_DOC_APPENDIX
        { "Signature", "Signature" }, // RES_POOLCOLL_SIGNATURE
//        { "Default Paragraph Font", "" },
        { "DefaultParagraphFont", "Default Paragraph Font" },
        { "Body Text", "Text body" }, // RES_POOLCOLL_TEXT
        { "BodyText", "Text body" }, // RES_POOLCOLL_TEXT
        { "BodyTextIndentItalic", "Text body indent italic" },
        { "Body Text Indent", "Text body indent" }, // RES_POOLCOLL_TEXT_MOVE
        { "BodyTextIndent", "Text body indent" }, // RES_POOLCOLL_TEXT_MOVE
        { "BodyTextIndent2", "Text body indent2" },
        { "List Continue", "List 1 Cont." }, // RES_POOLCOLL_BULLET_NONUM1
        { "List Continue 2", "List 2 Cont." }, // RES_POOLCOLL_BULLET_NONUM2
        { "List Continue 3", "List 3 Cont." }, // RES_POOLCOLL_BULLET_NONUM3
        { "List Continue 4", "List 4 Cont." }, // RES_POOLCOLL_BULLET_NONUM4
        { "List Continue 5", "List 5 Cont." }, // RES_POOLCOLL_BULLET_NONUM5
//        { "Message Header", "" },
        { "Subtitle", "Subtitle" }, // RES_POOLCOLL_DOC_SUBTITLE
        { "Salutation", "Salutation" }, // RES_POOLCOLL_GREETING
//        { "Date", "" },
        { "Body Text First Indent", "First line indent" }, // RES_POOLCOLL_TEXT_IDENT
//        { "Body Text First Indent 2", "" },
//        { "Note Heading", "" },
//        { "Body Text 2", "" },
//        { "Body Text 3", "" },
//        { "Body Text Indent 2", "" },
//        { "Body Text Indent 3", "" },
//        { "Block Text", "" },
        { "Hyperlink", "Internet link" }, // RES_POOLCHR_INET_NORMAL
        { "FollowedHyperlink", "Visited Internet Link" }, // RES_POOLCHR_INET_VISIT
        { "Strong", "Strong Emphasis" }, // RES_POOLCHR_HTML_STRONG
        { "Emphasis", "Emphasis" }, // RES_POOLCHR_HTML_EMPHASIS
//        { "Document Map", "" },
//        { "Plain Text", "" },
        { "NoList", "No List" },
        { "AbstractHeading", "Abstract Heading" },
        { "AbstractBody", "Abstract Body" },
        { "PageNumber", "Page Number" }, // RES_POOLCHR_PAGENO
        { "TableNormal", "Normal Table" },
        { "DocumentMap", "Document Map" },
    };

    // find style-name using map
    if (const auto aIt = StyleNameMap.find(sRet); aIt != StyleNameMap.end())
    {
        sRet = aIt->second;
    }
    else
    {
        // Style names which should not be used without a " (user)" suffix
        static const o3tl::sorted_vector<OUString> ReservedStyleNames = [] {
            o3tl::sorted_vector<OUString> set;
            for (const auto& pair : StyleNameMap)
                set.insert(pair.second);
            return set;
        }();
        // Similar to SwStyleNameMapper convention (where a " (user)" suffix is used to
        // disambiguate user styles with reserved names in localization where respective
        // built-in styles have different UI names), we add a " (WW)" suffix here. Unlike
        // the " (user)" suffix, it is not hidden from the UI; it will be handled when
        // exported to Word formats - see MSWordStyles::BuildWwNames.
        // We can't use the " (user)" suffix, because that system is built upon the assumption
        // that UI names of respective built-in styles are different from the user style name.
        // That is not necessarily true here, since the current localization may not change
        // the UI names of built-in styles.
        if (ReservedStyleNames.find(sRet) != ReservedStyleNames.end() || sRet.endsWith(" (WW)"))
            sRet += " (WW)";
    }

    return sRet;
}

void StyleSheetTable::applyDefaults(bool bParaProperties)
{
    try{

        if (!m_pImpl->m_bIsNewDoc)
        {
            // tdf#72942: do not corrupts original styles in master document
            // during inserting of text from second document
            return;
        }

        if(!m_pImpl->m_xTextDefaults.is())
        {
            m_pImpl->m_xTextDefaults.set(
                m_pImpl->m_rDMapper.GetTextFactory()->createInstance("com.sun.star.text.Defaults"),
                uno::UNO_QUERY_THROW );
        }

        // WARNING: these defaults only take effect IF there is a DocDefaults style section. Normally there is, but not always.
        if( bParaProperties && m_pImpl->m_pDefaultParaProps)
        {
            // tdf#87533 LO will have different defaults here, depending on the locale. Import with documented defaults
            SetDefaultParaProps(PROP_WRITING_MODE, uno::Any(sal_Int16(text::WritingMode_LR_TB)));
            SetDefaultParaProps(PROP_PARA_ADJUST, uno::Any(sal_Int16(style::ParagraphAdjust_LEFT)));

            // Widow/Orphan -> set both to two if not already set
            uno::Any aTwo(sal_Int8(2));
            SetDefaultParaProps(PROP_PARA_WIDOWS, aTwo);
            SetDefaultParaProps(PROP_PARA_ORPHANS, aTwo);

            uno::Reference<style::XStyleFamiliesSupplier> xStylesSupplier(m_pImpl->m_xTextDocument, uno::UNO_QUERY);
            uno::Reference<container::XNameAccess> xStyleFamilies = xStylesSupplier->getStyleFamilies();
            uno::Reference<container::XNameAccess> xParagraphStyles;
            xStyleFamilies->getByName("ParagraphStyles") >>= xParagraphStyles;
            uno::Reference<beans::XPropertySet> xDefault;
            // This is the built-in default style that every style inherits from
            xParagraphStyles->getByName("Paragraph style") >>= xDefault;

            const uno::Sequence< beans::PropertyValue > aPropValues = m_pImpl->m_pDefaultParaProps->GetPropertyValues();
            for( const auto& rPropValue : aPropValues )
            {
                try
                {
                    xDefault->setPropertyValue(rPropValue.Name, rPropValue.Value);
                }
                catch( const uno::Exception& )
                {
                    TOOLS_WARN_EXCEPTION( "writerfilter", "setPropertyValue");
                }
            }
        }
        if( !bParaProperties && m_pImpl->m_pDefaultCharProps )
        {
            // tdf#108350: Earlier in DomainMapper for DOCX, Calibri/11pt was set to match MSWord 2007+,
            // but that is valid only if DocDefaults_rPrDefault is omitted.
            // Now that DocDefaults_rPrDefault is known, the defaults should be reset to Times New Roman/10pt.
            if ( m_pImpl->m_rDMapper.IsOOXMLImport() )
                m_pImpl->m_xTextDefaults->setPropertyValue( getPropertyName(PROP_CHAR_FONT_NAME), css::uno::Any(OUString("Times New Roman")) );

            const uno::Sequence< beans::PropertyValue > aPropValues = m_pImpl->m_pDefaultCharProps->GetPropertyValues();
            for( const auto& rPropValue : aPropValues )
            {
                try
                {
                    m_pImpl->m_xTextDefaults->setPropertyValue( rPropValue.Name, rPropValue.Value );
                }
                catch( const uno::Exception& )
                {
                    TOOLS_WARN_EXCEPTION( "writerfilter", "exception");
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

    //create a new one otherwise
    const uno::Reference< container::XNameContainer >& xCharStyles = m_pImpl->m_rDMapper.GetCharacterStyles();
    sListLabel = m_pImpl->m_rDMapper.GetUnusedCharacterStyleName();
    uno::Reference< lang::XMultiServiceFactory > xDocFactory( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
    try
    {
        uno::Reference< style::XStyle > xStyle( xDocFactory->createInstance(
            getPropertyName( PROP_SERVICE_CHAR_STYLE )), uno::UNO_QUERY_THROW);
        uno::Reference< beans::XPropertySet > xStyleProps(xStyle, uno::UNO_QUERY_THROW );
        for( const auto& rCharProp : rCharProperties)
        {
            try
            {
                xStyleProps->setPropertyValue( rCharProp.Name, rCharProp.Value );
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION( "writerfilter", "StyleSheetTable::getOrCreateCharStyle - Style::setPropertyValue");
            }
        }
        xCharStyles->insertByName( sListLabel, uno::Any( xStyle) );
        m_pImpl->m_aListCharStylePropertyVector.emplace_back( sListLabel, std::vector(rCharProperties) );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "writerfilter", "StyleSheetTable::getOrCreateCharStyle");
    }

    return sListLabel;
}

}//namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
