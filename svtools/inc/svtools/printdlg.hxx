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

#ifndef _SV_PRINTDLG_HXX_
#define _SV_PRINTDLG_HXX_

#include "svtools/svtdllapi.h"

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>

class Printer;
class QueueInfo;
struct SvtPrinterImpl;

// ---------------------
// - PrintDialog-Types -
// ---------------------

enum PrintDialogRange{
    PRINTDIALOG_ALL, PRINTDIALOG_SELECTION, PRINTDIALOG_FROMTO, PRINTDIALOG_RANGE };

enum PrintSheetRange
{
    PRINTSHEETS_ALL,
    PRINTSHEETS_SELECTED_SHEETS,
    PRINTSHEETS_SELECTED_CELLS
};


// ---------------
// - PrintDialog -
// ---------------

class SVT_DLLPUBLIC PrintDialog : public ModalDialog
{
private:
    FixedLine           maFlPrinter;
    FixedText           maFtName;
    ListBox             maLbName;
    PushButton          maBtnProperties;
    FixedText           maFtStatus;
    FixedInfo           maFiStatus;
    FixedText           maFtType;
    FixedInfo           maFiType;
    FixedText           maFtLocation;
    FixedInfo           maFiLocation;
    FixedText           maFtComment;
    FixedInfo           maFiComment;

    // "Print to file" or "Fax number"
    CheckBox            maCbxFilePrint;
    FixedInfo           maFiPrintFile;
    FixedText           maFiFaxNo;
    Edit                maEdtFaxNo;
    //PushButton         maBtnBrowse_nomore;

    // "Print"
    FixedLine           maFlPrint;
    RadioButton         maRbtAllSheets;
    RadioButton         maRbtSelectedSheets;
    RadioButton         maRbtSelectedCells;

    // "Print range"
    FixedLine           maFlPrintRange;
    RadioButton         maRbtAll;
    RadioButton         maRbtPages;
    RadioButton         maRbtSelection;
    Edit                maEdtPages;

    FixedLine           maFlSepCopiesRange;

    // "Copies"
    FixedLine           maFlCopies;
    FixedText           maFtCopies;
    NumericField        maNumCopies;
    FixedImage          maImgCollate;
    FixedImage          maImgNotCollate;
    CheckBox            maCbxCollate;

    FixedLine           maFlSepButtonLine;
    PushButton          maBtnOptions;
    OKButton            maBtnOK;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;

    AutoTimer           maStatusTimer;
    Printer*            mpPrinter;
    SvtPrinterImpl*     mpPrinterImpl;

    XubString           maRangeText;
    USHORT              mnCopyCount;
    USHORT              mnFirstPage;
    USHORT              mnLastPage;
    USHORT              mnMinPage;
    USHORT              mnMaxPage;
    PrintDialogRange    meCheckRange;
    BOOL                mbAll;
    BOOL                mbSelection;
    BOOL                mbFromTo;
    BOOL                mbRange;
    BOOL                mbCollate;
    BOOL                mbCollateCheck;
    BOOL                mbOptions;
    bool                mbWithSheetsAndCells;
    Link                maOptionsHdlLink; // Link zum Options-Handler
    Link                maOKHdlLink;      // Link zum OK-Handler

    String              maAllFilterStr;

    SVT_DLLPRIVATE void     ImplCheckOK();
    SVT_DLLPRIVATE void     ImplInitControls();
    SVT_DLLPRIVATE void     ImplFillDialogData();
    SVT_DLLPRIVATE void     ImplSetInfo();
    SVT_DLLPRIVATE void     ImplSetImages();
    SVT_DLLPRIVATE bool     ImplGetFilename();

    DECL_DLLPRIVATE_LINK(   ImplPropertiesHdl, void* );
    DECL_DLLPRIVATE_LINK(   ImplChangePrinterHdl, void* );
    DECL_DLLPRIVATE_LINK(   ImplModifyControlHdl, void* );
    DECL_DLLPRIVATE_LINK(   ImplStatusHdl, Timer* );

public:
    PrintDialog( Window* pWindow, bool bWithSheetsAndCells );
    ~PrintDialog();

    virtual long        OK();
    virtual long        ClickOptionsHdl();

    void                SetPrinter( Printer* pNewPrinter ) { mpPrinter = pNewPrinter; }
    Printer*            GetPrinter() const { return mpPrinter; }

