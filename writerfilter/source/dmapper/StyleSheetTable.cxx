/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <map>
#include <stdio.h>
#include <rtl/ustrbuf.hxx>

#ifdef DEBUG_DOMAINMAPPER
#include <dmapperLoggers.hxx>
#include <resourcemodel/QNameToString.hxx>
#endif

using namespace ::com::sun::star;
namespace writerfilter {
namespace dmapper
{

typedef ::std::map< ::rtl::OUString, ::rtl::OUString> StringPairMap_t;

/*-- 21.06.2006 07:34:44---------------------------------------------------

  -----------------------------------------------------------------------*/
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
#if OSL_DEBUG_LEVEL > 1
            nStyleTypeCode = STYLE_TYPE_PARA;
#endif
}

StyleSheetEntry::~StyleSheetEntry()
{
}

#ifdef DEBUG_DOMAINMAPPER
XMLTag::Pointer_t StyleSheetEntry::toTag()
{
    XMLTag::Pointer_t pResult(new XMLTag("StyleSheetEntry"));
    
    pResult->addAttr("identifierI", sStyleIdentifierI);
    pResult->addAttr("identifierD", sStyleIdentifierD);
    pResult->addAttr("default", bIsDefaultStyle ? "true" : "false");
    pResult->addAttr("invalidHeight", bInvalidHeight ? "true" : "false");
    pResult->addAttr("hasUPE", bHasUPE ? "true" : "false");
    pResult->addAttr("styleType", nStyleTypeCode);
    pResult->addAttr("baseStyle", sBaseStyleIdentifier);
    pResult->addAttr("nextStyle", sNextStyleIdentifier);
    pResult->addAttr("styleName", sStyleName);
    pResult->addAttr("styleName1", sStyleName1);
    pResult->addAttr("convertedName", sConvertedStyleName);
    pResult->addTag(pProperties->toTag());
    
    return pResult;
}
#endif

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

    m_nColBandSize = 1;
    m_nRowBandSize = 1;
}

TableStyleSheetEntry::~TableStyleSheetEntry( )
{
    m_pStyleSheet = NULL;
}

void TableStyleSheetEntry::AddTblStylePr( TblStyleType nType, PropertyMapPtr pProps )
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("AddTblStylePr");
    dmapper_logger->attribute("type", nType);
    dmapper_logger->addTag(pProps->toTag());
    dmapper_logger->endElement("AddTblStylePr");
#endif

    static TblStyleType pTypesToFix[] =
    {
        TBL_STYLE_FIRSTROW,
        TBL_STYLE_LASTROW,
        TBL_STYLE_FIRSTCOL,
        TBL_STYLE_LASTCOL
    };

    static PropertyIds pPropsToCheck[] =
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
            PropertyMap::iterator pCheckedIt = pProps->find( PropertyDefinition( nChecked, false )  );

            PropertyIds nInsideProp = ( i < 2 ) ? META_PROP_HORIZONTAL_BORDER : META_PROP_VERTICAL_BORDER;
            PropertyMap::iterator pInsideIt = pProps->find( PropertyDefinition( nInsideProp, false )  );

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

PropertyMapPtr TableStyleSheetEntry::GetProperties( sal_Int32 nMask )
{
    PropertyMapPtr pProps( new PropertyMap );

    // First get the parent properties
    StyleSheetEntryPtr pEntry = m_pStyleSheet->FindParentStyleSheet( sBaseStyleIdentifier );

    if ( pEntry.get( ) )
    {
        TableStyleSheetEntry* pParent = static_cast<TableStyleSheetEntry *>( pEntry.get( ) );
        pProps->insert( pParent->GetProperties( nMask ) );
    }
    
    // And finally get the mask ones
    pProps->insert( GetLocalPropertiesFromMask( nMask ) );

    return pProps;
}

#ifdef DEBUG_DOMAINMAPPER
XMLTag::Pointer_t TableStyleSheetEntry::toTag()
{
    XMLTag::Pointer_t pResult(StyleSheetEntry::toTag());
    
    for (sal_Int32 nBit = 0; nBit < 13; ++nBit)
    {
        PropertyMapPtr pMap = GetProperties(1 << nBit);
        
        XMLTag::Pointer_t pTag = pMap->toTag();
        pTag->addAttr("kind", nBit);
        pResult->addTag(pTag);        
    }
    
    return pResult;
}
#endif

