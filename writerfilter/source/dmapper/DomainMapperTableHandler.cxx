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
#include <DomainMapperTableHandler.hxx>
#include <DomainMapper_Impl.hxx>
#include <StyleSheetTable.hxx>
#include <com/sun/star/table/TableBorderDistances.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <dmapperLoggers.hxx>

#ifdef DEBUG_DOMAINMAPPER
#include <PropertyMapHelper.hxx>
#endif

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;

#define DEF_BORDER_DIST 190  //0,19cm
#define DEFAULT_CELL_MARGIN 108 //default cell margin, not documented 

#ifdef DEBUG_DOMAINMAPPER
static void  lcl_printProperties( PropertyMapPtr pProps )
{
    if( pProps.get() )
    {
        dmapper_logger->startElement("properties");

        PropertyMap::const_iterator aMapIter = pProps->begin();
        PropertyMap::const_iterator aEndIter = pProps->end();
        PropertyNameSupplier& rPropSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        for( ; aMapIter != aEndIter; ++aMapIter )
        {
            rtl::OUString aOUStr = rPropSupplier.GetName( aMapIter->first.eId );
            rtl::OString aOStr(aOUStr.getStr(), aOUStr.getLength(),  RTL_TEXTENCODING_ASCII_US );
            clog << aOStr.getStr();

            table::BorderLine aLine;
            sal_Int32 nColor; 
            if ( aMapIter->second >>= aLine )
            {
                dmapper_logger->startElement("borderline");
                dmapper_logger->attribute("color", aLine.Color);
                dmapper_logger->attribute("inner", aLine.InnerLineWidth);
                dmapper_logger->attribute("outer", aLine.OuterLineWidth);
                dmapper_logger->endElement("borderline");
            }
            else if ( aMapIter->second >>= nColor )
            {
                dmapper_logger->startElement("color");
                dmapper_logger->attribute("number", nColor);
                dmapper_logger->endElement("color");
            }
        }
        
        dmapper_logger->endElement("properties");
    }
}
#endif

DomainMapperTableHandler::DomainMapperTableHandler(TextReference_t xText, DomainMapper_Impl& rDMapper_Impl)
    : m_xText(xText),
        m_rDMapper_Impl( rDMapper_Impl ),
        m_nCellIndex(0),
        m_nRowIndex(0)
{
}

DomainMapperTableHandler::~DomainMapperTableHandler() 
{
}

void DomainMapperTableHandler::startTable(unsigned int nRows,
                                          unsigned int /*nDepth*/,
                                          TablePropertyMapPtr pProps)
{
    m_aTableProperties = pProps;
    m_pTableSeq = TableSequencePointer_t(new TableSequence_t(nRows));
    m_nRowIndex = 0;

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("tablehandler.table");
    dmapper_logger->attribute("rows", nRows);
    
    if (pProps.get() != NULL)
        dmapper_logger->addTag(pProps->toTag());
#endif
}

