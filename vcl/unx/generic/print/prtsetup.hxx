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

#ifndef INCLUDED_VCL_GENERIC_PRINT_PRTSETUP_HXX
#define INCLUDED_VCL_GENERIC_PRINT_PRTSETUP_HXX

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

    // controls
    VclPtr<TabControl>      m_pTabControl;
    VclPtr<OKButton>        m_pOKButton;
    VclPtr<CancelButton>    m_pCancelButton;

    // pages
    VclPtr<RTSPaperPage>    m_pPaperPage;
    VclPtr<RTSDevicePage>   m_pDevicePage;

    // some resources
    OUString                m_aInvalidString;

    bool mbDataModified;

    DECL_LINK_TYPED( ActivatePage, TabControl*, void );
    DECL_LINK_TYPED( ClickButton, Button*, void );

    // helper functions
    void insertAllPPDValues( ListBox&, const psp::PPDParser*, const psp::PPDKey* );
public:
    RTSDialog(const ::psp::PrinterInfo& rJobData, vcl::Window* pParent = nullptr);
    virtual ~RTSDialog();
    virtual void dispose() override;

    const ::psp::PrinterInfo& getSetup() const { return m_aJobData; }

    void SetDataModified( bool bModified ) { mbDataModified = bModified; }
    bool GetDataModified() const { return mbDataModified; }
};

class RTSPaperPage : public TabPage
{
    VclPtr<RTSDialog>          m_pParent;

    VclPtr<FixedText>          m_pPaperText;
    VclPtr<ListBox>            m_pPaperBox;

    VclPtr<ListBox>            m_pOrientBox;

    VclPtr<FixedText>          m_pDuplexText;
    VclPtr<ListBox>            m_pDuplexBox;

    VclPtr<FixedText>          m_pSlotText;
    VclPtr<ListBox>            m_pSlotBox;

    DECL_LINK_TYPED( SelectHdl, ListBox&, void );
public:
    explicit RTSPaperPage( RTSDialog* );
    virtual ~RTSPaperPage();
    virtual void dispose() override;

    void update();

    sal_Int32 getOrientation() const { return m_pOrientBox->GetSelectEntryPos(); }
};

class RTSDevicePage : public TabPage
{
    VclPtr<RTSDialog>          m_pParent;

    VclPtr<ListBox>            m_pPPDKeyBox;
    VclPtr<ListBox>            m_pPPDValueBox;
    const psp::PPDValue* m_pCustomValue;
    VclPtr<Edit>               m_pCustomEdit;

    VclPtr<ListBox>            m_pLevelBox;
    VclPtr<ListBox>            m_pSpaceBox;
    VclPtr<ListBox>            m_pDepthBox;

    void FillValueBox( const ::psp::PPDKey* );

    DECL_LINK_TYPED( SelectHdl, ListBox&, void );
    DECL_LINK_TYPED( ModifyHdl, Edit&, void );
public:
    explicit RTSDevicePage( RTSDialog* );
    virtual ~RTSDevicePage();
    virtual void dispose() override;

    sal_uLong getLevel();
    sal_uLong getPDFDevice();
    sal_uLong getDepth();
    sal_uLong getColorDevice();
};

int SetupPrinterDriver(::psp::PrinterInfo& rJobData);

#endif // _PAD_PRTSETUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
