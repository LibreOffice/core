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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <string.h>

#include "reffind.hxx"
#include "global.hxx"
#include "compiler.hxx"
#include "document.hxx"

// STATIC DATA -----------------------------------------------------------

//  incl. Doppelpunkt -> Doppelte Referenzen werden einzeln behandelt
const sal_Unicode __FAR_DATA ScRefFinder::pDelimiters[] = {
    '=','(',')',';','+','-','*','/','^','&',' ','{','}','<','>',':', 0
};

// =======================================================================

inline sal_Bool IsText( sal_Unicode c )
{
    return !ScGlobal::UnicodeStrChr( ScRefFinder::pDelimiters, c );
}

inline sal_Bool IsText( sal_Bool& bQuote, sal_Unicode c )
{
    if ( c == '\'' )
    {
        bQuote = !bQuote;
        return sal_True;
    }
    if ( bQuote )
        return sal_True;
    return IsText( c );
}

ScRefFinder::ScRefFinder(const String& rFormula, ScDocument* pDocument,
             formula::FormulaGrammar::AddressConvention eConvP) :
    aFormula( rFormula ),
    eConv( eConvP ),
    pDoc( pDocument )
{
    nSelStart = nSelEnd = nFound = 0;
}

ScRefFinder::~ScRefFinder()
{
}

sal_uInt16 lcl_NextFlags( sal_uInt16 nOld )
{
    sal_uInt16 nNew = nOld & 7;                 // die drei Abs-Flags
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

        sal_Bool bQuote = sal_False;
        xub_StrLen nEEnd = nEStart;
        while ( nEEnd <= nEndPos && IsText(bQuote,pSource[nEEnd]) )
            ++nEEnd;

        aSep  = aFormula.Copy( nLoopStart, nEStart-nLoopStart );
        aExpr = aFormula.Copy( nEStart, nEEnd-nEStart );

        //  Test, ob aExpr eine Referenz ist

        sal_uInt16 nResult = aAddr.Parse( aExpr, pDoc, pDoc->GetAddressConvention() );
        if ( nResult & SCA_VALID )
        {
            sal_uInt16 nFlags = lcl_NextFlags( nResult );
            aAddr.Format( aExpr, nFlags, pDoc, pDoc->GetAddressConvention() );

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




