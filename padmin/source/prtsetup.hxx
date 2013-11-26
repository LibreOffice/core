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

#include "helper.hxx"

#include "tools/link.hxx"

#include "vcl/tabdlg.hxx"
#include "vcl/tabpage.hxx"
#include "vcl/tabctrl.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/field.hxx"
#include "vcl/combobox.hxx"
#include "vcl/ppdparser.hxx"
#include "vcl/printerinfomanager.hxx"

namespace padmin {

class RTSPaperPage;
class RTSDevicePage;
class RTSOtherPage;
class RTSCommandPage;

class RTSDialog : public TabDialog
{
    friend class RTSPaperPage;
    friend class RTSDevicePage;
    friend class RTSOtherPage;
    friend class RTSCommandPage;

    ::psp::PrinterInfo      m_aJobData;
    OUString                m_aPrinter;

    // controls
    TabControl*             m_pTabControl;
    OKButton*               m_pOKButton;
    CancelButton*           m_pCancelButton;

    // pages
    RTSPaperPage*           m_pPaperPage;
    RTSDevicePage*          m_pDevicePage;
    RTSOtherPage*           m_pOtherPage;
    RTSCommandPage*         m_pCommandPage;

    // some resources
    OUString                m_aInvalidString;

    DECL_LINK( ActivatePage, TabControl* );
    DECL_LINK( ClickButton, Button* );

    // helper functions
    void insertAllPPDValues( ListBox&, const psp::PPDParser*, const psp::PPDKey* );
public:
    RTSDialog( const ::psp::PrinterInfo& rJobData, const OUString& rPrinter, bool bAllPages, Window* pParent = NULL );
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

    sal_uInt16 getOrientation() { return m_pOrientBox->GetSelectEntryPos(); }
};

class RTSDevicePage : public TabPage
{
    RTSDialog*          m_pParent;

    OUString            m_aSpaceColor;
    OUString            m_aSpaceGray;

    ListBox*            m_pPPDKeyBox;
    ListBox*            m_pPPDValueBox;

    ListBox*            m_pLevelBox;
    ListBox*            m_pSpaceBox;
    ListBox*            m_pDepthBox;

    void FillValueBox( const ::psp::PPDKey* );

    DECL_LINK( SelectHdl, ListBox* );
public:
    RTSDevicePage( RTSDialog* );
    ~RTSDevicePage();

    void update();

    sal_uLong getLevel();
    sal_uLong getPDFDevice();
    sal_uLong getDepth();
    sal_uLong getColorDevice();
};

class RTSOtherPage : public TabPage
{
    RTSDialog*          m_pParent;

    FixedText           m_aLeftTxt;
    MetricField         m_aLeftLB;
    FixedText           m_aTopTxt;
    MetricField         m_aTopLB;
    FixedText           m_aRightTxt;
    MetricField         m_aRightLB;
    FixedText           m_aBottomTxt;
    MetricField         m_aBottomLB;
    FixedText           m_aCommentTxt;
    Edit                m_aCommentEdt;
    PushButton          m_aDefaultBtn;

    void initValues();

    DECL_LINK( ClickBtnHdl, Button *);

public:
    RTSOtherPage( RTSDialog* );
    ~RTSOtherPage();

    void save();
};

} // namespace

#endif // _PAD_PRTSETUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
