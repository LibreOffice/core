/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImplChartModel.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:00:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "ImplChartModel.hxx"
#include "CachedDataSequence.hxx"
#include "DataSeries.hxx"
#include "DataInterpreter.hxx"
#include "XYDataInterpreter.hxx"
#include "StyleFamilies.hxx"
#include "StyleFamily.hxx"
#include "macros.hxx"
#include "algohelper.hxx"
#include "ContextHelper.hxx"
// #include "SplitLayoutContainer.hxx"
#include "LayoutDefaults.hxx"
#include "PageBackground.hxx"
#include "DataSeriesTreeHelper.hxx"

#ifndef _CPPUHELPER_COMPONENT_CONTEXT_HXX_
#include <cppuhelper/component_context.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_XDATASINK_HPP_
#include <com/sun/star/chart2/XDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASEQUENCE_HPP_
#include <com/sun/star/chart2/XDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <com/sun/star/chart2/XAxis.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETS_HPP_
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCELLRANGECONTAINER_HPP_
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>
#endif

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

using ::rtl::OUString;

// enable this to get x-values in the data-series
#define USE_XY_DATA

namespace chart
{
namespace impl
{

ImplChartModel::ImplChartModel( Reference< uno::XComponentContext > const & xContext ) :
        m_xContext( xContext ),
        m_xChartTypeManager(
            xContext->getServiceManager()->createInstanceWithContext(
                C2U( "com.sun.star.chart2.ChartTypeManager" ),
                xContext ), uno::UNO_QUERY ),
        m_bIsDisposed( false )
//         m_xLayoutContainer( new SplitLayoutContainer() )
{
    GetStyleFamilies();


    uno::Reference< beans::XPropertySet > xProp( m_xChartTypeManager, uno::UNO_QUERY );
    uno::Reference< lang::XMultiServiceFactory > xFact( m_xChartTypeManager, uno::UNO_QUERY );
    if( xProp.is() &&
        xFact.is() )
    {
        OUString aServiceName;
        if( xProp->getPropertyValue( C2U( "ChartStyleTemplateServiceName" )) >>= aServiceName )
        {
            m_xChartTypeTemplate.set( xFact->createInstance( aServiceName ), uno::UNO_QUERY );
        }
    }

    m_xPageBackground = new PageBackground( m_xContext );
}

// ImplChartModel::CreateStyles()
// {
//     // set style
//     Reference< container::XNameAccess > xGraphicsFamily(
//         GetStyleFamilies()->getByName( C2U( "graphics" )), uno::UNO_QUERY );

//     if( xGraphicsFamily.is())
//         const sal_Int32 nModul = Reference< container::XIndexAccess >(
//             xGraphicsFamily, uno::UNO_QUERY )->getCount();
//     for( size_t nI = 0; nI < nMaxSeries; ++nI )
//     {
//         try
//         {
//             Reference< ::com::sun::star::style::XStyle > xStyle(
//                 xGraphicsFamily->getByName(
//                     aStyleNameStub + OUString::valueOf(
//                         static_cast< sal_Int32 >( (nI % nModul)+1 ))), uno::UNO_QUERY );

//             Reference< beans::XPropertySet > xSeriesProp(
//                 aResult[ nI ], uno::UNO_QUERY );

//             if( xSeriesProp.is())
//             {
//                 xSeriesProp->setPropertyValue(
//                     C2U( "Color" ),
//                     uno::makeAny( nDefaultColors[ sal_Int32( nI % nMaxDefaultColors ) ]));

//                 // set style
//                 Reference< ::com::sun::star::style::XStyleSupplier >
//                     xStyleSupp( xSeriesProp->getPropertyValue( C2U( "FirstGraphicsFormat" )),
//                                 uno::UNO_QUERY );

//                 if( xStyleSupp.is())
//                 {
//                     xStyleSupp->setStyle( xStyle );
//                 }
//             }
//         }
//         catch( uno::Exception ex )
//         {
//             OSL_ENSURE( false, "Couldn't set style" );
//         }
//     }
// }


void ImplChartModel::ReadData( const ::rtl::OUString & rRangeRepresentation )
{
    static sal_Int32 nDefaultColors[] =  {
        0xd43b3a,
        0xd0976d,
        0x76934b,
        0x386a53,
        0x14816b,
        0x103841,
        0x12406c,
        0x5929ba,
        0xda42c0,
        0xc179a2,
        0x31181b,
        0x4e6129
    };
    static const sal_Int32 nMaxDefaultColors = sizeof( nDefaultColors ) / sizeof( sal_Int32 );

    if( m_xDataProvider.is())
    {
        m_xChartData = m_xDataProvider->getDataByRangeRepresentation( rRangeRepresentation );

        InterpretData();

        const size_t nMaxSeries = m_aInterpretedData.size();
        const OUString aStyleNameStub( RTL_CONSTASCII_USTRINGPARAM( "Series " ));

        for( size_t nI = 0; nI < nMaxSeries; ++nI )
        {
            try
            {
                Reference< beans::XPropertySet > xSeriesProp(
                    m_aInterpretedData[ nI ], uno::UNO_QUERY );

                if( xSeriesProp.is())
                {
                    xSeriesProp->setPropertyValue(
                        C2U( "Color" ),
                        uno::makeAny( nDefaultColors[ sal_Int32( nI % nMaxDefaultColors ) ]));
                }
            }
            catch( uno::Exception ex )
            {
                OSL_ENSURE( false, "Couldn't set style" );
            }
        }
    }
}

void ImplChartModel::InterpretData()
{
    // create DataSeries objects using the standard data interpreter
    // todo: interpret data depending on selected chart type

#ifdef USE_XY_DATA
        m_aInterpretedData = XYDataInterpreter::InterpretData( m_xChartData );
#else
        m_aInterpretedData = DataInterpreter::InterpretData( m_xChartData );
#endif
}

Reference< container::XNameAccess > ImplChartModel::GetStyleFamilies()
{
    if( ! m_xFamilies.is())
    {
//         StyleFamilies * pStyleFamilies = new StyleFamilies();
//         StyleFamily *  pGraphicsFamily = new StyleFamily();
//         Reference< container::XNameAccess > xGraphicsFamily( pGraphicsFamily );

        // add some default styles for graphics

        // randomly generated default colors

//         sal_Int32 i = 0;
//         const sal_Int32 nMax = sizeof( nDefaults ) / sizeof( sal_Int32 );
//         for( ; i < nMax; ++i )
//         {
//             try
//             {
//                 Reference< ::com::sun::star::style::XStyle > xStyle(
//                     new GraphicsPropertiesStyle( xGraphicsFamily ) );
//                 xStyle->setName( C2U( "Series " ) + OUString::valueOf( static_cast< sal_Int32 >(i + 1) ) );
//                 Reference< beans::XPropertySet > xProp( xStyle, uno::UNO_QUERY );
//                 if( xProp.is())
//                 {
//                     xProp->setPropertyValue( C2U( "Color" ), uno::makeAny( nDefaults[ i ] ));
//                 }
//                 pGraphicsFamily->AddStyle( xStyle );
//             }
//             catch( uno::Exception ex )
//             {
//                 OSL_ENSURE( false, "Error on creating styles" );
//             }
//         }

//         bool bResult = pStyleFamilies->AddStyleFamily(
//             C2U( "graphics" ), xGraphicsFamily );

//         OSL_ASSERT( bResult );
//         m_xFamilies.set( pStyleFamilies );
    }

    return m_xFamilies;
}

// Diagram Access

void ImplChartModel::RemoveAllDiagrams()
{
    m_aDiagrams.clear();
}

bool ImplChartModel::RemoveDiagram( const Reference< chart2::XDiagram > & xDiagram )
{
    tDiagramContainer::iterator aIt( ::std::find(
                                         m_aDiagrams.begin(), m_aDiagrams.end(),
                                         xDiagram ));
    if( aIt == m_aDiagrams.end() )
        return false;

    m_aDiagrams.erase( aIt );
    return true;
}

void ImplChartModel::AppendDiagram( const Reference< chart2::XDiagram > & xDiagram )
{
    m_aDiagrams.push_back( xDiagram );
}

Reference< chart2::XDiagram > ImplChartModel::GetDiagram( size_t nIndex ) const
    throw( container::NoSuchElementException )
{
    if( nIndex >= m_aDiagrams.size() )
        throw container::NoSuchElementException();

    return m_aDiagrams[ nIndex ];
}

void ImplChartModel::SetDataProvider(
    const Reference< chart2::XDataProvider > & xProvider )
{
    m_xDataProvider = xProvider;

    // release other ressources
    m_xChartData = 0;
    m_aInterpretedData.clear();
}

void SAL_CALL ImplChartModel::SetRangeRepresentation( const ::rtl::OUString& aRangeRepresentation )
    throw (lang::IllegalArgumentException)
{
    OSL_TRACE( ::rtl::OUStringToOString(
                   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Chart Range: \"" )) +
                   aRangeRepresentation +
                   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\"")),
                   RTL_TEXTENCODING_ASCII_US ).getStr() );
    ReadData( aRangeRepresentation );
    CreateDefaultChart();
}

