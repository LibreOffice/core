/*************************************************************************
 *
 *  $RCSfile: hints.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:17 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "hints.hxx"

// -----------------------------------------------------------------------

TYPEINIT1(ScPaintHint, SfxHint);
TYPEINIT1(ScUpdateRefHint, SfxHint);
TYPEINIT1(ScPointerChangedHint, SfxHint);
TYPEINIT1(ScLinkRefreshedHint, SfxHint);
TYPEINIT1(ScAutoStyleHint, SfxHint);

// -----------------------------------------------------------------------
//      ScPaintHint - Angabe, was neu gezeichnet werden muss
// -----------------------------------------------------------------------

ScPaintHint::ScPaintHint() :
    aRange( 0,0,0, MAXCOL,MAXROW,MAXTAB ),
    nParts( PAINT_ALL ),
    bPrint( TRUE )
{
}

ScPaintHint::ScPaintHint( const ScRange& rRng, USHORT nPaint ) :
    aRange( rRng ),
    nParts( nPaint ),
    bPrint( TRUE )
{
}

ScPaintHint::~ScPaintHint()
{
}

// -----------------------------------------------------------------------
//      ScUpdateRefHint - Referenz-Updaterei
// -----------------------------------------------------------------------

ScUpdateRefHint::ScUpdateRefHint( UpdateRefMode eMode, const ScRange& rR,
                                    short nX, short nY, short nZ ) :
    eUpdateRefMode( eMode ),
    aRange( rR ),
    nDx( nX ),
    nDy( nY ),
    nDz( nZ )
{
}

ScUpdateRefHint::~ScUpdateRefHint()
{
}

// -----------------------------------------------------------------------
//      ScPointerChangedHint - Pointer ist ungueltig geworden
// -----------------------------------------------------------------------

ScPointerChangedHint::ScPointerChangedHint( USHORT nF ) :
    nFlags( nF )
{
}

ScPointerChangedHint::~ScPointerChangedHint()
{
}

// -----------------------------------------------------------------------
//      ScLinkRefreshedHint - a link has been refreshed
// -----------------------------------------------------------------------

ScLinkRefreshedHint::ScLinkRefreshedHint() :
    nLinkType( SC_LINKREFTYPE_NONE ),
    nDdeMode( 0 )
{
}

ScLinkRefreshedHint::~ScLinkRefreshedHint()
{
}

void ScLinkRefreshedHint::SetSheetLink( const String& rSourceUrl )
{
    nLinkType = SC_LINKREFTYPE_SHEET;
    aUrl = rSourceUrl;
}

void ScLinkRefreshedHint::SetDdeLink(
            const String& rA, const String& rT, const String& rI, BYTE nM )
{
    nLinkType = SC_LINKREFTYPE_DDE;
    aDdeAppl  = rA;
    aDdeTopic = rT;
    aDdeItem  = rI;
    nDdeMode  = nM;
}

void ScLinkRefreshedHint::SetAreaLink( const ScAddress& rPos )
{
    nLinkType = SC_LINKREFTYPE_AREA;
    aDestPos = rPos;
}

// -----------------------------------------------------------------------
//      ScAutoStyleHint - STYLE() function has been called
// -----------------------------------------------------------------------

ScAutoStyleHint::ScAutoStyleHint( const ScRange& rR, const String& rSt1,
                                        ULONG nT, const String& rSt2 ) :
    aRange( rR ),
    aStyle1( rSt1 ),
    aStyle2( rSt2 ),
    nTimeout( nT )
{
}

ScAutoStyleHint::~ScAutoStyleHint()
{
}




