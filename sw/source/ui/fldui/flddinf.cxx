/*************************************************************************
 *
 *  $RCSfile: flddinf.cxx,v $
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

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif

#ifndef _FLDTDLG_HRC
#include <fldtdlg.hrc>
#endif
#ifndef _FLDDINF_HXX
#include <flddinf.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldDokInfPage::SwFldDokInfPage(Window* pWindow, const SfxItemSet& rCoreSet ) :
    SwFldPage( pWindow, SW_RES( TP_FLD_DOKINF ), rCoreSet ),

    aTypeFT     (this, SW_RES(FT_DOKINFTYPE)),
    aTypeTLB    (this, SW_RES(TLB_DOKINFTYPE)),
    aSelectionFT(this, SW_RES(FT_DOKINFSELECTION)),
    aSelectionLB(this, SW_RES(LB_DOKINFSELECTION)),
    aFormatFT   (this, SW_RES(FT_DOKINFFORMAT)),
    aFormatLB   (this, SW_RES(LB_DOKINFFORMAT)),
    aFixedCB    (this, SW_RES(CB_DOKINFFIXEDCONTENT)),

    aRootOpened (SW_RES(BMP_DOKINFROOT_OPENED)),
    aRootClosed (SW_RES(BMP_DOKINFROOT_CLOSED)),
    aInfoStr    (SW_RES(STR_DOKINF_INFO)),
    pSelEntry   (0)
{
    FreeResource();

    aTypeTLB.SetHelpId(HID_FIELD_DINF_TYPE);
    aTypeTLB.SetSelectionMode(SINGLE_SELECTION);
    aTypeTLB.SetWindowBits(WB_HASLINES|WB_CLIPCHILDREN|WB_SORT|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    // Font nicht setzen, damit der Font des Controls uebernommen wird!
    // Sonst bei falschem Font Bug an OV.
    aTypeTLB.SetSpaceBetweenEntries(0);

    aTypeTLB.SetNodeBitmaps( aRootOpened, aRootClosed );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

__EXPORT SwFldDokInfPage::~SwFldDokInfPage()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void __EXPORT SwFldDokInfPage::Reset(const SfxItemSet& rSet)
{
    Init(); // Allgemeine initialisierung

    // TypeListBox initialisieren
    aTypeTLB.SetUpdateMode(FALSE);
    aTypeTLB.Clear();
    pSelEntry = 0;

    // SubTypes in der TypeLB anzeigen
    USHORT nTypeId = TYP_DOCINFOFLD;
    SvLBoxEntry* pEntry = 0;

    SvLBoxEntry* pInfo = 0;

    USHORT nSubType = USHRT_MAX;

    if (IsFldEdit())
        nSubType = ((SwDocInfoField*)GetCurField())->GetSubType() & 0xff;

    USHORT nSelEntryData = USHRT_MAX;
    String sUserData = GetUserData();
    if(sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
    {
        String sVal = sUserData.GetToken(1, ';');
        nSelEntryData = sVal.ToInt32();
    }

    SvStringsDtor& rLst = GetFldMgr().GetSubTypes(nTypeId);
    for (USHORT i = 0; i < rLst.Count(); ++i)
    {
        if (!IsFldEdit() || nSubType == i)
        {
            if (i >= DI_INFO1 && i <= DI_INFO4)
            {
                if (!pInfo && !IsFldEdit())
                {
                    pInfo = aTypeTLB.InsertEntry(aInfoStr);
                    pInfo->SetUserData((void*)USHRT_MAX);
                }
                pEntry = aTypeTLB.InsertEntry(*rLst[i], pInfo);
                pEntry->SetUserData((void*)i);
            }
            else
            {
                if (!(IsFldDlgHtmlMode() && (i == DI_EDIT || i == DI_THEMA || i == DI_PRINT)))
                {
                    pEntry = aTypeTLB.InsertEntry(*rLst[i]);
                    pEntry->SetUserData((void*)i);
                }
            }
            if(nSelEntryData == i)
                pSelEntry = pEntry;
        }
    }

    // alte Pos selektieren
    if (pSelEntry != 0)
    {
        aTypeTLB.Select(pSelEntry);
        nSubType = (USHORT)(ULONG)pSelEntry->GetUserData();
    }
    else
    {
        pSelEntry = aTypeTLB.GetEntry(0);
        nSubType = (USHORT)(ULONG)pSelEntry->GetUserData();
    }

    FillSelectionLB(nSubType);
    TypeHdl();

    aTypeTLB.SetUpdateMode(TRUE);
    aTypeTLB.SetSelectHdl(LINK(this, SwFldDokInfPage, TypeHdl));
    aTypeTLB.SetDoubleClickHdl(LINK(this, SwFldDokInfPage, InsertHdl));
    aSelectionLB.SetSelectHdl(LINK(this, SwFldDokInfPage, SubTypeHdl));
    aSelectionLB.SetDoubleClickHdl(LINK(this, SwFldDokInfPage, InsertHdl));
    aFormatLB.SetDoubleClickHdl(LINK(this, SwFldDokInfPage, InsertHdl));

    if (IsFldEdit())
    {
        nOldSel = aSelectionLB.GetSelectEntryPos();
        nOldFormat = GetCurField()->GetFormat();
        aFixedCB.SaveValue();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldDokInfPage, TypeHdl, ListBox *, EMPTYARG )
{
    // Alte ListBoxPos sichern
    SvLBoxEntry* pOldEntry = pSelEntry;

    // Aktuelle ListBoxPos
    pSelEntry = aTypeTLB.FirstSelected();

    if(!pSelEntry)
    {
        pSelEntry = aTypeTLB.GetEntry(0);
        aTypeTLB.Select(pSelEntry);
    }
    else

    if (pOldEntry != pSelEntry)
        FillSelectionLB((USHORT)(ULONG)pSelEntry->GetUserData());

    SubTypeHdl();

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldDokInfPage, SubTypeHdl, ListBox *, pBox )
{
    USHORT nSubType = (USHORT)(ULONG)pSelEntry->GetUserData();
    USHORT nPos = aSelectionLB.GetSelectEntryPos();
    USHORT nExtSubType;

    if (nSubType != DI_EDIT)
    {
        if (nPos == LISTBOX_ENTRY_NOTFOUND)
        {
            if (!aSelectionLB.GetEntryCount())
            {
                aFormatLB.Clear();
                aFormatLB.Enable(FALSE);
                aFormatFT.Enable(FALSE);
                return 0;
            }
            nPos = 0;
        }

        nExtSubType = (USHORT)(ULONG)aSelectionLB.GetEntryData(nPos);
    }
    else
        nExtSubType = DI_SUB_TIME;

    USHORT nOldType = 0;
    USHORT nNewType = 0;
    BOOL bEnable = FALSE;
    BOOL bOneArea = FALSE;

    if (aFormatLB.IsEnabled())
        nOldType = aFormatLB.GetFormatType();

    switch (nExtSubType)
    {
        case DI_SUB_AUTHOR:
            break;

        case DI_SUB_DATE:
            nNewType = NUMBERFORMAT_DATE;
            bOneArea = TRUE;
            break;

        case DI_SUB_TIME:
            nNewType = NUMBERFORMAT_TIME;
            bOneArea = TRUE;
            break;
    }

    if (!nNewType)
    {
        aFormatLB.Clear();
    }
    else
    {
        if (nOldType != nNewType)
        {
            aFormatLB.SetFormatType(nNewType);
            aFormatLB.SetOneArea(bOneArea);
        }
        bEnable = TRUE;
    }

    ULONG nFormat = IsFldEdit() ? ((SwDocInfoField*)GetCurField())->GetFormat() : 0;

    USHORT nOldSubType = IsFldEdit() ? (((SwDocInfoField*)GetCurField())->GetSubType() & 0xff00) : 0;

    if (IsFldEdit())
    {
        USHORT nPos = aSelectionLB.GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
        {
            nSubType = (USHORT)(ULONG)aSelectionLB.GetEntryData(nPos);

            nOldSubType &= ~DI_SUB_FIXED;
            if (nOldSubType == nSubType)
            {
                if (!nFormat && (nNewType == NUMBERFORMAT_DATE || nNewType == NUMBERFORMAT_TIME))
                {
                    SwWrtShell &rSh = ::GetActiveView()->GetWrtShell();
                    SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();
                    LanguageType eLang = aFormatLB.GetCurLanguage();

                    if (nNewType == NUMBERFORMAT_DATE)
                        nFormat = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_SHORT, eLang);
                    else if (nNewType == NUMBERFORMAT_TIME)
                        nFormat = pFormatter->GetFormatIndex( NF_TIME_HHMM, eLang);
                }
                aFormatLB.SetDefFormat(nFormat);
            }
        }
    }

    aFormatLB.Enable(bEnable);
    aFormatFT.Enable(bEnable);

    if (bEnable && aFormatLB.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND)
    {
        aFormatLB.SelectEntryPos(0);
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFldDokInfPage::FillSelectionLB(USHORT nSubType)
{
    // Format-Listbox fuellen
    USHORT nTypeId = TYP_DOCINFOFLD;

    EnableInsert(nSubType != USHRT_MAX);

    if (nSubType == USHRT_MAX)  // Info-Text
        nSubType = DI_INFO1;

    aSelectionLB.Clear();

    USHORT nSize = 0;
    USHORT nSelPos = USHRT_MAX;
    USHORT nExtSubType = IsFldEdit() ? (((SwDocInfoField*)GetCurField())->GetSubType() & 0xff00) : 0;

    if (IsFldEdit())
    {
        aFixedCB.Check((nExtSubType & DI_SUB_FIXED) != 0);
        nExtSubType = ((nExtSubType & ~DI_SUB_FIXED) >> 8) - 1;
    }

    if (nSubType < DI_CREATE || nSubType == DI_DOCNO || nSubType == DI_EDIT)
    {
        // Format Box ist fuer Title und Time leer
    }
    else
    {
        nSize = GetFldMgr().GetFormatCount(nTypeId, FALSE, IsFldDlgHtmlMode());
        for (USHORT i = 0; i < nSize; i++)
        {
            USHORT nPos = aSelectionLB.InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
            aSelectionLB.SetEntryData(nPos, (void*)GetFldMgr().GetFormatId(nTypeId, i));
            if (IsFldEdit() && i == nExtSubType)
                nSelPos = nPos;
        }
    }

    BOOL bEnable = nSize != 0;

    if (nSize)
    {
        if (!aSelectionLB.GetSelectEntryCount())
            aSelectionLB.SelectEntryPos(nSelPos == USHRT_MAX ? 0 : nSelPos);

        bEnable = TRUE;
    }

    aSelectionFT.Enable(bEnable);
    aSelectionLB.Enable(bEnable);

    return nSize;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL __EXPORT SwFldDokInfPage::FillItemSet(SfxItemSet& rSet)
{
    if (!pSelEntry || (USHORT)(ULONG)pSelEntry->GetUserData() == USHRT_MAX)
        return FALSE;

    USHORT nTypeId = TYP_DOCINFOFLD;
    USHORT nSubType = (USHORT)(ULONG)pSelEntry->GetUserData();

    ULONG nFormat = 0;

    USHORT nPos = aSelectionLB.GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
        nSubType |= (USHORT)(ULONG)aSelectionLB.GetEntryData(nPos);

    if (aFixedCB.IsChecked())
        nSubType |= DI_SUB_FIXED;

    nPos = aFormatLB.GetSelectEntryPos();
    if(nPos != LISTBOX_ENTRY_NOTFOUND)
        nFormat = aFormatLB.GetFormat();

    if (!IsFldEdit() || nOldSel != aSelectionLB.GetSelectEntryPos() ||
        nOldFormat != nFormat || aFixedCB.GetState() != aFixedCB.GetSavedValue())
    {
        InsertFld(nTypeId, nSubType, aEmptyStr, aEmptyStr, nFormat);
    }

    return FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxTabPage* __EXPORT SwFldDokInfPage::Create(   Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldDokInfPage( pParent, rAttrSet ) );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFldDokInfPage::GetGroup()
{
    return GRP_REG;
}
/* -----------------12.01.99 11:21-------------------
 *
 * --------------------------------------------------*/
