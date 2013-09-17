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
#include <resourcemodel/ResourceModelHelper.hxx>
#include <StyleSheetTable.hxx>
#include <dmapper/DomainMapper.hxx>
#include <NumberingManager.hxx>
#include <ConversionHelper.hxx>
#include <TblStylePrHandler.hxx>
#include <BorderHandler.hxx>
#include <doctok/resourceids.hxx>
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
#include <stdio.h>
#include <rtl/ustrbuf.hxx>

#include <dmapperLoggers.hxx>

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
    m_pStyleSheet = NULL;
}

void TableStyleSheetEntry::AddTblStylePr( TblStyleType nType, PropertyMapPtr pProps )
{
    static const TblStyleType pTypesToFix[] =
    {
        TBL_STYLE_FIRSTROW,
        TBL_STYLE_LASTROW,
        TBL_STYLE_FIRSTCOL,
        TBL_STYLE_LASTCOL
    };

    static const PropertyIds pPropsToCheck[] =
    {
        PROP_BOTTOM_BORDER,
        PROP_TOP_BORDER,
        PROP_RIGHT_BORDER,
        PROP_LEFT_BORDER
    };

    int i = 0;
    while ( i < 4 )
    {
        if ( nType == pTypesToFix[i] )
        {
            PropertyIds nChecked = pPropsToCheck[i];
            PropertyMap::iterator pCheckedIt = pProps->find( PropertyDefinition( nChecked )  );

            PropertyIds nInsideProp = ( i < 2 ) ? META_PROP_HORIZONTAL_BORDER : META_PROP_VERTICAL_BORDER;
            PropertyMap::iterator pInsideIt = pProps->find( PropertyDefinition( nInsideProp )  );

            bool bHasChecked = pCheckedIt != pProps->end( );
            bool bHasInside = pInsideIt != pProps->end( );

            if ( bHasChecked && bHasInside )
            {
                // In this case, remove the inside border
                pProps->erase( pInsideIt );
            }

            i = 4; // Stop looping stupidly
        }
        i++;
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
        if (pStack.get() == NULL)
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
        PropertyDefinition aProp( nId );
        PropertyMap::iterator pIt = pToAdd->find( aProp );

        if ( pIt != pToAdd->end( ) )
        {
            PropertyMap::iterator pDestIt = pToFill->find( aProp );

            if ( pRemoveInside[i] )
            {
                // Remove the insideH and insideV depending on the cell pos
                PropertyIds nInsideProp = ( i < 2 ) ? META_PROP_HORIZONTAL_BORDER : META_PROP_VERTICAL_BORDER;
                pDestIt = pToFill->find( PropertyDefinition( nInsideProp ) );
                if ( pDestIt != pToFill->end( ) )
                    pToFill->erase( pDestIt );
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
    ListCharStylePropertyVector_t           m_aListCharStylePropertyVector;
    bool                                    m_bIsNewDoc;

    StyleSheetTable_Impl(DomainMapper& rDMapper, uno::Reference< text::XTextDocument> xTextDocument, bool bIsNewDoc);

    OUString HasListCharStyle( const PropertyValueVector_t& rCharProperties );
};


StyleSheetTable_Impl::StyleSheetTable_Impl(DomainMapper& rDMapper, uno::Reference< text::XTextDocument> xTextDocument, bool bIsNewDoc ) :
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


StyleSheetTable::StyleSheetTable(DomainMapper& rDMapper, uno::Reference< text::XTextDocument> xTextDocument, bool bIsNewDoc)
: LoggedProperties(dmapper_logger, "StyleSheetTable")
, LoggedTable(dmapper_logger, "StyleSheetTable")
, m_pImpl( new StyleSheetTable_Impl(rDMapper, xTextDocument, bIsNewDoc) )
{
}


StyleSheetTable::~StyleSheetTable()
{
    delete m_pImpl;
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
    (void)nIntValue;
    OUString sValue = val.getString();

    // The default type is paragraph, and it needs to be processed first,
    // because the NS_ooxml::LN_CT_Style_type handling may set m_pImpl->m_pCurrentEntry
    // to point to a different object.
    if( m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_UNKNOWN )
    {
        if( Name != NS_rtf::LN_SGC && Name != NS_ooxml::LN_CT_Style_type )
            m_pImpl->m_pCurrentEntry->nStyleTypeCode = STYLE_TYPE_PARA;
    }
    switch(Name)
    {
        case NS_rtf::LN_ISTD:
            m_pImpl->m_pCurrentEntry->sStyleIdentifierD = OUString::number(nIntValue, 16);
        break;
        case NS_rtf::LN_STI:
        {
            OUString tempStyleIdentifier = GetStyleIdFromIndex(static_cast<sal_uInt32>(nIntValue));
            if (!tempStyleIdentifier.isEmpty())
                m_pImpl->m_pCurrentEntry->sStyleIdentifierI = tempStyleIdentifier;
            if (nIntValue == 0 || nIntValue == 65)
                m_pImpl->m_pCurrentEntry->bIsDefaultStyle = true;
        }
        break;
        case NS_rtf::LN_SGC:
            SAL_WARN_IF( m_pImpl->m_pCurrentEntry->nStyleTypeCode != STYLE_TYPE_UNKNOWN,
                "writerfilter", "Style type needs to be processed first" );
            m_pImpl->m_pCurrentEntry->nStyleTypeCode = (StyleType)nIntValue;
        break;
        case NS_rtf::LN_ISTDBASE:
            if (static_cast<sal_uInt32>(nIntValue) != 0xfff)
                m_pImpl->m_pCurrentEntry->sBaseStyleIdentifier = OUString::number(nIntValue, 16);
        break;
        case NS_rtf::LN_ISTDNEXT:
            if (static_cast<sal_uInt32>(nIntValue) != 0xfff)
                m_pImpl->m_pCurrentEntry->sNextStyleIdentifier = OUString::number(nIntValue, 16);
        break;
        case NS_rtf::LN_FSCRATCH:
        case NS_rtf::LN_FINVALHEIGHT:
        case NS_rtf::LN_FHASUPE:
        case NS_rtf::LN_FMASSCOPY:
        case NS_rtf::LN_CUPX:
        case NS_rtf::LN_BCHUPE:
        case NS_rtf::LN_FAUTOREDEF:
        case NS_rtf::LN_FHIDDEN:
        case NS_rtf::LN_UNUSED8_3:
            //noone seems to care about it
        break;
        case NS_rtf::LN_XSTZNAME:
            m_pImpl->m_pCurrentEntry->sStyleName1 = sValue;
            if (!m_pImpl->m_pCurrentEntry->sStyleIdentifierI.isEmpty())
                m_pImpl->m_pCurrentEntry->sStyleIdentifierI = sValue;
        break;
        case NS_rtf::LN_XSTZNAME1:
            m_pImpl->m_pCurrentEntry->sStyleName = sValue;
            if (!m_pImpl->m_pCurrentEntry->sStyleIdentifierI.isEmpty())
                m_pImpl->m_pCurrentEntry->sStyleIdentifierI = sValue;
        break;
        case NS_rtf::LN_UPX:
            resolveAttributeProperties(val);
        break;
        case NS_ooxml::LN_CT_Style_type:
        {
            SAL_WARN_IF( m_pImpl->m_pCurrentEntry->nStyleTypeCode != STYLE_TYPE_UNKNOWN,
                "writerfilter", "Style type needs to be processed first" );
            StyleType nType = ( StyleType ) nIntValue;
            if ( nType == STYLE_TYPE_TABLE )
            {
                StyleSheetEntryPtr pEntry = m_pImpl->m_pCurrentEntry;
                TableStyleSheetEntryPtr pTableEntry( new TableStyleSheetEntry( *pEntry.get( ), this ) );
                m_pImpl->m_pCurrentEntry = pTableEntry;
            }
            else
                m_pImpl->m_pCurrentEntry->nStyleTypeCode = (StyleType)nIntValue;
        }
        break;
        case NS_ooxml::LN_CT_Style_default:
            m_pImpl->m_pCurrentEntry->bIsDefaultStyle = (nIntValue != 0);
        break;
        case NS_ooxml::LN_CT_Style_customStyle:
        break;
        case NS_ooxml::LN_CT_Style_styleId:
            m_pImpl->m_pCurrentEntry->sStyleIdentifierI = sValue;
            m_pImpl->m_pCurrentEntry->sStyleIdentifierD = sValue;
        break;
        case NS_ooxml::LN_CT_TblWidth_w:
            dynamic_cast< StyleSheetPropertyMap* >( m_pImpl->m_pCurrentEntry->pProperties.get() )->SetCT_TblWidth_w( nIntValue );
        break;
        case NS_ooxml::LN_CT_TblWidth_type:
            dynamic_cast< StyleSheetPropertyMap* >( m_pImpl->m_pCurrentEntry->pProperties.get() )->SetCT_TblWidth_type( nIntValue );
        break;
        default:
        {
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->element("unhandled");
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
    (void)nIntValue;
    OUString sStringValue = pValue.get() ? pValue->getString() : OUString();

    switch(nSprmId)
    {
        case NS_ooxml::LN_CT_Style_name:
            //this is only a UI name!
            m_pImpl->m_pCurrentEntry->sStyleName = sStringValue;
            m_pImpl->m_pCurrentEntry->sStyleName1 = sStringValue;
            break;
        case NS_ooxml::LN_CT_Style_basedOn:
            m_pImpl->m_pCurrentEntry->sBaseStyleIdentifier = sStringValue;
            break;
        case NS_ooxml::LN_CT_Style_next:
            m_pImpl->m_pCurrentEntry->sNextStyleIdentifier = sStringValue;
            break;
        case NS_ooxml::LN_CT_Style_aliases:
        case NS_ooxml::LN_CT_Style_link:
        case NS_ooxml::LN_CT_Style_autoRedefine:
        case NS_ooxml::LN_CT_Style_hidden:
        case NS_ooxml::LN_CT_Style_uiPriority:
        case NS_ooxml::LN_CT_Style_semiHidden:
        case NS_ooxml::LN_CT_Style_unhideWhenUsed:
        case NS_ooxml::LN_CT_Style_qFormat:
        case NS_ooxml::LN_CT_Style_locked:
        case NS_ooxml::LN_CT_Style_personal:
        case NS_ooxml::LN_CT_Style_personalCompose:
        case NS_ooxml::LN_CT_Style_personalReply:
        case NS_ooxml::LN_CT_Style_rsid:
        case NS_ooxml::LN_CT_Style_trPr:
        case NS_ooxml::LN_CT_Style_tcPr:
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

                if (nType == TBL_STYLE_UNKNOWN)
                {
                    pEntry->pProperties->InsertProps(pProps);
                }
                else
                {
                    TableStyleSheetEntry * pTableEntry = dynamic_cast<TableStyleSheetEntry*>( pEntry );
                    if (pTableEntry != NULL)
                        pTableEntry->AddTblStylePr( nType, pProps );
                }
            }
            break;
        }
        case NS_ooxml::LN_CT_PPrDefault_pPr:
        case NS_ooxml::LN_CT_DocDefaults_pPrDefault:
            m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pDefaultParaProps );
            resourcemodel::resolveSprmProps( m_pImpl->m_rDMapper, rSprm );
            m_pImpl->m_rDMapper.PopStyleSheetProperties();
            applyDefaults( true );
        break;
        case NS_ooxml::LN_CT_RPrDefault_rPr:
        case NS_ooxml::LN_CT_DocDefaults_rPrDefault:
            m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pDefaultCharProps );
            resourcemodel::resolveSprmProps( m_pImpl->m_rDMapper, rSprm );
            m_pImpl->m_rDMapper.PopStyleSheetProperties();
            applyDefaults( false );
        break;
        case NS_ooxml::LN_CT_TblPrBase_jc:     //table alignment - row properties!
             m_pImpl->m_pCurrentEntry->pProperties->Insert( PROP_HORI_ORIENT,
                uno::makeAny( ConversionHelper::convertTableJustification( nIntValue )));
        break;
        case NS_ooxml::LN_CT_TrPrBase_jc:     //table alignment - row properties!
                dynamic_cast< StyleSheetPropertyMap* >( m_pImpl->m_pCurrentEntry->pProperties.get() )->SetCT_TrPrBase_jc(nIntValue);
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
        case NS_ooxml::LN_CT_Style_pPr:
            // no break
        case NS_ooxml::LN_CT_Style_rPr:
            // no break
        default:
            {
                if (!m_pImpl->m_pCurrentEntry)
                    break;

                TablePropertiesHandlerPtr pTblHandler( new TablePropertiesHandler( true ) );
                pTblHandler->SetProperties( m_pImpl->m_pCurrentEntry->pProperties );
                if ( !pTblHandler->sprm( rSprm ) )
                {
                    m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pCurrentEntry->pProperties );

                    PropertyMapPtr pProps(new PropertyMap());
                    bool bTableStyleRunProps = m_pImpl->m_pCurrentEntry->nStyleTypeCode == STYLE_TYPE_TABLE && nSprmId == NS_ooxml::LN_CT_Style_rPr;
                    if (bTableStyleRunProps)
                        m_pImpl->m_rDMapper.setInTableStyleRunProps(true);
                    m_pImpl->m_rDMapper.sprmWithProps( rSprm, pProps );
                    if (bTableStyleRunProps)
                        m_pImpl->m_rDMapper.setInTableStyleRunProps(false);

                    m_pImpl->m_pCurrentEntry->pProperties->InsertProps(pProps);

                    m_pImpl->m_rDMapper.PopStyleSheetProperties( );
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

    StyleSheetEntryPtr pEmptyEntry;
    m_pImpl->m_pCurrentEntry = pEmptyEntry;
}
/*-------------------------------------------------------------------------
    sorting helper
  -----------------------------------------------------------------------*/
typedef std::vector< beans::PropertyValue > _PropValVector;
class PropValVector : public _PropValVector
{
public:
    PropValVector(){}

    void    Insert( beans::PropertyValue aVal );
    uno::Sequence< uno::Any > getValues();
    uno::Sequence< OUString > getNames();
};
void    PropValVector::Insert( beans::PropertyValue aVal )
{
    _PropValVector::iterator aIt = begin();
    while(aIt != end())
    {
        if(aIt->Name > aVal.Name)
        {
            insert( aIt, aVal );
            return;
        }
        ++aIt;
    }
    push_back( aVal );
}
uno::Sequence< uno::Any > PropValVector::getValues()
{
    uno::Sequence< uno::Any > aRet( size() );
    uno::Any* pValues = aRet.getArray();
    sal_Int32 nVal = 0;
    _PropValVector::iterator aIt = begin();
    while(aIt != end())
    {
        pValues[nVal++] = aIt->Value;
        ++aIt;
    }
    return aRet;
}
uno::Sequence< OUString > PropValVector::getNames()
{
    uno::Sequence< OUString > aRet( size() );
    OUString* pNames = aRet.getArray();
    sal_Int32 nVal = 0;
    _PropValVector::iterator aIt = begin();
    while(aIt != end())
    {
        pNames[nVal++] = aIt->Name;
        ++aIt;
    }
    return aRet;
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

        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        xStyleFamilies->getByName(rPropNameSupplier.GetName( PROP_CHARACTER_STYLES )) >>= xCharStyles;
        xStyleFamilies->getByName(rPropNameSupplier.GetName( PROP_PARAGRAPH_STYLES )) >>= xParaStyles;
        if(xCharStyles.is() && xParaStyles.is())
        {
            std::vector< StyleSheetEntryPtr >::iterator aIt = m_pImpl->m_aStyleSheetEntries.begin();
            while( aIt != m_pImpl->m_aStyleSheetEntries.end() )
            {
                StyleSheetEntryPtr pEntry = *aIt;
                if( pEntry->nStyleTypeCode == STYLE_TYPE_CHAR || pEntry->nStyleTypeCode == STYLE_TYPE_PARA )
                {
                    bool bParaStyle = pEntry->nStyleTypeCode == STYLE_TYPE_PARA;
                    bool bInsert = false;
                    uno::Reference< container::XNameContainer > xStyles = bParaStyle ? xParaStyles : xCharStyles;
                    uno::Reference< style::XStyle > xStyle;
                    OUString sConvertedStyleName = ConvertStyleName( pEntry->sStyleName );

                    // When pasting, don't update existing styles.
                    if(xStyles->hasByName( sConvertedStyleName ) && m_pImpl->m_bIsNewDoc)
                        xStyles->getByName( sConvertedStyleName ) >>= xStyle;
                    else
                    {
                        bInsert = true;
                        xStyle = uno::Reference< style::XStyle >(xDocFactory->createInstance(
                                    bParaStyle ?
                                        rPropNameSupplier.GetName( PROP_SERVICE_PARA_STYLE ) :
                                        rPropNameSupplier.GetName( PROP_SERVICE_CHAR_STYLE )),
                                        uno::UNO_QUERY_THROW);
                    }
                    if( !pEntry->sBaseStyleIdentifier.isEmpty() )
                    {
                        try
                        {
                            //TODO: Handle cases where a paragraph <> character style relation is needed
                            StyleSheetEntryPtr pParent = FindStyleSheetByISTD( pEntry->sBaseStyleIdentifier );
                            if (pParent.get() != NULL)
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
                    }

                    uno::Sequence< beans::PropertyValue > aPropValues = pEntry->pProperties->GetPropertyValues();
                    bool bAddFollowStyle = false;
                    if(bParaStyle && pEntry->sNextStyleIdentifier.isEmpty() )
                    {
                            bAddFollowStyle = true;
                    }

                    // remove Left/RightMargin values from TOX heading styles
                    if( bParaStyle )
                    {
                        // Set the outline levels
                        const StyleSheetPropertyMap* pStyleSheetProperties = dynamic_cast<const StyleSheetPropertyMap*>(pEntry ? pEntry->pProperties.get() : 0);
                        if ( pStyleSheetProperties )
                        {
                            aPropValues.realloc( aPropValues.getLength( ) + 1 );

                            beans::PropertyValue aLvlVal( rPropNameSupplier.GetName( PROP_OUTLINE_LEVEL ), 0,
                                    uno::makeAny( sal_Int16( pStyleSheetProperties->GetOutlineLevel( ) + 1 ) ),
                                    beans::PropertyState_DIRECT_VALUE );
                            aPropValues[ aPropValues.getLength( ) - 1 ] = aLvlVal;

                            if ( pStyleSheetProperties->GetOutlineLevel( ) == 0 )
                            {
                                aPropValues.realloc( aPropValues.getLength( ) + 1 );
                                beans::PropertyValue aStyleVal( rPropNameSupplier.GetName( PROP_NUMBERING_STYLE_NAME ), 0,
                                        uno::makeAny( OUString() ),
                                        beans::PropertyState_DIRECT_VALUE );
                                aPropValues[ aPropValues.getLength( ) - 1 ] = aStyleVal;
                            }
                        }

                        uno::Reference< beans::XPropertyState >xState( xStyle, uno::UNO_QUERY_THROW );
                        if( sConvertedStyleName == "Contents Heading" ||
                            sConvertedStyleName == "User Index Heading" ||
                            sConvertedStyleName == "Index Heading" )
                        {
                            //left margin is set to NULL by default
                            uno::Reference< beans::XPropertyState >xState1( xStyle, uno::UNO_QUERY_THROW );
                            xState1->setPropertyToDefault(rPropNameSupplier.GetName( PROP_PARA_LEFT_MARGIN ));
                        }
                        else if ( sConvertedStyleName == "Text body" )
                            xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_PARA_BOTTOM_MARGIN ));
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
                            xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_WEIGHT ));
                            xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_WEIGHT_ASIAN ));
                            xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_WEIGHT_COMPLEX ));
                            xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_POSTURE ));
                            xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_POSTURE_ASIAN ));
                            xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_POSTURE_COMPLEX ));
                            xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_PROP_HEIGHT        ));
                            xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_PROP_HEIGHT_ASIAN  ));
                            xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_CHAR_PROP_HEIGHT_COMPLEX));
                        }
                        else if (sConvertedStyleName == "Title" || sConvertedStyleName == "Subtitle")
                        {
                            //set the default adjust for ParaStyle Title and Subtitle to left
                            try
                            {
                                uno::Reference< beans::XPropertySet > xProp( xStyle, uno::UNO_QUERY );
                                if( xProp.is() )
                                {
                                    uno::Any aMSDefaultVal = uno::makeAny( (sal_Int16)style::ParagraphAdjust_LEFT );
                                    xProp->setPropertyValue( rPropNameSupplier.GetName( PROP_PARA_ADJUST), aMSDefaultVal );
                                }
                            }
                            catch(...)
                            {
                                OSL_ENSURE( false, "Default ParaAdjust style property could not be set");
                            }
                        }
                    }

                    if(bAddFollowStyle || aPropValues.getLength())
                    {
                        PropValVector aSortedPropVals;
                        for( sal_Int32 nProp = 0; nProp < aPropValues.getLength(); ++nProp)
                        {
                            // Don't add the style name properties
                            bool bIsParaStyleName = aPropValues[nProp].Name == "ParaStyleName";
                            bool bIsCharStyleName = aPropValues[nProp].Name == "CharStyleName";
                            if ( !bIsParaStyleName && !bIsCharStyleName )
                            {
                                aSortedPropVals.Insert( aPropValues[nProp] );
                            }
                        }
                        if(bAddFollowStyle)
                        {
                            //find the name of the Next style
                            std::vector< StyleSheetEntryPtr >::iterator aNextStyleIt = m_pImpl->m_aStyleSheetEntries.begin();
                            for( ; aNextStyleIt !=  m_pImpl->m_aStyleSheetEntries.end(); ++aNextStyleIt )
                            {
                                if( !( *aNextStyleIt )->sStyleName.isEmpty() &&
                                        ( *aNextStyleIt )->sStyleName == pEntry->sNextStyleIdentifier)
                                {
                                    beans::PropertyValue aNew;
                                    aNew.Name = "FollowStyle";
                                    aNew.Value = uno::makeAny(ConvertStyleName( ( *aNextStyleIt )->sStyleIdentifierD ));
                                    aSortedPropVals.Insert( aNew );
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
                            OString aMessage("Some style properties could not be set");
#if OSL_DEBUG_LEVEL > 0
                            beans::UnknownPropertyException aUnknownPropertyException;

                            if( rWrapped.TargetException >>= aUnknownPropertyException )
                            {
                                aMessage += ": ";
                                OString sTemp;
                                aUnknownPropertyException.Message.convertToString(&sTemp, RTL_TEXTENCODING_ASCII_US, 0 );
                                aMessage += sTemp;
                            }
#endif
                            SAL_WARN("writerfilter", aMessage.getStr());
                        }
                        catch( const uno::Exception& rEx)
                        {
                            (void) rEx;
                            OSL_FAIL( "Some style properties could not be set");
                        }
                    }
                    if(bInsert)
                    {
                        xStyles->insertByName( sConvertedStyleName, uno::makeAny( xStyle) );
                    }
                }
                ++aIt;
            }
        }
    }
    catch( const uno::Exception& )
    {
        OSL_FAIL( "Styles could not be imported completely");
    }
}


