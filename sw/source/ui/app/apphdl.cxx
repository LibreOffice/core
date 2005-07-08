/*************************************************************************
 *
 *  $RCSfile: apphdl.cxx,v $
 *
 *  $Revision: 1.52 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 10:29:30 $
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
#ifndef INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX
#include <svtools/accessibilityoptions.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXEVENT_HXX //autogen
#include <sfx2/event.hxx>
#endif
#ifndef _SFXFRAME_HXX
#include <sfx2/frame.hxx>
#endif
#ifndef _SFX_OBJITEM_HXX
#include <sfx2/objitem.hxx>
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXISETHINT_HXX //autogen
#include <svtools/isethint.hxx>
#endif
#ifndef _SVX_DLG_HYPERLINK_HXX //autogen
#include <svx/hyprlink.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <svtools/ctloptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
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
#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#include <svx/xmlsecctrl.hxx>
#ifndef _NAVICFG_HXX
#include <navicfg.hxx>
#endif

#include <sfx2/objface.hxx>

#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWPVIEW_HXX
#include <pview.hxx>
#endif
#ifndef _SRCVIEW_HXX
#include <srcview.hxx>
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
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#ifndef _GLOBALS_H
#include <globals.h>        // globale Konstanten z.B.
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
#ifndef _DBCONFIG_HXX
#include <dbconfig.hxx>
#endif
#ifndef _MMCONFIGITEM_HXX
#include <mmconfigitem.hxx>
#endif
#ifndef _SWMAILMERGECHILDWINDOW_HXX
#include <mailmergechildwindow.hxx>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
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
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _SW_ABSTDLG_HXX
#include <swabstdlg.hxx>
#endif

// #107253#
#ifndef _SWLINGUCONFIG_HXX
#include <swlinguconfig.hxx>
#endif

#include <vcl/status.hxx>

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
#ifndef _CFGID_H
#include <cfgid.h>
#endif

#include <shells.hrc>

SFX_IMPL_INTERFACE( SwModule, SfxModule, SW_RES(RID_SW_NAME) )
{
    SFX_CHILDWINDOW_REGISTRATION(SvxHyperlinkDlgWrapper::GetChildWindowId());
    SFX_STATUSBAR_REGISTRATION(SW_RES(CFG_STATUSBAR));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION |
            SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER,
            SW_RES(RID_MODULE_TOOLBOX) );
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
            rVOpt.IsShowHiddenChar() ||
            rVOpt.IsBlank()))
    {
        rVOpt.SetParagraph(bOn);
        rVOpt.SetTab(bOn);
        rVOpt.SetLineBreak(bOn);
        rVOpt.SetBlank(bOn);
        rVOpt.SetShowHiddenChar(bOn);
    }
}


sal_Bool lcl_IsViewMarks( const SwViewOption& rVOpt )
{
    return  rVOpt.IsHardBlank() &&
            rVOpt.IsSoftHyph() &&
            SwViewOption::IsFieldShadings();
}
void lcl_SetViewMarks(SwViewOption& rVOpt, sal_Bool bOn )
{
    rVOpt.SetHardBlank(bOn);
    rVOpt.SetSoftHyph(bOn);
    SwViewOption::SetAppearanceFlag(
            VIEWOPT_FIELD_SHADINGS, bOn, TRUE);
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
                {
                    if(!pOpt->IsViewHRuler(TRUE) && !pOpt->IsViewVRuler(TRUE))
                    {
                        rSet.DisableItem(nWhich);
                        nWhich = 0;
                    }
                    else
                        aBool.SetValue( pOpt->IsViewAnyRuler());
                }
                break;
                case FN_VIEW_BOUNDS:
                    aBool.SetValue( SwViewOption::IsDocBoundaries()); break;
                case FN_VIEW_GRAPHIC:
                    aBool.SetValue( !pOpt->IsGraphic() ); break;
                case FN_VIEW_FIELDS:
                    aBool.SetValue( SwViewOption::IsFieldShadings() ); break;
                case FN_VIEW_FIELDNAME:
                    aBool.SetValue( pOpt->IsFldName() ); break;
                case FN_VIEW_MARKS:
                    aBool.SetValue( lcl_IsViewMarks(*pOpt) ); break;
                case FN_VIEW_META_CHARS:
                    aBool.SetValue( pOpt->IsViewMetaChars() ); break;
                case FN_VIEW_TABLEGRID:
                    aBool.SetValue( SwViewOption::IsTableBoundaries() ); break;
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
                        aBool.SetValue( pActView->IsHScrollbarVisible() ); break;
                case FN_VSCROLLBAR:
                    aBool.SetValue( pActView->IsVScrollbarVisible() ); break;
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
            case FN_XFORMS_INIT:
                // slot is always active!
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
    Beschreibung:   Einstellungen fuer den Bildschirm
 --------------------------------------------------------------------*/


