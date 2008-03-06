/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartController_Tools.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:59:46 $
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
#include "ChartModelHelper.hxx"
#include "TitleHelper.hxx"
#include "ThreeDHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "UndoGuard.hxx"
#include "ControllerLockGuard.hxx"
#include "macros.hxx"
#include "ResId.hxx"
#include "Strings.hrc"
#include "ObjectIdentifier.hxx"
#include "ReferenceSizeProvider.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "ChartTransferable.hxx"
#include "DrawViewWrapper.hxx"
#include "LegendHelper.hxx"
#include "AxisHelper.hxx"
#include "RegressionCurveHelper.hxx"

#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>

// #include <com/sun/star/drawing/XDrawPageSupplier.hpp>

#include <svx/ActionDescriptionProvider.hxx>
// for TransferableDataHelper/TransferableHelper
#include <svtools/transfer.hxx>
// for SotStorageStreamRef
#include <sot/storage.hxx>
// for Graphic
#include <vcl/graph.hxx>
// for SvxDrawingLayerImport/SvxUnoDrawingModel
#include <svx/unomodel.hxx>
// for SdrModel
#include <svx/svdmodel.hxx>
// for OInputStreamWrapper
#include <unotools/streamwrap.hxx>
// for SolarMutex
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace
{

bool lcl_deleteDataSeries(
    const OUString & rCID,
    const Reference< frame::XModel > & xModel,
    const Reference< chart2::XUndoManager > & xUndoManager )
{
    bool bResult = false;
    uno::Reference< chart2::XDataSeries > xSeries( ::chart::ObjectIdentifier::getDataSeriesForCID( rCID, xModel ));
    uno::Reference< chart2::XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
    if( xSeries.is() && xChartDoc.is())
    {
        uno::Reference< chart2::XChartType > xChartType(
            ::chart::DataSeriesHelper::getChartTypeOfSeries( xSeries, xChartDoc->getFirstDiagram()));
        if( xChartType.is())
        {
            ::chart::UndoGuard aUndoGuard(
                ActionDescriptionProvider::createDescription(
                    ActionDescriptionProvider::DELETE, ::rtl::OUString( String( ::chart::SchResId( STR_OBJECT_DATASERIES )))),
                xUndoManager, xModel );
            ::chart::DataSeriesHelper::deleteSeries( xSeries, xChartType );
            bResult = true;
            aUndoGuard.commitAction();
        }
    }
    return bResult;
}

bool lcl_deleteDataCurve(
    const OUString & rCID,
    const Reference< frame::XModel > & xModel,
    const Reference< chart2::XUndoManager > & xUndoManager )
{
    bool bResult = false;
    uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt(
        ::chart::ObjectIdentifier::getObjectPropertySet(
            ::chart::ObjectIdentifier::getSeriesParticleFromCID( rCID ), xModel ), uno::UNO_QUERY );
    if( xRegCurveCnt.is())
    {
        ::chart::UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::DELETE, ::rtl::OUString( String( ::chart::SchResId( STR_OBJECT_CURVE )))),
            xUndoManager, xModel );
        ::chart::RegressionCurveHelper::removeAllExceptMeanValueLine( xRegCurveCnt );
        bResult = true;
        aUndoGuard.commitAction();
    }
    return bResult;
}

// void lcl_CopyPageContentToPage(
//     const Reference< drawing::XDrawPage > & xSourcePage,
//     const Reference< drawing::XDrawPage > & xDestPage )
// {
//     try
//     {
//         Reference< container::XIndexAccess > xSourceIA( xSourcePage, uno::UNO_QUERY_THROW );
//         sal_Int32 nCount( xSourceIA->getCount());
//         for( sal_Int32 i=0; i<nCount; ++i )
//         {
//             Reference< drawing::XShape > xShape;
//             if( xSourceIA->getByIndex( i ) >>= xShape )
//                 xDestPage->add( xShape );
//         }
//     }
//     catch( const uno::Exception & ex )
//     {
//         ASSERT_EXCEPTION( ex );
//     }
// }