/*-- 22.02.2008 10:18:37---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyMapPtr lcl_SearchParentStyleSheetAndMergeProperties(const StyleSheetEntryPtr pStyleSheet, StyleSheetTablePtr pStyleSheetTable)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("lcl_SearchParentStyleSheetAndMergeProperties");
    dmapper_logger->addTag(pStyleSheet->toTag());
#endif

    PropertyMapPtr pRet;
    if( pStyleSheet->sBaseStyleIdentifier.getLength())
    {
        const StyleSheetEntryPtr pParentStyleSheet = pStyleSheetTable->FindStyleSheetByISTD( pStyleSheet->sBaseStyleIdentifier );
        pRet = lcl_SearchParentStyleSheetAndMergeProperties( pParentStyleSheet, pStyleSheetTable );
    }    
    else
    {
        pRet.reset( new PropertyMap );
    }

    pRet->insert(  pStyleSheet->pProperties, true );
    
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("lcl_SearchParentStyleSheetAndMergeProperties");
#endif

    return pRet;
}

void lcl_mergeBorder( PropertyIds nId, PropertyMapPtr pOrig, PropertyMapPtr pDest )
{
    PropertyDefinition aDef( nId, false );
    PropertyMap::iterator pOrigIt = pOrig->find( aDef );

    if ( pOrigIt != pOrig->end( ) )
    {
        pDest->Insert( nId, false, pOrigIt->second, false );
    }
}

void lcl_computeCellBorders( PropertyMapPtr pTableBorders, PropertyMapPtr pCellProps, 
        sal_Int32 nCell, sal_Int32 nRow, bool bIsEndCol, bool bIsEndRow )
{
    PropertyDefinition aVertPDef( META_PROP_VERTICAL_BORDER, false );
    PropertyDefinition aHorizPDef( META_PROP_HORIZONTAL_BORDER, false );

    PropertyMap::iterator aVerticalIter = pCellProps->find( aVertPDef );
    PropertyMap::iterator aHorizontalIter = pCellProps->find( aHorizPDef );

    // Handle the vertical and horizontal borders  
    bool bHasVert = ( aVerticalIter != pCellProps->end(  ) );
    uno::Any aVertProp;
    if ( !bHasVert )
    {
        aVerticalIter = pTableBorders->find( aVertPDef );
        bHasVert = ( aVerticalIter != pTableBorders->end( ) );
        if ( bHasVert )
            aVertProp = aVerticalIter->second;
    }
    else
    {
        aVertProp = aVerticalIter->second;
        pCellProps->erase( aVerticalIter );
    }

    bool bHasHoriz = ( aHorizontalIter != pCellProps->end(  ) );
    uno::Any aHorizProp;
    if ( !bHasHoriz )
    {
        aHorizontalIter = pTableBorders->find( aHorizPDef );
        bHasHoriz = ( aHorizontalIter != pTableBorders->end( ) );
        if ( bHasHoriz )
            aHorizProp = aHorizontalIter->second;
    }
    else
    {
        aHorizProp = aHorizontalIter->second;
        pCellProps->erase( aHorizontalIter );
    }

    if ( nCell == 0 )
    {
        lcl_mergeBorder( PROP_LEFT_BORDER, pTableBorders, pCellProps );
        if ( bHasVert )
            pCellProps->Insert( PROP_RIGHT_BORDER, false, aVertProp, false );
    }
    
    if ( bIsEndCol )
    {
        lcl_mergeBorder( PROP_RIGHT_BORDER, pTableBorders, pCellProps );
        if ( bHasVert )
            pCellProps->Insert( PROP_LEFT_BORDER, false, aVertProp, false );
    }
    
    if ( nCell > 0 && !bIsEndCol )
    {
        if ( bHasVert )
        {
            pCellProps->Insert( PROP_RIGHT_BORDER, false, aVertProp, false );
            pCellProps->Insert( PROP_LEFT_BORDER, false, aVertProp, false );
        }
    }

    if ( nRow == 0 )
    {
        lcl_mergeBorder( PROP_TOP_BORDER, pTableBorders, pCellProps );
        if ( bHasHoriz )
            pCellProps->Insert( PROP_BOTTOM_BORDER, false, aHorizProp, false );
    }

    if ( bIsEndRow )
    {
        lcl_mergeBorder( PROP_BOTTOM_BORDER, pTableBorders, pCellProps );
        if ( bHasHoriz )
            pCellProps->Insert( PROP_TOP_BORDER, false, aHorizProp, false );
    }
    
    if ( nRow > 0 && !bIsEndRow )
    {
        if ( bHasHoriz )
        {
            pCellProps->Insert( PROP_TOP_BORDER, false, aHorizProp, false );
            pCellProps->Insert( PROP_BOTTOM_BORDER, false, aHorizProp, false );
        }
    }
}

#ifdef DEBUG_DOMAINMAPPER

void lcl_debug_BorderLine(table::BorderLine & rLine)
{
    dmapper_logger->startElement("BorderLine");
    dmapper_logger->attribute("Color", rLine.Color);
    dmapper_logger->attribute("InnerLineWidth", rLine.InnerLineWidth);
    dmapper_logger->attribute("OuterLineWidth", rLine.OuterLineWidth);
    dmapper_logger->attribute("LineDistance", rLine.LineDistance);
    dmapper_logger->endElement("BorderLine");
}

void lcl_debug_TableBorder(table::TableBorder & rBorder)
{
    dmapper_logger->startElement("TableBorder");
    lcl_debug_BorderLine(rBorder.TopLine);
    dmapper_logger->attribute("IsTopLineValid", rBorder.IsTopLineValid);
    lcl_debug_BorderLine(rBorder.BottomLine);
    dmapper_logger->attribute("IsBottomLineValid", rBorder.IsBottomLineValid);
    lcl_debug_BorderLine(rBorder.LeftLine);
    dmapper_logger->attribute("IsLeftLineValid", rBorder.IsLeftLineValid);
    lcl_debug_BorderLine(rBorder.RightLine);
    dmapper_logger->attribute("IsRightLineValid", rBorder.IsRightLineValid);
    lcl_debug_BorderLine(rBorder.VerticalLine);
    dmapper_logger->attribute("IsVerticalLineValid", rBorder.IsVerticalLineValid);
    lcl_debug_BorderLine(rBorder.HorizontalLine);
    dmapper_logger->attribute("IsHorizontalLineValid", rBorder.IsHorizontalLineValid);
    dmapper_logger->attribute("Distance", rBorder.Distance);
    dmapper_logger->attribute("IsDistanceValid", rBorder.IsDistanceValid);
    dmapper_logger->endElement("TableBorder");
}
#endif

struct WRITERFILTER_DLLPRIVATE TableInfo
{
    sal_Int32 nLeftBorderDistance;
    sal_Int32 nRightBorderDistance;
    sal_Int32 nTopBorderDistance;
    sal_Int32 nBottomBorderDistance;
    PropertyMapPtr pTableDefaults;
    PropertyMapPtr pTableBorders;
    TableStyleSheetEntry* pTableStyle;
    TablePropertyValues_t aTableProperties;
    
    TableInfo()
    : nLeftBorderDistance(DEF_BORDER_DIST)
    , nRightBorderDistance(DEF_BORDER_DIST)
    , nTopBorderDistance(0)
    , nBottomBorderDistance(0)
    , pTableDefaults(new PropertyMap)
    , pTableBorders(new PropertyMap)
    , pTableStyle(NULL)
    {
    }
        
};
    
TableStyleSheetEntry * DomainMapperTableHandler::endTableGetTableStyle(TableInfo & rInfo)
{
    // will receive the table style if any
    TableStyleSheetEntry* pTableStyle = NULL;
            
    if( m_aTableProperties.get() )
    {
        //create properties from the table attributes
        //...pPropMap->Insert( PROP_LEFT_MARGIN, false, uno::makeAny( m_nLeftMargin - m_nGapHalf ));
        //pPropMap->Insert( PROP_HORI_ORIENT, false, uno::makeAny( text::HoriOrientation::RIGHT ));
        sal_Int32 nGapHalf = 0;
        sal_Int32 nLeftMargin = 0;
        sal_Int32 nTableWidth = 0;
        
        PropertyMap::iterator aTableStyleIter = 
        m_aTableProperties->find( PropertyDefinition( META_PROP_TABLE_STYLE_NAME, false ) );
        if(aTableStyleIter != m_aTableProperties->end())
        {
            // Apply table style properties recursively
            ::rtl::OUString sTableStyleName; 
            aTableStyleIter->second >>= sTableStyleName;
            StyleSheetTablePtr pStyleSheetTable = m_rDMapper_Impl.GetStyleSheetTable();
            const StyleSheetEntryPtr pStyleSheet = pStyleSheetTable->FindStyleSheetByISTD( sTableStyleName );
            pTableStyle = dynamic_cast<TableStyleSheetEntry*>( pStyleSheet.get( ) );
            m_aTableProperties->erase( aTableStyleIter );
            
            if( pStyleSheet )
            {
                // First get the style properties, then the table ones
                PropertyMapPtr pTableProps( m_aTableProperties );
                TablePropertyMapPtr pEmptyProps( new TablePropertyMap );
                
                m_aTableProperties = pEmptyProps;
                
                PropertyMapPtr pMergedProperties = lcl_SearchParentStyleSheetAndMergeProperties(pStyleSheet, pStyleSheetTable);
                
#ifdef DEBUG_DOMAINMAPPER
                dmapper_logger->startElement("mergedProps");
                dmapper_logger->addTag(pMergedProperties->toTag());
                dmapper_logger->endElement("mergedProps");
#endif
                
                m_aTableProperties->insert( pMergedProperties );
                m_aTableProperties->insert( pTableProps );
                
#ifdef DEBUG_DOMAINMAPPER
                dmapper_logger->startElement("TableProperties");
                dmapper_logger->addTag(m_aTableProperties->toTag());
                dmapper_logger->endElement("TableProperties");
#endif
            }    
        }
        
        // Set the table default attributes for the cells
        rInfo.pTableDefaults->insert( m_aTableProperties );
        
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->startElement("TableDefaults");
        dmapper_logger->addTag(rInfo.pTableDefaults->toTag());
        dmapper_logger->endElement("TableDefaults");
#endif
        
        m_aTableProperties->getValue( TablePropertyMap::GAP_HALF, nGapHalf );
        m_aTableProperties->getValue( TablePropertyMap::LEFT_MARGIN, nLeftMargin );
        
        m_aTableProperties->getValue( TablePropertyMap::CELL_MAR_LEFT, 
                                     rInfo.nLeftBorderDistance );
        m_aTableProperties->getValue( TablePropertyMap::CELL_MAR_RIGHT, 
                                     rInfo.nRightBorderDistance );
        m_aTableProperties->getValue( TablePropertyMap::CELL_MAR_TOP, 
                                     rInfo.nTopBorderDistance );
        m_aTableProperties->getValue( TablePropertyMap::CELL_MAR_BOTTOM, 
                                     rInfo.nBottomBorderDistance );
        
        table::TableBorderDistances aDistances;
        aDistances.IsTopDistanceValid = 
        aDistances.IsBottomDistanceValid = 
        aDistances.IsLeftDistanceValid = 
        aDistances.IsRightDistanceValid = sal_True;
        aDistances.TopDistance = static_cast<sal_Int16>( rInfo.nTopBorderDistance );
        aDistances.BottomDistance = static_cast<sal_Int16>( rInfo.nBottomBorderDistance );
        aDistances.LeftDistance = static_cast<sal_Int16>( rInfo.nLeftBorderDistance );
        aDistances.RightDistance = static_cast<sal_Int16>( rInfo.nRightBorderDistance );
        
        m_aTableProperties->Insert( PROP_TABLE_BORDER_DISTANCES, false, uno::makeAny( aDistances ) );
        
        //table border settings
        table::TableBorder aTableBorder;
        
        PropertyMap::iterator aTblBorderIter = m_aTableProperties->find( PropertyDefinition(PROP_TOP_BORDER, false) );
        if( aTblBorderIter != m_aTableProperties->end() )
        {    
            aTblBorderIter->second >>= aTableBorder.TopLine;
            aTableBorder.IsTopLineValid = true;
            m_aTableProperties->erase( aTblBorderIter );
            
            rInfo.pTableBorders->Insert( PROP_TOP_BORDER, false, 
                                        uno::makeAny( aTableBorder.TopLine ) );
            PropertyMap::iterator pIt = rInfo.pTableDefaults->find( PropertyDefinition( PROP_TOP_BORDER, false ) );
            if ( pIt != rInfo.pTableDefaults->end( ) )
                rInfo.pTableDefaults->erase( pIt );
        }
        aTblBorderIter = m_aTableProperties->find( PropertyDefinition(PROP_BOTTOM_BORDER, false) );
        if( aTblBorderIter != m_aTableProperties->end() )
        {    
            aTblBorderIter->second >>= aTableBorder.BottomLine;
            aTableBorder.IsBottomLineValid = true;
            m_aTableProperties->erase( aTblBorderIter );
            
            rInfo.pTableBorders->Insert( PROP_BOTTOM_BORDER, false, 
                                        uno::makeAny( aTableBorder.BottomLine));
            PropertyMap::iterator pIt = rInfo.pTableDefaults->find( PropertyDefinition( PROP_BOTTOM_BORDER, false ) );
            if ( pIt != rInfo.pTableDefaults->end( ) )
                rInfo.pTableDefaults->erase( pIt );
        }
        aTblBorderIter = m_aTableProperties->find( PropertyDefinition(PROP_LEFT_BORDER, false) );
        if( aTblBorderIter != m_aTableProperties->end() )
        {    
            aTblBorderIter->second >>= aTableBorder.LeftLine;
            aTableBorder.IsLeftLineValid = true;
            m_aTableProperties->erase( aTblBorderIter );
            
            rInfo.pTableBorders->Insert( PROP_LEFT_BORDER, false, 
                                        uno::makeAny( aTableBorder.LeftLine ) );
            PropertyMap::iterator pIt = rInfo.pTableDefaults->find( PropertyDefinition( PROP_LEFT_BORDER, false ) );
            if ( pIt != rInfo.pTableDefaults->end( ) )
                rInfo.pTableDefaults->erase( pIt );
        }
        aTblBorderIter = m_aTableProperties->find( PropertyDefinition(PROP_RIGHT_BORDER, false) );
        if( aTblBorderIter != m_aTableProperties->end() )
        {                                               
            aTblBorderIter->second >>= aTableBorder.RightLine;
            aTableBorder.IsRightLineValid = true;
            m_aTableProperties->erase( aTblBorderIter );
            
            rInfo.pTableBorders->Insert( PROP_RIGHT_BORDER, false, 
                                        uno::makeAny( aTableBorder.RightLine ) );
            PropertyMap::iterator pIt = rInfo.pTableDefaults->find( PropertyDefinition( PROP_RIGHT_BORDER, false ) );
            if ( pIt != rInfo.pTableDefaults->end( ) )
                rInfo.pTableDefaults->erase( pIt );
        }
        aTblBorderIter = m_aTableProperties->find( PropertyDefinition(META_PROP_HORIZONTAL_BORDER, false) );
        if( aTblBorderIter != m_aTableProperties->end() )
        {    
            aTblBorderIter->second >>= aTableBorder.HorizontalLine;
            aTableBorder.IsHorizontalLineValid = true;
            m_aTableProperties->erase( aTblBorderIter );
            
            rInfo.pTableBorders->Insert
                (META_PROP_HORIZONTAL_BORDER, false, 
                 uno::makeAny( aTableBorder.HorizontalLine ) );
            PropertyMap::iterator pIt = rInfo.pTableDefaults->find( PropertyDefinition( META_PROP_HORIZONTAL_BORDER, false ) );
            if ( pIt != rInfo.pTableDefaults->end( ) )
                rInfo.pTableDefaults->erase( pIt );
        }
        aTblBorderIter = m_aTableProperties->find( PropertyDefinition(META_PROP_VERTICAL_BORDER, false) );
        if( aTblBorderIter != m_aTableProperties->end() )
        {    
            aTblBorderIter->second >>= aTableBorder.VerticalLine;
            aTableBorder.IsVerticalLineValid = true;
            m_aTableProperties->erase( aTblBorderIter );
            
            rInfo.pTableBorders->Insert
                (META_PROP_VERTICAL_BORDER, false, 
                 uno::makeAny( aTableBorder.VerticalLine ) );
            PropertyMap::iterator pIt = rInfo.pTableDefaults->find( PropertyDefinition( META_PROP_VERTICAL_BORDER, false ) );
            if ( pIt != rInfo.pTableDefaults->end( ) )
                rInfo.pTableDefaults->erase( pIt );
        }
        aTableBorder.Distance = 0;
        aTableBorder.IsDistanceValid = false;
        
        m_aTableProperties->Insert( PROP_TABLE_BORDER, false, uno::makeAny( aTableBorder ) );
        
#ifdef DEBUG_DOMAINMAPPER
        lcl_debug_TableBorder(aTableBorder);
#endif
        
        m_aTableProperties->Insert( PROP_LEFT_MARGIN, false, uno::makeAny( nLeftMargin - nGapHalf - rInfo.nLeftBorderDistance));
        
        m_aTableProperties->getValue( TablePropertyMap::TABLE_WIDTH, nTableWidth );
        if( nTableWidth > 0 )
            m_aTableProperties->Insert( PROP_WIDTH, false, uno::makeAny( nTableWidth ));
        
        sal_Int32 nHoriOrient = text::HoriOrientation::LEFT_AND_WIDTH;
        m_aTableProperties->getValue( TablePropertyMap::HORI_ORIENT, nHoriOrient ) ;
        m_aTableProperties->Insert( PROP_HORI_ORIENT, false, uno::makeAny( sal_Int16(nHoriOrient) ) );
        
        //fill default value - if not available
        const PropertyMap::const_iterator aRepeatIter = 
        m_aTableProperties->find( PropertyDefinition( PROP_HEADER_ROW_COUNT, false ) );
        if( aRepeatIter == m_aTableProperties->end() )
            m_aTableProperties->Insert( PROP_HEADER_ROW_COUNT, false, uno::makeAny( (sal_Int32)0 ));
        
        // Remove the PROP_HEADER_ROW_COUNT from the table default to avoid
        // propagating it to the cells
        PropertyMap::iterator aDefaultRepeatIt =
        rInfo.pTableDefaults->find( PropertyDefinition( PROP_HEADER_ROW_COUNT, false ) );
        if ( aDefaultRepeatIt != rInfo.pTableDefaults->end( ) )
            rInfo.pTableDefaults->erase( aDefaultRepeatIt );
        
        rInfo.aTableProperties = m_aTableProperties->GetPropertyValues();
        
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->startElement("debug.tableprops");
        dmapper_logger->addTag(m_aTableProperties->toTag());
        dmapper_logger->endElement("debug.tableprops");
#endif
        
    }
 
    return pTableStyle;
}

CellPropertyValuesSeq_t DomainMapperTableHandler::endTableGetCellProperties(TableInfo & rInfo)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("getCellProperties");
#endif
    
    CellPropertyValuesSeq_t aCellProperties( m_aCellProperties.size() );
    
    // std::vector< std::vector<PropertyMapPtr> > m_aCellProperties
    PropertyMapVector2::const_iterator aRowOfCellsIterator = m_aCellProperties.begin();
    PropertyMapVector2::const_iterator aRowOfCellsIteratorEnd = m_aCellProperties.end();
    PropertyMapVector2::const_iterator aLastRowIterator = m_aCellProperties.end() - 1;
    sal_Int32 nRow = 0;
    
    //it's a uno::Sequence< beans::PropertyValues >*
    RowPropertyValuesSeq_t* pCellProperties = aCellProperties.getArray();
    while( aRowOfCellsIterator != aRowOfCellsIteratorEnd )
    {
        //aRowOfCellsIterator points to a vector of PropertyMapPtr
        PropertyMapVector1::const_iterator aCellIterator = aRowOfCellsIterator->begin();
        PropertyMapVector1::const_iterator aCellIteratorEnd = aRowOfCellsIterator->end();
        PropertyMapVector1::const_iterator aLastCellIterator = aRowOfCellsIterator->end() - 1;
        
        // Get the row style properties
        sal_Int32 nRowStyleMask = sal_Int32( 0 );
        PropertyMapPtr pRowProps = m_aRowProperties[nRow];
        if ( pRowProps.get( ) )
        {
            PropertyMap::iterator pTcCnfStyleIt = pRowProps->find( PropertyDefinition( PROP_CNF_STYLE, true ) );
            if ( pTcCnfStyleIt != pRowProps->end( ) )
            {
                if ( rInfo.pTableStyle )
                {
                    rtl::OUString sMask;
                    pTcCnfStyleIt->second >>= sMask;
                    nRowStyleMask = sMask.toInt32( 2 );
                }
                pRowProps->erase( pTcCnfStyleIt );
            }
        }
        
        sal_Int32 nCell = 0;
        pCellProperties[nRow].realloc( aRowOfCellsIterator->size() );
        beans::PropertyValues* pSingleCellProperties = pCellProperties[nRow].getArray();
        while( aCellIterator != aCellIteratorEnd )
        {
            PropertyMapPtr pAllCellProps( new PropertyMap );
            
            bool bIsEndCol = aCellIterator == aLastCellIterator;
            bool bIsEndRow = aRowOfCellsIterator == aLastRowIterator;
            
            //aCellIterator points to a PropertyMapPtr;
            if( aCellIterator->get() )
            {
                if ( rInfo.pTableDefaults->size( ) )
                    pAllCellProps->insert( rInfo.pTableDefaults );
                    
                    // Fill the cell properties with the ones of the style
                    sal_Int32 nCellStyleMask = 0;
                    const PropertyMap::iterator aCnfStyleIter = 
                    aCellIterator->get()->find( PropertyDefinition( PROP_CNF_STYLE, false ) );
                    if ( aCnfStyleIter != aCellIterator->get( )->end( ) )
                    {
                        if ( rInfo.pTableStyle ) {
                            rtl::OUString sMask;
                            aCnfStyleIter->second >>= sMask;
                            nCellStyleMask = sMask.toInt32( 2 );
                        }
                        aCellIterator->get( )->erase( aCnfStyleIter );
                    }
                
                if ( rInfo.pTableStyle )
                {
                    PropertyMapPtr pStyleProps = rInfo.pTableStyle->GetProperties( nCellStyleMask + nRowStyleMask );
                    pAllCellProps->insert( pStyleProps );
                }
                
                // Then add the cell properties
                pAllCellProps->insert( *aCellIterator );
                aCellIterator->get( )->swap( *pAllCellProps.get( ) );
                
#ifdef DEBUG_DOMAINMAPPER
                dmapper_logger->startElement("cell");
                dmapper_logger->attribute("cell", nCell);
                dmapper_logger->attribute("row", nRow);
#endif
                
                lcl_computeCellBorders( rInfo.pTableBorders, *aCellIterator, nCell, nRow, bIsEndCol, bIsEndRow );
                
                //now set the default left+right border distance TODO: there's an sprm containing the default distance!
                const PropertyMap::const_iterator aLeftDistanceIter = 
                aCellIterator->get()->find( PropertyDefinition(PROP_LEFT_BORDER_DISTANCE, false) );
                if( aLeftDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_LEFT_BORDER_DISTANCE, false, 
                                                 uno::makeAny(rInfo.nLeftBorderDistance ) );
                const PropertyMap::const_iterator aRightDistanceIter = 
                aCellIterator->get()->find( PropertyDefinition(PROP_RIGHT_BORDER_DISTANCE, false) );
                if( aRightDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_RIGHT_BORDER_DISTANCE, false, 
                                                 uno::makeAny((sal_Int32) rInfo.nRightBorderDistance ) );
                
                const PropertyMap::const_iterator aTopDistanceIter = 
                aCellIterator->get()->find( PropertyDefinition(PROP_TOP_BORDER_DISTANCE, false) );
                if( aTopDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_TOP_BORDER_DISTANCE, false, 
                                                 uno::makeAny((sal_Int32) rInfo.nTopBorderDistance ) );
                
                const PropertyMap::const_iterator aBottomDistanceIter = 
                aCellIterator->get()->find( PropertyDefinition(PROP_BOTTOM_BORDER_DISTANCE, false) );
                if( aBottomDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_BOTTOM_BORDER_DISTANCE, false, 
                                                 uno::makeAny((sal_Int32) rInfo.nBottomBorderDistance ) );
                
                pSingleCellProperties[nCell] = aCellIterator->get()->GetPropertyValues();
#ifdef DEBUG_DOMAINMAPPER
                dmapper_logger->endElement("cell");
#endif
            }
            ++nCell;
            ++aCellIterator;
        }    
#ifdef DEBUG_DOMAINMAPPER
        //-->debug cell properties
        {
            ::rtl::OUString sNames;
            const uno::Sequence< beans::PropertyValues > aDebugCurrentRow = aCellProperties[nRow];
            sal_Int32 nDebugCells = aDebugCurrentRow.getLength();
            (void) nDebugCells;
            for( sal_Int32  nDebugCell = 0; nDebugCell < nDebugCells; ++nDebugCell)
            {
                const uno::Sequence< beans::PropertyValue >& aDebugCellProperties = aDebugCurrentRow[nDebugCell];
                sal_Int32 nDebugCellProperties = aDebugCellProperties.getLength();
                for( sal_Int32  nDebugProperty = 0; nDebugProperty < nDebugCellProperties; ++nDebugProperty)
                {
                    const ::rtl::OUString sName = aDebugCellProperties[nDebugProperty].Name;
                    sNames += sName;
                    sNames += ::rtl::OUString('-');
                }
                sNames += ::rtl::OUString('\n');
            }
            (void)sNames;
        }
        //--<
#endif
        ++nRow;
        ++aRowOfCellsIterator;
    }
                   
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("getCellProperties");
#endif
    
    return aCellProperties;
}

RowPropertyValuesSeq_t DomainMapperTableHandler::endTableGetRowProperties()
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("getRowProperties");
#endif

    RowPropertyValuesSeq_t aRowProperties( m_aRowProperties.size() );
    PropertyMapVector1::const_iterator aRowIter = m_aRowProperties.begin();
    PropertyMapVector1::const_iterator aRowIterEnd = m_aRowProperties.end();
    sal_Int32 nRow = 0;
    while( aRowIter != aRowIterEnd )
    {
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->startElement("rowProps.row");
#endif
        if( aRowIter->get() )
        {
            //set default to 'break across pages"
            if( aRowIter->get()->find( PropertyDefinition( PROP_IS_SPLIT_ALLOWED, false )) == aRowIter->get()->end()) 
                aRowIter->get()->Insert( PROP_IS_SPLIT_ALLOWED, false, uno::makeAny(sal_True ) );
            
            aRowProperties[nRow] = (*aRowIter)->GetPropertyValues();
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->addTag((*aRowIter)->toTag());
            dmapper_logger->addTag(lcl_PropertyValuesToTag(aRowProperties[nRow]));
#endif
        }
        ++nRow;
        ++aRowIter;
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->endElement("rowProps.row");
#endif
    }
 
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("getRowProperties");
#endif

    return aRowProperties;
}
    
void DomainMapperTableHandler::endTable()
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("tablehandler.endTable");
#endif

    TableInfo aTableInfo;
    aTableInfo.pTableStyle = endTableGetTableStyle(aTableInfo);
    //  expands to uno::Sequence< Sequence< beans::PropertyValues > > 

    CellPropertyValuesSeq_t aCellProperties = endTableGetCellProperties(aTableInfo);
    
    RowPropertyValuesSeq_t aRowProperties = endTableGetRowProperties();
    
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->addTag(lcl_PropertyValueSeqToTag(aRowProperties));
#endif
    
    if (m_pTableSeq->getLength() > 0)
    {
        try
        {
            uno::Reference<text::XTextTable> xTable = m_xText->convertToTable(*m_pTableSeq, 
                                    aCellProperties,
                                    aRowProperties,
                                    aTableInfo.aTableProperties);

            m_xTableRange = xTable->getAnchor( );
        }
        catch (lang::IllegalArgumentException e)
        {
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->chars("failed to import table!");
#endif
        }
#ifdef DEBUG_DOMAINMAPPER
        catch ( uno::Exception e )
        {
            dmapper_logger->startElement("exception");
            dmapper_logger->chars(rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr( ));
            dmapper_logger->endElement("exeception");
        }
#endif
    }

    m_aTableProperties.reset();
    m_aCellProperties.clear();
    m_aRowProperties.clear();
    
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("tablehandler.endTable");
    dmapper_logger->endElement("tablehandler.table");
#endif
}

void DomainMapperTableHandler::startRow(unsigned int nCells,
                                        TablePropertyMapPtr pProps)
{
    m_aRowProperties.push_back( pProps );
    m_aCellProperties.push_back( PropertyMapVector1() );
    
#if DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("table.row");
    dmapper_logger->attribute("cells", nCells);
    if (pProps != NULL)
        dmapper_logger->addTag(pProps->toTag());
#endif

    m_pRowSeq = RowSequencePointer_t(new RowSequence_t(nCells));
    m_nCellIndex = 0;
}

void DomainMapperTableHandler::endRow()
{
    (*m_pTableSeq)[m_nRowIndex] = *m_pRowSeq;
    ++m_nRowIndex;
    m_nCellIndex = 0;
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("table.row");
#endif
}

void DomainMapperTableHandler::startCell(const Handle_t & start,
                                         TablePropertyMapPtr pProps )
{
    sal_uInt32 nRow = m_aRowProperties.size();
    if ( pProps.get( ) )
        m_aCellProperties[nRow - 1].push_back( pProps );
    else
    {
        // Adding an empty cell properties map to be able to get
        // the table defaults properties
        TablePropertyMapPtr pEmptyProps( new TablePropertyMap( ) );
        m_aCellProperties[nRow - 1].push_back( pEmptyProps );
    }
    
#if DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("table.cell");
    dmapper_logger->startElement("table.cell.start");
    dmapper_logger->chars(toString(start));
    dmapper_logger->endElement("table.cell.start");
    lcl_printProperties( pProps );
#endif

    //add a new 'row' of properties
//    if( m_pCellProperties.size() <= sal::static_int_cast< sal_uInt32, sal_Int32>(m_nRowIndex) )
//        m_pCellProperties.push_back( RowProperties_t() );
//    m_pCellProperties[m_nRowIndex].push_back( pProps );
    m_pCellSeq = CellSequencePointer_t(new CellSequence_t(2));
    if (!start.get()) 
        return;
    (*m_pCellSeq)[0] = start->getStart();
}

void DomainMapperTableHandler::endCell(const Handle_t & end)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("table.cell.end");
    dmapper_logger->chars(toString(end));
    dmapper_logger->endElement("table.cell.end");
    dmapper_logger->endElement("table.cell");
    clog << "</table.cell>" << endl;
#endif

    if (!end.get()) 
        return;
    (*m_pCellSeq)[1] = end->getEnd();
    (*m_pRowSeq)[m_nCellIndex] = *m_pCellSeq;
    ++m_nCellIndex;
}

}}
