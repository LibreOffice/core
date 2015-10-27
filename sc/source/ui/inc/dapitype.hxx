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

#ifndef INCLUDED_SC_SOURCE_UI_INC_DAPITYPE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DAPITYPE_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Sequence.hxx>

class ScDataPilotSourceTypeDlg : public ModalDialog
{
private:
    VclPtr<RadioButton> m_pBtnSelection;
    VclPtr<RadioButton> m_pBtnNamedRange;
    VclPtr<RadioButton> m_pBtnDatabase;
    VclPtr<RadioButton> m_pBtnExternal;
    VclPtr<ListBox>     m_pLbNamedRange;

public:
    ScDataPilotSourceTypeDlg(vcl::Window* pParent, bool bEnableExternal);
    virtual ~ScDataPilotSourceTypeDlg();
    virtual void dispose() override;
    bool IsDatabase() const;
    bool IsExternal() const;
    bool IsNamedRange() const;
    OUString GetSelectedNamedRange() const;
    void AppendNamedRange(const OUString& rNames);

private:
    DECL_LINK_TYPED( RadioClickHdl, Button *, void );
};

class ScDataPilotServiceDlg : public ModalDialog
{
private:
    VclPtr<ListBox>         m_pLbService;
    VclPtr<Edit>            m_pEdSource;
    VclPtr<Edit>            m_pEdName;
    VclPtr<Edit>            m_pEdUser;
    VclPtr<Edit>            m_pEdPasswd;

public:
            ScDataPilotServiceDlg( vcl::Window* pParent,
                    const css::uno::Sequence<OUString>& rServices );
            virtual ~ScDataPilotServiceDlg();
    virtual void dispose() override;

    OUString  GetServiceName() const;
    OUString  GetParSource() const;
    OUString  GetParName() const;
    OUString  GetParUser() const;
    OUString  GetParPass() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