void SwModule::ExecViewOptions(SfxRequest &rReq)
{
    SwViewOption* pOpt = 0;
    SfxApplication* pOffApp = OFF_APP();
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
    const SfxPoolItem* pAttr=NULL;

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
                    bFlag = !SwViewOption::IsFieldShadings() ;
                SwViewOption::SetAppearanceFlag(VIEWOPT_FIELD_SHADINGS, bFlag, TRUE );
                break;

        case FN_VIEW_BOUNDS:
                if( STATE_TOGGLE == eState )
                    bFlag = !SwViewOption::IsDocBoundaries();
                SwViewOption::SetAppearanceFlag(VIEWOPT_DOC_BOUNDARIES, bFlag, TRUE );
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
                    bFlag = !pOpt->IsViewVRuler();

                pOpt->SetViewVRuler( bFlag );
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
                    bFlag = !pOpt->IsViewAnyRuler();

                pOpt->SetViewAnyRuler( bFlag );
                break;

        case FN_VIEW_TABLEGRID:
                if( STATE_TOGGLE == eState )
                    bFlag = !SwViewOption::IsTableBoundaries();
                SwViewOption::SetAppearanceFlag(VIEWOPT_TABLE_BOUNDARIES, bFlag, TRUE );
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
            {
                uno::Any aVal( &bSet, ::getCppuBooleanType() );
                String aPropName( C2S(UPN_IS_SPELL_AUTO) );

                // #107253# Replaced SvtLinguConfig with SwLinguConfig wrapper with UsageCount
                SwLinguConfig().SetProperty( aPropName, aVal );

                if (xLngProp.is())
                    xLngProp->setPropertyValue( aPropName, aVal );
            }

            if (!(STATE_TOGGLE == eState && bSet && ( pOpt->IsHideSpell() )))
                break;
        case SID_AUTOSPELL_MARKOFF:
            if( STATE_TOGGLE == eState )
                bFlag = bSet = !pOpt->IsHideSpell();

            pOpt->SetHideSpell(bSet);
            {
                uno::Any aVal( &bSet, ::getCppuBooleanType() );
                String aPropName( C2S(UPN_IS_SPELL_HIDE) );

                // #107253# Replaced SvtLinguConfig with SwLinguConfig wrapper with UsageCount
                SwLinguConfig().SetProperty( aPropName, aVal );

                if (xLngProp.is())
                    xLngProp->setPropertyValue( aPropName, aVal );
            }
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
    {
        SwWrtShell &rSh = pApplyView->GetWrtShell();
        const BOOL bLockedView = rSh.IsViewLocked();
        rSh.LockView( TRUE );    //lock visible section
        pApplyView->GetWrtShell().EndAction();
        rSh.LockView( bLockedView );
    }

    delete pOpt;
    Invalidate(rReq.GetSlot());
    if(!pArgs)
        rReq.AppendItem(SfxBoolItem(nSlot, (BOOL)bFlag));
    rReq.Done();
}
/*-- 06.04.2004 15:21:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SwView* lcl_LoadDoc(SwView* pView, const String& rURL)
{
    sal_Bool bRet = sal_False;
    SwView* pNewView = 0;
    if(rURL.Len())
    {
        SfxStringItem aURL(SID_FILE_NAME, rURL);
        SfxStringItem aTargetFrameName( SID_TARGETNAME, String::CreateFromAscii("_blank") );
        SfxBoolItem aHidden( SID_HIDDEN, TRUE );
        SfxStringItem aReferer(SID_REFERER, pView->GetDocShell()->GetTitle());
        SfxObjectItem* pItem = (SfxObjectItem*)pView->GetViewFrame()->GetDispatcher()->
                Execute(SID_OPENDOC, SFX_CALLMODE_SYNCHRON,
                            &aURL, &aHidden, &aReferer, &aTargetFrameName, 0);
        SfxShell* pShell = pItem ? pItem->GetShell() : 0;

        if(pShell)
        {
            SfxViewShell* pViewShell = pShell->GetViewShell();
            if(pViewShell)
            {
                if( pViewShell->ISA(SwView) )
                {
                    pNewView = PTR_CAST(SwView,pViewShell);
                    pNewView->GetViewFrame()->GetFrame()->Appear();
                }
                else
                {
                    pViewShell->GetViewFrame()->DoClose();
                }
            }
        }
    }
    else
    {
        SfxStringItem aFactory(SID_NEWDOCDIRECT, SwDocShell::Factory().GetFilterContainer()->GetName());
        const SfxFrameItem* pItem = (SfxFrameItem*)
                            pView->GetViewFrame()->GetDispatcher()->Execute(SID_NEWDOCDIRECT,
                                SFX_CALLMODE_SYNCHRON, &aFactory, 0L);
        SfxFrame* pFrm = pItem ? pItem->GetFrame() : 0;
        SfxViewFrame* pFrame = pFrm ? pFrm->GetCurrentViewFrame() : 0;
        pNewView = pFrame ? PTR_CAST(SwView, pFrame->GetViewShell()) : 0;
    }

    return pNewView;
}
/*--------------------------------------------------------------------
    Beschreibung:   Felddialog starten
 --------------------------------------------------------------------*/