// // copies all shapes on all pages of xSource to the only page of xDestination
// void lcl_CopyShapesToChart(
//     const Reference< frame::XModel > & xSource, const Reference< frame::XModel > & xDestination )
// {
//     try
//     {
//         Reference< drawing::XDrawPageSupplier > xDestPGSupp( xDestination, uno::UNO_QUERY_THROW );
//         Reference< drawing::XDrawPage > xDestPage( xDestPGSupp->getDrawPage());
//         Reference< drawing::XDrawPagesSupplier > xSourcePGsSupp( xSource, uno::UNO_QUERY_THROW );
//         Reference< drawing::XDrawPages > xSourcePages( xSourcePGsSupp->getDrawPages());

//         sal_Int32 nCount( xSourcePages->getCount());
//         for( sal_Int32 i=0; i<nCount; ++i )
//         {
//             Reference< drawing::XDrawPage > xSourcePage( xSourcePages->getByIndex( i ), uno::UNO_QUERY_THROW );
//             lcl_CopyPageContentToPage( xSourcePage, xDestPage );
//         }
//     }
//     catch( const uno::Exception & ex )
//     {
//         ASSERT_EXCEPTION( ex );
//     }
// }

} // anonymous namespace

namespace
{
void lcl_InsertStringAsTextShapeIntoDrawPage(
    const Reference< lang::XMultiServiceFactory > & xShapeFactory,
    const Reference< drawing::XDrawPage > & xDrawPage,
    OUString & rString,
    const awt::Point & aPosition )
{
    OSL_ASSERT( xShapeFactory.is() && xDrawPage.is());
    if( ! (xShapeFactory.is()  && xDrawPage.is()))
        return;

    try
    {
        Reference< drawing::XShape > xTextShape(
            xShapeFactory->createInstance( C2U("com.sun.star.drawing.TextShape")), uno::UNO_QUERY_THROW );
        xDrawPage->add( xTextShape );

        Reference< text::XTextRange > xRange( xTextShape, uno::UNO_QUERY_THROW );
        xRange->setString( rString );

        float fCharHeight = 10.0;
        Reference< beans::XPropertySet > xProperties( xTextShape, uno::UNO_QUERY_THROW );
        xProperties->setPropertyValue( C2U("TextAutoGrowHeight"), uno::makeAny( true ));
        xProperties->setPropertyValue( C2U("TextAutoGrowWidth"), uno::makeAny( true ));
        xProperties->setPropertyValue( C2U("CharHeight"), uno::makeAny( fCharHeight ));
        xProperties->setPropertyValue( C2U("CharHeightAsian"), uno::makeAny( fCharHeight ));
        xProperties->setPropertyValue( C2U("CharHeightComplex"), uno::makeAny( fCharHeight ));
        xProperties->setPropertyValue( C2U("TextVerticalAdjust"), uno::makeAny( drawing::TextVerticalAdjust_CENTER ));
        xProperties->setPropertyValue( C2U("TextHorizontalAdjust"), uno::makeAny( drawing::TextHorizontalAdjust_CENTER ));
        xProperties->setPropertyValue( C2U("CharFontName"), uno::makeAny( C2U( "Albany" )));

        awt::Point aAdaptedPos( aPosition );
        aAdaptedPos.Y -= (xTextShape->getSize().Height / 2);
        aAdaptedPos.X -= (xTextShape->getSize().Width / 2);
        xTextShape->setPosition( aAdaptedPos );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

} // anonymous namespace

namespace chart
{

awt::Size ChartController::impl_getDiagramSize( sal_Int32 nDiaIndex ) const
{
   awt::Size aResult;

    try
    {
        ExplicitValueProvider * pValueProvider(
        ExplicitValueProvider::getExplicitValueProvider( m_xChartView ));
        if( pValueProvider )
        {
            awt::Rectangle aRect( pValueProvider->getRectangleOfObject( ObjectIdentifier::createClassifiedIdentifier(
                OBJECTTYPE_DIAGRAM, OUString::valueOf( sal_Int32( nDiaIndex )))));
            aResult.Width = aRect.Width;
            aResult.Height = aRect.Height;
        }
        else
        {
            // fallback: page size
            aResult = ChartModelHelper::getPageSize( m_aModel->getModel() );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return aResult;
}

::std::auto_ptr< ReferenceSizeProvider > ChartController::impl_createReferenceSizeProvider() const
{
    awt::Size aPageSize( ChartModelHelper::getPageSize( m_aModel->getModel()) );

    return ::std::auto_ptr< ReferenceSizeProvider >(
        new ReferenceSizeProvider(
            aPageSize, impl_getDiagramSize(),
            Reference< chart2::XChartDocument >( m_aModel->getModel(), uno::UNO_QUERY )));
}

void ChartController::impl_adaptDataSeriesAutoResize()
{
    ::std::auto_ptr< ReferenceSizeProvider > apRefSizeProvider(
        impl_createReferenceSizeProvider());
    if( apRefSizeProvider.get())
        apRefSizeProvider->setValuesAtAllDataSeries();
}

void ChartController::executeDispatch_NewArrangement()
{
    // remove manual positions at titles, legend and the diagram, remove manual
    // size at the diagram

    try
    {
        Reference< frame::XModel > xModel( m_aModel->getModel());
        Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( xModel ));
        if( xDiagram.is())
        {
            // using assignment for broken gcc 3.3
            UndoGuard aUndoGuard = UndoGuard(
                ::rtl::OUString( String( SchResId( STR_ACTION_REARRANGE_CHART ))),
                m_xUndoManager, xModel );
            ControllerLockGuard aCtlLockGuard( xModel );

            // diagram
            Reference< beans::XPropertyState > xState( xDiagram, uno::UNO_QUERY_THROW );
            xState->setPropertyToDefault( C2U("RelativeSize"));
            xState->setPropertyToDefault( C2U("RelativePosition"));

            // 3d rotation
            xState->setPropertyToDefault( C2U("D3DTransformMatrix"));
            xState->setPropertyToDefault( C2U("D3DSceneDistance"));
            xState->setPropertyToDefault( C2U("D3DSceneFocalLength"));
            Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY_THROW );
            drawing::CameraGeometry aCameraGeo( ThreeDHelper::getDefaultCameraGeometry());
            xDiaProp->setPropertyValue( C2U("D3DCameraGeometry"), uno::makeAny( aCameraGeo ));
            //todo: different defaults for pie and donut; todo ask template

            // legend
            Reference< beans::XPropertyState > xLegendState( xDiagram->getLegend(), uno::UNO_QUERY );
            if( xLegendState.is())
                xLegendState->setPropertyToDefault( C2U("RelativePosition"));

            // titles
            for( sal_Int32 eType = TitleHelper::TITLE_BEGIN;
                 eType < TitleHelper::NORMAL_TITLE_END;
                 ++eType )
            {
                Reference< beans::XPropertyState > xTitleState(
                    TitleHelper::getTitle(
                        static_cast< TitleHelper::eTitleType >( eType ), xModel ), uno::UNO_QUERY );
                if( xTitleState.is())
                    xTitleState->setPropertyToDefault( C2U("RelativePosition"));
            }

            // regression curve equations
            ::std::vector< Reference< chart2::XRegressionCurve > > aRegressionCurves(
                RegressionCurveHelper::getAllRegressionCurvesNotMeanValueLine( xDiagram ));
            ::std::for_each( aRegressionCurves.begin(), aRegressionCurves.end(),
                      RegressionCurveHelper::resetEquationPosition );

            aUndoGuard.commitAction();
        }
    }
    catch( uno::RuntimeException & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void ChartController::executeDispatch_ScaleText()
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
    // using assignment for broken gcc 3.3
    UndoGuard aUndoGuard = UndoGuard(
        ::rtl::OUString( String( SchResId( STR_ACTION_SCALE_TEXT ))),
        m_xUndoManager, m_aModel->getModel());
    ControllerLockGuard aCtlLockGuard( m_aModel->getModel());
    ::std::auto_ptr< ReferenceSizeProvider > apRefSizeProv( impl_createReferenceSizeProvider());
    OSL_ASSERT( apRefSizeProv.get());
    if( apRefSizeProv.get())
        apRefSizeProv->toggleAutoResizeState();
    aUndoGuard.commitAction();
}

void ChartController::executeDispatch_Paste()
{
    if( m_pChartWindow )
    {
        Graphic aGraphic;
        // paste location: center of window
        Point aPos;
        aPos = m_pChartWindow->PixelToLogic( Rectangle( aPos, m_pChartWindow->GetSizePixel()).Center());

        // handle different formats
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( m_pChartWindow ));
        if( aDataHelper.GetTransferable().is())
        {
//             if( aDataHelper.HasFormat( SOT_FORMATSTR_ID_DRAWING ))
//             {
//                 SotStorageStreamRef xStm;
//                 if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_DRAWING, xStm ))
//                 {
//                     xStm->Seek( 0 );
//                     uno::Reference< io::XInputStream > xInputStream( new utl::OInputStreamWrapper( *xStm ));
//                     SdrModel * pModel = new SdrModel();
//                     DrawModelWrapper * pDrawModelWrapper( this->GetDrawModelWrapper());
//                     if( SvxDrawingLayerImport( pModel, xInputStream ))
//                         lcl_CopyShapesToChart( *pModel, m_pDrawModelWrapper->getSdrModel());
//                     delete pModel;
//                 }
//             }
//             else
            if( aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVXB ))
            {
                // graphic exchange format (graphic manager bitmap format?)
                SotStorageStreamRef xStm;
                if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_SVXB, xStm ))
                    (*xStm) >> aGraphic;
            }
            else if( aDataHelper.HasFormat( FORMAT_GDIMETAFILE ))
            {
                // meta file
                GDIMetaFile aMetafile;
                if( aDataHelper.GetGDIMetaFile( FORMAT_GDIMETAFILE, aMetafile ))
                    aGraphic = Graphic( aMetafile );
            }
            else if( aDataHelper.HasFormat( FORMAT_BITMAP ))
            {
                // bitmap (non-graphic-manager)
                Bitmap aBmp;
                if( aDataHelper.GetBitmap( FORMAT_BITMAP, aBmp ))
                    aGraphic = Graphic( aBmp );
            }
            else if( aDataHelper.HasFormat( FORMAT_STRING ))
            {
                OUString aString;
                if( aDataHelper.GetString( FORMAT_STRING, aString ) && m_pDrawModelWrapper )
                {
                    awt::Point aTextPos;
                    awt::Size aPageSize( ChartModelHelper::getPageSize( m_aModel->getModel()));
                    aTextPos.X = (aPageSize.Width / 2);
                    aTextPos.Y = (aPageSize.Height / 2);
                    lcl_InsertStringAsTextShapeIntoDrawPage(
                        m_pDrawModelWrapper->getShapeFactory(),
                        m_pDrawModelWrapper->getMainDrawPage(),
                        aString, aTextPos );
                }
            }
        }

        if( aGraphic.GetType() != GRAPHIC_NONE )
        {
            Reference< graphic::XGraphic > xGraphic( aGraphic.GetXGraphic());
            if( xGraphic.is())
                impl_PasteGraphic( xGraphic, aPos );
        }
    }
}

