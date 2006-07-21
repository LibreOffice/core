/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: reffind.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:42:41 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <string.h>

#include "reffind.hxx"
#include "global.hxx"
#include "compiler.hxx"


// STATIC DATA -----------------------------------------------------------

//  incl. Doppelpunkt -> Doppelte Referenzen werden einzeln behandelt
const sal_Unicode __FAR_DATA ScRefFinder::pDelimiters[] = {
    '=','(',')',';','+','-','*','/','^','&',' ','{','}','<','>',':', 0
};

// =======================================================================

inline BOOL IsText( sal_Unicode c )
{
    return !ScGlobal::UnicodeStrChr( ScRefFinder::pDelimiters, c );
}

inline BOOL IsText( BOOL& bQuote, sal_Unicode c )
{
    if ( c == '\'' )
    {
        bQuote = !bQuote;
        return TRUE;
    }
    if ( bQuote )
        return TRUE;
    return IsText( c );
}

ScRefFinder::ScRefFinder(const String& rFormula, ScDocument* pDocument) :
    aFormula( rFormula ),
    pDoc( pDocument )
{
    nSelStart = nSelEnd = nFound = 0;
}

ScRefFinder::~ScRefFinder()
{
}

USHORT lcl_NextFlags( USHORT nOld )
{
    USHORT nNew = nOld & 7;                 // die drei Abs-Flags
    nNew = ( nNew - 1 ) & 7;                // weiterzaehlen

    if (!(nOld & SCA_TAB_3D))
        nNew &= ~SCA_TAB_ABSOLUTE;          // nicht 3D -> nie absolut!

    return ( nOld & 0xfff8 ) | nNew;
}

void ScRefFinder::ToggleRel( xub_StrLen nStartPos, xub_StrLen nEndPos )
{
    xub_StrLen nLen = aFormula.Len();
    if (!nLen)
        return;
    const sal_Unicode* pSource = aFormula.GetBuffer();      // fuer schnellen Zugriff

    //  Selektion erweitern, und statt Selektion Start- und Endindex

    if ( nEndPos < nStartPos )
    {
        xub_StrLen nTemp = nStartPos; nStartPos = nEndPos; nEndPos = nTemp;
    }
    while (nStartPos > 0 && IsText(pSource[nStartPos - 1]) )
        --nStartPos;
    if (nEndPos)
        --nEndPos;
    while (nEndPos+1 < nLen && IsText(pSource[nEndPos + 1]) )
        ++nEndPos;

    String aResult;
    String aExpr;
    String aSep;
    ScAddress aAddr;
    nFound = 0;

    xub_StrLen nLoopStart = nStartPos;
    while ( nLoopStart <= nEndPos )
    {
        //  Formel zerlegen

        xub_StrLen nEStart = nLoopStart;
        while ( nEStart <= nEndPos && !IsText(pSource[nEStart]) )
            ++nEStart;

        BOOL bQuote = FALSE;
        xub_StrLen nEEnd = nEStart;
        while ( nEEnd <= nEndPos && IsText(bQuote,pSource[nEEnd]) )
            ++nEEnd;

        aSep  = aFormula.Copy( nLoopStart, nEStart-nLoopStart );
        aExpr = aFormula.Copy( nEStart, nEEnd-nEStart );

        //  Test, ob aExpr eine Referenz ist

        USHORT nResult = aAddr.Parse( aExpr, pDoc );
        if ( nResult & SCA_VALID )
        {
            USHORT nFlags = lcl_NextFlags( nResult );
            aAddr.Format( aExpr, nFlags, pDoc );

            xub_StrLen nAbsStart = nStartPos+aResult.Len()+aSep.Len();

            if (!nFound)                            // erste Referenz ?
                nSelStart = nAbsStart;
            nSelEnd = nAbsStart+aExpr.Len();        // Selektion, keine Indizes
            ++nFound;
        }

        //  zusammenbauen

        aResult += aSep;
        aResult += aExpr;

        nLoopStart = nEEnd;
    }

    String aTotal = aFormula.Copy( 0, nStartPos );
    aTotal += aResult;
    aTotal += aFormula.Copy( nEndPos+1 );

    aFormula = aTotal;
}




