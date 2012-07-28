/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "fmvwimp.hxx"
#include <svx/fmshell.hxx>
#include "svx/fmtools.hxx"
#include "fmservs.hxx"
#include "fmprop.hrc"
#include "fmpgeimp.hxx"
#include "fmitems.hxx"
#include "fmundo.hxx"
#include <vcl/waitobj.hxx>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <sfx2/viewfrm.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <svl/whiter.hxx>
#include <sfx2/app.hxx>
#include <svl/intitem.hxx>
#include <svl/visitem.hxx>
#include <unotools/moduleoptions.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svx/svdobj.hxx>
#include <svx/fmpage.hxx>
#include "svx/svditer.hxx"
#include "fmobj.hxx"

#include <svx/svxids.hrc>

#include "svx/fmresids.hrc"
#include "fmexch.hxx"
#include <svx/fmglob.hxx>
#include <svl/eitem.hxx>
#include <tools/shl.hxx>
#include <tools/diagnose_ex.h>
#include <svx/svdpage.hxx>
#include <svx/fmmodel.hxx>
#include <svx/dialmgr.hxx>
#include "fmshimp.hxx"
#include <svx/svdpagv.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/viewsh.hxx>
#include "fmexpl.hxx"
#include "formcontrolling.hxx"
#include <svl/numuno.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/types.hxx>
#include <comphelper/processfactory.hxx>
#include "fmdocumentclassification.hxx"
#include "formtoolbars.hxx"

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include "svx/sdrobjectfilter.hxx"

#define FmFormShell
#include "svxslots.hxx"

#include "tbxform.hxx"
#include <comphelper/property.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <boost/scoped_ptr.hpp>

// wird fuer Invalidate verwendet -> mitpflegen
// aufsteigend sortieren !!!!!!
sal_uInt16 ControllerSlotMap[] =    // slots des Controllers
{
    SID_FM_CONFIG,
    SID_FM_PUSHBUTTON,
    SID_FM_RADIOBUTTON,
    SID_FM_CHECKBOX,
    SID_FM_FIXEDTEXT,
    SID_FM_GROUPBOX,
    SID_FM_EDIT,
    SID_FM_LISTBOX,
    SID_FM_COMBOBOX,
    SID_FM_DBGRID,
    SID_FM_IMAGEBUTTON,
    SID_FM_FILECONTROL,
    SID_FM_NAVIGATIONBAR,
    SID_FM_CTL_PROPERTIES,
    SID_FM_PROPERTIES,
    SID_FM_TAB_DIALOG,
    SID_FM_ADD_FIELD,
    SID_FM_DESIGN_MODE,
    SID_FM_SHOW_FMEXPLORER,
    SID_FM_SHOW_PROPERTIES,
    SID_FM_FMEXPLORER_CONTROL,
    SID_FM_DATEFIELD,
    SID_FM_TIMEFIELD,
    SID_FM_NUMERICFIELD,
    SID_FM_CURRENCYFIELD,
    SID_FM_PATTERNFIELD,
    SID_FM_OPEN_READONLY,
    SID_FM_IMAGECONTROL,
    SID_FM_USE_WIZARDS,
    SID_FM_FORMATTEDFIELD,
    SID_FM_FILTER_NAVIGATOR,
    SID_FM_AUTOCONTROLFOCUS,
    SID_FM_SCROLLBAR,
    SID_FM_SPINBUTTON,
    SID_FM_SHOW_DATANAVIGATOR,
    SID_FM_DATANAVIGATOR_CONTROL,

    0
};

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::form::runtime;
using namespace ::com::sun::star::frame;
using namespace ::svxform;

//========================================================================
// class FmDesignModeChangedHint
//========================================================================
TYPEINIT1( FmDesignModeChangedHint, SfxHint );

//------------------------------------------------------------------------
FmDesignModeChangedHint::FmDesignModeChangedHint( sal_Bool bDesMode )
    :m_bDesignMode( bDesMode )
{
}

//------------------------------------------------------------------------
FmDesignModeChangedHint::~FmDesignModeChangedHint()
{
}

//========================================================================
const sal_uInt32 FM_UI_FEATURE_SHOW_DATABASEBAR         = 0x00000001;
const sal_uInt32 FM_UI_FEATURE_SHOW_FIELD               = 0x00000002;
const sal_uInt32 FM_UI_FEATURE_SHOW_PROPERTIES          = 0x00000004;
const sal_uInt32 FM_UI_FEATURE_SHOW_EXPLORER            = 0x00000008;
const sal_uInt32 FM_UI_FEATURE_SHOW_FILTERBAR           = 0x00000010;
const sal_uInt32 FM_UI_FEATURE_SHOW_FILTERNAVIGATOR     = 0x00000020;
const sal_uInt32 FM_UI_FEATURE_SHOW_TEXT_CONTROL_BAR    = 0x00000040;
const sal_uInt32 FM_UI_FEATURE_TB_CONTROLS              = 0x00000080;
const sal_uInt32 FM_UI_FEATURE_TB_MORECONTROLS          = 0x00000100;
const sal_uInt32 FM_UI_FEATURE_TB_FORMDESIGN            = 0x00000200;
const sal_uInt32 FM_UI_FEATURE_SHOW_DATANAVIGATOR       = 0x00000400;

SFX_IMPL_INTERFACE(FmFormShell, SfxShell, SVX_RES(RID_STR_FORMSHELL))
{
    SFX_FEATURED_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_NAVIGATION|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_READONLYDOC,
        SVX_RES(RID_SVXTBX_FORM_NAVIGATION),
        FM_UI_FEATURE_SHOW_DATABASEBAR );

    SFX_FEATURED_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_NAVIGATION|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_READONLYDOC,
        SVX_RES(RID_SVXTBX_FORM_FILTER),
        FM_UI_FEATURE_SHOW_FILTERBAR );

    SFX_FEATURED_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_READONLYDOC,
        SVX_RES( RID_SVXTBX_TEXT_CONTROL_ATTRIBUTES ),
        FM_UI_FEATURE_SHOW_TEXT_CONTROL_BAR );

    SFX_FEATURED_CHILDWINDOW_REGISTRATION(SID_FM_ADD_FIELD, FM_UI_FEATURE_SHOW_FIELD);
    SFX_FEATURED_CHILDWINDOW_REGISTRATION(SID_FM_SHOW_PROPERTIES, FM_UI_FEATURE_SHOW_PROPERTIES);
    SFX_FEATURED_CHILDWINDOW_REGISTRATION(SID_FM_SHOW_FMEXPLORER, FM_UI_FEATURE_SHOW_EXPLORER);
    SFX_FEATURED_CHILDWINDOW_REGISTRATION(SID_FM_FILTER_NAVIGATOR, FM_UI_FEATURE_SHOW_FILTERNAVIGATOR);
    SFX_FEATURED_CHILDWINDOW_REGISTRATION(SID_FM_SHOW_DATANAVIGATOR, FM_UI_FEATURE_SHOW_DATANAVIGATOR);

    SFX_FEATURED_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT | SFX_VISIBILITY_STANDARD,
        SVX_RES( RID_SVXTBX_CONTROLS ),
        FM_UI_FEATURE_TB_CONTROLS );

    SFX_FEATURED_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT | SFX_VISIBILITY_STANDARD,
        SVX_RES( RID_SVXTBX_MORECONTROLS ),
        FM_UI_FEATURE_TB_MORECONTROLS );

    SFX_FEATURED_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT | SFX_VISIBILITY_STANDARD,
        SVX_RES( RID_SVXTBX_FORMDESIGN ),
        FM_UI_FEATURE_TB_FORMDESIGN );
}

