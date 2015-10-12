/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_FTESTDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_FTESTDIALOG_HXX

#include "StatisticsTwoVariableDialog.hxx"

class ScFTestDialog : public ScStatisticsTwoVariableDialog
{
public:
    ScFTestDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        vcl::Window* pParent, ScViewData* pViewData );

    virtual ~ScFTestDialog();

    virtual bool Close() override;

protected:
    virtual sal_Int16 GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
