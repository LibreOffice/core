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

#ifndef _DXFREPRD_HXX
#define _DXFREPRD_HXX

#include <dxfblkrd.hxx>
#include <dxftblrd.hxx>


//----------------------------------------------------------------------------
//--------------------Nebensachen---------------------------------------------
//----------------------------------------------------------------------------

//-------------------Eine 3D-Min/Max-Box--------------------------------------

class DXFBoundingBox {
public:
    sal_Bool bEmpty;
    double fMinX;
    double fMinY;
    double fMinZ;
    double fMaxX;
    double fMaxY;
    double fMaxZ;

    DXFBoundingBox() { bEmpty=sal_True; }
    void Union(const DXFVector & rVector);
};


//-------------------Die (konstante) Palette fuer DXF-------------------------

class DXFPalette {

public:

    DXFPalette();
    ~DXFPalette();

    sal_uInt8 GetRed(sal_uInt8 nIndex) const;
    sal_uInt8 GetGreen(sal_uInt8 nIndex) const;
    sal_uInt8 GetBlue(sal_uInt8 nIndex) const;

private:
    sal_uInt8 * pRed;
    sal_uInt8 * pGreen;
    sal_uInt8 * pBlue;
    void SetColor(sal_uInt8 nIndex, sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue);
};


//----------------------------------------------------------------------------
//-----------------DXF Datei lesen und repraesentieren------------------------
//----------------------------------------------------------------------------

class DXFRepresentation {

public:

    DXFPalette aPalette;
        // Die immer gleiche DXF-Farb-Palette

    DXFBoundingBox aBoundingBox;
        // Ist gleich den AutoCAD-Variablen EXTMIN, EXTMAX sofern in DXF-Datei
        // vorhanden, anderenfalls wird die BoundingBox berechnet (in Read()).

    DXFTables aTables;
        // Die Tabellen der DXF-Datei

    DXFBlocks aBlocks;
        // Die Bloecke der DXF-Datei

    DXFEntities aEntities;
        // Die Entities (aus der Entities-Section) der DXF-Datei

    rtl_TextEncoding mEnc;  // $DWGCODEPAGE

    double mfGlobalLineTypeScale; // $LTSCALE

    DXFRepresentation();
    ~DXFRepresentation();

        rtl_TextEncoding getTextEncoding() const;
        void setTextEncoding(rtl_TextEncoding aEnc);

        double getGlobalLineTypeScale() const;
        void setGlobalLineTypeScale(double fGlobalLineTypeScale);

    sal_Bool Read( SvStream & rIStream, sal_uInt16 nMinPercent, sal_uInt16 nMaxPercent);
        // Liesst die komplette DXF-Datei ein.

private:

    void ReadHeader(DXFGroupReader & rDGR);
    void CalcBoundingBox(const DXFEntities & rEntities,
                         DXFBoundingBox & rBox);
};

//----------------------------------------------------------------------------
//-------------------inlines--------------------------------------------------
//----------------------------------------------------------------------------

inline sal_uInt8 DXFPalette::GetRed(sal_uInt8 nIndex) const { return pRed[nIndex]; }
inline sal_uInt8 DXFPalette::GetGreen(sal_uInt8 nIndex) const { return pGreen[nIndex]; }
inline sal_uInt8 DXFPalette::GetBlue(sal_uInt8 nIndex) const { return pBlue[nIndex]; }
inline rtl_TextEncoding DXFRepresentation::getTextEncoding() const { return mEnc; }
inline void DXFRepresentation::setTextEncoding(rtl_TextEncoding aEnc) { mEnc = aEnc; }
inline double DXFRepresentation::getGlobalLineTypeScale() const { return mfGlobalLineTypeScale; }
inline void DXFRepresentation::setGlobalLineTypeScale(double fGlobalLineTypeScale) { mfGlobalLineTypeScale = fGlobalLineTypeScale; }

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