void lcl_mergeProps( PropertyMapPtr pToFill,  PropertyMapPtr pToAdd, TblStyleType nStyleId )
{
    static PropertyIds pPropsToCheck[] =
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
        PropertyDefinition aProp( nId, false );
        PropertyMap::iterator pIt = pToAdd->find( aProp );
   
        if ( pIt != pToAdd->end( ) )
        {
            PropertyMap::iterator pDestIt = pToFill->find( aProp );
            
            if ( pRemoveInside[i] )
            {
                // Remove the insideH and insideV depending on the cell pos
                PropertyIds nInsideProp = ( i < 2 ) ? META_PROP_HORIZONTAL_BORDER : META_PROP_VERTICAL_BORDER;
                pDestIt = pToFill->find( PropertyDefinition( nInsideProp, false ) );
                if ( pDestIt != pToFill->end( ) )
                    pToFill->erase( pDestIt );
            }
        }    
    }

    pToFill->insert( pToAdd );
}

PropertyMapPtr TableStyleSheetEntry::GetLocalPropertiesFromMask( sal_Int32 nMask )
{
    // Order from right to left
    static TblStyleType aBitsOrder[] = 
    {
        TBL_STYLE_SWCELL,
        TBL_STYLE_SECELL,
        TBL_STYLE_NWCELL,
        TBL_STYLE_NECELL,
        TBL_STYLE_BAND2HORZ,
        TBL_STYLE_BAND1HORZ,
        TBL_STYLE_BAND2VERT,
        TBL_STYLE_BAND1VERT,
        TBL_STYLE_LASTCOL,
        TBL_STYLE_FIRSTCOL,
        TBL_STYLE_LASTROW,
        TBL_STYLE_FIRSTROW,
        TBL_STYLE_UNKNOWN
    };

    // Get the properties applying according to the mask
    PropertyMapPtr pProps( new PropertyMap( ) );
    short nBit = 0;
    do
    {
        TblStyleType nStyleId = aBitsOrder[nBit];
        TblStylePrs::iterator pIt = m_aStyles.find( nStyleId );

        short nTestBit = 1 << nBit;
        sal_Int32 nBitMask = sal_Int32( nTestBit );
        if ( ( nMask & nBitMask ) && ( pIt != m_aStyles.end( ) ) )
            lcl_mergeProps( pProps, pIt->second, nStyleId );

        nBit++;
    }
    while ( nBit < 13 );

    return pProps;
}

/*-- 06.02.2008 11:30:46---------------------------------------------------

  -----------------------------------------------------------------------*/
struct ListCharStylePropertyMap_t
{
    ::rtl::OUString         sCharStyleName;
    PropertyValueVector_t   aPropertyValues;

    ListCharStylePropertyMap_t(const ::rtl::OUString& rCharStyleName, const PropertyValueVector_t& rPropertyValues):
        sCharStyleName( rCharStyleName ),
        aPropertyValues( rPropertyValues )
        {}
};
typedef std::vector< ListCharStylePropertyMap_t > ListCharStylePropertyVector_t;
/*-- 19.06.2006 12:04:32---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    
    
    StyleSheetTable_Impl(DomainMapper& rDMapper, uno::Reference< text::XTextDocument> xTextDocument);

    ::rtl::OUString HasListCharStyle( const PropertyValueVector_t& rCharProperties );
};
/*-- 15.11.2007 08:30:02---------------------------------------------------

  -----------------------------------------------------------------------*/
StyleSheetTable_Impl::StyleSheetTable_Impl(DomainMapper& rDMapper, uno::Reference< text::XTextDocument> xTextDocument ) :
            m_rDMapper( rDMapper ),
            m_xTextDocument( xTextDocument ),
            m_pCurrentEntry(),
            m_pDefaultParaProps(new PropertyMap),
            m_pDefaultCharProps(new PropertyMap)
{
    //set font height default to 10pt
    uno::Any aVal = uno::makeAny( double(10.) );
    m_pDefaultCharProps->Insert( PROP_CHAR_HEIGHT, true, aVal );
    m_pDefaultCharProps->Insert( PROP_CHAR_HEIGHT_ASIAN, true, aVal );
    m_pDefaultCharProps->Insert( PROP_CHAR_HEIGHT_COMPLEX, true, aVal );
}
/*-- 06.02.2008 11:45:21---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString StyleSheetTable_Impl::HasListCharStyle( const PropertyValueVector_t& rPropValues )
{
    ::rtl::OUString sRet;
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
/*-- 19.06.2006 12:04:32---------------------------------------------------

  -----------------------------------------------------------------------*/