// note: aPosition is ignored for now. The object is always pasted centered to
// the page
void ChartController::impl_PasteGraphic(
    uno::Reference< graphic::XGraphic > & xGraphic,
    const ::Point & /* aPosition */ )
{
    // note: the XPropertySet of the model is the old API. Also the property
    // "AdditionalShapes" that is used there.
    uno::Reference< beans::XPropertySet > xModelProp( m_aModel->getModel(), uno::UNO_QUERY );
    DrawModelWrapper * pDrawModelWrapper( this->GetDrawModelWrapper());
    if( ! (xGraphic.is() && xModelProp.is()))
        return;
    uno::Reference< lang::XMultiServiceFactory > xFact( pDrawModelWrapper->getShapeFactory());
    uno::Reference< drawing::XShape > xGraphicShape(
        xFact->createInstance( C2U( "com.sun.star.drawing.GraphicObjectShape" )), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xGraphicShapeProp( xGraphicShape, uno::UNO_QUERY );
    if( xGraphicShapeProp.is() && xGraphicShape.is())
    {
        uno::Reference< drawing::XShapes > xPage( pDrawModelWrapper->getMainDrawPage(), uno::UNO_QUERY );
        if( xPage.is())
        {
            xPage->add( xGraphicShape );
            //need to change the model state manually
            {
                uno::Reference< util::XModifiable > xModifiable( m_aModel->getModel(), uno::UNO_QUERY );
                if( xModifiable.is() )
                    xModifiable->setModified( true );
            }
            //select new shape
            m_aSelection.setSelection( xGraphicShape );
            m_aSelection.applySelection( m_pDrawViewWrapper );
        }
        xGraphicShapeProp->setPropertyValue( C2U("Graphic"), uno::makeAny( xGraphic ));
        uno::Reference< beans::XPropertySet > xGraphicProp( xGraphic, uno::UNO_QUERY );

        awt::Size aGraphicSize( 1000, 1000 );
        awt::Point aShapePos( 100,100 );
        // first try size in 100th mm, then pixel size
        if( ! ( xGraphicProp->getPropertyValue( C2U("Size100thMM")) >>= aGraphicSize ) &&
            ( ( xGraphicProp->getPropertyValue( C2U("SizePixel")) >>= aGraphicSize ) && m_pChartWindow ))
        {
            ::Size aVCLSize( m_pChartWindow->PixelToLogic( Size( aGraphicSize.Width, aGraphicSize.Height )));
            aGraphicSize.Width = aVCLSize.getWidth();
            aGraphicSize.Height = aVCLSize.getHeight();
        }
        xGraphicShape->setSize( aGraphicSize );

        awt::Size aPageSize( ChartModelHelper::getPageSize( m_aModel->getModel()));
        aShapePos.X = (aPageSize.Width / 2)  - (aGraphicSize.Width / 2);
        aShapePos.Y = (aPageSize.Height / 2) - (aGraphicSize.Height / 2);
        xGraphicShape->setPosition( aShapePos );
    }
}

void ChartController::executeDispatch_Copy()
{

    Reference< datatransfer::XTransferable > xTransferable;

    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        SdrObject * pSelectedObj = 0;
        if( m_pDrawViewWrapper && m_pDrawModelWrapper )
        {
            if( m_aSelection.getSelectedCID().getLength() )
                pSelectedObj = m_pDrawModelWrapper->getNamedSdrObject( m_aSelection.getSelectedCID() );
            else
                pSelectedObj = DrawViewWrapper::getSdrObject( m_aSelection.getSelectedAdditionalShape() );

            if( pSelectedObj )
            {
                xTransferable = Reference< datatransfer::XTransferable >( new ChartTransferable(
                        & m_pDrawModelWrapper->getSdrModel(), pSelectedObj ));
            }
        }
    }
    if( xTransferable.is() )
    {
        Reference< datatransfer::clipboard::XClipboard > xClipboard( TransferableHelper::GetSystemClipboard());
        if( xClipboard.is())
            xClipboard->setContents( xTransferable, Reference< datatransfer::clipboard::XClipboardOwner >() );
    }
}

