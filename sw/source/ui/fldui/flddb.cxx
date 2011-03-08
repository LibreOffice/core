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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <globals.hrc>
#include <dbfld.hxx>
#include <fldtdlg.hxx>
#include <numrule.hxx>

#include <fldtdlg.hrc>
#include <fldui.hrc>
#include <flddb.hxx>
#include <dbconfig.hxx>
#include <dbmgr.hxx>

#define USER_DATA_VERSION_1     "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFldDBPage::SwFldDBPage(Window* pParent, const SfxItemSet& rCoreSet ) :
    SwFldPage( pParent, SW_RES( TP_FLD_DB ), rCoreSet ),
    aTypeFT     (this, SW_RES(FT_DBTYPE)),
    aTypeLB     (this, SW_RES(LB_DBTYPE)),
    aSelectionFT(this, SW_RES(FT_DBSELECTION)),
    aDatabaseTLB(this, SW_RES(TLB_DBLIST), 0, aEmptyStr, FALSE),
    aAddDBFT(this,      SW_RES(FT_ADDDB)),
    aAddDBPB(this,      SW_RES(PB_ADDDB)),
    aConditionFT(this, SW_RES(FT_DBCONDITION)),
    aConditionED(this, SW_RES(ED_DBCONDITION)),
    aValueFT    (this, SW_RES(FT_DBSETNUMBER)),
    aValueED    (this, SW_RES(ED_DBSETNUMBER)),
    aDBFormatRB (this, SW_RES(RB_DBOWNFORMAT)),
    aNewFormatRB(this, SW_RES(RB_DBFORMAT)),
    aNumFormatLB(this, SW_RES(LB_DBNUMFORMAT)),
    aFormatLB   (this, SW_RES(LB_DBFORMAT)),
    aFormatFL   (this, SW_RES(FL_DBFORMAT)),
    aFormatVertFL   (this, SW_RES(FL_DBFORMAT_VERT))
{
    FreeResource();

    aOldNumSelectHdl = aNumFormatLB.GetSelectHdl();

    aNumFormatLB.SetSelectHdl(LINK(this, SwFldDBPage, NumSelectHdl));
    aDatabaseTLB.SetSelectHdl(LINK(this, SwFldDBPage, TreeSelectHdl));
    aDatabaseTLB.SetDoubleClickHdl(LINK(this, SwFldDBPage, InsertHdl));

    aValueED.SetModifyHdl(LINK(this, SwFldDBPage, ModifyHdl));
    aAddDBPB.SetClickHdl(LINK(this, SwFldDBPage, AddDBHdl));
}

SwFldDBPage::~SwFldDBPage()
{
}

/*--------------------------------------------------------------------
    Description: initialise TabPage
 --------------------------------------------------------------------*/