StyleSheetTable::StyleSheetTable(DomainMapper& rDMapper, uno::Reference< text::XTextDocument> xTextDocument) :
    m_pImpl( new StyleSheetTable_Impl(rDMapper, xTextDocument) )
{
}
/*-- 19.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
StyleSheetTable::~StyleSheetTable()
{
    delete m_pImpl;
}
/*-- 19.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void StyleSheetTable::attribute(Id Name, Value & val)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("StyleSheetTable.attribute");
    dmapper_logger->attribute("name", (*QNameToString::Instance())(Name));
    dmapper_logger->attribute("value", val.toString());
#endif

    OSL_ENSURE( m_pImpl->m_pCurrentEntry, "current entry has to be set here");
    if(!m_pImpl->m_pCurrentEntry)
        return ;
    int nIntValue = val.getInt();
    (void)nIntValue;
    ::rtl::OUString sValue = val.getString();
//    printf ( "StyleSheetTable::attribute(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)Name, (unsigned int)nIntValue, ::rtl::OUStringToOString(sValue, RTL_TEXTENCODING_DONTKNOW).getStr());
    /* WRITERFILTERSTATUS: table: StyleSheetTable_attributedata */
    switch(Name)
    {
        case NS_rtf::LN_ISTD:
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->sStyleIdentifierD = ::rtl::OUString::valueOf(static_cast<sal_Int32>(nIntValue), 16);
        break;
        case NS_rtf::LN_STI:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        {
            ::rtl::OUString tempStyleIdentifier = GetStyleIdFromIndex(static_cast<sal_uInt32>(nIntValue));
            if (tempStyleIdentifier.getLength())
                m_pImpl->m_pCurrentEntry->sStyleIdentifierI = tempStyleIdentifier;
            if (nIntValue == 0 || nIntValue == 65)
                m_pImpl->m_pCurrentEntry->bIsDefaultStyle = true;
        }
        break;
        case NS_rtf::LN_SGC:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->nStyleTypeCode = (StyleType)nIntValue;
        break;
        case NS_rtf::LN_ISTDBASE:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            if (static_cast<sal_uInt32>(nIntValue) != 0xfff)
                m_pImpl->m_pCurrentEntry->sBaseStyleIdentifier = ::rtl::OUString::valueOf(static_cast<sal_Int32>(nIntValue), 16);
        break;
        case NS_rtf::LN_ISTDNEXT:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            if (static_cast<sal_uInt32>(nIntValue) != 0xfff)
                m_pImpl->m_pCurrentEntry->sNextStyleIdentifier = ::rtl::OUString::valueOf(static_cast<sal_Int32>(nIntValue), 16);
        break;
        case NS_rtf::LN_FSCRATCH:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FINVALHEIGHT:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FHASUPE:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FMASSCOPY:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_CUPX:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_BCHUPE:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FAUTOREDEF:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FHIDDEN:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_UNUSED8_3:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //noone seems to care about it
        break;
        case NS_rtf::LN_XSTZNAME:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->sStyleName1 = sValue;
            if (m_pImpl->m_pCurrentEntry->sStyleIdentifierI.getLength())
                m_pImpl->m_pCurrentEntry->sStyleIdentifierI = sValue;
        break;
        case NS_rtf::LN_XSTZNAME1:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->sStyleName = sValue;
            if (m_pImpl->m_pCurrentEntry->sStyleIdentifierI.getLength())
                m_pImpl->m_pCurrentEntry->sStyleIdentifierI = sValue;
        break;
        case NS_rtf::LN_UPX:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            resolveAttributeProperties(val);
        break;
        case NS_ooxml::LN_CT_Style_type:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        {
/*          defaults should be set at the service "com.sun.star.text.Defaults"          
             if (nIntValue == 1)
                *(m_pImpl->m_pCurrentEntry->pProperties) = *(m_pImpl->m_pDefaultParaProps);
            else if (nIntValue == 2)
                *(m_pImpl->m_pCurrentEntry->pProperties) = *(m_pImpl->m_pDefaultCharProps);*/
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
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->bIsDefaultStyle = (nIntValue != 0);
        break;
        case NS_ooxml::LN_CT_Style_customStyle:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        break;
        case NS_ooxml::LN_CT_Style_styleId:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->sStyleIdentifierI = sValue;
            m_pImpl->m_pCurrentEntry->sStyleIdentifierD = sValue;
        break;
        case NS_ooxml::LN_CT_TblWidth_w:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            dynamic_cast< StyleSheetPropertyMap* >( m_pImpl->m_pCurrentEntry->pProperties.get() )->SetCT_TblWidth_w( nIntValue );
        break;
        case NS_ooxml::LN_CT_TblWidth_type:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
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
    
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("StyleSheetTable.attribute");
#endif
}
/*-- 19.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void StyleSheetTable::sprm(Sprm & rSprm)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("StyleSheetTable.sprm");
    dmapper_logger->attribute("sprm", rSprm.toString());
#endif

    sal_uInt32 nSprmId = rSprm.getId();
    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue.get() ? pValue->getInt() : 0;
    (void)nIntValue;
    rtl::OUString sStringValue = pValue.get() ? pValue->getString() : rtl::OUString();
    //printf ( "StyleSheetTable::sprm(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)nSprmId, (unsigned int)nIntValue, ::rtl::OUStringToOString(sStringValue, RTL_TEXTENCODING_DONTKNOW).getStr());
   
    /* WRITERFILTERSTATUS: table: StyleSheetTable_sprm */
    switch(nSprmId)
    {
        case NS_ooxml::LN_CT_Style_name:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            //this is only a UI name!
            m_pImpl->m_pCurrentEntry->sStyleName = sStringValue;
            m_pImpl->m_pCurrentEntry->sStyleName1 = sStringValue;
            break;      
        case NS_ooxml::LN_CT_Style_basedOn:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->m_pCurrentEntry->sBaseStyleIdentifier = sStringValue;
            break;
        case NS_ooxml::LN_CT_Style_next:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
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
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        break;
        case NS_ooxml::LN_CT_Style_tblPr: //contains table properties
        case NS_ooxml::LN_CT_Style_tblStylePr: //contains  to table properties
        case NS_ooxml::LN_CT_DocDefaults_rPrDefault:
        case NS_ooxml::LN_CT_TblPrBase_tblInd: //table properties - at least width value and type
        case NS_ooxml::LN_EG_RPrBase_rFonts: //table fonts
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
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
                    pEntry->pProperties->insert(pProps);
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
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pDefaultParaProps );
            m_pImpl->m_rDMapper.sprm( rSprm );
            m_pImpl->m_rDMapper.PopStyleSheetProperties();
            applyDefaults( true );
        break;
        case NS_ooxml::LN_CT_RPrDefault_rPr:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pDefaultCharProps );
            m_pImpl->m_rDMapper.sprm( rSprm );
            m_pImpl->m_rDMapper.PopStyleSheetProperties();
            applyDefaults( false );
        break;
        case NS_ooxml::LN_CT_TblPrBase_jc:     //table alignment - row properties!
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
             m_pImpl->m_pCurrentEntry->pProperties->Insert( PROP_HORI_ORIENT, false, 
                uno::makeAny( ConversionHelper::convertTableJustification( nIntValue )));
        break;
        case NS_ooxml::LN_CT_TrPrBase_jc:     //table alignment - row properties!
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
                dynamic_cast< StyleSheetPropertyMap* >( m_pImpl->m_pCurrentEntry->pProperties.get() )->SetCT_TrPrBase_jc(nIntValue);
        break;
        case NS_ooxml::LN_CT_TblPrBase_tblBorders: //table borders, might be defined in table style
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        {    
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                BorderHandlerPtr pBorderHandler( new BorderHandler(m_pImpl->m_rDMapper.IsOOXMLImport()) );
                pProperties->resolve(*pBorderHandler);
                m_pImpl->m_pCurrentEntry->pProperties->insert( pBorderHandler->getProperties(), true );
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
        case NS_ooxml::LN_CT_TblPrBase_tblCellMar:
            //no cell margins in styles
        break;
        case NS_ooxml::LN_CT_Style_pPr:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            // no break
        case NS_ooxml::LN_CT_Style_rPr:
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
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
                    m_pImpl->m_rDMapper.sprm( rSprm, pProps );

                    m_pImpl->m_pCurrentEntry->pProperties->insert(pProps);
                    
                    m_pImpl->m_rDMapper.PopStyleSheetProperties( );
                }
            }
            break;
    }
    
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("StyleSheetTable.sprm");
#endif
}
/*-- 19.06.2006 12:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void StyleSheetTable::entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("StyleSheetTable.entry");
#endif

    //create a new style entry
    // printf("StyleSheetTable::entry(...)\n");
    OSL_ENSURE( !m_pImpl->m_pCurrentEntry, "current entry has to be NULL here");
    StyleSheetEntryPtr pNewEntry( new StyleSheetEntry );
    m_pImpl->m_pCurrentEntry = pNewEntry;
    m_pImpl->m_rDMapper.PushStyleSheetProperties( m_pImpl->m_pCurrentEntry->pProperties );
    ref->resolve(*this);
    //append it to the table
    m_pImpl->m_rDMapper.PopStyleSheetProperties();
    if( !m_pImpl->m_rDMapper.IsOOXMLImport() || m_pImpl->m_pCurrentEntry->sStyleName.getLength() >0)
    {
        m_pImpl->m_pCurrentEntry->sConvertedStyleName = ConvertStyleName( m_pImpl->m_pCurrentEntry->sStyleName );
        m_pImpl->m_aStyleSheetEntries.push_back( m_pImpl->m_pCurrentEntry );
    }
    else
    {
        //TODO: this entry contains the default settings - they have to be added to the settings
    }    
    
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->addTag(m_pImpl->m_pCurrentEntry->toTag());
#endif

    StyleSheetEntryPtr pEmptyEntry;
    m_pImpl->m_pCurrentEntry = pEmptyEntry;
    
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("StyleSheetTable.entry");
#endif
}
/*-- 21.06.2006 15:34:49---------------------------------------------------
    sorting helper
  -----------------------------------------------------------------------*/
