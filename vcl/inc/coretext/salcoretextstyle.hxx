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

#ifndef _VCL_CORETEXT_SALCORETEXTSTYLE_HXX
#define _VCL_CORETEXT_SALCORETEXTSTYLE_HXX

#include "quartz/salgdicommon.hxx"

#include "coretext/salcoretextfontutils.hxx"
#include "outfont.hxx"

class CoreTextStyleInfo
{
public:
    CoreTextStyleInfo();
    ~CoreTextStyleInfo();
    CTFontRef GetFont() const { return m_CTFont; };
    CoreTextPhysicalFontFace* GetFontFace() const { return m_font_face; };
    long GetFontStretchedSize() const;
    float GetFontStretchFactor() const { return m_stretch_factor; };
    CTParagraphStyleRef GetParagraphStyle() const { return m_CTParagraphStyle; } ;
    CGSize    GetSize() const;
    CGColorRef GetColor() const { return m_color; } ;
    void SetColor(SalColor color);
    void SetColor(void);
    void SetFont(FontSelectPattern* requested_font);

private:
    bool m_fake_bold;
    bool m_fake_italic;
    CGAffineTransform m_matrix;
    float m_stretch_factor;
    float m_font_scale;
    float m_fake_dpi_scale;
    CTParagraphStyleRef m_CTParagraphStyle;
    CTFontRef m_CTFont;
    CGColorRef m_color;
    CoreTextPhysicalFontFace* m_font_face;
};

#endif // _VCL_AQUA_CORETEXT_SALCORETEXTSTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
