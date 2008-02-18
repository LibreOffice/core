/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartController_Properties.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:57:53 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ChartController.hxx"
#include "ChartWindow.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "ObjectIdentifier.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "macros.hxx"
#include "dlg_ObjectProperties.hxx"
#include "dlg_View3D.hxx"
#include "dlg_InsertErrorBars.hxx"
#include "ViewElementListProvider.hxx"
#include "DataPointItemConverter.hxx"
#include "AxisItemConverter.hxx"
#include "MultipleChartConverters.hxx"
#include "TitleItemConverter.hxx"
#include "LegendItemConverter.hxx"
#include "RegressionCurveItemConverter.hxx"
#include "RegressionEquationItemConverter.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"
#include "TitleHelper.hxx"
#include "LegendHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "ColorPerPointHelper.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "ControllerLockGuard.hxx"
#include "UndoGuard.hxx"
#include "ObjectNameProvider.hxx"
#include "ResId.hxx"
#include "Strings.hrc"
#include "ReferenceSizeProvider.hxx"

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif

//for auto_ptr
#include <memory>

// for test in executeDispatch_ObjectToDefault
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

// header for define RET_OK
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
// for SolarMutex
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SVX_ACTIONDESCRIPTIONPROVIDER_HXX
#include <svx/ActionDescriptionProvider.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace
{

::comphelper::ItemConverter* createItemConverter(
    const ::rtl::OUString & aObjectCID
    , const uno::Reference< frame::XModel > & xChartModel
    , const uno::Reference< uno::XComponentContext > & xContext
    , SdrModel & rDrawModel
    , NumberFormatterWrapper * pNumberFormatterWrapper = NULL
    , ExplicitValueProvider * pExplicitValueProvider = NULL
    , ::std::auto_ptr< ReferenceSizeProvider > pRefSizeProvider =
          ::std::auto_ptr< ReferenceSizeProvider >()
    )
{
    ::comphelper::ItemConverter* pItemConverter=NULL;

    //-------------------------------------------------------------
    //get type of selected object
    ObjectType eObjectType = ObjectIdentifier::getObjectType( aObjectCID );
    if( OBJECTTYPE_UNKNOWN==eObjectType )
    {
        DBG_ERROR("unknown ObjectType");
        return NULL;
    }
    //--
    rtl::OUString aParticleID = ObjectIdentifier::getParticleID( aObjectCID );
    bool bAffectsMultipleObjects = aParticleID.equals(C2U("ALLELEMENTS"));
    //-------------------------------------------------------------
    if( !bAffectsMultipleObjects )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties =
            ObjectIdentifier::getObjectPropertySet( aObjectCID, xChartModel );
        if(!xObjectProperties.is())
            return NULL;
        //create itemconverter for a single object
        switch(eObjectType)
        {
            case OBJECTTYPE_PAGE:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                        wrapper::GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES );
                    break;
            case OBJECTTYPE_TITLE:
            {
                ::std::auto_ptr< awt::Size > pRefSize;
                if( pRefSizeProvider.get() )
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                pItemConverter = new wrapper::TitleItemConverter( xObjectProperties,
                                                                  rDrawModel.GetItemPool(), rDrawModel,
                                                                  uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                                                  pRefSize );
            }
            break;
            case OBJECTTYPE_LEGEND:
            {
                ::std::auto_ptr< awt::Size > pRefSize;
                if( pRefSizeProvider.get() )
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                pItemConverter = new wrapper::LegendItemConverter( xObjectProperties,
                                                                   rDrawModel.GetItemPool(), rDrawModel,
                                                                   uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                                                   pRefSize );
            }
            break;
            case OBJECTTYPE_LEGEND_ENTRY:
                    break;
            case OBJECTTYPE_DIAGRAM:
                    break;
            case OBJECTTYPE_DIAGRAM_WALL:
            case OBJECTTYPE_DIAGRAM_FLOOR:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                        wrapper::GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES );
                    break;
            case OBJECTTYPE_AXIS:
            {
                ::std::auto_ptr< awt::Size > pRefSize;
                if( pRefSizeProvider.get() )
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getDiagramSize()));

                uno::Reference< beans::XPropertySet > xDiaProp;
                xDiaProp.set( ChartModelHelper::findDiagram( xChartModel ), uno::UNO_QUERY );

                // the second property set contains the property CoordinateOrigin
                // nOriginIndex is the index of the corresponding index of the
                // origin (x=0, y=1, z=2)

                ExplicitScaleData aExplicitScale;
                ExplicitIncrementData aExplicitIncrement;
                if( pExplicitValueProvider )
                    pExplicitValueProvider->getExplicitValuesForAxis(
                        uno::Reference< XAxis >( xObjectProperties, uno::UNO_QUERY ),
                        aExplicitScale, aExplicitIncrement );

                pItemConverter =  new wrapper::AxisItemConverter(
                    xObjectProperties, rDrawModel.GetItemPool(),
                    rDrawModel,
                    uno::Reference< chart2::XChartDocument >( xChartModel, uno::UNO_QUERY ),
                    &aExplicitScale, &aExplicitIncrement,
                    pRefSize );
            }
            break;
            case OBJECTTYPE_AXIS_UNITLABEL:
                    break;
            case OBJECTTYPE_DATA_LABELS:
            case OBJECTTYPE_DATA_SERIES:
            case OBJECTTYPE_DATA_LABEL:
            case OBJECTTYPE_DATA_POINT:
            {
                ::std::auto_ptr< awt::Size > pRefSize;
                if( pRefSizeProvider.get() )
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getDiagramSize()));

                wrapper::GraphicPropertyItemConverter::eGraphicObjectType eMapTo =
                    wrapper::GraphicPropertyItemConverter::FILLED_DATA_POINT;

                uno::Reference< XDataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( aObjectCID, xChartModel );
                uno::Reference< XChartType > xChartType = ChartModelHelper::getChartTypeOfSeries( xChartModel, xSeries );

                uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
                sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
                if( !ChartTypeHelper::isSupportingAreaProperties( xChartType, nDimensionCount ) )
                    eMapTo = wrapper::GraphicPropertyItemConverter::LINE_DATA_POINT;
                /*
                FILLED_DATA_POINT,
                LINE_DATA_POINT,
                LINE_PROPERTIES,
                FILL_PROPERTIES,
                LINE_AND_FILL_PROPERTIES
                */
                bool bDataSeries = ( eObjectType == OBJECTTYPE_DATA_SERIES || eObjectType == OBJECTTYPE_DATA_LABELS );

                //special color for pie chart:
                bool bUseSpecialFillColor = false;
                sal_Int32 nSpecialFillColor =0;
                sal_Int32 nPointIndex = -1; /*-1 for whole series*/
                if(!bDataSeries)
                {
                    nPointIndex = aParticleID.toInt32();
                    uno::Reference< beans::XPropertySet > xSeriesProp( xSeries, uno::UNO_QUERY );
                    bool bVaryColorsByPoint = false;
                    if( xSeriesProp.is() &&
                        (xSeriesProp->getPropertyValue(C2U("VaryColorsByPoint")) >>= bVaryColorsByPoint) &&
                        bVaryColorsByPoint )
                    {
                        if( !ColorPerPointHelper::hasPointOwnColor( xSeriesProp, nPointIndex, xObjectProperties ) )
                        {
                            bUseSpecialFillColor = true;
                            OSL_ASSERT( xDiagram.is());
                            uno::Reference< XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme() );
                            if( xColorScheme.is())
                                nSpecialFillColor = xColorScheme->getColorByIndex( nPointIndex );
                        }
                    }
                }
                sal_Int32 nNumberFormat=ExplicitValueProvider::getExplicitNumberFormatKeyForLabel( xObjectProperties, xSeries, nPointIndex,
                        uno::Reference< beans::XPropertySet >( DiagramHelper::getAttachedAxis( xSeries, xDiagram ), uno::UNO_QUERY ) );
                sal_Int32 nPercentNumberFormat=ExplicitValueProvider::getExplicitPercentageNumberFormatKeyForLabel(
                        xObjectProperties,uno::Reference< util::XNumberFormatsSupplier >(xChartModel, uno::UNO_QUERY));

                pItemConverter =  new wrapper::DataPointItemConverter( xChartModel, xContext,
                                        xObjectProperties, xSeries, rDrawModel.GetItemPool(), rDrawModel,
                                        pNumberFormatterWrapper,
                                        uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                        eMapTo, pRefSize, bDataSeries, bUseSpecialFillColor, nSpecialFillColor, false,
                                        nNumberFormat, nPercentNumberFormat );
                    break;
            }
            case OBJECTTYPE_GRID:
            case OBJECTTYPE_SUBGRID:
            case OBJECTTYPE_DATA_ERRORS:
            case OBJECTTYPE_DATA_AVERAGE_LINE:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                        wrapper::GraphicPropertyItemConverter::LINE_PROPERTIES );
                    break;

            case OBJECTTYPE_DATA_CURVE:
                pItemConverter =  new wrapper::RegressionCurveItemConverter(
                    xObjectProperties, uno::Reference< chart2::XRegressionCurveContainer >(
                        ObjectIdentifier::getDataSeriesForCID( aObjectCID, xChartModel ), uno::UNO_QUERY ),
                    rDrawModel.GetItemPool(), rDrawModel,
                    uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ));
                break;
            case OBJECTTYPE_DATA_CURVE_EQUATION:
            {
                ::std::auto_ptr< awt::Size > pRefSize;
                if( pRefSizeProvider.get() )
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                pItemConverter =  new wrapper::RegressionEquationItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(), rDrawModel,
                                        uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                        pRefSize );
                    break;
            }
            case OBJECTTYPE_DATA_ERRORS_X:
                    break;
            case OBJECTTYPE_DATA_ERRORS_Y:
                    break;
            case OBJECTTYPE_DATA_ERRORS_Z:
                    break;
            case OBJECTTYPE_DATA_STOCK_RANGE:
                    break;
            case OBJECTTYPE_DATA_STOCK_LOSS:
            case OBJECTTYPE_DATA_STOCK_GAIN:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                        wrapper::GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES );
                    break;
            default: //OBJECTTYPE_UNKNOWN
                    break;
        }
    }
    else
    {
        //create itemconverter for a all objects of given type
        switch(eObjectType)
        {
            case OBJECTTYPE_TITLE:
                pItemConverter =  new wrapper::AllTitleItemConverter( xChartModel, rDrawModel.GetItemPool(),
                                                                     rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ));
                break;
            case OBJECTTYPE_AXIS:
            {
                ::std::auto_ptr< awt::Size > pRefSize;
                if( pRefSizeProvider.get() )
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getDiagramSize()));

                pItemConverter =  new wrapper::AllAxisItemConverter( xChartModel, rDrawModel.GetItemPool(),
                                                                     rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ), pRefSize );
            }
            break;
            case OBJECTTYPE_GRID:
            case OBJECTTYPE_SUBGRID:
                pItemConverter =  new wrapper::AllGridItemConverter( xChartModel, rDrawModel.GetItemPool(),
                                                                     rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ));
                break;
            default: //for this type it is not supported to change all elements at once
                break;
        }

    }
    return pItemConverter;
}

