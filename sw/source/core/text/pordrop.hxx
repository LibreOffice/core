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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORDROP_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORDROP_HXX

#include "portxt.hxx"
#include <swfont.hxx>

#include <memory>

class SwFont;

// DropCap cache, global variable initialized/destroyed in txtinit.cxx
// and used in txtdrop.cxx for initial calculation

class SwDropCapCache;
extern SwDropCapCache *pDropCapCache;

// A drop portion can consist of one or more parts in order to allow
// attribute changes inside them.
class SwDropPortionPart
{
    std::unique_ptr<SwDropPortionPart> pFollow;
    std::unique_ptr<SwFont> pFnt;
    TextFrameIndex nLen;
    sal_uInt16 nWidth;
    bool m_bJoinBorderWithNext;
    bool m_bJoinBorderWithPrev;

public:
    SwDropPortionPart( SwFont& rFont, const TextFrameIndex nL )
            : pFnt( &rFont ), nLen( nL ), nWidth( 0 ), m_bJoinBorderWithNext(false), m_bJoinBorderWithPrev(false) {};
    ~SwDropPortionPart();

    SwDropPortionPart* GetFollow() const { return pFollow.get(); };
    void SetFollow( std::unique_ptr<SwDropPortionPart> pNew ) { pFollow = std::move(pNew); };
    SwFont& GetFont() const { return *pFnt; }
    TextFrameIndex GetLen() const { return nLen; }
    sal_uInt16 GetWidth() const { return nWidth; }
    void SetWidth( sal_uInt16 nNew )  { nWidth = nNew; }

    bool GetJoinBorderWithPrev() const { return m_bJoinBorderWithPrev; }
    bool GetJoinBorderWithNext() const { return m_bJoinBorderWithNext; }
    void SetJoinBorderWithPrev( const bool bJoinPrev ) { m_bJoinBorderWithPrev = bJoinPrev; }
    void SetJoinBorderWithNext( const bool bJoinNext ) { m_bJoinBorderWithNext = bJoinNext; }
};

class SwDropPortion : public SwTextPortion
{
    friend class SwDropCapCache;
    std::unique_ptr<SwDropPortionPart> pPart; // due to script/attribute changes
    sal_uInt16 nLines;          // Line count
    sal_uInt16 nDropHeight;     // Height
    sal_uInt16 nDropDescent;    // Distance to the next line
    sal_uInt16 nDistance;       // Distance to the text
    sal_uInt16 nFix;            // Fixed position
    short nY;               // Y Offset

    bool FormatText( SwTextFormatInfo &rInf );
    void PaintText( const SwTextPaintInfo &rInf ) const;

public:
    SwDropPortion( const sal_uInt16 nLineCnt,
                   const sal_uInt16 nDropHeight,
                   const sal_uInt16 nDropDescent,
                   const sal_uInt16 nDistance );
    virtual ~SwDropPortion() override;

    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
            void PaintDrop( const SwTextPaintInfo &rInf ) const;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const override;
    virtual TextFrameIndex GetModelPositionForViewPoint(sal_uInt16 nOfst) const override;

    sal_uInt16 GetLines() const { return nLines; }
    sal_uInt16 GetDistance() const { return nDistance; }
    sal_uInt16 GetDropHeight() const { return nDropHeight; }
    sal_uInt16 GetDropDescent() const { return nDropDescent; }
    sal_uInt16 GetDropLeft() const { return Width() + nFix; }

    SwDropPortionPart* GetPart() const { return pPart.get(); }
    void SetPart( std::unique_ptr<SwDropPortionPart> pNew ) { pPart = std::move(pNew); }

    void SetY( short nNew )  { nY = nNew; }

    SwFont* GetFnt() const { return pPart ? &pPart->GetFont() : nullptr; }

    static void DeleteDropCapCache();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
