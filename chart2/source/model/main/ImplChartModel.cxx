/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ImplChartModel.cxx,v $
 * $Revision: 1.17.44.2 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ImplChartModel.hxx"
#include "DataSeries.hxx"
#include "macros.hxx"
#include "PageBackground.hxx"
#include "DiagramHelper.hxx"
#include "NameContainer.hxx"
#include "CloneHelper.hxx"
#include "ModifyListenerHelper.hxx"
#include "DataSourceHelper.hxx"
#include "DisposeHelper.hxx"
#include "UndoManager.hxx"
#include "ThreeDHelper.hxx"
#include "AxisHelper.hxx"

// header for class SvNumberFormatter
#include <svtools/zforlist.hxx>
// header for class SvNumberFormatsSupplierObj
#include <svtools/numuno.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/component_context.hxx>

#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>

#include <vector>
#include <algorithm>
#include <functional>

using namespace ::com::sun::star;
using namespace ::chart::CloneHelper;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

using ::rtl::OUString;

namespace
{

struct lcl_removeListener : public ::std::unary_function< Reference< chart2::XDiagram >, void >
{
    explicit lcl_removeListener( const Reference< util::XModifyListener > & xListener ) :
            m_xListener( xListener )
    {}

    void operator() ( const Reference< chart2::XDiagram > & xDia )
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( xDia, uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->removeModifyListener( m_xListener );
    }
private:
    Reference< util::XModifyListener > m_xListener;
};

} // anonymous namespace

