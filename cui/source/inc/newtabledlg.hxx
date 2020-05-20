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
#ifndef INCLUDED_CUI_SOURCE_INC_NEWTABLEDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_NEWTABLEDLG_HXX

#include <svx/stddlg.hxx>
#include <svx/svxdlg.hxx>
#include <vcl/weld.hxx>

class SvxNewTableDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::SpinButton> mxNumColumns;
    std::unique_ptr<weld::SpinButton> mxNumRows;

public:
    SvxNewTableDialog(weld::Window* pParent);

    virtual sal_Int32 getRows() const;
    virtual sal_Int32 getColumns() const;
};

class SvxNewTableDialogWrapper : public SvxAbstractNewTableDialog
{
private:
    std::shared_ptr<weld::DialogController> m_xDlg;

public:
    SvxNewTableDialogWrapper(weld::Window* pParent)
    : m_xDlg(std::make_shared<SvxNewTableDialog>(pParent))
    {}

    virtual std::shared_ptr<weld::DialogController> getDialogController() override
    {
        return m_xDlg;
    }

    virtual sal_Int32 getRows() const override
    {
        SvxNewTableDialog* pDlg = dynamic_cast<SvxNewTableDialog*>(m_xDlg.get());
        if (pDlg)
            return pDlg->getRows();

        return 0;
    }

    virtual sal_Int32 getColumns() const override
    {
        SvxNewTableDialog* pDlg = dynamic_cast<SvxNewTableDialog*>(m_xDlg.get());
        if (pDlg)
            return pDlg->getColumns();

        return 0;
    }
};

#endif // INCLUDED_CUI_SOURCE_INC_NEWTABLEDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