//========================================================================
TYPEINIT1(FmFormShell,SfxShell)

//------------------------------------------------------------------------
FmFormShell::FmFormShell( SfxViewShell* _pParent, FmFormView* pView )
            :SfxShell(_pParent)
            ,m_pImpl(new FmXFormShell(*this, _pParent->GetViewFrame()))
            ,m_pFormView( pView )
            ,m_pFormModel( NULL )
            ,m_pParentShell(_pParent)
            ,m_nLastSlot( 0 )
            ,m_bDesignMode( sal_True )
            ,m_bHasForms(sal_False)
{
    m_pImpl->acquire();
    SetPool( &SFX_APP()->GetPool() );
    SetName( rtl::OUString("Form") );

    SetView(m_pFormView);
}

//------------------------------------------------------------------------
FmFormShell::~FmFormShell()
{
    if ( m_pFormView )
        SetView( NULL );

    m_pImpl->dispose();
    m_pImpl->release();
    m_pImpl = NULL;
}

//------------------------------------------------------------------------
void FmFormShell::NotifyMarkListChanged(FmFormView* pWhichView)
{
    FmNavViewMarksChanged aChangeNotification(pWhichView);
    Broadcast(aChangeNotification);
}

//------------------------------------------------------------------------
sal_uInt16 FmFormShell::PrepareClose(sal_Bool bUI, sal_Bool /*bForBrowsing*/)
{
    if ( GetImpl()->didPrepareClose() )
        // we already did a PrepareClose for the current modifications of the current form
        return sal_True;

    sal_Bool bResult = sal_True;
    // Save the data records, not in DesignMode and FilterMode
    if (!m_bDesignMode && !GetImpl()->isInFilterMode() &&
        m_pFormView && m_pFormView->GetActualOutDev() &&
        m_pFormView->GetActualOutDev()->GetOutDevType() == OUTDEV_WINDOW)
    {
        SdrPageView* pCurPageView = m_pFormView->GetSdrPageView();

        // sal_uInt16 nPos = pCurPageView ? pCurPageView->GetWinList().Find((OutputDevice*)m_pFormView->GetActualOutDev()) : SDRPAGEVIEWWIN_NOTFOUND;
        SdrPageWindow* pWindow = pCurPageView ? pCurPageView->FindPageWindow(*((OutputDevice*)m_pFormView->GetActualOutDev())) : 0L;

        if(pWindow)
        {
            // Zunaechst werden die aktuellen Inhalte der Controls gespeichert
            // Wenn alles glatt gelaufen ist, werden die modifizierten Datensaetze gespeichert
            if ( GetImpl()->getActiveController().is() )
            {
                const ::svx::ControllerFeatures& rController = GetImpl()->getActiveControllerFeatures();
                if ( rController->commitCurrentControl() )
                {
                    sal_Bool bModified = rController->isModifiedRow();

                    if ( bModified && bUI )
                    {
                        QueryBox aQry(NULL, SVX_RES(RID_QRY_SAVEMODIFIED));
                        switch (aQry.Execute())
                        {
                            case RET_NO:
                                bModified = sal_False;
                                GetImpl()->didPrepareClose( sal_True );
                                break;

                            case RET_CANCEL:
                                return sal_False;

                            case RET_NEWTASK:
                                return RET_NEWTASK;
                        }

                            if ( bModified )
                                bResult = rController->commitCurrentRecord( );
                    }
                }
            }
        }
    }
    return bResult;
}

//------------------------------------------------------------------------
void FmFormShell::impl_setDesignMode(sal_Bool bDesign)
{
    if (m_pFormView)
    {
        if (!bDesign)
            m_nLastSlot = SID_FM_DESIGN_MODE;

        GetImpl()->SetDesignMode(bDesign);
        // mein m_bDesignMode wird auch von der Impl gesetzt ...
    }
    else
    {
        m_bHasForms = sal_False;
        m_bDesignMode = bDesign;
        UIFeatureChanged();
    }

    GetViewShell()->GetViewFrame()->GetBindings().Invalidate(ControllerSlotMap);
}

//------------------------------------------------------------------------
sal_Bool FmFormShell::HasUIFeature( sal_uInt32 nFeature )
{
    sal_Bool bResult = sal_False;
    if ((nFeature & FM_UI_FEATURE_SHOW_DATABASEBAR) == FM_UI_FEATURE_SHOW_DATABASEBAR)
    {
        // nur wenn auch formulare verfuegbar
        bResult = !m_bDesignMode && GetImpl()->hasDatabaseBar() && !GetImpl()->isInFilterMode();
    }
    else if ((nFeature & FM_UI_FEATURE_SHOW_FILTERBAR) == FM_UI_FEATURE_SHOW_FILTERBAR)
    {
        // nur wenn auch formulare verfuegbar
        bResult = !m_bDesignMode && GetImpl()->hasDatabaseBar() && GetImpl()->isInFilterMode();
    }
    else if ((nFeature & FM_UI_FEATURE_SHOW_FILTERNAVIGATOR) == FM_UI_FEATURE_SHOW_FILTERNAVIGATOR)
    {
        bResult = !m_bDesignMode && GetImpl()->hasDatabaseBar() && GetImpl()->isInFilterMode();
    }
    else if ((nFeature & FM_UI_FEATURE_SHOW_FIELD) == FM_UI_FEATURE_SHOW_FIELD)
    {
        bResult = m_bDesignMode && m_pFormView && m_bHasForms;
    }
    else if ((nFeature & FM_UI_FEATURE_SHOW_PROPERTIES) == FM_UI_FEATURE_SHOW_PROPERTIES)
    {
        bResult = m_bDesignMode && m_pFormView && m_bHasForms;
    }
    else if ((nFeature & FM_UI_FEATURE_SHOW_EXPLORER) == FM_UI_FEATURE_SHOW_EXPLORER)
    {
        bResult = m_bDesignMode; // OJ #101593# && m_pFormView && m_bHasForms;
    }
    else if ( ( nFeature & FM_UI_FEATURE_SHOW_TEXT_CONTROL_BAR ) == FM_UI_FEATURE_SHOW_TEXT_CONTROL_BAR )
    {
        bResult = !GetImpl()->IsReadonlyDoc() && m_pImpl->IsActiveControl( true );
    }
    else if ((nFeature & FM_UI_FEATURE_SHOW_DATANAVIGATOR) == FM_UI_FEATURE_SHOW_DATANAVIGATOR)
    {
        bResult = GetImpl()->isEnhancedForm();
    }
    else if (  ( ( nFeature & FM_UI_FEATURE_TB_CONTROLS ) == FM_UI_FEATURE_TB_CONTROLS )
            || ( ( nFeature & FM_UI_FEATURE_TB_MORECONTROLS ) == FM_UI_FEATURE_TB_MORECONTROLS )
            || ( ( nFeature & FM_UI_FEATURE_TB_FORMDESIGN ) == FM_UI_FEATURE_TB_FORMDESIGN )
            )
    {
        bResult = sal_True;
    }

    return bResult;
}

