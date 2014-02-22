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

#include <sfx2/docfile.hxx>
#ifdef REFERENCE
#undef REFERENCE
#endif
#include <svtools/ehdl.hxx>
#include <unotools/moduleoptions.hxx>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/form/XReset.hpp>
#include "fmvwimp.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <basic/sbuno.hxx>
#include <basic/sbx.hxx>
#include "fmitems.hxx"
#include "fmobj.hxx"
#include "svx/svditer.hxx"
#include <svx/svdpagv.hxx>
#include <svx/svdogrp.hxx>
#include <svx/fmview.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <svx/fmshell.hxx>
#include "fmpgeimp.hxx"
#include "svx/fmtools.hxx"
#include "fmshimp.hxx"
#include "fmservs.hxx"
#include "fmprop.hrc"
#include "fmundo.hxx"
#include <svx/dataaccessdescriptor.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/stdtext.hxx>
#include <svx/fmglob.hxx>
#include <svx/sdrpagewindow.hxx>
#include "svx/sdrpaintwindow.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::util;
using namespace ::svxform;
using namespace ::svx;



TYPEINIT1(FmFormView, E3dView);


FmFormView::FmFormView( FmFormModel* pModel, OutputDevice* pOut )
    :E3dView(pModel,pOut)
{
    Init();
}


void FmFormView::Init()
{
    pFormShell = NULL;
    pImpl = new FmXFormView(this);
    pImpl->acquire();

    
    
    SdrModel* pModel = GetModel();

    DBG_ASSERT( pModel->ISA(FmFormModel), "Falsches Model" );
    if( !pModel->ISA(FmFormModel) ) return;
    FmFormModel* pFormModel = (FmFormModel*)pModel;

    
    
    sal_Bool bInitDesignMode = pFormModel->GetOpenInDesignMode();
    if ( pFormModel->OpenInDesignModeIsDefaulted( ) )
    {   
        
        
        
        
        DBG_ASSERT( !bInitDesignMode, "FmFormView::Init: doesn't the model default to FALSE anymore?" );
            
            
        bInitDesignMode = sal_True;
    }

    SfxObjectShell* pObjShell = pFormModel->GetObjectShell();
    if ( pObjShell && pObjShell->GetMedium() )
    {
        const SfxPoolItem *pItem=0;
        if ( pObjShell->GetMedium()->GetItemSet()->GetItemState( SID_COMPONENTDATA, false, &pItem ) == SFX_ITEM_SET )
        {
            ::comphelper::NamedValueCollection aComponentData( ((SfxUnoAnyItem*)pItem)->GetValue() );
            bInitDesignMode = aComponentData.getOrDefault( "ApplyFormDesignMode", bInitDesignMode );
        }
    }

    if( pObjShell && pObjShell->IsReadOnly() )
        bInitDesignMode = sal_False;

    
    
    SetDesignMode( bInitDesignMode );
}


FmFormView::~FmFormView()
{
    if( pFormShell )
        pFormShell->SetView( NULL );

    pImpl->notifyViewDying();
    pImpl->release();
    pImpl = NULL;
}


FmFormPage* FmFormView::GetCurPage()
{
    SdrPageView* pPageView = GetSdrPageView();
    FmFormPage*  pCurPage = pPageView ? PTR_CAST( FmFormPage, pPageView->GetPage() ) : NULL;
    return pCurPage;
}


void FmFormView::MarkListHasChanged()
{
    E3dView::MarkListHasChanged();

    if ( pFormShell && IsDesignMode() )
    {
        FmFormObj* pObj = getMarkedGrid();
        if ( pImpl->m_pMarkedGrid && pImpl->m_pMarkedGrid != pObj )
        {
            pImpl->m_pMarkedGrid = NULL;
            if ( pImpl->m_xWindow.is() )
            {
                pImpl->m_xWindow->removeFocusListener(pImpl);
                pImpl->m_xWindow = NULL;
            }
            SetMoveOutside(sal_False);
            
        }

        pFormShell->GetImpl()->SetSelectionDelayed();
    }
}

