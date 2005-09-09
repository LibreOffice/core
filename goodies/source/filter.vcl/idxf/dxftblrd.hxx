/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dxftblrd.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:56:43 $
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

#ifndef _DXFTBLRD_HXX
#define _DXFTBLRD_HXX

#ifndef _DXFGRPRD_HXX
#include <dxfgrprd.hxx>
#endif

#ifndef _DXFVEC_HXX
#include <dxfvec.hxx>
#endif


//----------------------------------------------------------------------------
//------------------ Linien-Typ ----------------------------------------------
//----------------------------------------------------------------------------

#define DXF_MAX_DASH_COUNT 32

class DXFLType {

public:

    DXFLType * pSucc;

    char sName[DXF_MAX_STRING_LEN+1];        //  2
    long nFlags;                             // 70
    char sDescription[DXF_MAX_STRING_LEN+1]; //  3
    long nDashCount;                         // 73
    double fPatternLength;                   // 40
    double fDash[DXF_MAX_DASH_COUNT];        // 49,49,...

    DXFLType();
    void Read(DXFGroupReader & rDGR);
};


//----------------------------------------------------------------------------
//------------------ Layer ---------------------------------------------------
//----------------------------------------------------------------------------

class DXFLayer {

public:

    DXFLayer * pSucc;

    char sName[DXF_MAX_STRING_LEN+1];     //  2
    long nFlags;                          // 70
    long nColor;                          // 62
    char sLineType[DXF_MAX_STRING_LEN+1]; //  6

    DXFLayer();
    void Read(DXFGroupReader & rDGR);
};


//----------------------------------------------------------------------------
//------------------ Style ---------------------------------------------------
//----------------------------------------------------------------------------

class DXFStyle {

public:

    DXFStyle * pSucc;

    char sName[DXF_MAX_STRING_LEN+1];         //  2
    long nFlags;                              // 70
    double fHeight;                           // 40
    double fWidthFak;                         // 41
    double fOblAngle;                         // 50
    long nTextGenFlags;                       // 71
    double fLastHeightUsed;                   // 42
    char sPrimFontFile[DXF_MAX_STRING_LEN+1]; //  3
    char sBigFontFile[DXF_MAX_STRING_LEN+1];  //  4

    DXFStyle();
    void Read(DXFGroupReader & rDGR);
};


//----------------------------------------------------------------------------
//------------------ VPort ---------------------------------------------------
//----------------------------------------------------------------------------

class DXFVPort {

public:

    DXFVPort * pSucc;

    char sName[DXF_MAX_STRING_LEN+1]; //  2
    long nFlags;                      // 70
    double fMinX;                     // 10
    double fMinY;                     // 20
    double fMaxX;                     // 11
    double fMaxY;                     // 21
    double fCenterX;                  // 12
    double fCenterY;                  // 22
    double fSnapBaseX;                // 13
    double fSnapBaseY;                // 23
    double fSnapSapcingX;             // 14
    double fSnapSpacingY;             // 24
    double fGridX;                    // 15
    double fGridY;                    // 25
    DXFVector aDirection;             // 16,26,36
    DXFVector aTarget;                // 17,27,37
    double fHeight;                   // 40
    double fAspectRatio;              // 41
    double fLensLength;               // 42
    double fFrontClipPlane;           // 43
    double fBackClipPlane;            // 44
    double fTwistAngle;               // 51
    long nStatus;                     // 68
    long nID;                         // 69
    long nMode;                       // 71
    long nCircleZoomPercent;          // 72
    long nFastZoom;                   // 73
    long nUCSICON;                    // 74
    long nSnap;                       // 75
    long nGrid;                       // 76
    long nSnapStyle;                  // 77
    long nSnapIsopair;                // 78

    DXFVPort();
    void Read(DXFGroupReader & rDGR);
};


//----------------------------------------------------------------------------
//------------------ Tabellen ------------------------------------------------
//----------------------------------------------------------------------------

class DXFTables {

public:

    DXFLType * pLTypes; // Liste der Linientypen
    DXFLayer * pLayers; // Liste der Layers
    DXFStyle * pStyles; // Liste der Styles
    DXFVPort * pVPorts; // Liste der Viewports

    DXFTables();
    ~DXFTables();

    void Read(DXFGroupReader & rDGR);
        // Liest die Tabellen ein bis zu einem ENDSEC oder EOF
        // (unbekannte Dinge/Tabellen werden uebersprungen)

    void Clear();

    // Suche nach Tabelleneintraegen:
    DXFLType * SearchLType(const char * pName) const;
    DXFLayer * SearchLayer(const char * pName) const;
    DXFStyle * SearchStyle(const char * pName) const;
    DXFVPort * SearchVPort(const char * pName) const;

};

#endif


