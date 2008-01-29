/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: printdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 16:11:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_PRINTDLG_HXX_
#define _SV_PRINTDLG_HXX_

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

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
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif

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

