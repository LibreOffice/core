/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_PRNSETUP_HXX_
#define _SV_PRNSETUP_HXX_

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
#ifndef _VCL_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _VCL_TIMER_HXX
#include <vcl/timer.hxx>
#endif
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
