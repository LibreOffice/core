/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "ChartController.hxx"

#include "macros.hxx"
#include "ChartWindow.hxx"
#include "DrawViewWrapper.hxx"
#include "PositionAndSizeHelper.hxx"
#include "ChartModelHelper.hxx"
#include "UndoGuard.hxx"
#include "Strings.hrc"
#include "ObjectNameProvider.hxx"
#include "DiagramHelper.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "CommonConverters.hxx"
#include <svx/ActionDescriptionProvider.hxx>


#include <vcl/msgbox.hxx>
#include <svx/svxids.hrc>
#include <svx/rectenum.hxx>
#include <svl/aeitem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

void lcl_getPositionAndSizeFromItemSet( const SfxItemSet& rItemSet, awt::Rectangle& rPosAndSize, const awt::Size aOriginalSize )
{
    long nPosX(0);
    long nPosY(0);
    long nSizX(0);
    long nSizY(0);

    RECT_POINT eRP = (RECT_POINT)RP_LT;

    const SfxPoolItem* pPoolItem=NULL;
    
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_POS_X,true,&pPoolItem))
        nPosX=((const SfxInt32Item*)pPoolItem)->GetValue();
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_POS_Y,true,&pPoolItem))
        nPosY=((const SfxInt32Item*)pPoolItem)->GetValue();
    
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_WIDTH,true,&pPoolItem))
        nSizX=((const SfxUInt32Item*)pPoolItem)->GetValue();
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_HEIGHT,true,&pPoolItem))
        nSizY=((const SfxUInt32Item*)pPoolItem)->GetValue();
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_SIZE_POINT,true,&pPoolItem))
        eRP=(RECT_POINT)((const SfxAllEnumItem*)pPoolItem)->GetValue();

    switch( eRP )
    {
        case RP_LT:
            break;
        case RP_MT:
            nPosX += ( aOriginalSize.Width - nSizX ) / 2;
            break;
        case RP_RT:
            nPosX += aOriginalSize.Width - nSizX;
            break;
        case RP_LM:
            nPosY += ( aOriginalSize.Height - nSizY ) / 2;
            break;
        case RP_MM:
            nPosX += ( aOriginalSize.Width  - nSizX ) / 2;
            nPosY += ( aOriginalSize.Height - nSizY ) / 2;
            break;
        case RP_RM:
            nPosX += aOriginalSize.Width - nSizX;
            nPosY += ( aOriginalSize.Height - nSizY ) / 2;
            break;
        case RP_LB:
            nPosY += aOriginalSize.Height - nSizY;
            break;
        case RP_MB:
            nPosX += ( aOriginalSize.Width - nSizX ) / 2;
            nPosY += aOriginalSize.Height - nSizY;
            break;
        case RP_RB:
            nPosX += aOriginalSize.Width - nSizX;
            nPosY += aOriginalSize.Height - nSizY;
            break;
        default:
            break;
    }

    rPosAndSize = awt::Rectangle(nPosX,nPosY,nSizX,nSizY);
}

void SAL_CALL ChartController::executeDispatch_PositionAndSize()
{
    const OUString aCID( m_aSelection.getSelectedCID() );

    if( aCID.isEmpty() )
        return;

    awt::Size aSelectedSize;
    ExplicitValueProvider* pProvider( ExplicitValueProvider::getExplicitValueProvider( m_xChartView ) );
    if( pProvider )
        aSelectedSize = ToSize( ( pProvider->getRectangleOfObject( aCID ) ) );

    ObjectType eObjectType = ObjectIdentifier::getObjectType( aCID );

    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::POS_SIZE,
            ObjectNameProvider::getName( eObjectType)),
        m_xUndoManager );

    SfxAbstractTabDialog * pDlg = NULL;
    try
    {
        SfxItemSet aItemSet = m_pDrawViewWrapper->getPositionAndSizeItemSetFromMarkedObject();

        
        SdrView* pSdrView = m_pDrawViewWrapper;
        bool bResizePossible = m_aSelection.isResizeableObjectSelected();

        SolarMutexGuard aGuard;
        SvxAbstractDialogFactory * pFact = SvxAbstractDialogFactory::Create();
        OSL_ENSURE( pFact, "No dialog factory" );
        pDlg = pFact->CreateSchTransformTabDialog(
            m_pChartWindow, &aItemSet, pSdrView, RID_SCH_TransformTabDLG_SVXPAGE_ANGLE, bResizePossible );
        OSL_ENSURE( pDlg, "Couldn't create SchTransformTabDialog" );

        if( pDlg->Execute() == RET_OK )
        {
            const SfxItemSet* pOutItemSet = pDlg->GetOutputItemSet();
            if(pOutItemSet)
            {
                awt::Rectangle aObjectRect;
                aItemSet.Put(*pOutItemSet);
                lcl_getPositionAndSizeFromItemSet( aItemSet, aObjectRect, aSelectedSize );
                awt::Size aPageSize( ChartModelHelper::getPageSize( getModel() ) );
                awt::Rectangle aPageRect( 0,0,aPageSize.Width,aPageSize.Height );

                bool bChanged = false;
                if ( eObjectType == OBJECTTYPE_LEGEND )
                {
                    ChartModel* pModel = dynamic_cast<ChartModel*>(getModel().get());
                    bChanged = DiagramHelper::switchDiagramPositioningToExcludingPositioning( *pModel, false , true );
                }

                bool bMoved = PositionAndSizeHelper::moveObject( m_aSelection.getSelectedCID(), getModel()
                            , aObjectRect, aPageRect );
                if( bMoved || bChanged )
                    aUndoGuard.commit();
            }
        }
        delete pDlg;
    }
    catch(const uno::Exception& e)
    {
        delete pDlg;
        ASSERT_EXCEPTION( e );
    }
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
