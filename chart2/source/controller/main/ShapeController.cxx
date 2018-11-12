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

#include "ShapeController.hxx"
#include <ShapeController.h>
#include <ChartController.hxx>
#include <ChartWindow.hxx>
#include <ViewElementListProvider.hxx>
#include <dlg_ShapeFont.hxx>
#include <dlg_ShapeParagraph.hxx>
#include <chartview/DrawModelWrapper.hxx>

#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <vcl/svapp.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/editids.hrc>
#include <editeng/eeitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/spltitem.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/widwitem.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

ShapeController::ShapeController( const Reference< uno::XComponentContext >& rxContext,
        ChartController* pController )
    :FeatureCommandDispatchBase( rxContext )
    ,m_pChartController( pController )
{
}

ShapeController::~ShapeController()
{
}

// WeakComponentImplHelperBase
void ShapeController::disposing()
{
}

// XEventListener
void ShapeController::disposing( const lang::EventObject& /* Source */ )
{
}

FeatureState ShapeController::getState( const OUString& rCommand )
{
    FeatureState aReturn;
    aReturn.bEnabled = false;
    aReturn.aState <<= false;

    bool bWritable = false;
    if ( m_pChartController )
    {
        Reference< frame::XStorable > xStorable( m_pChartController->getModel(), uno::UNO_QUERY );
        if ( xStorable.is() )
        {
            bWritable = !xStorable->isReadonly();
        }
    }

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( rCommand );
    if ( aIter != m_aSupportedFeatures.end() )
    {
        sal_uInt16 nFeatureId = aIter->second.nFeatureId;
        switch ( nFeatureId )
        {
            case COMMAND_ID_FORMAT_LINE:
            case COMMAND_ID_FORMAT_AREA:
            case COMMAND_ID_TEXT_ATTRIBUTES:
            case COMMAND_ID_TRANSFORM_DIALOG:
            case COMMAND_ID_OBJECT_TITLE_DESCRIPTION:
            case COMMAND_ID_RENAME_OBJECT:
                {
                    aReturn.bEnabled = bWritable;
                    aReturn.aState <<= false;
                }
                break;
            case COMMAND_ID_BRING_TO_FRONT:
            case COMMAND_ID_FORWARD:
                {
                    aReturn.bEnabled = ( bWritable && isForwardPossible() );
                    aReturn.aState <<= false;
                }
                break;
            case COMMAND_ID_BACKWARD:
            case COMMAND_ID_SEND_TO_BACK:
                {

                    aReturn.bEnabled = ( bWritable && isBackwardPossible() );
                    aReturn.aState <<= false;
                }
                break;
            case COMMAND_ID_FONT_DIALOG:
            case COMMAND_ID_PARAGRAPH_DIALOG:
                {
                    aReturn.bEnabled = bWritable;
                    aReturn.aState <<= false;
                }
                break;
            default:
                {
                    aReturn.bEnabled = false;
                    aReturn.aState <<= false;
                }
                break;
        }
    }

    return aReturn;
}

void ShapeController::execute( const OUString& rCommand, const Sequence< beans::PropertyValue>& )
{
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( rCommand );
    if ( aIter != m_aSupportedFeatures.end() )
    {
        sal_uInt16 nFeatureId = aIter->second.nFeatureId;
        switch ( nFeatureId )
        {
            case COMMAND_ID_FORMAT_LINE:
                {
                    executeDispatch_FormatLine();
                }
                break;
            case COMMAND_ID_FORMAT_AREA:
                {
                    executeDispatch_FormatArea();
                }
                break;
            case COMMAND_ID_TEXT_ATTRIBUTES:
                {
                    executeDispatch_TextAttributes();
                }
                break;
            case COMMAND_ID_TRANSFORM_DIALOG:
                {
                    executeDispatch_TransformDialog();
                }
                break;
            case COMMAND_ID_OBJECT_TITLE_DESCRIPTION:
                {
                    executeDispatch_ObjectTitleDescription();
                }
                break;
            case COMMAND_ID_RENAME_OBJECT:
                {
                    executeDispatch_RenameObject();
                }
                break;
            case COMMAND_ID_BRING_TO_FRONT:
            case COMMAND_ID_FORWARD:
            case COMMAND_ID_BACKWARD:
            case COMMAND_ID_SEND_TO_BACK:
                {
                    executeDispatch_ChangeZOrder( nFeatureId );
                }
                break;
            case COMMAND_ID_FONT_DIALOG:
                {
                    executeDispatch_FontDialog();
                }
                break;
            case COMMAND_ID_PARAGRAPH_DIALOG:
                {
                    executeDispatch_ParagraphDialog();
                }
                break;
            default:
                {
                }
                break;
        }
    }
}

