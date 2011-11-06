/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


