/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_GL3DBARCHARTDIALOGCONTROLLER_HXX
#define INCLUDED_CHART2_GL3DBARCHARTDIALOGCONTROLLER_HXX

#include "ChartTypeDialogController.hxx"

namespace chart {

class GL3DBarChartDialogController : public ChartTypeDialogController
{
public:
    GL3DBarChartDialogController();
    virtual ~GL3DBarChartDialogController();

    virtual bool shouldShow_GL3DResourceGroup() const SAL_OVERRIDE;

    virtual OUString getName() SAL_OVERRIDE;
    virtual Image getImage() SAL_OVERRIDE;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const SAL_OVERRIDE;

    virtual void fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& rParameter ) SAL_OVERRIDE;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