typedef std::vector< beans::PropertyValue > _PropValVector;
class PropValVector : public _PropValVector
{
public:
    PropValVector(){}

    void    Insert( beans::PropertyValue aVal );
    uno::Sequence< uno::Any > getValues();
    uno::Sequence< ::rtl::OUString > getNames();
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
uno::Sequence< ::rtl::OUString > PropValVector::getNames()
{
    uno::Sequence< ::rtl::OUString > aRet( size() );
    ::rtl::OUString* pNames = aRet.getArray();
    sal_Int32 nVal = 0;
    _PropValVector::iterator aIt = begin();
    while(aIt != end())
    {
        pNames[nVal++] = aIt->Name;
        ++aIt;
    }
    return aRet;
}
/*-- 21.06.2006 13:35:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void StyleSheetTable::ApplyStyleSheets( FontTablePtr rFontTable )
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("applyStyleSheets");
#endif

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
                    ::rtl::OUString sConvertedStyleName = ConvertStyleName( pEntry->sStyleName );
                    if(xStyles->hasByName( sConvertedStyleName ))
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
                    if( pEntry->sBaseStyleIdentifier.getLength() )
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
                            OSL_ENSURE( false, "Styles parent could not be set");
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
    //                      font size to 240 twip (12 pts) for all if not set
                            pEntry->pProperties->Insert(PROP_CHAR_HEIGHT, true, aTwoHundredFortyTwip, false);
    //                      western font not already set -> apply first font
                            const FontEntry::Pointer_t pWesternFontEntry(rFontTable->getFontEntry( 0 ));
                            rtl::OUString sWesternFontName = pWesternFontEntry->sFontName;
                            pEntry->pProperties->Insert(PROP_CHAR_FONT_NAME, true, uno::makeAny( sWesternFontName ), false);

    //                      CJK  ... apply second font
                            const FontEntry::Pointer_t pCJKFontEntry(rFontTable->getFontEntry( 2 ));
                            pEntry->pProperties->Insert(PROP_CHAR_FONT_NAME_ASIAN, true, uno::makeAny( pCJKFontEntry->sFontName ), false);
                            pEntry->pProperties->Insert(PROP_CHAR_HEIGHT_ASIAN, true, aTwoHundredFortyTwip, false);
    //                      CTL  ... apply third font, if available
                            if( nFontCount > 3 )
                            {
                                const FontEntry::Pointer_t pCTLFontEntry(rFontTable->getFontEntry( 3 ));
                                pEntry->pProperties->Insert(PROP_CHAR_FONT_NAME_COMPLEX, true, uno::makeAny( pCTLFontEntry->sFontName ), false);
                                pEntry->pProperties->Insert(PROP_CHAR_HEIGHT_COMPLEX, true, aTwoHundredFortyTwip, false);
                            }
                        }
    //                  Widow/Orphan -> set both to two if not already set
                        uno::Any aTwo = uno::makeAny(sal_Int8(2));
                        pEntry->pProperties->Insert(PROP_PARA_WIDOWS, true, aTwo, false);
                        pEntry->pProperties->Insert(PROP_PARA_ORPHANS, true, aTwo, false);
    //                  Left-to-right direction if not already set
                        pEntry->pProperties->Insert(PROP_WRITING_MODE, true, uno::makeAny( sal_Int16(text::WritingMode_LR_TB) ), false);
    //                  font color COL_AUTO if not already set
                        pEntry->pProperties->Insert(PROP_CHAR_COLOR, true, uno::makeAny( sal_Int32(0xffffffff) ), false);
                    }

                    uno::Sequence< beans::PropertyValue > aPropValues = pEntry->pProperties->GetPropertyValues();
                    bool bAddFollowStyle = false;
                    if(bParaStyle && !pEntry->sNextStyleIdentifier.getLength() )
                    {
                            bAddFollowStyle = true;
                    }
                    //remove Left/RightMargin values from TOX heading styles
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
                                        uno::makeAny( rtl::OUString::createFromAscii( "" ) ),
                                        beans::PropertyState_DIRECT_VALUE );
                                aPropValues[ aPropValues.getLength( ) - 1 ] = aStyleVal;
                            }
                        }

                        uno::Reference< beans::XPropertyState >xState( xStyle, uno::UNO_QUERY_THROW );
                        if( sConvertedStyleName.equalsAscii( "Contents Heading" ) ||
                            sConvertedStyleName.equalsAscii( "User Index Heading" ) ||
                            sConvertedStyleName.equalsAscii( "Index Heading" ))
                        {
                            //left margin is set to NULL by default
                            uno::Reference< beans::XPropertyState >xState1( xStyle, uno::UNO_QUERY_THROW );
                            xState1->setPropertyToDefault(rPropNameSupplier.GetName( PROP_PARA_LEFT_MARGIN ));
                        }
                        else if( sConvertedStyleName.equalsAscii( "Text body" ) )
                            xState->setPropertyToDefault(rPropNameSupplier.GetName( PROP_PARA_BOTTOM_MARGIN ));
                        else if( sConvertedStyleName.equalsAscii( "Heading 1" ) ||
                                sConvertedStyleName.equalsAscii( "Heading 2" ) ||
                                sConvertedStyleName.equalsAscii( "Heading 3" ) ||
                                sConvertedStyleName.equalsAscii( "Heading 4" ) ||
                                sConvertedStyleName.equalsAscii( "Heading 5" ) ||
                                sConvertedStyleName.equalsAscii( "Heading 6" ) ||
                                sConvertedStyleName.equalsAscii( "Heading 7" ) ||
                                sConvertedStyleName.equalsAscii( "Heading 8" ) ||
                                sConvertedStyleName.equalsAscii( "Heading 9" ) )
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
                    }

                    if(bAddFollowStyle || aPropValues.getLength())
                    {
                        PropValVector aSortedPropVals;
                        for( sal_Int32 nProp = 0; nProp < aPropValues.getLength(); ++nProp)
                        {
#ifdef DEBUG_DOMAINMAPPER
                            dmapper_logger->startElement("propvalue");
                            dmapper_logger->attribute("name", aPropValues[nProp].Name);
                            dmapper_logger->attribute("value", aPropValues[nProp].Value);
#endif
                                // Don't add the style name properties
                            bool bIsParaStyleName = aPropValues[nProp].Name.equalsAscii( "ParaStyleName" );
                            bool bIsCharStyleName = aPropValues[nProp].Name.equalsAscii( "CharStyleName" );
                            if ( !bIsParaStyleName && !bIsCharStyleName )
                            {
#ifdef DEBUG_DOMAINMAPPER
                                dmapper_logger->element("insert");
#endif
                                aSortedPropVals.Insert( aPropValues[nProp] );
                            }
#ifdef DEBUG_DOMAINMAPPER
                            dmapper_logger->endElement("propvalue");
#endif
                        }
                        if(bAddFollowStyle)
                        {
                            //find the name of the Next style
                            std::vector< StyleSheetEntryPtr >::iterator aNextStyleIt = m_pImpl->m_aStyleSheetEntries.begin();
                            for( ; aNextStyleIt !=  m_pImpl->m_aStyleSheetEntries.end(); ++aNextStyleIt )
                            {
                                if( ( *aNextStyleIt )->sStyleName.getLength() &&
                                        ( *aNextStyleIt )->sStyleName == pEntry->sNextStyleIdentifier)
                                {
                                    beans::PropertyValue aNew;
                                    aNew.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FollowStyle"));
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
                            rtl::OString aMessage("Some style properties could not be set");
#if OSL_DEBUG_LEVEL > 0                            
                            beans::UnknownPropertyException aUnknownPropertyException;
                            
                            if( rWrapped.TargetException >>= aUnknownPropertyException )
                            {
                                aMessage += rtl::OString(": " );
                                ::rtl::OString sTemp;
                                aUnknownPropertyException.Message.convertToString(&sTemp, RTL_TEXTENCODING_ASCII_US, 0 );
                                aMessage += sTemp;
                            }
#endif
                            OSL_ENSURE( false, aMessage.getStr());
                        }
                        catch( const uno::Exception& rEx)
                        {
                            (void) rEx;
                            OSL_ENSURE( false, "Some style properties could not be set");
                        }
                    }
                    if(bInsert)
                    {
                        xStyles->insertByName( sConvertedStyleName, uno::makeAny( xStyle) );
#ifdef DEBUG_DOMAINMAPPER
                        uno::Reference<beans::XPropertySet> xProps(xStyle, uno::UNO_QUERY);
                        dmapper_logger->startElement("insertStyle");
                        dmapper_logger->addTag(unoPropertySetToTag(xProps));
                        dmapper_logger->endElement("insertStyle");
#endif
                    }
                }
                ++aIt;
            }
        }
    }
    catch( uno::Exception& rEx)
    {
        (void)rEx;
        OSL_ENSURE( false, "Styles could not be imported completely");
    }

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("applyStyleSheets");
#endif
}
/*-- 22.06.2006 15:56:56---------------------------------------------------

  -----------------------------------------------------------------------*/
const StyleSheetEntryPtr StyleSheetTable::FindStyleSheetByISTD(const ::rtl::OUString& sIndex)
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
/*-- 28.12.2007 14:45:45---------------------------------------------------

  -----------------------------------------------------------------------*/
const StyleSheetEntryPtr StyleSheetTable::FindStyleSheetByStyleName(const ::rtl::OUString& sIndex)
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
/*-- 28.12.2007 14:45:45---------------------------------------------------

  -----------------------------------------------------------------------*/
const StyleSheetEntryPtr StyleSheetTable::FindStyleSheetByConvertedStyleName(const ::rtl::OUString& sIndex)
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

/*-- 17.07.2006 11:47:00---------------------------------------------------

  -----------------------------------------------------------------------*/
const StyleSheetEntryPtr StyleSheetTable::FindParentStyleSheet(::rtl::OUString sBaseStyle)
{
    if( !sBaseStyle.getLength() )
    {
        StyleSheetEntryPtr pEmptyPtr;
        return pEmptyPtr;
    }
    if( m_pImpl->m_pCurrentEntry)
        sBaseStyle = m_pImpl->m_pCurrentEntry->sBaseStyleIdentifier;

    return FindStyleSheetByISTD( sBaseStyle );
}
/*-- 21.12.2006 15:58:23---------------------------------------------------

  -----------------------------------------------------------------------*/
static const sal_Char *aStyleNamePairs[] =
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


::rtl::OUString StyleSheetTable::ConvertStyleName( const ::rtl::OUString& rWWName, bool bExtendedSearch)
{
    ::rtl::OUString sRet( rWWName );
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
                    ::rtl::OUString::createFromAscii(aStyleNamePairs[2 * nPair]),
                    ::rtl::OUString::createFromAscii(aStyleNamePairs[2 * nPair + 1]) ));
        }
    }
    StringPairMap_t::iterator aIt = m_pImpl->m_aStyleNameMap.find( sRet );
    if(aIt != m_pImpl->m_aStyleNameMap.end() && aIt->second.getLength())
        sRet = aIt->second;
    return sRet;
}

