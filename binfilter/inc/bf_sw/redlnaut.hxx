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
#ifndef _REDLNAUT_HXX
#define _REDLNAUT_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _COLOR_HXX //autogen
#include <tools/color.hxx>
#endif

#include "swtypes.hxx"
namespace binfilter {
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


} //namespace binfilter
#endif