void ChartController::executeDispatch_Cut()
{
    executeDispatch_Copy();
    executeDispatch_Delete();
}

//static
bool ChartController::isObjectDeleteable( const uno::Any& rSelection )
{
    OUString aSelObjCID;
    if( (rSelection >>= aSelObjCID) && aSelObjCID.getLength() > 0 )
    {
        ObjectType aObjectType(ObjectIdentifier::getObjectType( aSelObjCID ));
        if( (OBJECTTYPE_TITLE == aObjectType) || (OBJECTTYPE_LEGEND == aObjectType)
                || (OBJECTTYPE_DATA_SERIES == aObjectType) )
            return true;
        if( (OBJECTTYPE_DATA_SERIES == aObjectType) || (OBJECTTYPE_LEGEND_ENTRY == aObjectType) )
            return true;
        if( (OBJECTTYPE_DATA_CURVE_EQUATION == aObjectType) || (OBJECTTYPE_DATA_CURVE == aObjectType) ||
            (OBJECTTYPE_DATA_AVERAGE_LINE == aObjectType) || (OBJECTTYPE_DATA_ERRORS == aObjectType))
            return true;
        if( (OBJECTTYPE_DATA_LABELS == aObjectType) || (OBJECTTYPE_DATA_LABEL == aObjectType) )
            return true;
    }

    return false;
}