void ShapeController::describeSupportedFeatures()
{
    implDescribeSupportedFeature( ".uno:FormatLine",                COMMAND_ID_FORMAT_LINE,                 CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:FormatArea",                COMMAND_ID_FORMAT_AREA,                 CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:TextAttributes",            COMMAND_ID_TEXT_ATTRIBUTES,             CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:TransformDialog",           COMMAND_ID_TRANSFORM_DIALOG,            CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:ObjectTitleDescription",    COMMAND_ID_OBJECT_TITLE_DESCRIPTION,    CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:RenameObject",              COMMAND_ID_RENAME_OBJECT,               CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:BringToFront",              COMMAND_ID_BRING_TO_FRONT,              CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:Forward",                   COMMAND_ID_FORWARD,                     CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:Backward",                  COMMAND_ID_BACKWARD,                    CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SendToBack",                COMMAND_ID_SEND_TO_BACK,                CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:FontDialog",                COMMAND_ID_FONT_DIALOG,                 CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:ParagraphDialog",           COMMAND_ID_PARAGRAPH_DIALOG,            CommandGroup::EDIT );
}

IMPL_LINK( ShapeController, CheckNameHdl, AbstractSvxObjectNameDialog&, rDialog, bool )
{
    OUString aName;
    rDialog.GetName( aName );

    if ( !aName.isEmpty() )
    {
        DrawViewWrapper* pDrawViewWrapper = ( m_pChartController ? m_pChartController->GetDrawViewWrapper() : nullptr );
        if ( pDrawViewWrapper && pDrawViewWrapper->getNamedSdrObject( aName ) )
        {
            return false;
        }
    }
    return true;
}

void ShapeController::executeDispatch_FormatLine()
{
    SolarMutexGuard aGuard;
    if ( m_pChartController )
    {
        VclPtr<ChartWindow> pChartWindow( m_pChartController->GetChartWindow() );
        DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pChartWindow && pDrawModelWrapper && pDrawViewWrapper )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            SfxItemSet aAttr( pDrawViewWrapper->GetDefaultAttr() );
            bool bHasMarked = pDrawViewWrapper->AreObjectsMarked();
            if ( bHasMarked )
            {
                pDrawViewWrapper->MergeAttrFromMarked( aAttr, false );
            }
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            ScopedVclPtr< SfxAbstractTabDialog > pDlg(
                pFact->CreateSvxLineTabDialog(pChartWindow->GetFrameWeld(), &aAttr, &pDrawModelWrapper->getSdrModel(),
                    pSelectedObj, bHasMarked));
            if ( pDlg->Execute() == RET_OK )
            {
                const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                if ( bHasMarked )
                {
                    pDrawViewWrapper->SetAttrToMarked( *pOutAttr, false );
                }
                else
                {
                    pDrawViewWrapper->SetDefaultAttr( *pOutAttr, false );
                }
            }
        }
    }
}

void ShapeController::executeDispatch_FormatArea()
{
    SolarMutexGuard aGuard;
    if ( m_pChartController )
    {
        VclPtr<ChartWindow> pChartWindow( m_pChartController->GetChartWindow() );
        DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pChartWindow && pDrawModelWrapper && pDrawViewWrapper )
        {
            SfxItemSet aAttr( pDrawViewWrapper->GetDefaultAttr() );
            bool bHasMarked = pDrawViewWrapper->AreObjectsMarked();
            if ( bHasMarked )
            {
                pDrawViewWrapper->MergeAttrFromMarked( aAttr, false );
            }
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            ScopedVclPtr< AbstractSvxAreaTabDialog > pDlg(
                    pFact->CreateSvxAreaTabDialog(pChartWindow->GetFrameWeld(), &aAttr, &pDrawModelWrapper->getSdrModel(), true));
            if ( pDlg->Execute() == RET_OK )
            {
                const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                if ( bHasMarked )
                {
                    pDrawViewWrapper->SetAttrToMarked( *pOutAttr, false );
                }
                else
                {
                    pDrawViewWrapper->SetDefaultAttr( *pOutAttr, false );
                }
            }
        }
    }
}