void SwFldDBPage::Reset(const SfxItemSet&)
{
    Init(); // Allgemeine initialisierung

    aTypeLB.SetUpdateMode(FALSE);
    USHORT nOldPos = aTypeLB.GetSelectEntryPos();
    sOldDBName = aDatabaseTLB.GetDBName(sOldTableName, sOldColumnName);

    aTypeLB.Clear();

    USHORT nPos, nTypeId, i;

    if (!IsFldEdit())
    {
        // initialise TypeListBox
        const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

        for(i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = GetFldMgr().GetTypeId(i);
            nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(i));
            aTypeLB.SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }
    else
    {
        nTypeId = GetCurField()->GetTypeId();
        nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(GetFldMgr().GetPos(nTypeId)));
        aTypeLB.SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
    }

    // select old Pos
    if (GetTypeSel() != LISTBOX_ENTRY_NOTFOUND)
        aTypeLB.SelectEntryPos(GetTypeSel());

    aFormatLB.Clear();

    USHORT nSize = GetFldMgr().GetFormatCount(TYP_DBSETNUMBERFLD, FALSE, IsFldDlgHtmlMode());
    for( i = 0; i < nSize; ++i )
    {
        USHORT nEntryPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr(TYP_DBSETNUMBERFLD, i));
        USHORT nFmtId = GetFldMgr().GetFormatId( TYP_DBSETNUMBERFLD, i );
        aFormatLB.SetEntryData( nEntryPos, reinterpret_cast<void*>(nFmtId) );
        if( SVX_NUM_ARABIC == nFmtId )
            aFormatLB.SelectEntryPos( nEntryPos );
    }

    if (!IsFldEdit())
    {
        if (nOldPos != LISTBOX_ENTRY_NOTFOUND)
            aTypeLB.SelectEntryPos(nOldPos);

        if (sOldDBName.Len())
        {
            aDatabaseTLB.Select(sOldDBName, sOldTableName, sOldColumnName);
        }
        else
        {
            SwWrtShell *pSh = GetWrtShell();
            if(!pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh)
            {
                SwDBData aTmp(pSh->GetDBData());
                aDatabaseTLB.Select(aTmp.sDataSource, aTmp.sCommand, aEmptyStr);
            }
        }
    }

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if(sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
        {
            String sVal = sUserData.GetToken(1, ';');
            USHORT nVal = (USHORT)sVal.ToInt32();
            if(nVal != USHRT_MAX)
            {
                for(i = 0; i < aTypeLB.GetEntryCount(); i++)
                    if(nVal == (USHORT)(ULONG)aTypeLB.GetEntryData(i))
                    {
                        aTypeLB.SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(0);

    aTypeLB.SetUpdateMode(TRUE);
    aTypeLB.SetSelectHdl(LINK(this, SwFldDBPage, TypeHdl));
    aTypeLB.SetDoubleClickHdl(LINK(this, SwFldDBPage, InsertHdl));

    if (IsFldEdit())
    {
        aConditionED.SaveValue();
        aValueED.SaveValue();
        sOldDBName = aDatabaseTLB.GetDBName(sOldTableName, sOldColumnName);
        nOldFormat = GetCurField()->GetFormat();
        nOldSubType = GetCurField()->GetSubType();
    }
}

BOOL SwFldDBPage::FillItemSet(SfxItemSet& )
{
    String sTableName, sColumnName;
    SwDBData aData;
    sal_Bool bIsTable;
    aData.sDataSource = aDatabaseTLB.GetDBName(sTableName, sColumnName, &bIsTable);
    aData.sCommand = sTableName;
    aData.nCommandType = bIsTable ? 0 : 1;
    SwWrtShell *pSh = GetWrtShell();
    if(!pSh)
        pSh = ::GetActiveWrtShell();

    if (!aData.sDataSource.getLength())
        aData = pSh->GetDBData();

    if(aData.sDataSource.getLength())       // without database no new field command
    {
        USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());
        String aVal(aValueED.GetText());
        String aName(aConditionED.GetText());
        ULONG nFormat = 0;
        USHORT nSubType = 0;

        String sDBName = aData.sDataSource;
        sDBName += DB_DELIM;
        sDBName += (String)aData.sCommand;
        sDBName += DB_DELIM;
        sDBName += String::CreateFromInt32(aData.nCommandType);
        sDBName += DB_DELIM;
        if(sColumnName.Len())
        {
            sDBName += sColumnName;
            sDBName += DB_DELIM;
        }
        aName.Insert(sDBName, 0);

        switch (nTypeId)
        {
        case TYP_DBFLD:
            nFormat = aNumFormatLB.GetFormat();
            if (aNewFormatRB.IsEnabled() && aNewFormatRB.IsChecked())
                nSubType = nsSwExtendedSubType::SUB_OWN_FMT;
            aName = sDBName;
            break;

        case TYP_DBSETNUMBERFLD:
            nFormat = (USHORT)(ULONG)aFormatLB.GetEntryData(
                                aFormatLB.GetSelectEntryPos() );
            break;
        }


        String sTempDBName, sTempTableName, sTempColumnName;
        sTempDBName = aDatabaseTLB.GetDBName(sTempTableName, sTempColumnName);
        BOOL bDBListBoxChanged = sOldDBName != sTempDBName ||
            sOldTableName != sTempTableName || sOldColumnName != sTempColumnName;
        if (!IsFldEdit() ||
            aConditionED.GetSavedValue() != aConditionED.GetText() ||
             aValueED.GetSavedValue() != aValueED.GetText() ||
             bDBListBoxChanged ||
             nOldFormat != nFormat || nOldSubType != nSubType)
        {
            InsertFld( nTypeId, nSubType, aName, aVal, nFormat);
        }
    }

    return FALSE;
}

SfxTabPage* SwFldDBPage::Create(    Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldDBPage( pParent, rAttrSet ) );
}

USHORT SwFldDBPage::GetGroup()
{
    return GRP_DB;
}

IMPL_LINK( SwFldDBPage, TypeHdl, ListBox *, pBox )
{
    // save old ListBoxPos
    const USHORT nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(aTypeLB.GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        SetTypeSel(0);
        aTypeLB.SelectEntryPos(0);
    }

    if (nOld != GetTypeSel())
    {
        SwWrtShell *pSh = GetWrtShell();
        if(!pSh)
            pSh = ::GetActiveWrtShell();
        BOOL bCond = FALSE, bSetNo = FALSE, bFormat = FALSE, bDBFormat = FALSE;
        USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

        aDatabaseTLB.ShowColumns(nTypeId == TYP_DBFLD);

        if (IsFldEdit())
        {
            SwDBData aData;
            String sColumnName;
            if (nTypeId == TYP_DBFLD)
            {
                aData = ((SwDBField*)GetCurField())->GetDBData();
                sColumnName = ((SwDBFieldType*)GetCurField()->GetTyp())->GetColumnName();
            }
            else
            {
                aData = ((SwDBNameInfField*)GetCurField())->GetDBData(pSh->GetDoc());
            }
            aDatabaseTLB.Select(aData.sDataSource, aData.sCommand, sColumnName);
        }

        switch (nTypeId)
        {
            case TYP_DBFLD:
                bFormat = TRUE;
                bDBFormat = TRUE;
                aNumFormatLB.Show();
                aFormatLB.Hide();

                if (pBox)   // type was changed by user
                    aDBFormatRB.Check();

                if (IsFldEdit())
                {
                    if (GetCurField()->GetFormat() != 0 && GetCurField()->GetFormat() != SAL_MAX_UINT32)
                        aNumFormatLB.SetDefFormat(GetCurField()->GetFormat());

                    if (GetCurField()->GetSubType() & nsSwExtendedSubType::SUB_OWN_FMT)
                        aNewFormatRB.Check();
                    else
                        aDBFormatRB.Check();
                }
                break;

            case TYP_DBNUMSETFLD:
                bSetNo = TRUE;
                // no break!
            case TYP_DBNEXTSETFLD:
                bCond = TRUE;
                if (IsFldEdit())
                {
                    aConditionED.SetText(GetCurField()->GetPar1());
                    aValueED.SetText(GetCurField()->GetPar2());
                }
                break;

            case TYP_DBNAMEFLD:
                break;

            case TYP_DBSETNUMBERFLD:
                bFormat = TRUE;
                aNewFormatRB.Check();
                aNumFormatLB.Hide();
                aFormatLB.Show();
                if( IsFldEdit() )
                {
                    for( USHORT nI = aFormatLB.GetEntryCount(); nI; )
                        if( GetCurField()->GetFormat() == (USHORT)(ULONG)
                            aFormatLB.GetEntryData( --nI ))
                        {
                            aFormatLB.SelectEntryPos( nI );
                            break;
                        }
                }
                break;
        }

        aConditionFT.Enable(bCond);
        aConditionED.Enable(bCond);
        aValueFT.Enable(bSetNo);
        aValueED.Enable(bSetNo);
        if (nTypeId != TYP_DBFLD)
        {
            aDBFormatRB.Enable(bDBFormat);
            aNewFormatRB.Enable(bDBFormat|bFormat);
            aNumFormatLB.Enable(bDBFormat);
            aFormatLB.Enable(bFormat);
        }
        aFormatFL.Enable(bDBFormat|bFormat);

        if (!IsFldEdit())
        {
            aValueED.SetText(aEmptyStr);
            if (bCond)
                aConditionED.SetText( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( "TRUE" )));
            else
                aConditionED.SetText(aEmptyStr);
        }

        CheckInsert();
    }

    return 0;
}

