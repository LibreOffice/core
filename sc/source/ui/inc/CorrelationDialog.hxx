/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef CORRELATION_DIALOG_HXX
#define CORRELATION_DIALOG_HXX

#include "MatrixComparisonGenerator.hxx"

class ScCorrelationDialog : public ScMatrixComparisonGenerator
{
public:
    ScCorrelationDialog(
        SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
        Window* pParent, ScViewData* pViewData);

    virtual bool Close() SAL_OVERRIDE;

protected:
    virtual const OUString getLabel() SAL_OVERRIDE;
    virtual const OUString getTemplate() SAL_OVERRIDE;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
