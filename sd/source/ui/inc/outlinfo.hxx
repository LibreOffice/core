/*************************************************************************
 *
 *  $RCSfile: outlinfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2001-03-20 16:49:46 $
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
    USHORT      nCharCode;

                OutlinerCharacter( const Rectangle& _rRect, ULONG _nPara, const Color& _rCol, USHORT _nCharCode ) :
                    aRect( _rRect ), nPara( _nPara ), aColor( _rCol ), nCharCode( _nCharCode ) {}
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
    OutputDevice*       pOut;
    ULONG               nParaCount;
    ULONG               nCurPara;
    long                nExtraData;
    BOOL                bInit;
    BOOL                mbVertical;

                        OutlinerInfo( const OutlinerInfo& rInfo ) {}
    OutlinerInfo&       operator=( const OutlinerInfo& rInfo ) { return *this; }

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
