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

#ifndef _SV_PRNSETUP_HXX_
#define _SV_PRNSETUP_HXX_

#include "svtools/svtdllapi.h"

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/timer.hxx>
#include <svtools/stdctrl.hxx>

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
void ImplFreePrnDlgListBox( ListBox* pBox, sal_Bool bClear = sal_True );
Printer* ImplPrnDlgListBoxSelect( ListBox* pBox, PushButton* pPropBtn,
                                  Printer* pPrinter, Printer* pTempPrinter );
Printer* ImplPrnDlgUpdatePrinter( Printer* pPrinter, Printer* pTempPrinter );
void ImplPrnDlgUpdateQueueInfo( ListBox* pBox, QueueInfo& rInfo );
XubString ImplPrnDlgGetStatusText( const QueueInfo& rInfo );

#endif // _SV_PRNSETUP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
