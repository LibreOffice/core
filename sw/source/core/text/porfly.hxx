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

#include <ascharanchoredobjectposition.hxx>

#include "porglue.hxx"
#include <flyfrms.hxx>

class SwDrawContact;
class SwTextFrame;
struct SwCursorMoveState;

class SwFlyPortion : public SwFixPortion
{
    sal_uInt16 m_nBlankWidth;
public:
    explicit SwFlyPortion( const SwRect &rFlyRect )
        : SwFixPortion(rFlyRect), m_nBlankWidth( 0 ) { SetWhichPor( PortionType::Fly ); }
    sal_uInt16 GetBlankWidth( ) const { return m_nBlankWidth; }
    void SetBlankWidth( const sal_uInt16 nNew ) { m_nBlankWidth = nNew; }
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
};

/// This portion represents an as-character anchored fly (shape, frame, etc.)
class SwFlyCntPortion : public SwLinePortion
{
    Point m_aRef;     // Relatively to this point we calculate the AbsPos
    bool m_bMax;      // Line adjustment and height == line height
    bool m_bDeleted;  // Part of tracked deletion: it needs strikethrough
    size_t m_nAuthor; // Redline author for color of the strikethrough
    sw::LineAlign m_eAlign;

    virtual SdrObject* GetSdrObj(const SwTextFrame&) =0;

public:
    SwFlyCntPortion();
    const Point& GetRefPoint() const { return m_aRef; }
    bool IsMax() const { return m_bMax; }
    bool IsDeleted() const { return m_bDeleted; }
    void SetAuthor(size_t nAuthor) { m_nAuthor = nAuthor; }
    size_t GetAuthor() const { return m_nAuthor; }
    sw::LineAlign GetAlign() const { return m_eAlign; }
    void SetAlign(sw::LineAlign eAlign) { m_eAlign = eAlign; }
    void SetMax(bool bMax) { m_bMax = bMax; }
    void SetDeleted(bool bDeleted) { m_bDeleted = bDeleted; }
    void SetBase(const SwTextFrame& rFrame, const Point& rBase, tools::Long nLnAscent, tools::Long nLnDescent, tools::Long nFlyAscent, tools::Long nFlyDescent, AsCharFlags nFlags);
    virtual bool Format(SwTextFormatInfo& rInf) override;
};

namespace sw
{
    class FlyContentPortion final : public SwFlyCntPortion
    {
            SwFlyInContentFrame* m_pFly;
            virtual SdrObject* GetSdrObj(const SwTextFrame&) override;
        public:
            FlyContentPortion(SwFlyInContentFrame* pFly);
            static FlyContentPortion* Create(const SwTextFrame& rFrame, SwFlyInContentFrame* pFly, const Point& rBase, tools::Long nAscent, tools::Long nDescent, tools::Long nFlyAsc, tools::Long nFlyDesc, AsCharFlags nFlags);
            SwFlyInContentFrame* GetFlyFrame() { return m_pFly; }
            const SwFlyInContentFrame* GetFlyFrame() const { return m_pFly; }
            void GetFlyCursorOfst(Point& rPoint, SwPosition& rPos, SwCursorMoveState* pCMS) const { m_pFly->GetModelPositionForViewPoint(&rPos, rPoint, pCMS); };
            virtual void Paint(const SwTextPaintInfo& rInf) const override;
            virtual ~FlyContentPortion() override;
    };
    class DrawFlyCntPortion final : public SwFlyCntPortion
    {
            SwDrawContact* m_pContact;
            virtual SdrObject* GetSdrObj(const SwTextFrame&) override;
        public:
            DrawFlyCntPortion(SwFrameFormat const & rFormat);
            static DrawFlyCntPortion* Create(const SwTextFrame& rFrame, SwFrameFormat const & rFormat, const Point& rBase, tools::Long nAsc, tools::Long nDescent, tools::Long nFlyAsc, tools::Long nFlyDesc, AsCharFlags nFlags);
            virtual void Paint(const SwTextPaintInfo& rInf) const override;
            virtual ~DrawFlyCntPortion() override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
