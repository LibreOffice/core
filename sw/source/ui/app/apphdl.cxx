/*************************************************************************
 *
 *  $RCSfile: apphdl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-06 13:31:28 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_UNDOOPT_HXX
#include <svtools/undoopt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXEVENT_HXX //autogen
#include <sfx2/event.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXSTBMGR_HXX //autogen
#include <sfx2/stbmgr.hxx>
#endif
#ifndef _SFXISETHINT_HXX //autogen
#include <svtools/isethint.hxx>
#endif
#ifndef _SVX_DLG_HYPERLINK_HXX //autogen
#include <offmgr/hyprlink.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _OFA_OSPLCFG_HXX //autogen
#include <offmgr/osplcfg.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_ADRITEM_HXX //autogen
#include <svx/adritem.hxx>
#endif
#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif
#ifndef _DATAEX_HXX //autogen
#include <dataex.hxx>
#endif
#ifndef _VCL_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _VCL_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVX_INSCTRL_HXX //autogen
#include <svx/insctrl.hxx>
#endif
#ifndef _SVX_SELCTRL_HXX //autogen
#include <svx/selctrl.hxx>
#endif
#ifndef _NAVICFG_HXX
#include <navicfg.hxx>
#endif

#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>          // Funktion-Ids
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _WIZZARD_HXX
#include <wizzard.hxx>      // Wizzards
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _GLOSLST_HXX
#include <gloslst.hxx>      // SwGlossaryList
#endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>      // SwGlossaryList
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CFGITEMS_HXX
#include <cfgitems.hxx>
#endif
#ifndef _PRTOPT_HXX
#include <prtopt.hxx>
#endif
#ifndef _SRCVCFG_HXX
#include <srcvcfg.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#ifndef _GLOBALS_H
#include <globals.h>        // globale Konstanten z.B.
#endif
#ifndef _CFGID_H
#include <cfgid.h>
#endif
#ifndef _APP_HRC
#include <app.hrc>
#endif
#ifndef _USRPREF_HXX //autogen
#include <usrpref.hxx>
#endif
#ifndef _PRTOPT_HXX //autogen
#include <prtopt.hxx>
#endif
#ifndef _MODOPT_HXX //autogen
#include <modcfg.hxx>
#endif
#ifndef _FONTCFG_HXX //autogen
#include <fontcfg.hxx>
#endif
#ifndef _BARCFG_HXX
#include <barcfg.hxx>
#endif
#ifndef _UINUMS_HXX //autogen
#include <uinums.hxx>
#endif

#ifndef _LINGU_LNGPROPS_HHX_
#include <lingu/lngprops.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XVETOABLECHANGELISTENER_HPP_
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATECHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATECHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyStateChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif

using namespace ::com::sun::star;

#define C2S(cChar) String::CreateFromAscii(cChar)
/*--------------------------------------------------------------------
    Beschreibung: Slotmaps fuer Methoden der Applikation
 --------------------------------------------------------------------*/


// hier werden die SlotID's included
// siehe Idl-File
//
#define SwModule
#define ViewSettings
#define WebViewSettings
#define PrintSettings
#define _ExecAddress ExecOther
#define _StateAddress StateOther
#include "itemdef.hxx"
#include <svx/svxslots.hxx>
#include "swslots.hxx"


SFX_IMPL_INTERFACE( SwModule, SfxModule, SW_RES(RID_SW_NAME) )
{
    SFX_CHILDWINDOW_REGISTRATION(SvxHyperlinkDlgWrapper::GetChildWindowId());
    SFX_STATUSBAR_REGISTRATION(SW_RES(CFG_STATUSBAR));
}

/*------------------------------------------------------------------------
 Beschreibung:  Funktionen Ansicht
------------------------------------------------------------------------*/

void lcl_SetViewMetaChars( SwViewOption& rVOpt, sal_Bool bOn)
{
    rVOpt.SetViewMetaChars( bOn );
    if(bOn && !(rVOpt.IsParagraph()     ||
            rVOpt.IsTab()       ||
            rVOpt.IsLineBreak() ||
            rVOpt.IsBlank()))
    {
        rVOpt.SetParagraph(bOn);
        rVOpt.SetTab(bOn);
        rVOpt.SetLineBreak(bOn);
        rVOpt.SetBlank(bOn);
    }
}


sal_Bool lcl_IsViewMarks( const SwViewOption& rVOpt )
{
    return  rVOpt.IsHardBlank() &&
            rVOpt.IsSoftHyph() &&
            rVOpt.IsTox() &&
            rVOpt.IsFootNote() &&
            rVOpt.IsField();
}


void lcl_SetViewMarks(SwViewOption& rVOpt, sal_Bool bOn )
{
    rVOpt.SetHardBlank(bOn);
    rVOpt.SetSoftHyph(bOn);
    rVOpt.SetTox(bOn);
    rVOpt.SetFootNote(bOn);
    rVOpt.SetField(bOn);
    rVOpt.SetRef(bOn);
}

/*--------------------------------------------------------------------
    Beschreibung:   State mit CheckMark fuer ViewOptions
 --------------------------------------------------------------------*/