void ChartController::impl_ClearSelection()
{
    if( m_aSelection.hasSelection())
    {
        m_aSelection.clearSelection();
        impl_notifySelectionChangeListeners();
    }
}

bool ChartController::executeDispatch_Delete()
{
    bool bReturn = false;

    // remove the selected object
    //
    rtl::OUString aCID( m_aSelection.getSelectedCID() );
    if( aCID.getLength() )
    {
        if( !isObjectDeleteable( uno::Any( aCID ) ) )
            return false;

        //remove chart object
        impl_ClearSelection();

        uno::Reference< chart2::XChartDocument > xChartDoc( m_aModel->getModel(), uno::UNO_QUERY );
        if( !xChartDoc.is() )
            return false;

        ObjectType aObjectType( ObjectIdentifier::getObjectType( aCID ));
        switch( aObjectType )
        {
            case OBJECTTYPE_TITLE:
            {
                // using assignment for broken gcc 3.3
                UndoGuard aUndoGuard = UndoGuard(
                    ActionDescriptionProvider::createDescription(
                        ActionDescriptionProvider::DELETE, ::rtl::OUString( String( SchResId( STR_OBJECT_TITLE )))),
                    m_xUndoManager, m_aModel->getModel() );
                TitleHelper::removeTitle(
                    ObjectIdentifier::getTitleTypeForCID( aCID ), m_aModel->getModel());
                bReturn = true;
                aUndoGuard.commitAction();
                break;
            }
            case OBJECTTYPE_LEGEND:
            {
                uno::Reference< chart2::XDiagram > xDiagram( xChartDoc->getFirstDiagram());
                if( xDiagram.is())
                {
                    uno::Reference< beans::XPropertySet > xLegendProp( xDiagram->getLegend(), uno::UNO_QUERY );
                    if( xLegendProp.is())
                    {
                        // using assignment for broken gcc 3.3
                        UndoGuard aUndoGuard = UndoGuard(
                            ActionDescriptionProvider::createDescription(
                                ActionDescriptionProvider::DELETE, ::rtl::OUString( String( SchResId( STR_OBJECT_LEGEND )))),
                            m_xUndoManager, m_aModel->getModel() );
                        xLegendProp->setPropertyValue( C2U("Show"), uno::makeAny( false ));
                        bReturn = true;
                        aUndoGuard.commitAction();
                    }
                }
                break;
            }

            case OBJECTTYPE_DATA_SERIES:
                bReturn = lcl_deleteDataSeries( aCID, m_aModel->getModel(), m_xUndoManager );
                break;

            case OBJECTTYPE_LEGEND_ENTRY:
            {
                ObjectType eParentObjectType = ObjectIdentifier::getObjectType(
                    ObjectIdentifier::getFullParentParticle( aCID ));
                if( eParentObjectType == OBJECTTYPE_DATA_SERIES )
                    bReturn = lcl_deleteDataSeries( aCID, m_aModel->getModel(), m_xUndoManager );
                else if( eParentObjectType == OBJECTTYPE_DATA_CURVE )
                    bReturn = lcl_deleteDataCurve( aCID, m_aModel->getModel(), m_xUndoManager );
                break;
            }

            case OBJECTTYPE_DATA_AVERAGE_LINE:
            {
                uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt(
                    ObjectIdentifier::getObjectPropertySet(
                        ObjectIdentifier::getFullParentParticle( aCID ), m_aModel->getModel()), uno::UNO_QUERY );
                if( xRegCurveCnt.is())
                {
                    // using assignment for broken gcc 3.3
                    UndoGuard aUndoGuard = UndoGuard(
                        ActionDescriptionProvider::createDescription(
                            ActionDescriptionProvider::DELETE, ::rtl::OUString( String( SchResId( STR_OBJECT_AVERAGE_LINE )))),
                        m_xUndoManager, m_aModel->getModel() );
                    RegressionCurveHelper::removeMeanValueLine( xRegCurveCnt );
                    bReturn = true;
                    aUndoGuard.commitAction();
                }
                break;
            }

            case OBJECTTYPE_DATA_CURVE:
                bReturn = lcl_deleteDataCurve( aCID, m_aModel->getModel(), m_xUndoManager );
                break;

            case OBJECTTYPE_DATA_CURVE_EQUATION:
            {
                uno::Reference< beans::XPropertySet > xEqProp(
                    ObjectIdentifier::getObjectPropertySet( aCID, m_aModel->getModel()));
                if( xEqProp.is())
                {
                    uno::Reference< frame::XModel > xModel( m_aModel->getModel());
                    // using assignment for broken gcc 3.3
                    UndoGuard aUndoGuard = UndoGuard(
                        ActionDescriptionProvider::createDescription(
                            ActionDescriptionProvider::DELETE, ::rtl::OUString( String( SchResId( STR_OBJECT_CURVE_EQUATION )))),
                        m_xUndoManager, xModel );
                    {
                        ControllerLockGuard aCtlLockGuard( xModel );
                        xEqProp->setPropertyValue( C2U("ShowEquation"), uno::makeAny( false ));
                        xEqProp->setPropertyValue( C2U("ShowCorrelationCoefficient"), uno::makeAny( false ));
                    }
                    bReturn = true;
                    aUndoGuard.commitAction();
                }
                break;
            }

            case OBJECTTYPE_DATA_ERRORS:
            {
                uno::Reference< beans::XPropertySet > xErrorBarProp(
                    ObjectIdentifier::getObjectPropertySet( aCID, m_aModel->getModel()));
                if( xErrorBarProp.is())
                {
                    uno::Reference< frame::XModel > xModel( m_aModel->getModel());
                    // using assignment for broken gcc 3.3
                    UndoGuard aUndoGuard = UndoGuard(
                        ActionDescriptionProvider::createDescription(
                            ActionDescriptionProvider::DELETE, ::rtl::OUString( String( SchResId( STR_OBJECT_ERROR_INDICATOR )))),
                        m_xUndoManager, xModel );
                    {
                        ControllerLockGuard aCtlLockGuard( xModel );
                        xErrorBarProp->setPropertyValue(
                            C2U("ErrorBarStyle"),
                            uno::makeAny( ::com::sun::star::chart::ErrorBarStyle::NONE ));
                    }
                    bReturn = true;
                    aUndoGuard.commitAction();
                }
                break;
            }

            case OBJECTTYPE_DATA_LABELS:
            case OBJECTTYPE_DATA_LABEL:
            {
                uno::Reference< beans::XPropertySet > xObjectProperties =
                    ObjectIdentifier::getObjectPropertySet( aCID, m_aModel->getModel() );
                if( xObjectProperties.is() )
                {
                    UndoGuard aUndoGuard = UndoGuard(
                        ActionDescriptionProvider::createDescription(
                        ActionDescriptionProvider::DELETE, ::rtl::OUString( String(
                            SchResId( aObjectType == OBJECTTYPE_DATA_LABEL ? STR_OBJECT_LABEL : STR_OBJECT_DATALABELS )))),
                                m_xUndoManager, m_aModel->getModel() );
                    chart2::DataPointLabel aLabel;
                    xObjectProperties->getPropertyValue( C2U( "Label" ) ) >>= aLabel;
                    aLabel.ShowNumber = false;
                    aLabel.ShowNumberInPercent = false;
                    aLabel.ShowCategoryName = false;
                    aLabel.ShowLegendSymbol = false;
                    if( aObjectType == OBJECTTYPE_DATA_LABELS )
                    {
                        uno::Reference< chart2::XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( aCID, m_aModel->getModel() ));
                        ::chart::DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, C2U( "Label" ), uno::makeAny(aLabel) );
                    }
                    else
                        xObjectProperties->setPropertyValue( C2U( "Label" ), uno::makeAny(aLabel) );
                    bReturn = true;
                    aUndoGuard.commitAction();
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }
    else
    {
        //remove additional shape
        uno::Reference< drawing::XShape > xShape( m_aSelection.getSelectedAdditionalShape() );
        if( xShape.is() )
        {
            impl_ClearSelection();
            {
                ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
                if( m_pDrawViewWrapper )
                    m_pDrawViewWrapper->UnmarkAll();
            }
            bReturn = DrawModelWrapper::removeShape( xShape );
        }
    }
    return bReturn;
}

void ChartController::executeDispatch_ToggleLegend()
{
    Reference< frame::XModel > xModel( m_aModel->getModel());
    UndoGuard aUndoGuard = UndoGuard(
        ::rtl::OUString( String( SchResId( STR_ACTION_TOGGLE_LEGEND ))), m_xUndoManager, xModel );
    Reference< beans::XPropertySet > xLegendProp( LegendHelper::getLegend( xModel ), uno::UNO_QUERY );
    bool bChanged = false;
    if( xLegendProp.is())
    {
        try
        {
            bool bShow = false;
            if( xLegendProp->getPropertyValue( C2U("Show")) >>= bShow )
            {
                xLegendProp->setPropertyValue( C2U("Show"), uno::makeAny( ! bShow ));
                bChanged = true;
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
    else
    {
        xLegendProp.set( LegendHelper::getLegend( xModel, m_xCC, true ), uno::UNO_QUERY );
        if( xLegendProp.is())
            bChanged = true;
    }

    if( bChanged )
        aUndoGuard.commitAction();
}

void ChartController::executeDispatch_ToggleGridHorizontal()
{
    Reference< frame::XModel > xModel( m_aModel->getModel());
    UndoGuard aUndoGuard = UndoGuard(
        ::rtl::OUString( String( SchResId( STR_ACTION_TOGGLE_GRID_HORZ ))), m_xUndoManager, xModel );
    Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( m_aModel->getModel()));
    if( xDiagram.is())
    {
        sal_Int32 nDimensionIndex = 1;
        sal_Int32 nCooSysIndex = 0;
        bool bIsMainGrid = true;

        bool bHasMainYGrid = AxisHelper::isGridShown( nDimensionIndex, nCooSysIndex, bIsMainGrid, xDiagram );

        if( bHasMainYGrid )
            AxisHelper::hideGrid( nDimensionIndex, nCooSysIndex, bIsMainGrid, xDiagram );
        else
            AxisHelper::showGrid( nDimensionIndex, nCooSysIndex, bIsMainGrid, xDiagram, m_xCC );

        aUndoGuard.commitAction();
    }
}

} //  namespace chart
