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

SalLayoutGlyphs::SalLayoutGlyphs()
    : m_pImpl(nullptr)
{
}

SalLayoutGlyphs::~SalLayoutGlyphs() { delete m_pImpl; }

SalLayoutGlyphs::SalLayoutGlyphs(const SalLayoutGlyphs& rOther)
{
    m_pImpl = rOther.m_pImpl ? rOther.m_pImpl->clone(*this) : nullptr;
}

SalLayoutGlyphs& SalLayoutGlyphs::operator=(const SalLayoutGlyphs& rOther)
{
    if (this != &rOther)
    {
        delete m_pImpl;
        m_pImpl = rOther.m_pImpl ? rOther.m_pImpl->clone(*this) : nullptr;
    }
    return *this;
}

bool SalLayoutGlyphs::IsValid() const { return m_pImpl && m_pImpl->IsValid(); }

void SalLayoutGlyphs::Invalidate()
{
    if (m_pImpl)
        m_pImpl->Invalidate();
}

SalLayoutGlyphsImpl::~SalLayoutGlyphsImpl() {}

SalLayoutGlyphsImpl* SalLayoutGlyphsImpl::clone(SalLayoutGlyphs& rGlyphs) const
{
    SalLayoutGlyphsImpl* pNew = new SalLayoutGlyphsImpl(rGlyphs, *m_rFontInstance);
    *pNew = *this;
    return pNew;
}

bool SalLayoutGlyphsImpl::IsValid() const
{
    if (!m_rFontInstance.is())
        return false;
    if (empty())
        return false;
#if (defined UNX && !defined MACOSX && !defined IOS)
    const FreetypeFontInstance* pFFI = dynamic_cast<FreetypeFontInstance*>(m_rFontInstance.get());
    if (pFFI && !pFFI->GetFreetypeFont())
    {
        m_rFontInstance.clear();
        return false;
    }
#endif
    return true;
}

void SalLayoutGlyphsImpl::Invalidate()
{
    m_rFontInstance.clear();
    clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
