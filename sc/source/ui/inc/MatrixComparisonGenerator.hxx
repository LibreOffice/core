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
        vcl::Window* pParent, ScViewData* pViewData,
        const OUString& rID, const OUString& rUiXmlDescription );

    virtual ~ScMatrixComparisonGenerator() override;

protected:
    virtual const char* GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;

    virtual const OUString getLabel() = 0;
    virtual const OUString getTemplate() = 0;
};

class ScMatrixComparisonGeneratorController : public ScStatisticsInputOutputDialogController
{
public:
    ScMatrixComparisonGeneratorController(
        SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
        weld::Window* pParent, ScViewData* pViewData,
        const OUString& rUiXmlDescription, const OString& rID);

    virtual ~ScMatrixComparisonGeneratorController() override;

protected:
    virtual const char* GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;

    virtual const OUString getLabel() = 0;
    virtual const OUString getTemplate() = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
