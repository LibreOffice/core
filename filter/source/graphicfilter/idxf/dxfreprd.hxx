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
    BOOL bEmpty;
    double fMinX;
    double fMinY;
    double fMinZ;
    double fMaxX;
    double fMaxY;
    double fMaxZ;

    DXFBoundingBox() { bEmpty=TRUE; }
    void Union(const DXFVector & rVector);
};


//-------------------Die (konstante) Palette fuer DXF-------------------------

class DXFPalette {

public:

    DXFPalette();
    ~DXFPalette();

    BYTE GetRed(BYTE nIndex) const;
    BYTE GetGreen(BYTE nIndex) const;
    BYTE GetBlue(BYTE nIndex) const;

private:
    BYTE * pRed;
    BYTE * pGreen;
    BYTE * pBlue;
    void SetColor(BYTE nIndex, BYTE nRed, BYTE nGreen, BYTE nBlue);
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

    rtl_TextEncoding mEnc;	// $DWGCODEPAGE

    double mfGlobalLineTypeScale; // $LTSCALE

    DXFRepresentation();
    ~DXFRepresentation();

        rtl_TextEncoding getTextEncoding() const;
        void setTextEncoding(rtl_TextEncoding aEnc);

        double getGlobalLineTypeScale() const;
        void setGlobalLineTypeScale(double fGlobalLineTypeScale);

    BOOL Read( SvStream & rIStream, USHORT nMinPercent, USHORT nMaxPercent);
        // Liesst die komplette DXF-Datei ein.

private:

    void ReadHeader(DXFGroupReader & rDGR);
    void CalcBoundingBox(const DXFEntities & rEntities,
                         DXFBoundingBox & rBox);
};

//----------------------------------------------------------------------------
//-------------------inlines--------------------------------------------------
//----------------------------------------------------------------------------

inline BYTE DXFPalette::GetRed(BYTE nIndex) const { return pRed[nIndex]; }
inline BYTE DXFPalette::GetGreen(BYTE nIndex) const { return pGreen[nIndex]; }
inline BYTE DXFPalette::GetBlue(BYTE nIndex) const { return pBlue[nIndex]; }
inline rtl_TextEncoding DXFRepresentation::getTextEncoding() const { return mEnc; }
inline void DXFRepresentation::setTextEncoding(rtl_TextEncoding aEnc) { mEnc = aEnc; }
inline double DXFRepresentation::getGlobalLineTypeScale() const { return mfGlobalLineTypeScale; }
inline void DXFRepresentation::setGlobalLineTypeScale(double fGlobalLineTypeScale) { mfGlobalLineTypeScale = fGlobalLineTypeScale; }

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
