/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <limits.h>
#include <tools/shl.hxx>
#include <vcl/svapp.hxx>
#include <editeng/editrids.hrc>
#include <editeng/paperinf.hxx>
#include <editeng/eerdll.hxx>

/*--------------------------------------------------------------------
    Description:    Is the printer valid
 --------------------------------------------------------------------*/

inline sal_Bool IsValidPrinter( const Printer* pPtr )
{
    return pPtr->GetName().isEmpty() ? sal_False : sal_True;
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

OUString SvxPaperInfo::GetName( Paper ePaper )
{
    return Printer::GetPaperName( ePaper );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