namespace
{
    const SdrPageWindow* findPageWindow( const SdrPaintView* _pView, OutputDevice* _pWindow )
    {
        SdrPageView* pPageView = _pView->GetSdrPageView();
        if(pPageView)
        {
            for ( sal_uInt32 window = 0; window < pPageView->PageWindowCount(); ++window )
            {
                const SdrPageWindow* pPageWindow = pPageView->GetPageWindow( window );
                if ( !pPageWindow || &pPageWindow->GetPaintWindow().GetOutputDevice() != _pWindow )
                    continue;

                return pPageWindow;
            }
        }
        return NULL;
    }
}


void FmFormView::AddWindowToPaintView(OutputDevice* pNewWin)
{
    E3dView::AddWindowToPaintView(pNewWin);

    if ( !pNewWin )
        return;

    
    
    const SdrPageWindow* pPageWindow = findPageWindow( this, pNewWin );
    if ( pPageWindow )
        pImpl->addWindow( *pPageWindow );
}


void FmFormView::DeleteWindowFromPaintView(OutputDevice* pNewWin)
{
    const SdrPageWindow* pPageWindow = findPageWindow( this, pNewWin );
    if ( pPageWindow )
        pImpl->removeWindow( pPageWindow->GetControlContainer() );

    E3dView::DeleteWindowFromPaintView(pNewWin);
}


void FmFormView::ChangeDesignMode(sal_Bool bDesign)
{
    if (bDesign == IsDesignMode())
        return;

    FmFormModel* pModel = PTR_CAST(FmFormModel, GetModel());
    if (pModel)
    {   
        
        
        
        pModel->GetUndoEnv().Lock();
    }

    
    if ( bDesign )
        DeactivateControls( GetSdrPageView() );

    
    if ( pFormShell && pFormShell->GetImpl() )
        pFormShell->GetImpl()->viewDeactivated( *this, sal_True );
    else
        pImpl->Deactivate( sal_True );

    
    if ( !bDesign )
        ActivateControls( GetSdrPageView() );

    
    FmFormPage*  pCurPage = GetCurPage();
    if ( pCurPage )
    {
        if ( pFormShell && pFormShell->GetImpl() )
            pFormShell->GetImpl()->loadForms( pCurPage, ( bDesign ? FORMS_UNLOAD : FORMS_LOAD ) );
    }

    
    SetDesignMode( bDesign );

    
    OSL_PRECOND( pFormShell && pFormShell->GetImpl(), "FmFormView::ChangeDesignMode: is this really allowed? No shell?" );
    if ( pFormShell && pFormShell->GetImpl() )
        pFormShell->GetImpl()->viewActivated( *this );
    else
        pImpl->Activate();

    if ( pCurPage )
    {
        if ( bDesign )
        {
            if ( GetActualOutDev() && GetActualOutDev()->GetOutDevType() == OUTDEV_WINDOW )
            {
                const Window* pWindow = static_cast< const Window* >( GetActualOutDev() );
                const_cast< Window* >( pWindow )->GrabFocus();
            }

            
            if ( GetSdrPageView() )
            {
                SdrObjListIter aIter(*pCurPage);
                while( aIter.IsMore() )
                {
                    SdrObject* pObj = aIter.Next();
                    if (pObj && pObj->IsUnoObj())
                    {
                        
                        
                        pObj->ActionChanged();
                    }
                }
            }
        }
        else
        {
            
            sal_Bool bForceControlFocus = pModel ? pModel->GetAutoControlFocus() : sal_False;
            if (bForceControlFocus)
                pImpl->AutoFocus();
        }
    }

    
    if (pModel)
        pModel->GetUndoEnv().UnLock();
}


void FmFormView::GrabFirstControlFocus( sal_Bool _bForceSync )
{
    if ( !IsDesignMode() )
        pImpl->AutoFocus( _bForceSync );
}


SdrPageView* FmFormView::ShowSdrPage(SdrPage* pPage)
{
    SdrPageView* pPV = E3dView::ShowSdrPage(pPage);

    if (pPage)
    {
        if (!IsDesignMode())
        {
            
            ActivateControls(pPV);

            
            UnmarkAll();
        }
        else if ( pFormShell && pFormShell->IsDesignMode() )
        {
            FmXFormShell* pFormShellImpl = pFormShell->GetImpl();
            pFormShellImpl->UpdateForms( sal_True );

            
            pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_FMEXPLORER_CONTROL , sal_True, sal_False);

            pFormShellImpl->SetSelection(GetMarkedObjectList());
        }
    }

    
    if ( pFormShell && pFormShell->GetImpl() )
        pFormShell->GetImpl()->viewActivated( *this );
    else
        pImpl->Activate();

    return pPV;
}


