/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_editeng.hxx"

// include ---------------------------------------------------------------

#include <limits.h>
#include <tools/shl.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <editeng/editrids.hrc>
#include <editeng/paperinf.hxx>
#include <editeng/eerdll.hxx>

/*--------------------------------------------------------------------
    Description:    Is the printer valid
 --------------------------------------------------------------------*/

inline sal_Bool IsValidPrinter( const Printer* pPtr )
{
    return pPtr->GetName().Len() ? sal_True : sal_False;
}

//------------------------------------------------------------------------

Size SvxPaperInfo::GetPaperSize( Paper ePaper, MapUnit eUnit )
{
    PaperInfo aInfo(ePaper);
    Size aRet(aInfo.getWidth(), aInfo.getHeight()); // in 100thMM
    return eUnit == MAP_100TH_MM ? aRet : OutputDevice::LogicToLogic(aRet, MAP_100TH_MM, eUnit);
}

/*------------------------------------------------------------------------
 Description:   Return the paper size of the printer, aligned to our
                own sizes. If no Printer is set in the system, A4 portrait
                will be delivered as the default paper size.
------------------------------------------------------------------------*/

//Is this method may be confused about the units it returns ?
//Always returns TWIPS for known paper sizes or on failure.
//But in the case of PAPER_USER paper and with a Printer with a mapmode set
//will return in those printer units ?
Size SvxPaperInfo::GetPaperSize( const Printer* pPrinter )
{
    if ( !IsValidPrinter(pPrinter) )
        return GetPaperSize( PAPER_A4 );
    const Paper ePaper = pPrinter->GetPaper();

    if ( ePaper == PAPER_USER )
    {
        // Orientation not take into account, as the right size has
        // been already set by SV
        Size aPaperSize = pPrinter->GetPaperSize();
        const Size aInvalidSize;

        if ( aPaperSize == aInvalidSize )
            return GetPaperSize(PAPER_A4);
        MapMode aMap1 = pPrinter->GetMapMode();
        MapMode aMap2;

        if ( aMap1 == aMap2 )
            aPaperSize =
                pPrinter->PixelToLogic( aPaperSize, MapMode( MAP_TWIP ) );
        return aPaperSize;
    }

    const Orientation eOrient = pPrinter->GetOrientation();
    Size aSize( GetPaperSize( ePaper ) );
        // for Landscape exchange the pages, has already been done by SV
    if ( eOrient == ORIENTATION_LANDSCAPE )
        Swap( aSize );
    return aSize;
}

// -----------------------------------------------------------------------

Paper SvxPaperInfo::GetSvxPaper( const Size &rSize, MapUnit eUnit, bool bSloppy )
{
    Size aSize(eUnit == MAP_100TH_MM ? rSize : OutputDevice::LogicToLogic(rSize, eUnit, MAP_100TH_MM));
    PaperInfo aInfo(aSize.Width(), aSize.Height());
    if (bSloppy)
        aInfo.doSloppyFit();
    return aInfo.getPaper();
}

// -----------------------------------------------------------------------

long SvxPaperInfo::GetSloppyPaperDimension( long nSize, MapUnit eUnit )
{
    nSize = eUnit == MAP_100TH_MM ? nSize : OutputDevice::LogicToLogic(nSize, eUnit, MAP_100TH_MM);
    nSize = PaperInfo::sloppyFitPageDimension(nSize);
    return eUnit == MAP_100TH_MM ? nSize : OutputDevice::LogicToLogic(nSize, MAP_100TH_MM, eUnit);
}

// -----------------------------------------------------------------------

Size SvxPaperInfo::GetDefaultPaperSize( MapUnit eUnit )
{
    PaperInfo aInfo(PaperInfo::getSystemDefaultPaper());
    Size aRet(aInfo.getWidth(), aInfo.getHeight());
    return eUnit == MAP_100TH_MM ? aRet : OutputDevice::LogicToLogic(aRet, MAP_100TH_MM, eUnit);
}

/*------------------------------------------------------------------------
 Description:   String representation for the SV-defines of paper size
------------------------------------------------------------------------*/

String SvxPaperInfo::GetName( Paper ePaper )
{
    return String( Printer::GetPaperName( ePaper ) );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
