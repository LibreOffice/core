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
    pFormShell = nullptr;
    pImpl = new FmXFormView(this);
    pImpl->acquire();


    // Model setzen
    SdrModel* pModel = GetModel();

    DBG_ASSERT( dynamic_cast<const FmFormModel*>( pModel) !=  nullptr, "Falsches Model" );
    if( dynamic_cast<const FmFormModel*>( pModel) ==  nullptr ) return;
    FmFormModel* pFormModel = static_cast<FmFormModel*>(pModel);


    // DesignMode vom Model holen
    bool bInitDesignMode = pFormModel->GetOpenInDesignMode();
    if ( pFormModel->OpenInDesignModeIsDefaulted( ) )
    {   // this means that nobody ever explicitly set this on the model, and the model has never
        // been loaded from a stream.
        // This means this is a newly created document. This means, we want to have it in design
        // mode by default (though a newly created model returns true for GetOpenInDesignMode).
        // We _want_ to have this because it makes a lot of hacks following the original fix
        DBG_ASSERT( !bInitDesignMode, "FmFormView::Init: doesn't the model default to FALSE anymore?" );
            // if this asserts, either the on-construction default in the model has changed (then this here
            // may not be necessary anymore), or we're not dealing with a new document ....
        bInitDesignMode = true;
    }

    SfxObjectShell* pObjShell = pFormModel->GetObjectShell();
    if ( pObjShell && pObjShell->GetMedium() )
    {
        const SfxPoolItem *pItem=nullptr;
        if ( pObjShell->GetMedium()->GetItemSet()->GetItemState( SID_COMPONENTDATA, false, &pItem ) == SfxItemState::SET )
        {
            ::comphelper::NamedValueCollection aComponentData( static_cast<const SfxUnoAnyItem*>(pItem)->GetValue() );
            bInitDesignMode = aComponentData.getOrDefault( "ApplyFormDesignMode", bInitDesignMode );
        }
    }

    // dieses wird in der Shell vorgenommen
    // bDesignMode = !bInitDesignMode;  // erzwingt, dass SetDesignMode ausgefuehrt wird
    SetDesignMode( bInitDesignMode );
}


FmFormView::~FmFormView()
{
    if( pFormShell )
        pFormShell->SetView( nullptr );

    pImpl->notifyViewDying();
    pImpl->release();
    pImpl = nullptr;
}


FmFormPage* FmFormView::GetCurPage()
{
    SdrPageView* pPageView = GetSdrPageView();
    FmFormPage*  pCurPage = pPageView ? dynamic_cast<FmFormPage*>( pPageView->GetPage()  ) : nullptr;
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
            pImpl->m_pMarkedGrid = nullptr;
            if ( pImpl->m_xWindow.is() )
            {
                pImpl->m_xWindow->removeFocusListener(pImpl);
                pImpl->m_xWindow = nullptr;
            }
            SetMoveOutside(false);
            //OLMRefreshAllIAOManagers();
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
        return nullptr;
    }
}


void FmFormView::AddWindowToPaintView(OutputDevice* pNewWin, vcl::Window* pWindow)
{
    E3dView::AddWindowToPaintView(pNewWin, pWindow);

    if ( !pNewWin )
        return;

    // look up the PageViewWindow for the newly inserted window, and care for it
    // #i39269# / 2004-12-20 / frank.schoenheit@sun.com
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


void FmFormView::ChangeDesignMode(bool bDesign)
{
    if (bDesign == IsDesignMode())
        return;

    FmFormModel* pModel = dynamic_cast<FmFormModel*>( GetModel() );
    if (pModel)
    {   // fuer die Zeit des Uebergangs das Undo-Environment ausschalten, das sichert, dass man dort auch nicht-transiente
        // Properties mal eben aendern kann (sollte allerdings mit Vorsicht genossen und beim Rueckschalten des Modes
        // auch immer wieder rueckgaegig gemacht werden. Ein Beispiel ist das Setzen der maximalen Text-Laenge durch das
        // FmXEditModel an seinem Control.)
        pModel->GetUndoEnv().Lock();
    }

    // --- 1. deactivate all controls if we are switching to design mode
    if ( bDesign )
        DeactivateControls( GetSdrPageView() );

    // --- 2. simulate a deactivation (the shell will handle some things there ...?)
    if ( pFormShell && pFormShell->GetImpl() )
        pFormShell->GetImpl()->viewDeactivated( *this );
    else
        pImpl->Deactivate();

    // --- 3. activate all controls, if we're switching to alive mode
    if ( !bDesign )
        ActivateControls( GetSdrPageView() );

    // --- 4. load resp. unload the forms
    FmFormPage*  pCurPage = GetCurPage();
    if ( pCurPage )
    {
        if ( pFormShell && pFormShell->GetImpl() )
            pFormShell->GetImpl()->loadForms( pCurPage, ( bDesign ? FORMS_UNLOAD : FORMS_LOAD ) );
    }

    // --- 5. base class functionality
    SetDesignMode( bDesign );

    // --- 6. simulate a activation (the shell will handle some things there ...?)
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
                const vcl::Window* pWindow = static_cast< const vcl::Window* >( GetActualOutDev() );
                const_cast< vcl::Window* >( pWindow )->GrabFocus();
            }

            // redraw UNO objects
            if ( GetSdrPageView() )
            {
                SdrObjListIter aIter(*pCurPage);
                while( aIter.IsMore() )
                {
                    SdrObject* pObj = aIter.Next();
                    if (pObj && pObj->IsUnoObj())
                    {
                        // For redraw just use ActionChanged()
                        // pObj->BroadcastObjectChange();
                        pObj->ActionChanged();
                    }
                }
            }
        }
        else
        {
            // set the auto focus to the first control (if indicated by the model to do so)
            bool bForceControlFocus = pModel && pModel->GetAutoControlFocus();
            if (bForceControlFocus)
                pImpl->AutoFocus();
        }
    }

    // und mein Undo-Environment wieder an
    if (pModel)
        pModel->GetUndoEnv().UnLock();
}


