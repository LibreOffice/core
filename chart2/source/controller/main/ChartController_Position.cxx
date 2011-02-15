/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// header for define RET_OK
#include <vcl/msgbox.hxx>
#include <svx/svxids.hrc>
#include <svx/rectenum.hxx>
#include <svl/aeitem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void lcl_getPositionAndSizeFromItemSet( const SfxItemSet& rItemSet, Rectangle& rPosAndSize, const awt::Size aOriginalSize )
{
    long nPosX(0);
    long nPosY(0);
    long nSizX(0);
    long nSizY(0);

    RECT_POINT eRP = (RECT_POINT)RP_LT;

    const SfxPoolItem* pPoolItem=NULL;
    //read position
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_POS_X,sal_True,&pPoolItem))
        nPosX=((const SfxInt32Item*)pPoolItem)->GetValue();
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_POS_Y,sal_True,&pPoolItem))
        nPosY=((const SfxInt32Item*)pPoolItem)->GetValue();
    //read size
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_WIDTH,sal_True,&pPoolItem))
        nSizX=((const SfxUInt32Item*)pPoolItem)->GetValue();
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_HEIGHT,sal_True,&pPoolItem))
        nSizY=((const SfxUInt32Item*)pPoolItem)->GetValue();
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_SIZE_POINT,sal_True,&pPoolItem))
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

    rPosAndSize = Rectangle(Point(nPosX,nPosY),Size(nSizX,nSizY));
}

void SAL_CALL ChartController::executeDispatch_PositionAndSize()
{
    const ::rtl::OUString aCID( m_aSelection.getSelectedCID() );

    if( !aCID.getLength() )
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

        //prepare and open dialog
        SdrView* pSdrView = m_pDrawViewWrapper;
        bool bResizePossible = m_aSelection.isResizeableObjectSelected();

        ::vos::OGuard aGuard( Application::GetSolarMutex());
        SvxAbstractDialogFactory * pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT( pFact, "No dialog factory" );
        pDlg = pFact->CreateSchTransformTabDialog(
            m_pChartWindow, &aItemSet, pSdrView, RID_SCH_TransformTabDLG_SVXPAGE_ANGLE, bResizePossible );
        DBG_ASSERT( pDlg, "Couldn't create SchTransformTabDialog" );


        if( pDlg->Execute() == RET_OK )
        {
            const SfxItemSet* pOutItemSet = pDlg->GetOutputItemSet();
            if(pOutItemSet)
            {
                Rectangle aObjectRect;
                aItemSet.Put(*pOutItemSet);//overwrite old values with new values (-> all items are set)
                lcl_getPositionAndSizeFromItemSet( aItemSet, aObjectRect, aSelectedSize );
                awt::Size aPageSize( ChartModelHelper::getPageSize( getModel() ) );
                Rectangle aPageRect( 0,0,aPageSize.Width,aPageSize.Height );

                bool bChanged = false;
                if ( eObjectType == OBJECTTYPE_LEGEND )
                    bChanged = DiagramHelper::switchDiagramPositioningToExcludingPositioning( getModel(), false , true );

                bool bMoved = PositionAndSizeHelper::moveObject( m_aSelection.getSelectedCID(), getModel()
                            , awt::Rectangle(aObjectRect.getX(),aObjectRect.getY(),aObjectRect.getWidth(),aObjectRect.getHeight())
                            , awt::Rectangle(aPageRect.getX(),aPageRect.getY(),aPageRect.getWidth(),aPageRect.getHeight()) );
                if( bMoved || bChanged )
                    aUndoGuard.commit();
            }
        }
        delete pDlg;
    }
    catch( uno::Exception& e)
    {
        delete pDlg;
        ASSERT_EXCEPTION( e );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