void ShapeController::executeDispatch_TextAttributes()
{
    SolarMutexGuard aGuard;
    if ( m_pChartController )
    {
        VclPtr<ChartWindow> pChartWindow( m_pChartController->GetChartWindow() );
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pChartWindow && pDrawViewWrapper )
        {
            SfxItemSet aAttr( pDrawViewWrapper->GetDefaultAttr() );
            bool bHasMarked = pDrawViewWrapper->AreObjectsMarked();
            if ( bHasMarked )
            {
                pDrawViewWrapper->MergeAttrFromMarked( aAttr, false );
            }
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            ScopedVclPtr< SfxAbstractTabDialog > pDlg(
                pFact->CreateTextTabDialog(pChartWindow->GetFrameWeld(), &aAttr, pDrawViewWrapper));
            if ( pDlg->Execute() == RET_OK )
            {
                const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                if ( bHasMarked )
                {
                    pDrawViewWrapper->SetAttributes( *pOutAttr );
                }
                else
                {
                    pDrawViewWrapper->SetDefaultAttr( *pOutAttr, false );
                }
            }
        }
    }
}

void ShapeController::executeDispatch_TransformDialog()
{
    SolarMutexGuard aGuard;
    if ( m_pChartController )
    {
        VclPtr<ChartWindow> pChartWindow( m_pChartController->GetChartWindow() );
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pChartWindow && pDrawViewWrapper )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            if ( pSelectedObj && pSelectedObj->GetObjIdentifier() == OBJ_CAPTION )
            {
                // item set for caption
                SfxItemSet aAttr( pDrawViewWrapper->GetModel()->GetItemPool() );
                pDrawViewWrapper->GetAttributes( aAttr );
                // item set for position and size
                SfxItemSet aGeoAttr( pDrawViewWrapper->GetGeoAttrFromMarked() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr< SfxAbstractTabDialog > pDlg(
                    pFact->CreateCaptionDialog(pChartWindow->GetFrameWeld(), pDrawViewWrapper));
                const sal_uInt16* pRange = pDlg->GetInputRanges( *aAttr.GetPool() );
                SfxItemSet aCombAttr( *aAttr.GetPool(), pRange );
                aCombAttr.Put( aAttr );
                aCombAttr.Put( aGeoAttr );
                pDlg->SetInputSet( &aCombAttr );
                if ( pDlg->Execute() == RET_OK )
                {
                    const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                    pDrawViewWrapper->SetAttributes( *pOutAttr );
                    pDrawViewWrapper->SetGeoAttrToMarked( *pOutAttr );
                }
            }
            else
            {
                SfxItemSet aGeoAttr( pDrawViewWrapper->GetGeoAttrFromMarked() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr< SfxAbstractTabDialog > pDlg(
                    pFact->CreateSvxTransformTabDialog(pChartWindow ? pChartWindow->GetFrameWeld() : nullptr, &aGeoAttr, pDrawViewWrapper));
                if ( pDlg->Execute() == RET_OK )
                {
                    const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
                    pDrawViewWrapper->SetGeoAttrToMarked( *pOutAttr );
                }
            }
        }
    }
}

void ShapeController::executeDispatch_ObjectTitleDescription()
{
    SolarMutexGuard aGuard;
    if ( m_pChartController )
    {
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawViewWrapper && pDrawViewWrapper->GetMarkedObjectCount() == 1 )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            if ( pSelectedObj )
            {
                OUString aTitle( pSelectedObj->GetTitle() );
                OUString aDescription( pSelectedObj->GetDescription() );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                VclPtr<ChartWindow> pChartWindow( m_pChartController->GetChartWindow() );
                ScopedVclPtr< AbstractSvxObjectTitleDescDialog > pDlg(
                    pFact->CreateSvxObjectTitleDescDialog(pChartWindow ? pChartWindow->GetFrameWeld() : nullptr, aTitle, aDescription ) );
                if ( pDlg->Execute() == RET_OK )
                {
                    pDlg->GetTitle( aTitle );
                    pDlg->GetDescription( aDescription );
                    pSelectedObj->SetTitle( aTitle );
                    pSelectedObj->SetDescription( aDescription );
                }
            }
        }
    }
}

