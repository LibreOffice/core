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

#include <font/LogicalFontInstance.hxx>

#include <QtGui/QFont>

#include "QtFontFace.hxx"

class QtFont final : public QFont, public LogicalFontInstance
{
    friend rtl::Reference<LogicalFontInstance>
    QtFontFace::CreateFontInstance(const vcl::font::FontSelectPattern&) const;

    bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const override;

    explicit QtFont(const vcl::font::PhysicalFontFace&, const vcl::font::FontSelectPattern&);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