void NewXForms( SfxRequest& rReq ); // implementation: below

void SwModule::ExecOther(SfxRequest& rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;

    sal_uInt16 nWhich = rReq.GetSlot();
    switch (nWhich)
    {
        case FN_ENVELOP:
            InsertEnv( rReq );
            break;

        case FN_BUSINESS_CARD:
        case FN_LABEL:
            InsertLab(rReq, nWhich == FN_LABEL);
            break;

        case FN_XFORMS_INIT:
            NewXForms( rReq );
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
        case FN_MAILMERGE_WIZARD:
        {
            SwView* pView = ::GetActiveView();
            DBG_ASSERT(pView, "no current view?")
            if(pView)
            {
                // if called from the child window - get the config item and close the ChildWindow, then restore
                // the wizard
                SwMailMergeChildWindow* pChildWin =
                        static_cast<SwMailMergeChildWindow*>(pView->GetViewFrame()->GetChildWindow(FN_MAILMERGE_CHILDWINDOW));
                SwMailMergeConfigItem* pMMConfig = 0;
                bool bRestoreWizard = false;
                sal_uInt16 nRestartPage = 0;
                if(pChildWin && pChildWin->IsVisible())
                {
                    pMMConfig = pView->GetMailMergeConfigItem();
                    nRestartPage = pView->GetMailMergeRestartPage();
                    if(pView->IsMailMergeSourceView())
                        pMMConfig->SetSourceView( pView );
                    pView->SetMailMergeConfigItem(0, 0, sal_True);
                    SfxViewFrame* pViewFrame = pView->GetViewFrame();
                    pViewFrame->ShowChildWindow(FN_MAILMERGE_CHILDWINDOW, FALSE);
                    DBG_ASSERT(pMMConfig, "no MailMergeConfigItem available");
                    bRestoreWizard = true;
                }
                // to make it bullet proof ;-)
                if(!pMMConfig)
                {
                    pMMConfig = new SwMailMergeConfigItem;
                    pMMConfig->SetSourceView(pView);

                    //set the first used database as default source on the config item
                    if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(
                                    FN_PARAM_DATABASE_PROPERTIES, sal_False, &pItem))
                    {
                        //mailmerge has been called from the database beamer
                        uno::Sequence< beans::PropertyValue> aDBValues;
                        if(static_cast<const SfxUsrAnyItem*>(pItem)->GetValue() >>= aDBValues)
                        {
                            SwDBData aDBData;
                            svx::ODataAccessDescriptor aDescriptor(aDBValues);
                            aDescriptor[svx::daDataSource]   >>= aDBData.sDataSource;
                            aDescriptor[svx::daCommand]      >>= aDBData.sCommand;
                            aDescriptor[svx::daCommandType]  >>= aDBData.nCommandType;

                            uno::Sequence< uno::Any >                   aSelection;
                            uno::Reference< sdbc::XConnection>          xConnection;
                            uno::Reference< sdbc::XDataSource>          xSource;
                            uno::Reference< sdbcx::XColumnsSupplier>    xColumnsSupplier;
                            if ( aDescriptor.has(svx::daSelection) )
                                aDescriptor[svx::daSelection] >>= aSelection;
                            if ( aDescriptor.has(svx::daConnection) )
                                aDescriptor[svx::daConnection] >>= xConnection;
                            uno::Reference<container::XChild> xChild(xConnection, uno::UNO_QUERY);
                            if(xChild.is())
                                xSource = uno::Reference<sdbc::XDataSource>(
                                                            xChild->getParent(), uno::UNO_QUERY);
                            pMMConfig->SetCurrentConnection( xSource, SharedConnection( xConnection, false ),
                                                xColumnsSupplier, aDBData);
                        }
                    }
                    else
                    {
                        SvStringsDtor aDBNameList(5, 1);
                        SvStringsDtor aAllDBNames(5, 5);
                        pView->GetWrtShell().GetAllUsedDB( aDBNameList, &aAllDBNames );
                        if(aDBNameList.Count())
                        {
                            String sDBName = *aDBNameList[0];
                            SwDBData aDBData;
                            aDBData.sDataSource = sDBName.GetToken(0, DB_DELIM);
                            aDBData.sCommand = sDBName.GetToken(1, DB_DELIM);
                            aDBData.nCommandType = sDBName.GetToken(2).ToInt32();
                            //set the currently used database for the wizard
                            pMMConfig->SetCurrentDBData( aDBData );
                        }
                    }
                }
                bool bDeleteConfigItem = true;

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                AbstractMailMergeWizard* pWizard = pFact->CreateMailMergeWizard(*pView, *pMMConfig);

                if(bRestoreWizard)
                {
                    pWizard->ShowPage( nRestartPage );
                }
                while(true)
                {
                    USHORT nRet = pWizard->Execute();
                    nRestartPage = pWizard->GetRestartPage();
                    bDeleteConfigItem = true;
                    if(nRet == RET_LOAD_DOC)
                    {
                        SwView* pNewView = lcl_LoadDoc(pView, pWizard->GetReloadDocument());
                        delete pWizard;
                        if(pNewView)
                        {
                            pView = pNewView;
                            pMMConfig->DocumentReloaded();
                            //new source view!
                            pMMConfig->SetSourceView( pView );
                            pWizard = pFact->CreateMailMergeWizard(*pView, *pMMConfig);
                            pWizard->ShowPage( nRestartPage );
                        }
                        else
                        {
                            pWizard = pFact->CreateMailMergeWizard(*pView, *pMMConfig);
                        }
                    }
                    else if( nRet == RET_TARGET_CREATED )
                    {
                        delete pWizard;
                        SwView* pTargetView = pMMConfig->GetTargetView();
                        uno::Reference< frame::XFrame > xFrame =
                                pView->GetViewFrame()->GetFrame()->GetFrameInterface();
                        xFrame->getContainerWindow()->setVisible(sal_False);
                        DBG_ASSERT(pTargetView, "No target view has been created")
                        if(pTargetView)
                        {
                            pWizard = pFact->CreateMailMergeWizard(*pTargetView, *pMMConfig);
                            pWizard->ShowPage( nRestartPage );
                        }
                        else
                            break; //should not happen - just in case no target view has been created
                    }
                    else if((nRet == RET_EDIT_DOC) || (nRet == RET_EDIT_RESULT_DOC))
                    {
                        //create a non-modal dialog that allows to return to the wizard
                        //the ConfigItem ownership moves to this dialog
                        bool bResult = nRet == RET_EDIT_RESULT_DOC &&
                                pMMConfig->GetTargetView();
                        SwView* pTempView = bResult ? pMMConfig->GetTargetView() : pMMConfig->GetSourceView();
                        pTempView->SetMailMergeConfigItem(pMMConfig, nRestartPage, !bResult);
                        SfxViewFrame* pViewFrame = pTempView->GetViewFrame();
                        pViewFrame->GetDispatcher()->Execute(
                                    FN_MAILMERGE_CHILDWINDOW, SFX_CALLMODE_SYNCHRON);
                        bDeleteConfigItem = false;
                        break;
                    }
                    else if(nRet == RET_REMOVE_TARGET)
                    {
                        SwView* pTargetView = pMMConfig->GetTargetView();
                        SwView* pSourceView = pMMConfig->GetSourceView();
                        DBG_ASSERT(pTargetView && pSourceView, "source or target view not available" )
                        if(pTargetView && pSourceView)
                        {
                            pTargetView->GetViewFrame()->DoClose();
                            pSourceView->GetViewFrame()->GetFrame()->Appear();
                            // the current view has be be set when the target is destroyed
                            pView = pSourceView;
                            pMMConfig->SetTargetView(0);
                            bDeleteConfigItem = false;
                            pWizard = pFact->CreateMailMergeWizard(*pSourceView, *pMMConfig);
                            pWizard->ShowPage( nRestartPage );
                        }
                    }
                    else if( RET_CANCEL == nRet )
                    {
                        //the wizard has been canceled
                        if(pMMConfig->GetTargetView())
                        {
                            delete pWizard;
                            pWizard = 0;
                            pMMConfig->GetTargetView()->GetViewFrame()->DoClose();
                            pMMConfig->SetTargetView(0);
                        }
                        if(pMMConfig->GetSourceView())
                            pMMConfig->GetSourceView()->GetViewFrame()->GetFrame()->Appear();
                        break;
                    }
                    else //finish
                    {
                        SwView* pSourceView = pMMConfig->GetSourceView();
                        if(pSourceView)
                        {
                            SwDocShell* pDocShell = pSourceView->GetDocShell();
                            if(pDocShell->HasName() && !pDocShell->IsModified())
                                pMMConfig->GetSourceView()->GetViewFrame()->DoClose();
                            else
                                pMMConfig->GetSourceView()->GetViewFrame()->GetFrame()->Appear();
                        }
                        break;
                    }
                }
                pMMConfig->Commit();
                if(bDeleteConfigItem)
                    delete pMMConfig;
                if(pWizard)
                    delete pWizard;
            }
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
                    SFX_ITEMSET_ARG( pDocSh->GetMedium()->GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
                    sal_Bool bUpdateFields = sal_True;
                    if( pUpdateDocItem &&  pUpdateDocItem->GetValue() == com::sun::star::document::UpdateDocMode::NO_UPDATE)
                        bUpdateFields = sal_False;
                    pWrtSh->SetFixFields();
                    if(bUpdateFields)
                    {
                        pWrtSh->UpdateInputFlds();

                        // Sind Datenbankfelder enthalten?
                        // Erstmal alle verwendeten Datenbanken holen
                        SwDoc *pDoc = pDocSh->GetDoc();
                        SvStringsDtor aDBNameList;
                        pDoc->GetAllUsedDB( aDBNameList );
                        sal_uInt16 nCount = aDBNameList.Count();
                        if (nCount)
                        {   // Datenbankbeamer oeffnen
                            ShowDBObj(pWrtSh->GetView(), pDoc->GetDBData());
                        }
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
    }
    else if(rHint.ISA(SfxSimpleHint))
    {
        ULONG nHintId = ((SfxSimpleHint&)rHint).GetId();
        if(SFX_HINT_COLORS_CHANGED == nHintId ||
           SFX_HINT_ACCESSIBILITY_CHANGED == nHintId )
        {
            sal_Bool bAccessibility = sal_False;
            if(SFX_HINT_COLORS_CHANGED == nHintId)
                SwViewOption::ApplyColorConfigValues(*pColorConfig);
            else
                bAccessibility = sal_True;

            //invalidate all edit windows
            const TypeId aSwViewTypeId = TYPE(SwView);
            const TypeId aSwPreViewTypeId = TYPE(SwPagePreView);
            const TypeId aSwSrcViewTypeId = TYPE(SwSrcView);
            SfxViewShell* pViewShell = SfxViewShell::GetFirst();
            while(pViewShell)
            {
                if(pViewShell->GetWindow())
                {
                    if((pViewShell->IsA(aSwViewTypeId) ||
                        pViewShell->IsA(aSwPreViewTypeId) ||
                        pViewShell->IsA(aSwSrcViewTypeId)))
                    {
                        if(bAccessibility)
                        {
                            if(pViewShell->IsA(aSwViewTypeId))
                                ((SwView*)pViewShell)->ApplyAccessiblityOptions(*pAccessibilityOptions);
                            else if(pViewShell->IsA(aSwPreViewTypeId))
                                ((SwPagePreView*)pViewShell)->ApplyAccessiblityOptions(*pAccessibilityOptions);
                        }
                        pViewShell->GetWindow()->Invalidate();
                    }
                }
                pViewShell = SfxViewShell::GetNext( *pViewShell );
            }
        }
        else if( SFX_HINT_CTL_SETTINGS_CHANGED == nHintId )
        {
            const SfxObjectShell* pObjSh = SfxObjectShell::GetFirst();
            while( pObjSh )
            {
                if( pObjSh->IsA(TYPE(SwDocShell)) )
                {
                    const SwDoc* pDoc = ((SwDocShell*)pObjSh)->GetDoc();
                    ViewShell* pVSh = 0;
                    pDoc->GetEditShell( &pVSh );
                    if ( pVSh )
                        pVSh->ChgNumberDigits();
                }
                pObjSh = SfxObjectShell::GetNext(*pObjSh);
            }
        }
        else if(SFX_HINT_USER_OPTIONS_CHANGED == nHintId)
        {
            bAuthorInitialised = FALSE;
        }
        else if(SFX_HINT_UNDO_OPTIONS_CHANGED == nHintId)
        {
            const int nNew = GetUndoOptions().GetUndoCount();
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
            SwEditShell::SetUndoActionCount(nNew);
        }
        else if(SFX_HINT_DEINITIALIZING == nHintId)
        {
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
            DELETEZ(pDBConfig);
            EndListening(*pColorConfig);
            DELETEZ(pColorConfig);
            EndListening(*pAccessibilityOptions);
            DELETEZ(pAccessibilityOptions);
            EndListening(*pCTLOptions);
            DELETEZ(pCTLOptions);
            EndListening(*pUserOptions);
            DELETEZ(pUserOptions);
            EndListening(*pUndoOptions);
            DELETEZ(pUndoOptions);
        }
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

    // signatures
    rStatusBar.InsertItem( SID_SIGNATURE, XmlSecStatusBarControl::GetDefItemWidth( rStatusBar ), SIB_USERDRAW );
    rStatusBar.SetHelpId(SID_SIGNATURE, SID_SIGNATURE);

    // den aktuellen Context anzeigen Uhrzeit / FrmPos / TabellenInfo
    aTmp.Fill( 25, sal_Unicode('X') );
    rStatusBar.InsertItem( SID_ATTR_SIZE, rStatusBar.GetTextWidth(
                    aTmp ), SIB_AUTOSIZE | SIB_LEFT | SIB_USERDRAW);
    rStatusBar.SetHelpId(SID_ATTR_SIZE, SID_ATTR_SIZE);
}

/* -----------------------------20.02.01 12:43--------------------------------

 ---------------------------------------------------------------------------*/
SwDBConfig* SwModule::GetDBConfig()
{
    if(!pDBConfig)
        pDBConfig = new SwDBConfig;
    return pDBConfig;
}
/* -----------------------------11.04.2002 15:27------------------------------

 ---------------------------------------------------------------------------*/
svtools::ColorConfig& SwModule::GetColorConfig()
{
    if(!pColorConfig)
    {
        pColorConfig = new svtools::ColorConfig;
        SwViewOption::ApplyColorConfigValues(*pColorConfig);
        StartListening(*pColorConfig);
    }
    return *pColorConfig;
}
/* -----------------------------06.05.2002 09:42------------------------------

 ---------------------------------------------------------------------------*/
SvtAccessibilityOptions& SwModule::GetAccessibilityOptions()
{
    if(!pAccessibilityOptions)
    {
        pAccessibilityOptions = new SvtAccessibilityOptions;
        StartListening(*pAccessibilityOptions);
    }
    return *pAccessibilityOptions;
}
/* -----------------06.05.2003 14:52-----------------

 --------------------------------------------------*/
SvtCTLOptions& SwModule::GetCTLOptions()
{
    if(!pCTLOptions)
    {
        pCTLOptions = new SvtCTLOptions;
        StartListening(*pCTLOptions);
    }
    return *pCTLOptions;
}
/* -----------------07.07.2003 09:31-----------------

 --------------------------------------------------*/
SvtUserOptions& SwModule::GetUserOptions()
{
    if(!pUserOptions)
    {
        pUserOptions = new SvtUserOptions;
        StartListening(*pUserOptions);
    }
    return *pUserOptions;
}
/* -----------------18.07.2003 13:31-----------------

 --------------------------------------------------*/
SvtUndoOptions& SwModule::GetUndoOptions()
{
    if(!pUndoOptions)
    {
        pUndoOptions = new SvtUndoOptions;
        StartListening(*pUndoOptions);
    }
    return *pUndoOptions;
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



void NewXForms( SfxRequest& rReq )
{
    // copied & excerpted from SwModule::InsertLab(..)

    // create new document
    SfxObjectShellRef xDocSh( new SwDocShell( SFX_CREATE_MODE_STANDARD) );
    xDocSh->DoInitNew( 0 );

    // initialize XForms
    static_cast<SwDocShell*>( &xDocSh )->GetDoc()->initXForms( true );

    // put document into frame
    const SfxItemSet* pArgs = rReq.GetArgs();
    DBG_ASSERT( pArgs, "no arguments in SfxRequest");
    if( pArgs != NULL )
    {
        const SfxPoolItem* pFrameItem = NULL;
        pArgs->GetItemState( SID_DOCFRAME, FALSE, &pFrameItem );
        if( pFrameItem != NULL )
        {
            SfxFrame* pFrame =
                static_cast<const SfxFrameItem*>( pFrameItem )->GetFrame();
            DBG_ASSERT( pFrame != NULL, "no frame?" );
            pFrame->InsertDocument( xDocSh );
        }
    }

    // set return value
    rReq.SetReturnValue( SfxVoidItem( rReq.GetSlot() ) );
}
