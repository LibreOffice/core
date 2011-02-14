/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _PORDROP_HXX
#define _PORDROP_HXX

#include "portxt.hxx"

class SwFont;

// DropCap-Cache, globale Variable, in txtinit.cxx initialisiert/zerstoert
// und in txtdrop.cxx benutzt bei der Initialenberechnung

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

public:
    SwDropPortionPart( SwFont& rFont, const xub_StrLen nL )
            : pFollow( 0 ), pFnt( &rFont ), nLen( nL ), nWidth( 0 ) {};
    ~SwDropPortionPart();

    inline SwDropPortionPart* GetFollow() const { return pFollow; };
    inline void SetFollow( SwDropPortionPart* pNew ) { pFollow = pNew; };
    inline SwFont& GetFont() const { return *pFnt; }
    inline xub_StrLen GetLen() const { return nLen; }
    inline sal_uInt16 GetWidth() const { return nWidth; }
    inline void SetWidth( sal_uInt16 nNew )  { nWidth = nNew; }
};

/*************************************************************************
 *                      class SwDropPortion
 *************************************************************************/

class SwDropPortion : public SwTxtPortion
{
    friend class SwDropCapCache;
    SwDropPortionPart* pPart; // due to script / attribute changes
    MSHORT nLines;          // Anzahl der Zeilen
    KSHORT nDropHeight;     // Hoehe
    KSHORT nDropDescent;    // Abstand zur naechsten Zeile
    KSHORT nDistance;       // Abstand zum Text
    KSHORT nFix;            // Fixposition
    short nX;               // X-PaintOffset
    short nY;               // Y-Offset

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
