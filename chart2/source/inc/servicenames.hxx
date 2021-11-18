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
#pragma once

#include <rtl/ustring.hxx>

inline constexpr OUStringLiteral CHART_MODEL_SERVICE_IMPLEMENTATION_NAME
    = u"com.sun.star.comp.chart2.ChartModel";
inline constexpr OUStringLiteral CHART_MODEL_SERVICE_NAME = u"com.sun.star.chart2.ChartDocument";
//@todo create your own service containing the service com.sun.star.document.OfficeDocument

inline constexpr OUStringLiteral CHART_CONTROLLER_SERVICE_IMPLEMENTATION_NAME
    = u"com.sun.star.comp.chart2.ChartController";
inline constexpr OUStringLiteral CHART_CONTROLLER_SERVICE_NAME
    = u"com.sun.star.chart2.ChartController";
//@todo create your own service containing the service com.sun.star.frame.Controller

inline constexpr OUStringLiteral CHART_VIEW_SERVICE_IMPLEMENTATION_NAME
    = u"com.sun.star.comp.chart2.ChartView";
inline constexpr OUStringLiteral CHART_VIEW_SERVICE_NAME = u"com.sun.star.chart2.ChartView";

inline constexpr OUStringLiteral CHART_FRAMELOADER_SERVICE_IMPLEMENTATION_NAME
    = u"com.sun.star.comp.chart2.ChartFrameLoader";
inline constexpr OUStringLiteral CHART_FRAMELOADER_SERVICE_NAME
    = u"com.sun.star.frame.SynchronousFrameLoader";

inline constexpr OUStringLiteral CHART_WIZARD_DIALOG_SERVICE_IMPLEMENTATION_NAME
    = u"com.sun.star.comp.chart2.WizardDialog";
inline constexpr OUStringLiteral CHART_WIZARD_DIALOG_SERVICE_NAME
    = u"com.sun.star.chart2.WizardDialog";

inline constexpr OUStringLiteral CHART_TYPE_DIALOG_SERVICE_IMPLEMENTATION_NAME
    = u"com.sun.star.comp.chart2.ChartTypeDialog";
inline constexpr OUStringLiteral CHART_TYPE_DIALOG_SERVICE_NAME
    = u"com.sun.star.chart2.ChartTypeDialog";

// wrapper for old UNO API (com.sun.star.chart)
inline constexpr OUStringLiteral CHART_CHARTAPIWRAPPER_IMPLEMENTATION_NAME
    = u"com.sun.star.comp.chart2.ChartDocumentWrapper";
inline constexpr OUStringLiteral CHART_CHARTAPIWRAPPER_SERVICE_NAME
    = u"com.sun.star.chart2.ChartDocumentWrapper";

// accessibility
inline constexpr OUStringLiteral CHART_ACCESSIBLE_TEXT_SERVICE_NAME
    = u"com.sun.star.accessibility.AccessibleTextComponent";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
