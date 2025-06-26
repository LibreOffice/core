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

#include <editeng/outliner.hxx>
#include <optional>
#include <svx/svxdllapi.h>
#include <unotools/weakref.hxx>
#include <editeng/StripPortionsHelper.hxx>

class SdrTextObj;
class SdrPage;

class SVXCORE_DLLPUBLIC SdrOutliner : public Outliner
{
    ::unotools::WeakReference<SdrTextObj> mxWeakTextObj;
    const SdrPage* mpVisualizedPage;

public:
    SdrOutliner( SfxItemPool* pItemPool, OutlinerMode nMode );
    virtual ~SdrOutliner() override ;

    void  SetTextObj( const SdrTextObj* pObj );
    void SetTextObjNoInit( const SdrTextObj* pObj );
    const SdrTextObj* GetTextObj() const;

    void setVisualizedPage(const SdrPage* pPage) { if(pPage != mpVisualizedPage) mpVisualizedPage = pPage; }
    const SdrPage* getVisualizedPage() const { return mpVisualizedPage; }

    virtual OUString CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, std::optional<Color>& rpTxtColor, std::optional<Color>& rpFldColor, std::optional<FontLineStyle>& rpFldLineStyle) override;

    bool hasEditViewCallbacks() const;

    /// @returns state of the SdrCompatibilityFlag
    virtual std::optional<bool> GetCompatFlag(SdrCompatibilityFlag eFlag) const override;
};

class TextHierarchyBreakupOutliner : public TextHierarchyBreakup
{
    SdrOutliner&                mrOutliner;

protected:
    virtual sal_Int16 getOutlineLevelFromParagraph(sal_Int32 nPara) const;
    virtual sal_Int32 getParagraphCount() const;

public:
    TextHierarchyBreakupOutliner(
        SdrOutliner& rOutliner,
        const basegfx::B2DHomMatrix& rNewTransformA,
        const basegfx::B2DHomMatrix& rNewTransformB);
};

class TextHierarchyBreakupBlockText : public TextHierarchyBreakupOutliner
{
    // ClipRange for BlockText decomposition; only text portions completely
    // inside are to be accepted, so this is different from geometric clipping
    // (which would allow e.g. upper parts of portions to remain)
    const basegfx::B2DRange&    mrClipRange;

public:
    virtual void processDrawPortionInfo(const DrawPortionInfo& rDrawPortionInfo);

    TextHierarchyBreakupBlockText(
        SdrOutliner& rOutliner,
        const basegfx::B2DHomMatrix& rNewTransformA,
        const basegfx::B2DHomMatrix& rNewTransformB,
        const basegfx::B2DRange& rClipRange);
};

class TextHierarchyBreakupContourText : public TextHierarchyBreakupOutliner
{
    // the visible area for contour text decomposition
    basegfx::B2DVector              maScale;

public:
    virtual void processDrawPortionInfo(const DrawPortionInfo& rDrawPortionInfo);

    TextHierarchyBreakupContourText(
        SdrOutliner& rOutliner,
        const basegfx::B2DHomMatrix& rNewTransformA,
        const basegfx::B2DHomMatrix& rNewTransformB,
        const basegfx::B2DVector& rScale);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
