/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_CORRELATIONDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CORRELATIONDIALOG_HXX

#include "MatrixComparisonGenerator.hxx"

class ScCorrelationDialog : public ScMatrixComparisonGenerator
{
public:
    ScCorrelationDialog(
        SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
        weld::Window* pParent, ScViewData& rViewData);

    virtual void Close() override;

protected:
    virtual OUString getLabel() override;
    virtual OUString getTemplate() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
