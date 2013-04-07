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

#include "coretext/common.h"
#include "outfont.hxx"
#include "coretext/salcoretextfontutils.hxx"
#include "coretext/salcoretextstyle.hxx"

CoreTextStyleInfo::CoreTextStyleInfo() :
    m_fake_bold(false),
    m_fake_italic(false),
    m_matrix(CGAffineTransformIdentity),
    m_stretch_factor(1.0),
    m_CTParagraphStyle(NULL),
    m_CTFont(NULL),
    m_color(NULL),
    m_font_face(NULL)
{
    SAL_INFO( "vcl.coretext.style", "CoreTextStyleInfo::CoreTextStyleInfo() " << this );
}

CoreTextStyleInfo::~CoreTextStyleInfo()
{
    SAL_INFO( "vcl.coretext.style", "~CoreTextStyleInfo(" << this << ")" );

    SafeCFRelease(m_CTFont);
    SafeCFRelease(m_CTParagraphStyle);
    SafeCFRelease(m_color);
}

long CoreTextStyleInfo::GetFontStretchedSize() const
{
    CGFloat size = CTFontGetSize(m_CTFont);
    return static_cast<long>(size * m_stretch_factor + 0.5);
}

void CoreTextStyleInfo::SetFont(FontSelectPattern* requested_font)
{
    if(!requested_font)
    {
        SafeCFRelease(m_CTFont);
        m_font_face = NULL;
        return;
    }
    m_font_face = (CoreTextPhysicalFontFace*)(requested_font->mpFontData);

    m_matrix = CGAffineTransformIdentity;
    CGFloat font_size = (CGFloat)requested_font->mfExactHeight;

    // enable bold-emulation if needed
    if( (requested_font->GetWeight() >= WEIGHT_BOLD) &&
        (m_font_face->GetWeight() < WEIGHT_SEMIBOLD) )
    {
        /* FIXME: add support for fake bold */
        m_fake_bold = true;
    }
    if( ((requested_font->GetSlant() == ITALIC_NORMAL) || (requested_font->GetSlant() == ITALIC_OBLIQUE)) &&
        !((m_font_face->GetSlant() == ITALIC_NORMAL) || (m_font_face->GetSlant() == ITALIC_OBLIQUE)) )
    {
#define kRotationForItalicText 10
        m_fake_italic = true;
        /* about 6 degree of slant */
        m_matrix = CGAffineTransformMake( 1, 0, -tanf( kRotationForItalicText * acosf(0) / 90 ), 1, 0, 0);
    }

    // prepare font stretching
    if( (requested_font->mnWidth != 0) && (requested_font->mnWidth != requested_font->mnHeight) )
    {
        m_stretch_factor = (float)requested_font->mnWidth / requested_font->mnHeight;
        m_matrix = CGAffineTransformScale(m_matrix, m_stretch_factor, 1.0F );
    }

    SafeCFRelease(m_CTFont);

    /* FIXME: pass attribute to take into accout 'VerticalStyle' */
    /* FIXME: how to deal with 'rendering options' i.e anti-aliasing, does it even matter in CoreText ? */
    m_CTFont = CTFontCreateCopyWithAttributes(m_font_face->GetCTFont(), font_size, &m_matrix, NULL);
}

void CoreTextStyleInfo::SetColor(SalColor color)
{
    SafeCFRelease(m_color);
#ifdef IOS
    // No CGColorCreateGenericRGB on iOS
    CGColorSpaceRef rgb_space = CGColorSpaceCreateDeviceRGB();
    CGFloat c[] = { SALCOLOR_RED(color) / 255.0f, SALCOLOR_GREEN(color) / 255.0f, SALCOLOR_BLUE(color) / 255.0f, 1.0 };
    m_color = CGColorCreate(rgb_space, c);
    CGColorSpaceRelease(rgb_space);
#else
    m_color = CGColorCreateGenericRGB(SALCOLOR_RED(color) / 255.0, SALCOLOR_GREEN(color) / 255.0, SALCOLOR_BLUE(color) / 255.0, 1.0);
#endif
}

void CoreTextStyleInfo::SetColor(void)
{
    SafeCFRelease(m_color);
}

std::ostream &operator <<(std::ostream& s, CoreTextStyleInfo &rStyle)
{
#ifndef SAL_LOG_INFO
    (void) rStyle;
#else
    s << "{Font=" << rStyle.GetFont();
    s << ",Color=" << rStyle.GetColor();
    s << "}";
#endif
    return s;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
