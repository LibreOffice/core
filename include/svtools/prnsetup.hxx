/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVTOOLS_PRNSETUP_HXX
#define INCLUDED_SVTOOLS_PRNSETUP_HXX

#include <svtools/svtdllapi.h>

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/timer.hxx>
#include <svtools/stdctrl.hxx>

class Printer;
class QueueInfo;


// - PrinterSetupDialog -


class SVT_DLLPUBLIC PrinterSetupDialog : public ModalDialog
{
private:
    ListBox*        m_pLbName;
    PushButton*     m_pBtnProperties;
    PushButton*     m_pBtnOptions;
    FixedText*      m_pFiStatus;
    FixedText*      m_pFiType;
    FixedText*      m_pFiLocation;
    FixedText*      m_pFiComment;
    AutoTimer       maStatusTimer;
    Printer*        mpPrinter;
    Printer*        mpTempPrinter;

    SVT_DLLPRIVATE void         ImplSetInfo();

                    DECL_DLLPRIVATE_LINK( ImplPropertiesHdl, void* );
                    DECL_DLLPRIVATE_LINK( ImplOptionsHdl, void* );
                    DECL_DLLPRIVATE_LINK( ImplChangePrinterHdl, void* );
                    DECL_DLLPRIVATE_LINK( ImplStatusHdl, void* );

public:
                    PrinterSetupDialog( Window* pWindow );
                    ~PrinterSetupDialog();

    void            SetPrinter( Printer* pNewPrinter ) { mpPrinter = pNewPrinter; }
    Printer*        GetPrinter() const { return mpPrinter; }

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual bool    Notify( NotifyEvent& rNEvt );

    virtual short   Execute();

    void                SetOptionsHdl( const Link& rLink );
};


// - Helper functions for print dialog -


#define IMPL_PRINTDLG_STATUS_UPDATE     15000

void ImplFillPrnDlgListBox( const Printer* pPrinter,
                            ListBox* pBox, PushButton* pPropBtn );
void ImplFreePrnDlgListBox( ListBox* pBox, sal_Bool bClear = sal_True );
Printer* ImplPrnDlgListBoxSelect( ListBox* pBox, PushButton* pPropBtn,
                                  Printer* pPrinter, Printer* pTempPrinter );
Printer* ImplPrnDlgUpdatePrinter( Printer* pPrinter, Printer* pTempPrinter );
void ImplPrnDlgUpdateQueueInfo( ListBox* pBox, QueueInfo& rInfo );
OUString ImplPrnDlgGetStatusText( const QueueInfo& rInfo );

#endif // INCLUDED_SVTOOLS_PRNSETUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
