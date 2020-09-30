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

#include <svx/svxdlg.hxx>
#include <vcl/weld.hxx>

class SvxSplitTableDlg : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::SpinButton> m_xCountEdit;
    std::unique_ptr<weld::RadioButton> m_xHorzBox;
    std::unique_ptr<weld::RadioButton> m_xVertBox;
    std::unique_ptr<weld::CheckButton> m_xPropCB;

    tools::Long                mnMaxVertical;
    tools::Long                mnMaxHorizontal;

public:
    SvxSplitTableDlg(weld::Window *pParent, bool bIsTableVertical, tools::Long nMaxVertical, tools::Long nMaxHorizontal);

    DECL_LINK(ClickHdl, weld::Button&, void);

    virtual bool IsHorizontal() const;
    virtual bool IsProportional() const;
    virtual tools::Long GetCount() const;

    virtual void SetSplitVerticalByDefault();
};

class SvxAbstractSplitTableDialog_Impl : public SvxAbstractSplitTableDialog
{
    std::shared_ptr<SvxSplitTableDlg> m_xDlg;

public:
    SvxAbstractSplitTableDialog_Impl(std::shared_ptr<SvxSplitTableDlg> pDlg) : m_xDlg(std::move(pDlg)) {}

    virtual bool IsHorizontal() const override;
    virtual bool IsProportional() const override;
    virtual tools::Long GetCount() const override;

    virtual void SetSplitVerticalByDefault() override;

    virtual short Execute() override;
    virtual bool StartExecuteAsync(AsyncContext& rContext) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
