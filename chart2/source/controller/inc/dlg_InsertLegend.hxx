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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_INSERTLEGEND_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_INSERTLEGEND_HXX

#include <vcl/weld.hxx>

#include <memory>

#include "res_LegendPosition.hxx"

namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }

namespace chart
{

class SchLegendDlg : public weld::GenericDialogController
{
private:
    std::unique_ptr<SchLegendPositionResources> m_xLegendPositionResources;

public:
    SchLegendDlg(weld::Window* pParent, const css::uno::Reference< css::uno::XComponentContext>& xCC);

    void init( const css::uno::Reference< css::frame::XModel >& xChartModel );
    void writeToModel( const css::uno::Reference< css::frame::XModel >& xChartModel ) const;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
