/*************************************************************************
 *
 *  $RCSfile: chardlg.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:15:35 $
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

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_CHARDLG_HXX //autogen
#include <svx/chardlg.hxx>
#endif
#ifndef _SVX_BACKGRND_HXX //autogen
#include <svx/backgrnd.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
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
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _MACASSGN_HXX
#include <macassgn.hxx>
#endif
#ifndef _CHRDLG_HXX
#include <chrdlg.hxx>       // der Dialog
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif

#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _CHRDLG_HRC
#include <chrdlg.hrc>
#endif
#ifndef _CHARDLG_HRC
#include <chardlg.hrc>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

using namespace com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::sfx2;

/*--------------------------------------------------------------------
    Beschreibung:   Der Traeger des Dialoges
 --------------------------------------------------------------------*/


SwCharDlg::SwCharDlg(Window* pParent, SwView& rVw, const SfxItemSet& rCoreSet,
                     const String* pStr, BOOL bIsDrwTxtDlg) :
    SfxTabDialog(pParent, SW_RES(DLG_CHAR), &rCoreSet, pStr != 0),
    rView(rVw),
    bIsDrwTxtMode(bIsDrwTxtDlg)
{
    FreeResource();

    // bspFonr fuer beide Bsp-TabPages

    if(pStr)
    {
        String aTmp( GetText() );
        aTmp += SW_RESSTR(STR_TEXTCOLL_HEADER);
        aTmp += *pStr;
        aTmp += ')';
        SetText(aTmp);
    }

    //OS: Unter OS/2 darf die erste TabPage nie per RemoveTabPage entfernt werden
    AddTabPage(TP_CHAR_STD, SvxCharNamePage::Create, 0);
    AddTabPage(TP_CHAR_EXT, SvxCharEffectsPage::Create, 0);
    AddTabPage(TP_CHAR_POS, SvxCharPositionPage::Create, 0);
    AddTabPage(TP_CHAR_TWOLN, SvxCharTwoLinesPage::Create, 0);
    AddTabPage(TP_CHAR_URL, SwCharURLPage::Create, 0);
    AddTabPage(TP_BACKGROUND,SvxBackgroundTabPage::Create,  0);

    SvtCJKOptions aCJKOptions;
    if(bIsDrwTxtMode)
    {
        RemoveTabPage( TP_CHAR_URL );
        RemoveTabPage( TP_BACKGROUND );
        RemoveTabPage( TP_CHAR_TWOLN );
    }
    else if(!aCJKOptions.IsDoubleLinesEnabled())
        RemoveTabPage( TP_CHAR_TWOLN );
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwCharDlg::~SwCharDlg()
{
}

/*--------------------------------------------------------------------
    Beschreibung:   FontList setzen
 --------------------------------------------------------------------*/



void SwCharDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case TP_CHAR_STD:
            ((SvxCharNamePage&)rPage).SetFontList( *( (SvxFontListItem*)
               ( rView.GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) ) );
                if(!bIsDrwTxtMode)
                    ((SvxCharNamePage&)rPage).SetPreviewBackgroundToCharacter();
            break;
        case TP_CHAR_EXT:
            if(bIsDrwTxtMode)
                ((SvxCharEffectsPage&)rPage).DisableControls(DISABLE_CASEMAP);
            else
            {
                ((SvxCharEffectsPage&)rPage).SetPreviewBackgroundToCharacter();
                ((SvxCharEffectsPage&)rPage).EnableFlash();
            }
            break;
        case TP_CHAR_POS:
            ((SvxCharPositionPage&)rPage).SetPreviewBackgroundToCharacter();
        break;
        case TP_CHAR_TWOLN:
            ((SvxCharTwoLinesPage&)rPage).SetPreviewBackgroundToCharacter();
        break;
    }
}

/*-----------------14.08.96 11.28-------------------

--------------------------------------------------*/

