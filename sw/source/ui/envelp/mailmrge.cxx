/*************************************************************************
 *
 *  $RCSfile: mailmrge.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:35 $
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
#ifndef _SDB_SDBHASH_HXX
#include <sdb/sdbhash.hxx>
#endif
#ifndef _IODLG_HXX
#include "sfx2/iodlg.hxx"
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _MAILENUM_HXX //autogen
#include <goodies/mailenum.hxx>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#ifndef _SVX_MULTIFIL_HXX
#include <svx/multifil.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#include "helpid.h"
#include "view.hxx"
#include "docsh.hxx"
#include "wrtsh.hxx"
#include "dbmgr.hxx"
#include "dbui.hxx"
#include "swmodule.hxx"
#include "modcfg.hxx"

#include "envelp.hrc"
#include "mailmrge.hrc"
#include "mailmrge.hxx"

#ifdef REPLACE_OFADBMGR
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif

using namespace rtl;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::uno;

#endif

#define C2S(cChar) UniString::CreateFromAscii(cChar)

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SwMailMergeDlg::SwMailMergeDlg(Window *pParent, SwWrtShell *pShell,
            const String& rName,
#ifdef REPLACE_OFADBMGR
        const String& rTblName,
#endif
            const String& rStat,
            SbaSelectionListRef& pSelList):

    SvxStandardDialog(pParent, SW_RES(DLG_MAILMERGE)),

    aAllRB          (this, SW_RES(RB_ALL)),
    aMarkedRB       (this, SW_RES(RB_MARKED)),
    aFromRB         (this, SW_RES(RB_FROM)),
    aFromNF         (this, SW_RES(NF_FROM)),
    aBisFT          (this, SW_RES(FT_BIS)),
    aToNF           (this, SW_RES(NF_TO)),
    aRecordGB       (this, SW_RES(GB_RECORD)),

    aPrinterRB      (this, SW_RES(RB_PRINTER)),
    aMailingRB      (this, SW_RES(RB_MAILING)),
    aFileRB         (this, SW_RES(RB_FILE)),

    aSingleJobsCB   (this, SW_RES(CB_SINGLE_JOBS)),

    aPathFT         (this, SW_RES(FT_PATH)),
    aPathED         (this, SW_RES(ED_PATH)),
    aPathPB         (this, SW_RES(PB_PATH)),
    aFilenameFT     (this, SW_RES(FT_FILENAME)),
    aColumnRB       (this, SW_RES(RB_COLUMN)),
    aFilenameRB     (this, SW_RES(RB_FILENAME)),
    aColumnLB       (this, SW_RES(LB_COLUMN)),
    aFilenameED     (this, SW_RES(ED_FILENAME)),

    aAddressFT      (this, SW_RES(FT_ADDRESS)),
    aAddressFldLB   (this, SW_RES(LB_ADDRESSFLD)),
    aSubjectFT      (this, SW_RES(FT_SUBJECT)),
    aSubjectED      (this, SW_RES(ED_SUBJECT)),
    aFormatFT       (this, SW_RES(FT_FORMAT)),
    aAttachFT       (this, SW_RES(FT_ATTACH)),
    aAttachED       (this, SW_RES(ED_ATTACH)),
    aAttachPB       (this, SW_RES(PB_ATTACH)),
    aFormatSwCB     (this, SW_RES(CB_FORMAT_SW)),
    aFormatHtmlCB   (this, SW_RES(CB_FORMAT_HTML)),
    aFormatRtfCB    (this, SW_RES(CB_FORMAT_RTF)),
    aDestGB         (this, SW_RES(GB_DEST)),

    aOkBTN          (this, SW_RES(BTN_OK)),
    aCancelBTN      (this, SW_RES(BTN_CANCEL)),
    aHelpBTN        (this, SW_RES(BTN_HELP)),

    pSh             (pShell),
    rDBName         (rName),
#ifdef REPLACE_OFADBMGR
    rTableName      (rTblName),
#endif
    rStatement      (rStat),
    rSelectionList  (pSelList),
    nMergeType      (DBMGR_MERGE_MAILING)

{
    FreeResource();

    DBG_ASSERT(pSh, "Shell fehlt"  );

#ifdef REPLACE_OFADBMGR
#else
    pSbaObject = pSh->GetNewDBMgr()->GetSbaObject();
#endif
    pModOpt = SW_MOD()->GetModuleConfig();

    aSingleJobsCB.Check(pModOpt->IsSinglePrintJob());

    BYTE nMailingMode(pModOpt->GetMailingFormats());
    aFormatSwCB.Check((nMailingMode & TXTFORMAT_OFFICE) != 0);
    aFormatHtmlCB.Check((nMailingMode & TXTFORMAT_HTML) != 0);
    aFormatRtfCB.Check((nMailingMode & TXTFORMAT_RTF) != 0);

#ifdef REPLACE_OFADBMGR
#else
    DBG_ASSERT(pSbaObject, "SbaObject nicht gefunden!");
#endif
    aAllRB.Check(TRUE);

    // Handler installieren
    Link aLk = LINK(this, SwMailMergeDlg, ButtonHdl);
    aOkBTN.SetClickHdl(aLk);

    aPathPB.SetClickHdl(LINK(this, SwMailMergeDlg, InsertPathHdl));
    aAttachPB.SetClickHdl(LINK(this, SwMailMergeDlg, AttachFileHdl));

    aLk = LINK(this, SwMailMergeDlg, RadioButtonHdl);
    aPrinterRB.SetClickHdl(aLk);
    aMailingRB.SetClickHdl(aLk);
    aFileRB.SetClickHdl(aLk);

    aLk = LINK(this, SwMailMergeDlg, FilenameHdl);
    aColumnRB.SetClickHdl(aLk);
    aFilenameRB.SetClickHdl(aLk);
    BOOL bColumn = pModOpt->IsNameFromColumn();
    if (bColumn)
        aColumnRB.Check();
    else
        aFilenameRB.Check();

    FilenameHdl(bColumn ? &aColumnRB : &aFilenameRB);

    aLk = LINK(this, SwMailMergeDlg, ModifyHdl);
    aFromNF.SetModifyHdl(aLk);
    aToNF.SetModifyHdl(aLk);

#ifdef REPLACE_OFADBMGR
    pSh->GetNewDBMgr()->GetColumnNames(&aAddressFldLB, rDBName, rTableName);
#else
    pSh->GetNewDBMgr()->GetColumnNames(&aAddressFldLB, rDBName);
#endif
    for(USHORT nEntry = 0; nEntry < aAddressFldLB.GetEntryCount(); nEntry++)
        aColumnLB.InsertEntry(aAddressFldLB.GetEntry(nEntry));
    aAddressFldLB.SelectEntry(C2S("EMAIL"));

    aPathED.SetText(pModOpt->GetMailingPath());
    if (!aPathED.GetText().Len())
    {
        SfxIniManager* pIniManager = SFX_APP()->GetIniManager();
        ASSERT(pIniManager, "Kein Inimanager angelegt !!");
        aPathED.SetText(pIniManager->Get(SFX_KEY_WORK_PATH));
    }
    String sMailName = pModOpt->GetMailName();

    if (!bColumn || !sMailName.Len())
    {
        aColumnLB.SelectEntry(C2S("NAME"));
        aFilenameED.SetText(sMailName);
    }
    else
        aColumnLB.SelectEntry(sMailName);

    if (aAddressFldLB.GetSelectEntryCount() == 0)
        aAddressFldLB.SelectEntryPos(0);
    if (aColumnLB.GetSelectEntryCount() == 0)
        aColumnLB.SelectEntryPos(0);

    const BOOL bEnable = rSelectionList->Count() != 0;
    aMarkedRB.Enable(bEnable);
    if (bEnable && (long)rSelectionList->GetObject(0) != -1L)
        aMarkedRB.Check();
    else
    {
        aAllRB.Check();
        aMarkedRB.Enable(FALSE);
    }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SwMailMergeDlg::~SwMailMergeDlg()
{
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwMailMergeDlg::Apply()
{
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeDlg, ButtonHdl, Button *, pBtn )
{
    if (pBtn == &aOkBTN)
    {
        ExecQryShell(FALSE);
        EndDialog(RET_OK);
    }
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeDlg, RadioButtonHdl, RadioButton *, pBtn )
{
    if (pBtn == &aFileRB)
    {
        aAddressFT.Hide();
        aAddressFldLB.Hide();
        aSubjectFT.Hide();
        aSubjectED.Hide();
        aFormatFT.Hide();
        aFormatSwCB.Hide();
        aFormatHtmlCB.Hide();
        aFormatRtfCB.Hide();
        aSingleJobsCB.Hide();
        aAttachFT.Hide();
        aAttachED.Hide();
        aAttachPB.Hide();

        aPathFT.Show();
        aPathED.Show();
        aPathPB.Show();
        aFilenameFT.Show();
        aColumnRB.Show();
        aFilenameRB.Show();
        aColumnLB.Show();
        aFilenameED.Show();
    }
    else
    {
        aAddressFT.Show();
        aAddressFldLB.Show();
        aSubjectFT.Show();
        aSubjectED.Show();
        aFormatFT.Show();
        aFormatSwCB.Show();
        aFormatHtmlCB.Show();
        aFormatRtfCB.Show();
        aAttachFT.Show();
        aAttachED.Show();
        aAttachPB.Show();

        aPathFT.Hide();
        aPathED.Hide();
        aPathPB.Hide();
        aFilenameFT.Hide();
        aColumnRB.Hide();
        aFilenameRB.Hide();
        aColumnLB.Hide();
        aFilenameED.Hide();

        const BOOL bEnable = pBtn != &aPrinterRB;
        aAddressFT.Enable(bEnable);
        aAddressFldLB.Enable(bEnable);
        aSubjectFT.Enable(bEnable);
        aSubjectED.Enable(bEnable);
        aFormatFT.Enable(bEnable);
        aFormatSwCB.Enable(bEnable);
        aFormatHtmlCB.Enable(bEnable);
        aFormatRtfCB.Enable(bEnable);
        aAttachFT.Enable(bEnable);
        aAttachED.Enable(bEnable);
        aAttachPB.Enable(bEnable);

        aAddressFT.Show(bEnable);
        aAddressFldLB.Show(bEnable);
        aSingleJobsCB.Show(!bEnable);
        aSingleJobsCB.Enable(!bEnable);
    }
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeDlg, FilenameHdl, RadioButton *, pBtn )
{
    BOOL bEnable = pBtn == &aColumnRB;

    aColumnLB.Enable(bEnable);
    aFilenameED.Enable(!bEnable);

    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeDlg, ModifyHdl, NumericField *, pFld )
{
    aFromRB.Check();
    return (0);
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwMailMergeDlg::ExecQryShell(BOOL bVisible)
{
    SwNewDBMgr* pMgr = pSh->GetNewDBMgr();

    if (aPrinterRB.IsChecked())
        nMergeType = DBMGR_MERGE_MAILMERGE;
    else if (aMailingRB.IsChecked())
    {
        nMergeType = DBMGR_MERGE_MAILING;
        pMgr->SetEMailColumn(aAddressFldLB.GetSelectEntry());
        pMgr->SetSubject(aSubjectED.GetText());
        pMgr->SetAttachment(aAttachED.GetText());
    }
    else
    {
        nMergeType = DBMGR_MERGE_MAILFILES;
        String sPath(aPathED.GetText());
        pModOpt->SetMailingPath(sPath);
        String sDelim(INET_PATH_TOKEN);

        if (sPath.Len() >= sDelim.Len() &&
            sPath.Copy(sPath.Len()-sDelim.Len()).CompareTo(sDelim) != COMPARE_EQUAL)
            sPath += sDelim;

        pModOpt->SetNameFromColumn(aColumnRB.IsChecked());

        if (aColumnRB.IsChecked())
        {
            pMgr->SetEMailColumn(aColumnLB.GetSelectEntry());
            pModOpt->SetMailName(aColumnLB.GetSelectEntry());
        }
        else
        {
            String sName(aFilenameED.GetText());
            if (!sName.Len())
            {
                sName = pSh->GetView().GetDocShell()->GetTitle();
                INetURLObject aTemp(sName);
                sName = aTemp.GetBase();
            }
            sPath += sName;
            pMgr->SetEMailColumn(aEmptyStr);
            pModOpt->SetMailName(sName);
        }

        pMgr->SetSubject(sPath);
    }

    if (aFromRB.IsChecked())    // Liste Einfuegen
    {
        ULONG nStart = aFromNF.GetValue();
        ULONG nEnd = aToNF.GetValue();

        if (nEnd < nStart)
        {
            ULONG nZw = nEnd;
            nEnd = nStart;
            nStart = nZw;
        }

        rSelectionList->Clear();

        for (ULONG i = nStart; i <= nEnd; i++)
            rSelectionList->Insert((void*)i , LIST_APPEND);
    }

    if (aAllRB.IsChecked() )
        rSelectionList->Clear();    // Leere Selektion = Alles einfuegen

    pModOpt->SetSinglePrintJob(aSingleJobsCB.IsChecked());

    BYTE nMailingMode = 0;

    if (aFormatSwCB.IsChecked())
        nMailingMode |= TXTFORMAT_OFFICE;
    if (aFormatHtmlCB.IsChecked())
        nMailingMode |= TXTFORMAT_HTML;
    if (aFormatRtfCB.IsChecked())
        nMailingMode |= TXTFORMAT_RTF;
    pModOpt->SetMailingFormats(nMailingMode);
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeDlg, InsertPathHdl, PushButton *, pBtn )
{
    SfxIniManager* pIniManager = SFX_APP()->GetIniManager();
    ASSERT(pIniManager, "Kein Inimanager angelegt !!");
    String sPath(aPathED.GetText());
    if (!sPath.Len())
        sPath = pIniManager->Get(SFX_KEY_WORK_PATH);
    WinBits nBits = WB_3DLOOK|WB_STDMODAL|WB_OPEN|SFXWB_PATHDIALOG;

    SfxFileDialog* pFileDlg = new SfxFileDialog( this, nBits );
    pFileDlg->DisableSaveLastDirectory();
    pFileDlg->SetPath( sPath );
    pFileDlg->SetHelpId(HID_FILEDLG_MAILMRGE1);

    if (pFileDlg->Execute())
        aPathED.SetText(pFileDlg->GetPath());

    delete pFileDlg;
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeDlg, AttachFileHdl, PushButton *, pBtn )
{
    SvxMultiFileDialog* pFileDlg = new SvxMultiFileDialog(this);
    pFileDlg->SetFiles(aAttachED.GetText());
    pFileDlg->SetHelpId(HID_FILEDLG_MAILMRGE2);

    if (pFileDlg->Execute())
        aAttachED.SetText(pFileDlg->GetFiles());

    delete pFileDlg;

    return 0;
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.75  2000/09/18 16:05:27  willem.vandorp
    OpenOffice header added.

    Revision 1.74  2000/07/18 12:50:08  os
    replace ofadbmgr

    Revision 1.73  2000/06/07 13:26:46  os
    using UCB

    Revision 1.72  2000/04/19 12:56:34  os
    include sfx2/filedlg.hxx removed

    Revision 1.71  2000/04/18 15:31:35  os
    UNICODE

    Revision 1.70  2000/03/03 15:17:00  os
    StarView remainders removed

    Revision 1.69  2000/02/11 14:45:27  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.68  2000/01/24 12:48:37  os
    #72153# call SfxFileDialog::DisableSaveLastDirectory

    Revision 1.67  1998/09/02 12:11:56  OM
    #45378# HelpIDs fuer Dateidialoge


      Rev 1.66   02 Sep 1998 14:11:56   OM
   #45378# HelpIDs fuer Dateidialoge

      Rev 1.65   23 Jul 1998 13:26:52   OM
   #52257# Mail Attachments

      Rev 1.64   17 Jul 1998 17:19:52   TJ
   include

      Rev 1.63   09 Jul 1998 09:52:32   JP
   EmptyStr benutzen

      Rev 1.62   09 Dec 1997 12:31:28   OM
   #45200# Serienbrief: Speichern-Monitor

      Rev 1.61   02 Dec 1997 19:39:12   MA
   #45900#, SelectionList muss fuer Dialog existieren

      Rev 1.60   24 Nov 1997 11:52:12   MA
   includes

      Rev 1.59   03 Nov 1997 13:17:14   MA
   precomp entfernt

      Rev 1.58   30 Sep 1997 14:22:06   MH
   chg: header

      Rev 1.57   02 Sep 1997 09:58:22   OM
   SDB-Headeranpassung

      Rev 1.56   26 Aug 1997 15:34:46   TRI
   VCL Includes

      Rev 1.55   05 Aug 1997 12:31:44   MH
   chg: header

      Rev 1.54   08 Jul 1997 14:12:44   OS
   ConfigItems von der App ans Module

      Rev 1.53   09 Jun 1997 17:22:32   OM
   Serienbriefe als Dateien speichern

      Rev 1.52   04 Jun 1997 11:43:02   OM
   Korrektes Mailformat RTF

      Rev 1.51   20 Mar 1997 12:44:42   OM
   Mailing: Format RTF

      Rev 1.50   11 Mar 1997 11:27:18   OM
   Serienbrief-eMail in waehlbaren Formaten versenden

      Rev 1.49   14 Feb 1997 11:36:24   OM
   #36178# GPF im Serienbriefdruck behoben

      Rev 1.48   29 Jan 1997 18:13:04   MA
   unbenutzes entfernt

      Rev 1.47   06 Dec 1996 14:39:36   OM
   OpenDB-Fehlercode korrigiert

      Rev 1.46   05 Dec 1996 13:36:58   OM
   Serienbrief reanimiert

      Rev 1.45   11 Nov 1996 09:44:18   MA
   ResMgr

      Rev 1.44   07 Oct 1996 09:33:18   MA
   Umstellung Enable/Disable

      Rev 1.43   18 Sep 1996 10:38:54   OM
   Serienbriefe wieder angebunden

      Rev 1.42   13 Sep 1996 15:46:48   OM
   Serienbrief

      Rev 1.41   22 Aug 1996 12:30:32   OM
   Serienbrief Dlg umgestellt

      Rev 1.40   06 Aug 1996 16:45:38   OM
   Datenbankumstellung

      Rev 1.39   26 Jul 1996 20:36:38   MA
   includes

      Rev 1.38   25 Jul 1996 16:39:52   OM
   DB-Auswahldialog eingebaut

      Rev 1.37   23 Jul 1996 13:26:32   OM
   Datenbank und Tabelle auswaehlen

      Rev 1.36   22 Jul 1996 11:03:00   OM
   Datenbankauswahldialog

      Rev 1.35   19 Jul 1996 16:10:02   OM
   SBA-Umstellung

      Rev 1.34   17 Jul 1996 13:47:06   OM
   Datenbankumstellung 327

      Rev 1.33   05 Jul 1996 15:57:50   OM
   #28464# Serienbrief mit markierten Datensaetzen

      Rev 1.32   28 Jun 1996 10:21:50   OM
   Neue Segs

      Rev 1.31   28 Jun 1996 10:20:54   OM
   #29103# Radiobutton bei Eingabe selektieren

      Rev 1.30   04 Jun 1996 14:10:28   OM
   Neue Segs

      Rev 1.29   04 Jun 1996 14:09:40   OM
   Serienbrief Mailing

------------------------------------------------------------------------*/


