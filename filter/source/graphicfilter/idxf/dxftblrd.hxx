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

#ifndef _DXFTBLRD_HXX
#define _DXFTBLRD_HXX

#include <dxfgrprd.hxx>
#include <dxfvec.hxx>


//----------------------------------------------------------------------------
//------------------ Linien-Typ ----------------------------------------------
//----------------------------------------------------------------------------

#define DXF_MAX_DASH_COUNT 32

class DXFLType {

public:

    DXFLType * pSucc;

    OString m_sName;                         //  2
    long nFlags;                             // 70
    OString m_sDescription;                  //  3
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

    OString m_sName;                      //  2
    long nFlags;                          // 70
    long nColor;                          // 62
    OString m_sLineType;                  //  6

    DXFLayer();
    void Read(DXFGroupReader & rDGR);
};


//----------------------------------------------------------------------------
//------------------ Style ---------------------------------------------------
//----------------------------------------------------------------------------

class DXFStyle {

public:

    DXFStyle * pSucc;

    OString m_sName;                          //  2
    long nFlags;                              // 70
    double fHeight;                           // 40
    double fWidthFak;                         // 41
    double fOblAngle;                         // 50
    long nTextGenFlags;                       // 71
    double fLastHeightUsed;                   // 42
    OString m_sPrimFontFile;                  //  3
    OString m_sBigFontFile;                   //  4

    DXFStyle();
    void Read(DXFGroupReader & rDGR);
};


//----------------------------------------------------------------------------
//------------------ VPort ---------------------------------------------------
//----------------------------------------------------------------------------

class DXFVPort {

public:

    DXFVPort * pSucc;

    OString m_sName;                  //  2
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
    DXFLType * SearchLType(OString const& rName) const;
    DXFLayer * SearchLayer(OString const& rName) const;
    DXFVPort * SearchVPort(OString const& rName) const;

};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
