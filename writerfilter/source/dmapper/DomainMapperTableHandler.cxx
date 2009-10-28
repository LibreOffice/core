/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DomainMapperTableHandler.cxx,v $
 * $Revision: 1.15 $
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
#if OSL_DEBUG_LEVEL > 1
#include <iostream>
#endif

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;

#define DEF_BORDER_DIST 190  //0,19cm
#define DEFAULT_CELL_MARGIN 108 //default cell margin, not documented

#ifdef DEBUG
static void lcl_printHandle(const Handle_t rHandle)
{
    if (!rHandle.get())
        return;
    rtl::OUString aOUStr = rHandle->getString();
    rtl::OString aOStr(aOUStr.getStr(), aOUStr.getLength(),  RTL_TEXTENCODING_ASCII_US );

    clog << aOStr.getStr() << endl;
}
static void  lcl_printProperties( PropertyMapPtr pProps )
{
    if( pProps.get() )
    {
        clog << "<properties>";
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
                clog << ": BorderLine ( Color: " << aLine.Color;
                clog << ", Inner: " << aLine.InnerLineWidth;
                clog << ", Outer: " << aLine.OuterLineWidth << ") ";
            }
            else if ( aMapIter->second >>= nColor )
            {
                clog << ": Color ( " << nColor << " ) ";
            }

            clog << " - ";
        }
        clog << endl;
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

#if OSL_DEBUG_LEVEL > 1
    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%d", nRows);
    clog << "<table rows=\"" << sBuffer << "\">" << endl;
    lcl_printProperties( pProps );
#endif
}

