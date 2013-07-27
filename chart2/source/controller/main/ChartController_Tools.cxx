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
#include "ShapeController.hxx"
#include "DiagramHelper.hxx"
#include "ObjectNameProvider.hxx"

#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>

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
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
// for OutlinerView
#include <editeng/outliner.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/unoapi.hxx>
#include <svx/unopage.hxx>

#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

namespace
{

bool lcl_deleteDataSeries(
    const OUString & rCID,
    const Reference< frame::XModel > & xModel,
    const Reference< document::XUndoManager > & xUndoManager )
{
    bool bResult = false;
    uno::Reference< chart2::XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( rCID, xModel ));
    uno::Reference< chart2::XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
    if( xSeries.is() && xChartDoc.is())
    {
        uno::Reference< chart2::XChartType > xChartType(
            DataSeriesHelper::getChartTypeOfSeries( xSeries, xChartDoc->getFirstDiagram()));
        if( xChartType.is())
        {
            UndoGuard aUndoGuard(
                ActionDescriptionProvider::createDescription(
                    ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_DATASERIES )),
                xUndoManager );

            Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( xModel ) );
            uno::Reference< chart2::XAxis > xAxis( DiagramHelper::getAttachedAxis( xSeries, xDiagram ) );

            DataSeriesHelper::deleteSeries( xSeries, xChartType );

            AxisHelper::hideAxisIfNoDataIsAttached( xAxis, xDiagram );

            bResult = true;
            aUndoGuard.commit();
        }
    }
    return bResult;
}

bool lcl_deleteDataCurve(
    const OUString & rCID,
    const Reference< frame::XModel > & xModel,
    const Reference< document::XUndoManager > & xUndoManager )
{
    bool bResult = false;

    uno::Reference< beans::XPropertySet > xProperties(
        ObjectIdentifier::getObjectPropertySet( rCID, xModel));

    uno::Reference< chart2::XRegressionCurve > xRegressionCurve( xProperties, uno::UNO_QUERY );

    if( xRegressionCurve.is())
    {
        uno::Reference< chart2::XRegressionCurveContainer > xRegressionCurveContainer(
            ObjectIdentifier::getObjectPropertySet(
                ObjectIdentifier::getFullParentParticle( rCID ), xModel), uno::UNO_QUERY );

        if( xRegressionCurveContainer.is())
        {
            UndoGuard aUndoGuard = UndoGuard(
                ActionDescriptionProvider::createDescription(
                    ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_CURVE )),
                xUndoManager );

            xRegressionCurveContainer->removeRegressionCurve( xRegressionCurve );

            bResult = true;
            aUndoGuard.commit();
        }
    }
    return bResult;
}

} // anonymous namespace

SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr< ReferenceSizeProvider > ChartController::impl_createReferenceSizeProvider()
{
    awt::Size aPageSize( ChartModelHelper::getPageSize( getModel() ) );

    return ::std::auto_ptr< ReferenceSizeProvider >(
        new ReferenceSizeProvider( aPageSize,
            Reference< chart2::XChartDocument >( getModel(), uno::UNO_QUERY )));
}
SAL_WNODEPRECATED_DECLARATIONS_POP

void ChartController::impl_adaptDataSeriesAutoResize()
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ReferenceSizeProvider > apRefSizeProvider(
        impl_createReferenceSizeProvider());
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if( apRefSizeProvider.get())
        apRefSizeProvider->setValuesAtAllDataSeries();
}

