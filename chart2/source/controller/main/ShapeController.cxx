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
#include <ChartController.hxx>
#include <ViewElementListProvider.hxx>
#include <dlg_ShapeFont.hxx>
#include <dlg_ShapeParagraph.hxx>
#include <ChartModel.hxx>
#include <chartview/DrawModelWrapper.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/frame/CommandGroup.hpp>

#include <vcl/svapp.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/editids.hrc>
#include <editeng/eeitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/spltitem.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/widwitem.hxx>
#include <comphelper/diagnose_ex.hxx>

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
void ShapeController::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
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
        rtl::Reference< ChartModel > xStorable = m_pChartController->getChartModel();
        if ( xStorable.is() )
        {
            bWritable = !xStorable->isReadonly();
        }
    }

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( rCommand );
    if ( aIter != m_aSupportedFeatures.end() )
    {
        ChartCommandID nFeatureId = aIter->second.nFeatureId;
        switch ( nFeatureId )
        {
            case ChartCommandID::ShapeFormatLine:
            case ChartCommandID::ShapeFormatArea:
            case ChartCommandID::ShapeTextAttributes:
            case ChartCommandID::ShapeTransformDialog:
            case ChartCommandID::ShapeObjectTitleDescription:
            case ChartCommandID::ShapeRenameObject:
                {
                    aReturn.bEnabled = bWritable;
                    aReturn.aState <<= false;
                }
                break;
            case ChartCommandID::ShapeBringToFront:
            case ChartCommandID::ShapeForward:
                {
                    aReturn.bEnabled = ( bWritable && isForwardPossible() );
                    aReturn.aState <<= false;
                }
                break;
            case ChartCommandID::ShapeBackward:
            case ChartCommandID::ShapeSendToBack:
                {

                    aReturn.bEnabled = ( bWritable && isBackwardPossible() );
                    aReturn.aState <<= false;
                }
                break;
            case ChartCommandID::ShapeFontDialog:
            case ChartCommandID::ShapeParagraphDialog:
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
    if ( aIter == m_aSupportedFeatures.end() )
        return;

    ChartCommandID nFeatureId = aIter->second.nFeatureId;
    switch ( nFeatureId )
    {
        case ChartCommandID::ShapeFormatLine:
            {
                executeDispatch_FormatLine();
            }
            break;
        case ChartCommandID::ShapeFormatArea:
            {
                executeDispatch_FormatArea();
            }
            break;
        case ChartCommandID::ShapeTextAttributes:
            {
                executeDispatch_TextAttributes();
            }
            break;
        case ChartCommandID::ShapeTransformDialog:
            {
                executeDispatch_TransformDialog();
            }
            break;
        case ChartCommandID::ShapeObjectTitleDescription:
            {
                executeDispatch_ObjectTitleDescription();
            }
            break;
        case ChartCommandID::ShapeRenameObject:
            {
                executeDispatch_RenameObject();
            }
            break;
        case ChartCommandID::ShapeBringToFront:
        case ChartCommandID::ShapeForward:
        case ChartCommandID::ShapeBackward:
        case ChartCommandID::ShapeSendToBack:
            {
                executeDispatch_ChangeZOrder( nFeatureId );
            }
            break;
        case ChartCommandID::ShapeFontDialog:
            {
                executeDispatch_FontDialog();
            }
            break;
        case ChartCommandID::ShapeParagraphDialog:
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

void ShapeController::describeSupportedFeatures()
{
    implDescribeSupportedFeature( ".uno:FormatLine",                ChartCommandID::ShapeFormatLine,                 CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:FormatArea",                ChartCommandID::ShapeFormatArea,                 CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:TextAttributes",            ChartCommandID::ShapeTextAttributes,             CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:TransformDialog",           ChartCommandID::ShapeTransformDialog,            CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:ObjectTitleDescription",    ChartCommandID::ShapeObjectTitleDescription,    CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:RenameObject",              ChartCommandID::ShapeRenameObject,               CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:BringToFront",              ChartCommandID::ShapeBringToFront,              CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:Forward",                   ChartCommandID::ShapeForward,                     CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:Backward",                  ChartCommandID::ShapeBackward,                    CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:SendToBack",                ChartCommandID::ShapeSendToBack,                CommandGroup::FORMAT );
    implDescribeSupportedFeature( ".uno:FontDialog",                ChartCommandID::ShapeFontDialog,                 CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:ParagraphDialog",           ChartCommandID::ShapeParagraphDialog,            CommandGroup::EDIT );
}

IMPL_LINK( ShapeController, CheckNameHdl, AbstractSvxObjectNameDialog&, rDialog, bool )
{
    OUString aName = rDialog.GetName();

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
    if ( !m_pChartController )
        return;

    weld::Window* pChartWindow(m_pChartController->GetChartFrame());
    DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
    DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
    if ( !(pChartWindow && pDrawModelWrapper && pDrawViewWrapper) )
        return;

    SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
    SfxItemSet aAttr( pDrawViewWrapper->GetDefaultAttr() );
    bool bHasMarked = pDrawViewWrapper->GetMarkedObjectList().GetMarkCount() != 0;
    if ( bHasMarked )
    {
        pDrawViewWrapper->MergeAttrFromMarked( aAttr, false );
    }
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr< SfxAbstractTabDialog > pDlg(
        pFact->CreateSvxLineTabDialog(pChartWindow, &aAttr, &pDrawModelWrapper->getSdrModel(),
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

void ShapeController::executeDispatch_FormatArea()
{
    SolarMutexGuard aGuard;
    if ( !m_pChartController )
        return;

    weld::Window* pChartWindow(m_pChartController->GetChartFrame());
    DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
    DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
    if ( !(pChartWindow && pDrawModelWrapper && pDrawViewWrapper) )
        return;

    SfxItemSet aAttr( pDrawViewWrapper->GetDefaultAttr() );
    bool bHasMarked = pDrawViewWrapper->GetMarkedObjectList().GetMarkCount() != 0;
    if ( bHasMarked )
    {
        pDrawViewWrapper->MergeAttrFromMarked( aAttr, false );
    }
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr< AbstractSvxAreaTabDialog > pDlg(
            pFact->CreateSvxAreaTabDialog(pChartWindow, &aAttr, &pDrawModelWrapper->getSdrModel(), true, false));
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

void ShapeController::executeDispatch_TextAttributes()
{
    SolarMutexGuard aGuard;
    if ( !m_pChartController )
        return;

    weld::Window* pChartWindow(m_pChartController->GetChartFrame());
    DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
    if ( !(pChartWindow && pDrawViewWrapper) )
        return;

    SfxItemSet aAttr( pDrawViewWrapper->GetDefaultAttr() );
    bool bHasMarked = pDrawViewWrapper->GetMarkedObjectList().GetMarkCount() != 0;
    if ( bHasMarked )
    {
        pDrawViewWrapper->MergeAttrFromMarked( aAttr, false );
    }
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    VclPtr< SfxAbstractTabDialog > pDlg(
        pFact->CreateTextTabDialog(pChartWindow, &aAttr, pDrawViewWrapper));
    pDlg->StartExecuteAsync(
        [pDlg, bHasMarked, pDrawViewWrapper] (sal_Int32 nResult)->void
        {
            if ( RET_OK == nResult )
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
            pDlg->disposeOnce();
        }
    );
}

void ShapeController::executeDispatch_TransformDialog()
{
    SolarMutexGuard aGuard;
    if ( !m_pChartController )
        return;

    weld::Window* pChartWindow(m_pChartController->GetChartFrame());
    DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
    if ( !(pChartWindow && pDrawViewWrapper) )
        return;

    SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
    if ( pSelectedObj && pSelectedObj->GetObjIdentifier() == SdrObjKind::Caption )
    {
        // item set for caption
        SfxItemSet aAttr(pDrawViewWrapper->GetModel().GetItemPool());
        pDrawViewWrapper->GetAttributes( aAttr );
        // item set for position and size
        SfxItemSet aGeoAttr( pDrawViewWrapper->GetGeoAttrFromMarked() );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr< SfxAbstractTabDialog > pDlg(
            pFact->CreateCaptionDialog(pChartWindow, pDrawViewWrapper));
        const WhichRangesContainer& pRange = pDlg->GetInputRanges( *aAttr.GetPool() );
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
            pFact->CreateSvxTransformTabDialog(pChartWindow, &aGeoAttr, pDrawViewWrapper));
        if ( pDlg->Execute() == RET_OK )
        {
            const SfxItemSet* pOutAttr = pDlg->GetOutputItemSet();
            pDrawViewWrapper->SetGeoAttrToMarked( *pOutAttr );
        }
    }
}

void ShapeController::executeDispatch_ObjectTitleDescription()
{
    SolarMutexGuard aGuard;
    if ( !m_pChartController )
        return;

    DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
    if ( !(pDrawViewWrapper && pDrawViewWrapper->GetMarkedObjectList().GetMarkCount() == 1) )
        return;

    SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
    if ( !pSelectedObj )
        return;

    OUString aTitle( pSelectedObj->GetTitle() );
    OUString aDescription( pSelectedObj->GetDescription() );
    bool isDecorative(pSelectedObj->IsDecorative());
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    weld::Window* pChartWindow(m_pChartController->GetChartFrame());
    VclPtr< AbstractSvxObjectTitleDescDialog > pDlg(
        pFact->CreateSvxObjectTitleDescDialog(pChartWindow, aTitle, aDescription, isDecorative));
    pDlg->StartExecuteAsync(
        [pDlg, pSelectedObj] (sal_Int32 nResult)->void
        {
            if (nResult == RET_OK)
            {
                pSelectedObj->SetTitle( pDlg->GetTitle() );
                pSelectedObj->SetDescription( pDlg->GetDescription() );
                pSelectedObj->SetDecorative(pDlg->IsDecorative());
            }
            pDlg->disposeOnce();
        }
    );
}

void ShapeController::executeDispatch_RenameObject()
{
    SolarMutexGuard aGuard;
    if ( !m_pChartController )
        return;

    DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
    if ( !(pDrawViewWrapper && pDrawViewWrapper->GetMarkedObjectList().GetMarkCount() == 1) )
        return;

    SdrObject* pSelectedObj = pDrawViewWrapper->getSelectedObject();
    if ( !pSelectedObj )
        return;

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    weld::Window* pChartWindow(m_pChartController->GetChartFrame());
    VclPtr< AbstractSvxObjectNameDialog > pDlg(
        pFact->CreateSvxObjectNameDialog(pChartWindow, pSelectedObj->GetName()));
    pDlg->SetCheckNameHdl( LINK( this, ShapeController, CheckNameHdl ) );
    pDlg->StartExecuteAsync(
        [pDlg, pSelectedObj] (sal_Int32 nResult)->void
        {
            if (nResult == RET_OK)
            {
                OUString aName = pDlg->GetName();
                if (pSelectedObj->GetName() != aName)
                {
                    pSelectedObj->SetName( aName );
                }
            }
            pDlg->disposeOnce();
        }
    );
}

void ShapeController::executeDispatch_ChangeZOrder( ChartCommandID nId )
{
    SolarMutexGuard aGuard;
    DrawViewWrapper* pDrawViewWrapper = ( m_pChartController ? m_pChartController->GetDrawViewWrapper() : nullptr );
    if ( !pDrawViewWrapper )
        return;

    switch ( nId )
    {
        case ChartCommandID::ShapeBringToFront:
            {
                if ( isForwardPossible() )
                {
                    pDrawViewWrapper->PutMarkedToTop();
                }
            }
            break;
        case ChartCommandID::ShapeForward:
            {
                if ( isForwardPossible() )
                {
                    pDrawViewWrapper->MovMarkedToTop();
                }
            }
            break;
        case ChartCommandID::ShapeBackward:
            {
                if ( isBackwardPossible() )
                {
                    pDrawViewWrapper->MovMarkedToBtm();
                }
            }
            break;
        case ChartCommandID::ShapeSendToBack:
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

void ShapeController::executeDispatch_FontDialog()
{
    SolarMutexGuard aGuard;
    if ( !m_pChartController )
        return;

    weld::Window* pChartWindow(m_pChartController->GetChartFrame());
    DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
    DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
    if ( pChartWindow && pDrawModelWrapper && pDrawViewWrapper )
    {
        SfxItemSet aAttr(pDrawViewWrapper->GetModel().GetItemPool());
        pDrawViewWrapper->GetAttributes( aAttr );
        ViewElementListProvider aViewElementListProvider( pDrawModelWrapper );
        ShapeFontDialog aDlg(pChartWindow, &aAttr, &aViewElementListProvider);
        if (aDlg.run() == RET_OK)
        {
            const SfxItemSet* pOutAttr = aDlg.GetOutputItemSet();
            pDrawViewWrapper->SetAttributes( *pOutAttr );
        }
    }
}

void ShapeController::executeDispatch_ParagraphDialog()
{
    SolarMutexGuard aGuard;
    if ( !m_pChartController )
        return;

    weld::Window* pChartWindow(m_pChartController->GetChartFrame());
    DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
    if ( !(pChartWindow && pDrawViewWrapper) )
        return;

    SfxItemPool& rPool = pDrawViewWrapper->GetModel().GetItemPool();
    SfxItemSet aAttr( rPool );
    pDrawViewWrapper->GetAttributes( aAttr );

    SfxItemSetFixed<
            EE_ITEMS_START, EE_ITEMS_END,
            SID_ATTR_PARA_PAGEBREAK, SID_ATTR_PARA_WIDOWS>  aNewAttr(rPool);
    aNewAttr.Put( aAttr );
    aNewAttr.Put( SvxHyphenZoneItem( false, SID_ATTR_PARA_HYPHENZONE ) );
    aNewAttr.Put( SvxFormatBreakItem( SvxBreak::NONE, SID_ATTR_PARA_PAGEBREAK ) );
    aNewAttr.Put( SvxFormatSplitItem( true, SID_ATTR_PARA_SPLIT)  );
    aNewAttr.Put( SvxWidowsItem( 0, SID_ATTR_PARA_WIDOWS) );
    aNewAttr.Put( SvxOrphansItem( 0, SID_ATTR_PARA_ORPHANS) );

    ShapeParagraphDialog aDlg(pChartWindow, &aNewAttr);
    if (aDlg.run() == RET_OK)
    {
        const SfxItemSet* pOutAttr = aDlg.GetOutputItemSet();
        pDrawViewWrapper->SetAttributes( *pOutAttr );
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
            rtl::Reference<SvxDrawPage> xDrawPage( pDrawModelWrapper->getMainDrawPage() );
            Reference< drawing::XShapes > xChartRoot( DrawModelWrapper::getChartRootShape( xDrawPage ) );
            sal_Int32 nCount = xDrawPage->getCount();
            for ( sal_Int32 i = 0; i < nCount; ++i )
            {
                Reference< drawing::XShape > xShape;
                if ( xDrawPage->getByIndex( i ) >>= xShape )
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
            rtl::Reference<SvxDrawPage> xDrawPage( pDrawModelWrapper->getMainDrawPage() );
            Reference< drawing::XShapes > xChartRoot( DrawModelWrapper::getChartRootShape( xDrawPage ) );
            sal_Int32 nCount = xDrawPage->getCount();
            for ( sal_Int32 i = nCount - 1; i >= 0; --i )
            {
                Reference< drawing::XShape > xShape;
                if ( xDrawPage->getByIndex( i ) >>= xShape )
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