IMPL_LINK( SwFldDBPage, NumSelectHdl, NumFormatListBox *, pLB )
{
    aNewFormatRB.Check();
    aOldNumSelectHdl.Call(pLB);

    return 0;
}

void SwFldDBPage::CheckInsert()
{
    BOOL bInsert = TRUE;
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    SvLBoxEntry* pEntry = aDatabaseTLB.GetCurEntry();

    if (pEntry)
    {
        pEntry = aDatabaseTLB.GetParent(pEntry);

        if (nTypeId == TYP_DBFLD && pEntry)
            pEntry = aDatabaseTLB.GetParent(pEntry);

        bInsert &= pEntry != 0;
    }
    else
        bInsert = FALSE;

    if (nTypeId == TYP_DBNUMSETFLD)
    {
        BOOL bHasValue = aValueED.GetText().Len() != 0;

        bInsert &= bHasValue;
    }

    EnableInsert(bInsert);
}

IMPL_LINK( SwFldDBPage, TreeSelectHdl, SvTreeListBox *, pBox )
{
    SvLBoxEntry* pColEntry;
    SvLBoxEntry* pEntry = pColEntry = pBox->GetCurEntry();
    if (pEntry)
    {
        USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

        pEntry = aDatabaseTLB.GetParent(pEntry);

        if (nTypeId == TYP_DBFLD && pEntry)
            pEntry = aDatabaseTLB.GetParent(pEntry);

        CheckInsert();

        if (nTypeId == TYP_DBFLD)
        {
            BOOL bNumFormat = FALSE;

            if (pEntry != 0)
            {
                String sTableName;
                String sColumnName;
                BOOL bIsTable;
                String sDBName = aDatabaseTLB.GetDBName(sTableName, sColumnName, &bIsTable);
                bNumFormat = GetFldMgr().IsDBNumeric(sDBName,
                            sTableName,
                            bIsTable,
                            sColumnName);
                if (!IsFldEdit())
                    aDBFormatRB.Check();
            }

            aDBFormatRB.Enable(bNumFormat);
            aNewFormatRB.Enable(bNumFormat);
            aNumFormatLB.Enable(bNumFormat);
            aFormatFL.Enable(bNumFormat);
        }
    }
    return 0;
}

