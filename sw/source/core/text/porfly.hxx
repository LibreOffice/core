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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORFLY_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORFLY_HXX

#include <ascharanchoredobjectposition.hxx>

#include "porglue.hxx"
#include <flyfrms.hxx>

class SwDrawContact;
class SwFlyInContentFrame;
class SwTextFrame;
struct SwCursorMoveState;

class SwFlyPortion : public SwFixPortion
{
    sal_uInt16 nBlankWidth;
public:
    explicit SwFlyPortion( const SwRect &rFlyRect )
        : SwFixPortion(rFlyRect), nBlankWidth( 0 ) { SetWhichPor( PortionType::Fly ); }
    sal_uInt16 GetBlankWidth( ) const { return nBlankWidth; }
    void SetBlankWidth( const sal_uInt16 nNew ) { nBlankWidth = nNew; }
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
};

/// This portion represents an as-character anchored fly (shape, frame, etc.)
class SwFlyCntPortion : public SwLinePortion
{
    Point m_aRef;     // Relatively to this point we calculate the AbsPos
    bool m_bMax;   // Line adjustment and height == line height
    sw::LineAlign m_eAlign;

    virtual SdrObject* GetSdrObj(const SwTextFrame&) =0;

public:
    SwFlyCntPortion();
    const Point& GetRefPoint() const { return m_aRef; }
    bool IsMax() const { return m_bMax; }
    sw::LineAlign GetAlign() const { return m_eAlign; }
    void SetAlign(sw::LineAlign eAlign) { m_eAlign = eAlign; }
    void SetMax(bool bMax) { m_bMax = bMax; }
    void SetBase(const SwTextFrame& rFrame, const Point& rBase, long nLnAscent, long nLnDescent, long nFlyAscent, long nFlyDescent, AsCharFlags nFlags);
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
            static FlyContentPortion* Create(const SwTextFrame& rFrame, SwFlyInContentFrame* pFly, const Point& rBase, long nAscent, long nDescent, long nFlyAsc, long nFlyDesc, AsCharFlags nFlags);
            SwFlyInContentFrame* GetFlyFrame() { return m_pFly; }
            void GetFlyCursorOfst(Point& rPoint, SwPosition& rPos, SwCursorMoveState* pCMS) const { m_pFly->GetCursorOfst(&rPos, rPoint, pCMS); };
            virtual void Paint(const SwTextPaintInfo& rInf) const override;
            virtual ~FlyContentPortion() override;
    };
    class DrawFlyCntPortion final : public SwFlyCntPortion
    {
            SwDrawContact* m_pContact;
            virtual SdrObject* GetSdrObj(const SwTextFrame&) override;
        public:
            DrawFlyCntPortion(SwFrameFormat const & rFormat);
            static DrawFlyCntPortion* Create(const SwTextFrame& rFrame, SwFrameFormat const & rFormat, const Point& rBase, long nAsc, long nDescent, long nFlyAsc, long nFlyDesc, AsCharFlags nFlags);
            virtual void Paint(const SwTextPaintInfo& rInf) const override;
            virtual ~DrawFlyCntPortion() override;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