void ChartController::executeDispatch_NewArrangement()
{
    // remove manual positions at titles, legend and the diagram, remove manual
    // size at the diagram

    try
    {
        Reference< frame::XModel > xModel( getModel() );
        Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( xModel ));
        if( xDiagram.is())
        {
            // using assignment for broken gcc 3.3
            UndoGuard aUndoGuard = UndoGuard(
                SCH_RESSTR( STR_ACTION_REARRANGE_CHART ),
                m_xUndoManager );
            ControllerLockGuard aCtlLockGuard( xModel );

            // diagram
            Reference< beans::XPropertyState > xState( xDiagram, uno::UNO_QUERY_THROW );
            xState->setPropertyToDefault( "RelativeSize");
            xState->setPropertyToDefault( "RelativePosition");
            xState->setPropertyToDefault( "PosSizeExcludeAxes");

            // 3d rotation
            ThreeDHelper::set3DSettingsToDefault( uno::Reference< beans::XPropertySet >( xDiagram, uno::UNO_QUERY ) );

            // legend
            Reference< beans::XPropertyState > xLegendState( xDiagram->getLegend(), uno::UNO_QUERY );
            if( xLegendState.is())
            {
                xLegendState->setPropertyToDefault( "RelativePosition");
                xLegendState->setPropertyToDefault( "RelativeSize");
                xLegendState->setPropertyToDefault( "AnchorPosition");
            }

            // titles
            for( sal_Int32 eType = TitleHelper::TITLE_BEGIN;
                 eType < TitleHelper::NORMAL_TITLE_END;
                 ++eType )
            {
                Reference< beans::XPropertyState > xTitleState(
                    TitleHelper::getTitle(
                        static_cast< TitleHelper::eTitleType >( eType ), xModel ), uno::UNO_QUERY );
                if( xTitleState.is())
                    xTitleState->setPropertyToDefault( "RelativePosition");
            }

            // regression curve equations
            ::std::vector< Reference< chart2::XRegressionCurve > > aRegressionCurves(
                RegressionCurveHelper::getAllRegressionCurvesNotMeanValueLine( xDiagram ));
            ::std::for_each( aRegressionCurves.begin(), aRegressionCurves.end(),
                      RegressionCurveHelper::resetEquationPosition );

            aUndoGuard.commit();
        }
    }
    catch( const uno::RuntimeException & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void ChartController::executeDispatch_ScaleText()
{
    SolarMutexGuard aSolarGuard;
    // using assignment for broken gcc 3.3
    UndoGuard aUndoGuard = UndoGuard(
        SCH_RESSTR( STR_ACTION_SCALE_TEXT ),
        m_xUndoManager );
    ControllerLockGuard aCtlLockGuard( getModel() );
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ReferenceSizeProvider > apRefSizeProv( impl_createReferenceSizeProvider());
    SAL_WNODEPRECATED_DECLARATIONS_POP
    OSL_ASSERT( apRefSizeProv.get());
    if( apRefSizeProv.get())
        apRefSizeProv->toggleAutoResizeState();
    aUndoGuard.commit();
}

void ChartController::executeDispatch_Paste()
{
    SolarMutexGuard aGuard;
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
            if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_DRAWING ) )
            {
                SotStorageStreamRef xStm;
                if ( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_DRAWING, xStm ) )
                {
                    xStm->Seek( 0 );
                    Reference< io::XInputStream > xInputStream( new utl::OInputStreamWrapper( *xStm ) );
                    ::boost::scoped_ptr< SdrModel > spModel( new SdrModel() );
                    if ( SvxDrawingLayerImport( spModel.get(), xInputStream ) )
                    {
                        impl_PasteShapes( spModel.get() );
                    }
                }
            }
            else if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVXB ) )
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
                BitmapEx aBmpEx;
                if( aDataHelper.GetBitmapEx( FORMAT_BITMAP, aBmpEx ))
                    aGraphic = Graphic( aBmpEx );
            }
            else if( aDataHelper.HasFormat( FORMAT_STRING ))
            {
                OUString aString;
                if( aDataHelper.GetString( FORMAT_STRING, aString ) && m_pDrawModelWrapper )
                {
                    if( m_pDrawViewWrapper )
                    {
                        OutlinerView* pOutlinerView = m_pDrawViewWrapper->GetTextEditOutlinerView();
                        if( pOutlinerView )//in case of edit mode insert into edited string
                            pOutlinerView->InsertText( aString );
                        else
                        {
                            impl_PasteStringAsTextShape( aString, awt::Point( 0, 0 ) );
                        }
                    }
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
    uno::Reference< beans::XPropertySet > xModelProp( getModel(), uno::UNO_QUERY );
    DrawModelWrapper * pDrawModelWrapper( this->GetDrawModelWrapper());
    if( ! (xGraphic.is() && xModelProp.is()))
        return;
    uno::Reference< lang::XMultiServiceFactory > xFact( pDrawModelWrapper->getShapeFactory());
    uno::Reference< drawing::XShape > xGraphicShape(
        xFact->createInstance( "com.sun.star.drawing.GraphicObjectShape" ), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xGraphicShapeProp( xGraphicShape, uno::UNO_QUERY );
    if( xGraphicShapeProp.is() && xGraphicShape.is())
    {
        uno::Reference< drawing::XShapes > xPage( pDrawModelWrapper->getMainDrawPage(), uno::UNO_QUERY );
        if( xPage.is())
        {
            xPage->add( xGraphicShape );
            //need to change the model state manually
            {
                uno::Reference< util::XModifiable > xModifiable( getModel(), uno::UNO_QUERY );
                if( xModifiable.is() )
                    xModifiable->setModified( true );
            }
            //select new shape
            m_aSelection.setSelection( xGraphicShape );
            m_aSelection.applySelection( m_pDrawViewWrapper );
        }
        xGraphicShapeProp->setPropertyValue( "Graphic", uno::makeAny( xGraphic ));
        uno::Reference< beans::XPropertySet > xGraphicProp( xGraphic, uno::UNO_QUERY );

        awt::Size aGraphicSize( 1000, 1000 );
        // first try size in 100th mm, then pixel size
        if( ! ( xGraphicProp->getPropertyValue( "Size100thMM") >>= aGraphicSize ) &&
            ( ( xGraphicProp->getPropertyValue( "SizePixel") >>= aGraphicSize ) && m_pChartWindow ))
        {
            ::Size aVCLSize( m_pChartWindow->PixelToLogic( Size( aGraphicSize.Width, aGraphicSize.Height )));
            aGraphicSize.Width = aVCLSize.getWidth();
            aGraphicSize.Height = aVCLSize.getHeight();
        }
        xGraphicShape->setSize( aGraphicSize );
        xGraphicShape->setPosition( awt::Point( 0, 0 ) );
    }
}

void ChartController::impl_PasteShapes( SdrModel* pModel )
{
    DrawModelWrapper* pDrawModelWrapper( this->GetDrawModelWrapper() );
    if ( pDrawModelWrapper && m_pDrawViewWrapper )
    {
        Reference< drawing::XDrawPage > xDestPage( pDrawModelWrapper->getMainDrawPage() );
        SdrPage* pDestPage = GetSdrPageFromXDrawPage( xDestPage );
        if ( pDestPage )
        {
            Reference< drawing::XShape > xSelShape;
            m_pDrawViewWrapper->BegUndo( SVX_RESSTR( RID_SVX_3D_UNDO_EXCHANGE_PASTE ) );
            sal_uInt16 nCount = pModel->GetPageCount();
            for ( sal_uInt16 i = 0; i < nCount; ++i )
            {
                const SdrPage* pPage = pModel->GetPage( i );
                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                while ( aIter.IsMore() )
                {
                    SdrObject* pObj = aIter.Next();
                    SdrObject* pNewObj = ( pObj ? pObj->Clone() : NULL );
                    if ( pNewObj )
                    {
                        pNewObj->SetModel( &pDrawModelWrapper->getSdrModel() );
                        pNewObj->SetPage( pDestPage );

                        // set position
                        Reference< drawing::XShape > xShape( pNewObj->getUnoShape(), uno::UNO_QUERY );
                        if ( xShape.is() )
                        {
                            xShape->setPosition( awt::Point( 0, 0 ) );
                        }

                        pDestPage->InsertObject( pNewObj );
                        m_pDrawViewWrapper->AddUndo( new SdrUndoInsertObj( *pNewObj ) );
                        xSelShape = xShape;
                    }
                }
            }

            Reference< util::XModifiable > xModifiable( getModel(), uno::UNO_QUERY );
            if ( xModifiable.is() )
            {
                xModifiable->setModified( true );
            }

            // select last inserted shape
            m_aSelection.setSelection( xSelShape );
            m_aSelection.applySelection( m_pDrawViewWrapper );

            m_pDrawViewWrapper->EndUndo();

            impl_switchDiagramPositioningToExcludingPositioning();
        }
    }
}

void ChartController::impl_PasteStringAsTextShape( const OUString& rString, const awt::Point& rPosition )
{
    DrawModelWrapper* pDrawModelWrapper( this->GetDrawModelWrapper() );
    if ( pDrawModelWrapper && m_pDrawViewWrapper )
    {
        const Reference< lang::XMultiServiceFactory >& xShapeFactory( pDrawModelWrapper->getShapeFactory() );
        const Reference< drawing::XDrawPage >& xDrawPage( pDrawModelWrapper->getMainDrawPage() );
        OSL_ASSERT( xShapeFactory.is() && xDrawPage.is() );

        if ( xShapeFactory.is() && xDrawPage.is() )
        {
            try
            {
                Reference< drawing::XShape > xTextShape(
                    xShapeFactory->createInstance( "com.sun.star.drawing.TextShape" ), uno::UNO_QUERY_THROW );
                xDrawPage->add( xTextShape );

                Reference< text::XTextRange > xRange( xTextShape, uno::UNO_QUERY_THROW );
                xRange->setString( rString );

                float fCharHeight = 10.0;
                Reference< beans::XPropertySet > xProperties( xTextShape, uno::UNO_QUERY_THROW );
                xProperties->setPropertyValue( "TextAutoGrowHeight", uno::makeAny( true ) );
                xProperties->setPropertyValue( "TextAutoGrowWidth", uno::makeAny( true ) );
                xProperties->setPropertyValue( "CharHeight", uno::makeAny( fCharHeight ) );
                xProperties->setPropertyValue( "CharHeightAsian", uno::makeAny( fCharHeight ) );
                xProperties->setPropertyValue( "CharHeightComplex", uno::makeAny( fCharHeight ) );
                xProperties->setPropertyValue( "TextVerticalAdjust", uno::makeAny( drawing::TextVerticalAdjust_CENTER ) );
                xProperties->setPropertyValue( "TextHorizontalAdjust", uno::makeAny( drawing::TextHorizontalAdjust_CENTER ) );
                xProperties->setPropertyValue( "CharFontName", uno::makeAny( OUString("Albany") ) );

                xTextShape->setPosition( rPosition );

                m_aSelection.setSelection( xTextShape );
                m_aSelection.applySelection( m_pDrawViewWrapper );

                SdrObject* pObj = DrawViewWrapper::getSdrObject( xTextShape );
                if ( pObj )
                {
                    m_pDrawViewWrapper->BegUndo( SVX_RESSTR( RID_SVX_3D_UNDO_EXCHANGE_PASTE ) );
                    m_pDrawViewWrapper->AddUndo( new SdrUndoInsertObj( *pObj ) );
                    m_pDrawViewWrapper->EndUndo();

                    impl_switchDiagramPositioningToExcludingPositioning();
                }
            }
            catch ( const uno::Exception& ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
}

void ChartController::executeDispatch_Copy()
{
    if ( m_pDrawViewWrapper )
    {
        OutlinerView* pOutlinerView = m_pDrawViewWrapper->GetTextEditOutlinerView();
        if ( pOutlinerView )
        {
            pOutlinerView->Copy();
        }
        else
        {
            Reference< datatransfer::XTransferable > xTransferable;
            {
                SolarMutexGuard aSolarGuard;
                SdrObject* pSelectedObj = 0;
                if ( m_pDrawModelWrapper )
                {
                    ObjectIdentifier aSelOID( m_aSelection.getSelectedOID() );
                    if ( aSelOID.isAutoGeneratedObject() )
                    {
                        pSelectedObj = m_pDrawModelWrapper->getNamedSdrObject( aSelOID.getObjectCID() );
                    }
                    else if ( aSelOID.isAdditionalShape() )
                    {
                        pSelectedObj = DrawViewWrapper::getSdrObject( aSelOID.getAdditionalShape() );
                    }
                    if ( pSelectedObj )
                    {
                        xTransferable = Reference< datatransfer::XTransferable >( new ChartTransferable(
                                &m_pDrawModelWrapper->getSdrModel(), pSelectedObj, aSelOID.isAdditionalShape() ) );
                    }
                }
            }
            if ( xTransferable.is() )
            {
                Reference< datatransfer::clipboard::XClipboard > xClipboard( TransferableHelper::GetSystemClipboard() );
                if ( xClipboard.is() )
                {
                    xClipboard->setContents( xTransferable, Reference< datatransfer::clipboard::XClipboardOwner >() );
                }
            }
        }
    }
}

void ChartController::executeDispatch_Cut()
{
    executeDispatch_Copy();
    executeDispatch_Delete();
}

bool ChartController::isObjectDeleteable( const uno::Any& rSelection )
{
    ObjectIdentifier aSelOID( rSelection );
    if ( aSelOID.isAutoGeneratedObject() )
    {
        OUString aSelObjCID( aSelOID.getObjectCID() );
        ObjectType aObjectType(ObjectIdentifier::getObjectType( aSelObjCID ));

        switch(aObjectType)
        {
        case OBJECTTYPE_TITLE:
        case OBJECTTYPE_LEGEND:
        case OBJECTTYPE_DATA_SERIES:
        case OBJECTTYPE_LEGEND_ENTRY:
        case OBJECTTYPE_DATA_CURVE_EQUATION:
        case OBJECTTYPE_DATA_CURVE:
        case OBJECTTYPE_DATA_AVERAGE_LINE:
        case OBJECTTYPE_DATA_ERRORS_X:
        case OBJECTTYPE_DATA_ERRORS_Y:
        case OBJECTTYPE_DATA_ERRORS_Z:
        case OBJECTTYPE_DATA_LABELS:
        case OBJECTTYPE_DATA_LABEL:
        case OBJECTTYPE_AXIS:
        case OBJECTTYPE_GRID:
        case OBJECTTYPE_SUBGRID:
            return true;
        default:
            break;
        }
    }
    else if ( aSelOID.isAdditionalShape() )
    {
        return true;
    }

    return false;
}

bool ChartController::isShapeContext() const
{
    if ( m_aSelection.isAdditionalShapeSelected() ||
         ( m_pDrawViewWrapper && m_pDrawViewWrapper->AreObjectsMarked() &&
           ( m_pDrawViewWrapper->GetCurrentObjIdentifier() == OBJ_TEXT ) ) )
    {
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
    OUString aCID( m_aSelection.getSelectedCID() );
    if( !aCID.isEmpty() )
    {
        if( !isObjectDeleteable( uno::Any( aCID ) ) )
            return false;

        //remove chart object
        uno::Reference< chart2::XChartDocument > xChartDoc( getModel(), uno::UNO_QUERY );
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
                        ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_TITLE )),
                    m_xUndoManager );
                TitleHelper::removeTitle(
                    ObjectIdentifier::getTitleTypeForCID( aCID ), getModel() );
                bReturn = true;
                aUndoGuard.commit();
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
                                ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_LEGEND )),
                            m_xUndoManager );
                        xLegendProp->setPropertyValue( "Show", uno::makeAny( false ));
                        bReturn = true;
                        aUndoGuard.commit();
                    }
                }
                break;
            }

            case OBJECTTYPE_DATA_SERIES:
                bReturn = lcl_deleteDataSeries( aCID, getModel(), m_xUndoManager );
                break;

            case OBJECTTYPE_LEGEND_ENTRY:
            {
                ObjectType eParentObjectType = ObjectIdentifier::getObjectType(
                    ObjectIdentifier::getFullParentParticle( aCID ));
                if( eParentObjectType == OBJECTTYPE_DATA_SERIES )
                {
                    bReturn = lcl_deleteDataSeries( aCID, getModel(), m_xUndoManager );
                }
                else if( eParentObjectType == OBJECTTYPE_DATA_CURVE )
                {
                    sal_Int32 nEndPos = aCID.lastIndexOf(':');
                    OUString aParentCID = aCID.copy(0, nEndPos);

                    bReturn = lcl_deleteDataCurve(aParentCID, getModel(), m_xUndoManager );
                }
                else if( eParentObjectType == OBJECTTYPE_DATA_AVERAGE_LINE )
                {
                    executeDispatch_DeleteMeanValue();
                    bReturn = true;
                }
                break;
            }

            case OBJECTTYPE_DATA_AVERAGE_LINE:
            {
                uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt(
                    ObjectIdentifier::getObjectPropertySet(
                        ObjectIdentifier::getFullParentParticle( aCID ), getModel()), uno::UNO_QUERY );
                if( xRegCurveCnt.is())
                {
                    // using assignment for broken gcc 3.3
                    UndoGuard aUndoGuard = UndoGuard(
                        ActionDescriptionProvider::createDescription(
                            ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_AVERAGE_LINE )),
                        m_xUndoManager );
                    RegressionCurveHelper::removeMeanValueLine( xRegCurveCnt );
                    bReturn = true;
                    aUndoGuard.commit();
                }
            }
            break;

            case OBJECTTYPE_DATA_CURVE:
            {
                bReturn = lcl_deleteDataCurve( aCID, getModel(), m_xUndoManager );
            }
            break;

            case OBJECTTYPE_DATA_CURVE_EQUATION:
            {
                uno::Reference< beans::XPropertySet > xEqProp(
                    ObjectIdentifier::getObjectPropertySet( aCID, getModel()));

                if( xEqProp.is())
                {
                    uno::Reference< frame::XModel > xModel( getModel() );
                    // using assignment for broken gcc 3.3
                    UndoGuard aUndoGuard = UndoGuard(
                        ActionDescriptionProvider::createDescription(
                            ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_CURVE_EQUATION )),
                        m_xUndoManager );
                    {
                        ControllerLockGuard aCtlLockGuard( xModel );
                        xEqProp->setPropertyValue( "ShowEquation", uno::makeAny( false ));
                        xEqProp->setPropertyValue( "ShowCorrelationCoefficient", uno::makeAny( false ));
                    }
                    bReturn = true;
                    aUndoGuard.commit();
                }
            }
            break;

            case OBJECTTYPE_DATA_ERRORS_X:
            case OBJECTTYPE_DATA_ERRORS_Y:
            case OBJECTTYPE_DATA_ERRORS_Z:
            {
                uno::Reference< beans::XPropertySet > xErrorBarProp(
                    ObjectIdentifier::getObjectPropertySet( aCID, getModel() ));
                if( xErrorBarProp.is())
                {
                    sal_Int16 nId;

                    if ( aObjectType == OBJECTTYPE_DATA_ERRORS_X )
                        nId = STR_OBJECT_ERROR_BARS_X;
                    else if ( aObjectType == OBJECTTYPE_DATA_ERRORS_Y )
                        nId = STR_OBJECT_ERROR_BARS_Y;
                    else
                        nId = STR_OBJECT_ERROR_BARS_Z;

                    uno::Reference< frame::XModel > xModel( getModel() );
                    // using assignment for broken gcc 3.3
                    UndoGuard aUndoGuard = UndoGuard(
                        ActionDescriptionProvider::createDescription(
                            ActionDescriptionProvider::DELETE, SCH_RESSTR( nId )),
                        m_xUndoManager );
                    {
                        ControllerLockGuard aCtlLockGuard( xModel );
                        xErrorBarProp->setPropertyValue(
                            "ErrorBarStyle",
                            uno::makeAny( ::com::sun::star::chart::ErrorBarStyle::NONE ));
                    }
                    bReturn = true;
                    aUndoGuard.commit();
                }
                break;
            }

            case OBJECTTYPE_DATA_LABELS:
            case OBJECTTYPE_DATA_LABEL:
            {
                uno::Reference< beans::XPropertySet > xObjectProperties =
                    ObjectIdentifier::getObjectPropertySet( aCID, getModel() );
                if( xObjectProperties.is() )
                {
                    UndoGuard aUndoGuard = UndoGuard(
                        ActionDescriptionProvider::createDescription(
                        ActionDescriptionProvider::DELETE,
                            SCH_RESSTR( aObjectType == OBJECTTYPE_DATA_LABEL ? STR_OBJECT_LABEL : STR_OBJECT_DATALABELS )),
                                m_xUndoManager );
                    chart2::DataPointLabel aLabel;
                    xObjectProperties->getPropertyValue( "Label" ) >>= aLabel;
                    aLabel.ShowNumber = false;
                    aLabel.ShowNumberInPercent = false;
                    aLabel.ShowCategoryName = false;
                    aLabel.ShowLegendSymbol = false;
                    if( aObjectType == OBJECTTYPE_DATA_LABELS )
                    {
                        uno::Reference< chart2::XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( aCID, getModel() ));
                        ::chart::DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "Label", uno::makeAny(aLabel) );
                    }
                    else
                        xObjectProperties->setPropertyValue( "Label", uno::makeAny(aLabel) );
                    bReturn = true;
                    aUndoGuard.commit();
                }
                break;
            }
            case OBJECTTYPE_AXIS:
            {
                executeDispatch_DeleteAxis();
                bReturn = true;
                break;
            }
            case OBJECTTYPE_GRID:
            {
                executeDispatch_DeleteMajorGrid();
                bReturn = true;
                break;
            }
            case OBJECTTYPE_SUBGRID:
            {
                executeDispatch_DeleteMinorGrid();
                bReturn = true;
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
        impl_ClearSelection();
        {
            SolarMutexGuard aSolarGuard;
            if ( m_pDrawViewWrapper )
            {
                m_pDrawViewWrapper->DeleteMarked();
                bReturn = true;
            }
        }
    }
    return bReturn;
}

