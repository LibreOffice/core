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

#include <ChartController.hxx>

#include <ChartWindow.hxx>
#include <DrawViewWrapper.hxx>
#include <PositionAndSizeHelper.hxx>
#include <ChartModelHelper.hxx>
#include "UndoGuard.hxx"
#include <ObjectNameProvider.hxx>
#include <DiagramHelper.hxx>
#include <chartview/ExplicitValueProvider.hxx>
#include <CommonConverters.hxx>
#include <svx/ActionDescriptionProvider.hxx>

#include <sal/log.hxx>
#include <svx/svxids.hrc>
#include <svx/rectenum.hxx>
#include <svl/aeitem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <vcl/svapp.hxx>
#include <memory>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

void lcl_getPositionAndSizeFromItemSet( const SfxItemSet& rItemSet, awt::Rectangle& rPosAndSize, const awt::Size& rOriginalSize )
{
    long nPosX(0);
    long nPosY(0);
    long nSizX(0);
    long nSizY(0);

    RectPoint eRP = RectPoint::LT;

    const SfxPoolItem* pPoolItem=nullptr;
    //read position
    if (rItemSet.GetItemState(SID_ATTR_TRANSFORM_POS_X,true,&pPoolItem)==SfxItemState::SET)
        nPosX= static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
    if (rItemSet.GetItemState(SID_ATTR_TRANSFORM_POS_Y,true,&pPoolItem)==SfxItemState::SET)
        nPosY=static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
    //read size
    if (rItemSet.GetItemState(SID_ATTR_TRANSFORM_WIDTH,true,&pPoolItem)==SfxItemState::SET)
        nSizX=static_cast<const SfxUInt32Item*>(pPoolItem)->GetValue();
    if (rItemSet.GetItemState(SID_ATTR_TRANSFORM_HEIGHT,true,&pPoolItem)==SfxItemState::SET)
        nSizY=static_cast<const SfxUInt32Item*>(pPoolItem)->GetValue();
    if (rItemSet.GetItemState(SID_ATTR_TRANSFORM_SIZE_POINT,true,&pPoolItem)==SfxItemState::SET)
        eRP=static_cast<RectPoint>(static_cast<const SfxAllEnumItem*>(pPoolItem)->GetValue());

    switch( eRP )
    {
        case RectPoint::LT:
            break;
        case RectPoint::MT:
            nPosX += ( rOriginalSize.Width - nSizX ) / 2;
            break;
        case RectPoint::RT:
            nPosX += rOriginalSize.Width - nSizX;
            break;
        case RectPoint::LM:
            nPosY += ( rOriginalSize.Height - nSizY ) / 2;
            break;
        case RectPoint::MM:
            nPosX += ( rOriginalSize.Width  - nSizX ) / 2;
            nPosY += ( rOriginalSize.Height - nSizY ) / 2;
            break;
        case RectPoint::RM:
            nPosX += rOriginalSize.Width - nSizX;
            nPosY += ( rOriginalSize.Height - nSizY ) / 2;
            break;
        case RectPoint::LB:
            nPosY += rOriginalSize.Height - nSizY;
            break;
        case RectPoint::MB:
            nPosX += ( rOriginalSize.Width - nSizX ) / 2;
            nPosY += rOriginalSize.Height - nSizY;
            break;
        case RectPoint::RB:
            nPosX += rOriginalSize.Width - nSizX;
            nPosY += rOriginalSize.Height - nSizY;
            break;
        default:
            break;
    }

    rPosAndSize = awt::Rectangle(nPosX,nPosY,nSizX,nSizY);
}

void ChartController::executeDispatch_PositionAndSize()
{
    const OUString aCID( m_aSelection.getSelectedCID() );

    if( aCID.isEmpty() )
        return;

    awt::Size aSelectedSize;
    ExplicitValueProvider* pProvider( ExplicitValueProvider::getExplicitValueProvider( m_xChartView ) );
    if( pProvider )
        aSelectedSize = ToSize( pProvider->getRectangleOfObject( aCID ) );

    ObjectType eObjectType = ObjectIdentifier::getObjectType( aCID );

    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::PosSize,
            ObjectNameProvider::getName( eObjectType)),
        m_xUndoManager );

    try
    {
        SfxItemSet aItemSet = m_pDrawViewWrapper->getPositionAndSizeItemSetFromMarkedObject();

        //prepare and open dialog
        SdrView* pSdrView = m_pDrawViewWrapper.get();
        bool bResizePossible = m_aSelection.isResizeableObjectSelected();

        SolarMutexGuard aGuard;
        SvxAbstractDialogFactory * pFact = SvxAbstractDialogFactory::Create();
        vcl::Window* pWin = GetChartWindow();
        ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateSchTransformTabDialog(
            pWin ? pWin->GetFrameWeld() : nullptr, &aItemSet, pSdrView, bResizePossible));

        if( pDlg->Execute() == RET_OK )
        {
            const SfxItemSet* pOutItemSet = pDlg->GetOutputItemSet();
            if(pOutItemSet)
            {
                awt::Rectangle aObjectRect;
                aItemSet.Put(*pOutItemSet);//overwrite old values with new values (-> all items are set)
                lcl_getPositionAndSizeFromItemSet( aItemSet, aObjectRect, aSelectedSize );
                awt::Size aPageSize( ChartModelHelper::getPageSize( getModel() ) );
                awt::Rectangle aPageRect( 0,0,aPageSize.Width,aPageSize.Height );

                bool bChanged = false;
                if ( eObjectType == OBJECTTYPE_LEGEND )
                {
                    ChartModel& rModel = dynamic_cast<ChartModel&>(*getModel().get());
                    bChanged = DiagramHelper::switchDiagramPositioningToExcludingPositioning(rModel, false , true);
                }

                bool bMoved = PositionAndSizeHelper::moveObject( m_aSelection.getSelectedCID(), getModel()
                            , aObjectRect, aPageRect );
                if( bMoved || bChanged )
                    aUndoGuard.commit();
            }
        }
    }
    catch(const uno::Exception& e)
    {
        SAL_WARN("chart2", "Exception caught. " << e );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