rtl::OUString lcl_getTitleCIDForCommand( const ::rtl::OString& rDispatchCommand, const uno::Reference< frame::XModel > & xChartModel )
{
    if( rDispatchCommand.equals("AllTitles"))
        return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_TITLE, C2U("ALLELEMENTS") );

    TitleHelper::eTitleType nTitleType( TitleHelper::MAIN_TITLE );
    if( rDispatchCommand.equals("SubTitle") )
        nTitleType = TitleHelper::SUB_TITLE;
    else if( rDispatchCommand.equals("XTitle") )
        nTitleType = TitleHelper::X_AXIS_TITLE;
    else if( rDispatchCommand.equals("YTitle") )
        nTitleType = TitleHelper::Y_AXIS_TITLE;
    else if( rDispatchCommand.equals("ZTitle") )
        nTitleType = TitleHelper::Z_AXIS_TITLE;
    else if( rDispatchCommand.equals("SecondaryXTitle") )
        nTitleType = TitleHelper::SECONDARY_X_AXIS_TITLE;
    else if( rDispatchCommand.equals("SecondaryYTitle") )
        nTitleType = TitleHelper::SECONDARY_Y_AXIS_TITLE;

    uno::Reference< XTitle > xTitle( TitleHelper::getTitle( nTitleType, xChartModel ) );
    return ObjectIdentifier::createClassifiedIdentifierForObject( xTitle, xChartModel );
}