SwCharURLPage::SwCharURLPage(   Window* pParent,
                                const SfxItemSet& rCoreSet ) :
    SfxTabPage( pParent, SW_RES( TP_CHAR_URL ), rCoreSet ),
    aURLFL (        this, SW_RES(FL_URL)),
    aURLFT(         this, SW_RES(FT_URL        )),
    aURLED(         this, SW_RES(ED_URL        )),
    aTextFT(        this, SW_RES(FT_TEXT          )),
    aTextED(        this, SW_RES(ED_TEXT          )),
    aNameFT(        this, SW_RES(FT_NAME    )),
    aNameED(        this, SW_RES(ED_NAME)),
    aTargetFrmFT(   this, SW_RES(FT_TARGET     )),
    aTargetFrmLB(   this, SW_RES(LB_TARGET    )),
    aStyleFL(       this, SW_RES(FL_STYLE      )),
    aVisitedFT(     this, SW_RES(FT_VISITED    )),
    aVisitedLB(     this, SW_RES(LB_VISITED    )),
    aNotVisitedFT(  this, SW_RES(FT_NOT_VISITED)),
    aNotVisitedLB(  this, SW_RES(LB_NOT_VISITED)),
    aEventPB(       this, SW_RES(PB_EVENT      )),
    aURLPB(         this, SW_RES(PB_URL        )),
    pINetItem(0),
    bModified(FALSE)

{
    FreeResource();

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_HTML_MODE, FALSE, &pItem) ||
        ( 0 != ( pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        USHORT nHtmlMode = ((const SfxUInt16Item*)pItem)->GetValue();
        if(HTMLMODE_ON & nHtmlMode)
        {
            aStyleFL.Hide();
            aVisitedFT.Hide();
            aVisitedLB.Hide();
            aNotVisitedFT.Hide();
            aNotVisitedLB.Hide();
        }
    }

    aURLPB.SetClickHdl  (LINK( this, SwCharURLPage, InsertFileHdl));
    aEventPB.SetClickHdl(LINK( this, SwCharURLPage, EventHdl ));

    SwView *pView = ::GetActiveView();
    ::FillCharStyleListBox(aVisitedLB, pView->GetDocShell());
    ::FillCharStyleListBox(aNotVisitedLB, pView->GetDocShell());

    TargetList* pList = new TargetList;
    const SfxFrame* pFrame = pView->GetViewFrame()->GetTopFrame();
    pFrame->GetTargetList(*pList);
    USHORT nCount = (USHORT)pList->Count();
    if( nCount )
    {
        for ( USHORT i = 0; i < nCount; i++ )
        {
            aTargetFrmLB.InsertEntry(*pList->GetObject(i));
        }
        for ( i = nCount; i; i-- )
        {
            delete pList->GetObject( i - 1 );
        }
    }
    delete pList;


}

/*-----------------15.08.96 09.04-------------------

--------------------------------------------------*/

SwCharURLPage::~SwCharURLPage()
{
    delete pINetItem;
}

/*-----------------14.08.96 11.31-------------------

--------------------------------------------------*/

void SwCharURLPage::Reset(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState(RES_TXTATR_INETFMT, FALSE, &pItem))
    {
        const SwFmtINetFmt* pINetFmt = (const SwFmtINetFmt*)pItem;
        aURLED.SetText( INetURLObject::decode( pINetFmt->GetValue(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ));
        aURLED.SaveValue();
        aNameED.SetText(pINetFmt->GetName());
        String sEntry = pINetFmt->GetVisitedFmt();
        if( !sEntry.Len() )
            SwStyleNameMapper::FillUIName( RES_POOLCHR_INET_VISIT, sEntry );
        aVisitedLB.SelectEntry(sEntry);
        sEntry = pINetFmt->GetINetFmt();
        if(!sEntry.Len())
            SwStyleNameMapper::FillUIName( RES_POOLCHR_INET_NORMAL, sEntry );
        aNotVisitedLB.SelectEntry(sEntry);

        aTargetFrmLB.SetText(pINetFmt->GetTargetFrame());
        aVisitedLB.   SaveValue();
        aNotVisitedLB.SaveValue();
        aTargetFrmLB. SaveValue();
        pINetItem = new SvxMacroItem(FN_INET_FIELD_MACRO);

        if( pINetFmt->GetMacroTbl() )
            pINetItem->SetMacroTable( *pINetFmt->GetMacroTbl() );
    }
    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_SELECTION, FALSE, &pItem))
    {
        aTextED.SetText(((const SfxStringItem*)pItem)->GetValue());
        aTextFT.Enable( FALSE );
        aTextED.Enable( FALSE );
    }
}

