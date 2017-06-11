/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_EXPONENTIALSMOOTHINGDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_EXPONENTIALSMOOTHINGDIALOG_HXX

#include "global.hxx"
#include "address.hxx"
#include "anyrefdg.hxx"

#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>

#include "StatisticsInputOutputDialog.hxx"

class ScExponentialSmoothingDialog : public ScStatisticsInputOutputDialog
{
private:
    VclPtr<NumericField> mpSmoothingFactor;

public:
    ScExponentialSmoothingDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        vcl::Window* pParent, ScViewData* pViewData );

    virtual ~ScExponentialSmoothingDialog() override;
    virtual void dispose() override;

    virtual bool Close() override;

protected:
    virtual const char* GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
