/*************************************************************************
 *
 *  $RCSfile: fmshell.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-23 16:25:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop
#ifndef _SVX_FMVWIMP_HXX
#include "fmvwimp.hxx"
#endif

#ifndef _SVX_FMSHELL_HXX
#include "fmshell.hxx"
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_FMUNOPGE_HXX
#include "fmpgeimp.hxx"
#endif
#ifndef _SVX_FMITEMS_HXX
#include "fmitems.hxx"
#endif
#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
#endif

#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif

#ifndef _MULTIPRO_HXX
#include "multipro.hxx"
#endif

#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
//#ifndef _COM_SUN_STAR_DATA_XDATABASEDIALOGS_HPP_
//#include <com/sun/star/data/XDatabaseDialogs.hpp>
//#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_SDDB_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif

#ifndef _COM_SUN_STAR_FORM_XCONFIRMDELETELISTENER_HPP_
#include <com/sun/star/form/XConfirmDeleteListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ROWCHANGEACTION_HPP_
#include <com/sun/star/sdb/RowChangeAction.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif

#ifndef _SVDOBJ_HXX //autogen
#include "svdobj.hxx"
#endif

#ifndef _SVX_FMPAGE_HXX
#include "fmpage.hxx"
#endif

#ifndef _SVDITER_HXX //autogen
#include "svditer.hxx"
#endif

#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SVX_ADTABDLG_HXX
#include "adtabdlg.hxx"
#endif

#ifndef _SVX_TABORDER_HXX
#include "taborder.hxx"
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _SVX_FMEXCH_HXX
#include "fmexch.hxx"
#endif

#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVDPAGE_HXX //autogen
#include <svdpage.hxx>
#endif

#ifndef _SVX_FMMODEL_HXX
#include "fmmodel.hxx"
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _SVX_FMSHIMP_HXX
#include "fmshimp.hxx"
#endif

#ifndef _SVDPAGV_HXX //autogen
#include "svdpagv.hxx"
#endif

#ifndef _SFX_OBJITEM_HXX //autogen
#include <sfx2/objitem.hxx>
#endif

#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif

#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

#ifndef _SVX_FMEXPL_HXX
#include "fmexpl.hxx"
#endif

#ifndef _NUMUNO_HXX //autogen
#include <svtools/numuno.hxx>
#endif

#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#define HANDLE_SQL_ERRORS( action, successflag, context, message )          \
    try                                                                     \
    {                                                                       \
        successflag = sal_False;                                                \
        action;                                                             \
        successflag = sal_True;                                                 \
    }                                                                       \
    catch(::com::sun::star::sdbc::SQLException& e)                                                  \
    {                                                                       \
        ::com::sun::star::sdb::SQLContext eExtendedInfo =                                           \
        ::dbtools::prependContextInfo(e, Reference< XInterface > (), context);              \
        displayException(eExtendedInfo);                                    \
    }                                                                       \
    catch(Exception&)                                                           \
    {                                                                       \
        DBG_ERROR(message);                                                 \
    }                                                                       \


#define DO_SAFE_WITH_ERROR( action, message ) try { action; } catch(Exception&) { DBG_ERROR(message); }


extern SfxType0 aSfxVoidItem_Impl;

#define FmFormShell
#include <svxslots.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <svxids.hrc>
#endif

#ifndef _SVX_TBXFORM_HXX
#include "tbxform.hxx"
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

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
    SID_FM_URLBUTTON,
    SID_FM_DBGRID,
    SID_FM_IMAGEBUTTON,
    SID_FM_FILECONTROL,
    SID_FM_CTL_PROPERTIES,
    SID_FM_PROPERTIES,
    SID_FM_TAB_DIALOG,
    SID_FM_DESIGN_MODE,
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
    SID_FM_AUTOCONTROLFOCUS,
    0
};

sal_uInt16 DatabaseSlotMap[] =  // slots des Controllers
{
    SID_FM_RECORD_FIRST,
    SID_FM_RECORD_NEXT,
    SID_FM_RECORD_PREV,
    SID_FM_RECORD_LAST,
    SID_FM_RECORD_NEW,
    SID_FM_RECORD_DELETE,
    SID_FM_RECORD_ABSOLUTE,
    SID_FM_RECORD_TOTAL,
    SID_FM_RECORD_SAVE,
    SID_FM_RECORD_UNDO,
    SID_FM_REMOVE_FILTER_SORT,
    SID_FM_SORTUP,
    SID_FM_SORTDOWN,
    SID_FM_ORDERCRIT,
    SID_FM_AUTOFILTER,
    SID_FM_FORM_FILTERED,
    SID_FM_REFRESH,
    SID_FM_SEARCH,
    SID_FM_FILTER_START,
    SID_FM_VIEW_AS_GRID,
    0
};

sal_uInt16 AutoSlotMap[] =
{
    SID_FM_SORTUP,
    SID_FM_SORTDOWN,
    SID_FM_AUTOFILTER,
    0
};
using namespace ::com::sun::star::uno;
using namespace ::svxform;

//========================================================================
//========================================================================
#ifdef NOOLDSV
//------------------------------------------------------------------------
FmFormShell::FormShellWaitObject::FormShellWaitObject(const FmFormShell* _pShell)
    :m_pWindow(NULL)
{
    const OutputDevice* pDevice = _pShell ? _pShell->GetCurrentViewDevice() : NULL;
    m_pWindow = pDevice && (pDevice->GetOutDevType() == OUTDEV_WINDOW) ? const_cast<Window*>(static_cast<const Window*>(pDevice)) : NULL;

    if (m_pWindow)
        m_pWindow->EnterWait();
    else
        DBG_ERROR("FormShellWaitObject::FormShellWaitObject : could not find a window for the waitcursor !");
}
//------------------------------------------------------------------------
FmFormShell::FormShellWaitObject::~FormShellWaitObject()
{
    if (m_pWindow)
        m_pWindow->LeaveWait();
}
#else
class FormShellWaitObject : public WaitObject
{
public:
    FormShellWaitObject(FmFormShell* _pShell) { }
};
#endif
//========================================================================
//------------------------------------------------------------------------
sal_Bool IsFormComponent( const SdrObject& rObj )
{
    //////////////////////////////////////////////////////////////////////
    // Gruppenobjekte rekursiv pruefen
    if( rObj.IsGroupObject() )
    {
        SdrObject* pObj;
        SdrObjListIter aIter( *rObj.GetSubList() );

        while( aIter.IsMore() )
        {
            pObj = aIter.Next();

            if( !IsFormComponent(*pObj) )
                return sal_False;
        }
    }

    //////////////////////////////////////////////////////////////////
    // ...ansonsten Pruefen, ob SdrObj eine FormComponent ist
    else
    {
        if( !rObj.ISA(SdrUnoObj) )
            return sal_False;

        Reference< ::com::sun::star::form::XFormComponent >  xFormComponent(((SdrUnoObj*)&rObj)->GetUnoControlModel(), UNO_QUERY);
        if (!xFormComponent.is())
            return sal_False;
    }

    return sal_True;
}

//------------------------------------------------------------------------
sal_Bool IsFormComponentList( const SdrMarkList& rMarkList )
{
    SdrObject* pObj;
    sal_uInt32 nMarkCount = rMarkList.GetMarkCount();

    if( nMarkCount==0 )
        return sal_False;

    for( sal_uInt32 i=0; i<nMarkCount; i++ )
    {
        pObj = rMarkList.GetMark(i)->GetObj();
        if( !IsFormComponent(*pObj) )
            return sal_False;
    }

    return sal_True;
}

//------------------------------------------------------------------------
sal_Bool IsFormComponentList( const SdrObjList& rSdrObjList )
{
    SdrObject* pSdrObj;


    SdrObjListIter aIter( rSdrObjList );
    while( aIter.IsMore() )
    {
        pSdrObj = aIter.Next();

        //////////////////////////////////////////////////////////////////
        // Gruppenobjekte rekursiv pruefen
        if( pSdrObj->IsGroupObject() )
        {
            if( !IsFormComponentList(*pSdrObj->GetSubList()) )
                return sal_False;
        }

        //////////////////////////////////////////////////////////////////
        // ...ansonsten Pruefen, ob SdrObj eine FormComponent ist
        else
        {
            if (!pSdrObj->ISA(SdrUnoObj))
                return sal_False;

            Reference< ::com::sun::star::form::XFormComponent >  xFormComponent(((SdrUnoObj*)pSdrObj)->GetUnoControlModel(), UNO_QUERY);
            if (!xFormComponent.is())
                return sal_False;
        }
    }

    return sal_True;
}

#if SUPD>500
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
#endif

//========================================================================
const sal_uInt32 FM_UI_FEATURE_SHOW_DATABASEBAR     = 0x00000001;
const sal_uInt32 FM_UI_FEATURE_SHOW_FIELD           = 0x00000002;
const sal_uInt32 FM_UI_FEATURE_SHOW_PROPERTIES      = 0x00000004;
const sal_uInt32 FM_UI_FEATURE_SHOW_EXPLORER            = 0x00000008;
const sal_uInt32 FM_UI_FEATURE_SHOW_FILTERBAR       = 0x00000010;
const sal_uInt32 FM_UI_FEATURE_SHOW_FILTERNAVIGATOR = 0x00000020;

SFX_IMPL_INTERFACE(FmFormShell, SfxShell, SVX_RES(RID_STR_FORMSHELL))
{
    SFX_FEATURED_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_NAVIGATION|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_READONLYDOC,
        SVX_RES(RID_SVXTBX_FORM_NAVIGATION),
        FM_UI_FEATURE_SHOW_DATABASEBAR );

    SFX_FEATURED_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_NAVIGATION|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_READONLYDOC,
        SVX_RES(RID_SVXTBX_FORM_FILTER),
        FM_UI_FEATURE_SHOW_FILTERBAR );

    SFX_FEATURED_CHILDWINDOW_REGISTRATION(SID_FM_ADD_FIELD, FM_UI_FEATURE_SHOW_FIELD);
    SFX_FEATURED_CHILDWINDOW_REGISTRATION(SID_FM_SHOW_PROPERTIES, FM_UI_FEATURE_SHOW_PROPERTIES);
    SFX_FEATURED_CHILDWINDOW_REGISTRATION(SID_FM_SHOW_FMEXPLORER, FM_UI_FEATURE_SHOW_EXPLORER);
    SFX_FEATURED_CHILDWINDOW_REGISTRATION(SID_FM_FILTER_NAVIGATOR, FM_UI_FEATURE_SHOW_FILTERNAVIGATOR);
}

//========================================================================
TYPEINIT1(FmFormShell,SfxShell)

//------------------------------------------------------------------------
FmFormShell::FmFormShell( SfxViewShell* _pParent, FmFormView* pView )
            :SfxShell(_pParent)
            ,m_pFormView( pView )
            ,m_pFormModel( NULL )
            ,m_pImpl(new FmXFormShell(this, _pParent->GetViewFrame()))
            ,m_nLastSlot( 0 )
            ,m_bDesignMode( sal_True )
            ,m_bHasForms(sal_False)
            ,m_pParentShell(_pParent)
{
    m_pImpl->acquire();
    SetPool( &SFX_APP()->GetPool() );
    String sName;
    sName.AssignAscii("Form");
    SetName( sName );
    // SetHelpId( SVX_INTERFACE_FORM_SH );

    SetView(m_pFormView);
}

//------------------------------------------------------------------------
FmFormShell::~FmFormShell()
{
    if (m_pFormView)
    {
        GetImpl()->ResetForms();
        m_pFormView->SetFormShell(NULL);
        m_pFormView  = NULL;
    }

    m_pImpl->dispose();
    m_pImpl->release();

    // reset the model after disposing the shell !
    m_pFormModel = NULL;
}

//------------------------------------------------------------------------
void FmFormShell::NotifyMarkListChanged(FmFormView* pWhichView)
{
    FmExplViewMarksChanged aChangeNotification(pWhichView);
    Broadcast(aChangeNotification);
}

//------------------------------------------------------------------------
sal_uInt16 FmFormShell::PrepareClose(sal_Bool bUI, sal_Bool bForBrowsing)
{
    sal_Bool nResult = sal_True;
    // Save the data records, not in DesignMode and FilterMode
    if (!m_bDesignMode && !GetImpl()->isInFilterMode() &&
        m_pFormView && m_pFormView->GetActualOutDev() &&
        m_pFormView->GetActualOutDev()->GetOutDevType() == OUTDEV_WINDOW)
    {
        if (GetImpl()->HasAnyPendingCursorAction())
            GetImpl()->CancelAnyPendingCursorAction();

        SdrPageView* pCurPageView = m_pFormView->GetPageViewPvNum(0);
        sal_uInt16 nPos = pCurPageView ? pCurPageView->GetWinList().Find((OutputDevice*)m_pFormView->GetActualOutDev()) : SDRPAGEVIEWWIN_NOTFOUND;
        if (nPos != SDRPAGEVIEWWIN_NOTFOUND)
        {
            // Zunaechst werden die aktuellen Inhalte der Controls gespeichert
            // Wenn alles glatt gelaufen ist, werden die modifizierten Datensaetze gespeichert
            Reference< ::com::sun::star::form::XFormController >  xController(GetImpl()->getActiveController());
            if (xController.is() && FmXFormShell::CommitCurrent(xController))
            {
                sal_Bool bModified = FmXFormShell::IsModified(xController);

                if (bModified)
                {
                    QueryBox aQry(NULL, SVX_RES(RID_QRY_SAVEMODIFIED));
                    if (bForBrowsing)
                        aQry.AddButton(ResId(RID_STR_NEW_TASK), RET_NEWTASK,
                            BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON);

                    switch (aQry.Execute())
                    {
                        case RET_NO:
                            bModified = sal_False;
                            break;
                        case RET_CANCEL:
                            return sal_False;
                        case RET_NEWTASK:
                            return RET_NEWTASK;
                    }

                    if (bModified)
                        nResult = (sal_uInt16)FmXFormShell::SaveModified(xController, sal_False);
                }
            }
        }
    }
    return nResult;
}

//------------------------------------------------------------------------
void FmFormShell::SetDesignMode(sal_Bool bDesign)
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
        bResult = m_bDesignMode;
    }
    return bResult;
}
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

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
        case SID_FM_URLBUTTON:
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
            m_nLastSlot = nSlot;
            GetViewShell()->GetViewFrame()->GetBindings().Invalidate( SID_FM_CONFIG );
            break;
        case SID_FM_DESIGN_MODE:    // gibt es was zu tun?
        {
            // if we are moving our data source cursor currently ....
            if (GetImpl()->HasAnyPendingCursorAction())
                // ... cancel this
                GetImpl()->CancelAnyPendingCursorAction();

            SFX_REQUEST_ARG(rReq, pDesignItem, SfxBoolItem, nSlot, sal_False);
            if (!pDesignItem || pDesignItem->GetValue() != m_bDesignMode)
            {
                m_nLastSlot = nSlot;
                GetViewShell()->GetViewFrame()->GetBindings().Invalidate( SID_FM_CONFIG );
            }
            else
            {
                rReq.Done();
                return;
            }
        }   break;
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
        {
            SfxUInt16Item aIdentifierItem( SID_FM_CONTROL_IDENTIFIER, nIdentifier );
            SfxUInt32Item aInventorItem( SID_FM_CONTROL_INVENTOR, FmFormInventor );
            GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( SID_FM_CREATE_CONTROL, SFX_CALLMODE_ASYNCHRON,
                                      &aInventorItem, &aIdentifierItem, 0L );
            rReq.Done();
        }   break;
    }

    sal_Bool bSortUp = sal_True;
    // Individuelle Aktionen
    switch( nSlot )
    {
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
        case SID_FM_CONVERTTO_GRID          :
        case SID_FM_CONVERTTO_IMAGEBUTTON   :
        case SID_FM_CONVERTTO_FILECONTROL   :
        case SID_FM_CONVERTTO_DATE          :
        case SID_FM_CONVERTTO_TIME          :
        case SID_FM_CONVERTTO_NUMERIC       :
        case SID_FM_CONVERTTO_CURRENCY      :
        case SID_FM_CONVERTTO_PATTERN       :
        case SID_FM_CONVERTTO_IMAGECONTROL  :
        case SID_FM_CONVERTTO_FORMATTED     :
            GetImpl()->ExecuteControlConversionSlot(Reference< ::com::sun::star::form::XFormComponent > (GetImpl()->getCurControl(), UNO_QUERY),
                nSlot);
            // nach dem Konvertieren die Selektion neu bestimmern, da sich ja das selektierte Objekt
            // geaendert hat
            GetImpl()->SetSelection(GetFormView()->GetMarkList());
            break;
        case SID_FM_LEAVE_CREATE:
            m_nLastSlot = 0;
            GetViewShell()->GetViewFrame()->GetBindings().Invalidate( SID_FM_CONFIG );
            rReq.Done();
            break;
        case SID_FM_SHOW_PROPERTY_BROWSER:
        {
            SFX_REQUEST_ARG( rReq, pInterfaceItem, FmInterfaceItem, nSlot, sal_False );
            SFX_REQUEST_ARG( rReq, pShowItem, SfxBoolItem, SID_FM_SHOW_PROPERTIES, sal_False );
            Reference< XInterface >  xInterface;
            sal_Bool bShow = sal_True;
            if( pShowItem )
                bShow = pShowItem->GetValue();
            if( pInterfaceItem )
                xInterface = pInterfaceItem->GetInterface();
            GetImpl()->ShowProperties( xInterface, bShow );

            rReq.Done();
        } break;

        case SID_FM_PROPERTIES:
        {
            // PropertyBrowser anzeigen
            SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, nSlot, sal_False);
            sal_Bool bShow = pShowItem ? pShowItem->GetValue() : sal_True;
            GetImpl()->setSelObject(GetImpl()->getCurForm());
            GetImpl()->ShowProperties(GetImpl()->getCurForm(), bShow );

            rReq.Done();
        }   break;

        case SID_FM_CTL_PROPERTIES:
        {
            GetImpl()->setSelObject(GetImpl()->getCurControl());

            // PropertyBrowser anzeigen
            SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, nSlot, sal_False);
            sal_Bool bShow = pShowItem ? pShowItem->GetValue() : sal_True;
            GetImpl()->ShowProperties( GetImpl()->getSelObject(), bShow );

            rReq.Done();
        }   break;
        case SID_FM_SHOW_PROPERTIES:
        case SID_FM_ADD_FIELD:
        case SID_FM_FILTER_NAVIGATOR:
        {
            GetViewShell()->GetViewFrame()->ChildWindowExecute(rReq);
            rReq.Done();
        }   break;
        case SID_FM_SHOW_FMEXPLORER:
        {
            if (!m_pFormView)   // setzen der ::com::sun::star::sdbcx::View Forcieren
                GetViewShell()->GetViewFrame()->GetDispatcher()->Execute(SID_CREATE_SW_DRAWVIEW);

            GetViewShell()->GetViewFrame()->ChildWindowExecute(rReq);
            rReq.Done();
        }   break;
        case SID_FM_TAB_DIALOG:
        {
            FmTabOrderDlg aTabOrderDlg(::comphelper::getProcessServiceFactory(), GetpApp()->GetAppWindow(), this );
            aTabOrderDlg.Execute();
            rReq.Done();
        }   break;
        case SID_FM_DESIGN_MODE:
        {
            SFX_REQUEST_ARG(rReq, pDesignItem, SfxBoolItem, nSlot, sal_False);
            if (!pDesignItem || pDesignItem->GetValue() != m_bDesignMode)
            {
                FmFormModel* pModel = GetFormModel();
                if (pModel)
                {   // fuer die Zeit des Uebergangs das Undo-Environment ausschalten, das sichert, dass man dort auch nicht-transiente
                    // Properties mal eben aendern kann (sollte allerdings mit Vorsicht genossen und beim Rueckschalten des Modes
                    // auch immer wieder rueckgaegig gemacht werden. Ein Beispiel ist das Setzen der maximalen Text-Laenge durch das
                    // OEditModel an seinem Control.)
                    pModel->GetUndoEnv().Lock();
                }

                // dann die eigentliche Umschaltung
                if (m_bDesignMode || PrepareClose(sal_True))
                {
                    SetDesignMode(!m_bDesignMode );
                    rReq.Done();
                }
                // und mein Undo-Environment wieder an
                if (pModel)
                    pModel->GetUndoEnv().UnLock();
            }
        }   break;
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
            if (GetImpl()->SaveModified(GetImpl()->getActiveController()))
                GetImpl()->ExecuteSearch();
            rReq.Done();
        } break;
        // First, Next, Prev, Last, Absolute wirken auf den NavController
        case SID_FM_RECORD_FIRST:
        {
            if (GetImpl()->SaveModified(GetImpl()->getNavController()))
            {
                Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(GetImpl()->getNavController()->getModel(), UNO_QUERY);
                DO_SAFE( xCursor->first(); );
            }
            rReq.Done();
        }   break;
        case SID_FM_RECORD_NEXT:
        {
            if (!GetImpl()->CommitCurrent(GetImpl()->getNavController()))
            {
                rReq.Done();
                break;
            }
            // SaveModified kann hier nicht verwendet werden
            GetImpl()->MoveRight(GetImpl()->getNavController());
            rReq.Done();
        }   break;
        case SID_FM_RECORD_PREV:
        {
            if (!GetImpl()->CommitCurrent(GetImpl()->getNavController()))
            {
                rReq.Done();
                break;
            }
            // SaveModified kann hier nicht verwendet werden
            GetImpl()->MoveLeft(GetImpl()->getNavController());
            rReq.Done();
        }   break;
        case SID_FM_RECORD_LAST:
        {
            if (GetImpl()->SaveModified(GetImpl()->getNavController()))
            {
                Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(GetImpl()->getNavController()->getModel(), UNO_QUERY);

                // run in an own thread if ...
                Reference< ::com::sun::star::beans::XPropertySet >  xCursorProps(xCursor, UNO_QUERY);
                // ... the data source is thread safe ...
                sal_Bool bAllowOwnThread = ::comphelper::hasProperty(FM_PROP_THREADSAFE, xCursorProps) && ::comphelper::getBOOL(xCursorProps->getPropertyValue(FM_PROP_THREADSAFE));
                // ... the record count is unknown
                sal_Bool bNeedOwnThread = ::comphelper::hasProperty(FM_PROP_ROWCOUNTFINAL, xCursorProps) && !::comphelper::getBOOL(xCursorProps->getPropertyValue(FM_PROP_ROWCOUNTFINAL));

                if (bNeedOwnThread && bAllowOwnThread)
                    GetImpl()->DoAsyncCursorAction(GetImpl()->getNavController(), FmXFormShell::CA_MOVE_TO_LAST);
                else
                    DO_SAFE( xCursor->last(); );
            }
            rReq.Done();
        }   break;
        case SID_FM_RECORD_NEW:
        {
            if (GetImpl()->SaveModified(GetImpl()->getActiveController()))
            {
                Reference< ::com::sun::star::sdbc::XResultSetUpdate >  xUpdateCursor(GetImpl()->getActiveForm(), UNO_QUERY);
                DO_SAFE( xUpdateCursor->moveToInsertRow(); );
            }
            rReq.Done();
        }   break;
        case SID_FM_RECORD_DELETE:
        {
            Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(GetImpl()->getActiveForm(), UNO_QUERY);
            Reference< ::com::sun::star::sdbc::XResultSetUpdate >  xUpdateCursor(xCursor, UNO_QUERY);

            Reference< ::com::sun::star::beans::XPropertySet >  xSet(GetImpl()->getActiveForm(), UNO_QUERY);
            sal_uInt32 nCount = ::comphelper::getINT32(xSet->getPropertyValue(FM_PROP_ROWCOUNT));

            // naechste position festellen
            sal_Bool bLeft = xCursor->isLast() && nCount > 1;
            sal_Bool bRight= !xCursor->isLast();
            sal_Bool bSuccess = sal_False;
            try
            {
                // ask for confirmation
                Reference< ::com::sun::star::form::XConfirmDeleteListener >  xConfirm(GetImpl()->getActiveController(),UNO_QUERY);
                if (xConfirm.is())
                {
                    ::com::sun::star::sdb::RowChangeEvent aEvent;
                    aEvent.Source = Reference< XInterface > (xCursor, UNO_QUERY);
                    aEvent.Action = ::com::sun::star::sdb::RowChangeAction::DELETE;
                    aEvent.Rows = 1;
                    bSuccess = xConfirm->confirmDelete(aEvent);
                }

                // das Ding loeschen
                if (bSuccess)
                    xUpdateCursor->deleteRow();
            }
            catch(Exception&)
            {
                bSuccess = sal_False;
            }

            if (bSuccess)
            {
                if (bLeft || bRight)
                    xCursor->relative(bRight ? 1 : -1);
                else
                {
                    sal_Bool bCanInsert = ::dbtools::canInsert(xSet);
                    // kann noch ein Datensatz eingefuegt weden
                    try
                    {
                        if (bCanInsert)
                            xUpdateCursor->moveToInsertRow();
                        else
                            // Datensatz bewegen um Stati neu zu setzen
                            xCursor->first();
                    }
                    catch(Exception&)
                    {
                        DBG_ERROR("FmFormShell::Execute : couldn't position on the next record !");
                    }
                }
            }
            rReq.Done();
        }   break;
        case SID_FM_RECORD_ABSOLUTE:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            sal_Int32 nRecord = -1;

            Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(GetImpl()->getActiveForm(), UNO_QUERY);

            if ( pArgs )
            {
                const SfxPoolItem* pItem;
                if( (pArgs->GetItemState(SID_FM_RECORD_ABSOLUTE, sal_True, &pItem)) == SFX_ITEM_SET )
                {
                    nRecord = Max((((const FmFormInfoItem*)pItem)->GetInfo().Pos), (sal_Int32)0);
                }
            }
            else
            {
                FmInputRecordNoDialog dlg(NULL);
                dlg.SetValue(xCursor->getRow());
                if (dlg.Execute() == RET_OK)
                {
                    nRecord = dlg.GetValue() - 1;
                }
            }

            if (nRecord != -1)
            {
                Reference< ::com::sun::star::beans::XPropertySet >     xSet    (GetImpl()->getActiveForm(), UNO_QUERY);
                sal_Bool    bFinal      = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ROWCOUNTFINAL));
                sal_Int32  nRecordCount= ::comphelper::getINT32(xSet->getPropertyValue(FM_PROP_ROWCOUNT));

                if (bFinal && (sal_uInt32)nRecord >= nRecordCount)
                {
                    Sound::Beep();
                    rReq.Done();
                    break;
                }
                if (GetImpl()->SaveModified(GetImpl()->getNavController()))
                    DO_SAFE( xCursor->absolute(nRecord); );
            }

            rReq.Done();
        }   break;
        case SID_FM_RECORD_SAVE:
        {
            if (!GetImpl()->CommitCurrent(GetImpl()->getActiveController()))
            {
                rReq.Done();
                break;
            }

            Reference< ::com::sun::star::beans::XPropertySet >  xSet(GetImpl()->getActiveForm(), UNO_QUERY);
            sal_Bool bIsInserting  = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW));

            Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(GetImpl()->getActiveForm(), UNO_QUERY);
            Reference< ::com::sun::star::sdbc::XResultSetUpdate >  xUpdateCursor(GetImpl()->getActiveForm(), UNO_QUERY);
            sal_Bool bSuccess = sal_True;
            try
            {
                if (bIsInserting)
                    xUpdateCursor->insertRow();
                else
                    xUpdateCursor->updateRow();
            }
            catch(Exception&)
            {
                bSuccess = sal_False;
            }


            if (bSuccess && bIsInserting)
            {
                DO_SAFE( xCursor->last(); );
            }

            rReq.Done();
        }   break;
        case SID_FM_RECORD_UNDO:
        {
            Reference< ::com::sun::star::sdbc::XResultSetUpdate >  xUpdateCursor(GetImpl()->getActiveForm(), UNO_QUERY);
            Reference< ::com::sun::star::beans::XPropertySet >  xSet(GetImpl()->getActiveForm(), UNO_QUERY);
            sal_Bool bInserting = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW));
            if (!bInserting)
                xUpdateCursor->cancelRowUpdates();


//          GetImpl()->ResetCurrent(GetImpl()->getActiveController());
            GetImpl()->ResetAll(Reference< ::com::sun::star::form::XForm > (GetImpl()->getActiveController()->getModel(), UNO_QUERY));

            if (bInserting)                 // wieder in den EinfuegeModus
                DO_SAFE( xUpdateCursor->moveToInsertRow(); );

            GetImpl()->m_bActiveModified = sal_False;
            GetViewShell()->GetViewFrame()->GetBindings().Invalidate(DatabaseSlotMap);
            rReq.Done();
        }   break;
        case SID_FM_REMOVE_FILTER_SORT:
        {
            if (GetImpl()->SaveModified(GetImpl()->getActiveController()))
            {
                // gleichzeitiges Zuruecksetzen der Filter- und der Sort-Eigenschaft
                Reference< ::com::sun::star::beans::XMultiPropertySet >  xProperties(GetImpl()->getActiveForm(), UNO_QUERY);
                if (xProperties.is())
                {
                    ::rtl::OUString ustrNames[2] = { FM_PROP_FILTER_CRITERIA, FM_PROP_SORT };
                    Any  anyValues[2] = { makeAny(::rtl::OUString()),
                        makeAny(::rtl::OUString()) };
                    Sequence< ::rtl::OUString> seqNames(ustrNames, 2);
                    Sequence< Any> seqValues(anyValues, 2);

                    FormShellWaitObject aWO(this);
                    xProperties->setPropertyValues(seqNames, seqValues);

                    try
                    {
                        Reference< ::com::sun::star::form::XLoadable >  xReload(xProperties, UNO_QUERY);
                        xReload->reload();
                    }
                    catch(::com::sun::star::sdbc::SQLException e)
                    {
                        e;
                        DBG_ERROR("FmFormShell::Execute(REMOVE_FILTER_SORT) : catched an SQL exeption !");
                    }

                }
            }
            rReq.Done();
        }   break;
        case SID_FM_SORTDOWN:
            bSortUp = sal_False;
        case SID_FM_SORTUP:
        {
            Reference< ::com::sun::star::sdb::XSQLQueryComposer >  xParser = GetImpl()->getParser();
            Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(GetImpl()->getActiveForm(), UNO_QUERY);
            Reference< ::com::sun::star::form::XFormController >  xFormCtrler = GetImpl()->getActiveController();

            Reference< ::com::sun::star::awt::XControl >  xControl = xFormCtrler->getCurrentControl();
            if (GetImpl()->SaveModified(xFormCtrler) && xControl.is() && xCursor.is())
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xField = GetImpl()->GetBoundField(xControl, GetImpl()->getActiveForm());
                if (xField.is())
                {
                    Reference< ::com::sun::star::beans::XPropertySet >  xFormSet(GetImpl()->getActiveForm(), UNO_QUERY);
                    ::rtl::OUString sOriginalSort;
                    try { sOriginalSort = ::comphelper::getString(xFormSet->getPropertyValue(FM_PROP_SORT)); }
                    catch(Exception&) { }


                    // immer nur eine Sortierung
                    DO_SAFE_WITH_ERROR( xParser->setOrder(::rtl::OUString()), "FmFormShell::Execute(AUTOFILTER) : could not reset the parsers order !" );

                    sal_Bool bParserSuccess;
                    HANDLE_SQL_ERRORS(
                        xParser->appendOrderByColumn(xField, bSortUp),
                        bParserSuccess,
                        ::rtl::OUString(SVX_RES(RID_STR_COULDNOTSETORDER)),
                        "FmFormShell::Execute(SORTUP/DOWN) : unknown exception while updating the parser !"
                    )

                    if (bParserSuccess)
                    {
                        FormShellWaitObject aWO(this);
                        Reference< ::com::sun::star::form::XLoadable >  xReload(xFormSet, UNO_QUERY);
                        try
                        {
                            xFormSet->setPropertyValue(FM_PROP_SORT, makeAny(xParser->getOrder()));
                            xReload->reload();
                        }
                        catch(Exception&)
                        {
                        }


                        if (!isRowSetAlive(xFormSet))
                        {   // restore the original state
                            try
                            {
                                xParser->setOrder(sOriginalSort);
                                xFormSet->setPropertyValue(FM_PROP_SORT, makeAny(xParser->getOrder()));
                                xReload->reload();
                            }
                            catch(Exception&)
                            {
                            }

                        }
                    }
                }
            }
            rReq.Done();
        }   break;
        case SID_FM_FILTER_EXECUTE:
        {
            Reference< ::com::sun::star::form::XFormController >  xController(GetImpl()->getActiveController());
            if (xController.is() && !FmXFormShell::CommitCurrent(xController))
            {
                rReq.Done();
                break;
            }
        }   // continue !!!
        case SID_FM_FILTER_EXIT:
            GetImpl()->stopFiltering(SID_FM_FILTER_EXECUTE == nSlot);
            rReq.Done();
            break;
        case SID_FM_FILTER_START:
        {
            GetImpl()->startFiltering();
            rReq.Done();
        }   break;
        case SID_FM_ORDERCRIT:
        {
            Reference< ::com::sun::star::form::XFormController >    xFormCtrler = GetImpl()->getActiveController();
            Reference< ::com::sun::star::awt::XControl >            xControl = xFormCtrler->getCurrentControl();

            Reference< ::com::sun::star::sdb::XSQLQueryComposer >       xParser = GetImpl()->getParser();
//          Reference< ::com::sun::star::data::XDatabaseDialogs >           xDlgs(xParser, UNO_QUERY);

            if (GetImpl()->SaveModified(xFormCtrler))
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xSet(GetImpl()->getActiveForm(), UNO_QUERY);
                Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(xSet, UNO_QUERY);
                sal_Bool bIsDeleted = xCursor.is() && xCursor->rowDeleted();

                Reference< ::com::sun::star::container::XNamed >  xField;
                if (bIsDeleted)
                {
                    Reference< ::com::sun::star::beans::XPropertySet >  xSet(GetImpl()->GetBoundField(xControl, GetImpl()->getActiveForm()), UNO_QUERY);
                    xField = Reference< ::com::sun::star::container::XNamed > (xSet, UNO_QUERY);
                    // auslesen der Searchflags
                    if (xSet.is())
                    {
                        if (!::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_SEARCHABLE)))
                            xField = NULL;
                    }
                }
                Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSup(xSet,UNO_QUERY);
                OSL_ENSURE(xSup.is(),"no columnssupplier!");
                Sequence<Any> aInit(3);
                aInit[0] <<= xParser;
                aInit[1] <<= xParser;
                aInit[2] <<= xSup->getColumns();


                Reference< com::sun::star::ui::XExecutableDialog> xDlg(
                    ::comphelper::getProcessServiceFactory()->createInstanceWithArguments(::rtl::OUString::createFromAscii("com.sun.star.sdb.OrderDialog"),aInit),UNO_QUERY);

                if(xDlg.is())
                    xDlg->execute();

                Reference< ::com::sun::star::beans::XPropertySet >  xFormSet(GetImpl()->getActiveForm(), UNO_QUERY);
                FormShellWaitObject aWO(this);
                try
                {
                    xFormSet->setPropertyValue(FM_PROP_SORT, makeAny(xParser->getOrder()));
                    Reference< ::com::sun::star::form::XLoadable >  xReload(xFormSet, UNO_QUERY);
                    xReload->reload();

                }
                catch(Exception&)
                {
                    DBG_ERROR("Exception occured!");
                }
            }
            rReq.Done();
        }   break;
        case SID_FM_AUTOFILTER:
        {
            Reference< ::com::sun::star::form::XFormController >    xFormCtrler = GetImpl()->getActiveController();
            Reference< ::com::sun::star::awt::XControl >            xControl = xFormCtrler->getCurrentControl();

            Reference< ::com::sun::star::sdb::XSQLQueryComposer >   xParser = GetImpl()->getParser();
            Reference< ::com::sun::star::sdbc::XResultSet >         xCursor(GetImpl()->getActiveForm(), UNO_QUERY);

            if (GetImpl()->SaveModified(xFormCtrler) && xControl.is() && xCursor.is())
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xField = GetImpl()->GetBoundField(xControl, GetImpl()->getActiveForm());
                if (xField.is())
                {
                    Reference< ::com::sun::star::beans::XPropertySet >  xActiveSet(GetImpl()->getActiveForm(), UNO_QUERY);
                    ::rtl::OUString sOriginalFilter = ::comphelper::getString(xActiveSet->getPropertyValue(FM_PROP_FILTER_CRITERIA));
                    sal_Bool bApplied = ::comphelper::getBOOL(xActiveSet->getPropertyValue(FM_PROP_APPLYFILTER));

                    // do we have a filter but not applied, then we have to overwrite it, else append one
                    if (!bApplied)
                    {
                        DO_SAFE_WITH_ERROR( xParser->setFilter(::rtl::OUString()), "FmFormShell::Execute(AUTOFILTER) : could not reset the parsers filter !" );
                    }

                    sal_Bool bParserSuccess;
                    HANDLE_SQL_ERRORS(
                        xParser->appendFilterByColumn(xField),
                        bParserSuccess,
                        ::rtl::OUString(SVX_RES(RID_STR_COULDNOTSETFILTER)),
                        "FmFormShell::Execute(AUTOFILTER) : could not append the filter to my parser !"
                    )
                    if (bParserSuccess)
                    {
                        Reference< ::com::sun::star::form::XLoadable >  xReload(xActiveSet, UNO_QUERY);
                        try
                        {
                            FormShellWaitObject aWO(this);
                            xActiveSet->setPropertyValue(FM_PROP_FILTER_CRITERIA, makeAny(xParser->getFilter()));
                            sal_Bool bB(sal_True);
                            xActiveSet->setPropertyValue(FM_PROP_APPLYFILTER, Any(&bB,getBooleanCppuType()));

                            xReload->reload();
                        }
                        catch(Exception&)
                        {
                        }


                        if (!isRowSetAlive(xActiveSet))
                        {   // restore the original state
                            try
                            {
                                xParser->setOrder(sOriginalFilter);
                                xActiveSet->setPropertyValue(FM_PROP_APPLYFILTER, Any(&bApplied,getBooleanCppuType()));
                                xActiveSet->setPropertyValue(FM_PROP_FILTER_CRITERIA, makeAny(xParser->getFilter()));
                                xReload->reload();
                            }
                            catch(Exception&)
                            {
                            }

                        }
                    }
                }
            }
            rReq.Done();
        }   break;
        case SID_FM_REFRESH:
            if (GetImpl()->SaveModified(GetImpl()->getActiveController()))
            {
                FormShellWaitObject aWO(this);
                Reference< ::com::sun::star::form::XLoadable >  xReload(GetImpl()->getActiveForm(), UNO_QUERY);
                xReload->reload();
            }
            rReq.Done();
            break;
        case SID_FM_FORM_FILTERED: // toggle the item
            if (GetImpl()->SaveModified(GetImpl()->getActiveController()))
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xActiveSet(GetImpl()->getActiveForm(), UNO_QUERY);
                sal_Bool bApplied = ::comphelper::getBOOL(xActiveSet->getPropertyValue(FM_PROP_APPLYFILTER));

                sal_Bool bB(!bApplied);
                xActiveSet->setPropertyValue(FM_PROP_APPLYFILTER, Any(&bB,getBooleanCppuType()));

                FormShellWaitObject aWO(this);
                Reference< ::com::sun::star::form::XLoadable >  xReload(GetImpl()->getActiveForm(), UNO_QUERY);
                xReload->reload();
            }
            rReq.Done();
            break;
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
            case SID_FM_USE_WIZARDS:
                if (!m_bDesignMode || !GetFormModel())
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem(nWhich, GetImpl()->GetWizardUsing() ) );
                break;
            case SID_FM_AUTOCONTROLFOCUS:
                if (!m_bDesignMode || !GetFormModel())
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem(nWhich, GetFormModel()->GetAutoControlFocus() ) );
                break;
            case SID_FM_OPEN_READONLY:
                if (!m_bDesignMode || !GetFormModel())
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem(nWhich, GetFormModel()->GetOpenInDesignMode() ) );
                break;
            case SID_FM_RADIOBUTTON:
            case SID_FM_COMBOBOX:
            case SID_FM_URLBUTTON:
            case SID_FM_GROUPBOX:
            case SID_FM_CHECKBOX:
            case SID_FM_PUSHBUTTON:
            case SID_FM_FIXEDTEXT:
            case SID_FM_LISTBOX:
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
                if (!m_bDesignMode)
                    rSet.DisableItem( nWhich );
                else
                {
                    sal_Bool bLayerLocked = sal_False;
                    if (m_pFormView)
                    {
                        // Ist der ::com::sun::star::drawing::Layer gelocked, so müssen die Slots disabled werden. #36897
                        SdrPageView* pPV = m_pFormView->GetPageViewPvNum(0);
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
            {
                if ( GetViewShell()->GetViewFrame()->KnowsChildWindow(nWhich) )
                    rSet.Put( SfxBoolItem( nWhich, GetViewShell()->GetViewFrame()->HasChildWindow(nWhich)) );
                else
                    rSet.DisableItem(nWhich);
            }   break;

            case SID_FM_SHOW_PROPERTY_BROWSER:
            {
                rSet.Put(SfxBoolItem(GetImpl()->IsPropBrwOpen()));
            } break;

            case SID_FM_CTL_PROPERTIES:
            {
                // der Impl eventuell die Moeglichjkeit geben, ihre an der aktuellen MarkList ausgerichteten Objekte
                // auf den neuesten Stand zu bringen
                if (GetImpl()->IsSelectionUpdatePending())
                    GetImpl()->ForceUpdateSelection(sal_False);

                if (!m_pFormView || !m_bDesignMode || !GetImpl()->getCurControl().is())
                    rSet.DisableItem( nWhich );
                else
                {
                    sal_Bool bChecked  = GetImpl()->IsPropBrwOpen() && (GetImpl()->getSelObject() == GetImpl()->getCurControl());
                    rSet.Put(SfxBoolItem(nWhich, bChecked));
                }
            }   break;

            case SID_FM_PROPERTIES:
            {
                // der Impl eventuell die Moeglichjkeit geben, ihre an der aktuellen MarkList ausgerichteten Objekte
                // auf den neuesten Stand zu bringen
                if (GetImpl()->IsSelectionUpdatePending())
                    GetImpl()->ForceUpdateSelection(sal_False);

                if (!m_pFormView || !m_bDesignMode || !GetImpl()->getCurForm().is())
                    rSet.DisableItem( nWhich );
                else
                {
                    // aktuelle form
                    Reference< ::com::sun::star::form::XForm >  xForm(GetImpl()->getSelObject(), UNO_QUERY);
                    sal_Bool bChecked = GetImpl()->IsPropBrwOpen() && xForm.is();
                    rSet.Put(SfxBoolItem(nWhich, bChecked));
                }
            }   break;
            case SID_FM_TAB_DIALOG:
                // der Impl eventuell die Moeglichjkeit geben, ihre an der aktuellen MarkList ausgerichteten Objekte
                // auf den neuesten Stand zu bringen
                if (GetImpl()->IsSelectionUpdatePending())
                    GetImpl()->ForceUpdateSelection(sal_False);

                if (!m_pFormView || !m_bDesignMode || !GetImpl()->getCurForm().is() )
                    rSet.DisableItem( nWhich );
                break;
            case SID_FM_CONFIG:
                rSet.Put(SfxUInt16Item(nWhich, m_nLastSlot));
                break;
            case SID_FM_DESIGN_MODE:
                if (!m_pFormView)
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
            case SID_FM_VIEW_AS_GRID:
                GetFormState(rSet,nWhich);
                break;

            case SID_FM_FILTER_EXECUTE:
            case SID_FM_FILTER_EXIT:
                break;
            /*case SID_FM_FILTER_REMOVE:
            {
                if (!m_pFormView || !GetImpl()->isInFilterMode())
                    rSet.DisableItem( nWhich );
            }   break;*/
            case SID_FM_CHANGECONTROLTYPE:
            {
                if (!m_pFormView || !m_bDesignMode || !GetImpl()->getCurControl().is())
                    rSet.DisableItem( nWhich );
                else
                {
                    Reference< ::com::sun::star::form::XFormComponent >  xSelAsComp(GetImpl()->getCurControl(), UNO_QUERY);
                    if (!xSelAsComp.is())
                        rSet.DisableItem( nWhich );
                }
            } break;

            case SID_FM_CONVERTTO_EDIT          :
            case SID_FM_CONVERTTO_BUTTON            :
            case SID_FM_CONVERTTO_FIXEDTEXT     :
            case SID_FM_CONVERTTO_LISTBOX       :
            case SID_FM_CONVERTTO_CHECKBOX      :
            case SID_FM_CONVERTTO_RADIOBUTTON   :
            case SID_FM_CONVERTTO_GROUPBOX      :
            case SID_FM_CONVERTTO_COMBOBOX      :
            case SID_FM_CONVERTTO_GRID          :
            case SID_FM_CONVERTTO_IMAGEBUTTON   :
            case SID_FM_CONVERTTO_FILECONTROL   :
            case SID_FM_CONVERTTO_DATE          :
            case SID_FM_CONVERTTO_TIME          :
            case SID_FM_CONVERTTO_NUMERIC       :
            case SID_FM_CONVERTTO_CURRENCY      :
            case SID_FM_CONVERTTO_PATTERN       :
            case SID_FM_CONVERTTO_IMAGECONTROL  :
            case SID_FM_CONVERTTO_FORMATTED     :
            {
                if (!m_pFormView || !m_bDesignMode || !GetImpl()->getCurControl().is())
                    rSet.DisableItem( nWhich );
                else
                {
                    Reference< ::com::sun::star::beans::XPropertySet >  xSelProps(GetImpl()->getCurControl(), UNO_QUERY);
                    if (!xSelProps.is() || !GetImpl()->IsConversionPossible(xSelProps, nWhich))
                        // it isn't a form component
                        rSet.DisableItem( nWhich );
                    else
                        rSet.Put(SfxBoolItem(nWhich, sal_False));
                        // just to have a defined state (available and not checked)
                }
            } break;
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
        ||  (   GetImpl()->HasPendingCursorAction(GetImpl()->getNavController())
            &&  (SID_FM_RECORD_TOTAL != nWhich)
            )
        )
        rSet.DisableItem(nWhich);
    else
    {
        sal_Bool    bEnable = sal_False;
        switch (nWhich)
        {
            case SID_FM_VIEW_AS_GRID:
                if (GetImpl()->m_xAttachedFrame.is() && GetImpl()->getNavController().is())
                {
                    bEnable = sal_True;
                    sal_Bool bDisplayingCurrent = GetImpl()->getInternalForm(Reference< ::com::sun::star::form::XForm > (GetImpl()->getNavController()->getModel(), UNO_QUERY)) == GetImpl()->m_xExternalDisplayedForm;
                    rSet.Put(SfxBoolItem(nWhich, bDisplayingCurrent));
                }
                break;

            case SID_FM_SEARCH:
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xNavSet(GetImpl()->getActiveForm(), UNO_QUERY);
                sal_Int32 nCount = ::comphelper::getINT32(xNavSet->getPropertyValue(FM_PROP_ROWCOUNT));
                bEnable = nCount != 0;
            }   break;
            case SID_FM_RECORD_FIRST:
            case SID_FM_RECORD_PREV:
                bEnable = GetImpl()->canNavigate() && GetImpl()->CanMoveLeft(GetImpl()->getNavController());
                break;
            case SID_FM_RECORD_NEXT:
                if (GetImpl()->canNavigate())
                {
                    if (GetImpl()->CanMoveRight(GetImpl()->getNavController()))
                        bEnable = sal_True;
                    else if (GetImpl()->getNavController() == GetImpl()->getActiveController() &&
                             GetImpl()->getActiveForm().is())
                    {
                        Reference< ::com::sun::star::beans::XPropertySet >  xActiveSet(GetImpl()->getActiveForm(), UNO_QUERY);
                        sal_Bool bIsNew = ::comphelper::getBOOL(xActiveSet->getPropertyValue(FM_PROP_ISNEW));
                        bEnable = bIsNew && GetImpl()->isActiveModified();
                    }
                }
                break;
            case SID_FM_RECORD_LAST:
            {
                if (GetImpl()->canNavigate())
                {
                    Reference< ::com::sun::star::beans::XPropertySet >  xNavSet(GetImpl()->getNavController()->getModel(), UNO_QUERY);
                    Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(xNavSet, UNO_QUERY);
                    sal_Int32  nCount       = ::comphelper::getINT32(xNavSet->getPropertyValue(FM_PROP_ROWCOUNT));
                    sal_Bool bIsNew = ::comphelper::getBOOL(xNavSet->getPropertyValue(FM_PROP_ISNEW));
                    bEnable = nCount && (!xCursor->isLast() || bIsNew);
                }
            }   break;
            case SID_FM_RECORD_NEW:
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xActiveSet(GetImpl()->getActiveForm(), UNO_QUERY);
                bEnable = ::dbtools::canInsert(xActiveSet);
                // if we are inserting we can move to the next row if the current record is modified
                bEnable  = ::comphelper::getBOOL(xActiveSet->getPropertyValue(FM_PROP_ISNEW))
                    ? GetImpl()->isActiveModified() || ::comphelper::getBOOL(xActiveSet->getPropertyValue(FM_PROP_ISMODIFIED))
                    : ::dbtools::canInsert(xActiveSet);
            }   break;
            case SID_FM_RECORD_DELETE:
            {
                // already deleted ?
                Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(GetImpl()->getActiveForm(), UNO_QUERY);
                sal_Bool bIsDeleted = !xCursor.is() || xCursor->rowDeleted();
                if (!bIsDeleted)
                {
                    Reference< ::com::sun::star::beans::XPropertySet >  xActiveSet(xCursor, UNO_QUERY);
                    // allowed to delete the row ?
                    bEnable = !::comphelper::getBOOL(xActiveSet->getPropertyValue(FM_PROP_ISNEW)) && ::dbtools::canDelete(xActiveSet);
                }
                else
                    bEnable = sal_False;
            }   break;
            case SID_FM_RECORD_ABSOLUTE:
            {
                if (GetImpl()->canNavigate())
                {
                    Reference< ::com::sun::star::beans::XPropertySet >  xNavSet(GetImpl()->getNavController()->getModel(), UNO_QUERY);
                    Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(xNavSet, UNO_QUERY);

                    sal_Int32  nPos      = xCursor->getRow();
                    sal_Bool bIsNew     = ::comphelper::getBOOL(xNavSet->getPropertyValue(FM_PROP_ISNEW));
                    sal_Int32 nCount     = ::comphelper::getINT32(xNavSet->getPropertyValue(FM_PROP_ROWCOUNT));
                    sal_Bool   bTotal    = ::comphelper::getBOOL(xNavSet->getPropertyValue(FM_PROP_ROWCOUNTFINAL));
                    if (nPos >= 0 || bIsNew)
                    {
                        if (bTotal)
                        {
                            // Sonderfall, es koennen keine Datensaetze eingefuegt werden
                            // und es gibt keinen Datensatz -> dann
                            if (nCount == 0 && !::dbtools::canInsert(xNavSet))
                            {
                                bEnable = sal_False;
                            }
                            else
                            {
                                if (bIsNew)
                                    nPos = ++nCount;
                                rSet.Put(FmFormInfoItem(nWhich, FmFormInfo(nPos, nCount, sal_False)));
                                bEnable = sal_True;
                            }
                        }
                        else
                        {
                            rSet.Put(FmFormInfoItem(nWhich, FmFormInfo(nPos, -1, sal_False)));
                            bEnable = sal_True;
                        }
                    }
                }
            }   break;
            case SID_FM_RECORD_TOTAL:
            {
                String aValue;
                if (!GetImpl()->canNavigate())
                {
                    aValue = String();
                    bEnable = sal_False;
                }
                else
                {
                    bEnable = sal_True;
                    Reference< ::com::sun::star::beans::XPropertySet >  xNavSet(GetImpl()->getNavController()->getModel(), UNO_QUERY);
                    sal_Bool bIsNew     = ::comphelper::getBOOL(xNavSet->getPropertyValue(FM_PROP_ISNEW));
                    sal_Int32  nCount   = ::comphelper::getINT32(xNavSet->getPropertyValue(FM_PROP_ROWCOUNT));

                    if (bIsNew)
                        nCount++;

                    aValue = String::CreateFromInt32(sal_uInt32(nCount));
                }
                rSet.Put(SfxStringItem(nWhich, aValue));
            }   break;
            case SID_FM_REMOVE_FILTER_SORT:
                if (GetImpl()->isParsable() && GetImpl()->hasFilter())
                {
                    Reference< ::com::sun::star::beans::XPropertySet >  xSet(GetImpl()->getActiveForm(), UNO_QUERY);
                    bEnable = !::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_INSERTONLY));
                }
                break;
            case SID_FM_SORTUP:
            case SID_FM_SORTDOWN:
            case SID_FM_AUTOFILTER:
                if (GetImpl()->isParsable())
                {
                    Reference< ::com::sun::star::beans::XPropertySet >  xActiveSet(GetImpl()->getActiveForm(), UNO_QUERY);
                    sal_Bool bInsertOnly = ::comphelper::getBOOL(xActiveSet->getPropertyValue(FM_PROP_INSERTONLY));

                    Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(xActiveSet, UNO_QUERY);
                    sal_Bool bIsDeleted = xCursor.is() && xCursor->rowDeleted();

                    if (!bIsDeleted && !bInsertOnly)
                    {
                        Reference< ::com::sun::star::form::XFormController >  xFormCtrler(GetImpl()->getActiveController());
                        Reference< ::com::sun::star::awt::XControl >  xControl(xFormCtrler->getCurrentControl());

                        Reference< ::com::sun::star::beans::XPropertySet >  xSet = GetImpl()->GetBoundField(xControl, GetImpl()->getActiveForm());
                        // auslesen der Searchflags
                        if (xSet.is())
                        {
                            bEnable = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_SEARCHABLE));
                        }
                    }
                }   break;
            case SID_FM_ORDERCRIT:
            case SID_FM_FILTER_START:
                if (GetImpl()->isParsable())
                {
                    Reference< ::com::sun::star::beans::XPropertySet >  xSet(GetImpl()->getActiveForm(), UNO_QUERY);
                    bEnable = !::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_INSERTONLY));
                }
                break;
            case SID_FM_REFRESH:
            {
                Reference< ::com::sun::star::sdbc::XRowSet >            xRowSet(GetImpl()->getActiveForm(), UNO_QUERY);
                Reference< ::com::sun::star::beans::XPropertySet >      xSet(GetImpl()->getActiveForm(), UNO_QUERY);
                bEnable = ::dbtools::getConnection(xRowSet).is() && ::comphelper::getString(xSet->getPropertyValue(FM_PROP_ACTIVECOMMAND)).len();
            }   break;
            case SID_FM_FORM_FILTERED:
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xActiveSet(GetImpl()->getActiveForm(), UNO_QUERY);
                ::rtl::OUString aFilter = ::comphelper::getString(xActiveSet->getPropertyValue(FM_PROP_FILTER_CRITERIA));
                if (aFilter.len())
                {
                    rSet.Put(SfxBoolItem(nWhich, ::comphelper::getBOOL(xActiveSet->getPropertyValue(FM_PROP_APPLYFILTER))));
                    bEnable = !::comphelper::getBOOL(xActiveSet->getPropertyValue(FM_PROP_INSERTONLY));
                }
            }   break;
            case SID_FM_RECORD_SAVE:
            case SID_FM_RECORD_UNDO:
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xActiveSet(GetImpl()->getActiveForm(), UNO_QUERY);
                sal_Bool bIsModified = ::comphelper::getBOOL(xActiveSet->getPropertyValue(FM_PROP_ISMODIFIED));
                bEnable = bIsModified || GetImpl()->isActiveModified();
            }   break;
        }
        if (!bEnable)
            rSet.DisableItem(nWhich);
    }
}

