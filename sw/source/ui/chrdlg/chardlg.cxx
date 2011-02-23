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
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <hintids.hxx>

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <svl/urihelper.hxx>
#include <svl/stritem.hxx>
#include <editeng/flstitem.hxx>
#include <svx/htmlmode.hxx>
#include <svl/cjkoptions.hxx>

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#include <swtypes.hxx>
#include <errhdl.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <wrtsh.hxx>
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <uitool.hxx>
#include <fmtinfmt.hxx>
#include <macassgn.hxx>
#ifndef _CHRDLG_HXX
#include <chrdlg.hxx>       // der Dialog
#endif
#include <swmodule.hxx>
#include <poolfmt.hxx>

#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _CHRDLG_HRC
#include <chrdlg.hrc>
#endif
#ifndef _CHARDLG_HRC
#include <chardlg.hrc>
#endif
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <SwStyleNameMapper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/viewfrm.hxx>

#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <svx/flagsdef.hxx>
#include <svx/dialogs.hrc>

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::sfx2;

/*--------------------------------------------------------------------
    Beschreibung:   Der Traeger des Dialoges
 --------------------------------------------------------------------*/


SwCharDlg::SwCharDlg(Window* pParent, SwView& rVw, const SfxItemSet& rCoreSet,
                     const String* pStr, sal_Bool bIsDrwTxtDlg) :
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
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    AddTabPage(TP_CHAR_STD, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), 0 );
    AddTabPage(TP_CHAR_EXT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), 0 );
    AddTabPage(TP_CHAR_POS, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), 0 );
    AddTabPage(TP_CHAR_TWOLN, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), 0 );
    AddTabPage(TP_CHAR_URL, SwCharURLPage::Create, 0);
    AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );

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



void SwCharDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch( nId )
    {
        case TP_CHAR_STD:
            {
            SvxFontListItem aFontListItem( *( (SvxFontListItem*)
               ( rView.GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) ) );
            aSet.Put (SvxFontListItem( aFontListItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
                if(!bIsDrwTxtMode)
                    aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
            rPage.PageCreated(aSet);
            }
            break;
        case TP_CHAR_EXT:
            if(bIsDrwTxtMode)
                aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));

            else
            {
                aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER|SVX_ENABLE_FLASH));
            }
            rPage.PageCreated(aSet);
            break;
        case TP_CHAR_POS:
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
            rPage.PageCreated(aSet);
        break;
        case TP_CHAR_TWOLN:
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
            rPage.PageCreated(aSet);
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
    aURLPB(         this, SW_RES(PB_URL        )),
    aEventPB(       this, SW_RES(PB_EVENT      )),
    aStyleFL(       this, SW_RES(FL_STYLE      )),
    aVisitedFT(     this, SW_RES(FT_VISITED    )),
    aVisitedLB(     this, SW_RES(LB_VISITED    )),
    aNotVisitedFT(  this, SW_RES(FT_NOT_VISITED)),
    aNotVisitedLB(  this, SW_RES(LB_NOT_VISITED)),
    pINetItem(0),
    bModified(sal_False)

{
    FreeResource();

    aEventPB.SetAccessibleRelationMemberOf(&aURLFL);

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_HTML_MODE, sal_False, &pItem) ||
        ( 0 != ( pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        sal_uInt16 nHtmlMode = ((const SfxUInt16Item*)pItem)->GetValue();
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
    const SfxFrame& rFrame = pView->GetViewFrame()->GetTopFrame();
    rFrame.GetTargetList(*pList);
    sal_uInt16 nCount = (sal_uInt16)pList->Count();
    if( nCount )
    {
        sal_uInt16 i;

        for ( i = 0; i < nCount; i++ )
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
    if(SFX_ITEM_SET == rSet.GetItemState(RES_TXTATR_INETFMT, sal_False, &pItem))
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
    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_SELECTION, sal_False, &pItem))
    {
        aTextED.SetText(((const SfxStringItem*)pItem)->GetValue());
        aTextFT.Enable( sal_False );
        aTextED.Enable( sal_False );
    }
}

/*-----------------14.08.96 11.32-------------------

--------------------------------------------------*/

sal_Bool SwCharURLPage::FillItemSet(SfxItemSet& rSet)
{
   ::rtl::OUString sURL = aURLED.GetText();
   if(sURL.getLength())
    {
        sURL = URIHelper::SmartRel2Abs(INetURLObject(), sURL, Link(), false );
        // #i100683# file URLs should be normalized in the UI
        static const sal_Char* pFile = "file:";
       sal_Int32 nLength = ((sal_Int32)sizeof(pFile)-1);
       if( sURL.copy(0, nLength ).equalsAsciiL( pFile, nLength ))
            sURL = URIHelper::simpleNormalizedMakeRelative(::rtl::OUString(), sURL);
    }

    SwFmtINetFmt aINetFmt(sURL, aTargetFrmLB.GetText());
    aINetFmt.SetName(aNameED.GetText());
    bModified |= aURLED.GetText() != aURLED.GetSavedValue();
    bModified |= aNameED.IsModified();
    bModified |= aTargetFrmLB.GetSavedValue() != aTargetFrmLB.GetText();

    //zuerst die gueltigen Einstellungen setzen
    String sEntry = aVisitedLB.GetSelectEntry();
    sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( sEntry, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
    aINetFmt.SetVisitedFmtId(nId);
    aINetFmt.SetVisitedFmt(nId == RES_POOLCHR_INET_VISIT ? aEmptyStr : sEntry);

    sEntry = aNotVisitedLB.GetSelectEntry();
    nId = SwStyleNameMapper::GetPoolIdFromUIName( sEntry, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
    aINetFmt.SetINetFmtId( nId );
    aINetFmt.SetINetFmt(nId == RES_POOLCHR_INET_NORMAL ? aEmptyStr : sEntry);

    if( pINetItem && pINetItem->GetMacroTable().Count() )
        aINetFmt.SetMacroTbl( &pINetItem->GetMacroTable() );

    if(aVisitedLB.GetSavedValue() != aVisitedLB.GetSelectEntryPos())
        bModified = sal_True;

    if(aNotVisitedLB.GetSavedValue() != aNotVisitedLB.GetSelectEntryPos())
        bModified = sal_True;

    if(aTextED.IsModified())
    {
        bModified = sal_True;
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

IMPL_LINK( SwCharURLPage, InsertFileHdl, PushButton *, EMPTYARG )
{
    FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE, 0 );
    if( aDlgHelper.Execute() == ERRCODE_NONE )
    {
        Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();
        aURLED.SetText(xFP->getFiles().getConstArray()[0]);
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