const StyleSheetEntryPtr StyleSheetTable::FindStyleSheetByISTD(const OUString& sIndex)
{
    StyleSheetEntryPtr pRet;
    for( sal_uInt32 nPos = 0; nPos < m_pImpl->m_aStyleSheetEntries.size(); ++nPos )
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
    for( sal_uInt32 nPos = 0; nPos < m_pImpl->m_aStyleSheetEntries.size(); ++nPos )
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
    for( sal_uInt32 nPos = 0; nPos < m_pImpl->m_aStyleSheetEntries.size(); ++nPos )
    {
        if( m_pImpl->m_aStyleSheetEntries[nPos]->sConvertedStyleName == sIndex)
        {
            pRet = m_pImpl->m_aStyleSheetEntries[nPos];
            break;
        }
    }
    return pRet;
}



const StyleSheetEntryPtr StyleSheetTable::FindParentStyleSheet(OUString sBaseStyle)
{
    if( sBaseStyle.isEmpty() )
    {
        StyleSheetEntryPtr pEmptyPtr;
        return pEmptyPtr;
    }
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
    "TOC Heading",               "Contents Heading",
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
    "Strong",                    "Strong Emphasis",
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
        //TODO: performance issue - put styles list into a map sorted by it's sStyleIdentifierD members
        while( aIt != m_pImpl->m_aStyleSheetEntries.end() )
        {
            if( rWWName == ( *aIt )->sStyleIdentifierD )
                sRet = ( *aIt )->sStyleName;
            ++aIt;
        }
    }
    if(!m_pImpl->m_aStyleNameMap.size())
    {
        for( sal_uInt32 nPair = 0; nPair < sizeof(aStyleNamePairs) / sizeof( sal_Char*) / 2; ++nPair)
        {
                m_pImpl->m_aStyleNameMap.insert( StringPairMap_t::value_type(
                    OUString::createFromAscii(aStyleNamePairs[2 * nPair]),
                    OUString::createFromAscii(aStyleNamePairs[2 * nPair + 1]) ));
        }
    }
    StringPairMap_t::iterator aIt = m_pImpl->m_aStyleNameMap.find( sRet );
    if(aIt != m_pImpl->m_aStyleNameMap.end() && !aIt->second.isEmpty())
        sRet = aIt->second;
    return sRet;
}

