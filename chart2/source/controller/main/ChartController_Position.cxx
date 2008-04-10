/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChartController_Position.cxx,v $
 * $Revision: 1.11 $
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
#include <svx/ActionDescriptionProvider.hxx>

// header for define RET_OK
#include <vcl/msgbox.hxx>

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svx/svxdlg.hxx>
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
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

void lcl_getPositionAndSizeFromItemSet( const SfxItemSet& rItemSet, Rectangle& rPosAndSize )
{
    long nPosX(0);
    long nPosY(0);
    long nSizX(0);
    long nSizY(0);

    const SfxPoolItem* pPoolItem=NULL;
    //read position
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_POS_X,TRUE,&pPoolItem))
        nPosX=((const SfxInt32Item*)pPoolItem)->GetValue();
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_POS_Y,TRUE,&pPoolItem))
        nPosY=((const SfxInt32Item*)pPoolItem)->GetValue();
    //read size
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_WIDTH,TRUE,&pPoolItem))
        nSizX=((const SfxUInt32Item*)pPoolItem)->GetValue();
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_HEIGHT,TRUE,&pPoolItem))
        nSizY=((const SfxUInt32Item*)pPoolItem)->GetValue();

    rPosAndSize = Rectangle(Point(nPosX,nPosY),Size(nSizX,nSizY));
}

void SAL_CALL ChartController::executeDispatch_PositionAndSize()
{
    const ::rtl::OUString aCID( m_aSelection.getSelectedCID() );

    if( !aCID.getLength() )
        return;

    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::POS_SIZE,
            ObjectNameProvider::getName( ObjectIdentifier::getObjectType( aCID ))),
        m_xUndoManager, m_aModel->getModel() );

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
                lcl_getPositionAndSizeFromItemSet( aItemSet, aObjectRect );
                awt::Size aPageSize( ChartModelHelper::getPageSize( m_aModel->getModel() ) );
                Rectangle aPageRect( 0,0,aPageSize.Width,aPageSize.Height );

                bool bChanged = PositionAndSizeHelper::moveObject( m_aSelection.getSelectedCID()
                            , m_aModel->getModel()
                            , awt::Rectangle(aObjectRect.getX(),aObjectRect.getY(),aObjectRect.getWidth(),aObjectRect.getHeight())
                            , awt::Rectangle(aPageRect.getX(),aPageRect.getY(),aPageRect.getWidth(),aPageRect.getHeight())
                            , m_xChartView );
                if( bChanged )
                    aUndoGuard.commitAction();
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
