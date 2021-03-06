/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <address.hxx>
#include "viewdata.hxx"

#include "StatisticsInputOutputDialog.hxx"

class ScExponentialSmoothingDialog : public ScStatisticsInputOutputDialog
{
private:
    std::unique_ptr<weld::SpinButton> mxSmoothingFactor;

public:
    ScExponentialSmoothingDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        weld::Window* pParent, ScViewData& rViewData );

    virtual ~ScExponentialSmoothingDialog() override;

    virtual void Close() override;

protected:
    virtual const char* GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
