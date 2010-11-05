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

#ifndef _VCL_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _VCL_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _VCL_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _VCL_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _VCL_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _VCL_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _VCL_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
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
    sal_uInt16              mnCopyCount;
    sal_uInt16              mnFirstPage;
    sal_uInt16              mnLastPage;
    sal_uInt16              mnMinPage;
    sal_uInt16              mnMaxPage;
    PrintDialogRange    meCheckRange;
    sal_Bool                mbAll;
    sal_Bool                mbSelection;
    sal_Bool                mbFromTo;
    sal_Bool                mbRange;
    sal_Bool                mbCollate;
    sal_Bool                mbCollateCheck;
    sal_Bool                mbOptions;
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
    sal_Bool                IsRangeEnabled( PrintDialogRange eRange ) const;

    void                CheckRange( PrintDialogRange eRange = PRINTDIALOG_ALL )
                            { meCheckRange = eRange; }
    PrintDialogRange    GetCheckedRange() const { return meCheckRange; }
    sal_Bool                IsRangeChecked( PrintDialogRange eRange ) const;

    void                SetRangeText( const XubString& rRange ) { maRangeText = rRange; }
    const XubString&    GetRangeText() const { return maRangeText; }

    void                SetFirstPage( sal_uInt16 nPage = 0 );
    sal_uInt16              GetFirstPage() const { return mnFirstPage; }
    void                SetLastPage( sal_uInt16 nPage = 0 );
    sal_uInt16              GetLastPage() const { return mnLastPage; }

    void                SetMinPage( sal_uInt16 nPage = 1 ) { mnMinPage = nPage; }
    sal_uInt16              GetMinPage() const { return mnMinPage; }
    void                SetMaxPage( sal_uInt16 nPage = 65535 ) { mnMaxPage = nPage; }
    sal_uInt16              GetMaxPage() const { return mnMaxPage; }

    void                SetCopyCount( sal_uInt16 nCopies = 1 ) { mnCopyCount = nCopies; }
    sal_uInt16              GetCopyCount() const { return mnCopyCount; }

    void                EnableCollate( sal_Bool bEnable = sal_True )
                            { mbCollate = bEnable; }
    sal_Bool                IsCollateEnabled() const { return mbCollate; }
    void                CheckCollate( sal_Bool bCheck = sal_True )
                            { mbCollateCheck = bCheck; }
    sal_Bool                IsCollateChecked() const { return mbCollateCheck; }

    void                ShowOptionsButton( sal_Bool bShow = sal_True )
                            { mbOptions = bShow; }
    sal_Bool                IsOptionsButtonVisible() const { return mbOptions; }

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
        mbAll = sal_True;
    else if ( eRange == PRINTDIALOG_SELECTION )
        mbSelection = sal_True;
    else if ( eRange == PRINTDIALOG_FROMTO )
        mbFromTo = sal_True;
    else
        mbRange = sal_True;
}

inline void PrintDialog::DisableRange( PrintDialogRange eRange )
{
    if ( eRange == PRINTDIALOG_ALL )
        mbAll = sal_False;
    else if ( eRange == PRINTDIALOG_SELECTION )
        mbSelection = sal_False;
    else if ( eRange == PRINTDIALOG_FROMTO )
        mbFromTo = sal_False;
    else
        mbRange = sal_False;
}

inline sal_Bool PrintDialog::IsRangeEnabled( PrintDialogRange eRange ) const
{
    sal_Bool bRet;

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

inline sal_Bool PrintDialog::IsRangeChecked( PrintDialogRange eRange ) const
{
    if ( eRange == meCheckRange )
        return sal_True;
    else
        return sal_False;
}

inline void PrintDialog::SetFirstPage( sal_uInt16 nPage )
{
    mnFirstPage = nPage;
    if ( nPage && (nPage < mnMinPage) )
        mnMinPage = nPage;
}

inline void PrintDialog::SetLastPage( sal_uInt16 nPage )
{
    mnLastPage = nPage;
    if ( nPage && (nPage > mnMaxPage) )
        mnMaxPage = nPage;
}

#endif // _SV_PRINTDLG_HXX_

