/*************************************************************************
 *
 *  $RCSfile: flddb.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-27 11:24:22 $
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

#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _FLDTDLG_HXX
#include <fldtdlg.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif

#ifndef _FLDTDLG_HRC
#include <fldtdlg.hrc>
#endif
#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#ifndef _FLDDB_HXX
#include <flddb.hxx>
#endif

#define USER_DATA_VERSION_1     "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldDBPage::SwFldDBPage(Window* pParent, const SfxItemSet& rCoreSet ) :
    SwFldPage( pParent, SW_RES( TP_FLD_DB ), rCoreSet ),
    aTypeFT     (this, SW_RES(FT_DBTYPE)),
    aTypeLB     (this, SW_RES(LB_DBTYPE)),
    aSelectionFT(this, SW_RES(FT_DBSELECTION)),
    aDatabaseTLB(this, SW_RES(TLB_DBLIST), aEmptyStr, FALSE),
    aConditionFT(this, SW_RES(FT_DBCONDITION)),
    aConditionED(this, SW_RES(ED_DBCONDITION)),
    aValueFT    (this, SW_RES(FT_DBSETNUMBER)),
    aValueED    (this, SW_RES(ED_DBSETNUMBER)),
    aDBFormatRB (this, SW_RES(RB_DBOWNFORMAT)),
    aNewFormatRB(this, SW_RES(RB_DBFORMAT)),
    aNumFormatLB(this, SW_RES(LB_DBNUMFORMAT)),
    aFormatLB   (this, SW_RES(LB_DBFORMAT)),
    aFormatGB   (this, SW_RES(GB_DBFORMAT))
{
    FreeResource();

    aOldNumSelectHdl = aNumFormatLB.GetSelectHdl();

    aNumFormatLB.SetSelectHdl(LINK(this, SwFldDBPage, NumSelectHdl));
    aDatabaseTLB.SetSelectHdl(LINK(this, SwFldDBPage, TreeSelectHdl));
    aDatabaseTLB.SetDoubleClickHdl(LINK(this, SwFldDBPage, InsertHdl));

    aValueED.SetModifyHdl(LINK(this, SwFldDBPage, ModifyHdl));
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

__EXPORT SwFldDBPage::~SwFldDBPage()
{
}

/*--------------------------------------------------------------------
    Beschreibung: TabPage initialisieren
 --------------------------------------------------------------------*/

