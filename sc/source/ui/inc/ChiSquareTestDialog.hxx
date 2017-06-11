/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_CHISQUARETESTDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CHISQUARETESTDIALOG_HXX

#include "StatisticsInputOutputDialog.hxx"

class ScChiSquareTestDialog : public ScStatisticsInputOutputDialog
{
public:
    ScChiSquareTestDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        vcl::Window* pParent, ScViewData* pViewData );

    virtual ~ScChiSquareTestDialog() override;

    virtual bool Close() override;

protected:
    virtual const char* GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