/*-----------------14.08.96 11.32-------------------

--------------------------------------------------*/

BOOL SwCharURLPage::FillItemSet(SfxItemSet& rSet)
{
    String sURL = aURLED.GetText();
    if(sURL.Len())
        sURL = URIHelper::SmartRelToAbs(sURL);
    SwFmtINetFmt aINetFmt(sURL, aTargetFrmLB.GetText());
    aINetFmt.SetName(aNameED.GetText());
    SwWrtShell &rSh = ::GetActiveView()->GetWrtShell();
    bModified |= aURLED.GetText() != aURLED.GetSavedValue();
    bModified |= aNameED.IsModified();
    bModified |= aTargetFrmLB.GetSavedValue() != aTargetFrmLB.GetText();

    //zuerst die gueltigen Einstellungen setzen
    String sEntry = aVisitedLB.GetSelectEntry();
    USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName( sEntry, GET_POOLID_CHRFMT);
    aINetFmt.SetVisitedFmtId(nId);
    aINetFmt.SetVisitedFmt(nId == RES_POOLCHR_INET_VISIT ? aEmptyStr : sEntry);

    sEntry = aNotVisitedLB.GetSelectEntry();
    nId = SwStyleNameMapper::GetPoolIdFromUIName( sEntry, GET_POOLID_CHRFMT);
    aINetFmt.SetINetFmtId( nId );
    aINetFmt.SetINetFmt(nId == RES_POOLCHR_INET_NORMAL ? aEmptyStr : sEntry);

    if( pINetItem && pINetItem->GetMacroTable().Count() )
        aINetFmt.SetMacroTbl( &pINetItem->GetMacroTable() );

    if(aVisitedLB.GetSavedValue() != aVisitedLB.GetSelectEntryPos())
        bModified = TRUE;

    if(aNotVisitedLB.GetSavedValue() != aNotVisitedLB.GetSelectEntryPos())
        bModified = TRUE;

    if(aTextED.IsModified())
    {
        bModified = TRUE;
        rSet.Put(SfxStringItem(FN_PARAM_SELECTION, aTextED.GetText()));
    }
    if(bModified)
        rSet.Put(aINetFmt);
    return bModified;
}


/*-----------------14.08.96 11.30-------------------

--------------------------------------------------*/

SfxTabPage* SwCharURLPage::Create(  Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwCharURLPage( pParent, rAttrSet ) );
}

/*-----------------14.08.96 15.00-------------------

--------------------------------------------------*/

IMPL_LINK( SwCharURLPage, InsertFileHdl, PushButton *, pBtn )
{
    FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE, 0 );
    if( aDlgHelper.Execute() == ERRCODE_NONE )
    {
        Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();
        aURLED.SetText(URIHelper::SmartRelToAbs(xFP->getFiles().getConstArray()[0]));
    }
    return 0;
}
/*-----------------14.08.96 15.00-------------------

--------------------------------------------------*/

IMPL_LINK( SwCharURLPage, EventHdl, PushButton *, EMPTYARG )
{
    bModified |= SwMacroAssignDlg::INetFmtDlg( this,
                    ::GetActiveView()->GetWrtShell(), pINetItem );
    return 0;
}


