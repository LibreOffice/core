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

#pragma once

#include <svtools/svtdllapi.h>
#include <vcl/print.hxx>
#include <vcl/weld.hxx>
#include <vcl/timer.hxx>

class QueueInfo;
class VclSimpleEvent;

class SVT_DLLPUBLIC PrinterSetupDialog final : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::ComboBox>        m_xLbName;
    std::unique_ptr<weld::Button>     m_xBtnProperties;
    std::unique_ptr<weld::Button>     m_xBtnOptions;
    std::unique_ptr<weld::Label>      m_xFiStatus;
    std::unique_ptr<weld::Label>      m_xFiType;
    std::unique_ptr<weld::Label>      m_xFiLocation;
    std::unique_ptr<weld::Label>      m_xFiComment;
    AutoTimer              maStatusTimer;
    VclPtr<Printer>        mpPrinter;
    VclPtr<Printer>        mpTempPrinter;

    SVT_DLLPRIVATE void         ImplSetInfo();

    DECL_DLLPRIVATE_LINK( ImplPropertiesHdl, weld::Button&, void );
    DECL_DLLPRIVATE_LINK( ImplChangePrinterHdl, weld::ComboBox&, void );
    DECL_DLLPRIVATE_LINK( ImplGetFocusHdl, weld::Widget&, void );
    DECL_DLLPRIVATE_LINK( ImplStatusHdl, Timer*, void );
    DECL_DLLPRIVATE_LINK( ImplDataChangedHdl, VclSimpleEvent&, void);

public:
    PrinterSetupDialog(weld::Window* pWindow);
    virtual         ~PrinterSetupDialog() override;

    void            SetPrinter( Printer* pNewPrinter ) { mpPrinter = pNewPrinter; }
    Printer*        GetPrinter() const { return mpPrinter; }

    virtual short   run() override;

    weld::Window*   GetFrameWeld() const { return m_xDialog.get(); }

    void            SetOptionsHdl( const Link<weld::Button&,void>& rLink );
};


#define IMPL_PRINTDLG_STATUS_UPDATE     15000

void ImplFillPrnDlgListBox( const Printer* pPrinter,
                            weld::ComboBox* pBox, weld::Button* pPropBtn );
void ImplFreePrnDlgListBox( weld::ComboBox* pBox, bool bClear = true );
Printer* ImplPrnDlgListBoxSelect( weld::ComboBox const * pBox, weld::Button* pPropBtn,
                                  Printer const * pPrinter, Printer* pTempPrinter );
Printer* ImplPrnDlgUpdatePrinter( Printer const * pPrinter, Printer* pTempPrinter );
void ImplPrnDlgUpdateQueueInfo( weld::ComboBox const * pBox, QueueInfo& rInfo );
OUString ImplPrnDlgGetStatusText( const QueueInfo& rInfo );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
