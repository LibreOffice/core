/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CHART2_SOURCE_CHARTRESOURCEGROUPDLGS_HXX
#define INCLUDED_CHART2_SOURCE_CHARTRESOURCEGROUPDLGS_HXX

#include <vcl/weld.hxx>

namespace chart
{
class ChartTypeParameter;

class SplinePropertiesDialog : public weld::GenericDialogController
{
public:
    explicit SplinePropertiesDialog(weld::Window* pParent);

    void fillControls(const ChartTypeParameter& rParameter);
    void fillParameter(ChartTypeParameter& rParameter, bool bSmoothLines);

private:
    DECL_LINK(SplineTypeListBoxHdl, weld::ComboBox&, void);

private:
    std::unique_ptr<weld::ComboBox> m_xLB_Spline_Type;
    std::unique_ptr<weld::SpinButton> m_xMF_SplineResolution;
    std::unique_ptr<weld::Label> m_xFT_SplineOrder;
    std::unique_ptr<weld::SpinButton> m_xMF_SplineOrder;
};

class SteppedPropertiesDialog : public weld::GenericDialogController
{
public:
    explicit SteppedPropertiesDialog(weld::Window* pParent);

    void fillControls(const ChartTypeParameter& rParameter);
    void fillParameter(ChartTypeParameter& rParameter, bool bSteppedLines);

private:
    std::unique_ptr<weld::RadioButton> m_xRB_Start;
    std::unique_ptr<weld::RadioButton> m_xRB_End;
    std::unique_ptr<weld::RadioButton> m_xRB_CenterX;
    std::unique_ptr<weld::RadioButton> m_xRB_CenterY;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