void ImplChartModel::SetChartTypeManager(
    const Reference< chart2::XChartTypeManager > & xManager )
{
    m_xChartTypeManager = xManager;
}

Reference< chart2::XChartTypeManager > ImplChartModel::GetChartTypeManager()
{
    return m_xChartTypeManager;
}

void ImplChartModel::SetChartTypeTemplate(
    const Reference< chart2::XChartTypeTemplate > & xTemplate )
{
    m_xChartTypeTemplate = xTemplate;
}

Reference< chart2::XChartTypeTemplate > ImplChartModel::GetChartTypeTemplate()
{
    return m_xChartTypeTemplate;
}

void ImplChartModel::CreateDefaultChart()
{
    // clean up
    RemoveAllDiagrams();

    if( GetChartTypeTemplate().is())
    {
        AppendDiagram(
            GetChartTypeTemplate()->createDiagram( helper::VectorToSequence( m_aInterpretedData )));
    }
}

uno::Reference< chart2::XTitle > ImplChartModel::GetTitle()
{
    return m_xTitle;
}

void ImplChartModel::SetTitle( const uno::Reference< chart2::XTitle >& rTitle )
{
    m_xTitle = rTitle;
}

void ImplChartModel::dispose()
{
    // exception is thrown by ChartModel
    if( m_bIsDisposed )
        return;

    m_bIsDisposed = true;
}