//------------------------------------------------------------------------
void FmFormShell::Execute(SfxRequest &rReq)
{
    sal_uInt16 nSlot = rReq.GetSlot();

    //////////////////////////////////////////////////////////////////////
    // MasterSlot setzen
    switch( nSlot )
    {
        case SID_FM_PUSHBUTTON:
        case SID_FM_RADIOBUTTON:
        case SID_FM_CHECKBOX:
        case SID_FM_FIXEDTEXT:
        case SID_FM_GROUPBOX:
        case SID_FM_LISTBOX:
        case SID_FM_COMBOBOX:
        case SID_FM_NAVIGATIONBAR:
        case SID_FM_EDIT:
        case SID_FM_DBGRID:
        case SID_FM_IMAGEBUTTON:
        case SID_FM_IMAGECONTROL:
        case SID_FM_FILECONTROL:
        case SID_FM_DATEFIELD:
        case SID_FM_TIMEFIELD:
        case SID_FM_NUMERICFIELD:
        case SID_FM_CURRENCYFIELD:
        case SID_FM_PATTERNFIELD:
        case SID_FM_FORMATTEDFIELD:
        case SID_FM_SCROLLBAR:
        case SID_FM_SPINBUTTON:
            m_nLastSlot = nSlot;
            GetViewShell()->GetViewFrame()->GetBindings().Invalidate( SID_FM_CONFIG );
            break;
    }

    //////////////////////////////////////////////////////////////////////
    // Identifier und Inventor des Uno-Controls setzen
    sal_uInt16 nIdentifier = 0;
    switch( nSlot )
    {
        case SID_FM_CHECKBOX:
            nIdentifier = OBJ_FM_CHECKBOX;
            break;
        case SID_FM_PUSHBUTTON:
            nIdentifier = OBJ_FM_BUTTON;
            break;
        case SID_FM_FIXEDTEXT:
            nIdentifier = OBJ_FM_FIXEDTEXT;
            break;
        case SID_FM_LISTBOX:
            nIdentifier = OBJ_FM_LISTBOX;
            break;
        case SID_FM_EDIT:
            nIdentifier = OBJ_FM_EDIT;
            break;
        case SID_FM_RADIOBUTTON:
            nIdentifier = OBJ_FM_RADIOBUTTON;
            break;
        case SID_FM_GROUPBOX:
            nIdentifier = OBJ_FM_GROUPBOX;
            break;
        case SID_FM_COMBOBOX:
            nIdentifier = OBJ_FM_COMBOBOX;
            break;
        case SID_FM_NAVIGATIONBAR:
            nIdentifier = OBJ_FM_NAVIGATIONBAR;
            break;
        case SID_FM_DBGRID:
            nIdentifier = OBJ_FM_GRID;
            break;
        case SID_FM_IMAGEBUTTON:
            nIdentifier = OBJ_FM_IMAGEBUTTON;
            break;
        case SID_FM_IMAGECONTROL:
            nIdentifier = OBJ_FM_IMAGECONTROL;
            break;
        case SID_FM_FILECONTROL:
            nIdentifier = OBJ_FM_FILECONTROL;
            break;
        case SID_FM_DATEFIELD:
            nIdentifier = OBJ_FM_DATEFIELD;
            break;
        case SID_FM_TIMEFIELD:
            nIdentifier = OBJ_FM_TIMEFIELD;
            break;
        case SID_FM_NUMERICFIELD:
            nIdentifier = OBJ_FM_NUMERICFIELD;
            break;
        case SID_FM_CURRENCYFIELD:
            nIdentifier = OBJ_FM_CURRENCYFIELD;
            break;
        case SID_FM_PATTERNFIELD:
            nIdentifier = OBJ_FM_PATTERNFIELD;
            break;
        case SID_FM_FORMATTEDFIELD:
            nIdentifier = OBJ_FM_FORMATTEDFIELD;
            break;
        case SID_FM_SCROLLBAR:
            nIdentifier = OBJ_FM_SCROLLBAR;
            break;
        case SID_FM_SPINBUTTON:
            nIdentifier = OBJ_FM_SPINBUTTON;
            break;
    }

    switch ( nSlot )
    {
        case SID_FM_CHECKBOX:
        case SID_FM_PUSHBUTTON:
        case SID_FM_FIXEDTEXT:
        case SID_FM_LISTBOX:
        case SID_FM_EDIT:
        case SID_FM_RADIOBUTTON:
        case SID_FM_COMBOBOX:
        case SID_FM_NAVIGATIONBAR:
        case SID_FM_GROUPBOX:
        case SID_FM_DBGRID:
        case SID_FM_IMAGEBUTTON:
        case SID_FM_IMAGECONTROL:
        case SID_FM_FILECONTROL:
        case SID_FM_DATEFIELD:
        case SID_FM_TIMEFIELD:
        case SID_FM_NUMERICFIELD:
        case SID_FM_CURRENCYFIELD:
        case SID_FM_PATTERNFIELD:
        case SID_FM_FORMATTEDFIELD:
        case SID_FM_SCROLLBAR:
        case SID_FM_SPINBUTTON:
        {
            SFX_REQUEST_ARG( rReq, pGrabFocusItem, SfxBoolItem, SID_FM_TOGGLECONTROLFOCUS, sal_False );
            if ( pGrabFocusItem && pGrabFocusItem->GetValue() )
            {   // see below
                SfxViewShell* pShell = GetViewShell();
                Window* pShellWnd = pShell ? pShell->GetWindow() : NULL;
                if ( pShellWnd )
                    pShellWnd->GrabFocus();
                break;
            }

            SfxUInt16Item aIdentifierItem( SID_FM_CONTROL_IDENTIFIER, nIdentifier );
            SfxUInt32Item aInventorItem( SID_FM_CONTROL_INVENTOR, FmFormInventor );
            const SfxPoolItem* pArgs[] =
            {
                &aIdentifierItem, &aInventorItem, NULL
            };
            const SfxPoolItem* pInternalArgs[] =
            {
                NULL
            };

            GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( SID_FM_CREATE_CONTROL, SFX_CALLMODE_ASYNCHRON,
                                      pArgs, rReq.GetModifier(), pInternalArgs );

            if ( rReq.GetModifier() & KEY_MOD1 )
            {
                //  #99013# if selected with control key, return focus to current view
                // do this asynchron, so that the creation can be finished first
                // reusing the SID_FM_TOGGLECONTROLFOCUS is somewhat hacky ... which it wouldn't if it would have another
                // name, so I do not really have a big problem with this ....
                SfxBoolItem aGrabFocusIndicatorItem( SID_FM_TOGGLECONTROLFOCUS, sal_True );
                GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( nSlot, SFX_CALLMODE_ASYNCHRON,
                                          &aGrabFocusIndicatorItem, NULL );
            }

            rReq.Done();
        }   break;
    }

    // Individuelle Aktionen
    switch( nSlot )
    {
        case SID_FM_MORE_CONTROLS:
        case SID_FM_FORM_DESIGN_TOOLS:
        {
            FormToolboxes aToolboxAccess( GetImpl()->getHostFrame() );
            aToolboxAccess.toggleToolbox( nSlot );
            rReq.Done();
        }
        break;

        case SID_FM_TOGGLECONTROLFOCUS:
        {
            FmFormView* pFormView = GetFormView();
            if ( !pFormView )
                break;

            // if we execute this ourself, then either the application does not implement an own handling for this,
            // of we're on the top of the dispatcher stack, which means a control has the focus.
            // In the latter case, we put the focus to the document window, otherwise, we focus the first control
            const bool bHasControlFocus = GetImpl()->HasControlFocus();
            if ( bHasControlFocus )
            {
                const OutputDevice* pDevice = GetCurrentViewDevice();
                Window* pWindow = dynamic_cast< Window* >( const_cast< OutputDevice* >( pDevice ) );
                if ( pWindow )
                    pWindow->GrabFocus();
            }
            else
            {
                pFormView->GrabFirstControlFocus( );
            }
        }
        break;

        case SID_FM_VIEW_AS_GRID:
            GetImpl()->CreateExternalView();
            break;
        case SID_FM_CONVERTTO_EDIT          :
        case SID_FM_CONVERTTO_BUTTON            :
        case SID_FM_CONVERTTO_FIXEDTEXT     :
        case SID_FM_CONVERTTO_LISTBOX       :
        case SID_FM_CONVERTTO_CHECKBOX      :
        case SID_FM_CONVERTTO_RADIOBUTTON   :
        case SID_FM_CONVERTTO_GROUPBOX      :
        case SID_FM_CONVERTTO_COMBOBOX      :
        case SID_FM_CONVERTTO_IMAGEBUTTON   :
        case SID_FM_CONVERTTO_FILECONTROL   :
        case SID_FM_CONVERTTO_DATE          :
        case SID_FM_CONVERTTO_TIME          :
        case SID_FM_CONVERTTO_NUMERIC       :
        case SID_FM_CONVERTTO_CURRENCY      :
        case SID_FM_CONVERTTO_PATTERN       :
        case SID_FM_CONVERTTO_IMAGECONTROL  :
        case SID_FM_CONVERTTO_FORMATTED     :
        case SID_FM_CONVERTTO_SCROLLBAR     :
        case SID_FM_CONVERTTO_SPINBUTTON    :
        case SID_FM_CONVERTTO_NAVIGATIONBAR :
            GetImpl()->executeControlConversionSlot( nSlot );
            // nach dem Konvertieren die Selektion neu bestimmern, da sich ja das selektierte Objekt
            // geaendert hat
            GetImpl()->SetSelection(GetFormView()->GetMarkedObjectList());
            break;
        case SID_FM_LEAVE_CREATE:
            m_nLastSlot = 0;
            GetViewShell()->GetViewFrame()->GetBindings().Invalidate( SID_FM_CONFIG );
            rReq.Done();
            break;
        case SID_FM_SHOW_PROPERTY_BROWSER:
        {
            SFX_REQUEST_ARG( rReq, pShowItem, SfxBoolItem, SID_FM_SHOW_PROPERTIES, sal_False );
            sal_Bool bShow = sal_True;
            if ( pShowItem )
                bShow = pShowItem->GetValue();
            GetImpl()->ShowSelectionProperties( bShow );

            rReq.Done();
        } break;

        case SID_FM_PROPERTIES:
        {
            // PropertyBrowser anzeigen
            SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, nSlot, sal_False);
            sal_Bool bShow = pShowItem ? pShowItem->GetValue() : sal_True;

            InterfaceBag aOnlyTheForm;
            aOnlyTheForm.insert( Reference< XInterface >( GetImpl()->getCurrentForm(), UNO_QUERY ) );
            GetImpl()->setCurrentSelection( aOnlyTheForm );

            GetImpl()->ShowSelectionProperties( bShow );

            rReq.Done();
        }   break;

        case SID_FM_CTL_PROPERTIES:
        {
            SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, nSlot, sal_False);
            sal_Bool bShow = pShowItem ? pShowItem->GetValue() : sal_True;

            OSL_ENSURE( GetImpl()->onlyControlsAreMarked(), "FmFormShell::Execute: ControlProperties should be disabled!" );
            if ( bShow )
                GetImpl()->selectLastMarkedControls();
            GetImpl()->ShowSelectionProperties( bShow );

            rReq.Done();
        }   break;
        case SID_FM_SHOW_PROPERTIES:
        case SID_FM_ADD_FIELD:
        case SID_FM_FILTER_NAVIGATOR:
        case SID_FM_SHOW_DATANAVIGATOR :
        {
            GetViewShell()->GetViewFrame()->ChildWindowExecute( rReq );
            rReq.Done();
        }   break;
        case SID_FM_SHOW_FMEXPLORER:
        {
            if (!m_pFormView)   // setzen der ::com::sun::star::sdbcx::View Forcieren
                GetViewShell()->GetViewFrame()->GetDispatcher()->Execute(SID_CREATE_SW_DRAWVIEW);

            GetViewShell()->GetViewFrame()->ChildWindowExecute(rReq);
            rReq.Done();
        }
        break;

        case SID_FM_TAB_DIALOG:
        {
            GetImpl()->ExecuteTabOrderDialog( Reference< XTabControllerModel >( GetImpl()->getCurrentForm(), UNO_QUERY ) );
            rReq.Done();
        }
        break;

        case SID_FM_DESIGN_MODE:
        {
            SFX_REQUEST_ARG(rReq, pDesignItem, SfxBoolItem, nSlot, sal_False);
            sal_Bool bDesignMode = pDesignItem ? pDesignItem->GetValue() : !m_bDesignMode;
            SetDesignMode( bDesignMode );
            if ( m_bDesignMode == bDesignMode )
                rReq.Done();

            m_nLastSlot = SID_FM_DESIGN_MODE;
            GetViewShell()->GetViewFrame()->GetBindings().Invalidate( SID_FM_CONFIG );
        }
        break;

        case SID_FM_AUTOCONTROLFOCUS:
        {
            FmFormModel* pModel = GetFormModel();
            DBG_ASSERT(pModel, "FmFormShell::Execute : invalid call !");
                // should have been disabled in GetState if we don't have a FormModel
            pModel->SetAutoControlFocus( !pModel->GetAutoControlFocus() );
            GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_AUTOCONTROLFOCUS);
        }
        break;
        case SID_FM_OPEN_READONLY:
        {
            FmFormModel* pModel = GetFormModel();
            DBG_ASSERT(pModel, "FmFormShell::Execute : invalid call !");
                // should have been disabled in GetState if we don't have a FormModel
            pModel->SetOpenInDesignMode( !pModel->GetOpenInDesignMode() );
            GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_OPEN_READONLY);
        }
        break;
        case SID_FM_USE_WIZARDS:
        {
            GetImpl()->SetWizardUsing(!GetImpl()->GetWizardUsing());
            GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_USE_WIZARDS);
        }
        break;
        case SID_FM_SEARCH:
        {
            const ::svx::ControllerFeatures& rController = GetImpl()->getActiveControllerFeatures();
            if ( rController->commitCurrentControl() && rController->commitCurrentRecord() )
                GetImpl()->ExecuteSearch();
            rReq.Done();
        }
        break;

        case SID_FM_RECORD_FIRST:
        case SID_FM_RECORD_PREV:
        case SID_FM_RECORD_NEXT:
        case SID_FM_RECORD_LAST:
        case SID_FM_RECORD_NEW:
        case SID_FM_REFRESH:
        case SID_FM_REFRESH_FORM_CONTROL:
        case SID_FM_RECORD_DELETE:
        case SID_FM_RECORD_UNDO:
        case SID_FM_RECORD_SAVE:
        case SID_FM_REMOVE_FILTER_SORT:
        case SID_FM_SORTDOWN:
        case SID_FM_SORTUP:
        case SID_FM_AUTOFILTER:
        case SID_FM_ORDERCRIT:
        case SID_FM_FORM_FILTERED:
        {
            GetImpl()->ExecuteFormSlot( nSlot );
            rReq.Done();
        }
        break;

        case SID_FM_RECORD_ABSOLUTE:
        {
            const ::svx::ControllerFeatures& rController = GetImpl()->getNavControllerFeatures();
            sal_Int32 nRecord = -1;

            const SfxItemSet* pArgs = rReq.GetArgs();
            if ( pArgs )
            {
                const SfxPoolItem* pItem;
                if ( ( pArgs->GetItemState( FN_PARAM_1, sal_True, &pItem ) ) == SFX_ITEM_SET )
                {
                    const SfxInt32Item* pTypedItem = PTR_CAST( SfxInt32Item, pItem );
                    if ( pTypedItem )
                        nRecord = Max( pTypedItem->GetValue(), sal_Int32(0) );
                }
            }
            else
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                DBG_ASSERT( pFact, "no dialog factory!" );
                if ( pFact )
                {
                    boost::scoped_ptr< AbstractFmInputRecordNoDialog > dlg( pFact->CreateFmInputRecordNoDialog( NULL ) );
                    DBG_ASSERT( dlg.get(), "Dialogdiet fail!" );
                    dlg->SetValue( rController->getCursor()->getRow() );
                    if ( dlg->Execute() == RET_OK )
                        nRecord = dlg->GetValue();

                    rReq.AppendItem( SfxInt32Item( FN_PARAM_1, nRecord ) );
                }
            }

            if ( nRecord != -1 )
                rController->execute( nSlot, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Position" )), makeAny( (sal_Int32)nRecord ) );

            rReq.Done();
        }   break;
        case SID_FM_FILTER_EXECUTE:
        case SID_FM_FILTER_EXIT:
        {
            sal_Bool bCancelled = ( SID_FM_FILTER_EXIT == nSlot );
            sal_Bool bReopenNavigator = sal_False;

            if ( !bCancelled )
            {
                // if the filter navigator is still open, we need to close it, so it can possibly
                // commit it's most recent changes
                if ( GetViewShell() && GetViewShell()->GetViewFrame() )
                    if ( GetViewShell()->GetViewFrame()->HasChildWindow( SID_FM_FILTER_NAVIGATOR ) )
                    {
                        GetViewShell()->GetViewFrame()->ToggleChildWindow( SID_FM_FILTER_NAVIGATOR );
                        bReopenNavigator = sal_True;
                    }

                Reference< runtime::XFormController >  xController( GetImpl()->getActiveController() );

                if  (   GetViewShell()->GetViewFrame()->HasChildWindow( SID_FM_FILTER_NAVIGATOR )
                        // closing the window was denied, for instance because of a invalid criterion

                    ||  (   xController.is()
                        &&  !GetImpl()->getActiveControllerFeatures()->commitCurrentControl( )
                        )
                        // committing the controller was denied
                    )
                {
                    rReq.Done();
                    break;
                }
            }

            GetImpl()->stopFiltering( !bCancelled );
            rReq.Done();

            if ( bReopenNavigator )
                // we closed the navigator only to implicitly commit it (as we do not have another
                // direct wire to it), but to the user, it should look it it was always open
                GetViewShell()->GetViewFrame()->ToggleChildWindow( SID_FM_FILTER_NAVIGATOR );
        }
        break;

        case SID_FM_FILTER_START:
        {
            GetImpl()->startFiltering();
            rReq.Done();

            // initially open the filter navigator, the whole form based filter is pretty useless without it
            SfxBoolItem aIdentifierItem( SID_FM_FILTER_NAVIGATOR, sal_True );
            GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( SID_FM_FILTER_NAVIGATOR, SFX_CALLMODE_ASYNCHRON,
                &aIdentifierItem, NULL );
        }   break;
    }
}

