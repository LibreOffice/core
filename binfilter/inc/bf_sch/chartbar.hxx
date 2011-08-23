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
#ifndef SCH_CHARTBAR_HXX
#define SCH_CHARTBAR_HXX
class Rectangle;
namespace binfilter {

class ChartModel;
class ChartAxis;
//Achtung! evtl. wird der default-Zuweisungsoperator benutzt!
//bei Erweiterungen bedenken! ganz wichtig!
class ChartBarDescriptor
{
    ChartAxis* mpAxis;
    ChartModel* mpModel;
    long nOverlapPercent;
    long nGapPercent;

    long nOver;      //Position des nächsten Balkens, kann gleich nBarWidth sein
    long nGap;       //Größe der Lücke
    long nPartWidth; //gesamte Breite je Spalte mit Gap
    long nColWidth;  //    "     "     "   "    ohne Gap
    long nBarWidth;  //Breite eines einzelnen Balkens
    long nCnt;       //Anzahl der Elemente, die der Berechnung zugrunde liegen

    long nCurrent;
    long nLeft;

public:

    inline void Assign(ChartModel* pModel,ChartAxis* pAxis)
    {
        mpModel=pModel;
        mpAxis=pAxis;
        SetGap(nGapPercent);//StarOne: Diese Eigenschaften hängen als Attribute an der Achse
        SetOverlap(nOverlapPercent);
    };
    void SetOverlap(long nPercent);
    long GetOverlap() const;
    long GetGap() const;
    void SetGap(long nPercent);

    ChartBarDescriptor(long nOverlap=0,long nGap=100):
        mpAxis(NULL),
        nOverlapPercent(nOverlap),
        nGapPercent(nGap)
    {
    }
    ChartBarDescriptor(const Rectangle& aRect,long nColCnt,long nRowCnt,long nOverlap=0,long nGap=100):
        mpAxis(NULL),
        nOverlapPercent(nOverlap),
        nGapPercent(nGap)
    {
        Create(aRect,nColCnt,nRowCnt);
    }

    ~ChartBarDescriptor(){};

    /// copy CTOR using the new model given
    ChartBarDescriptor( const ChartBarDescriptor& rSrc, ChartModel* pNewModel );

    inline long Middle()                      //aktuelle Spaltenmitte
    {
        return (nLeft + (nPartWidth - nGap)/2 );
    };
    inline long NextCol()                     //eine Spalte weiterschalten
    {
        nCurrent = nLeft + nPartWidth;
        nLeft = nCurrent;
        return nCurrent;
    };
    inline long BarLeft()
    {
        return nCurrent;
    };          //linke  BarPosition
    inline long BarRight()
    {
        return nCurrent+nBarWidth;
    };//rechte BarPsoition
    inline long BarWidth()
    {
        return nBarWidth;
    };
    inline long BarTop()
    {
        return nCurrent+nBarWidth;
    };//obere  BarPsoition
    inline long BarBottom()
    {
        return nCurrent;
    };          //untere BarPosition

    inline long NextBar() //innerhalb der Spalte einen Balken weiterschalten
    {
        nCurrent+=nOver;//if(nOver)nCurrent+=nOver;else nCurrent+=nBarWidth;
        return nCurrent;
    };

    void Create(const Rectangle& aRect,long nColCnt,long nRowCnt);
};
} //namespace binfilter
#endif


