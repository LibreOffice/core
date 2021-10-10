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

#include <sal/config.h>

#include <rtl/ustring.hxx>

inline constexpr OUStringLiteral XML_IMPORT_FILTER_WRITER = u"com.sun.star.comp.Writer.XMLOasisImporter";
inline constexpr OUStringLiteral XML_IMPORT_FILTER_CALC = u"com.sun.star.comp.Calc.XMLOasisImporter";
inline constexpr OUStringLiteral XML_IMPORT_FILTER_DRAW = u"com.sun.star.comp.Draw.XMLOasisImporter";
inline constexpr OUStringLiteral XML_IMPORT_FILTER_IMPRESS = u"com.sun.star.comp.Impress.XMLOasisImporter";
inline constexpr OUStringLiteral XML_IMPORT_FILTER_MATH = u"com.sun.star.comp.Math.XMLImporter";
inline constexpr OUStringLiteral XML_IMPORT_FILTER_CHART = u"com.sun.star.comp.Chart.XMLOasisImporter";

inline constexpr OUStringLiteral XML_EXPORT_FILTER_WRITER = u"com.sun.star.comp.Writer.XMLOasisExporter";
inline constexpr OUStringLiteral XML_EXPORT_FILTER_CALC = u"com.sun.star.comp.Calc.XMLOasisExporter";
inline constexpr OUStringLiteral XML_EXPORT_FILTER_DRAW = u"com.sun.star.comp.Draw.XMLOasisExporter";
inline constexpr OUStringLiteral XML_EXPORT_FILTER_IMPRESS = u"com.sun.star.comp.Impress.XMLOasisExporter";
inline constexpr OUStringLiteral XML_EXPORT_FILTER_MATH = u"com.sun.star.comp.Math.XMLExporter";
inline constexpr OUStringLiteral XML_EXPORT_FILTER_CHART = u"com.sun.star.comp.Chart.XMLOasisExporter";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
