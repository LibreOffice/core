/*************************************************************************
 *
 *  $RCSfile: mailmrge.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2000-10-31 16:01:08 $
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
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _MAILENUM_HXX //autogen
#include <goodies/mailenum.hxx>
#endif
#ifndef _IODLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SVX_MULTIFIL_HXX
#include <svx/multifil.hxx>
#endif

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _DBUI_HXX
#include <dbui.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif

#ifndef _ENVELP_HRC
#include <envelp.hrc>
#endif
#ifndef _MAILMRGE_HRC
#include <mailmrge.hrc>
#endif
#ifndef _MAILMRGE_HXX
#include <mailmrge.hxx>
#endif


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


#define C2S(cChar) UniString::CreateFromAscii(cChar)

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SwMailMergeDlg::SwMailMergeDlg(Window *pParent, SwWrtShell *pShell,
            const String& rName,
            const String& rTblName,
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
    rTableName      (rTblName),
    rStatement      (rStat),
    rSelectionList  (pSelList),
    nMergeType      (DBMGR_MERGE_MAILING)

{
    FreeResource();

    DBG_ASSERT(pSh, "Shell fehlt"  );

    pModOpt = SW_MOD()->GetModuleConfig();

    aSingleJobsCB.Check(pModOpt->IsSinglePrintJob());

    BYTE nMailingMode(pModOpt->GetMailingFormats());
    aFormatSwCB.Check((nMailingMode & TXTFORMAT_OFFICE) != 0);
    aFormatHtmlCB.Check((nMailingMode & TXTFORMAT_HTML) != 0);
    aFormatRtfCB.Check((nMailingMode & TXTFORMAT_RTF) != 0);

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

    pSh->GetNewDBMgr()->GetColumnNames(&aAddressFldLB, rDBName, rTableName);
    for(USHORT nEntry = 0; nEntry < aAddressFldLB.GetEntryCount(); nEntry++)
        aColumnLB.InsertEntry(aAddressFldLB.GetEntry(nEntry));
    aAddressFldLB.SelectEntry(C2S("EMAIL"));

    aPathED.SetText(pModOpt->GetMailingPath());
    if (!aPathED.GetText().Len())
    {
        SvtPathOptions aPathOpt;
        aPathED.SetText( aPathOpt.GetWorkPath() );
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
    String sPath( aPathED.GetText() );
    if( !sPath.Len() )
    {
        SvtPathOptions aPathOpt;
        sPath = aPathOpt.GetWorkPath();
    }
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


