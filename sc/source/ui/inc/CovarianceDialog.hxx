/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef COVARIANCE_DIALOG_HXX
#define COVARIANCE_DIALOG_HXX

#include "MatrixComparisonGenerator.hxx"

class ScCovarianceDialog : public ScMatrixComparisonGenerator
{
public:
    ScCovarianceDialog(
        SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
        Window* pParent, ScViewData* pViewData);

    virtual bool Close() SAL_OVERRIDE;

protected:
    virtual const OUString getLabel() SAL_OVERRIDE;
    virtual const OUString getTemplate() SAL_OVERRIDE;
    virtual sal_Int16 GetUndoNameId() SAL_OVERRIDE;
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
