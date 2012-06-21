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

#ifndef _PAD_COMMANDDLG_HXX_
#define _PAD_COMMANDDLG_HXX_

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/tabpage.hxx>

#include <list>

namespace padmin {

class RTSDialog;

class CommandStore
{
    static void getStoredCommands( const char* pGroup, ::std::list< String >& rCommands );
    static void getSystemPrintCommands( ::std::list< String >& rCommands );
    static void getSystemPdfCommands( ::std::list< String >& rCommands );
    static void setCommands( const char* pGroup, const ::std::list< String >& rCommands, const ::std::list< String >& rSysCommands );

public:
    static void getPrintCommands( ::std::list< String >& rCommands );
    static void getFaxCommands( ::std::list< String >& rCommands );
    static void getPdfCommands( ::std::list< String >& rCommands );

    static void setPrintCommands( const ::std::list< String >& rCommands );
    static void setFaxCommands( const ::std::list< String >& rCommands );
    static void setPdfCommands( const ::std::list< String >& rCommands );
};

class RTSCommandPage : public TabPage
{
private:
    RTSDialog*                      m_pParent;

    ComboBox                        m_aCommandsCB;
    CheckBox                        m_aExternalCB;
    FixedText                       m_aQuickFT;
    ComboBox                        m_aQuickCB;
    FixedLine                       m_aCommandTitle;
    FixedText                       m_aPrinterName;
    FixedText                       m_aConnectedTo;
    FixedLine                       m_aPrinterFL;

    FixedText                       m_aConfigureText;
    ListBox                         m_aConfigureBox;
    sal_uInt16                          m_nPrinterEntry;
    sal_uInt16                          m_nFaxEntry;
    sal_uInt16                          m_nPdfEntry;
    FixedText                       m_aPdfDirectoryText;
    PushButton                      m_aPdfDirectoryButton;
    Edit                            m_aPdfDirectoryEdit;
    CheckBox                        m_aFaxSwallowBox;

    PushButton                      m_aHelpButton;
    PushButton                      m_aRemovePB;

    ::std::list< String >           m_aPrinterCommands;
    ::std::list< String >           m_aFaxCommands;
    ::std::list< String >           m_aPdfCommands;

    String                          m_aFaxHelp;
    String                          m_aPrinterHelp;
    String                          m_aPdfHelp;

    bool                            m_bWasFax;
    bool                            m_bWasPdf;
    bool                            m_bWasExternalDialog;

    DECL_LINK( DoubleClickHdl, ComboBox* );
    DECL_LINK( ClickBtnHdl, Button* );
    DECL_LINK( SelectHdl, Control* );
    DECL_LINK( ModifyHdl, Edit* );

    void ConnectCommand();
    void UpdateCommands();

public:
    RTSCommandPage( RTSDialog* );
    ~RTSCommandPage();

    void save();
};

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
