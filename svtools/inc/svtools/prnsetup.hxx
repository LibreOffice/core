/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prnsetup.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:39:42 $
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

#ifndef _SV_PRNSETUP_HXX_
#define _SV_PRNSETUP_HXX_

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
#ifndef _VCL_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _VCL_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif

class Printer;
class QueueInfo;

// ----------------------
// - PrinterSetupDialog -
// ----------------------

class SVT_DLLPUBLIC PrinterSetupDialog : public ModalDialog
{
private:
    FixedLine       maFlPrinter;
    FixedText       maFtName;
    ListBox         maLbName;
    PushButton      maBtnProperties;
    PushButton      maBtnOptions;
    FixedText       maFtStatus;
    FixedInfo       maFiStatus;
    FixedText       maFtType;
    FixedInfo       maFiType;
    FixedText       maFtLocation;
    FixedInfo       maFiLocation;
    FixedText       maFtComment;
    FixedInfo       maFiComment;
    FixedLine       maFlSepButton;
    OKButton        maBtnOK;
    CancelButton    maBtnCancel;
    HelpButton      maBtnHelp;
    AutoTimer       maStatusTimer;
    Printer*        mpPrinter;
    Printer*        mpTempPrinter;

    SVT_DLLPRIVATE void         ImplSetInfo();

                    DECL_DLLPRIVATE_LINK( ImplPropertiesHdl, void* );
                    DECL_DLLPRIVATE_LINK( ImplOptionsHdl, void* );
                    DECL_DLLPRIVATE_LINK( ImplChangePrinterHdl, void* );
                    DECL_DLLPRIVATE_LINK( ImplStatusHdl, Timer* );

public:
                    PrinterSetupDialog( Window* pWindow );
                    ~PrinterSetupDialog();

    void            SetPrinter( Printer* pNewPrinter ) { mpPrinter = pNewPrinter; }
    Printer*        GetPrinter() const { return mpPrinter; }

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual long    Notify( NotifyEvent& rNEvt );

    virtual short   Execute();

    void                SetOptionsHdl( const Link& rLink );
    const Link&         GetOptionsHdl() const;
};

// --------------------------------------
// - Hilfsfunktionen fuer Print-Dialoge -
// --------------------------------------

#define IMPL_PRINTDLG_STATUS_UPDATE     15000

void ImplFillPrnDlgListBox( const Printer* pPrinter,
                            ListBox* pBox, PushButton* pPropBtn );
void ImplFreePrnDlgListBox( ListBox* pBox, BOOL bClear = TRUE );
Printer* ImplPrnDlgListBoxSelect( ListBox* pBox, PushButton* pPropBtn,
                                  Printer* pPrinter, Printer* pTempPrinter );
Printer* ImplPrnDlgUpdatePrinter( Printer* pPrinter, Printer* pTempPrinter );
void ImplPrnDlgUpdateQueueInfo( ListBox* pBox, QueueInfo& rInfo );
XubString ImplPrnDlgGetStatusText( const QueueInfo& rInfo );

#endif // _SV_PRNSETUP_HXX_