IMPL_LINK( SwFldDBPage, AddDBHdl, PushButton *, EMPTYARG )
{
    String sNewDB = SwNewDBMgr::LoadAndRegisterDataSource();
    if(sNewDB.Len())
    {
        aDatabaseTLB.AddDataSource(sNewDB);
    }
    return 0;
}

/*--------------------------------------------------------------------
    Description: Modify
 --------------------------------------------------------------------*/
IMPL_LINK( SwFldDBPage, ModifyHdl, Edit *, EMPTYARG )
{
    CheckInsert();
    return 0;
}

void    SwFldDBPage::FillUserData()
{
    String sData( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( USER_DATA_VERSION )));
    sData += ';';
    USHORT nTypeSel = aTypeLB.GetSelectEntryPos();

    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = (USHORT)(ULONG)aTypeLB.GetEntryData( nTypeSel );
    sData += String::CreateFromInt32( nTypeSel );
    SetUserData(sData);
}

void SwFldDBPage::ActivateMailMergeAddress()
{
    ULONG nData = TYP_DBFLD;
    aTypeLB.SelectEntryPos(aTypeLB.GetEntryPos( (const void*) nData ));
    aTypeLB.GetSelectHdl().Call(&aTypeLB);
    const SwDBData& rData = SW_MOD()->GetDBConfig()->GetAddressSource();
    aDatabaseTLB.Select(rData.sDataSource, rData.sCommand, aEmptyStr);
}

void SwFldDBPage::SetWrtShell(SwWrtShell& rSh)
{
    aDatabaseTLB.SetWrtShell(rSh);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