void FmFormView::HideSdrPage()
{
    
    if ( !IsDesignMode() )
        DeactivateControls(GetSdrPageView());

    
    if ( pFormShell && pFormShell->GetImpl() )
        pFormShell->GetImpl()->viewDeactivated( *this, sal_True );
    else
        pImpl->Deactivate( sal_True );

    
    E3dView::HideSdrPage();
}


SdrModel* FmFormView::GetMarkedObjModel() const
{
    return E3dView::GetMarkedObjModel();
}


sal_Bool FmFormView::Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst, sal_uInt32 nOptions)
{
    return E3dView::Paste(rMod, rPos, pLst, nOptions);
}


void FmFormView::ActivateControls(SdrPageView* pPageView)
{
    if (!pPageView)
        return;

    for (sal_uInt32 i = 0L; i < pPageView->PageWindowCount(); ++i)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(i);
        pImpl->addWindow(rPageWindow);
    }
}


void FmFormView::DeactivateControls(SdrPageView* pPageView)
{
    if( !pPageView )
        return;

    for (sal_uInt32 i = 0L; i < pPageView->PageWindowCount(); ++i)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(i);
        pImpl->removeWindow(rPageWindow.GetControlContainer() );
    }
}


SdrObject* FmFormView::CreateFieldControl( const ODataAccessDescriptor& _rColumnDescriptor )
{
    return pImpl->implCreateFieldControl( _rColumnDescriptor );
}


SdrObject* FmFormView::CreateXFormsControl( const OXFormsDescriptor &_rDesc )
{
    return pImpl->implCreateXFormsControl(_rDesc);
}


SdrObject* FmFormView::CreateFieldControl(const OUString& rFieldDesc) const
{
    OUString sDataSource     = rFieldDesc.getToken(0,sal_Unicode(11));
    OUString sObjectName     = rFieldDesc.getToken(1,sal_Unicode(11));
    sal_uInt16 nObjectType   = (sal_uInt16)rFieldDesc.getToken(2,sal_Unicode(11)).toInt32();
    OUString sFieldName      = rFieldDesc.getToken(3,sal_Unicode(11));

    if (sFieldName.isEmpty() || sObjectName.isEmpty() || sDataSource.isEmpty())
        return NULL;

    ODataAccessDescriptor aColumnDescriptor;
    aColumnDescriptor.setDataSource(sDataSource);
    aColumnDescriptor[ daCommand ]          <<= sObjectName;
    aColumnDescriptor[ daCommandType ]      <<= nObjectType;
    aColumnDescriptor[ daColumnName ]       <<= sFieldName;

    return pImpl->implCreateFieldControl( aColumnDescriptor );
}


void FmFormView::InsertControlContainer(const Reference< ::com::sun::star::awt::XControlContainer > & xCC)
{
    if( !IsDesignMode() )
    {
        SdrPageView* pPageView = GetSdrPageView();
        if( pPageView )
        {
            for( sal_uInt32 i = 0L; i < pPageView->PageWindowCount(); i++ )
            {
                const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(i);

                if( rPageWindow.GetControlContainer( false ) == xCC )
                {
                    pImpl->addWindow(rPageWindow);
                    break;
                }
            }
        }
    }
}


void FmFormView::RemoveControlContainer(const Reference< ::com::sun::star::awt::XControlContainer > & xCC)
{
    if( !IsDesignMode() )
    {
        pImpl->removeWindow( xCC );
    }
}


SdrPaintWindow* FmFormView::BeginCompleteRedraw(OutputDevice* pOut)
{
    SdrPaintWindow* pPaintWindow = E3dView::BeginCompleteRedraw( pOut );
    pImpl->suspendTabOrderUpdate();
    return pPaintWindow;
}


void FmFormView::EndCompleteRedraw( SdrPaintWindow& rPaintWindow, bool bPaintFormLayer )
{
    E3dView::EndCompleteRedraw( rPaintWindow, bPaintFormLayer );
    pImpl->resumeTabOrderUpdate();
}


