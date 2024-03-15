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

#include <DrawViewWrapper.hxx>
#include <PositionAndSizeHelper.hxx>
#include <ChartModel.hxx>
#include <ChartModelHelper.hxx>
#include <ChartView.hxx>
#include "UndoGuard.hxx"
#include <ObjectNameProvider.hxx>
#include <DiagramHelper.hxx>
#include <CommonConverters.hxx>
#include <svx/ActionDescriptionProvider.hxx>

#include <svx/svxids.hrc>
#include <svx/rectenum.hxx>
#include <svl/intitem.hxx>
#include <svx/svxdlg.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/svapp.hxx>
#include <memory>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

static void lcl_getPositionAndSizeFromItemSet( const SfxItemSet& rItemSet, awt::Rectangle& rPosAndSize, const awt::Size& rOriginalSize )
{
    tools::Long nPosX(0);
    tools::Long nPosY(0);
    tools::Long nSizX(0);
    tools::Long nSizY(0);

    RectPoint eRP = RectPoint::LT;

    //read position
    if (const SfxInt32Item* pPosXItem = rItemSet.GetItemIfSet(SID_ATTR_TRANSFORM_POS_X))
        nPosX = pPosXItem->GetValue();
    if (const SfxInt32Item* pPosYItem = rItemSet.GetItemIfSet(SID_ATTR_TRANSFORM_POS_Y))
        nPosY = pPosYItem->GetValue();
    //read size
    if (const SfxUInt32Item* pWidthItem = rItemSet.GetItemIfSet(SID_ATTR_TRANSFORM_WIDTH))
        nSizX = pWidthItem->GetValue();
    if (const SfxUInt32Item* pHeightItem = rItemSet.GetItemIfSet(SID_ATTR_TRANSFORM_HEIGHT))
        nSizY = pHeightItem->GetValue();
    if (const SfxUInt16Item* pSizeItem = rItemSet.GetItemIfSet(SID_ATTR_TRANSFORM_SIZE_POINT))
        eRP=static_cast<RectPoint>(pSizeItem->GetValue());

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

void ChartController::executeDispatch_PositionAndSize(const ::css::uno::Sequence< ::css::beans::PropertyValue >* pArgs)
{
    const OUString aCID( m_aSelection.getSelectedCID() );

    if( aCID.isEmpty() )
        return;

    ObjectType eObjectType = ObjectIdentifier::getObjectType( aCID );

    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::PosSize,
            ObjectNameProvider::getName( eObjectType)),
        m_xUndoManager );

    try
    {
        SfxItemSet aItemSet = m_pDrawViewWrapper->getPositionAndSizeItemSetFromMarkedObject();
        const SfxItemSet* pOutItemSet = nullptr;
        if (!pArgs)
        {
            //prepare and open dialog
            SdrView* pSdrView = m_pDrawViewWrapper.get();
            bool bResizePossible = m_aSelection.isResizeableObjectSelected();

            SolarMutexGuard aGuard;
            SvxAbstractDialogFactory * pFact = SvxAbstractDialogFactory::Create();
            ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateSchTransformTabDialog(
               GetChartFrame(), &aItemSet, pSdrView, bResizePossible));

            if( pDlg->Execute() == RET_OK )
            {
                pOutItemSet = pDlg->GetOutputItemSet();
                if (pOutItemSet)
                    aItemSet.Put(*pOutItemSet);//overwrite old values with new values (-> all items are set)
            }
        }
        else
        {
            const SfxItemPool* pPool = aItemSet.GetPool();
            if (!pPool)
                return;

            for (const auto& aProp: *pArgs)
            {
                sal_Int32 nValue = 0;
                aProp.Value >>= nValue;
                if (aProp.Name == "TransformPosX") {
                    aItemSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_POS_X, nValue));
                }
                else if (aProp.Name == "TransformPosY") {
                    aItemSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_POS_Y, nValue));
                }
                else if (aProp.Name == "TransformWidth") {
                    aItemSet.Put(SfxUInt32Item(SID_ATTR_TRANSFORM_WIDTH, static_cast<sal_uInt32>(nValue)));
                }
                else if (aProp.Name == "TransformHeight") {
                    aItemSet.Put(SfxUInt32Item(SID_ATTR_TRANSFORM_HEIGHT, static_cast<sal_uInt32>(nValue)));
                }
            }
        }

        if(pOutItemSet || pArgs)
        {
            awt::Rectangle aOldObjectRect;
            if( m_xChartView )
                aOldObjectRect = m_xChartView->getRectangleOfObject(aCID);

            awt::Rectangle aNewObjectRect;
            lcl_getPositionAndSizeFromItemSet( aItemSet, aNewObjectRect, ToSize(aOldObjectRect) );
            awt::Size aPageSize( ChartModelHelper::getPageSize( getChartModel() ) );
            awt::Rectangle aPageRect( 0,0,aPageSize.Width,aPageSize.Height );

            bool bChanged = false;
            if ( eObjectType == OBJECTTYPE_LEGEND )
            {
                bChanged = DiagramHelper::switchDiagramPositioningToExcludingPositioning(*getChartModel(), false , true);
            }

            bool bMoved = PositionAndSizeHelper::moveObject( m_aSelection.getSelectedCID(), getChartModel()
                        , aNewObjectRect, aOldObjectRect, aPageRect );
            if( bMoved || bChanged )
                aUndoGuard.commit();
        }
    }
    catch(const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
