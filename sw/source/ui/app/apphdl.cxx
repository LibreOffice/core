/*************************************************************************
 *
 *  $RCSfile: apphdl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2000-10-10 07:10:51 $
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
            if(pSrcViewConfig->IsModified())
                pSrcViewConfig->Commit();
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