void FmFormView::GrabFirstControlFocus( bool _bForceSync )
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
            // creating the controllers
            ActivateControls(pPV);

            // Alles deselektieren
            UnmarkAll();
        }
        else if ( pFormShell && pFormShell->IsDesignMode() )
        {
            FmXFormShell* pFormShellImpl = pFormShell->GetImpl();
            pFormShellImpl->UpdateForms( true );

            // damit der Formular-Navigator auf den Seitenwechsel reagieren kann
            pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_FMEXPLORER_CONTROL, true);

            pFormShellImpl->SetSelection(GetMarkedObjectList());
        }
    }

    // notify our shell that we have been activated
    if ( pFormShell && pFormShell->GetImpl() )
        pFormShell->GetImpl()->viewActivated( *this );
    else
        pImpl->Activate();

    return pPV;
}


void FmFormView::HideSdrPage()
{
    // --- 1. deactivate controls
    if ( !IsDesignMode() )
        DeactivateControls(GetSdrPageView());

    // --- 2. tell the shell the view is (going to be) deactivated
    if ( pFormShell && pFormShell->GetImpl() )
        pFormShell->GetImpl()->viewDeactivated( *this );
    else
        pImpl->Deactivate();

    // --- 3. base class behavior
    E3dView::HideSdrPage();
}


SdrModel* FmFormView::GetMarkedObjModel() const
{
    return E3dView::GetMarkedObjModel();
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
        return nullptr;

    ODataAccessDescriptor aColumnDescriptor;
    aColumnDescriptor.setDataSource(sDataSource);
    aColumnDescriptor[ daCommand ]          <<= sObjectName;
    aColumnDescriptor[ daCommandType ]      <<= nObjectType;
    aColumnDescriptor[ daColumnName ]       <<= sFieldName;

    return pImpl->implCreateFieldControl( aColumnDescriptor );
}


void FmFormView::InsertControlContainer(const Reference< css::awt::XControlContainer > & xCC)
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


void FmFormView::RemoveControlContainer(const Reference< css::awt::XControlContainer > & xCC)
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


bool FmFormView::KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin)
{
    bool bDone = false;
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if  (   IsDesignMode()
        &&  rKeyCode.GetCode() == KEY_RETURN
        )
    {
        // RETURN alone enters grid controls, for keyboard accessibility
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
                    // add as listener to get notified when ESC will be pressed inside the grid
                    pImpl->m_xWindow->addFocusListener(pImpl);
                    SetMoveOutside(true);
                    //OLMRefreshAllIAOManagers();
                    xWindow->setFocus();
                    bDone = true;
                }
            }
        }
        // Alt-RETURN alone shows the properties of the selection
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

bool FmFormView::checkUnMarkAll(const Reference< XInterface >& _xSource)
{
    Reference< css::awt::XControl> xControl(pImpl->m_xWindow,UNO_QUERY);
    bool bRet = !xControl.is() || !_xSource.is() || _xSource != xControl->getModel();
    if ( bRet )
        UnmarkAll();

    return bRet;
}


bool FmFormView::MouseButtonDown( const MouseEvent& _rMEvt, vcl::Window* _pWin )
{
    bool bReturn = E3dView::MouseButtonDown( _rMEvt, _pWin );

    if ( pFormShell && pFormShell->GetImpl() )
    {
        SdrViewEvent aViewEvent;
        PickAnything( _rMEvt, SdrMouseEventKind::BUTTONDOWN, aViewEvent );
        pFormShell->GetImpl()->handleMouseButtonDown( aViewEvent );
    }

    return bReturn;
}


FmFormObj* FmFormView::getMarkedGrid() const
{
    FmFormObj* pFormObject = nullptr;
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
                    pFormObject = nullptr;
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