sal_Bool FmFormView::KeyInput(const KeyEvent& rKEvt, Window* pWin)
{
    sal_Bool bDone = sal_False;
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if  (   IsDesignMode()
        &&  rKeyCode.GetCode() == KEY_RETURN
        )
    {
        
        if  (   pWin
            &&  !rKeyCode.IsShift()
            &&  !rKeyCode.IsMod1()
            &&  !rKeyCode.IsMod2()
            )
        {
            FmFormObj* pObj = getMarkedGrid();
            if ( pObj )
            {
                Reference< awt::XWindow > xWindow( pObj->GetUnoControl( *this, *pWin ), UNO_QUERY );
                if ( xWindow.is() )
                {
                    pImpl->m_pMarkedGrid = pObj;
                    pImpl->m_xWindow = xWindow;
                    
                    pImpl->m_xWindow->addFocusListener(pImpl);
                    SetMoveOutside(sal_True);
                    
                    xWindow->setFocus();
                    bDone = sal_True;
                }
            }
        }
        
        if  (   pFormShell
            &&  pFormShell->GetImpl()
            &&  !rKeyCode.IsShift()
            &&  !rKeyCode.IsMod1()
            &&   rKeyCode.IsMod2()
            )
        {
            pFormShell->GetImpl()->handleShowPropertiesRequest();
        }

    }

    if ( !bDone )
        bDone = E3dView::KeyInput(rKEvt,pWin);
    return bDone;
}

sal_Bool FmFormView::checkUnMarkAll(const Reference< XInterface >& _xSource)
{
    Reference< ::com::sun::star::awt::XControl> xControl(pImpl->m_xWindow,UNO_QUERY);
    sal_Bool bRet = !xControl.is() || !_xSource.is() || _xSource != xControl->getModel();
    if ( bRet )
        UnmarkAll();

    return bRet;
}


sal_Bool FmFormView::MouseButtonDown( const MouseEvent& _rMEvt, Window* _pWin )
{
    sal_Bool bReturn = E3dView::MouseButtonDown( _rMEvt, _pWin );

    if ( pFormShell && pFormShell->GetImpl() )
    {
        SdrViewEvent aViewEvent;
        PickAnything( _rMEvt, SDRMOUSEBUTTONDOWN, aViewEvent );
        pFormShell->GetImpl()->handleMouseButtonDown( aViewEvent );
    }

    return bReturn;
}


FmFormObj* FmFormView::getMarkedGrid() const
{
    FmFormObj* pFormObject = NULL;
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if ( 1 == rMarkList.GetMarkCount() )
    {
        SdrMark* pMark = rMarkList.GetMark(0);
        if ( pMark )
        {
            pFormObject = FmFormObj::GetFormObject( pMark->GetMarkedSdrObj() );
            if ( pFormObject )
            {
                Reference< XServiceInfo > xServInfo( pFormObject->GetUnoControlModel(), UNO_QUERY );
                if ( !xServInfo.is() || !xServInfo->supportsService( FM_SUN_COMPONENT_GRIDCONTROL ) )
                    pFormObject = NULL;
            }
        }
    }
    return pFormObject;
}


void FmFormView::createControlLabelPair( OutputDevice* _pOutDev, sal_Int32 _nXOffsetMM, sal_Int32 _nYOffsetMM,
    const Reference< XPropertySet >& _rxField, const Reference< XNumberFormats >& _rxNumberFormats,
    sal_uInt16 _nControlObjectID, const OUString& _rFieldPostfix, sal_uInt32 _nInventor, sal_uInt16 _nLabelObjectID,
    SdrPage* _pLabelPage, SdrPage* _pControlPage, SdrModel* _pModel, SdrUnoObj*& _rpLabel, SdrUnoObj*& _rpControl )
{
    FmXFormView::createControlLabelPair(
        *_pOutDev, _nXOffsetMM, _nYOffsetMM,
        _rxField, _rxNumberFormats,
        _nControlObjectID, _rFieldPostfix, _nInventor, _nLabelObjectID,
        _pLabelPage, _pControlPage, _pModel,
        _rpLabel, _rpControl
    );
}

Reference< runtime::XFormController > FmFormView::GetFormController( const Reference< XForm >& _rxForm, const OutputDevice& _rDevice ) const
{
    return pImpl->getFormController( _rxForm, _rDevice );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