uno::Reference< beans::XPropertySet > ImplChartModel::GetPageBackground()
{
    return m_xPageBackground;
}

void ImplChartModel::CloneData(
    const uno::Reference< sheet::XSpreadsheetDocument > & xCalcDoc )
{
    if( ! xCalcDoc.is())
        return;
    Reference< chart2::XDiagram > xDia( GetDiagram(0));
    if( ! xDia.is())
        return;

    Sequence< Reference< chart2::XDataSeries > > aSeq(
        helper::DataSeriesTreeHelper::getDataSeriesFromDiagram( xDia ));

    try
    {
        // insert new sheet
        Reference< sheet::XSpreadsheets > xSheets( xCalcDoc->getSheets() );
        xSheets->insertNewByName( C2U( "Chart Data" ), 0 );
        Reference< sheet::XSpreadsheet > xDataSheet( xSheets->getByName( C2U( "Chart Data" ) ), uno::UNO_QUERY );
        OSL_ASSERT( xDataSheet.is());

        // delete all except one of the existing sheets
        Reference< container::XNameAccess > xNameAccess( xSheets, uno::UNO_QUERY_THROW );
        OSL_ASSERT( xNameAccess.is());
        Reference< container::XNameContainer > xNameContainer( xNameAccess, uno::UNO_QUERY_THROW );
        OSL_ASSERT( xNameContainer.is());
        Sequence< ::rtl::OUString > aNames( xNameAccess->getElementNames());
        for( sal_Int32 nNameIdx = 1; nNameIdx < aNames.getLength(); ++nNameIdx )
        {
            OSL_TRACE( "Removing %s", U2C( aNames[nNameIdx] ));
            xNameContainer->removeByName( aNames[nNameIdx] );
        }

        // fill data sheet
        static const uno::Type aDoubleType = ::getCppuType( reinterpret_cast< const double * >(0));
        double fVal = 0.0;
        ::rtl::OUString aStrVal;

        sal_Int32 nCol = 0;
        sal_Int32 nMaxRow = 0;
        for( sal_Int32 nSeries = 0; nSeries < aSeq.getLength(); ++nSeries )
        {
            Reference< chart2::XDataSource > xSource( aSeq[nSeries], uno::UNO_QUERY );
            OSL_ASSERT( xSource.is());
            Sequence< Reference< chart2::XDataSequence > > aDataSeq( xSource->getDataSequences());
            for( sal_Int32 nSeqIdx = 0; nSeqIdx < aDataSeq.getLength(); ++nSeqIdx )
            {
                Reference< beans::XPropertySet > xProp( aDataSeq[nSeqIdx],uno::UNO_QUERY );
                ::rtl::OUString aRole;
                if( xProp.is() &&
                    (xProp->getPropertyValue( C2U( "Role" )) >>= aRole ) &&
                    aRole.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "y-values" )) )
                {
                    Sequence< uno::Any > aData( aDataSeq[nSeqIdx]->getData());
                    sal_Int32 nMaxData = aData.getLength();
                    nMaxRow = ::std::max( nMaxRow, nMaxData );
                    for( sal_Int32 nRow = 0; nRow < nMaxData; ++nRow )
                    {
                        Reference< table::XCell > xCell( xDataSheet->getCellByPosition( nCol, nRow ));
                        OSL_ASSERT( xCell.is());
                        if( aData[nRow].isExtractableTo( aDoubleType ))
                        {
                            bool bCouldExtract = (aData[nRow] >>= fVal);
                            OSL_ASSERT( bCouldExtract );
                            xCell->setValue( fVal );
                        }
                        else
                        {
                            bool bCouldExtract = (aData[nRow] >>= aStrVal);
                            OSL_ASSERT( bCouldExtract );
                            xCell->setFormula( aStrVal );
                        }
                    }
                    ++nCol;
                }
            }
        }

        // set new data
        if( nCol > 0 )
        {
            Reference< lang::XMultiServiceFactory > xCalcFact( xCalcDoc, uno::UNO_QUERY_THROW );
            Reference< sheet::XSheetCellRangeContainer > xRangeCnt(
                xCalcFact->createInstance( C2U( "com.sun.star.sheet.SheetCellRanges" )), uno::UNO_QUERY_THROW );
            xRangeCnt->addRangeAddress(
                table::CellRangeAddress( 0, 0, 0, nCol, nMaxRow ), sal_Bool( sal_False ) );
            ::rtl::OUString aRangeStr( xRangeCnt->getRangeAddressesAsString());

            SetRangeRepresentation( aRangeStr );
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void ImplChartModel::CreateDefaultData(
    const uno::Reference< sheet::XSpreadsheetDocument > & xCalcDoc )
{
    if( ! xCalcDoc.is())
        return;

    const sal_Int32 nNumRows = 4;
    const sal_Int32 nNumColumns = 3;

    const double fDefaultData[ nNumColumns ][ nNumRows ] =
        {
            { 9.10,  2.40,  3.10,  4.30 },
            { 3.20,  8.80,  1.50,  9.02 },
            { 4.54,  9.65,  3.70,  6.20 }
        };

    try
    {
        // insert new sheet
        Reference< sheet::XSpreadsheets > xSheets( xCalcDoc->getSheets() );
        xSheets->insertNewByName( C2U( "Chart Data" ), 0 );
        Reference< sheet::XSpreadsheet > xDataSheet( xSheets->getByName( C2U( "Chart Data" ) ), uno::UNO_QUERY );
        OSL_ASSERT( xDataSheet.is());

        // delete all except one of the existing sheets
        Reference< container::XNameAccess > xNameAccess( xSheets, uno::UNO_QUERY_THROW );
        OSL_ASSERT( xNameAccess.is());
        Reference< container::XNameContainer > xNameContainer( xNameAccess, uno::UNO_QUERY_THROW );
        OSL_ASSERT( xNameContainer.is());
        Sequence< ::rtl::OUString > aNames( xNameAccess->getElementNames());
        for( sal_Int32 nNameIdx = 1; nNameIdx < aNames.getLength(); ++nNameIdx )
        {
            OSL_TRACE( "Removing %s", U2C( aNames[nNameIdx] ));
            xNameContainer->removeByName( aNames[nNameIdx] );
        }

        // fill data sheet
        for( sal_Int32 nCol = 0; nCol < nNumColumns; ++nCol )
        {
            for( sal_Int32 nRow = 0; nRow < nNumRows; ++nRow )
            {
                Reference< table::XCell > xCell( xDataSheet->getCellByPosition( nCol, nRow ));
                OSL_ASSERT( xCell.is());
                xCell->setValue( fDefaultData[ nCol ][ nRow ]  );
            }
        }

        Reference< lang::XMultiServiceFactory > xCalcFact( xCalcDoc, uno::UNO_QUERY_THROW );
        Reference< sheet::XSheetCellRangeContainer > xRangeCnt(
            xCalcFact->createInstance( C2U( "com.sun.star.sheet.SheetCellRanges" )), uno::UNO_QUERY_THROW );
        xRangeCnt->addRangeAddress(
            table::CellRangeAddress( 0, 0, 0, nNumColumns, nNumColumns ), sal_Bool( sal_False ) );
        ::rtl::OUString aRangeStr( xRangeCnt->getRangeAddressesAsString());

        SetRangeRepresentation( aRangeStr );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

}  // namespace impl
}  // namespace chart
