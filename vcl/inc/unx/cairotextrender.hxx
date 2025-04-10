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

#include <unx/freetypetextrender.hxx>

class GenericSalLayout;
struct CairoCommon;
typedef struct _cairo cairo_t;
typedef struct _cairo_font_options cairo_font_options_t;

class VCL_DLLPUBLIC CairoTextRender final : public FreeTypeTextRenderImpl
{
private:
    CairoCommon& mrCairoCommon;
protected:
    cairo_t*                getCairoContext();
    void                    releaseCairoContext(cairo_t* cr);

public:
    // helper to call DrawTextLayout with already setup cairo_t context,
    // so no CairoCommon is needed.
    static void ImplDrawTextLayout(cairo_t* cr, const Color& rTextColor, const GenericSalLayout& rLayout, CairoCommon* pCairoCommon, bool bAntiAlias);

    virtual void DrawTextLayout(const GenericSalLayout&, const SalGraphics&) override;
    CairoTextRender(CairoCommon& rCairoCommon);
    virtual ~CairoTextRender();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
