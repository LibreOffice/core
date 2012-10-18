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

#include "swtypes.hxx"
#include <view.hxx>
#include <IMark.hxx>
#include <expfld.hxx>
#include <swmodule.hxx>
#include <fldref.hxx>
#include <reffld.hxx>
#include <wrtsh.hxx>

#include <fldui.hrc>
#include <fldtdlg.hrc>
#include <globals.hrc>
// #i83479#
#include <SwNodeNum.hxx>
#include <IDocumentMarkAccess.hxx>
#include <ndtxt.hxx>

#define REFFLDFLAG          0x4000
#define REFFLDFLAG_BOOKMARK 0x4800
#define REFFLDFLAG_FOOTNOTE 0x5000
#define REFFLDFLAG_ENDNOTE  0x6000
// #i83479#
#define REFFLDFLAG_HEADING  0x7100
#define REFFLDFLAG_NUMITEM  0x7200

sal_uInt16  nFldDlgFmtSel       = 0;

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFldRefPage::SwFldRefPage(Window* pParent, const SfxItemSet& rCoreSet ) :
    SwFldPage( pParent, SW_RES( TP_FLD_REF ), rCoreSet ),

    aTypeFT         (this, SW_RES(FT_REFTYPE)),
    aTypeLB         (this, SW_RES(LB_REFTYPE)),
    aSelectionFT    (this, SW_RES(FT_REFSELECTION)),
    aSelectionLB    (this, SW_RES(LB_REFSELECTION)),
    // #i83479#
    aSelectionToolTipLB( this, SW_RES(LB_REFSELECTION_TOOLTIP) ),
    aFormatFT       (this, SW_RES(FT_REFFORMAT)),
    aFormatLB       (this, SW_RES(LB_REFFORMAT)),
    aNameFT         (this, SW_RES(FT_REFNAME)),
    aNameED         (this, SW_RES(ED_REFNAME)),
    aValueFT        (this, SW_RES(FT_REFVALUE)),
    aValueED        (this, SW_RES(ED_REFVALUE)),

    sBookmarkTxt    (SW_RES(STR_REFBOOKMARK)),
    sFootnoteTxt    (SW_RES(STR_REFFOOTNOTE)),
    sEndnoteTxt     (SW_RES(STR_REFENDNOTE)),
    // #i83479#
    sHeadingTxt     (SW_RES(STR_REFHEADING)),
    sNumItemTxt     (SW_RES(STR_REFNUMITEM)),
    maOutlineNodes(),
    maNumItems(),
    mpSavedSelectedTxtNode( 0 ),
    mnSavedSelectedPos( 0 )
{
    FreeResource();

    aNameED.SetModifyHdl(LINK(this, SwFldRefPage, ModifyHdl));

    aTypeLB.SetDoubleClickHdl       (LINK(this, SwFldRefPage, InsertHdl));
    aTypeLB.SetSelectHdl            (LINK(this, SwFldRefPage, TypeHdl));
    aSelectionLB.SetSelectHdl       (LINK(this, SwFldRefPage, SubTypeHdl));
    aSelectionLB.SetDoubleClickHdl  (LINK(this, SwFldRefPage, InsertHdl));
    aFormatLB.SetDoubleClickHdl     (LINK(this, SwFldRefPage, InsertHdl));

    // #i83479#
    aSelectionToolTipLB.SetSelectHdl( LINK(this, SwFldRefPage, SubTypeHdl) );
    aSelectionToolTipLB.SetDoubleClickHdl( LINK(this, SwFldRefPage, InsertHdl) );
    aSelectionToolTipLB.SetStyle( aSelectionToolTipLB.GetStyle() | WB_HSCROLL );
    aSelectionToolTipLB.SetSpaceBetweenEntries(1);
    aSelectionToolTipLB.SetHighlightRange();
}

SwFldRefPage::~SwFldRefPage()
{
}

