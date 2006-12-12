/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outlinfo.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:46:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OUTLINFO_HXX
#define _OUTLINFO_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SV_OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

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

    const ULONG         GetParaCount() const { return nParaCount; }

    const Rectangle&    GetParaRect( const ULONG nPara ) const;
    BOOL                GetParaCharCount( const ULONG nPara ) const;

    ULONG               GetCharacterCount() const { return aCharacterList.Count(); }
    OutlinerCharacter*  GetCharacter( ULONG nPos ) const { return (OutlinerCharacter*) aCharacterList.GetObject( nPos ); };
    OutlinerCharacter*  GetFirstCharacter() { return (OutlinerCharacter*) aCharacterList.First(); }
    OutlinerCharacter*  GetNextCharacter() { return (OutlinerCharacter*) aCharacterList.Next(); }

    void                SetExtraData( const long _nExtraData = 0L ) { nExtraData = _nExtraData; }
    const long          GetExtraData() const { return nExtraData; }

    BOOL IsVertical() const { return mbVertical; }
};

#endif