//------------------------------------------------------------------------
void FmFormShell::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch( nWhich )
        {
            case SID_FM_MORE_CONTROLS:
            case SID_FM_FORM_DESIGN_TOOLS:
            {
                FormToolboxes aToolboxAccess( GetImpl()->getHostFrame() );
                rSet.Put( SfxBoolItem( nWhich, aToolboxAccess.isToolboxVisible( nWhich ) ) );
            }
            break;

            case SID_FM_FILTER_EXECUTE:
            case SID_FM_FILTER_EXIT:
                if (!GetImpl()->isInFilterMode())
                    rSet.DisableItem( nWhich );
                break;

            case SID_FM_USE_WIZARDS:
                if  ( !SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
                    rSet.Put( SfxVisibilityItem( nWhich, sal_False ) );
                else if (!GetFormModel())
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem(nWhich, GetImpl()->GetWizardUsing() ) );
                break;
            case SID_FM_AUTOCONTROLFOCUS:
                if (!GetFormModel())
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem(nWhich, GetFormModel()->GetAutoControlFocus() ) );
                break;
            case SID_FM_OPEN_READONLY:
                if (!GetFormModel())
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem(nWhich, GetFormModel()->GetOpenInDesignMode() ) );
                break;

            case SID_FM_NAVIGATIONBAR:
            case SID_FM_DBGRID:
                if ( !SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
                {
                    rSet.Put( SfxVisibilityItem( nWhich, sal_False ) );
                    break;
                }
                // NO break!

            case SID_FM_SCROLLBAR:
            case SID_FM_IMAGECONTROL:
            case SID_FM_FILECONTROL:
            case SID_FM_CURRENCYFIELD:
            case SID_FM_PATTERNFIELD:
            case SID_FM_IMAGEBUTTON:
            case SID_FM_RADIOBUTTON:
            case SID_FM_COMBOBOX:
            case SID_FM_GROUPBOX:
            case SID_FM_CHECKBOX:
            case SID_FM_PUSHBUTTON:
            case SID_FM_FIXEDTEXT:
            case SID_FM_LISTBOX:
            case SID_FM_EDIT:
            case SID_FM_DATEFIELD:
            case SID_FM_TIMEFIELD:
            case SID_FM_NUMERICFIELD:
            case SID_FM_FORMATTEDFIELD:
            case SID_FM_SPINBUTTON:
                if (!m_bDesignMode)
                    rSet.DisableItem( nWhich );
                else
                {
                    sal_Bool bLayerLocked = sal_False;
                    if (m_pFormView)
                    {
                        // Ist der ::com::sun::star::drawing::Layer gelocked, so m???ssen die Slots disabled werden. #36897
                        SdrPageView* pPV = m_pFormView->GetSdrPageView();
                        if (pPV != NULL)
                            bLayerLocked = pPV->IsLayerLocked(m_pFormView->GetActiveLayer());
                    }
                    if (bLayerLocked)
                        rSet.DisableItem( nWhich );
                    else
                        rSet.Put( SfxBoolItem(nWhich, (nWhich==m_nLastSlot)) );
                }
                break;
            case SID_FM_FILTER_NAVIGATOR_CONTROL:
            {
                if (GetImpl()->isInFilterMode())
                    rSet.Put(SfxObjectItem(nWhich, this));
                else
                    rSet.Put(SfxObjectItem(nWhich));
            }   break;
            case SID_FM_FIELDS_CONTROL:
            case SID_FM_PROPERTY_CONTROL:
            {
                if (!m_bDesignMode || !m_pFormView || !m_bHasForms)
                    rSet.Put(SfxObjectItem(nWhich));
                else
                    rSet.Put(SfxObjectItem(nWhich, this));

            }   break;
            case SID_FM_FMEXPLORER_CONTROL:
            case SID_FM_DATANAVIGATOR_CONTROL :
            {
                if (!m_bDesignMode || !m_pFormView)
                    rSet.Put(SfxObjectItem(nWhich));
                else
                    rSet.Put(SfxObjectItem(nWhich, this));

            }   break;
            case SID_FM_ADD_FIELD:
            case SID_FM_SHOW_FMEXPLORER:
            case SID_FM_SHOW_PROPERTIES:
            case SID_FM_FILTER_NAVIGATOR:
            case SID_FM_SHOW_DATANAVIGATOR:
            {
                if ( GetViewShell()->GetViewFrame()->KnowsChildWindow(nWhich) )
                    rSet.Put( SfxBoolItem( nWhich, GetViewShell()->GetViewFrame()->HasChildWindow(nWhich)) );
                else
                    rSet.DisableItem(nWhich);
            }   break;

            case SID_FM_SHOW_PROPERTY_BROWSER:
            {
                rSet.Put(SfxBoolItem(GetImpl()->IsPropBrwOpen()));
            }
            break;

            case SID_FM_CTL_PROPERTIES:
            {
                // der Impl eventuell die Moeglichjkeit geben, ihre an der aktuellen MarkList ausgerichteten Objekte
                // auf den neuesten Stand zu bringen
                if (GetImpl()->IsSelectionUpdatePending())
                    GetImpl()->ForceUpdateSelection(sal_False);

                if ( !m_pFormView || !m_bDesignMode || !GetImpl()->onlyControlsAreMarked() )
                    rSet.DisableItem( nWhich );
                else
                {
                    sal_Bool bChecked  = GetImpl()->IsPropBrwOpen() && !GetImpl()->isSolelySelected( GetImpl()->getCurrentForm() );
                        // if the property browser is open, and only controls are marked, and the current selection
                        // does not consist of only the current form, then the current selection is the (composition of)
                        // the currently marked controls
                    rSet.Put( SfxBoolItem( nWhich, bChecked ) );
                }
            }   break;

            case SID_FM_PROPERTIES:
            {
                // der Impl eventuell die Moeglichjkeit geben, ihre an der aktuellen MarkList ausgerichteten Objekte
                // auf den neuesten Stand zu bringen
                if (GetImpl()->IsSelectionUpdatePending())
                    GetImpl()->ForceUpdateSelection(sal_False);

                if ( !m_pFormView || !m_bDesignMode || !GetImpl()->getCurrentForm().is() )
                    rSet.DisableItem( nWhich );
                else
                {
                    sal_Bool bChecked = GetImpl()->IsPropBrwOpen() && GetImpl()->isSolelySelected( GetImpl()->getCurrentForm() );
                    rSet.Put(SfxBoolItem(nWhich, bChecked));
                }
            }   break;
            case SID_FM_TAB_DIALOG:
                // der Impl eventuell die Moeglichjkeit geben, ihre an der aktuellen MarkList ausgerichteten Objekte
                // auf den neuesten Stand zu bringen
                if (GetImpl()->IsSelectionUpdatePending())
                    GetImpl()->ForceUpdateSelection(sal_False);

                if (!m_pFormView || !m_bDesignMode || !GetImpl()->getCurrentForm().is() )
                    rSet.DisableItem( nWhich );
                break;
            case SID_FM_CONFIG:
                rSet.Put(SfxUInt16Item(nWhich, m_nLastSlot));
                break;
            case SID_FM_DESIGN_MODE:
                if (!m_pFormView || GetImpl()->IsReadonlyDoc() )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem(nWhich, m_bDesignMode) );
                break;
            case SID_FM_SEARCH:
            case SID_FM_RECORD_FIRST:
            case SID_FM_RECORD_NEXT:
            case SID_FM_RECORD_PREV:
            case SID_FM_RECORD_LAST:
            case SID_FM_RECORD_NEW:
            case SID_FM_RECORD_DELETE:
            case SID_FM_RECORD_ABSOLUTE:
            case SID_FM_RECORD_TOTAL:
            case SID_FM_RECORD_SAVE:
            case SID_FM_RECORD_UNDO:
            case SID_FM_FORM_FILTERED:
            case SID_FM_REMOVE_FILTER_SORT:
            case SID_FM_SORTUP:
            case SID_FM_SORTDOWN:
            case SID_FM_ORDERCRIT:
            case SID_FM_FILTER_START:
            case SID_FM_AUTOFILTER:
            case SID_FM_REFRESH:
            case SID_FM_REFRESH_FORM_CONTROL:
            case SID_FM_VIEW_AS_GRID:
                GetFormState(rSet,nWhich);
                break;

            case SID_FM_CHANGECONTROLTYPE:
            {
                if ( !m_pFormView || !m_bDesignMode )
                    rSet.DisableItem( nWhich );
                else
                {
                    if ( !GetImpl()->canConvertCurrentSelectionToControl( OBJ_FM_FIXEDTEXT ) )
                        // if it cannot be converted to a fixed text, it is no single control
                        rSet.DisableItem( nWhich );
                }
            } break;

            case SID_FM_CONVERTTO_FILECONTROL   :
            case SID_FM_CONVERTTO_CURRENCY      :
            case SID_FM_CONVERTTO_PATTERN       :
            case SID_FM_CONVERTTO_IMAGECONTROL  :
            case SID_FM_CONVERTTO_SCROLLBAR     :
            case SID_FM_CONVERTTO_NAVIGATIONBAR :
            case SID_FM_CONVERTTO_IMAGEBUTTON   :
            case SID_FM_CONVERTTO_EDIT          :
            case SID_FM_CONVERTTO_BUTTON        :
            case SID_FM_CONVERTTO_FIXEDTEXT     :
            case SID_FM_CONVERTTO_LISTBOX       :
            case SID_FM_CONVERTTO_CHECKBOX      :
            case SID_FM_CONVERTTO_RADIOBUTTON   :
            case SID_FM_CONVERTTO_GROUPBOX      :
            case SID_FM_CONVERTTO_COMBOBOX      :
            case SID_FM_CONVERTTO_DATE          :
            case SID_FM_CONVERTTO_TIME          :
            case SID_FM_CONVERTTO_NUMERIC       :
            case SID_FM_CONVERTTO_FORMATTED     :
            case SID_FM_CONVERTTO_SPINBUTTON    :
            {
                if ( !m_pFormView || !m_bDesignMode || !GetImpl()->canConvertCurrentSelectionToControl( nWhich ) )
                    rSet.DisableItem( nWhich );
                else
                {
                    rSet.Put( SfxBoolItem( nWhich, sal_False ) );
                    // just to have a defined state (available and not checked)
                }
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

//------------------------------------------------------------------------
void FmFormShell::GetFormState(SfxItemSet &rSet, sal_uInt16 nWhich)
{
    if  (   !GetImpl()->getNavController().is()
        ||  !isRowSetAlive(GetImpl()->getNavController()->getModel())
        ||  !m_pFormView
        ||  m_bDesignMode
        ||  !GetImpl()->getActiveForm().is()
        ||  GetImpl()->isInFilterMode()
        )
        rSet.DisableItem(nWhich);
    else
    {
        sal_Bool bEnable = sal_False;
        try
        {
            switch (nWhich)
            {
            case SID_FM_VIEW_AS_GRID:
                if (GetImpl()->getHostFrame().is() && GetImpl()->getNavController().is())
                {
                    bEnable = sal_True;
                    sal_Bool bDisplayingCurrent =
                        GetImpl()->getInternalForm(
                            Reference< XForm >( GetImpl()->getNavController()->getModel(), UNO_QUERY )
                        ) == GetImpl()->getExternallyDisplayedForm();
                    rSet.Put(SfxBoolItem(nWhich, bDisplayingCurrent));
                }
                break;

            case SID_FM_SEARCH:
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xNavSet(GetImpl()->getActiveForm(), UNO_QUERY);
                sal_Int32 nCount = ::comphelper::getINT32(xNavSet->getPropertyValue(FM_PROP_ROWCOUNT));
                bEnable = nCount != 0;
            }   break;
            case SID_FM_RECORD_ABSOLUTE:
            case SID_FM_RECORD_TOTAL:
            {
                FeatureState aState;
                GetImpl()->getNavControllerFeatures()->getState( nWhich, aState );
                if ( SID_FM_RECORD_ABSOLUTE == nWhich )
                {
                    sal_Int32 nPosition = 0;
                    aState.State >>= nPosition;
                    rSet.Put( SfxInt32Item( nWhich, nPosition ) );
                }
                else if ( SID_FM_RECORD_TOTAL == nWhich )
                {
                    ::rtl::OUString sTotalCount;
                    aState.State >>= sTotalCount;
                    rSet.Put( SfxStringItem( nWhich, sTotalCount ) );
                }
                bEnable = aState.Enabled;
            }
            break;

            // first, prev, next, last, and absolute affect the nav controller, not the
            // active controller
            case SID_FM_RECORD_FIRST:
            case SID_FM_RECORD_PREV:
            case SID_FM_RECORD_NEXT:
            case SID_FM_RECORD_LAST:
            case SID_FM_RECORD_NEW:
            case SID_FM_RECORD_SAVE:
            case SID_FM_RECORD_UNDO:
            case SID_FM_RECORD_DELETE:
            case SID_FM_REFRESH:
            case SID_FM_REFRESH_FORM_CONTROL:
            case SID_FM_REMOVE_FILTER_SORT:
            case SID_FM_SORTUP:
            case SID_FM_SORTDOWN:
            case SID_FM_AUTOFILTER:
            case SID_FM_ORDERCRIT:
                bEnable = GetImpl()->IsFormSlotEnabled( nWhich );
                break;

            case SID_FM_FORM_FILTERED:
            {
                FeatureState aState;
                bEnable = GetImpl()->IsFormSlotEnabled( nWhich, &aState );

                rSet.Put( SfxBoolItem( nWhich, ::comphelper::getBOOL( aState.State ) ) );
            }
            break;

            case SID_FM_FILTER_START:
                bEnable = GetImpl()->getActiveControllerFeatures()->canDoFormFilter();
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmFormShell::GetFormState: caught an exception while determining the state!" );
        }
        if (!bEnable)
            rSet.DisableItem(nWhich);
    }
}

//------------------------------------------------------------------------
FmFormPage* FmFormShell::GetCurPage() const
{
    FmFormPage* pP = NULL;
    if (m_pFormView && m_pFormView->GetSdrPageView())
        pP = PTR_CAST(FmFormPage,m_pFormView->GetSdrPageView()->GetPage());
    return pP;
}

//------------------------------------------------------------------------
void FmFormShell::SetView( FmFormView* _pView )
{
    if ( m_pFormView )
    {
        if ( IsActive() )
            GetImpl()->viewDeactivated( *m_pFormView );

        m_pFormView->SetFormShell( NULL, FmFormView::FormShellAccess() );
        m_pFormView = NULL;
        m_pFormModel = NULL;
    }

    if ( !_pView )
        return;

    m_pFormView = _pView;
    m_pFormView->SetFormShell( this, FmFormView::FormShellAccess() );
    m_pFormModel = (FmFormModel*)m_pFormView->GetModel();

    impl_setDesignMode( m_pFormView->IsDesignMode() );

    // We activate our view if we are activated ourself, but sometimes the Activate precedes the SetView.
    // But here we know both the view and our activation state so we at least are able to pass the latter
    // to the former.
    // FS - 30.06.99 - 67308
    if ( IsActive() )
        GetImpl()->viewActivated( *m_pFormView );
}

//------------------------------------------------------------------------
void FmFormShell::DetermineForms(sal_Bool bInvalidate)
{
    // Existieren Formulare auf der aktuellen Page
    sal_Bool bForms = GetImpl()->hasForms();
    if (bForms != m_bHasForms)
    {
        m_bHasForms = bForms;
        if (bInvalidate)
            UIFeatureChanged();
    }
}

//------------------------------------------------------------------------
sal_Bool FmFormShell::GetY2KState(sal_uInt16& nReturn)
{
    return GetImpl()->GetY2KState(nReturn);
}

//------------------------------------------------------------------------
void FmFormShell::SetY2KState(sal_uInt16 n)
{
    GetImpl()->SetY2KState(n);
}

//------------------------------------------------------------------------
void FmFormShell::Activate(sal_Bool bMDI)
{
    SfxShell::Activate(bMDI);

    if ( m_pFormView )
        GetImpl()->viewActivated( *m_pFormView, sal_True );
}

//------------------------------------------------------------------------
void FmFormShell::Deactivate(sal_Bool bMDI)
{
    SfxShell::Deactivate(bMDI);

    if ( m_pFormView )
        GetImpl()->viewDeactivated( *m_pFormView, sal_False );
}

//------------------------------------------------------------------------
void FmFormShell::ExecuteTextAttribute( SfxRequest& _rReq )
{
    m_pImpl->ExecuteTextAttribute( _rReq );
}

//------------------------------------------------------------------------
void FmFormShell::GetTextAttributeState( SfxItemSet& _rSet )
{
    m_pImpl->GetTextAttributeState( _rSet );
}

//------------------------------------------------------------------------
bool FmFormShell::IsActiveControl() const
{
    return m_pImpl->IsActiveControl();
}

//------------------------------------------------------------------------
void FmFormShell::ForgetActiveControl()
{
    m_pImpl->ForgetActiveControl();
}

//------------------------------------------------------------------------
void FmFormShell::SetControlActivationHandler( const Link& _rHdl )
{
    m_pImpl->SetControlActivationHandler( _rHdl );
}

//------------------------------------------------------------------------
namespace
{
    SdrUnoObj* lcl_findUnoObject( const SdrObjList& _rObjList, const Reference< XControlModel >& _rxModel )
    {
        SdrObjListIter aIter( _rObjList );
        while ( aIter.IsMore() )
        {
            SdrObject* pObject = aIter.Next();
            SdrUnoObj* pUnoObject = pObject ? PTR_CAST( SdrUnoObj, pObject ) : NULL;
            if ( !pUnoObject )
                continue;

            Reference< XControlModel > xControlModel = pUnoObject->GetUnoControlModel();
            if ( !xControlModel.is() )
                continue;

            if ( _rxModel == xControlModel )
                return pUnoObject;
        }
        return NULL;
    }
}

//------------------------------------------------------------------------
void FmFormShell::ToggleControlFocus( const SdrUnoObj& i_rUnoObject, const SdrView& i_rView, OutputDevice& i_rDevice ) const
{
    try
    {
        // check if the focus currently is in a control
        // Well, okay, do it the other way 'round: Check whether the current control of the active controller
        // actually has the focus. This should be equivalent.
        const bool bHasControlFocus = GetImpl()->HasControlFocus();

        if ( bHasControlFocus )
        {
            Window* pWindow( dynamic_cast< Window* >( &i_rDevice ) );
            OSL_ENSURE( pWindow, "FmFormShell::ToggleControlFocus: I need a Window, really!" );
            if ( pWindow )
                pWindow->GrabFocus();
        }
        else
        {
            Reference< XControl > xControl;
            GetFormControl( i_rUnoObject.GetUnoControlModel(), i_rView, i_rDevice, xControl );
            Reference< XWindow > xControlWindow( xControl, UNO_QUERY );
            if ( xControlWindow.is() )
                xControlWindow->setFocus();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------
namespace
{
    class FocusableControlsFilter : public ::svx::ISdrObjectFilter
    {
    public:
        FocusableControlsFilter( const SdrView& i_rView, const OutputDevice& i_rDevice )
            :m_rView( i_rView )
            ,m_rDevice( i_rDevice )
        {
        }

    public:
        virtual bool    includeObject( const SdrObject& i_rObject ) const
        {
            const SdrUnoObj* pUnoObj = dynamic_cast< const SdrUnoObj* >( &i_rObject );
            if ( !pUnoObj )
                return false;

            Reference< XControl > xControl = pUnoObj->GetUnoControl( m_rView, m_rDevice );
            return FmXFormView::isFocusable( xControl );
        }

    private:
        const SdrView&      m_rView;
        const OutputDevice& m_rDevice;
    };
}

//------------------------------------------------------------------------
SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr< ::svx::ISdrObjectFilter > FmFormShell::CreateFocusableControlFilter( const SdrView& i_rView, const OutputDevice& i_rDevice ) const
{
    ::std::auto_ptr< ::svx::ISdrObjectFilter > pFilter;

    if ( !i_rView.IsDesignMode() )
        pFilter.reset( new FocusableControlsFilter( i_rView, i_rDevice ) );

    return pFilter;
}
SAL_WNODEPRECATED_DECLARATIONS_POP

//------------------------------------------------------------------------
SdrUnoObj* FmFormShell::GetFormControl( const Reference< XControlModel >& _rxModel, const SdrView& _rView, const OutputDevice& _rDevice, Reference< XControl >& _out_rxControl ) const
{
    if ( !_rxModel.is() )
        return NULL;

    // we can only retrieve controls for SdrObjects which belong to page which is actually displayed in the given view
    SdrPageView* pPageView = _rView.GetSdrPageView();
    SdrPage* pPage = pPageView ? pPageView->GetPage() : NULL;
    OSL_ENSURE( pPage, "FmFormShell::GetFormControl: no page displayed in the given view!" );
    if ( !pPage )
        return NULL;

    SdrUnoObj* pUnoObject = lcl_findUnoObject( *pPage, _rxModel );
    if ( pUnoObject )
    {
        _out_rxControl = pUnoObject->GetUnoControl( _rView, _rDevice );
        return pUnoObject;
    }

#if OSL_DEBUG_LEVEL > 0
    // perhaps we are fed with a control model which lives on a page other than the one displayed
    // in the given view. This is worth being reported as error, in non-product builds.
    FmFormModel* pModel = GetFormModel();
    if ( pModel )
    {
        sal_uInt16 pageCount = pModel->GetPageCount();
        for ( sal_uInt16 page = 0; page < pageCount; ++page )
        {
            pPage = pModel->GetPage( page );
            OSL_ENSURE( pPage, "FmFormShell::GetFormControl: NULL page encountered!" );
            if  ( !pPage )
                continue;

            pUnoObject = lcl_findUnoObject( *pPage, _rxModel );
            OSL_ENSURE( !pUnoObject, "FmFormShell::GetFormControl: the given control model belongs to a wrong page (displayed elsewhere)!" );
        }
    }
#endif

    return NULL;
}

//------------------------------------------------------------------------
Reference< runtime::XFormController > FmFormShell::GetFormController( const Reference< XForm >& _rxForm, const SdrView& _rView, const OutputDevice& _rDevice ) const
{
    const FmFormView* pFormView = dynamic_cast< const FmFormView* >( &_rView );
    if ( !pFormView )
        return NULL;

    return pFormView->GetFormController( _rxForm, _rDevice );
}

//------------------------------------------------------------------------
void FmFormShell::SetDesignMode( sal_Bool _bDesignMode )
{
    if ( _bDesignMode == m_bDesignMode )
        return;

    FmFormModel* pModel = GetFormModel();
    if (pModel)
        // fuer die Zeit des Uebergangs das Undo-Environment ausschalten, das sichert, dass man dort auch nicht-transiente
        // Properties mal eben aendern kann (sollte allerdings mit Vorsicht genossen und beim Rueckschalten des Modes
        // auch immer wieder rueckgaegig gemacht werden. Ein Beispiel ist das Setzen der maximalen Text-Laenge durch das
        // OEditModel an seinem Control.)
        pModel->GetUndoEnv().Lock();

    // dann die eigentliche Umschaltung
    if ( m_bDesignMode || PrepareClose( sal_True ) )
        impl_setDesignMode(!m_bDesignMode );

    // und mein Undo-Environment wieder an
    if ( pModel )
        pModel->GetUndoEnv().UnLock();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
