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
#ifndef _PORDROP_HXX
#define _PORDROP_HXX

#include "portxt.hxx"

class SwFont;

// DropCap cache, global variable initialized/destroyed in txtinit.cxx
// and used in txtdrop.cxx for initial calculation

class SwDropCapCache;
extern SwDropCapCache *pDropCapCache;

/*************************************************************************
 *                      class SwDropPortionPart
 *
 * A drop portion can consist of one or more parts in order to allow
 * attribute changes inside them.
 *************************************************************************/

class SwDropPortionPart
{
    SwDropPortionPart* pFollow;
    SwFont* pFnt;
    xub_StrLen nLen;
    sal_uInt16 nWidth;
    bool m_bJoinBorderWithNext;
    bool m_bJoinBorderWithPrev;

public:
    SwDropPortionPart( SwFont& rFont, const xub_StrLen nL )
            : pFollow( 0 ), pFnt( &rFont ), nLen( nL ), nWidth( 0 ), m_bJoinBorderWithNext(false), m_bJoinBorderWithPrev(false) {};
    ~SwDropPortionPart();

    inline SwDropPortionPart* GetFollow() const { return pFollow; };
    inline void SetFollow( SwDropPortionPart* pNew ) { pFollow = pNew; };
    inline SwFont& GetFont() const { return *pFnt; }
    inline xub_StrLen GetLen() const { return nLen; }
    inline sal_uInt16 GetWidth() const { return nWidth; }
    inline void SetWidth( sal_uInt16 nNew )  { nWidth = nNew; }

    bool GetJoinBorderWithPrev() const { return m_bJoinBorderWithPrev; }
    bool GetJoinBorderWithNext() const { return m_bJoinBorderWithNext; }
    void SetJoinBorderWithPrev( const bool bJoinPrev ) { m_bJoinBorderWithPrev = bJoinPrev; }
    void SetJoinBorderWithNext( const bool bJoinNext ) { m_bJoinBorderWithNext = bJoinNext; }
};

/*************************************************************************
 *                      class SwDropPortion
 *************************************************************************/

class SwDropPortion : public SwTxtPortion
{
    friend class SwDropCapCache;
    SwDropPortionPart* pPart; // due to script/attribute changes
    MSHORT nLines;          // Line count
    KSHORT nDropHeight;     // Height
    KSHORT nDropDescent;    // Distance to the next line
    KSHORT nDistance;       // Distance to the text
    KSHORT nFix;            // Fixed position
    short nX;               // X PaintOffset
    short nY;               // Y Offset

    sal_Bool FormatTxt( SwTxtFormatInfo &rInf );
    void PaintTxt( const SwTxtPaintInfo &rInf ) const;

    inline void Fix( const KSHORT nNew ) { nFix = nNew; }
public:
    SwDropPortion( const MSHORT nLineCnt,
                   const KSHORT nDropHeight,
                   const KSHORT nDropDescent,
                   const KSHORT nDistance );
    virtual ~SwDropPortion();

    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
            void PaintDrop( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    virtual xub_StrLen GetCrsrOfst( const MSHORT nOfst ) const;

    inline MSHORT GetLines() const { return nLines; }
    inline KSHORT GetDistance() const { return nDistance; }
    inline KSHORT GetDropHeight() const { return nDropHeight; }
    inline KSHORT GetDropDescent() const { return nDropDescent; }
    inline KSHORT GetDropLeft() const { return Width() + nFix; }

    inline SwDropPortionPart* GetPart() const { return pPart; }
    inline void SetPart( SwDropPortionPart* pNew ) { pPart = pNew; }

    inline void SetY( short nNew )  { nY = nNew; }

    inline SwFont* GetFnt() const { return pPart ? &pPart->GetFont() : NULL; }

    static void DeleteDropCapCache();

    OUTPUT_OPERATOR
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
