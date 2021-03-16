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

#ifndef INCLUDED_VCL_GLYPHITEM_HXX
#define INCLUDED_VCL_GLYPHITEM_HXX

#include <sal/types.h>
#include <vcl/dllapi.h>

#include <vector>

typedef sal_uInt16 sal_GlyphId;

class SalLayoutGlyphsImpl;

class VCL_DLLPUBLIC SalLayoutGlyphs final
{
    std::vector<SalLayoutGlyphsImpl*> m_pImpls;

public:
    SalLayoutGlyphs() = default;
    SalLayoutGlyphs(const SalLayoutGlyphs&) = delete;
    SalLayoutGlyphs(SalLayoutGlyphs&&);
    ~SalLayoutGlyphs();

    SalLayoutGlyphs& operator=(const SalLayoutGlyphs&) = delete;
    SalLayoutGlyphs& operator=(SalLayoutGlyphs&&);

    SalLayoutGlyphsImpl* Impl(unsigned int nLevel) const
    {
        return nLevel < m_pImpls.size() ? m_pImpls[nLevel] : nullptr;
    }
    void AppendImpl(SalLayoutGlyphsImpl* pImpl) { m_pImpls.push_back(pImpl); }

    bool IsValid() const;
    void Invalidate();
};

#endif // INCLUDED_VCL_GLYPHITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
