/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef T_TEST_DIALOG_HXX
#define T_TEST_DIALOG_HXX

#include "StatisticsTwoVariableDialog.hxx"

class ScTTestDialog : public ScStatisticsTwoVariableDialog
{
public:
    ScTTestDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        Window* pParent, ScViewData* pViewData );

    virtual ~ScTTestDialog();

    virtual bool Close();

protected:
    virtual sal_Int16 GetUndoNameId();
    virtual ScRange ApplyOutput(ScDocShell* pDocShell);
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
