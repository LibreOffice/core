/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: outlinfo.hxx,v $
 * $Revision: 1.6.108.1 $
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

#ifndef _OUTLINFO_HXX
#define _OUTLINFO_HXX

#include <tools/gen.hxx>
#include <vcl/outdev.hxx>

// ---------------
// - SdLaserInfo -
// ---------------

struct OutlinerCharacter
{
    Rectangle   aRect;
    ULONG       nPara;
    Color       aColor;

    // #101500# Removed CharCode, it's only used in one place to compare
    // for single space character. This can be done at creation, too.
    //USHORT        nCharCode;

    OutlinerCharacter( const Rectangle& _rRect, ULONG _nPara, const Color& _rCol /* #101500#, USHORT _nCharCode*/ )
    :   aRect( _rRect ),
        nPara( _nPara ),
        aColor( _rCol )
    {
    }
};

// -----------------------------------------------------------------------------

struct OutlinerParagraph
{
    Rectangle   aRect;
    ULONG       nCharCount;

                OutlinerParagraph() : nCharCount( 0 ) {}
                ~OutlinerParagraph() {};
};

// ----------------
// - OutlinerInfo -
// ----------------

class DrawPortionInfo;
class SdDrawDocument;
class SdrRectObj;

class OutlinerInfo
{
private:

    List                aCharacterList;
    Rectangle           aObjBound;
    Rectangle           aParaBound;
    Point               aTextOffset;
    OutlinerParagraph*  pParagraphs;
    OutputDevice*       mpOut;
    ULONG               nParaCount;
    ULONG               nCurPara;
    long                nExtraData;
    BOOL                bInit;
    BOOL                mbVertical;

                        OutlinerInfo( const OutlinerInfo& ) {}
    OutlinerInfo&       operator=( const OutlinerInfo& ) { return *this; }

                        DECL_LINK( DrawPortionHdl, DrawPortionInfo* );

public:

                        OutlinerInfo();
                        ~OutlinerInfo();

    void                Clear();

    void                SetTextObj( SdDrawDocument* pDoc, SdrRectObj* pObj, OutputDevice* _pOut );

    const Rectangle&    GetObjRect() const { return aObjBound; }
    const Rectangle&    GetTextRect() const { return aParaBound; }
    const Point&        GetTextOffset() const { return aTextOffset; }

    ULONG           GetParaCount() const { return nParaCount; }

    const Rectangle&    GetParaRect( const ULONG nPara ) const;
    BOOL                GetParaCharCount( const ULONG nPara ) const;

    ULONG               GetCharacterCount() const { return aCharacterList.Count(); }
    OutlinerCharacter*  GetCharacter( ULONG nPos ) const { return (OutlinerCharacter*) aCharacterList.GetObject( nPos ); };
    OutlinerCharacter*  GetFirstCharacter() { return (OutlinerCharacter*) aCharacterList.First(); }
    OutlinerCharacter*  GetNextCharacter() { return (OutlinerCharacter*) aCharacterList.Next(); }

    void                SetExtraData( const long _nExtraData = 0L ) { nExtraData = _nExtraData; }
    long            GetExtraData() const { return nExtraData; }

    BOOL IsVertical() const { return mbVertical; }
};

#endif
