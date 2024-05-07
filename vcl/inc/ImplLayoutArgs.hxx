/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <i18nlangtag/languagetag.hxx>
#include <vcl/outdev.hxx>

#include "impglyphitem.hxx"
#include "ImplLayoutRuns.hxx"
#include "justificationdata.hxx"

#include <span>

namespace vcl::text
{
class VCL_DLLPUBLIC ImplLayoutArgs
{
public:
    // string related inputs
    LanguageTag maLanguageTag;
    SalLayoutFlags mnFlags;
    const OUString& mrStr;
    int mnMinCharPos;
    int mnEndCharPos;
    int mnDrawOriginCluster = std::numeric_limits<int>::min();
    int mnDrawMinCharPos = std::numeric_limits<int>::min();
    int mnDrawEndCharPos = std::numeric_limits<int>::max();

    // performance hack
    vcl::text::TextLayoutCache const* m_pTextLayoutCache;

    // positioning related inputs
    JustificationData mstJustification;
    double mnLayoutWidth; // in pixel units
    Degree10 mnOrientation; // in 0-3600 system

    // data for bidi and glyph+script fallback
    ImplLayoutRuns maRuns;
    ImplLayoutRuns maFallbackRuns;

    ImplLayoutArgs(OUString const& rStr, int nMinCharPos, int nEndCharPos, SalLayoutFlags nFlags,
                   LanguageTag aLanguageTag, vcl::text::TextLayoutCache const* pLayoutCache);

    void SetLayoutWidth(double nWidth);
    void SetJustificationData(JustificationData stJustification);
    void SetOrientation(Degree10 nOrientation);

    void ResetPos();
    bool GetNextPos(int* nCharPos, bool* bRTL);
    bool GetNextRun(int* nMinRunPos, int* nEndRunPos, bool* bRTL);
    void AddFallbackRun(int nMinRunPos, int nEndRunPos, bool bRTL);

    // methods used by BiDi and glyph fallback
    bool HasFallbackRun() const;
    bool PrepareFallback(const SalLayoutGlyphsImpl* pGlyphsImpl);

private:
    void AddRun(int nMinCharPos, int nEndCharPos, bool bRTL);
};
}

// For nice SAL_INFO logging of ImplLayoutArgs values
std::ostream& operator<<(std::ostream& s, vcl::text::ImplLayoutArgs const& rArgs);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