// #i83479#
void SwFldRefPage::SaveSelectedTxtNode()
{
    mpSavedSelectedTxtNode = 0;
    mnSavedSelectedPos = 0;
    if ( aSelectionToolTipLB.IsVisible() )
    {
        SvTreeListEntry* pEntry = aSelectionToolTipLB.GetCurEntry();
        if ( pEntry )
        {
            const sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());
            SwWrtShell *pSh = GetWrtShell();
            if ( !pSh )
            {
                pSh = ::GetActiveWrtShell();
            }
            if ( nTypeId == REFFLDFLAG_HEADING )
            {
                mnSavedSelectedPos = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(pEntry->GetUserData()));
                if ( mnSavedSelectedPos < maOutlineNodes.size() )
                {
                    mpSavedSelectedTxtNode = maOutlineNodes[mnSavedSelectedPos];
                }
            }
            else if ( nTypeId == REFFLDFLAG_NUMITEM )
            {
                mnSavedSelectedPos = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(pEntry->GetUserData()));
                if ( mnSavedSelectedPos < maNumItems.size() )
                {
                    mpSavedSelectedTxtNode = maNumItems[mnSavedSelectedPos]->GetTxtNode();
                }
            }
        }
    }
}

const SwTxtNode* SwFldRefPage::GetSavedSelectedTxtNode() const
{
    return mpSavedSelectedTxtNode;
}

sal_uInt16 SwFldRefPage::GetSavedSelectedPos() const
{
    return mnSavedSelectedPos;
}


