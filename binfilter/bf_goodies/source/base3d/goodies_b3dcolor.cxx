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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "b3dcolor.hxx"

namespace binfilter {
/*************************************************************************
|*
|* Farbwert interpolieren
|*
\************************************************************************/

void B3dColor::CalcInBetween(Color& rOld1, Color& rOld2, double t)
{
    if(rOld1 != rOld2)
    {
        UINT16 nFac = (UINT16)(t * 256.0);
        UINT16 nNegFac = 256 - nFac;
        SetRed((UINT8)(((nFac * (UINT16)rOld2.GetRed())
            + (nNegFac * (UINT16)rOld1.GetRed())) >> 8));
        SetGreen((UINT8)(((nFac * (UINT16)rOld2.GetGreen())
            + (nNegFac * (UINT16)rOld1.GetGreen())) >> 8));
        SetBlue((UINT8)(((nFac * (UINT16)rOld2.GetBlue())
            + (nNegFac * (UINT16)rOld1.GetBlue())) >> 8));
        SetTransparency((UINT8)(((nFac * (UINT16)rOld2.GetTransparency())
            + (nNegFac * (UINT16)rOld1.GetTransparency())) >> 8));
    }
    else
    {
        SetColor(rOld1.GetColor());
    }
}

/*************************************************************************
|*
|* Farbwert Mittelwert finden mit 2 Farben
|*
\************************************************************************/

void B3dColor::CalcMiddle(Color& rOld1, Color& rOld2)
{
    if(rOld1 != rOld2)
    {
        SetRed((UINT8)(((UINT16)rOld1.GetRed()
            + (UINT16)rOld2.GetRed())>>1));
        SetGreen((UINT8)(((UINT16)rOld1.GetGreen()
            + (UINT16)rOld2.GetGreen())>>1));
        SetBlue((UINT8)(((UINT16)rOld1.GetBlue()
            + (UINT16)rOld2.GetBlue())>>1));
        SetTransparency((UINT8)(((UINT16)rOld1.GetTransparency()
            + (UINT16)rOld2.GetTransparency())>>1));
    }
    else
    {
        SetColor(rOld1.GetColor());
    }
}

/*************************************************************************
|*
|* Farbaddition mit clamping
|*
\************************************************************************/

B3dColor& B3dColor::operator+= (const B3dColor& rCol)
{
    UINT16 nZwi;
    if(rCol.GetRed())
    {
        nZwi = (UINT16)GetRed() + (UINT16)rCol.GetRed();
        if(nZwi > 255)
            nZwi = 255;
        SetRed((UINT8)nZwi);
    }
    if(rCol.GetGreen())
    {
        nZwi = (UINT16)GetGreen() + (UINT16)rCol.GetGreen();
        if(nZwi > 255)
            nZwi = 255;
        SetGreen((UINT8)nZwi);
    }
    if(rCol.GetBlue())
    {
        nZwi = (UINT16)GetBlue() + (UINT16)rCol.GetBlue();
        if(nZwi > 255)
            nZwi = 255;
        SetBlue((UINT8)nZwi);
    }
    if(rCol.GetTransparency())
    {
        nZwi = (UINT16)GetTransparency() + (UINT16)rCol.GetTransparency();
        if(nZwi > 255)
            nZwi = 255;
        SetTransparency((UINT8)nZwi);
    }
    return *this;
}

/*************************************************************************
|*
|* Farbsubtraktion mit clamping
|*
\************************************************************************/

B3dColor& B3dColor::operator-= (const B3dColor& rCol)
{
    INT16 nZwi;
    if(rCol.GetRed())
    {
        nZwi = (INT16)GetRed() - (INT16)rCol.GetRed();
        if(nZwi < 0)
            nZwi = 0;
        SetRed((UINT8)nZwi);
    }
    if(rCol.GetGreen())
    {
        nZwi = (INT16)GetGreen() - (INT16)rCol.GetGreen();
        if(nZwi < 0)
            nZwi = 0;
        SetGreen((UINT8)nZwi);
    }
    if(rCol.GetBlue())
    {
        nZwi = (INT16)GetBlue() - (INT16)rCol.GetBlue();
        if(nZwi < 0)
            nZwi = 0;
        SetBlue((UINT8)nZwi);
    }
    if(rCol.GetTransparency())
    {
        nZwi = (INT16)GetTransparency() - (INT16)rCol.GetTransparency();
        if(nZwi < 0)
            nZwi = 0;
        SetTransparency((UINT8)nZwi);
    }
    return *this;
}

/*************************************************************************
|*
|* Farbaddition mit clamping, neue Farbe erzeugen
|*
\************************************************************************/

B3dColor B3dColor::operator+ (const B3dColor& rCol) const
{
    B3dColor aSum = *this;
    aSum += rCol;
    return aSum;
}

/*************************************************************************
|*
|* Farbsubtraktion mit clamping, neue Farbe erzeugen
|*
\************************************************************************/

B3dColor B3dColor::operator- (const B3dColor& rCol) const
{
    B3dColor aSub = *this;
    aSub -= rCol;
    return aSub;
}

/*************************************************************************
|*
|* Farbmultiplikation, d.h. Gewichtung der Farben aneinander
|*
\************************************************************************/

B3dColor& B3dColor::operator*= (const B3dColor& rCol)
{
    // urspruengliches Objekt ist die Farbe, rCol die zu berechnende
    // Gewichtung
    SetRed((UINT8)(((((UINT16)GetRed())+1)
        * (UINT16)rCol.GetRed()) >> 8));
    SetGreen((UINT8)(((((UINT16)GetGreen())+1)
        * (UINT16)rCol.GetGreen()) >> 8));
    SetBlue((UINT8)(((((UINT16)GetBlue())+1)
        * (UINT16)rCol.GetBlue()) >> 8));
    SetTransparency((UINT8)(((((UINT16)GetTransparency())+1)
        * (UINT16)rCol.GetTransparency()) >> 8));
    return *this;
}

/*************************************************************************
|*
|* Farbmultiplikation, neue Farbe erzeugen
|*
\************************************************************************/

B3dColor B3dColor::operator* (const B3dColor& rCol) const
{
    B3dColor aMul = *this;
    aMul *= rCol;
    return aMul;
}

/*************************************************************************
|*
|* Farbmultiplikation mit Faktor im Bereich [0.0 .. 1.0]
|*
\************************************************************************/

B3dColor& B3dColor::operator*= (const double fVal)
{
    ULONG nVal = (ULONG)(fVal * 65536.0);
    SetRed((UINT8)(((ULONG)GetRed() * nVal) >> 16));
    SetGreen((UINT8)(((ULONG)GetGreen() * nVal) >> 16));
    SetBlue((UINT8)(((ULONG)GetBlue() * nVal) >> 16));
    SetTransparency((UINT8)(((ULONG)GetTransparency() * nVal) >> 16));
    return *this;
}

/*************************************************************************
|*
|* Farbmultiplikation mit Faktor, neue Farbe erzeugen
|*
\************************************************************************/

B3dColor B3dColor::operator* (const double fVal) const
{
    B3dColor aMul = *this;
    aMul *= fVal;
    return aMul;
}

}//end of namespace binfilter

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