namespace chart
{
namespace impl
{

ImplChartModel::ImplChartModel(
    Reference< uno::XComponentContext > const & xContext,
    const Reference< util::XModifyListener > & xListener ) :
        m_xContext( xContext ),
        m_spChartData( new ChartData( m_xContext )),
        m_bIsDisposed( false ),
        m_xPageBackground( new PageBackground( m_xContext )),
        m_xUndoManager( new UndoManager()),
        m_xDashTable( new NameContainer( ::getCppuType( reinterpret_cast< const drawing::LineDash * >(0)),
                C2U( "com.sun.star.drawing.DashTable" ), C2U( "com.sun.star.comp.chart.DashTable" ) )),
        m_xGradientTable( new NameContainer( ::getCppuType( reinterpret_cast< const awt::Gradient * >(0)),
                C2U( "com.sun.star.drawing.GradientTable" ), C2U( "com.sun.star.comp.chart.GradientTable" ) )),
        m_xHatchTable( new NameContainer( ::getCppuType( reinterpret_cast< const drawing::Hatch * >(0)),
                C2U( "com.sun.star.drawing.HatchTable" ), C2U( "com.sun.star.comp.chart.HatchTable" ) )),
        m_xBitmapTable( new NameContainer( ::getCppuType( reinterpret_cast< const OUString * >(0)), // URL
                C2U( "com.sun.star.drawing.BitmapTable" ), C2U( "com.sun.star.comp.chart.BitmapTable" ) )),
        m_xTransparencyGradientTable( new NameContainer( ::getCppuType( reinterpret_cast< const awt::Gradient * >(0)),
                C2U( "com.sun.star.drawing.TransparencyGradientTable" ), C2U( "com.sun.star.comp.chart.TransparencyGradientTable" ) )),
        m_xXMLNamespaceMap( new NameContainer( ::getCppuType( (const OUString*) 0 ),
                C2U( "com.sun.star.xml.NamespaceMap" ), C2U( "com.sun.star.comp.chart.XMLNameSpaceMap" ) )),
        m_xModifyListener( xListener )
{
    ModifyListenerHelper::addListener( m_xPageBackground, m_xModifyListener );
    m_xChartTypeManager.set(
        xContext->getServiceManager()->createInstanceWithContext(
            C2U( "com.sun.star.chart2.ChartTypeManager" ),
            xContext ), uno::UNO_QUERY );

    GetStyleFamilies();
    CreateDefaultChartTypeTemplate();
}

ImplChartModel::ImplChartModel( const ImplChartModel & rOther, const Reference< util::XModifyListener > & xListener ) :
        m_xContext( rOther.m_xContext ),
        m_spChartData( rOther.m_spChartData ),
        m_bIsDisposed( rOther.m_bIsDisposed ),
        m_xModifyListener( xListener )
{
    m_xFamilies.set( CreateRefClone< Reference< container::XNameAccess > >()( rOther.m_xFamilies ));
    m_xChartTypeManager.set( CreateRefClone< Reference< chart2::XChartTypeManager > >()( rOther.m_xChartTypeManager ));
    m_xChartTypeTemplate.set( CreateRefClone< Reference< chart2::XChartTypeTemplate > >()( rOther.m_xChartTypeTemplate ));
    m_xTitle.set( CreateRefClone< Reference< chart2::XTitle > >()( rOther.m_xTitle ));
    ModifyListenerHelper::addListener( m_xTitle, m_xModifyListener );
    m_xPageBackground.set( CreateRefClone< Reference< beans::XPropertySet > >()( rOther.m_xPageBackground ));
    ModifyListenerHelper::addListener( m_xPageBackground, m_xModifyListener );

    m_xDashTable.set( CreateRefClone< Reference< container::XNameContainer > >()( rOther.m_xDashTable ));
    m_xGradientTable.set( CreateRefClone< Reference< container::XNameContainer > >()( rOther.m_xGradientTable ));
    m_xHatchTable.set( CreateRefClone< Reference< container::XNameContainer > >()( rOther.m_xHatchTable ));
    m_xBitmapTable.set( CreateRefClone< Reference< container::XNameContainer > >()( rOther.m_xBitmapTable ));
    m_xTransparencyGradientTable.set( CreateRefClone< Reference< container::XNameContainer > >()( rOther.m_xTransparencyGradientTable ));

    m_xXMLNamespaceMap.set( CreateRefClone< Reference< container::XNameAccess > >()( rOther.m_xXMLNamespaceMap ));

    CloneRefVector< Reference< chart2::XDiagram > >( rOther.m_aDiagrams, m_aDiagrams );
}

ImplChartModel::~ImplChartModel()
{}

Reference< container::XNameAccess > ImplChartModel::GetStyleFamilies()
{
    return m_xFamilies;
}

// Diagram Access

void ImplChartModel::RemoveAllDiagrams()
{
    ModifyListenerHelper::removeListenerFromAllElements( m_aDiagrams, m_xModifyListener );
    m_aDiagrams.clear();
}

void ImplChartModel::AppendDiagram( const Reference< chart2::XDiagram > & xDiagram )
{
    Reference< util::XModifyBroadcaster > xBroadcaster( xDiagram, uno::UNO_QUERY );
    ModifyListenerHelper::addListener( xDiagram, m_xModifyListener );
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
    const Reference< chart2::data::XDataProvider > & xProvider )
{
    OSL_ASSERT( m_spChartData.get() );

    m_spChartData->setDataProvider( xProvider );

    //the numberformatter is kept independent of the data provider!

    // release other ressources

    // @todo: maybe we need to save some properties of the old diagrams.  When
    // the data provider changes from an outside Calc to an internal Calc,
    // e.g. when copying a chart into the clipboard as "standalone" format
//     if( bDeleteDiagrams && ! m_aDiagrams.empty())
//         m_aDiagrams.clear();
}

Reference< chart2::data::XDataProvider > ImplChartModel::GetDataProvider() const
{
    OSL_ASSERT( m_spChartData.get() );

    return m_spChartData->getDataProvider();
}

void ImplChartModel::CreateInternalDataProvider(
    bool bCloneExistingData,
    const Reference< chart2::XChartDocument > & xChartDoc )
{
    m_spChartData->createInternalData( bCloneExistingData, xChartDoc );
}

bool ImplChartModel::HasInternalDataProvider() const
{
    OSL_ASSERT( m_spChartData.get() );

    return m_spChartData->hasInternalData();
}

Reference< chart2::data::XDataSource > SAL_CALL ImplChartModel::SetArguments(
    const Sequence< beans::PropertyValue > & aArguments,
    bool bSetData )
    throw (lang::IllegalArgumentException)
{
    Reference< chart2::data::XDataSource > xResult;
    try
    {
        OSL_ASSERT( m_spChartData.get() );

        Reference< chart2::data::XDataProvider > xDataProvider(
            m_spChartData->getDataProvider());
        if( xDataProvider.is() )
        {
            xResult.set( xDataProvider->createDataSource( aArguments ));

            if( bSetData && xResult.is())
                SetNewData( xResult, aArguments );
        }
    }
    catch( lang::IllegalArgumentException & )
    {
        throw;
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< chart2::data::XDataSource > SAL_CALL ImplChartModel::SetRangeRepresentation(
    const OUString & rRangeRepresentation, bool bSetData )
    throw (::com::sun::star::lang::IllegalArgumentException)
{
    uno::Sequence< beans::PropertyValue > aArgs( 4 );
    aArgs[0] = beans::PropertyValue(
        ::rtl::OUString::createFromAscii("CellRangeRepresentation"), -1,
        uno::makeAny( rRangeRepresentation ), beans::PropertyState_DIRECT_VALUE );
    aArgs[1] = beans::PropertyValue(
        ::rtl::OUString::createFromAscii("HasCategories"), -1,
        uno::makeAny( true ), beans::PropertyState_DIRECT_VALUE );
    aArgs[2] = beans::PropertyValue(
        ::rtl::OUString::createFromAscii("FirstCellAsLabel"), -1,
        uno::makeAny( true ), beans::PropertyState_DIRECT_VALUE );
    aArgs[3] = beans::PropertyValue(
        ::rtl::OUString::createFromAscii("DataRowSource"), -1,
        uno::makeAny( ::com::sun::star::chart::ChartDataRowSource_COLUMNS ), beans::PropertyState_DIRECT_VALUE );
    return SetArguments( aArgs, bSetData );
    /*
    uno::Sequence< beans::PropertyValue > aArgs();

    Reference< chart2::data::XDataProvider > xDataProvider( this->GetDataProvider() );
    if( xDataProvider.is() )
        aArgs = xDataProvider->detectArguments( DataSourceHelper::getUsedData( xChartModel ) ),

    ::rtl::OUString aRangeString;
    uno::Sequence< sal_Int32 > aSequenceMapping;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;

    DataSourceHelper::detectRangeSegmentation(
        uno::Reference< frame::XModel >( m_xChartDoc, uno::UNO_QUERY ),
        aRangeString, aSequenceMapping, bUseColumns, bFirstCellAsLabel, bHasCategories );

    aArgs = createArguments( rRangeRepresentation, aSequenceMapping, bUseColumns, bFirstCellAsLabel, bHasCategories ) );

    return SetArguments( aArgs, bSetData );
    */
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

Reference< chart2::XChartTypeTemplate > ImplChartModel::GetChartTypeTemplate()
{
    return m_xChartTypeTemplate;
}

void ImplChartModel::CreateDefaultChart()
{
    CreateDefaultChartTypeTemplate();

    // clean up
    RemoveAllDiagrams();

    Reference< chart2::XChartTypeTemplate > xTemplate( GetChartTypeTemplate());
    if( xTemplate.is())
    {
        try
        {
            Reference< chart2::data::XDataSource > xDataSource( CreateDefaultData());
            Sequence< beans::PropertyValue > aParam;

            Sequence< OUString > aParamNames( xTemplate->getAvailableCreationParameterNames());
            const OUString * pBeg = aParamNames.getConstArray();
            const OUString * pEnd = pBeg + aParamNames.getLength();
            const OUString * pFound( ::std::find( pBeg, pEnd, C2U("HasCategories")));
            if( pFound != pEnd )
            {
                aParam.realloc( 1 );
                aParam[0] = beans::PropertyValue( C2U("HasCategories"), -1, uno::makeAny( true ),
                                                  beans::PropertyState_DIRECT_VALUE );
            }

            Reference< chart2::XDiagram > xDiagram( xTemplate->createDiagramByDataSource( xDataSource, aParam ) );

            AppendDiagram( xDiagram );

            bool bIsRTL = Application::GetSettings().GetLayoutRTL();
            //reverse x axis for rtl charts
            if( bIsRTL )
                AxisHelper::setRTLAxisLayout( AxisHelper::getCoordinateSystemByIndex( xDiagram, 0 ) );

            // create and attach legend
            Reference< chart2::XLegend > xLegend(
                m_xContext->getServiceManager()->createInstanceWithContext(
                    C2U( "com.sun.star.chart2.Legend" ), m_xContext ), uno::UNO_QUERY_THROW );
            Reference< beans::XPropertySet > xLegendProperties( xLegend, uno::UNO_QUERY );
            if( xLegendProperties.is() )
            {
                xLegendProperties->setPropertyValue( C2U( "FillStyle" ), uno::makeAny( drawing::FillStyle_NONE ));
                xLegendProperties->setPropertyValue( C2U( "LineStyle" ), uno::makeAny( drawing::LineStyle_NONE ));
                xLegendProperties->setPropertyValue( C2U( "LineColor" ), uno::makeAny( static_cast< sal_Int32 >( 0xb3b3b3 ) ));  // gray30
                xLegendProperties->setPropertyValue( C2U( "FillColor" ), uno::makeAny( static_cast< sal_Int32 >( 0xe6e6e6 ) ) ); // gray10

                if( bIsRTL )
                    xLegendProperties->setPropertyValue( C2U( "AnchorPosition" ), uno::makeAny( chart2::LegendPosition_LINE_START ));
            }
            if(xDiagram.is())
                xDiagram->setLegend( xLegend );

            // set simple 3D look
            Reference< beans::XPropertySet > xDiagramProperties( xDiagram, uno::UNO_QUERY );
            if( xDiagramProperties.is() )
            {
                xDiagramProperties->setPropertyValue( C2U("RightAngledAxes"), uno::makeAny( sal_True ));
                xDiagramProperties->setPropertyValue( C2U("D3DScenePerspective"), uno::makeAny( drawing::ProjectionMode_PARALLEL ));
                ThreeDHelper::setScheme( xDiagram, ThreeDLookScheme_Simple );
            }

            //set some new 'defaults' for wall and floor
            if( xDiagram.is() )
            {
                Reference< beans::XPropertySet > xWall( xDiagram->getWall() );
                if( xWall.is() )
                {
                    xWall->setPropertyValue( C2U( "LineStyle" ), uno::makeAny( drawing::LineStyle_SOLID ) );
                    xWall->setPropertyValue( C2U( "FillStyle" ), uno::makeAny( drawing::FillStyle_NONE ) );
                    xWall->setPropertyValue( C2U( "LineColor" ), uno::makeAny( static_cast< sal_Int32 >( 0xb3b3b3 ) ) ); // gray30
                    xWall->setPropertyValue( C2U( "FillColor" ), uno::makeAny( static_cast< sal_Int32 >( 0xe6e6e6 ) ) ); // gray10
                }
                Reference< beans::XPropertySet > xFloor( xDiagram->getFloor() );
                if( xFloor.is() )
                {
                    xFloor->setPropertyValue( C2U( "LineStyle" ), uno::makeAny( drawing::LineStyle_NONE ) );
                    xFloor->setPropertyValue( C2U( "FillStyle" ), uno::makeAny( drawing::FillStyle_SOLID ) );
                    xFloor->setPropertyValue( C2U( "LineColor" ), uno::makeAny( static_cast< sal_Int32 >( 0xb3b3b3 ) ) ); // gray30
                    xFloor->setPropertyValue( C2U( "FillColor" ), uno::makeAny( static_cast< sal_Int32 >( 0xcccccc ) ) ); // gray20
                }

            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

Reference< chart2::XTitle > ImplChartModel::GetTitle()
{
    return m_xTitle;
}

void ImplChartModel::SetTitle( const Reference< chart2::XTitle >& rTitle )
{
    if( m_xTitle.is())
        ModifyListenerHelper::removeListener( m_xTitle, m_xModifyListener );
    m_xTitle = rTitle;
    ModifyListenerHelper::addListener( m_xTitle, m_xModifyListener );
}

void ImplChartModel::dispose()
{
    // exception is thrown by ChartModel
    if( m_bIsDisposed )
        return;

    m_spChartData.reset();
    m_xNumberFormatsSupplier.clear();

    DisposeHelper::DisposeAndClear( m_xFamilies );
    DisposeHelper::DisposeAndClear( m_xOwnNumberFormatsSupplier );
    DisposeHelper::DisposeAndClear( m_xChartTypeManager );
    DisposeHelper::DisposeAndClear( m_xChartTypeTemplate );
    DisposeHelper::DisposeAllElements( m_aDiagrams );
    m_aDiagrams.clear();
    DisposeHelper::DisposeAndClear( m_xTitle );
    DisposeHelper::DisposeAndClear( m_xPageBackground );
    DisposeHelper::DisposeAndClear( m_xDashTable );
    DisposeHelper::DisposeAndClear( m_xGradientTable );
    DisposeHelper::DisposeAndClear( m_xHatchTable );
    DisposeHelper::DisposeAndClear( m_xBitmapTable );
    DisposeHelper::DisposeAndClear( m_xTransparencyGradientTable );

    DisposeHelper::DisposeAndClear( m_xXMLNamespaceMap );

    // note: m_xModifyListener is the ChartModel, so don't call dispose()
    m_xModifyListener.clear();

    m_bIsDisposed = true;
}

Reference< beans::XPropertySet > ImplChartModel::GetPageBackground()
{
    return m_xPageBackground;
}

Reference< chart2::XUndoManager > ImplChartModel::GetUndoManager()
{
    return m_xUndoManager;
}

void ImplChartModel::SetNewData( const Reference< chart2::data::XDataSource > & xDataSource,
                                 const Sequence< beans::PropertyValue > & rArgs )
{
    Reference< chart2::XDiagram > xDia;
    if( m_aDiagrams.size() > 0 )
        xDia.set( GetDiagram(0));
    Reference< chart2::XChartTypeTemplate > xTemplate;

    if( xDia.is())
    {
        // apply new data
        DiagramHelper::tTemplateWithServiceName aTemplateAndService =
            DiagramHelper::getTemplateForDiagram(
                xDia, Reference< lang::XMultiServiceFactory >( m_xChartTypeManager, uno::UNO_QUERY ));
        xTemplate.set( aTemplateAndService.first );
    }

    if( !xTemplate.is())
        xTemplate.set( GetChartTypeTemplate());

    if( xTemplate.is())
    {
        if( xDia.is())
            xTemplate->changeDiagramData( xDia, xDataSource, rArgs );
        else
        {
            RemoveAllDiagrams();
            AppendDiagram( xTemplate->createDiagramByDataSource( xDataSource, rArgs ));
        }
    }
}

Reference< chart2::data::XDataSource > ImplChartModel::CreateDefaultData()
{
    Reference< chart2::data::XDataSource > xResult;
    if( m_spChartData->createDefaultData())
        xResult.set( SetRangeRepresentation( C2U("all"), false /* bSetData */ ));
    return xResult;
}

void ImplChartModel::CreateDefaultChartTypeTemplate()
{
    // set default chart type
    Reference< lang::XMultiServiceFactory > xFact( m_xChartTypeManager, uno::UNO_QUERY );
    if( xFact.is() )
    {
        m_xChartTypeTemplate.set(
            xFact->createInstance( C2U( "com.sun.star.chart2.template.Column" ) ), uno::UNO_QUERY );
    }
}

Reference< uno::XInterface > ImplChartModel::GetDashTable() const
{
    return Reference< uno::XInterface >( m_xDashTable );
}
Reference< uno::XInterface > ImplChartModel::GetGradientTable() const
{
    return Reference< uno::XInterface >( m_xGradientTable );
}
Reference< uno::XInterface > ImplChartModel::GetHatchTable() const
{
    return Reference< uno::XInterface >( m_xHatchTable );
}
Reference< uno::XInterface > ImplChartModel::GetBitmapTable() const
{
    return Reference< uno::XInterface >( m_xBitmapTable );
}
Reference< uno::XInterface > ImplChartModel::GetTransparencyGradientTable() const
{
    return Reference< uno::XInterface >( m_xTransparencyGradientTable );
}

Reference< uno::XInterface > ImplChartModel::GetXMLNameSpaceMap() const
{
    return Reference< uno::XInterface >( m_xXMLNamespaceMap );
}

void ImplChartModel::SetNumberFormatsSupplier(
    const Reference< util::XNumberFormatsSupplier > & xNew )
{
    if( xNew==m_xNumberFormatsSupplier )
        return;
    if( xNew==m_xOwnNumberFormatsSupplier )
        return;
    if( m_xOwnNumberFormatsSupplier.is() && xNew.is() )
    {
        //@todo
        //merge missing numberformats from own to new formatter
    }
    else if( !xNew.is() )
    {
        if( m_xNumberFormatsSupplier.is() )
        {
            //@todo
            //merge missing numberformats from old numberformatter to own numberformatter
            //create own numberformatter if necessary
        }
    }

    m_xNumberFormatsSupplier.set( xNew );
    m_xOwnNumberFormatsSupplier.clear();
}

Reference< util::XNumberFormatsSupplier > ImplChartModel::GetNumberFormatsSupplier()
{
    if( !m_xNumberFormatsSupplier.is() )
    {
        if( !m_xOwnNumberFormatsSupplier.is() )
        {
            Reference< lang::XMultiServiceFactory > xFactory( m_xContext->getServiceManager(), uno::UNO_QUERY );
            m_xOwnNumberFormatsSupplier = new SvNumberFormatsSupplierObj( new SvNumberFormatter( xFactory, LANGUAGE_SYSTEM ) );
            //pOwnNumberFormatter->ChangeStandardPrec( 15 ); todo?
        }
        m_xNumberFormatsSupplier = m_xOwnNumberFormatsSupplier;
    }
    return m_xNumberFormatsSupplier;
}

}  // namespace impl
}  // namespace chart