void ChartController::executeDispatch_ToggleLegend()
{
    Reference< frame::XModel > xModel( getModel() );
    UndoGuard aUndoGuard = UndoGuard(
        SCH_RESSTR( STR_ACTION_TOGGLE_LEGEND ), m_xUndoManager );
    Reference< beans::XPropertySet > xLegendProp( LegendHelper::getLegend( xModel ), uno::UNO_QUERY );
    bool bChanged = false;
    if( xLegendProp.is())
    {
        try
        {
            bool bShow = false;
            if( xLegendProp->getPropertyValue( "Show") >>= bShow )
            {
                xLegendProp->setPropertyValue( "Show", uno::makeAny( ! bShow ));
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
        aUndoGuard.commit();
}

void ChartController::executeDispatch_ToggleGridHorizontal()
{
    Reference< frame::XModel > xModel( getModel() );
    UndoGuard aUndoGuard = UndoGuard(
        SCH_RESSTR( STR_ACTION_TOGGLE_GRID_HORZ ), m_xUndoManager );
    Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( getModel() ));
    if( xDiagram.is())
    {
        sal_Int32 nDimensionIndex = 1;
        sal_Int32 nCooSysIndex = 0;

        bool bHasMajorYGrid = AxisHelper::isGridShown( nDimensionIndex, nCooSysIndex, true,  xDiagram );
        bool bHasMinorYGrid = AxisHelper::isGridShown( nDimensionIndex, nCooSysIndex, false, xDiagram );

        if( bHasMajorYGrid )
        {
            if ( bHasMinorYGrid )
            {
                AxisHelper::hideGrid( nDimensionIndex, nCooSysIndex, true,  xDiagram );
                AxisHelper::hideGrid( nDimensionIndex, nCooSysIndex, false, xDiagram );
            }
            else
            {
                AxisHelper::showGrid( nDimensionIndex, nCooSysIndex, false, xDiagram, m_xCC );
            }
        }
        else
        {
            AxisHelper::showGrid( nDimensionIndex, nCooSysIndex, true, xDiagram, m_xCC );
        }
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_ToggleGridVertical()
{
    Reference< frame::XModel > xModel( getModel() );
    UndoGuard aUndoGuard = UndoGuard(
        SCH_RESSTR( STR_ACTION_TOGGLE_GRID_VERTICAL ), m_xUndoManager );
    Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( getModel() ));
    if( xDiagram.is())
    {
        sal_Int32 nDimensionIndex = 0;
        sal_Int32 nCooSysIndex = 0;

        bool bHasMajorXGrid = AxisHelper::isGridShown( nDimensionIndex, nCooSysIndex, true,  xDiagram );
        bool bHasMinorXGrid = AxisHelper::isGridShown( nDimensionIndex, nCooSysIndex, false, xDiagram );
        if( bHasMajorXGrid )
        {
            if (bHasMinorXGrid)
            {
                AxisHelper::hideGrid( nDimensionIndex, nCooSysIndex, true,  xDiagram );
                AxisHelper::hideGrid( nDimensionIndex, nCooSysIndex, false, xDiagram );
            }
            else
            {
                AxisHelper::showGrid( nDimensionIndex, nCooSysIndex, false, xDiagram, m_xCC );
            }
        }
        else
        {
            AxisHelper::showGrid( nDimensionIndex, nCooSysIndex, true, xDiagram, m_xCC );
        }

        aUndoGuard.commit();
    }
}


void ChartController::impl_ShapeControllerDispatch( const util::URL& rURL, const Sequence< beans::PropertyValue >& rArgs )
{
    Reference< frame::XDispatch > xDispatch( m_aDispatchContainer.getShapeController() );
    if ( xDispatch.is() )
    {
        xDispatch->dispatch( rURL, rArgs );
    }
}

void ChartController::impl_switchDiagramPositioningToExcludingPositioning()
{
    UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription(
        ActionDescriptionProvider::POS_SIZE,
        ObjectNameProvider::getName( OBJECTTYPE_DIAGRAM)),
        m_xUndoManager );
    if( DiagramHelper::switchDiagramPositioningToExcludingPositioning( m_aModel->getModel(), true, true ) )
        aUndoGuard.commit();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