void ShapeController::executeDispatch_RenameObject()
{
    SolarMutexGuard aGuard;
    if ( m_pChartController )
    {
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawViewWrapper && pDrawViewWrapper->GetMarkedObjectCount() == 1 )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            if ( pSelectedObj )
            {
                OUString aName = pSelectedObj->GetName();
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                VclPtr<ChartWindow> pChartWindow( m_pChartController->GetChartWindow() );
                ScopedVclPtr< AbstractSvxObjectNameDialog > pDlg(
                    pFact->CreateSvxObjectNameDialog(pChartWindow ? pChartWindow->GetFrameWeld() : nullptr, aName));
                pDlg->SetCheckNameHdl( LINK( this, ShapeController, CheckNameHdl ) );
                if ( pDlg->Execute() == RET_OK )
                {
                    pDlg->GetName(aName);
                    if (pSelectedObj->GetName() == aName)
                    {
                        pSelectedObj->SetName( aName );
                    }
                }
            }
        }
    }
}

void ShapeController::executeDispatch_ChangeZOrder( sal_uInt16 nId )
{
    SolarMutexGuard aGuard;
    DrawViewWrapper* pDrawViewWrapper = ( m_pChartController ? m_pChartController->GetDrawViewWrapper() : nullptr );
    if ( pDrawViewWrapper )
    {
        switch ( nId )
        {
            case COMMAND_ID_BRING_TO_FRONT:
                {
                    if ( isForwardPossible() )
                    {
                        pDrawViewWrapper->PutMarkedToTop();
                    }
                }
                break;
            case COMMAND_ID_FORWARD:
                {
                    if ( isForwardPossible() )
                    {
                        pDrawViewWrapper->MovMarkedToTop();
                    }
                }
                break;
            case COMMAND_ID_BACKWARD:
                {
                    if ( isBackwardPossible() )
                    {
                        pDrawViewWrapper->MovMarkedToBtm();
                    }
                }
                break;
            case COMMAND_ID_SEND_TO_BACK:
                {
                    if ( isBackwardPossible() )
                    {
                        SdrObject* pFirstObj = getFirstAdditionalShape();
                        pDrawViewWrapper->PutMarkedBehindObj( pFirstObj );
                    }
                }
                break;
            default:
                {
                }
                break;
        }
    }
}

void ShapeController::executeDispatch_FontDialog()
{
    SolarMutexGuard aGuard;
    if ( m_pChartController )
    {
        VclPtr<ChartWindow> pChartWindow( m_pChartController->GetChartWindow() );
        DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pChartWindow && pDrawModelWrapper && pDrawViewWrapper )
        {
            SfxItemSet aAttr( pDrawViewWrapper->GetModel()->GetItemPool() );
            pDrawViewWrapper->GetAttributes( aAttr );
            ViewElementListProvider aViewElementListProvider( pDrawModelWrapper );
            ShapeFontDialog aDlg(pChartWindow->GetFrameWeld(), &aAttr, &aViewElementListProvider);
            if (aDlg.run() == RET_OK)
            {
                const SfxItemSet* pOutAttr = aDlg.GetOutputItemSet();
                pDrawViewWrapper->SetAttributes( *pOutAttr );
            }
        }
    }
}

void ShapeController::executeDispatch_ParagraphDialog()
{
    SolarMutexGuard aGuard;
    if ( m_pChartController )
    {
        VclPtr<ChartWindow> pChartWindow( m_pChartController->GetChartWindow() );
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pChartWindow && pDrawViewWrapper )
        {
            SfxItemPool& rPool = pDrawViewWrapper->GetModel()->GetItemPool();
            SfxItemSet aAttr( rPool );
            pDrawViewWrapper->GetAttributes( aAttr );

            SfxItemSet aNewAttr(
                rPool,
                svl::Items<
                    EE_ITEMS_START, EE_ITEMS_END,
                    SID_ATTR_PARA_PAGEBREAK, SID_ATTR_PARA_WIDOWS>{});
            aNewAttr.Put( aAttr );
            aNewAttr.Put( SvxHyphenZoneItem( false, SID_ATTR_PARA_HYPHENZONE ) );
            aNewAttr.Put( SvxFormatBreakItem( SvxBreak::NONE, SID_ATTR_PARA_PAGEBREAK ) );
            aNewAttr.Put( SvxFormatSplitItem( true, SID_ATTR_PARA_SPLIT)  );
            aNewAttr.Put( SvxWidowsItem( 0, SID_ATTR_PARA_WIDOWS) );
            aNewAttr.Put( SvxOrphansItem( 0, SID_ATTR_PARA_ORPHANS) );

            ShapeParagraphDialog aDlg(pChartWindow->GetFrameWeld(), &aNewAttr);
            if (aDlg.run() == RET_OK)
            {
                const SfxItemSet* pOutAttr = aDlg.GetOutputItemSet();
                pDrawViewWrapper->SetAttributes( *pOutAttr );
            }
        }
    }
}

