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

#include <impglyphitem.hxx>

#if (defined UNX && !defined MACOSX && !defined IOS)
#include <unx/freetype_glyphcache.hxx>
#endif

SalLayoutGlyphs::~SalLayoutGlyphs()
{
    for (SalLayoutGlyphsImpl* impl : m_pImpls)
        delete impl;
}

SalLayoutGlyphs::SalLayoutGlyphs(SalLayoutGlyphs&& rOther) { std::swap(m_pImpls, rOther.m_pImpls); }

SalLayoutGlyphs& SalLayoutGlyphs::operator=(SalLayoutGlyphs&& rOther)
{
    if (this != &rOther)
        std::swap(m_pImpls, rOther.m_pImpls);
    return *this;
}

bool SalLayoutGlyphs::IsValid() const
{
    if (m_pImpls.empty())
        return false;
    for (SalLayoutGlyphsImpl* impl : m_pImpls)
        if (!impl->IsValid())
            return false;
    return true;
}

void SalLayoutGlyphs::Invalidate()
{
    // Invalidating is in fact simply clearing.
    for (SalLayoutGlyphsImpl* impl : m_pImpls)
        delete impl;
    m_pImpls.clear();
}

SalLayoutGlyphsImpl* SalLayoutGlyphsImpl::clone() const { return new SalLayoutGlyphsImpl(*this); }

bool SalLayoutGlyphsImpl::IsValid() const
{
    if (!m_rFontInstance.is())
        return false;
    if (empty())
        return false;
    return true;
}

void SalLayoutGlyphsImpl::Invalidate()
{
    m_rFontInstance.clear();
    clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