    inline bool         IsSheetRangeAvailable() const { return mbWithSheetsAndCells; }
    void                EnableSheetRange( bool bEnable, PrintSheetRange eRange );
    bool                IsSheetRangeEnabled( PrintSheetRange eRange ) const;
    void                CheckSheetRange( PrintSheetRange eRange );
    PrintSheetRange     GetCheckedSheetRange() const;
    bool                IsSheetRangeChecked( PrintSheetRange eRange ) const;

    void                EnableRange( PrintDialogRange eRange );
    void                DisableRange( PrintDialogRange eRange );
    BOOL                IsRangeEnabled( PrintDialogRange eRange ) const;

    void                CheckRange( PrintDialogRange eRange = PRINTDIALOG_ALL )
                            { meCheckRange = eRange; }
    PrintDialogRange    GetCheckedRange() const { return meCheckRange; }
    BOOL                IsRangeChecked( PrintDialogRange eRange ) const;

    void                SetRangeText( const XubString& rRange ) { maRangeText = rRange; }
    const XubString&    GetRangeText() const { return maRangeText; }

    void                SetFirstPage( USHORT nPage = 0 );
    USHORT              GetFirstPage() const { return mnFirstPage; }
    void                SetLastPage( USHORT nPage = 0 );
    USHORT              GetLastPage() const { return mnLastPage; }

    void                SetMinPage( USHORT nPage = 1 ) { mnMinPage = nPage; }
    USHORT              GetMinPage() const { return mnMinPage; }
    void                SetMaxPage( USHORT nPage = 65535 ) { mnMaxPage = nPage; }
    USHORT              GetMaxPage() const { return mnMaxPage; }

    void                SetCopyCount( USHORT nCopies = 1 ) { mnCopyCount = nCopies; }
    USHORT              GetCopyCount() const { return mnCopyCount; }

    void                EnableCollate( BOOL bEnable = TRUE )
                            { mbCollate = bEnable; }
    BOOL                IsCollateEnabled() const { return mbCollate; }
    void                CheckCollate( BOOL bCheck = TRUE )
                            { mbCollateCheck = bCheck; }
    BOOL                IsCollateChecked() const { return mbCollateCheck; }

    void                ShowOptionsButton( BOOL bShow = TRUE )
                            { mbOptions = bShow; }
    BOOL                IsOptionsButtonVisible() const { return mbOptions; }

    void                SetOptionsHdl( const Link& rLink ) { maOptionsHdlLink = rLink; }
    const Link&         GetOptionsHdl() const { return maOptionsHdlLink; }

    void                SetOKHdl( const Link& rLink ) { maOKHdlLink = rLink; }
    const Link&         GetOKHdl() const { return maOKHdlLink; }

    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual long        Notify( NotifyEvent& rNEvt );

    virtual short       Execute();

    void                DisableHelp();
};

inline void PrintDialog::EnableRange( PrintDialogRange eRange )
{
    if ( eRange == PRINTDIALOG_ALL )
        mbAll = TRUE;
    else if ( eRange == PRINTDIALOG_SELECTION )
        mbSelection = TRUE;
    else if ( eRange == PRINTDIALOG_FROMTO )
        mbFromTo = TRUE;
    else
        mbRange = TRUE;
}

inline void PrintDialog::DisableRange( PrintDialogRange eRange )
{
    if ( eRange == PRINTDIALOG_ALL )
        mbAll = FALSE;
    else if ( eRange == PRINTDIALOG_SELECTION )
        mbSelection = FALSE;
    else if ( eRange == PRINTDIALOG_FROMTO )
        mbFromTo = FALSE;
    else
        mbRange = FALSE;
}

inline BOOL PrintDialog::IsRangeEnabled( PrintDialogRange eRange ) const
{
    BOOL bRet;

    if ( eRange == PRINTDIALOG_ALL )
        bRet = mbAll;
    else if ( eRange == PRINTDIALOG_SELECTION )
        bRet = mbSelection;
    else if ( eRange == PRINTDIALOG_FROMTO )
        bRet = mbFromTo;
    else
        bRet = mbRange;

    return bRet;
}

inline BOOL PrintDialog::IsRangeChecked( PrintDialogRange eRange ) const
{
    if ( eRange == meCheckRange )
        return TRUE;
    else
        return FALSE;
}

inline void PrintDialog::SetFirstPage( USHORT nPage )
{
    mnFirstPage = nPage;
    if ( nPage && (nPage < mnMinPage) )
        mnMinPage = nPage;
}

inline void PrintDialog::SetLastPage( USHORT nPage )
{
    mnLastPage = nPage;
    if ( nPage && (nPage > mnMaxPage) )
        mnMaxPage = nPage;
}

#endif // _SV_PRINTDLG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