rtl::OUString lcl_getAxisCIDForCommand( const ::rtl::OString& rDispatchCommand, const uno::Reference< frame::XModel >& xChartModel )
{
    if( rDispatchCommand.equals("DiagramAxisAll"))
        return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_AXIS, C2U("ALLELEMENTS") );

    sal_Int32   nDimensionIndex=0;
    bool        bMainAxis=true;
    if( rDispatchCommand.equals("DiagramAxisX"))
    {
        nDimensionIndex=0; bMainAxis=true;
    }
    else if( rDispatchCommand.equals("DiagramAxisY"))
    {
        nDimensionIndex=1; bMainAxis=true;
    }
    else if( rDispatchCommand.equals("DiagramAxisZ"))
    {
        nDimensionIndex=2; bMainAxis=true;
    }
    else if( rDispatchCommand.equals("DiagramAxisA"))
    {
        nDimensionIndex=0; bMainAxis=false;
    }
    else if( rDispatchCommand.equals("DiagramAxisB"))
    {
        nDimensionIndex=1; bMainAxis=false;
    }

    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    uno::Reference< XAxis > xAxis( AxisHelper::getAxis( nDimensionIndex, bMainAxis, xDiagram ) );
    return ObjectIdentifier::createClassifiedIdentifierForObject( xAxis, xChartModel );
}

