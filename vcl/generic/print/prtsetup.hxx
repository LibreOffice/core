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

#ifndef _PAD_PRTSETUP_HXX_
#define _PAD_PRTSETUP_HXX_

#include "tools/link.hxx"

#include "vcl/tabdlg.hxx"
#include "vcl/tabpage.hxx"
#include "vcl/tabctrl.hxx"
#include "vcl/button.hxx"
#include "vcl/edit.hxx"
#include "vcl/fixed.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/field.hxx"
#include "vcl/combobox.hxx"
#include "vcl/ppdparser.hxx"
#include "vcl/printerinfomanager.hxx"

class RTSPaperPage;
class RTSDevicePage;

class RTSDialog : public TabDialog
{
    friend class RTSPaperPage;
    friend class RTSDevicePage;

    ::psp::PrinterInfo      m_aJobData;
    OUString                m_aPrinter;

    // controls
    TabControl*             m_pTabControl;
    OKButton*               m_pOKButton;
    CancelButton*           m_pCancelButton;

    // pages
    RTSPaperPage*           m_pPaperPage;
    RTSDevicePage*          m_pDevicePage;

    // some resources
    OUString                m_aInvalidString;

    DECL_LINK( ActivatePage, TabControl* );
    DECL_LINK( ClickButton, Button* );

    // helper functions
    void insertAllPPDValues( ListBox&, const psp::PPDParser*, const psp::PPDKey* );
public:
    RTSDialog(const ::psp::PrinterInfo& rJobData, const OUString& rPrinter, Window* pParent = NULL);
    ~RTSDialog();

    const ::psp::PrinterInfo& getSetup() const { return m_aJobData; }
};

class RTSPaperPage : public TabPage
{
    RTSDialog*          m_pParent;

    FixedText*          m_pPaperText;
    ListBox*            m_pPaperBox;

    ListBox*            m_pOrientBox;

    FixedText*          m_pDuplexText;
    ListBox*            m_pDuplexBox;

    FixedText*          m_pSlotText;
    ListBox*            m_pSlotBox;

    DECL_LINK( SelectHdl, ListBox* );
public:
    RTSPaperPage( RTSDialog* );
    ~RTSPaperPage();

    void update();

    sal_Int32 getOrientation() const { return m_pOrientBox->GetSelectEntryPos(); }
};

class RTSDevicePage : public TabPage
{
    RTSDialog*          m_pParent;

    OUString            m_aSpaceColor;
    OUString            m_aSpaceGray;

    ListBox*            m_pPPDKeyBox;
    ListBox*            m_pPPDValueBox;
    const psp::PPDValue* m_pCustomValue;
    Edit*               m_pCustomEdit;

    ListBox*            m_pLevelBox;
    ListBox*            m_pSpaceBox;
    ListBox*            m_pDepthBox;

    void FillValueBox( const ::psp::PPDKey* );

    DECL_LINK( SelectHdl, ListBox* );
    DECL_LINK( ModifyHdl, Edit* );
public:
    RTSDevicePage( RTSDialog* );
    ~RTSDevicePage();

    void update();

    sal_uLong getLevel();
    sal_uLong getPDFDevice();
    sal_uLong getDepth();
    sal_uLong getColorDevice();
};

int SetupPrinterDriver(::psp::PrinterInfo& rJobData);

#endif // _PAD_PRTSETUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