void __EXPORT SwFldDBPage::Reset(const SfxItemSet& rSet)
{
    Init(); // Allgemeine initialisierung

    aTypeLB.SetUpdateMode(FALSE);
    USHORT nOldPos = aTypeLB.GetSelectEntryPos();
    sOldDBName = aDatabaseTLB.GetDBName(sOldTableName, sOldColumnName);

    aTypeLB.Clear();

    USHORT nPos, nTypeId, i;

    if (!IsFldEdit())
    {
        // TypeListBox initialisieren
        const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

        for(i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = GetFldMgr().GetTypeId(i);
            nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(i));
            aTypeLB.SetEntryData(nPos, (void*)nTypeId);
        }
    }
    else
    {
        nTypeId = GetCurField()->GetTypeId();
        nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(GetFldMgr().GetPos(nTypeId)));
        aTypeLB.SetEntryData(nPos, (void*)nTypeId);
    }

    // alte Pos selektieren
    if (GetTypeSel() != LISTBOX_ENTRY_NOTFOUND)
        aTypeLB.SelectEntryPos(GetTypeSel());

    aFormatLB.Clear();

    USHORT nSize = GetFldMgr().GetFormatCount(TYP_DBSETNUMBERFLD, FALSE, IsFldDlgHtmlMode());
    for( i = 0; i < nSize; ++i )
    {
        USHORT nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr(TYP_DBSETNUMBERFLD, i));
        USHORT nFmtId = GetFldMgr().GetFormatId( TYP_DBSETNUMBERFLD, i );
        aFormatLB.SetEntryData( nPos, (void*)nFmtId );
        if( SVX_NUM_ARABIC == nFmtId )
            aFormatLB.SelectEntryPos( nPos );
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
            SwWrtShell *pSh = ::GetActiveView()->GetWrtShellPtr();
            String sTmp(pSh->GetDBName());

            aDatabaseTLB.Select(
            sTmp.GetToken(0, DB_DELIM),
            sTmp.GetToken(1, DB_DELIM),
            sTmp.GetToken(2, DB_DELIM));
        }
    }

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if(sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
        {
            String sVal = sUserData.GetToken(1, ';');
            USHORT nVal = sVal.ToInt32();
            if(nVal != USHRT_MAX)
            {
                for(USHORT i = 0; i < aTypeLB.GetEntryCount(); i++)
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL __EXPORT SwFldDBPage::FillItemSet(SfxItemSet& rSet)
{
    String sTableName, sColumnName;
    String sDBName = aDatabaseTLB.GetDBName(sTableName, sColumnName);
    SwWrtShell *pSh = ::GetActiveView()->GetWrtShellPtr();

    if (!sDBName.Len())
    {
        String sTmp = pSh->GetDBName();
        sDBName = sTmp.GetToken(0, DB_DELIM);
        sTableName = sTmp.GetToken(1, DB_DELIM);
    }
    else
    {
        String sNewDBName = sDBName;
        sNewDBName += DB_DELIM;
        sNewDBName += sTableName;
        pSh->ChgDBName(sNewDBName);
    }

    if (sDBName.Len())      // Ohne Datenbank kein neuer Feldbefehl
    {
        USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());
        String aVal(aValueED.GetText());
        String aName(aConditionED.GetText());
        ULONG nFormat = 0;
        USHORT nSubType = 0;

        sDBName += DB_DELIM;
        sDBName += sTableName;
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
                nSubType = SUB_OWN_FMT;
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxTabPage* __EXPORT SwFldDBPage::Create(   Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldDBPage( pParent, rAttrSet ) );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFldDBPage::GetGroup()
{
    return GRP_DB;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldDBPage, TypeHdl, ListBox *, pBox )
{
    // Alte ListBoxPos sichern
    const USHORT nOld = GetTypeSel();

    // Aktuelle ListBoxPos
    SetTypeSel(aTypeLB.GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        SetTypeSel(0);
        aTypeLB.SelectEntryPos(0);
    }

    if (nOld != GetTypeSel())
    {
        SwWrtShell *pSh = ::GetActiveView()->GetWrtShellPtr();
        BOOL bCond = FALSE, bSetNo = FALSE, bFormat = FALSE, bDBFormat = FALSE;
        USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

        aDatabaseTLB.ShowColumns(nTypeId == TYP_DBFLD);

        if (IsFldEdit())
        {
            String sDBName, sTableName, sColumnName;
            if (nTypeId == TYP_DBFLD)
            {
                sDBName = ((SwDBField*)GetCurField())->GetDBName();
                sColumnName = ((SwDBFieldType*)GetCurField()->GetTyp())->GetColumnName();
            }
            else
            {
                sDBName = ((SwDBNameInfField*)GetCurField())->GetDBName(pSh->GetDoc());
            }
            sTableName = sDBName.GetToken(1, DB_DELIM);
            sDBName = sDBName.GetToken(0, DB_DELIM);

            aDatabaseTLB.Select(sDBName, sTableName, sColumnName);
        }

        switch (nTypeId)
        {
            case TYP_DBFLD:
                bFormat = TRUE;
                bDBFormat = TRUE;
                aNumFormatLB.Show();
                aFormatLB.Hide();

                if (pBox)   // Typ wurde vom User geaendert
                    aDBFormatRB.Check();

                if (IsFldEdit())
                {
                    if (GetCurField()->GetFormat() != 0 && GetCurField()->GetFormat() != ULONG_MAX)
                        aNumFormatLB.SetDefFormat(GetCurField()->GetFormat());

                    if (GetCurField()->GetSubType() & SUB_OWN_FMT)
                        aNewFormatRB.Check();
                    else
                        aDBFormatRB.Check();
                }
                break;

            case TYP_DBNUMSETFLD:
                bSetNo = TRUE;
                // kein break!
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
        aFormatGB.Enable(bDBFormat|bFormat);

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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldDBPage, NumSelectHdl, NumFormatListBox *, pLB )
{
    aNewFormatRB.Check();
    aOldNumSelectHdl.Call(pLB);

    return 0;
}

/*---------------------------------------------------------------------
    Beschreibung:
---------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------
    Beschreibung:
---------------------------------------------------------------------*/

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
            aFormatGB.Enable(bNumFormat);
        }
    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: Modify
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldDBPage, ModifyHdl, Edit *, EMPTYARG )
{
    CheckInsert();
    return 0;
}

/* -----------------12.01.99 11:21-------------------
 *
 * --------------------------------------------------*/
void    SwFldDBPage::FillUserData()
{
    String sData( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( USER_DATA_VERSION )));
    sData += ';';
    USHORT nTypeSel = aTypeLB.GetSelectEntryPos();

    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = (ULONG)aTypeLB.GetEntryData( nTypeSel );
    sData += String::CreateFromInt32( nTypeSel );
    SetUserData(sData);
}