SdrObject* ShapeController::getFirstAdditionalShape()
{
    SdrObject* pFirstObj = nullptr;

    try
    {
        DrawModelWrapper* pDrawModelWrapper = ( m_pChartController ? m_pChartController->GetDrawModelWrapper() : nullptr );
        if ( pDrawModelWrapper )
        {
            Reference< drawing::XShape > xFirstShape;
            Reference< drawing::XDrawPage > xDrawPage( pDrawModelWrapper->getMainDrawPage() );
            Reference< drawing::XShapes > xDrawPageShapes( xDrawPage, uno::UNO_QUERY_THROW );
            Reference< drawing::XShapes > xChartRoot( DrawModelWrapper::getChartRootShape( xDrawPage ) );
            sal_Int32 nCount = xDrawPageShapes->getCount();
            for ( sal_Int32 i = 0; i < nCount; ++i )
            {
                Reference< drawing::XShape > xShape;
                if ( xDrawPageShapes->getByIndex( i ) >>= xShape )
                {
                    if ( xShape.is() && xShape != xChartRoot )
                    {
                        xFirstShape = xShape;
                        break;
                    }
                }
            }
            if ( xFirstShape.is() )
            {
                pFirstObj = DrawViewWrapper::getSdrObject( xFirstShape );
            }
        }
    }
    catch ( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return pFirstObj;
}

SdrObject* ShapeController::getLastAdditionalShape()
{
    SdrObject* pLastObj = nullptr;

    try
    {
        DrawModelWrapper* pDrawModelWrapper = ( m_pChartController ? m_pChartController->GetDrawModelWrapper() : nullptr );
        if ( pDrawModelWrapper )
        {
            Reference< drawing::XShape > xLastShape;
            Reference< drawing::XDrawPage > xDrawPage( pDrawModelWrapper->getMainDrawPage() );
            Reference< drawing::XShapes > xDrawPageShapes( xDrawPage, uno::UNO_QUERY_THROW );
            Reference< drawing::XShapes > xChartRoot( DrawModelWrapper::getChartRootShape( xDrawPage ) );
            sal_Int32 nCount = xDrawPageShapes->getCount();
            for ( sal_Int32 i = nCount - 1; i >= 0; --i )
            {
                Reference< drawing::XShape > xShape;
                if ( xDrawPageShapes->getByIndex( i ) >>= xShape )
                {
                    if ( xShape.is() && xShape != xChartRoot )
                    {
                        xLastShape = xShape;
                        break;
                    }
                }
            }
            if ( xLastShape.is() )
            {
                pLastObj = DrawViewWrapper::getSdrObject( xLastShape );
            }
        }
    }
    catch ( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return pLastObj;
}

bool ShapeController::isBackwardPossible()
{
    if ( m_pChartController && m_pChartController->isAdditionalShapeSelected() )
    {
        SolarMutexGuard aGuard;
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawViewWrapper )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            SdrObject* pFirstObj = getFirstAdditionalShape();
            if ( pSelectedObj && pFirstObj && pSelectedObj != pFirstObj )
            {
                return true;
            }
        }
    }
    return false;
}

bool ShapeController::isForwardPossible()
{
    if ( m_pChartController && m_pChartController->isAdditionalShapeSelected() )
    {
        SolarMutexGuard aGuard;
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawViewWrapper )
        {
            SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
            SdrObject* pLastObj = getLastAdditionalShape();
            if ( pSelectedObj && pLastObj && pSelectedObj != pLastObj )
            {
                return true;
            }
        }
    }
    return false;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