OUString StyleSheetTable::GetStyleIdFromIndex(const sal_uInt32 sti)
{
    OUString sRet;
    if (sti >= (sizeof(aStyleNamePairs) / sizeof( sal_Char*) / 2))
        sRet = OUString();
    else
        sRet = OUString::createFromAscii(aStyleNamePairs[2 * sti]);
    return sRet;
}

void StyleSheetTable::resolveAttributeProperties(Value & val)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = val.getProperties();
    if( pProperties.get())
        pProperties->resolve(*this);
}


void StyleSheetTable::applyDefaults(bool bParaProperties)
{
    try{
        if(!m_pImpl->m_xTextDefaults.is())
        {
            m_pImpl->m_xTextDefaults = uno::Reference< beans::XPropertySet>(
                m_pImpl->m_rDMapper.GetTextFactory()->createInstance("com.sun.star.text.Defaults"),
                uno::UNO_QUERY_THROW );
        }
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        if( bParaProperties && m_pImpl->m_pDefaultParaProps.get() && m_pImpl->m_pDefaultParaProps->size())
        {
            PropertyMap::iterator aMapIter = m_pImpl->m_pDefaultParaProps->begin();
            for( ; aMapIter != m_pImpl->m_pDefaultParaProps->end(); ++aMapIter )
            {
                try
                {
                    m_pImpl->m_xTextDefaults->setPropertyValue(rPropNameSupplier.GetName( aMapIter->first.eId ), aMapIter->second);
                }
                catch( const uno::Exception& )
                {
                    OSL_FAIL( "setPropertyValue exception");
                }
            }
        }
        if( !bParaProperties && m_pImpl->m_pDefaultCharProps.get() && m_pImpl->m_pDefaultCharProps->size())
        {
            PropertyMap::iterator aMapIter = m_pImpl->m_pDefaultCharProps->begin();
            for( ; aMapIter != m_pImpl->m_pDefaultCharProps->end(); ++aMapIter )
            {
                try
                {
                    m_pImpl->m_xTextDefaults->setPropertyValue(rPropNameSupplier.GetName( aMapIter->first.eId ), aMapIter->second);
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


OUString StyleSheetTable::getOrCreateCharStyle( PropertyValueVector_t& rCharProperties )
{
    //find out if any of the styles already has the required properties then return it's name
    OUString sListLabel = m_pImpl->HasListCharStyle(rCharProperties);
    if( !sListLabel.isEmpty() )
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
        if( pStyleNames[nStyle].matchAsciiL( cListLabel, sizeof( cListLabel ) - 1  ))
        {
            OUString sSuffix = pStyleNames[nStyle].copy( sizeof( cListLabel ) - 1 );
            sal_Int32 nSuffix = sSuffix.toInt32();
            if( nSuffix > 0 )
            {
                if( nSuffix > nStyleFound )
                    nStyleFound = nSuffix;
            }
        }
    }
    sListLabel = OUString::createFromAscii( cListLabel );
    sListLabel += OUString::number( ++nStyleFound );
    //create a new one otherwise
    uno::Reference< lang::XMultiServiceFactory > xDocFactory( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    try
    {
        uno::Reference< style::XStyle > xStyle( xDocFactory->createInstance(
            rPropNameSupplier.GetName( PROP_SERVICE_CHAR_STYLE )), uno::UNO_QUERY_THROW);
        uno::Reference< beans::XPropertySet > xStyleProps(xStyle, uno::UNO_QUERY_THROW );
        PropertyValueVector_t::const_iterator aCharPropIter = rCharProperties.begin();
        while( aCharPropIter != rCharProperties.end())
        {
            try
            {
                xStyleProps->setPropertyValue( aCharPropIter->Name, aCharPropIter->Value );
            }
            catch( const uno::Exception& rEx )
            {
                (void)rEx;
                OSL_FAIL( "Exception in StyleSheetTable::getOrCreateCharStyle - Style::setPropertyValue");
            }
            ++aCharPropIter;
        }
        xCharStyles->insertByName( sListLabel, uno::makeAny( xStyle) );
        m_pImpl->m_aListCharStylePropertyVector.push_back( ListCharStylePropertyMap_t( sListLabel, rCharProperties ));
    }
    catch( const uno::Exception& rEx )
    {
        (void)rEx;
        OSL_FAIL( "Exception in StyleSheetTable::getOrCreateCharStyle");
    }

    return sListLabel;
}

}//namespace dmapper
}//namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