rtl::OUString lcl_getGridCIDForCommand( const ::rtl::OString& rDispatchCommand, const uno::Reference< frame::XModel >& xChartModel )
{
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );

    if( rDispatchCommand.equals("DiagramGridAll"))
        return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_GRID, C2U("ALLELEMENTS") );

    sal_Int32   nDimensionIndex=0;
    bool        bMainGrid=true;

    //x and y is swapped in the commands

    if( rDispatchCommand.equals("DiagramGridYMain"))
    {
        nDimensionIndex=0; bMainGrid=true;
    }
    else if( rDispatchCommand.equals("DiagramGridXMain"))
    {
        nDimensionIndex=1; bMainGrid=true;
    }
    else if( rDispatchCommand.equals("DiagramGridZMain"))
    {
        nDimensionIndex=2; bMainGrid=true;
    }
    else if( rDispatchCommand.equals("DiagramGridYHelp"))
    {
        nDimensionIndex=0; bMainGrid=false;
    }
    else if( rDispatchCommand.equals("DiagramGridXHelp"))
    {
        nDimensionIndex=1; bMainGrid=false;
    }
    else if( rDispatchCommand.equals("DiagramGridZHelp"))
    {
        nDimensionIndex=2; bMainGrid=false;
    }

    bool bMainAxis = true;
    uno::Reference< XAxis > xAxis( AxisHelper::getAxis( nDimensionIndex, bMainAxis, xDiagram ) );

    sal_Int32   nSubGridIndex= bMainGrid ? (-1) : 0;
    rtl::OUString aCID( ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartModel, nSubGridIndex ) );
    return aCID;
}
rtl::OUString lcl_getObjectCIDForCommand( const ::rtl::OString& rDispatchCommand, const uno::Reference< XChartDocument > & xChartDocument )
{
    ObjectType eObjectType = OBJECTTYPE_UNKNOWN;
    rtl::OUString aParticleID;

    uno::Reference< frame::XModel > xChartModel( xChartDocument, uno::UNO_QUERY );

    //-------------------------------------------------------------------------
    //legend
    if( rDispatchCommand.equals("Legend"))
    {
        eObjectType = OBJECTTYPE_LEGEND;
        //@todo set particular aParticleID if we have more than one legend
    }
    //-------------------------------------------------------------------------
    //wall floor area
    else if( rDispatchCommand.equals("DiagramWall"))
    {
        //OBJECTTYPE_DIAGRAM;
        eObjectType = OBJECTTYPE_DIAGRAM_WALL;
        //@todo set particular aParticleID if we have more than one diagram
    }
    else if( rDispatchCommand.equals("DiagramFloor"))
    {
        eObjectType = OBJECTTYPE_DIAGRAM_FLOOR;
        //@todo set particular aParticleID if we have more than one diagram
    }
    else if( rDispatchCommand.equals("DiagramArea"))
    {
        eObjectType = OBJECTTYPE_PAGE;
    }
    //-------------------------------------------------------------------------
    //title
    else if( rDispatchCommand.equals("MainTitle")
        || rDispatchCommand.equals("SubTitle")
        || rDispatchCommand.equals("XTitle")
        || rDispatchCommand.equals("YTitle")
        || rDispatchCommand.equals("ZTitle")
        || rDispatchCommand.equals("SecondaryXTitle")
        || rDispatchCommand.equals("SecondaryYTitle")
        || rDispatchCommand.equals("AllTitles")
        )
    {
        return lcl_getTitleCIDForCommand( rDispatchCommand, xChartModel );
    }
    //-------------------------------------------------------------------------
    //axis
    else if( rDispatchCommand.equals("DiagramAxisX")
        || rDispatchCommand.equals("DiagramAxisY")
        || rDispatchCommand.equals("DiagramAxisZ")
        || rDispatchCommand.equals("DiagramAxisA")
        || rDispatchCommand.equals("DiagramAxisB")
        || rDispatchCommand.equals("DiagramAxisAll")
        )
    {
        return lcl_getAxisCIDForCommand( rDispatchCommand, xChartModel );
    }
    //-------------------------------------------------------------------------
    //grid
    else if( rDispatchCommand.equals("DiagramGridYMain")
        || rDispatchCommand.equals("DiagramGridXMain")
        || rDispatchCommand.equals("DiagramGridZMain")
        || rDispatchCommand.equals("DiagramGridYHelp")
        || rDispatchCommand.equals("DiagramGridXHelp")
        || rDispatchCommand.equals("DiagramGridZHelp")
        || rDispatchCommand.equals("DiagramGridAll")
        )
    {
        return lcl_getGridCIDForCommand( rDispatchCommand, xChartModel );
    }
    return ObjectIdentifier::createClassifiedIdentifier(
        eObjectType, aParticleID );
}

}
// anonymous namespace

