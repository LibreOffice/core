/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <algorithm>

#include <DrawViewWrapper.hxx>
#include <PositionAndSizeHelper.hxx>
#include <ChartModel.hxx>
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

void ChartController::executeDispatch_PositionAndSize(const ::cpo::uno::Sequence< ::css::beans::PropertyValue >* pArgs)
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
        const SfxItemSet aMarkedItemSet(aItemSet);
        const bool bResizable = m_aSelection.isResizeableObjectSelected();
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

            // Only what the dialog would offer: elements that can be positioned, and a
            // size only for those that can be resized.
            if (eObjectType == OBJECTTYPE_DATA_POINT || !ObjectIdentifier::isDragableObject(aCID))
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
                else if (aProp.Name == "TransformWidth" && bResizable) {
                    aItemSet.Put(SfxUInt32Item(SID_ATTR_TRANSFORM_WIDTH, static_cast<sal_uInt32>(nValue)));
                }
                else if (aProp.Name == "TransformHeight" && bResizable) {
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
            awt::Size aPageSize( getChartModel()->getPageSize() );
            awt::Rectangle aPageRect( 0,0,aPageSize.Width,aPageSize.Height );

            // A position given by arguments has not been limited to the page the way a
            // drag with the mouse is, so keep the object inside it, together with what it
            // shows around the area it is positioned by, like the axes of a diagram
            if (pArgs)
            {
                awt::Rectangle aMarkedRect;
                lcl_getPositionAndSizeFromItemSet(aMarkedItemSet, aMarkedRect, ToSize(aOldObjectRect));
                const sal_Int32 nMinX = std::max(aMarkedRect.X - aOldObjectRect.X, sal_Int32(0));
                const sal_Int32 nMinY = std::max(aMarkedRect.Y - aOldObjectRect.Y, sal_Int32(0));
                const sal_Int32 nMaxX = aPageSize.Width
                    - std::max(aOldObjectRect.X + aOldObjectRect.Width - aMarkedRect.X - aMarkedRect.Width, sal_Int32(0));
                const sal_Int32 nMaxY = aPageSize.Height
                    - std::max(aOldObjectRect.Y + aOldObjectRect.Height - aMarkedRect.Y - aMarkedRect.Height, sal_Int32(0));

                const bool bResize = bResizable && std::any_of(pArgs->begin(), pArgs->end(),
                    [](const beans::PropertyValue& rProp) {
                        return rProp.Name == "TransformWidth" || rProp.Name == "TransformHeight";
                    });
                if (bResize)
                {
                    const sal_Int32 nLeft = std::max(aNewObjectRect.X, nMinX);
                    const sal_Int32 nTop = std::max(aNewObjectRect.Y, nMinY);
                    const sal_Int32 nRight = std::min(aNewObjectRect.X + aNewObjectRect.Width, nMaxX);
                    const sal_Int32 nBottom = std::min(aNewObjectRect.Y + aNewObjectRect.Height, nMaxY);
                    aNewObjectRect = awt::Rectangle(nLeft, nTop, std::max(nRight - nLeft, sal_Int32(1)),
                                                    std::max(nBottom - nTop, sal_Int32(1)));
                }
                else
                {
                    aNewObjectRect.Width = std::max(std::min(aNewObjectRect.Width, nMaxX - nMinX), sal_Int32(1));
                    aNewObjectRect.Height = std::max(std::min(aNewObjectRect.Height, nMaxY - nMinY), sal_Int32(1));
                    aNewObjectRect.X = std::clamp(aNewObjectRect.X, nMinX, std::max(nMaxX - aNewObjectRect.Width, nMinX));
                    aNewObjectRect.Y = std::clamp(aNewObjectRect.Y, nMinY, std::max(nMaxY - aNewObjectRect.Height, nMinY));
                }
            }

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
    catch(const cpo::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