/*-- 22.02.2008 10:18:37---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyMapPtr lcl_SearchParentStyleSheetAndMergeProperties(const StyleSheetEntryPtr pStyleSheet, StyleSheetTablePtr pStyleSheetTable)
{
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

void DomainMapperTableHandler::endTable()
{
#if OSL_DEBUG_LEVEL > 1
{
    clog << "</table>" << endl;
    sal_uInt32 nCells = 0;
    sal_uInt32 nRows = m_aRowProperties.size();
    if( nRows == m_aCellProperties.size() )
    {
        for( sal_uInt32 nRow = 0; nRow < nRows; ++nRow )
            nCells += m_aCellProperties[nRow].size();
    }
    sal_uInt32 nTblPropSize = m_aTableProperties.get() ? m_aTableProperties->size() : 0;
    (void)nTblPropSize;

    ::rtl::OUString sNames;
    if( nTblPropSize )
    {
        const beans::PropertyValues aDebugTbl = m_aTableProperties->GetPropertyValues();
        for( sal_uInt32 nDebug = 0; nDebug < nTblPropSize; ++nDebug)
        {
            const ::rtl::OUString sName = aDebugTbl[nDebug].Name;
            sNames += sName;
            sNames += ::rtl::OUString('-');
        }
        m_aTableProperties->Invalidate();
        sNames += ::rtl::OUString(' ');
        clog << "Props: " << rtl::OUStringToOString( sNames, RTL_TEXTENCODING_UTF8 ).getStr( ) << endl;
    }
}
#endif

    TablePropertyValues_t       aTableProperties;
    sal_Int32 nLeftBorderDistance, nRightBorderDistance, nTopBorderDistance, nBottomBorderDistance;
    nLeftBorderDistance = nRightBorderDistance = DEF_BORDER_DIST;
    nTopBorderDistance = nBottomBorderDistance = 0;

    PropertyMapPtr pTableDefaults( new PropertyMap );
    PropertyMapPtr pTableBorders( new PropertyMap );

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
            pTableStyle = static_cast<TableStyleSheetEntry*>( pStyleSheet.get( ) );
            m_aTableProperties->erase( aTableStyleIter );

            if( pStyleSheet )
            {
                // First get the style properties, then the table ones
                PropertyMapPtr pTableProps( m_aTableProperties );
                TablePropertyMapPtr pEmptyProps( new TablePropertyMap );

                m_aTableProperties = pEmptyProps;

                PropertyMapPtr pMergedProperties = lcl_SearchParentStyleSheetAndMergeProperties(pStyleSheet, pStyleSheetTable);

                m_aTableProperties->insert( pMergedProperties );
                m_aTableProperties->insert( pTableProps );
            }
        }

        // Set the table default attributes for the cells
        pTableDefaults->insert( m_aTableProperties );

        m_aTableProperties->getValue( TablePropertyMap::GAP_HALF, nGapHalf );
        m_aTableProperties->getValue( TablePropertyMap::LEFT_MARGIN, nLeftMargin );

        m_aTableProperties->getValue( TablePropertyMap::CELL_MAR_LEFT, nLeftBorderDistance );
        m_aTableProperties->getValue( TablePropertyMap::CELL_MAR_RIGHT, nRightBorderDistance );
        m_aTableProperties->getValue( TablePropertyMap::CELL_MAR_TOP, nTopBorderDistance );
        m_aTableProperties->getValue( TablePropertyMap::CELL_MAR_BOTTOM, nBottomBorderDistance );

        table::TableBorderDistances aDistances;
        aDistances.IsTopDistanceValid =
                    aDistances.IsBottomDistanceValid =
                    aDistances.IsLeftDistanceValid =
                    aDistances.IsRightDistanceValid = sal_True;
        aDistances.TopDistance = static_cast<sal_Int16>( nTopBorderDistance );
        aDistances.BottomDistance = static_cast<sal_Int16>( nBottomBorderDistance );
        aDistances.LeftDistance = static_cast<sal_Int16>( nLeftBorderDistance );
        aDistances.RightDistance = static_cast<sal_Int16>( nRightBorderDistance );

        m_aTableProperties->Insert( PROP_TABLE_BORDER_DISTANCES, false, uno::makeAny( aDistances ) );

        //table border settings
        table::TableBorder aTableBorder;

        PropertyMap::iterator aTblBorderIter = m_aTableProperties->find( PropertyDefinition(PROP_TOP_BORDER, false) );
        if( aTblBorderIter != m_aTableProperties->end() )
        {
            aTblBorderIter->second >>= aTableBorder.TopLine;
            aTableBorder.IsTopLineValid = true;
            m_aTableProperties->erase( aTblBorderIter );

            pTableBorders->Insert( PROP_TOP_BORDER, false, uno::makeAny( aTableBorder.TopLine ) );
            PropertyMap::iterator pIt = pTableDefaults->find( PropertyDefinition( PROP_TOP_BORDER, false ) );
            if ( pIt != pTableDefaults->end( ) )
                pTableDefaults->erase( pIt );
        }
        aTblBorderIter = m_aTableProperties->find( PropertyDefinition(PROP_BOTTOM_BORDER, false) );
        if( aTblBorderIter != m_aTableProperties->end() )
        {
            aTblBorderIter->second >>= aTableBorder.BottomLine;
            aTableBorder.IsBottomLineValid = true;
            m_aTableProperties->erase( aTblBorderIter );

            pTableBorders->Insert( PROP_BOTTOM_BORDER, false, uno::makeAny( aTableBorder.BottomLine ) );
            PropertyMap::iterator pIt = pTableDefaults->find( PropertyDefinition( PROP_BOTTOM_BORDER, false ) );
            if ( pIt != pTableDefaults->end( ) )
                pTableDefaults->erase( pIt );
        }
        aTblBorderIter = m_aTableProperties->find( PropertyDefinition(PROP_LEFT_BORDER, false) );
        if( aTblBorderIter != m_aTableProperties->end() )
        {
            aTblBorderIter->second >>= aTableBorder.LeftLine;
            aTableBorder.IsLeftLineValid = true;
            m_aTableProperties->erase( aTblBorderIter );

            pTableBorders->Insert( PROP_LEFT_BORDER, false, uno::makeAny( aTableBorder.LeftLine ) );
            PropertyMap::iterator pIt = pTableDefaults->find( PropertyDefinition( PROP_LEFT_BORDER, false ) );
            if ( pIt != pTableDefaults->end( ) )
                pTableDefaults->erase( pIt );
        }
        aTblBorderIter = m_aTableProperties->find( PropertyDefinition(PROP_RIGHT_BORDER, false) );
        if( aTblBorderIter != m_aTableProperties->end() )
        {
            aTblBorderIter->second >>= aTableBorder.RightLine;
            aTableBorder.IsRightLineValid = true;
            m_aTableProperties->erase( aTblBorderIter );

            pTableBorders->Insert( PROP_RIGHT_BORDER, false, uno::makeAny( aTableBorder.RightLine ) );
            PropertyMap::iterator pIt = pTableDefaults->find( PropertyDefinition( PROP_RIGHT_BORDER, false ) );
            if ( pIt != pTableDefaults->end( ) )
                pTableDefaults->erase( pIt );
        }
        aTblBorderIter = m_aTableProperties->find( PropertyDefinition(META_PROP_HORIZONTAL_BORDER, false) );
        if( aTblBorderIter != m_aTableProperties->end() )
        {
            aTblBorderIter->second >>= aTableBorder.HorizontalLine;
            aTableBorder.IsHorizontalLineValid = true;
            m_aTableProperties->erase( aTblBorderIter );

            pTableBorders->Insert( META_PROP_HORIZONTAL_BORDER, false, uno::makeAny( aTableBorder.HorizontalLine ) );
            PropertyMap::iterator pIt = pTableDefaults->find( PropertyDefinition( META_PROP_HORIZONTAL_BORDER, false ) );
            if ( pIt != pTableDefaults->end( ) )
                pTableDefaults->erase( pIt );
        }
        aTblBorderIter = m_aTableProperties->find( PropertyDefinition(META_PROP_VERTICAL_BORDER, false) );
        if( aTblBorderIter != m_aTableProperties->end() )
        {
            aTblBorderIter->second >>= aTableBorder.VerticalLine;
            aTableBorder.IsVerticalLineValid = true;
            m_aTableProperties->erase( aTblBorderIter );

            pTableBorders->Insert( META_PROP_VERTICAL_BORDER, false, uno::makeAny( aTableBorder.VerticalLine ) );
            PropertyMap::iterator pIt = pTableDefaults->find( PropertyDefinition( META_PROP_VERTICAL_BORDER, false ) );
            if ( pIt != pTableDefaults->end( ) )
                pTableDefaults->erase( pIt );
        }
        aTableBorder.Distance = 0;
        aTableBorder.IsDistanceValid = false;

        m_aTableProperties->Insert( PROP_TABLE_BORDER, false, uno::makeAny( aTableBorder ) );

        m_aTableProperties->Insert( PROP_LEFT_MARGIN, false, uno::makeAny( nLeftMargin - nGapHalf - nLeftBorderDistance));

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
                                pTableDefaults->find( PropertyDefinition( PROP_HEADER_ROW_COUNT, false ) );
        if ( aDefaultRepeatIt != pTableDefaults->end( ) )
            pTableDefaults->erase( aDefaultRepeatIt );

        aTableProperties = m_aTableProperties->GetPropertyValues();
    }

    //  expands to uno::Sequence< Sequence< beans::PropertyValues > >
    CellPropertyValuesSeq_t     aCellProperties( m_aCellProperties.size() );

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
                if ( pTableStyle )
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
                if ( pTableDefaults->size( ) )
                    pAllCellProps->insert( pTableDefaults );

                // Fill the cell properties with the ones of the style
                sal_Int32 nCellStyleMask = 0;
                const PropertyMap::iterator aCnfStyleIter =
                    aCellIterator->get()->find( PropertyDefinition( PROP_CNF_STYLE, false ) );
                if ( aCnfStyleIter != aCellIterator->get( )->end( ) )
                {
                    if ( pTableStyle ) {
                        rtl::OUString sMask;
                        aCnfStyleIter->second >>= sMask;
                        nCellStyleMask = sMask.toInt32( 2 );
                    }
                    aCellIterator->get( )->erase( aCnfStyleIter );
                }

                if ( pTableStyle )
                {
                    PropertyMapPtr pStyleProps = pTableStyle->GetProperties( nCellStyleMask + nRowStyleMask );
                    pAllCellProps->insert( pStyleProps );
                }

                // Then add the cell properties
                pAllCellProps->insert( *aCellIterator );
                aCellIterator->get( )->swap( *pAllCellProps.get( ) );

#if DEBUG
                clog << "Cell #" << nCell << ", Row #" << nRow << endl;
#endif

                lcl_computeCellBorders( pTableBorders, *aCellIterator, nCell, nRow, bIsEndCol, bIsEndRow );

                //now set the default left+right border distance TODO: there's an sprm containing the default distance!
                const PropertyMap::const_iterator aLeftDistanceIter =
                                aCellIterator->get()->find( PropertyDefinition(PROP_LEFT_BORDER_DISTANCE, false) );
                if( aLeftDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_LEFT_BORDER_DISTANCE, false,
                                                                        uno::makeAny(nLeftBorderDistance ) );
                const PropertyMap::const_iterator aRightDistanceIter =
                                aCellIterator->get()->find( PropertyDefinition(PROP_RIGHT_BORDER_DISTANCE, false) );
                if( aRightDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_RIGHT_BORDER_DISTANCE, false,
                                                        uno::makeAny((sal_Int32) nRightBorderDistance ) );

                const PropertyMap::const_iterator aTopDistanceIter =
                                aCellIterator->get()->find( PropertyDefinition(PROP_TOP_BORDER_DISTANCE, false) );
                if( aTopDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_TOP_BORDER_DISTANCE, false,
                                                        uno::makeAny((sal_Int32) nTopBorderDistance ) );

                const PropertyMap::const_iterator aBottomDistanceIter =
                                aCellIterator->get()->find( PropertyDefinition(PROP_BOTTOM_BORDER_DISTANCE, false) );
                if( aBottomDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_BOTTOM_BORDER_DISTANCE, false,
                                                        uno::makeAny((sal_Int32) nBottomBorderDistance ) );

                pSingleCellProperties[nCell] = aCellIterator->get()->GetPropertyValues();
            }
            ++nCell;
            ++aCellIterator;
        }
#if OSL_DEBUG_LEVEL > 1
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
#if OSL_DEBUG_LEVEL > 1
//-->debug cell properties of all rows
    {
        ::rtl::OUString sNames;
        for( sal_Int32  nDebugRow = 0; nDebugRow < aCellProperties.getLength(); ++nDebugRow)
        {
            const uno::Sequence< beans::PropertyValues > aDebugCurrentRow = aCellProperties[nDebugRow];
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
                sNames += ::rtl::OUString('+');
            }
            sNames += ::rtl::OUString('|');
        }
        (void)sNames;
    }
//--<
#endif

    RowPropertyValuesSeq_t      aRowProperties( m_aRowProperties.size() );
    PropertyMapVector1::const_iterator aRowIter = m_aRowProperties.begin();
    PropertyMapVector1::const_iterator aRowIterEnd = m_aRowProperties.end();
    nRow = 0;
    while( aRowIter != aRowIterEnd )
    {
        if( aRowIter->get() )
        {
            //set default to 'break across pages"
            if( aRowIter->get()->find( PropertyDefinition( PROP_IS_SPLIT_ALLOWED, false )) == aRowIter->get()->end())
                aRowIter->get()->Insert( PROP_IS_SPLIT_ALLOWED, false, uno::makeAny(sal_True ) );

            aRowProperties[nRow] = aRowIter->get()->GetPropertyValues();
        }
        ++nRow;
        ++aRowIter;
    }

    if (m_pTableSeq->getLength() > 0)
    {
        try
        {
#if OSL_DEBUG_LEVEL > 1
    {
        sal_Int32 nCellPropertiesRows = aCellProperties.getLength();
        sal_Int32 nCellPropertiesCells = aCellProperties[0].getLength();
        sal_Int32 nCellPropertiesProperties = aCellProperties[0][0].getLength();
        (void) nCellPropertiesRows;
        (void) nCellPropertiesCells;
        (void) nCellPropertiesProperties;
        ++nCellPropertiesProperties;
    }
            clog << "Converting table" << endl;
#endif

            uno::Reference<text::XTextTable> xTable = m_xText->convertToTable(*m_pTableSeq,
                                    aCellProperties,
                                    aRowProperties,
                                    aTableProperties);

            m_xTableRange = xTable->getAnchor( );
        }
        catch (lang::IllegalArgumentException e)
        {
#if OSL_DEBUG_LEVEL > 1
            clog << "failed to import table!" << endl;
#endif
        }
#if OSL_DEBUG_LEVEL > 1
        catch ( uno::Exception e )
        {
            clog << "Caught an other exception: " << rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr( ) << endl;
        }
#endif
    }

    m_aTableProperties.reset();
    m_aCellProperties.clear();
    m_aRowProperties.clear();
}

void DomainMapperTableHandler::startRow(unsigned int nCells,
                                        TablePropertyMapPtr pProps)
{
    m_aRowProperties.push_back( pProps );
    m_aCellProperties.push_back( PropertyMapVector1() );

#if OSL_DEBUG_LEVEL > 1
    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%d", nCells);

    clog << "<table.row cells=\"" << sBuffer << "\">" << endl;
    lcl_printProperties( pProps );
#endif

    m_pRowSeq = RowSequencePointer_t(new RowSequence_t(nCells));
    m_nCellIndex = 0;
}

void DomainMapperTableHandler::endRow()
{
    (*m_pTableSeq)[m_nRowIndex] = *m_pRowSeq;
    ++m_nRowIndex;
    m_nCellIndex = 0;
#if OSL_DEBUG_LEVEL > 1
    clog << "</table.row>" << endl;
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

#if OSL_DEBUG_LEVEL > 1
    clog << "<table.cell>";
    lcl_printHandle(start);
    lcl_printProperties( pProps );
    clog << ",";
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
#if OSL_DEBUG_LEVEL > 1
    lcl_printHandle(end);
    clog << "</table.cell>" << endl;
#endif

    if (!end.get())
        return;
    (*m_pCellSeq)[1] = end->getEnd();
    (*m_pRowSeq)[m_nCellIndex] = *m_pCellSeq;
    ++m_nCellIndex;
}

}}
