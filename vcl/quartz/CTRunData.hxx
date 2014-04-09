/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef CTRunData_Included
#define CTRunData_Included

#include "premac.h"
#if defined(MACOSX) && MACOSX_SDK_VERSION < 1080
#include <ApplicationServices/ApplicationServices.h>
#else
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#endif
#include "postmac.h"

class CTRunData
{
public:
    int   ownership_flags;
#define CTRUNDATA_F_OWN_ADVANCES  (1<<0)
#define CTRUNDATA_F_OWN_GLYPHS    (1<<1)
#define CTRUNDATA_F_OWN_INDICES   (1<<2)
#define CTRUNDATA_F_OWN_POSITIONS (1<<3)

    int m_nGlyphs;
    int m_StartPos;
    int m_EndPos;
    CTRunRef m_pRun;
    CTFontRef m_pFont;
    const CGGlyph* m_pGlyphs;
    const CGPoint* m_pPositions;
    const CGSize*  m_pAdvances;
    const CFIndex* m_pStringIndices;

    CTRunData(CTRunRef pRun, int start);
   ~CTRunData(void);
};

#endif /* NDef CTRunData_Included */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
