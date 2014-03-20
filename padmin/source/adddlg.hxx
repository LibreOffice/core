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

#ifndef _PAD_ADDDLG_HXX_
#define _PAD_ADDDLG_HXX_

#include "helper.hxx"
#include "titlectrl.hxx"

#include "vcl/dialog.hxx"
#include "vcl/tabpage.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "vcl/combobox.hxx"
#include "vcl/printerinfomanager.hxx"

namespace padmin
{

class AddPrinterDialog;

namespace DeviceKind { enum type { Printer, Pdf }; }

class APTabPage : public TabPage
{
    OUString            m_aTitle;
protected:
    AddPrinterDialog*   m_pParent;
public:
    APTabPage( AddPrinterDialog* pParent, const ResId& rResId );

    // returns false if information is incomplete or invalid
    virtual bool check() = 0;
    virtual void fill( ::psp::PrinterInfo& rInfo ) = 0;
    const OUString& getTitle() const { return m_aTitle; }
};

class APChooseDevicePage : public APTabPage
{
    RadioButton             m_aPrinterBtn;
    RadioButton             m_aPDFBtn;
    FixedText               m_aOverTxt;
public:
    APChooseDevicePage( AddPrinterDialog* pParent );
    ~APChooseDevicePage();

    bool isPrinter() { return m_aPrinterBtn.IsChecked(); }
    bool isPDF() { return m_aPDFBtn.IsChecked(); }

    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );
};

class APChooseDriverPage : public APTabPage
{
    FixedText               m_aDriverTxt;
    DelListBox              m_aDriverBox;
    PushButton              m_aAddBtn;
    PushButton              m_aRemBtn;

    OUString                m_aRemStr;
    OUString                m_aLastPrinterName;

    DECL_LINK( ClickBtnHdl, PushButton* );
    DECL_LINK( DelPressedHdl, ListBox* );

    void updateDrivers( bool bRefresh = false, const OUString& rSelectDriver = OUString( "SGENPRT" ) );
public:
    APChooseDriverPage( AddPrinterDialog* pParent );
    ~APChooseDriverPage();

    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );
};

class APNamePage : public APTabPage
{
    FixedText               m_aNameTxt;
    Edit                    m_aNameEdt;
    CheckBox                m_aDefaultBox;
public:
    APNamePage( AddPrinterDialog* pParent, const OUString& rInitName, DeviceKind::type eKind );
    ~APNamePage();

    bool isDefault() { return m_aDefaultBox.IsChecked(); }

    void setText( const OUString& rText ) { m_aNameEdt.SetText( rText ); }

    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );
};

class APCommandPage : public APTabPage
{
    FixedText               m_aCommandTxt;
    ComboBox                m_aCommandBox;
    PushButton              m_aHelpBtn;
    OUString                m_aHelpTxt;
    FixedText               m_aPdfDirTxt;
    Edit                    m_aPdfDirEdt;
    PushButton              m_aPdfDirBtn;

    DeviceKind::type        m_eKind;

    DECL_LINK( ClickBtnHdl, PushButton* );
    DECL_LINK( ModifyHdl, ComboBox* );
public:

    APCommandPage( AddPrinterDialog* pParent, DeviceKind::type eKind );
    ~APCommandPage();

    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );

    OUString getPdfDir() { return m_aPdfDirEdt.GetText(); }
};

class APOldPrinterPage : public APTabPage
{
    FixedText                           m_aOldPrinterTxt;
    MultiListBox                        m_aOldPrinterBox;
    PushButton                          m_aSelectAllBtn;

    ::std::list< ::psp::PrinterInfo >   m_aOldPrinters;

    DECL_LINK( ClickBtnHdl, PushButton* );
public:
    APOldPrinterPage( AddPrinterDialog* pParent );
    ~APOldPrinterPage();

    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );

    void addOldPrinters();
};

class APPdfDriverPage : public APTabPage
{
    FixedText               m_aPdfTxt;
    RadioButton             m_aDefBtn;
    RadioButton             m_aDistBtn;
    RadioButton             m_aSelectBtn;
public:
    APPdfDriverPage( AddPrinterDialog* pParent );
    ~APPdfDriverPage();

    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );

    bool isDefault() { return m_aDefBtn.IsChecked(); }
    bool isDist() { return m_aDistBtn.IsChecked(); }
};

class AddPrinterDialog : public ModalDialog
{
    CancelButton            m_aCancelPB;
    PushButton              m_aPrevPB;
    PushButton              m_aNextPB;
    OKButton                m_aFinishPB;
    FixedLine               m_aLine;
    TitleImage              m_aTitleImage;

    ::psp::PrinterInfo      m_aPrinter;

    APTabPage*              m_pCurrentPage;

    APChooseDevicePage*     m_pChooseDevicePage;
    APCommandPage*          m_pCommandPage;
    APChooseDriverPage*     m_pChooseDriverPage;
    APNamePage*             m_pNamePage;
    APPdfDriverPage*        m_pPdfDriverPage;
    APChooseDriverPage*     m_pPdfSelectDriverPage;
    APNamePage*             m_pPdfNamePage;
    APCommandPage*          m_pPdfCommandPage;

    DECL_LINK( ClickBtnHdl, PushButton* );

    void advance();
    void back();
    void addPrinter();

    void updateSettings();
    virtual void DataChanged( const DataChangedEvent& rEv );

public:
    AddPrinterDialog( Window* pParent );
    ~AddPrinterDialog();

    static OUString uniquePrinterName( const OUString& rString );

    void enableNext( bool bEnable ) { m_aNextPB.Enable( bEnable ); }
};

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
