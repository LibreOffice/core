/*************************************************************************
 *
 *  $RCSfile: pordrop.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2003-12-09 11:37:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    USHORT nWidth;

public:
    SwDropPortionPart( SwFont& rFont, const xub_StrLen nL )
            : pFollow( 0 ), pFnt( &rFont ), nLen( nL ), nWidth( 0 ) {};
    ~SwDropPortionPart();

    inline SwDropPortionPart* GetFollow() const { return pFollow; };
    inline void SetFollow( SwDropPortionPart* pNew ) { pFollow = pNew; };
    inline SwFont& GetFont() const { return *pFnt; }
    inline xub_StrLen GetLen() const { return nLen; }
    inline USHORT GetWidth() const { return nWidth; }
    inline void SetWidth( USHORT nNew )  { nWidth = nNew; }
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
