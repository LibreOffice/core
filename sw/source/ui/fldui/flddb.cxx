/*************************************************************************
 *
 *  $RCSfile: flddb.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:36 $
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
#ifdef REPLACE_OFADBMGR
    sOldDBName = aDatabaseTLB.GetDBName(sOldTableName, sOldColumnName);
#else
    sOldDBName = aDatabaseTLB.GetDBName();
#endif

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
#ifdef REPLACE_OFADBMGR
            aDatabaseTLB.Select(sOldDBName, sOldTableName, sOldColumnName);
#else
            aDatabaseTLB.Select(sOldDBName);
#endif
        }
        else
        {
            SwWrtShell *pSh = ::GetActiveView()->GetWrtShellPtr();
#ifdef REPLACE_OFADBMGR
            String sTmp(pSh->GetDBName());

            aDatabaseTLB.Select(
                sTmp.GetToken(0, DB_DELIM),
                sTmp.GetToken(1, DB_DELIM),
                sTmp.GetToken(2, DB_DELIM));
#else
            aDatabaseTLB.Select(pSh->GetDBName());
#endif
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
#ifdef REPLACE_OFADBMGR
        sOldDBName = aDatabaseTLB.GetDBName(sOldTableName, sOldColumnName);
#else
        sOldDBName = aDatabaseTLB.GetDBName();
#endif
        nOldFormat = GetCurField()->GetFormat();
        nOldSubType = GetCurField()->GetSubType();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL __EXPORT SwFldDBPage::FillItemSet(SfxItemSet& rSet)
{
#ifdef REPLACE_OFADBMGR
    String sTableName, sColumnName;
    String sDBName = aDatabaseTLB.GetDBName(sTableName, sColumnName);
#else
    String sDBName = aDatabaseTLB.GetDBName();
#endif
    SwWrtShell *pSh = ::GetActiveView()->GetWrtShellPtr();

    if (!sDBName.Len())
    {
#ifdef REPLACE_OFADBMGR
        String sTmp = pSh->GetDBName();
        sDBName = sTmp.GetToken(0, DB_DELIM);
        sTableName = sTmp.GetToken(1, DB_DELIM);
#else
        sDBName = pSh->GetDBName();
#endif
    }
    else
    {
#ifdef REPLACE_OFADBMGR
        String sNewDBName = sDBName;
        sNewDBName += DB_DELIM;
        sNewDBName += sTableName;
        pSh->ChgDBName(sNewDBName);
#else
        // keine Spaltennamen an ChgDBName uebergeben
        String sNewDBName = sDBName.GetToken(0, DB_DELIM);
        sNewDBName += DB_DELIM;
        sNewDBName += sDBName.GetToken(1, DB_DELIM);
        pSh->ChgDBName(sNewDBName);
#endif
    }

    if (sDBName.Len())      // Ohne Datenbank kein neuer Feldbefehl
    {
        USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());
        String aVal(aValueED.GetText());
        String aName(aConditionED.GetText());
        ULONG nFormat = 0;
        USHORT nSubType = 0;

        sDBName += DB_DELIM;
#ifdef REPLACE_OFADBMGR
        sDBName += sTableName;
        sDBName += DB_DELIM;
        if(sColumnName.Len())
        {
            sDBName += sColumnName;
            sDBName += DB_DELIM;
        }
#endif
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


#ifdef REPLACE_OFADBMGR
        String sTempDBName, sTempTableName, sTempColumnName;
        sTempDBName = aDatabaseTLB.GetDBName(sTempTableName, sTempColumnName);
        BOOL bDBListBoxChanged = sOldDBName != sTempDBName ||
            sOldTableName != sTempTableName || sOldColumnName != sTempColumnName;
#else
        BOOL bDBListBoxChanged = sOldDBName != aDatabaseTLB.GetDBName();
#endif
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
#ifdef REPLACE_OFADBMGR
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
#else
            String sDBName;
            if (nTypeId == TYP_DBFLD)
            {
                sDBName = ((SwDBField*)GetCurField())->GetDBName();
                sDBName += DB_DELIM;
                sDBName += ((SwDBFieldType*)GetCurField()->GetTyp())->GetColumnName();
            }
            else
                sDBName = ((SwDBNameInfField*)GetCurField())->GetDBName(pSh->GetDoc());

            aDatabaseTLB.Select(sDBName);
#endif
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
#ifdef REPLACE_OFADBMGR
                String sTableName;
                String sColumnName;
                BOOL bIsTable;
                String sDBName = aDatabaseTLB.GetDBName(sTableName, sColumnName, &bIsTable);
                bNumFormat = GetFldMgr().IsDBNumeric(sDBName,
                            sTableName,
                            bIsTable,
                            sColumnName);
#else
                String sName(aDatabaseTLB.GetDBName());
                String sColumnName(sName.GetToken(2, DB_DELIM));
                String sDBName(sName.GetToken(0, DB_DELIM));
                sDBName += DB_DELIM;
                sDBName += sName.GetToken(1, DB_DELIM);
                bNumFormat = GetFldMgr().IsDBNumeric(sDBName, sColumnName);
#endif
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
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.31  2000/09/18 16:05:28  willem.vandorp
    OpenOffice header added.

    Revision 1.30  2000/07/07 15:25:43  os
    replace ofadbmgr

    Revision 1.29  2000/06/30 08:52:52  os
    #76541# string assertions removed

    Revision 1.28  2000/06/26 13:35:59  os
    new DataBase API

    Revision 1.27  2000/05/23 18:36:33  jp
    Bugfixes for Unicode

    Revision 1.26  2000/04/18 15:17:31  os
    UNICODE

    Revision 1.25  1999/02/25 16:24:50  JP
    Bug #62438#: UserData nur auswerten, wenn kein Refresh ist


      Rev 1.24   25 Feb 1999 17:24:50   JP
   Bug #62438#: UserData nur auswerten, wenn kein Refresh ist

      Rev 1.23   21 Jan 1999 09:50:36   OS
   #59900# Fussnoten im Dialog korrekt sortieren; keine prot. Member

      Rev 1.22   12 Jan 1999 11:42:26   OS
   #60579# ausgewaehlten Typ in den UserData speichern

      Rev 1.21   11 Dec 1998 15:16:24   OM
   #60051# Richtiges Format fuer Serienbrieffelder waehlen

      Rev 1.20   17 Nov 1998 10:50:38   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.19   04 Nov 1998 13:27:56   OM
   #58939# Bei Formataenderung RadioButton vorselektieren

      Rev 1.18   10 Aug 1998 16:39:58   JP
   Bug #54796#: neue NumerierungsTypen (WW97 kompatibel)

      Rev 1.17   09 Jul 1998 09:52:58   JP
   EmptyStr benutzen

      Rev 1.16   10 Jun 1998 13:26:08   OM
   Alten Typ nach Reset restaurieren

      Rev 1.15   27 Mar 1998 15:23:54   OM
   #48909# Insert-Btn korrekt enablen/disablen

      Rev 1.14   06 Mar 1998 13:07:48   OM
   Nur bei Aenderung Feld aktualisieren

      Rev 1.13   04 Mar 1998 08:32:24   MH
   chg: Syntax

      Rev 1.12   16 Feb 1998 09:39:00   OM
   Fuer Solaris nicht direkt von void* auf ushort casten

      Rev 1.11   06 Jan 1998 18:12:54   OM
   Felbefehl-Dlg

      Rev 1.10   05 Jan 1998 17:44:34   OM
   DB-Feldbefehl bearbeiten

      Rev 1.9   12 Dec 1997 17:14:00   OM
   Listbox bei numerischen Formaten enablen

      Rev 1.8   11 Dec 1997 16:58:00   OM
   Feldumstellung

      Rev 1.7   20 Nov 1997 17:01:20   OM
   Neuer Felddialog

      Rev 1.6   19 Nov 1997 16:30:42   OM
   Datenbank-TP Drag&Drop

      Rev 1.5   18 Nov 1997 10:34:36   OM
   Neuer Feldbefehldialog

      Rev 1.4   17 Nov 1997 09:06:36   OM
   Basisklasse fuer Feldbefehl-TPs

      Rev 1.3   05 Nov 1997 17:02:46   OM
   Spaltennamen anzeigen

      Rev 1.2   05 Nov 1997 15:35:38   OM
   Neuer Feldbefehldialog

      Rev 1.1   30 Oct 1997 14:31:42   OM
   Feldbefehl-Umstellung

      Rev 1.0   28 Oct 1997 15:05:06   OM
   Initial revision.

------------------------------------------------------------------------*/