void SAL_CALL ChartController::executeDispatch_FormatObject(const ::rtl::OUString& rDispatchCommand)
{
    uno::Reference< XChartDocument > xChartDocument( m_aModel->getModel(), uno::UNO_QUERY );
    rtl::OString aCommand( rtl::OUStringToOString( rDispatchCommand, RTL_TEXTENCODING_ASCII_US ) );
    rtl::OUString rObjectCID = lcl_getObjectCIDForCommand( aCommand, xChartDocument );
    executeDlg_ObjectProperties( rObjectCID );
}

void SAL_CALL ChartController::executeDispatch_ObjectProperties()
{
    executeDlg_ObjectProperties( m_aSelection.getSelectedCID() );
}

void SAL_CALL ChartController::executeDlg_ObjectProperties( const ::rtl::OUString& rObjectCID )
{
    if( !rObjectCID.getLength() )
    {
        //DBG_ERROR("empty ObjectID");
        return;
    }
    try
    {
        ::rtl::OUString aObjectCID(rObjectCID);
        NumberFormatterWrapper aNumberFormatterWrapper( uno::Reference< util::XNumberFormatsSupplier >(m_aModel->getModel(), uno::UNO_QUERY) );

        //-------------------------------------------------------------
        //get type of selected object
        ObjectType eObjectType = ObjectIdentifier::getObjectType( aObjectCID );
        if( OBJECTTYPE_UNKNOWN==eObjectType )
        {
            //DBG_ERROR("unknown ObjectType");
            return;
        }

        // some legend entries are handled as if they were data series
        if( OBJECTTYPE_LEGEND_ENTRY==eObjectType )
        {
            rtl::OUString aParentParticle( ObjectIdentifier::getFullParentParticle( aObjectCID ) );
            eObjectType = ObjectIdentifier::getObjectType( aParentParticle );
            aObjectCID  = ObjectIdentifier::createClassifiedIdentifierForParticle( aParentParticle );
        }

        // treat diagram as wall
        if( OBJECTTYPE_DIAGRAM==eObjectType )
        {
            aObjectCID  = ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_WALL, rtl::OUString() );
            eObjectType = OBJECTTYPE_DIAGRAM_WALL;
        }

        if( OBJECTTYPE_DIAGRAM_WALL==eObjectType || OBJECTTYPE_DIAGRAM_FLOOR==eObjectType )
        {
            if( !DiagramHelper::isSupportingFloorAndWall( ChartModelHelper::findDiagram( m_aModel->getModel() ) ) )
                return;
        }

        //-------------------------------------------------------------
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::FORMAT,
                ObjectNameProvider::getName( ObjectIdentifier::getObjectType( aObjectCID ))),
            m_xUndoManager, m_aModel->getModel() );

        //-------------------------------------------------------------
        //convert properties to ItemSet

        awt::Size aPageSize( ChartModelHelper::getPageSize(m_aModel->getModel()) );

        ::std::auto_ptr< ReferenceSizeProvider > pRefSizeProv(
            impl_createReferenceSizeProvider());
        ::std::auto_ptr< ::comphelper::ItemConverter > apItemConverter(
            createItemConverter( aObjectCID, m_aModel->getModel(), m_xCC,
                                 m_pDrawModelWrapper->getSdrModel(),
                                 &aNumberFormatterWrapper,
                                 ExplicitValueProvider::getExplicitValueProvider(m_xChartView),
                                 pRefSizeProv ));
        if(!apItemConverter.get())
            return;

        SfxItemSet aItemSet = apItemConverter->CreateEmptyItemSet();
        apItemConverter->FillItemSet( aItemSet );

        //-------------------------------------------------------------
        //prepare dialog
        ObjectPropertiesDialogParameter aDialogParameter = ObjectPropertiesDialogParameter( aObjectCID );
        aDialogParameter.init( m_aModel->getModel() );
        ViewElementListProvider aViewElementListProvider( m_pDrawModelWrapper.get() );

        ::vos::OGuard aGuard( Application::GetSolarMutex());
        SchAttribTabDlg aDlg( m_pChartWindow, &aItemSet, &aDialogParameter, &aViewElementListProvider
            , uno::Reference< util::XNumberFormatsSupplier >( m_aModel->getModel(), uno::UNO_QUERY ) );

        if(aDialogParameter.HasSymbolProperties())
        {
            SfxItemSet* pSymbolShapeProperties=NULL;
            uno::Reference< beans::XPropertySet > xObjectProperties =
                ObjectIdentifier::getObjectPropertySet( aObjectCID, m_aModel->getModel() );
            wrapper::DataPointItemConverter aSymbolItemConverter( m_aModel->getModel(), m_xCC
                                        , xObjectProperties, ObjectIdentifier::getDataSeriesForCID( aObjectCID, m_aModel->getModel() )
                                        , m_pDrawModelWrapper->getSdrModel().GetItemPool()
                                        , m_pDrawModelWrapper->getSdrModel()
                                        , &aNumberFormatterWrapper
                                        , uno::Reference< lang::XMultiServiceFactory >( m_aModel->getModel(), uno::UNO_QUERY )
                                        , wrapper::GraphicPropertyItemConverter::FILLED_DATA_POINT );

            pSymbolShapeProperties = new SfxItemSet( aSymbolItemConverter.CreateEmptyItemSet() );
            aSymbolItemConverter.FillItemSet( *pSymbolShapeProperties );

            sal_Int32   nStandardSymbol=0;//@todo get from somewhere
            Graphic*    pAutoSymbolGraphic = new Graphic( aViewElementListProvider.GetSymbolGraphic( nStandardSymbol, pSymbolShapeProperties ) );
            // note: the dialog takes the ownership of pSymbolShapeProperties and pAutoSymbolGraphic
            aDlg.setSymbolInformation( pSymbolShapeProperties, pAutoSymbolGraphic );
        }
        if( aDialogParameter.HasRegressionProperties() )
        {
            aDlg.SetAxisMinorStepWidthForErrorBarDecimals(
                InsertErrorBarsDialog::getAxisMinorStepWidthForErrorBarDecimals( m_aModel->getModel(), m_xChartView, aObjectCID ) );
        }

        //-------------------------------------------------------------
        //open the dialog
        if( aDlg.Execute() == RET_OK )
        {
            const SfxItemSet* pOutItemSet = aDlg.GetOutputItemSet();
            if(pOutItemSet)
            {
                bool bChanged = false;
                {
                    ControllerLockGuard aCLGuard( m_aModel->getModel());
                    bChanged = apItemConverter->ApplyItemSet( *pOutItemSet );//model should be changed now
                }

                if( bChanged )
                    aUndoGuard.commitAction();
            }
        }
    }
    catch( util::CloseVetoException& )
    {
    }
    catch( uno::RuntimeException& )
    {
    }
}