void SwModule::StateViewOptions(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    SfxBoolItem aBool;
    const SwViewOption* pOpt = 0;
    SwView* pActView = ::GetActiveView();
    SwDoc *pDoc = 0;
    if(pActView)
    {
        pOpt = pActView->GetWrtShell().GetViewOptions();
        pDoc = pActView->GetDocShell()->GetDoc();
    }

    while(nWhich)
    {
        if(pActView)
        {
            sal_Bool bReadonly = pActView->GetDocShell()->IsReadOnly();
            sal_Bool bBrowse = pDoc ? pDoc->IsBrowseMode() : sal_False;
            if ( bReadonly && nWhich != FN_VIEW_GRAPHIC )
            {
                rSet.DisableItem(nWhich);
                nWhich = 0;
            }
            switch(nWhich)
            {
                case FN_RULER:
                    aBool.SetValue( pActView->StatTab() );  break;
                case FN_VIEW_BOUNDS:
                    aBool.SetValue( pOpt->IsSubsLines() ); break;
                case FN_VIEW_GRAPHIC:
                    aBool.SetValue( !pOpt->IsGraphic() ); break;
                case FN_VIEW_FIELDS:
                    aBool.SetValue( pOpt->IsField() ); break;
                case FN_VIEW_FIELDNAME:
                    aBool.SetValue( pOpt->IsFldName() ); break;
                case FN_VIEW_MARKS:
                    aBool.SetValue( lcl_IsViewMarks(*pOpt) ); break;
                case FN_VIEW_META_CHARS:
                    aBool.SetValue( pOpt->IsViewMetaChars() ); break;
                case FN_VIEW_TABLEGRID:
                    aBool.SetValue( pOpt->IsSubsTable() ); break;
                case FN_VIEW_HIDDEN_PARA:
                    aBool.SetValue( pOpt->IsShowHiddenPara()); break;
                case SID_GRID_VISIBLE:
                    aBool.SetValue( pOpt->IsGridVisible() ); break;
                case SID_GRID_USE:
                    aBool.SetValue( pOpt->IsSnap() ); break;
                case SID_HELPLINES_MOVE:
                    aBool.SetValue( pOpt->IsCrossHair() ); break;
                case FN_VIEW_SMOOTH_SCROLL:
                    aBool.SetValue( pOpt->IsSmoothScroll()); break;
                case FN_VLINEAL:
                    aBool.SetValue( pActView->StatVLineal() ); break;
                case FN_HSCROLLBAR:
                    if(bBrowse)
                    {
                        rSet.DisableItem(nWhich);
                        nWhich = 0;
                    }
                    else
                        aBool.SetValue( pActView->StatHScrollbar() ); break;
                case FN_VSCROLLBAR:
                    aBool.SetValue( pActView->StatVScrollbar() ); break;
                case SID_AUTOSPELL_CHECK:
                    aBool.SetValue( pOpt->IsOnlineSpell() );
                break;
                case SID_AUTOSPELL_MARKOFF:
                    aBool.SetValue( pOpt->IsHideSpell() );
                break;
                case FN_SHADOWCURSOR:
                    if (pDoc == 0 || pDoc->IsBrowseMode())
                    {
                        rSet.DisableItem( nWhich );
                        nWhich = 0;
                    }
                    else
                        aBool.SetValue( pOpt->IsShadowCursor() );
                break;

            }
        }
        else
        {
            rSet.DisableItem( nWhich );
            nWhich = 0;
        }

        if( nWhich )
        {
            aBool.SetWhich( nWhich );
            rSet.Put( aBool );
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Andere States
 --------------------------------------------------------------------*/


void SwModule::StateOther(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    SwView* pActView = ::GetActiveView();
    sal_Bool bWebView = 0 != PTR_CAST(SwWebView, pActView);

    while(nWhich)
    {
        switch(nWhich)
        {
            case FN_BUSINESS_CARD:
            case FN_LABEL:
            case FN_ENVELOP:
            {
                sal_Bool bDisable = sal_False;
                SfxViewShell* pView = SfxViewShell::Current();
                if( !pView || (pView && !pView->ISA(SwView)) )
                    bDisable = sal_True;
                SwDocShell *pDocSh = (SwDocShell*) SfxObjectShell::Current();
                if ( bDisable ||
                    (pDocSh &&    (pDocSh->IsReadOnly() ||
                                  pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED)) )
                    rSet.DisableItem( nWhich );

            }
            break;
            case FN_EDIT_FORMULA:
                {
                    SwWrtShell* pSh = 0;
                    int nSelection = 0;
                    if( pActView )
                        pSh = &pActView->GetWrtShell();
                    if( pSh )
                        nSelection = pSh->GetSelectionType();

                    if( (pSh && pSh->HasSelection()) ||
                        !(nSelection & (SwWrtShell::SEL_TXT | SwWrtShell::SEL_TBL)))
                        rSet.DisableItem(nWhich);
                }
            break;
            case SID_ATTR_ADDRESS:
            {
                SvxAddressItem aAddress( *SFX_APP()->GetIniManager() );
                aAddress.SetWhich( nWhich );
                rSet.Put( aAddress );
            }
            break;
            case SID_ATTR_UNDO_COUNT:
                rSet.Put(SfxUInt16Item( SID_ATTR_UNDO_COUNT, SwEditShell::GetUndoActionCount()));
            break;
            case SID_ATTR_METRIC:
                rSet.Put( SfxUInt16Item( SID_ATTR_METRIC, ::GetDfltMetric(bWebView)));
            break;
            case FN_SET_MODOPT_TBLNUMFMT:
                rSet.Put( SfxBoolItem( nWhich, pModuleConfig->
                                            IsInsTblFormatNum( bWebView )));
            break;

            default:
                DBG_ERROR("::StateOther: default");
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Wizzards
 --------------------------------------------------------------------*/


void SwModule::ExecWizzard(SfxRequest & rReq)
{
    Wizzard( rReq.GetSlot() );
}

/*--------------------------------------------------------------------
    Beschreibung:   Einstellungen fuer den Bildschirm
 --------------------------------------------------------------------*/



void SwModule::ExecViewOptions(SfxRequest &rReq)
{
    SwViewOption* pOpt = 0;
    OfficeApplication* pOffApp = OFF_APP();
    sal_Bool bModified = sal_True;
    if ( GetView() )
    {
        pOpt = new SwViewOption( *GetView()->GetWrtShell().GetViewOptions() );
        bModified = GetView()->GetWrtShell().IsModified();
    }
    else
    {
        pOpt = new SwViewOption();
        DBG_ERROR("ExecViewOptions: ohne View - falsche UsrPref?")
        MakeUsrPref(*pOpt, sal_False);
    }


    int eState = STATE_TOGGLE;
    sal_Bool bSet = sal_False;

    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxPoolItem* pAttr;

    if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( nSlot , sal_False, &pAttr ))
    {
        bSet = ((SfxBoolItem*)pAttr)->GetValue();
        eState = bSet ? STATE_ON : STATE_OFF;
    }


    int bFlag = STATE_ON == eState;
    // fuer die (WebViewSettings
    enum WebDepend
    {
        DEPEND_NONE,
        DEPEND_NOWEB,
        DEPEND_WEB
    }eWebDepend = DEPEND_NONE;

    uno::Reference< beans::XPropertySet >  xLngProp( ::GetLinguPropertySet() );

    switch ( nSlot )
    {
        case FN_VIEW_GRAPHIC:

                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsGraphic();
                pOpt->SetGraphic( bFlag );
                break;

        case FN_VIEW_FIELDS:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsField() ;

                pOpt->SetField( bFlag );
                pOpt->SetRef( bFlag );
                break;

        case FN_VIEW_BOUNDS:

                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsSubsLines();

                pOpt->SetSubsLines( bFlag );
                break;

        case SID_GRID_VISIBLE:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsGridVisible();

                pOpt->SetGridVisible( bFlag );
                break;

        case SID_GRID_USE:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsSnap();

                pOpt->SetSnap( bFlag );
                break;

        case SID_HELPLINES_MOVE:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsCrossHair();

                pOpt->SetCrossHair( bFlag );
                break;

        case FN_VIEW_HIDDEN_PARA:
                if ( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsShowHiddenPara();

                pOpt->SetShowHiddenPara( bFlag );
                break;

        case FN_VIEW_SMOOTH_SCROLL:

                if ( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsSmoothScroll();

                pOpt->SetSmoothScroll( bFlag );
                break;


        case FN_VLINEAL:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsViewVLin();

                pOpt->SetViewVLin( bFlag );
                break;

        case FN_VSCROLLBAR:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsViewVScrollBar();

                pOpt->SetViewVScrollBar( bFlag );
                break;

        case FN_HSCROLLBAR:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsViewHScrollBar();

                pOpt->SetViewHScrollBar( bFlag );
                break;
        case FN_RULER:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsViewTabwin();

                pOpt->SetViewTabwin( bFlag );
                break;

        case FN_VIEW_TABLEGRID:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsSubsTable();

                pOpt->SetSubsTable( bFlag );
                break;

        case FN_VIEW_FIELDNAME:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsFldName() ;

                pOpt->SetFldName( bFlag );
                break;
        case FN_VIEW_MARKS:
                if( STATE_TOGGLE == eState )
                    bFlag = !lcl_IsViewMarks(*pOpt) ;

                lcl_SetViewMarks( *pOpt, bFlag );
                break;
        case FN_VIEW_META_CHARS:
                if( STATE_TOGGLE == eState )
                    bFlag = !pOpt->IsViewMetaChars();

                lcl_SetViewMetaChars( *pOpt, bFlag );
        break;
        case SID_AUTOSPELL_CHECK:
            if( STATE_TOGGLE == eState )
                bFlag = bSet = !pOpt->IsOnlineSpell();

            pOpt->SetOnlineSpell(bSet);
            if (xLngProp.is())
            {
                uno::Any aVal(&bSet, ::getCppuBooleanType());
                xLngProp->setPropertyValue( C2S(UPN_IS_SPELL_AUTO), aVal );
            }
            //pOffApp->GetLinguConfig()->SetDefault( sal_False );

            if (!(STATE_TOGGLE == eState && bSet && ( pOpt->IsHideSpell() )))
                break;
        case SID_AUTOSPELL_MARKOFF:
            if( STATE_TOGGLE == eState )
                bFlag = bSet = !pOpt->IsHideSpell();

            pOpt->SetHideSpell(bSet);
            if (xLngProp.is())
            {
                uno::Any aVal(&bSet, ::getCppuBooleanType());
                xLngProp->setPropertyValue( C2S(UPN_IS_SPELL_HIDE), aVal );
            }
            //pOffApp->GetLinguConfig()->SetDefault( sal_False );
        break;

        case FN_SHADOWCURSOR:
            if( STATE_TOGGLE == eState )
                bFlag = bSet = !pOpt->IsShadowCursor();

            pOpt->SetShadowCursor(bSet);
        break;

        default:
            ASSERT(sal_False, Falsche Request-Methode);
            return;
    }

    // UserPrefs setzen Request als bearbeitet kennzeichnen
    SwView* pApplyView = GetView();
    sal_Bool bWebView =  0 != PTR_CAST(SwWebView, pApplyView);
    // die ViewSettings duerfen nur der View zugewiesen werden,
    // wenn es auch die richtigen sind
    if(pApplyView && DEPEND_NONE != eWebDepend)
    {
        if(bWebView && DEPEND_NOWEB == eWebDepend ||
            !bWebView && DEPEND_WEB == eWebDepend )
                pApplyView = 0;
    }
    if( pApplyView )
    {
        SwWrtShell &rSh = GetView()->GetWrtShell();
        rSh.StartAction();
        if( !(*rSh.GetViewOptions() == *pOpt ))
        {
            rSh.ApplyViewOptions( *pOpt );

            //Die UsrPref muessen als Modified gekennzeichnet werden.
            if(bWebView)
            {
                // fuer die Initialisierung
                if(!pWebUsrPref)
                    GetUsrPref(sal_True);
            }
            else
            {
                if(!pUsrPref)
                    GetUsrPref(sal_False);
            }
            SW_MOD()->CheckSpellChanges( pOpt->IsOnlineSpell(), sal_False, sal_False );

        }
        //OS:   Modified wieder zuruecksetzen, weil Ansicht/Felder
        //      das Doc modified setzt.
        if( !bModified )
            rSh.ResetModified();
    }
    sal_uInt16 nDest = VIEWOPT_DEST_VIEW;
    if(DEPEND_WEB == eWebDepend )
        nDest = VIEWOPT_DEST_WEB;
    else if(DEPEND_NOWEB == eWebDepend)
        nDest = VIEWOPT_DEST_TEXT;
    ApplyUsrPref( *pOpt, pApplyView, nDest );
    if ( pApplyView )
        pApplyView->GetWrtShell().EndAction();

    delete pOpt;
    Invalidate(rReq.GetSlot());
    if(!pArgs)
        rReq.AppendItem(SfxUInt16Item(nSlot, bFlag));
}

/*--------------------------------------------------------------------
    Beschreibung:   Felddialog starten
 --------------------------------------------------------------------*/


void SwModule::ExecOther(SfxRequest& rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;

    sal_uInt16 nWhich = rReq.GetSlot();
    switch (nWhich)
    {
        case FN_ENVELOP:
            InsertEnv();
            break;

        case FN_BUSINESS_CARD:
        case FN_LABEL:
            InsertLab(nWhich == FN_LABEL);
            break;

        case SID_SW_DOCMAN_PATH:
            {
                ASSERT(pArgs && pArgs->Count(), "fehlende Parameter GetDocManPath");
                if(!pArgs)
                    return;

                String aParam(((const SfxStringItem &)pArgs->Get(rReq.GetSlot())).GetValue());
                String aPath;

                if(aParam == C2S("~~~GetMacroPath"))
                {
                    SvtPathOptions aPathOpt;
                    aPath = URIHelper::SmartRelToAbs( aPathOpt.GetConfigPath() );
                    if(aPath.Len())
                        aPath += INET_PATH_TOKEN;
                }
                else if(aParam == C2S("~~~GetDefDocExt"))
                    aPath =
#if defined(PM2) || defined(MAC_WITHOUT_EXT)
                    aEmptyStr;
#else
                    C2S(".sdw");
#endif
                else if(aParam == C2S("~~~SetNoFrameBeep"))
                {
                    SwView* pActView = ::GetActiveView();
                    if( pActView && pActView->GetDocShell() )
                        pActView->GetDocShell()->GetDoc()->SetFrmBeepEnabled( sal_False );
                }
                else if(aParam == C2S("~~~SetFrameBeep"))
                {
                    SwView* pActView = ::GetActiveView();
                    if( pActView && pActView->GetDocShell() )
                        pActView->GetDocShell()->GetDoc()->SetFrmBeepEnabled( sal_True );
                }
                rReq.SetReturnValue(SfxStringItem( SID_SW_DOCMAN_PATH, aPath ));
            }
            break;

        case SID_ATTR_ADDRESS:
        {
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(nWhich, sal_False, &pItem))
            {
                ((SvxAddressItem*)pItem)->Store( *SFX_APP()->GetIniManager() );
            }
        }
        break;
        case SID_ATTR_UNDO_COUNT:
        if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(nWhich, sal_False, &pItem))
        {
            const int nNew = ((SfxUInt16Item*)pItem)->GetValue();
            const int nOld = SwEditShell::GetUndoActionCount();
            if(!nNew || !nOld)
            {
                sal_Bool bUndo = nNew != 0;
                //ueber DocShells iterieren und Undo umschalten

                TypeId aType(TYPE(SwDocShell));
                SwDocShell* pDocShell = (SwDocShell*)SfxObjectShell::GetFirst(&aType);
                while( pDocShell )
                {
                    pDocShell->GetDoc()->DoUndo( bUndo );
                    pDocShell = (SwDocShell*)SfxObjectShell::GetNext(*pDocShell, &aType);
                }
            }
            SwEditShell::SetUndoActionCount( nNew );
            // intern kommt der Request aus dem Dialog, dort werden die Options gesetzt
            if( rReq.IsAPI() )
            {
                SvtUndoOptions aOpt;
                aOpt.SetUndoCount(  nNew );
            }
            break;
        }
        break;
        case SID_ATTR_METRIC:
        if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(nWhich, sal_False, &pItem))
        {
            FieldUnit eUnit = (FieldUnit)((const SfxUInt16Item*)pItem)->GetValue();
            switch( eUnit )
            {
                case FUNIT_MM:
                case FUNIT_CM:
                case FUNIT_INCH:
                case FUNIT_PICA:
                case FUNIT_POINT:
                {
                    SwView* pActView = ::GetActiveView();
                    sal_Bool bWebView = 0 != PTR_CAST(SwWebView, pActView);
                    ::SetDfltMetric(eUnit, bWebView);
                }
                break;
            }
        }
        break;

        case FN_SET_MODOPT_TBLNUMFMT:
            {
                sal_Bool bWebView = 0 != PTR_CAST(SwWebView, ::GetActiveView() ),
                     bSet;

                if( pArgs && SFX_ITEM_SET == pArgs->GetItemState(
                        nWhich, sal_False, &pItem ))
                    bSet = ((SfxBoolItem*)pItem)->GetValue();
                else
                    bSet = !pModuleConfig->IsInsTblFormatNum( bWebView );

                pModuleConfig->SetInsTblFormatNum( bWebView, bSet );
            }
            break;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*
SfxMacro *SwWriterApp::CreateMacro() const
{
    return BasicIDE::CreateMacro();
} */

/*--------------------------------------------------------------------
    Beschreibung: Notifies abfangen
 --------------------------------------------------------------------*/


    // Hint abfangen fuer DocInfo
void SwModule::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( rHint.ISA( SfxEventHint ) )
    {
        SfxEventHint& rEvHint = (SfxEventHint&) rHint;
        SwDocShell* pDocSh = PTR_CAST( SwDocShell, rEvHint.GetObjShell() );
        if( pDocSh )
        {
            SwWrtShell* pWrtSh = pDocSh ? pDocSh->GetWrtShell() : 0;
            switch( rEvHint.GetEventId() )
            {
/*          MA 07. Mar. 96: UpdateInputFlds() nur noch bei Dokument neu.
                                (Und bei Einfuegen Textbaust.)
                case SFX_EVENT_OPENDOC:
                // dann am aktuellen Dokument die Input-Fedler updaten
                if( pWrtSh )
                    pWrtSh->UpdateInputFlds();
                break;
*/
            case SFX_EVENT_CREATEDOC:
                // alle FIX-Date/Time Felder auf akt. setzen
                if( pWrtSh )
                {
                    pWrtSh->SetFixFields();
                    pWrtSh->UpdateInputFlds();

                    // Sind Datenbankfelder enthalten?
                    // Erstmal alle verwendeten Datenbanken holen
                    SwDoc *pDoc = pDocSh->GetDoc();
                    SvStringsDtor aDBNameList;
                    pDoc->GetAllUsedDB( aDBNameList );
                    sal_uInt16 nCount = aDBNameList.Count();
                    if (nCount)
                    {   // Datenbankbeamer oeffnen
                        String sDBName = pDoc->GetDBName();
                        ShowDBObj(*pWrtSh, sDBName);
                    }
                }
                break;
            }
        }
    }
    else if(rHint.ISA(SfxItemSetHint))
    {
        if( SFX_ITEM_SET == ((SfxItemSetHint&)rHint).GetItemSet().GetItemState(SID_ATTR_PATHNAME))
        {
            ::GetGlossaries()->UpdateGlosPath( sal_False );
            SwGlossaryList* pList = ::GetGlossaryList();
            if(pList->IsActive())
                pList->Update();
        }

        const SfxPoolItem* pItem = 0;

        if( SFX_ITEM_SET == ((SfxItemSetHint&)rHint).GetItemSet().GetItemState(SID_ATTR_ADDRESS, sal_False, &pItem))
        {
            sActAuthor = ((const SvxAddressItem*)pItem)->GetFirstName();
            sActAuthor += ' ';
            sActAuthor += ((const SvxAddressItem*)pItem)->GetName();
        }
    }
    else if(rHint.ISA(SfxSimpleHint) &&
        ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_DEINITIALIZING)
    {
        // Clipboard loslassen:
        if( pClipboard )
        {
            sal_Bool bFlushToSystem = sal_False;
            if( VclClipboard::ExistsSystemClipboard() )
            {
                if( pClipboard->IsBig() )
                {
                    QueryBox aBox( 0, SW_RES(MSG_CLPBRD_CLEAR) );
                    if( RET_YES == aBox.Execute() )
                        bFlushToSystem = sal_True;
                }
                else
                    bFlushToSystem = sal_True;
            }

            if ( pClipboard )
            {
                VclClipboard::ReleaseClipboard( bFlushToSystem );
                ASSERT( !pClipboard, "can't get off this Clipboard" );
            }
        }

        if(pSrcViewConfig)
        {
            if(!pSrcViewConfig->IsDefault())
                pSrcViewConfig->StoreConfig();
            DELETEZ( pSrcViewConfig );
        }

        DELETEZ(pWebUsrPref);
        DELETEZ(pUsrPref)   ;
        DELETEZ(pModuleConfig);
        DELETEZ(pPrtOpt)      ;
        DELETEZ(pWebPrtOpt)   ;
        DELETEZ(pChapterNumRules);
        DELETEZ(pStdFontConfig)     ;
        DELETEZ(pNavigationConfig)  ;
        DELETEZ(pToolbarConfig)     ;
        DELETEZ(pWebToolbarConfig)  ;
        DELETEZ(pAuthorNames)       ;
    }
}
void SwModule::FillStatusBar( StatusBar& rStatusBar )
{
    // Hier den StatusBar initialisieren
    // und Elemente reinschieben

    // Anzeige Seite
    String aTmp; aTmp.Fill( 10, 'X' );
    rStatusBar.InsertItem( FN_STAT_PAGE, rStatusBar.GetTextWidth(
                                    aTmp ), SIB_AUTOSIZE | SIB_LEFT);
    rStatusBar.SetHelpId(FN_STAT_PAGE, FN_STAT_PAGE);

    // Seitenvorlage
    aTmp.Fill( 15, 'X' );
    rStatusBar.InsertItem( FN_STAT_TEMPLATE, rStatusBar.GetTextWidth(
                                    aTmp ), SIB_AUTOSIZE | SIB_LEFT );
    rStatusBar.SetHelpId(FN_STAT_TEMPLATE, FN_STAT_TEMPLATE);

    // Zoomeinstellungen
    rStatusBar.InsertItem( SID_ATTR_ZOOM, rStatusBar.GetTextWidth(
                                                            C2S("1000%")) );
    rStatusBar.SetHelpId(SID_ATTR_ZOOM, SID_ATTR_ZOOM);

    // Insert/Overwrite
    rStatusBar.InsertItem( SID_ATTR_INSERT,
        SvxInsertStatusBarControl::GetDefItemWidth(rStatusBar));
    rStatusBar.SetHelpId(SID_ATTR_INSERT, SID_ATTR_INSERT);

    // awt::Selection-Modus
    rStatusBar.InsertItem( FN_STAT_SELMODE,
            SvxSelectionModeControl::GetDefItemWidth(rStatusBar));

    rStatusBar.SetHelpId(FN_STAT_SELMODE, FN_STAT_SELMODE);

    // Hyperlink ausfuehren/bearbeiten
    rStatusBar.InsertItem( FN_STAT_HYPERLINKS, rStatusBar.GetTextWidth(
                                                            C2S("HYP")) );
    rStatusBar.SetHelpId(FN_STAT_HYPERLINKS, FN_STAT_HYPERLINKS);

    // Dokument geaendert
    rStatusBar.InsertItem( SID_DOC_MODIFIED, rStatusBar.GetTextWidth(
                                                                C2S("*")));
    rStatusBar.SetHelpId(SID_DOC_MODIFIED, SID_DOC_MODIFIED);

    // den aktuellen Context anzeigen Uhrzeit / FrmPos / TabellenInfo
    aTmp.Fill( 25, sal_Unicode('X') );
    rStatusBar.InsertItem( SID_ATTR_SIZE, rStatusBar.GetTextWidth(
                    aTmp ), SIB_AUTOSIZE | SIB_LEFT | SIB_USERDRAW);
    rStatusBar.SetHelpId(SID_ATTR_SIZE, SID_ATTR_SIZE);
}

/*-----------------18.11.96 10.42-------------------

--------------------------------------------------*/

SwSrcViewConfig* SwModule::GetSourceViewConfig()
{
    if(!pSrcViewConfig)
    {
        pSrcViewConfig = new SwSrcViewConfig();
        pSrcViewConfig->Initialize();
    }
    return pSrcViewConfig;
}

/*-----------------30.01.97 08.30-------------------

--------------------------------------------------*/
const SwMasterUsrPref *SwModule::GetUsrPref(sal_Bool bWeb) const
{
    SwModule* pNonConstModule = (SwModule*)this;
    if(bWeb && !pWebUsrPref)
    {
        // im Load der SwMasterUsrPref wird der SpellChecker gebraucht, dort darf
        // er aber nicht angelegt werden #58256#
        pNonConstModule->pWebUsrPref = new SwMasterUsrPref(TRUE);
    }
    else if(!bWeb && !pUsrPref)
    {
        pNonConstModule->pUsrPref = new SwMasterUsrPref(FALSE);
    }
    return  bWeb ? pWebUsrPref : pUsrPref;
}

/*-------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.2  2000/09/28 15:22:17  os
    use of configuration service in view options

    Revision 1.1.1.1  2000/09/18 17:14:31  hr
    initial import

    Revision 1.289  2000/09/18 16:05:09  willem.vandorp
    OpenOffice header added.

    Revision 1.288  2000/09/08 15:11:56  os
    use configuration service

    Revision 1.287  2000/07/18 12:50:07  os
    replace ofadbmgr

    Revision 1.286  2000/06/26 13:03:00  os
    INetURLObject::SmartRelToAbs removed

    Revision 1.285  2000/06/16 08:25:40  martin.hollmichel
    add: header

    Revision 1.284  2000/06/13 09:56:08  os
    using UCB

    Revision 1.283  2000/05/26 07:21:27  os
    old SW Basic API Slots removed

    Revision 1.282  2000/05/11 12:00:13  tl
    if[n]def ONE_LINGU und alte SUPD entfernt

    Revision 1.281  2000/05/10 11:52:26  os
    Basic API removed

    Revision 1.280  2000/05/09 14:40:15  os
    BASIC interface partially removed

    Revision 1.279  2000/05/08 11:23:33  os
    CreateSubObject

    Revision 1.278  2000/04/11 08:01:30  os
    UNICODE

    Revision 1.277  2000/03/21 15:47:50  os
    UNOIII


    Revision 1.276  2000/03/14 13:47:26  jp
    GetAppWindow() - misuse as parent window eliminated

    Revision 1.275  2000/03/08 17:21:49  os
    GetAppWindow() - misuse as parent window eliminated

    Revision 1.274  2000/01/31 10:14:56  tl
    #71447# workaround removed

    Revision 1.273  2000/01/11 16:24:13  jp
    Bug #71447#: set config as modified if AutoSpell-Flag is changed

    Revision 1.272  2000/01/11 10:20:04  tl
    #70735# CheckSpellChanges now called from SW_MOD for ONE_LINGU

    Revision 1.271  1999/11/30 10:18:55  jp
    Task #70259#: more entries into to the popup menus

    Revision 1.270  1999/11/29 14:39:27  tl
    fixed old header merge problem

    Revision 1.269  1999/11/25 09:27:46  tl
    SvxGetLinguPropertySet => ::GetLinguPropertySet

    Revision 1.266  1999/10/27 11:00:45  hr
    #65293#

    Revision 1.265  1999/10/25 19:36:26  tl
    ongoing ONE_LINGU implementation

    Revision 1.264  1999/10/21 17:45:23  jp
    have to change - SearchFile with SfxIniManager, dont use SwFinder for this

    Revision 1.263  1999/09/28 13:17:45  os
    #67382# separate dialog for business cards

    Revision 1.262  1999/08/31 08:33:52  TL
    #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)


      Rev 1.261   31 Aug 1999 10:33:52   TL
   #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)

      Rev 1.260   04 Feb 1999 21:13:04   JP
   Bug #61404#/61014#: neu: SetFixFields statt zweier Methoden

      Rev 1.259   04 Jan 1999 13:41:38   OS
   #60209# StatusBar: Groessen uber GetDefItemWidth am Controller ermitteln

      Rev 1.258   26 Nov 1998 15:20:44   MA
   #59896# Raster usw jetzt an der Application

      Rev 1.257   23 Nov 1998 17:36:52   JP
   Bug #59754#: TerminateHdl wird nicht mehr fuers Clipboard benoetigt

      Rev 1.256   05 Nov 1998 11:22:10   OS
   #58396# ConfigItems im Notify loeschen

      Rev 1.255   21 Oct 1998 15:29:20   OS
   #58256# Spellchecker vor dem Load der UsrPref anziehen

      Rev 1.254   05 Oct 1998 17:29:26   HR
   UPD->SUPD

      Rev 1.253   08 Sep 1998 16:48:10   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.252   15 Jul 1998 12:51:00   OS
   Navigator an der SwView registrieren #34794#

      Rev 1.251   09 Jul 1998 17:13:24   MA
   muss: kein docman mehr

      Rev 1.250   15 May 1998 17:26:10   OM
   #50043# DirectCursor nicht im OnlineLayout

      Rev 1.249   03 Apr 1998 14:39:16   OS
   RID_DEFAULTTOLOLBOX endlich entsorgt

      Rev 1.248   16 Mar 1998 19:09:12   OM
   Zugriff auf Ini optimiert

      Rev 1.247   12 Feb 1998 14:28:44   OS
   SID_TOGGLESTATUSBAR

      Rev 1.246   29 Jan 1998 14:20:38   OS
   SvxItemFactory an der DesktopApplication anlegen #46753#

      Rev 1.245   26 Jan 1998 09:12:54   OS
   neu: WebViewSettings

      Rev 1.244   22 Dec 1997 17:16:26   MA
   ShadowCursor

      Rev 1.243   10 Dec 1997 14:42:32   OM
   Fixe DocInfoFelder

      Rev 1.242   03 Dec 1997 17:01:24   OS
   Statuszeile per Basic geht wieder #45887#

      Rev 1.241   28 Nov 1997 14:17:14   MA
   includes

      Rev 1.240   28 Nov 1997 11:00:36   TJ
   include

      Rev 1.239   24 Nov 1997 14:22:46   MA
   includes

      Rev 1.238   11 Nov 1997 14:02:58   MA
   precomp entfernt

      Rev 1.237   03 Nov 1997 16:13:08   JP
   neu: Optionen/-Page/Basic-Schnittst. fuer ShadowCursor

      Rev 1.236   01 Sep 1997 13:06:38   OS
   DLL-Umstellung

      Rev 1.235   12 Aug 1997 14:42:40   OS
   Header-Umstellung

      Rev 1.234   08 Aug 1997 17:27:00   OM
   Headerfile-Umstellung

      Rev 1.233   05 Aug 1997 16:09:20   TJ
   include svx/srchitem.hxx

      Rev 1.232   08 Jul 1997 14:04:16   OS
   ConfigItems von der App ans Module

      Rev 1.231   11 Jun 1997 14:47:10   OM
   DB-Beamer bei Vorlagen mit DB-Feldern oeffnen

      Rev 1.230   09 Jun 1997 14:28:18   MA
   chg: Browse-Flag nur noch am Doc

      Rev 1.229   06 Jun 1997 12:43:18   MA
   chg: versteckte Absaetze ausblenden

      Rev 1.228   05 Jun 1997 09:32:50   OS
   FN_INSERT_HEADER/FOOTER invalidieren #40441#

      Rev 1.227   28 May 1997 18:17:28   HJS
   includes

      Rev 1.226   28 May 1997 08:13:06   OS
   SID_ATTR_METRIC am SwModule ausfuehren fr #40192#

      Rev 1.225   05 May 1997 08:02:14   OS
   SID_MAIL_NOTIFY mit richtigen Parametern im  StatusBar

      Rev 1.224   25 Apr 1997 09:19:52   OS
   SID_MAIL_NOTIFY in der Statuszeile

      Rev 1.223   11 Apr 1997 11:00:08   MA
   includes

      Rev 1.222   07 Apr 1997 15:31:10   MH
   chg: header

      Rev 1.221   01 Apr 1997 16:25:56   MH
   add: include sfxdcmn.hxx

      Rev 1.220   23 Mar 1997 23:47:42   OS
   Mail nicht mehr in der Statuszeile

      Rev 1.219   19 Mar 1997 09:26:22   NF
   includes (OS2)

      Rev 1.218   10 Mar 1997 09:29:48   JP
   neues Flag am Doc, um die Beeps aus dem LayoutPaint abzuschalten

      Rev 1.217   05 Feb 1997 18:47:20   MH
   inetdlg.hxx raus

      Rev 1.216   05 Feb 1997 13:34:44   OS
   PrintOptins auch fuer Web

      Rev 1.215   30 Jan 1997 11:10:30   OS
   UsrPrefs verdoppelt; neu: FN_PRINT_LAYOUT

      Rev 1.214   27 Jan 1997 16:25:58   OS
   HtmlMode entfernt

      Rev 1.213   14 Jan 1997 14:25:00   OS
   pModuleConfig immer anlegen

      Rev 1.212   13 Jan 1997 16:45:24   OS
   neu: SwModuleOptions

      Rev 1.211   19 Dec 1996 16:05:50   OS
   BrowseMode mit HtmlMode koppeln; SetPrinter nur, wenn keiner da ist

      Rev 1.210   13 Dec 1996 14:34:08   OS
   UndoCount wird aus der SfxApp besorgt

      Rev 1.209   12 Dec 1996 20:30:32   OS
   GetSbxObject und ItemFactories von der App an das Module

      Rev 1.208   12 Dec 1996 08:41:18   OS
   GetSpellChecker an der App rufen

      Rev 1.207   11 Dec 1996 14:17:40   OS
   View nicht von der App holen; Spell-Opts auch an der MiscConfig setzen

      Rev 1.206   03 Dec 1996 16:56:48   AMA
   Chg: Der Drucker wird nur im !Browsemodus angelegt.

      Rev 1.205   26 Nov 1996 19:44:40   OS
   GetDocManPath mit SID_SW_DOCMAN_PATH

      Rev 1.204   25 Nov 1996 10:30:40   MA
   apptitle ist desktop, dafuer name fuer interface

      Rev 1.203   22 Nov 1996 13:05:44   OS
   neue Ids fuer Wizards

      Rev 1.202   18 Nov 1996 14:15:26   OS
   neu: GetSourceViewConfig am Module

      Rev 1.201   08 Nov 1996 19:39:36   MA
   ResMgr

      Rev 1.200   07 Nov 1996 16:20:40   OS
   FillStatusBar jetzt am Module

      Rev 1.199   05 Nov 1996 15:28:40   OS
   FN_BASIC war auch ueberfluessig

      Rev 1.198   01 Nov 1996 18:02:40   MA
   Writer Module

      Rev 1.197   24 Oct 1996 16:55:52   MA
   #32609#

      Rev 1.196   23 Oct 1996 13:49:10   JP
   SVMEM -> SVSTDARR Umstellung

      Rev 1.195   18 Oct 1996 11:14:14   OS
   SID_RULER_PROTECT auch fuer Browser-Umschaltung invalidieren

      Rev 1.194   27 Sep 1996 13:59:48   OS
   Etiketten und Umschlaege nicht in Readonly-Docs

      Rev 1.193   27 Sep 1996 12:58:12   OS
   Aktualisieren der Autotexte nachdem die Pfade an der App veraendert wurden

      Rev 1.192   23 Sep 1996 08:21:54   OS
   Umschalten der Viewoptions nur noch per Bool

 -------------------------------------------------------------------------*/


