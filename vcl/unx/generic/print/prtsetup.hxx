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

#include <vcl/idle.hxx>
#include <vcl/weld.hxx>
#include <ppdparser.hxx>
#include <printerinfomanager.hxx>

class RTSPaperPage;
class RTSDevicePage;

class RTSDialog : public weld::GenericDialogController
{
    friend class RTSPaperPage;
    friend class RTSDevicePage;

    ::psp::PrinterInfo m_aJobData;

    bool m_bDataModified;

    // controls
    std::unique_ptr<weld::Notebook> m_xTabControl;
    std::unique_ptr<weld::Button> m_xOKButton;
    std::unique_ptr<weld::Button> m_xCancelButton;

    // pages
    std::unique_ptr<RTSPaperPage> m_xPaperPage;
    std::unique_ptr<RTSDevicePage> m_xDevicePage;

    DECL_LINK(ActivatePage, const OString&, void);
    DECL_LINK(ClickButton, weld::Button&, void);

    // helper functions
    void insertAllPPDValues(weld::ComboBox&, const psp::PPDParser*, const psp::PPDKey*);

public:
    RTSDialog(const ::psp::PrinterInfo& rJobData, weld::Window* pParent);
    virtual ~RTSDialog() override;

    const ::psp::PrinterInfo& getSetup() const { return m_aJobData; }

    void SetDataModified(bool bModified) { m_bDataModified = bModified; }
    bool GetDataModified() const { return m_bDataModified; }
};

class RTSPaperPage
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;

    RTSDialog* m_pParent;

    std::unique_ptr<weld::Widget> m_xContainer;

    std::unique_ptr<weld::CheckButton> m_xCbFromSetup;

    std::unique_ptr<weld::Label> m_xPaperText;
    std::unique_ptr<weld::ComboBox> m_xPaperBox;

    std::unique_ptr<weld::Label> m_xOrientText;
    std::unique_ptr<weld::ComboBox> m_xOrientBox;

    std::unique_ptr<weld::Label> m_xDuplexText;
    std::unique_ptr<weld::ComboBox> m_xDuplexBox;

    std::unique_ptr<weld::Label> m_xSlotText;
    std::unique_ptr<weld::ComboBox> m_xSlotBox;

    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(CheckBoxHdl, weld::ToggleButton&, void);

public:
    RTSPaperPage(weld::Widget* pPage, RTSDialog* pDialog);
    ~RTSPaperPage();

    void update();

    sal_Int32 getOrientation() const { return m_xOrientBox->get_active(); }
};

class RTSDevicePage
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;

    const psp::PPDValue* m_pCustomValue;
    RTSDialog* m_pParent;

    std::unique_ptr<weld::Widget> m_xContainer;
    std::unique_ptr<weld::TreeView> m_xPPDKeyBox;
    std::unique_ptr<weld::TreeView> m_xPPDValueBox;
    std::unique_ptr<weld::Entry> m_xCustomEdit;

    std::unique_ptr<weld::ComboBox> m_xLevelBox;
    std::unique_ptr<weld::ComboBox> m_xSpaceBox;
    std::unique_ptr<weld::ComboBox> m_xDepthBox;

    void FillValueBox(const ::psp::PPDKey*);
    void ValueBoxChanged(const ::psp::PPDKey*);

    Idle m_aReselectCustomIdle;

    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(ModifyHdl, weld::Entry&, void);
    DECL_LINK(ComboChangedHdl, weld::ComboBox&, void);
    DECL_LINK(ImplHandleReselectHdl, Timer*, void);

public:
    RTSDevicePage(weld::Widget* pPage, RTSDialog* pDialog);
    ~RTSDevicePage();

    sal_uLong getLevel() const;
    sal_uLong getPDFDevice() const;
    sal_uLong getDepth() const;
    sal_uLong getColorDevice() const;
};

int SetupPrinterDriver(weld::Window* pParent, ::psp::PrinterInfo& rJobData);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
