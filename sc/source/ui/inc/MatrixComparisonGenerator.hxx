/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_MATRIXCOMPARISONGENERATOR_HXX
#define INCLUDED_SC_SOURCE_UI_INC_MATRIXCOMPARISONGENERATOR_HXX

#include <address.hxx>
#include "viewdata.hxx"

#include "StatisticsInputOutputDialog.hxx"

class ScMatrixComparisonGenerator : public ScStatisticsInputOutputDialog
{
public:
    ScMatrixComparisonGenerator(
        SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
        weld::Window* pParent, ScViewData& rViewData,
        const OUString& rUiXmlDescription, const OString& rID);

    virtual ~ScMatrixComparisonGenerator() override;

protected:
    virtual const char* GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;

    virtual OUString getLabel() = 0;
    virtual OUString getTemplate() = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
