/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_GL3DBARCHARTDIALOGCONTROLLER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_GL3DBARCHARTDIALOGCONTROLLER_HXX

#include "ChartTypeDialogController.hxx"

namespace chart {

class GL3DBarChartDialogController : public ChartTypeDialogController
{
public:
    GL3DBarChartDialogController();
    virtual ~GL3DBarChartDialogController();

    virtual bool shouldShow_GL3DResourceGroup() const override;

    virtual OUString getName() override;
    virtual Image getImage() override;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const override;

    virtual void fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& rParameter ) override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