void SAL_CALL ChartController::executeDispatch_ObjectToDefault()
{
    ::rtl::OUString aObjectCID(m_aSelection.getSelectedCID());
    if( !aObjectCID.getLength() )
    {
        DBG_ERROR("nothing is selected");
        return;
    }

    try
    {
        //-------------------------------------------------------------
        //get type of selected object
        ObjectType eObjectType = ObjectIdentifier::getObjectType( aObjectCID );
        if( OBJECTTYPE_UNKNOWN==eObjectType )
        {
            DBG_ERROR("unknown ObjectType");
            return;
        }
        //-------------------------------------------------------------
        //get properties of selected object
        uno::Reference< beans::XPropertySet > xObjectProperties = NULL;
        xObjectProperties = ObjectIdentifier::getObjectPropertySet( aObjectCID, getModel() );
        if(!xObjectProperties.is())
            return;

        uno::Sequence< beans::Property > aProps( xObjectProperties->getPropertySetInfo()->getProperties() );
        uno::Reference< beans::XPropertyState > xState( xObjectProperties, uno::UNO_QUERY );

        if( xState.is() )
        {
            for( sal_Int32 i = 0; i < aProps.getLength(); ++i )
            {
                if( aProps[i].Attributes & beans::PropertyAttribute::MAYBEDEFAULT )
                    xState->setPropertyToDefault( aProps[i].Name );
            }
        }
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

void SAL_CALL ChartController::executeDispatch_View3D()
{
    try
    {
        // using assignment for broken gcc 3.3
        UndoLiveUpdateGuard aUndoGuard = UndoLiveUpdateGuard(
            ::rtl::OUString( String( SchResId( STR_ACTION_EDIT_3D_VIEW ))),
            m_xUndoManager, m_aModel->getModel());

        // /--
        //open dialog
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        View3DDialog aDlg( m_pChartWindow, m_aModel->getModel(), m_pDrawModelWrapper->GetColorTable() );
        if( aDlg.Execute() == RET_OK )
            aUndoGuard.commitAction();
        // \--
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