//------------------------------------------------------------------------
FmFormPage* FmFormShell::GetCurPage() const
{
    FmFormPage* pP = NULL;
    if (m_pFormView && m_pFormView->GetPageViewPvNum(0))
        pP = PTR_CAST(FmFormPage,m_pFormView->GetPageViewPvNum(0)->GetPage());
    return pP;
}

//------------------------------------------------------------------------
void FmFormShell::SetView(FmFormView* pView)
{
    if (m_pFormView)
    {
        m_pFormView->SetFormShell( NULL );
        GetImpl()->ResetForms();
        m_pFormModel = NULL;
    }
    if (pView)
    {
        m_pFormView = pView;
        m_pFormView->SetFormShell( this );
        m_pFormModel = (FmFormModel*)m_pFormView->GetModel();

        // We activate our view if we are activated ourself, but sometimes the Activate precedes the SetView.
        // But here we know both the view and our activation state so we at least are able to pass the latter
        // to the former.
        // FS - 30.06.99 - 67308
        if (IsActive() && m_pFormView->GetImpl() && !m_pFormView->IsDesignMode())
            m_pFormView->GetImpl()->Activate(m_pFormView->GetPageViewPvNum(0));

        SetDesignMode(pView->IsDesignMode());
    }
    else
    {
        m_pFormView = NULL;
    }
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
    // activate our view if we are activated ourself
    // FS - 30.06.99 - 67308
    if (m_pFormView && m_pFormView->GetImpl() && !m_pFormView->IsDesignMode())
    {
        SdrPageView* pCurPageView = m_pFormView->GetPageViewPvNum(0);
        m_pFormView->GetImpl()->Activate(pCurPageView, sal_True);
    }
}

//------------------------------------------------------------------------
void FmFormShell::Deactivate(sal_Bool bMDI)
{
    SfxShell::Deactivate(bMDI);
    // deactivate our view if we are deactivated ourself
    // FS - 30.06.99 - 67308
    if (m_pFormView && m_pFormView->GetImpl() && !m_pFormView->IsDesignMode())
        m_pFormView->GetImpl()->Deactivate(NULL, FALSE);
}


