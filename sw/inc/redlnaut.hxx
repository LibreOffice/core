/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: redlnaut.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:06:46 $
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
#ifndef _REDLNAUT_HXX
#define _REDLNAUT_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#include "swtypes.hxx"

class SfxItemSet;

class SwRedlineAuthor
{
    String sAuthor;
    Color aChgLineColor;
    SfxItemSet *pInsAttrSet, *pDelAttrSet, *pFmtAttrSet;
    SwHoriOrient eChgLineOrient;
    BYTE cDelChar;
public:
    SwRedlineAuthor( SwAttrPool& rPool, const String& );
    SwRedlineAuthor( const SwRedlineAuthor& );
    ~SwRedlineAuthor();

    SwRedlineAuthor& operator=( const SwRedlineAuthor& );

    const String& GetAuthor() const { return sAuthor; }

    SfxItemSet& GetInsAttrSet() const { return *pInsAttrSet; }
    SfxItemSet& GetDelAttrSet() const { return *pDelAttrSet; }
    SfxItemSet& GetFmtAttrSet() const { return *pFmtAttrSet; }

    const Color& GetChgLineColor() const { return aChgLineColor; }
    void SetChgLineColor( const Color& rCol ) { aChgLineColor = rCol; }

    SwHoriOrient GetChgLineOrient() const { return eChgLineOrient; }
    void SetChgLineOrient( SwHoriOrient eVal ) { eChgLineOrient = eVal; }

    BYTE GetDelChar() const { return cDelChar; }
    void SetDelChar( BYTE cCh = 0 ) { cDelChar = cCh; }
};


#endif