void SwFldRefPage::Reset(const SfxItemSet& )
{
    if (!IsFldEdit())
    {
        SavePos(&aTypeLB);
        // #i83479#
        SaveSelectedTxtNode();
    }
    SetSelectionSel(LISTBOX_ENTRY_NOTFOUND);
    SetTypeSel(LISTBOX_ENTRY_NOTFOUND);
    Init(); // general initialisation

    // initialise TypeListBox
    aTypeLB.SetUpdateMode(sal_False);
    aTypeLB.Clear();

    // fill Type-Listbox

    sal_uInt16 nPos;
    // set/insert reference
    const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

    for (short i = rRg.nStart; i < rRg.nEnd; ++i)
    {
        const sal_uInt16 nTypeId = GetFldMgr().GetTypeId(i);

        if (!IsFldEdit() || nTypeId != TYP_SETREFFLD)
        {
            nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(i), i - rRg.nStart);
            aTypeLB.SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }

    // #i83479#
    // entries for headings and numbered items
    nPos = aTypeLB.InsertEntry(sHeadingTxt);
    aTypeLB.SetEntryData(nPos, (void*)REFFLDFLAG_HEADING);
    nPos = aTypeLB.InsertEntry(sNumItemTxt);
    aTypeLB.SetEntryData(nPos, (void*)REFFLDFLAG_NUMITEM);

    // fill up with the sequence types
    SwWrtShell *pSh = GetWrtShell();
    if(!pSh)
        pSh = ::GetActiveWrtShell();

    sal_uInt16 nFldTypeCnt = pSh->GetFldTypeCount(RES_SETEXPFLD);

    for (sal_uInt16 n = 0; n < nFldTypeCnt; ++n)
    {
        SwSetExpFieldType* pType = (SwSetExpFieldType*)pSh->GetFldType(n, RES_SETEXPFLD);

        if ((nsSwGetSetExpType::GSE_SEQ & pType->GetType()) && pType->GetDepends() && pSh->IsUsed(*pType))
        {
            nPos = aTypeLB.InsertEntry(pType->GetName());
            aTypeLB.SetEntryData(nPos, (void*)(sal_uIntPtr)(REFFLDFLAG | n));
        }
    }

    // text marks - now always (because of globaldocuments)
    nPos = aTypeLB.InsertEntry(sBookmarkTxt);
    aTypeLB.SetEntryData(nPos, (void*)REFFLDFLAG_BOOKMARK);

    // footnotes:
    if( pSh->HasFtns() )
    {
        nPos = aTypeLB.InsertEntry(sFootnoteTxt);
        aTypeLB.SetEntryData(nPos, (void*)REFFLDFLAG_FOOTNOTE);
    }

    // endnotes:
    if ( pSh->HasFtns(true) )
    {
        nPos = aTypeLB.InsertEntry(sEndnoteTxt);
        aTypeLB.SetEntryData(nPos, (void*)REFFLDFLAG_ENDNOTE);
    }

    // select old Pos
    if (!IsFldEdit())
        RestorePos(&aTypeLB);

    aTypeLB.SetUpdateMode(sal_True);

    nFldDlgFmtSel = 0;

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if(!IsRefresh() && sUserData.GetToken(0, ';').
                                EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
        {
            String sVal = sUserData.GetToken(1, ';');
            sal_uInt16 nVal = static_cast< sal_uInt16 >(sVal.ToInt32());
            if(nVal != USHRT_MAX)
            {
                for(sal_uInt16 i = 0; i < aTypeLB.GetEntryCount(); i++)
                    if(nVal == (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(i))
                    {
                        aTypeLB.SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(0);

    if (IsFldEdit())
    {
        aTypeLB.SaveValue();
        aSelectionLB.SaveValue();
        aFormatLB.SaveValue();
        aNameED.SaveValue();
        aValueED.SaveValue();
    }
}

IMPL_LINK_NOARG(SwFldRefPage, TypeHdl)
{
    // save old ListBoxPos
    const sal_uInt16 nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(aTypeLB.GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        if (IsFldEdit())
        {
            // select positions
            String sName;
            sal_uInt16 nFlag = 0;

            switch( GetCurField()->GetSubType() )
            {
                case REF_BOOKMARK:
                {
                    // #i83479#
                    SwGetRefField* pRefFld = dynamic_cast<SwGetRefField*>(GetCurField());
                    if ( pRefFld &&
                         pRefFld->IsRefToHeadingCrossRefBookmark() )
                    {
                        sName = sHeadingTxt;
                        nFlag = REFFLDFLAG_HEADING;
                    }
                    else if ( pRefFld &&
                              pRefFld->IsRefToNumItemCrossRefBookmark() )
                    {
                        sName = sNumItemTxt;
                        nFlag = REFFLDFLAG_NUMITEM;
                    }
                    else
                    {
                        sName = sBookmarkTxt;
                        nFlag = REFFLDFLAG_BOOKMARK;
                    }
                }
                break;

                case REF_FOOTNOTE:
                    sName = sFootnoteTxt;
                    nFlag = REFFLDFLAG_FOOTNOTE;
                    break;

                case REF_ENDNOTE:
                    sName = sEndnoteTxt;
                    nFlag = REFFLDFLAG_ENDNOTE;
                    break;

                case REF_SETREFATTR:
                    sName = SW_RESSTR(STR_GETREFFLD);
                    nFlag = REF_SETREFATTR;
                    break;

                case REF_SEQUENCEFLD:
                    sName = ((SwGetRefField*)GetCurField())->GetSetRefName();
                    nFlag = REFFLDFLAG;
                    break;
            }

            if (aTypeLB.GetEntryPos(sName) == LISTBOX_ENTRY_NOTFOUND)   // reference to deleted mark
            {
                sal_uInt16 nPos = aTypeLB.InsertEntry(sName);
                aTypeLB.SetEntryData(nPos, reinterpret_cast<void*>(nFlag));
            }

            aTypeLB.SelectEntry(sName);
            SetTypeSel(aTypeLB.GetSelectEntryPos());
        }
        else
        {
            SetTypeSel(0);
            aTypeLB.SelectEntryPos(0);
        }
    }

    if (nOld != GetTypeSel())
    {
        sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

        // fill selection-ListBox
        UpdateSubType();

        sal_Bool bName = sal_False;     nFldDlgFmtSel = 0;

        if ( ( !IsFldEdit() || aSelectionLB.GetEntryCount() ) &&
             nOld != LISTBOX_ENTRY_NOTFOUND )
        {
            aNameED.SetText(aEmptyStr);
            aValueED.SetText(aEmptyStr);
        }

        switch (nTypeId)
        {
            case TYP_GETREFFLD:
                if (REFFLDFLAG & (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(nOld))
                    // the the old one stays
                    nFldDlgFmtSel = aFormatLB.GetSelectEntryPos();
                bName = sal_True;
                break;

            case TYP_SETREFFLD:
                bName = sal_True;
                break;

            case REFFLDFLAG_BOOKMARK:
                bName = sal_True;
                // no break!!!
            default:
                if( REFFLDFLAG & nTypeId )
                {
                    sal_uInt16 nOldId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(nOld);
                    if( nOldId & REFFLDFLAG || nOldId == TYP_GETREFFLD )
                        // then the old one stays
                        nFldDlgFmtSel = aFormatLB.GetSelectEntryPos();
                }
                break;
        }

        aNameED.Enable(bName);
        aNameFT.Enable(bName);

        // fill Format-Listbox
        sal_uInt16 nSize = FillFormatLB(nTypeId);
        sal_Bool bFormat = nSize != 0;
        aFormatLB.Enable(bFormat);
        aFormatFT.Enable(bFormat);

        SubTypeHdl();
        ModifyHdl();
    }

    return 0;
}

IMPL_LINK_NOARG(SwFldRefPage, SubTypeHdl)
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    switch(nTypeId)
    {
        case TYP_GETREFFLD:
            if (!IsFldEdit() || aSelectionLB.GetSelectEntryCount())
            {
                aNameED.SetText(aSelectionLB.GetSelectEntry());
                ModifyHdl(&aNameED);
            }
            break;

        case TYP_SETREFFLD:
        {
            SwWrtShell *pSh = GetWrtShell();
            if(!pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh)
            {
                aValueED.SetText(pSh->GetSelTxt());
            }

        }
        break;
        // #i83479#
        case REFFLDFLAG_HEADING:
        case REFFLDFLAG_NUMITEM:
        {
            if ( aSelectionToolTipLB.GetCurEntry() )
            {
                aNameED.SetText( aSelectionToolTipLB.GetEntryText(
                                        aSelectionToolTipLB.GetCurEntry() ) );
            }
        }
        break;

        default:
            if (!IsFldEdit() || aSelectionLB.GetSelectEntryCount())
                aNameED.SetText(aSelectionLB.GetSelectEntry());
            break;
    }

    return 0;
}

/*--------------------------------------------------------------------
     Description: renew types in SelectionLB
 --------------------------------------------------------------------*/

void SwFldRefPage::UpdateSubType()
{
    SwWrtShell *pSh = GetWrtShell();
    if(!pSh)
        pSh = ::GetActiveWrtShell();
    SwGetRefField* pRefFld = (SwGetRefField*)GetCurField();
    const sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    String sOldSel;
    // #i83479#
    if ( aSelectionLB.IsVisible() )
    {
        const sal_uInt16 nSelectionSel = aSelectionLB.GetSelectEntryPos();
        if (nSelectionSel != LISTBOX_ENTRY_NOTFOUND)
        {
            sOldSel = aSelectionLB.GetEntry(nSelectionSel);
        }
    }
    if (IsFldEdit() && !sOldSel.Len())
        sOldSel = String::CreateFromInt32( pRefFld->GetSeqNo() + 1 );

    aSelectionLB.SetUpdateMode(sal_False);
    aSelectionLB.Clear();
    // #i83479#
    aSelectionToolTipLB.SetUpdateMode(sal_False);
    aSelectionToolTipLB.Clear();
    bool bShowSelectionToolTipLB( false );

    if( REFFLDFLAG & nTypeId )
    {
        if (nTypeId == REFFLDFLAG_BOOKMARK)     // text marks!
        {
            aSelectionLB.SetStyle(aSelectionLB.GetStyle()|WB_SORT);
            // get all text marks
            IDocumentMarkAccess* const pMarkAccess = pSh->getIDocumentMarkAccess();
            for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getBookmarksBegin();
                ppMark != pMarkAccess->getBookmarksEnd();
                ++ppMark)
            {
                const ::sw::mark::IMark* pBkmk = ppMark->get();
                if(IDocumentMarkAccess::BOOKMARK == IDocumentMarkAccess::GetType(*pBkmk))
                    aSelectionLB.InsertEntry( pBkmk->GetName() );
            }
            if (IsFldEdit())
                sOldSel = pRefFld->GetSetRefName();
        }
        else if (nTypeId == REFFLDFLAG_FOOTNOTE)
        {
            aSelectionLB.SetStyle(aSelectionLB.GetStyle() & ~WB_SORT);
            SwSeqFldList aArr;
            sal_uInt16 nCnt = pSh->GetSeqFtnList( aArr );

            for( sal_uInt16 n = 0; n < nCnt; ++n )
            {
                aSelectionLB.InsertEntry( aArr[ n ]->sDlgEntry );
                if (IsFldEdit() && pRefFld->GetSeqNo() == aArr[ n ]->nSeqNo)
                    sOldSel = aArr[n]->sDlgEntry;
            }
        }
        else if (nTypeId == REFFLDFLAG_ENDNOTE)
        {
            aSelectionLB.SetStyle(aSelectionLB.GetStyle() & ~WB_SORT);
            SwSeqFldList aArr;
            sal_uInt16 nCnt = pSh->GetSeqFtnList( aArr, true );

            for( sal_uInt16 n = 0; n < nCnt; ++n )
            {
                aSelectionLB.InsertEntry( aArr[ n ]->sDlgEntry );
                if (IsFldEdit() && pRefFld->GetSeqNo() == aArr[ n ]->nSeqNo)
                    sOldSel = aArr[n]->sDlgEntry;
            }
        }
        // #i83479#
        else if ( nTypeId == REFFLDFLAG_HEADING )
        {
            bShowSelectionToolTipLB = true;

            const IDocumentOutlineNodes* pIDoc( pSh->getIDocumentOutlineNodesAccess() );
            pIDoc->getOutlineNodes( maOutlineNodes );
            bool bCertainTxtNodeSelected( false );
            SvTreeListEntry* pEntry = 0;
            sal_uInt16 nOutlIdx = 0;
            for ( nOutlIdx = 0; nOutlIdx < maOutlineNodes.size(); ++nOutlIdx )
            {
                pEntry = aSelectionToolTipLB.InsertEntry(
                                pIDoc->getOutlineText( nOutlIdx, true, true ) );
                pEntry->SetUserData( reinterpret_cast<void*>(nOutlIdx) );
                if ( ( IsFldEdit() &&
                       pRefFld->GetReferencedTxtNode() == maOutlineNodes[nOutlIdx] ) ||
                     GetSavedSelectedTxtNode() == maOutlineNodes[nOutlIdx] )
                {
                    aSelectionToolTipLB.Select( pEntry );
                    sOldSel.Erase();
                    bCertainTxtNodeSelected = true;
                }
                else if ( !bCertainTxtNodeSelected &&
                          GetSavedSelectedPos() == nOutlIdx )
                {
                    aSelectionToolTipLB.Select( pEntry );
                    sOldSel.Erase();
                }
            }
        }
        else if ( nTypeId == REFFLDFLAG_NUMITEM )
        {
            bShowSelectionToolTipLB = true;

            const IDocumentListItems* pIDoc( pSh->getIDocumentListItemsAccess() );
            pIDoc->getNumItems( maNumItems );
            bool bCertainTxtNodeSelected( false );
            SvTreeListEntry* pEntry = 0;
            sal_uInt16 nNumItemIdx = 0;
            for ( nNumItemIdx = 0; nNumItemIdx < maNumItems.size(); ++nNumItemIdx )
            {
                pEntry = aSelectionToolTipLB.InsertEntry(
                            pIDoc->getListItemText( *maNumItems[nNumItemIdx], true, true ) );
                pEntry->SetUserData( reinterpret_cast<void*>(nNumItemIdx) );
                if ( ( IsFldEdit() &&
                       pRefFld->GetReferencedTxtNode() == maNumItems[nNumItemIdx]->GetTxtNode() ) ||
                     GetSavedSelectedTxtNode() == maNumItems[nNumItemIdx]->GetTxtNode() )
                {
                    aSelectionToolTipLB.Select( pEntry );
                    sOldSel.Erase();
                    bCertainTxtNodeSelected = true;
                }
                else if ( !bCertainTxtNodeSelected &&
                          GetSavedSelectedPos() == nNumItemIdx )
                {
                    aSelectionToolTipLB.Select( pEntry );
                    sOldSel.Erase();
                }
            }
        }
        else
        {
            aSelectionLB.SetStyle(aSelectionLB.GetStyle()|WB_SORT);
            // get the fields to Seq-FieldType:

            SwSetExpFieldType* pType = (SwSetExpFieldType*)pSh->GetFldType(
                                nTypeId & ~REFFLDFLAG, RES_SETEXPFLD );
            if( pType )
            {
                SwSeqFldList aArr;
                // old selection should be kept in non-edit mode
                if(IsFldEdit())
                    sOldSel.Erase();

                sal_uInt16 nCnt = pType->GetSeqFldList( aArr );
                for( sal_uInt16 n = 0; n < nCnt; ++n )
                {
                    aSelectionLB.InsertEntry( aArr[ n ]->sDlgEntry );
                    if (IsFldEdit() && !sOldSel.Len() &&
                        aArr[ n ]->nSeqNo == pRefFld->GetSeqNo())
                        sOldSel = aArr[ n ]->sDlgEntry;
                }

                if (IsFldEdit() && !sOldSel.Len())
                    sOldSel = String::CreateFromInt32( pRefFld->GetSeqNo() + 1);
            }
        }
    }
    else
    {
        std::vector<rtl::OUString> aLst;
        GetFldMgr().GetSubTypes(nTypeId, aLst);
        for(size_t i = 0; i < aLst.size(); ++i)
            aSelectionLB.InsertEntry(aLst[i]);

        if (IsFldEdit())
            sOldSel = pRefFld->GetSetRefName();
    }

    // #i83479#
    aSelectionToolTipLB.Show( bShowSelectionToolTipLB );
    aSelectionLB.Show( !bShowSelectionToolTipLB );
    if ( bShowSelectionToolTipLB )
    {
        aSelectionToolTipLB.SetUpdateMode(sal_True);

        sal_Bool bEnable = aSelectionToolTipLB.GetEntryCount() != 0;
        aSelectionToolTipLB.Enable( bEnable );
        aSelectionFT.Enable( bEnable );

        if ( aSelectionToolTipLB.GetCurEntry() != 0 )
        {
            aSelectionToolTipLB.MakeVisible( aSelectionToolTipLB.GetCurEntry() );
        }

        if ( IsFldEdit() && aSelectionToolTipLB.GetCurEntry() == 0 )
        {
            aNameED.SetText(sOldSel);
        }
    }
    else
    {
        aSelectionLB.SetUpdateMode(sal_True);

        // enable or disable
        sal_Bool bEnable = aSelectionLB.GetEntryCount() != 0;
        aSelectionLB.Enable( bEnable );
        aSelectionFT.Enable( bEnable );

        if ( bEnable )
        {
            aSelectionLB.SelectEntry(sOldSel);
            if (!aSelectionLB.GetSelectEntryCount() && !IsFldEdit())
                aSelectionLB.SelectEntryPos(0);
        }

        if (IsFldEdit() && !aSelectionLB.GetSelectEntryCount()) // in case the reference was already deleted...
            aNameED.SetText(sOldSel);
    }
}

sal_uInt16 SwFldRefPage::FillFormatLB(sal_uInt16 nTypeId)
{
    String sOldSel;

    sal_uInt16 nFormatSel = aFormatLB.GetSelectEntryPos();
    if (nFormatSel != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = aFormatLB.GetEntry(nFormatSel);

    // fill Format-Listbox
    aFormatLB.Clear();

    // refernce has less that the annotation
    sal_uInt16 nSize( 0 );
    bool bAddCrossRefFormats( false );
    switch (nTypeId)
    {
        // #i83479#
        case REFFLDFLAG_HEADING:
        case REFFLDFLAG_NUMITEM:
            bAddCrossRefFormats = true;
            // intentional no break here

        case TYP_GETREFFLD:
        case REFFLDFLAG_BOOKMARK:
        case REFFLDFLAG_FOOTNOTE:
        case REFFLDFLAG_ENDNOTE:
            nSize = FMT_REF_PAGE_PGDSC - FMT_REF_BEGIN + 1;
            break;

        default:
            // #i83479#

            if ( REFFLDFLAG & nTypeId )
            {
                nSize = FMT_REF_ONLYSEQNO - FMT_REF_BEGIN + 1;
            }
            else
            {
                nSize = GetFldMgr().GetFormatCount( nTypeId, sal_False, IsFldDlgHtmlMode() );
            }
            break;
    }

    if (REFFLDFLAG & nTypeId)
        nTypeId = TYP_GETREFFLD;

    for (sal_uInt16 i = 0; i < nSize; i++)
    {
        sal_uInt16 nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr( nTypeId, i ));
        aFormatLB.SetEntryData( nPos, reinterpret_cast<void*>(GetFldMgr().GetFormatId( nTypeId, i )));
    }
    // #i83479#
    if ( bAddCrossRefFormats )
    {
        sal_uInt16 nFormat = FMT_REF_NUMBER - FMT_REF_BEGIN;
        sal_uInt16 nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr( nTypeId, nFormat ));
        aFormatLB.SetEntryData( nPos, reinterpret_cast<void*>(GetFldMgr().GetFormatId( nTypeId, nFormat )));
        nFormat = FMT_REF_NUMBER_NO_CONTEXT - FMT_REF_BEGIN;
        nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr( nTypeId, nFormat ));
        aFormatLB.SetEntryData( nPos, reinterpret_cast<void*>(GetFldMgr().GetFormatId( nTypeId, nFormat )));
        nFormat = FMT_REF_NUMBER_FULL_CONTEXT - FMT_REF_BEGIN;
        nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr( nTypeId, nFormat ));
        aFormatLB.SetEntryData( nPos, reinterpret_cast<void*>(GetFldMgr().GetFormatId( nTypeId, nFormat )));
        nSize += 3;
    }

    // select a certain entry
    if (nSize)
    {
        if (!IsFldEdit())
            aFormatLB.SelectEntry(sOldSel);
        else
            aFormatLB.SelectEntry(SW_RESSTR(FMT_REF_BEGIN + (sal_uInt16)GetCurField()->GetFormat()));

        if (!aFormatLB.GetSelectEntryCount())
        {
            aFormatLB.SelectEntryPos(nFldDlgFmtSel);
            if (!aFormatLB.GetSelectEntryCount())
                aFormatLB.SelectEntryPos(0);
        }
    }

    return nSize;
}

/*--------------------------------------------------------------------
    Description: Modify
 --------------------------------------------------------------------*/

IMPL_LINK_NOARG(SwFldRefPage, ModifyHdl)
{
    String aName(aNameED.GetText());
    const sal_uInt16 nLen = aName.Len();

    sal_Bool bEnable = sal_True;
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    if ((nTypeId == TYP_SETREFFLD && !GetFldMgr().CanInsertRefMark(aName)) ||
        (nLen == 0 && (nTypeId == TYP_GETREFFLD || nTypeId == TYP_SETREFFLD ||
                       nTypeId == REFFLDFLAG_BOOKMARK)))
        bEnable = sal_False;

    EnableInsert(bEnable);

    aSelectionLB.SelectEntry(aName);

    return 0;
}

sal_Bool SwFldRefPage::FillItemSet(SfxItemSet& )
{
    sal_Bool bModified = sal_False;
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    sal_uInt16 nSubType = 0;
    sal_uLong nFormat;

    nFormat = aFormatLB.GetSelectEntryPos();

    if(nFormat == LISTBOX_ENTRY_NOTFOUND)
        nFormat = 0;
    else
        nFormat = (sal_uLong)aFormatLB.GetEntryData((sal_uInt16)nFormat);

    String aVal(aValueED.GetText());
    String aName(aNameED.GetText());

    switch(nTypeId)
    {
        case TYP_GETREFFLD:
            nSubType = REF_SETREFATTR;
            break;

        case TYP_SETREFFLD:
        {
            SwFieldType* pType = GetFldMgr().GetFldType(RES_SETEXPFLD, aName);

            if(!pType)  // Only insert when the name doesn't exist yet
            {
                aSelectionLB.InsertEntry(aName);
                aSelectionLB.Enable();
                aSelectionFT.Enable();
            }
            break;
        }
    }

    SwGetRefField* pRefFld = (SwGetRefField*)GetCurField();

    if (REFFLDFLAG & nTypeId)
    {
        SwWrtShell *pSh = GetWrtShell();
        if(!pSh)
        {
            pSh = ::GetActiveWrtShell();
        }
        if (nTypeId == REFFLDFLAG_BOOKMARK)     // text marks!
        {
            aName = aNameED.GetText();
            nTypeId = TYP_GETREFFLD;
            nSubType = REF_BOOKMARK;
        }
        else if (REFFLDFLAG_FOOTNOTE == nTypeId)        // footnotes
        {
            SwSeqFldList aArr;
            _SeqFldLstElem aElem( aSelectionLB.GetSelectEntry(), 0 );

            sal_uInt16 nPos;

            nTypeId = TYP_GETREFFLD;
            nSubType = REF_FOOTNOTE;
            aName.Erase();

            if (pSh->GetSeqFtnList(aArr) && aArr.SeekEntry(aElem, &nPos))
            {
                aVal = String::CreateFromInt32( aArr[nPos]->nSeqNo );

                if (IsFldEdit() && aArr[nPos]->nSeqNo == pRefFld->GetSeqNo())
                    bModified = sal_True; // can happen with fields of which the references were deleted
            }
            else if (IsFldEdit())
                aVal = String::CreateFromInt32( pRefFld->GetSeqNo() );
        }
        else if (REFFLDFLAG_ENDNOTE == nTypeId)         // endnotes
        {
            SwSeqFldList aArr;
            _SeqFldLstElem aElem( aSelectionLB.GetSelectEntry(), 0 );

            sal_uInt16 nPos;

            nTypeId = TYP_GETREFFLD;
            nSubType = REF_ENDNOTE;
            aName.Erase();

            if (pSh->GetSeqFtnList(aArr, true) && aArr.SeekEntry(aElem, &nPos))
            {
                aVal = String::CreateFromInt32( aArr[nPos]->nSeqNo );

                if (IsFldEdit() && aArr[nPos]->nSeqNo == pRefFld->GetSeqNo())
                    bModified = sal_True; // can happen with fields of which the reference was deleted
            }
            else if (IsFldEdit())
                aVal = String::CreateFromInt32( pRefFld->GetSeqNo() );
        }
        // #i83479#
        else if ( nTypeId == REFFLDFLAG_HEADING )
        {
            SvTreeListEntry* pEntry = aSelectionToolTipLB.GetCurEntry();
            OSL_ENSURE( pEntry,
                    "<SwFldRefPage::FillItemSet(..)> - no entry selected in selection tool tip listbox!" );
            if ( pEntry )
            {
                const sal_uInt16 nOutlIdx( static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(pEntry->GetUserData())) );
                pSh->getIDocumentOutlineNodesAccess()->getOutlineNodes( maOutlineNodes );
                if ( nOutlIdx < maOutlineNodes.size() )
                {
                    ::sw::mark::IMark const * const pMark = pSh->getIDocumentMarkAccess()->getMarkForTxtNode(
                        *(maOutlineNodes[nOutlIdx]),
                        IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK);
                    aName = pMark->GetName();
                    nTypeId = TYP_GETREFFLD;
                    nSubType = REF_BOOKMARK;
                }
            }
        }
        else if ( nTypeId == REFFLDFLAG_NUMITEM )
        {
            SvTreeListEntry* pEntry = aSelectionToolTipLB.GetCurEntry();
            OSL_ENSURE( pEntry,
                    "<SwFldRefPage::FillItemSet(..)> - no entry selected in selection tool tip listbox!" );
            if ( pEntry )
            {
                const sal_uInt16 nNumItemIdx( static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(pEntry->GetUserData())) );
                pSh->getIDocumentListItemsAccess()->getNumItems( maNumItems );
                if ( nNumItemIdx < maNumItems.size() )
                {
                    ::sw::mark::IMark const * const pMark = pSh->getIDocumentMarkAccess()->getMarkForTxtNode(
                        *(maNumItems[nNumItemIdx]->GetTxtNode()),
                        IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK);
                    aName = pMark->GetName();
                    nTypeId = TYP_GETREFFLD;
                    nSubType = REF_BOOKMARK;
                }
            }
        }
        else                                // SeqenceFields
        {
            // get fields for Seq-FeldType:
            SwSetExpFieldType* pType = (SwSetExpFieldType*)pSh->GetFldType(
                                    nTypeId & ~REFFLDFLAG, RES_SETEXPFLD );
            if( pType )
            {
                SwSeqFldList aArr;
                _SeqFldLstElem aElem( aSelectionLB.GetSelectEntry(), 0 );

                sal_uInt16 nPos;

                nTypeId = TYP_GETREFFLD;
                nSubType = REF_SEQUENCEFLD;
                aName = pType->GetName();

                if (pType->GetSeqFldList(aArr) && aArr.SeekEntry(aElem, &nPos))
                {
                    aVal = String::CreateFromInt32( aArr[nPos]->nSeqNo );

                    if (IsFldEdit() && aArr[nPos]->nSeqNo == pRefFld->GetSeqNo())
                        bModified = sal_True; // can happen with fields of which the reference was deleted
                }
                else if (IsFldEdit())
                    aVal = String::CreateFromInt32( pRefFld->GetSeqNo() );
            }
        }
    }

    if (IsFldEdit() && nTypeId == TYP_GETREFFLD)
    {
        aVal.Insert('|', 0);
        aVal.Insert(String::CreateFromInt32(nSubType), 0);
    }

    if (!IsFldEdit() || bModified ||
        aNameED.GetSavedValue() != aNameED.GetText() ||
        aValueED.GetSavedValue() != aValueED.GetText() ||
        aTypeLB.GetSavedValue() != aTypeLB.GetSelectEntryPos() ||
        aSelectionLB.GetSavedValue() != aSelectionLB.GetSelectEntryPos() ||
        aFormatLB.GetSavedValue() != aFormatLB.GetSelectEntryPos())
    {
        InsertFld( nTypeId, nSubType, aName, aVal, nFormat );
    }

    ModifyHdl();    // enable/disable insert if applicable

    return sal_False;
}

SfxTabPage* SwFldRefPage::Create(   Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldRefPage( pParent, rAttrSet ) );
}

sal_uInt16 SwFldRefPage::GetGroup()
{
    return GRP_REF;
}

void    SwFldRefPage::FillUserData()
{
    String sData(rtl::OUString(USER_DATA_VERSION));
    sData += ';';
    sal_uInt16 nTypeSel = aTypeLB.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = sal::static_int_cast< sal_uInt16 >(reinterpret_cast< sal_uIntPtr >(aTypeLB.GetEntryData( nTypeSel )));
    sData += String::CreateFromInt32( nTypeSel );
    SetUserData(sData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