void    SwFldDokInfPage::FillUserData()
{
    String sData( String::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM( USER_DATA_VERSION )));
    sData += ';';
    SvLBoxEntry* pEntry = aTypeTLB.FirstSelected();
    USHORT nTypeSel = pEntry ? (ULONG)pEntry->GetUserData() : USHRT_MAX;
    sData += String::CreateFromInt32( nTypeSel );
    SetUserData(sData);
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.21  2000/09/18 16:05:28  willem.vandorp
    OpenOffice header added.

    Revision 1.20  2000/06/30 08:52:52  os
    #76541# string assertions removed

    Revision 1.19  2000/05/23 18:36:37  jp
    Bugfixes for Unicode

    Revision 1.18  2000/04/18 15:17:31  os
    UNICODE

    Revision 1.17  2000/02/11 14:46:08  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.16  1999/02/19 07:48:26  MA
    #61949# CurrShell gibt es nicht mehr


      Rev 1.15   19 Feb 1999 08:48:26   MA
   #61949# CurrShell gibt es nicht mehr

      Rev 1.14   21 Jan 1999 09:46:24   OS
   #59900# Fussnoten im Dialog korrekt sortieren; keine prot. Member

      Rev 1.13   12 Jan 1999 11:42:24   OS
   #60579# ausgewaehlten Typ in den UserData speichern

      Rev 1.12   06 Mar 1998 14:53:52   OM
   Nur bei Aenderung Feld aktualisieren

      Rev 1.11   25 Feb 1998 16:47:58   OM
   Fixe Author- und ExtUser-Felder

      Rev 1.10   16 Feb 1998 10:10:22   OM
   Fuer Solaris nicht direkt von void* auf ushort casten

      Rev 1.9   12 Feb 1998 11:06:02   OM
   Zahlenformat fuer Docinfofelder

      Rev 1.8   03 Feb 1998 12:21:38   OM
   #46621# HelpIDs fuer Feldbefehl-Dlg

      Rev 1.7   19 Dec 1997 18:24:30   OM
   Feldbefehl-bearbeiten Dlg

      Rev 1.6   12 Dec 1997 16:10:06   OM
   AutoUpdate bei FocusWechsel u.a.

      Rev 1.5   11 Dec 1997 16:58:00   OM
   Feldumstellung

      Rev 1.4   10 Dec 1997 14:42:08   OM
   Fixe DocInfoFelder

      Rev 1.3   21 Nov 1997 17:19:46   OM
   Feldbefehl-Umstellung: DocInfo

      Rev 1.2   20 Nov 1997 17:01:22   OM
   Neuer Felddialog

      Rev 1.1   05 Nov 1997 15:35:38   OM
   Neuer Feldbefehldialog

      Rev 1.0   30 Oct 1997 14:30:02   OM
   Initial revision.

------------------------------------------------------------------------*/