::rtl::OUString StyleSheetTable::GetStyleIdFromIndex(const sal_uInt32 sti)
{
    ::rtl::OUString sRet;
    if (sti >= (sizeof(aStyleNamePairs) / sizeof( sal_Char*) / 2))
        sRet = ::rtl::OUString();
    else
        sRet = ::rtl::OUString::createFromAscii(aStyleNamePairs[2 * sti]);
    return sRet;
}

void StyleSheetTable::resolveSprmProps(Sprm & rSprm)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
    if( pProperties.get())
        pProperties->resolve(*this);
}

void StyleSheetTable::resolveAttributeProperties(Value & val)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = val.getProperties();
    if( pProperties.get())
        pProperties->resolve(*this);
}
/*-- 18.07.2007 15:59:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void StyleSheetTable::applyDefaults(bool bParaProperties)
{
    try{
        if(!m_pImpl->m_xTextDefaults.is())
        {
            m_pImpl->m_xTextDefaults = uno::Reference< beans::XPropertySet>( 
                m_pImpl->m_rDMapper.GetTextFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Defaults"))), 
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
                    OSL_ENSURE( false, "setPropertyValue exception");
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
                    OSL_ENSURE( false, "setPropertyValue exception");
                }    
            }
        }
    }
    catch( const uno::Exception& )
    {
    }    
}
/*-- 05.02.2008 10:27:36---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString StyleSheetTable::getOrCreateCharStyle( PropertyValueVector_t& rCharProperties )
{
    //find out if any of the styles already has the required properties then return it's name
    ::rtl::OUString sListLabel = m_pImpl->HasListCharStyle(rCharProperties);
    if( sListLabel.getLength() )
        return sListLabel;
    const char cListLabel[] = "ListLabel ";
    uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
    uno::Reference<container::XNameContainer> xCharStyles;
    xStyleFamilies->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharacterStyles"))) >>= xCharStyles;
    //search for all character styles with the name sListLabel + <index>
    sal_Int32 nStyleFound = 0;
    uno::Sequence< ::rtl::OUString > aStyleNames = xCharStyles->getElementNames();
    const ::rtl::OUString* pStyleNames = aStyleNames.getConstArray();
    for( sal_Int32 nStyle = 0; nStyle < aStyleNames.getLength(); ++nStyle )
    {
        if( pStyleNames[nStyle].matchAsciiL( cListLabel, sizeof( cListLabel ) - 1  ))
        {
            ::rtl::OUString sSuffix = pStyleNames[nStyle].copy( sizeof( cListLabel ) - 1 );
            sal_Int32 nSuffix = sSuffix.toInt32();
            if( nSuffix > 0 )
            {
                if( nSuffix > nStyleFound )
                    nStyleFound = nSuffix;
            }
        }    
    }
    sListLabel = ::rtl::OUString::createFromAscii( cListLabel );
    sListLabel += ::rtl::OUString::valueOf( ++nStyleFound );
    //create a new one otherwise
    uno::Reference< lang::XMultiServiceFactory > xDocFactory( m_pImpl->m_xTextDocument, uno::UNO_QUERY_THROW );
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    try
    {
        uno::Reference< style::XStyle > xStyle( xDocFactory->createInstance(
            rPropNameSupplier.GetName( PROP_SERVICE_CHAR_STYLE )), uno::UNO_QUERY_THROW);
        //uno::Reference< container::XNamed >xNamed( xStyle, uno::UNO_QUERY_THROW );
        //xNamed->setName( sListLabel );
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
                OSL_ENSURE( false, "Exception in StyleSheetTable::getOrCreateCharStyle - Style::setPropertyValue");
            }    
            ++aCharPropIter;
        }
        xCharStyles->insertByName( sListLabel, uno::makeAny( xStyle) );
        m_pImpl->m_aListCharStylePropertyVector.push_back( ListCharStylePropertyMap_t( sListLabel, rCharProperties ));
    }
    catch( const uno::Exception& rEx )
    {
        (void)rEx;
        OSL_ENSURE( false, "Exception in StyleSheetTable::getOrCreateCharStyle");
    }    
    
    return sListLabel;
}

}//namespace dmapper
}//namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
