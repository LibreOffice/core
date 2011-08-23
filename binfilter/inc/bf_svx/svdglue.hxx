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

#ifndef _SVDGLUE_HXX
#define _SVDGLUE_HXX
class Window;
class OutputDevice;
class SvStream;
namespace binfilter {
class SdrObject;
}//end of namespace binfilter
#include <tools/contnr.hxx>
#include <tools/gen.hxx>
namespace binfilter {
////////////////////////////////////////////////////////////////////////////////////////////////////

#define SDRESC_SMART  0x0000
#define SDRESC_LEFT   0x0001
#define SDRESC_RIGHT  0x0002
#define SDRESC_TOP    0x0004
#define SDRESC_BOTTOM 0x0008
#define SDRESC_LO    0x0010 /* ni */
#define SDRESC_LU    0x0020 /* ni */
#define SDRESC_RO    0x0040 /* ni */
#define SDRESC_RU    0x0080 /* ni */
#define SDRESC_HORZ  (SDRESC_LEFT|SDRESC_RIGHT)
#define SDRESC_VERT  (SDRESC_TOP|SDRESC_BOTTOM)
#define SDRESC_ALL   0x00FF

#define SDRHORZALIGN_CENTER   0x0000
#define SDRHORZALIGN_LEFT     0x0001
#define SDRHORZALIGN_RIGHT    0x0002
#define SDRHORZALIGN_DONTCARE 0x0010
#define SDRVERTALIGN_CENTER   0x0000
#define SDRVERTALIGN_TOP      0x0100
#define SDRVERTALIGN_BOTTOM   0x0200
#define SDRVERTALIGN_DONTCARE 0x1000

class SdrGluePoint {
    // Bezugspunkt ist SdrObject::GetSnapRect().Center()
    // bNoPercent=FALSE: Position ist -5000..5000 (1/100)% bzw. 0..10000 (je nach Align)
    // bNoPercent=TRUE : Position ist in log Einh, rel zum Bezugspunkt
    Point    aPos;
    USHORT   nEscDir;
    USHORT   nId;
    USHORT   nAlign;
    FASTBOOL bNoPercent:1;
    FASTBOOL bReallyAbsolute:1; // Temporaer zu setzen fuer Transformationen am Bezugsobjekt
public:
    SdrGluePoint(): nEscDir(SDRESC_SMART),nId(0),nAlign(0) { bNoPercent=FALSE; bReallyAbsolute=FALSE; }
    SdrGluePoint(const Point& rNewPos, FASTBOOL bNewPercent=TRUE, USHORT nNewAlign=0): aPos(rNewPos),nEscDir(SDRESC_SMART),nId(0),nAlign(nNewAlign) { bNoPercent=!bNewPercent; bReallyAbsolute=FALSE; }
    FASTBOOL operator==(const SdrGluePoint& rCmpGP) const   { return aPos==rCmpGP.aPos && nEscDir==rCmpGP.nEscDir && nId==rCmpGP.nId && nAlign==rCmpGP.nAlign && bNoPercent==rCmpGP.bNoPercent && bReallyAbsolute==rCmpGP.bReallyAbsolute; }
    FASTBOOL operator!=(const SdrGluePoint& rCmpGP) const   { return !operator==(rCmpGP); }
    const Point& GetPos() const                             { return aPos; }
    void         SetPos(const Point& rNewPos)               { aPos=rNewPos; }
    USHORT       GetEscDir() const                          { return nEscDir; }
    void         SetEscDir(USHORT nNewEsc)                  { nEscDir=nNewEsc; }
    /*FASTBOOL   IsEscapeSmart() const                      { return nEscDir==SDRESC_SMART; }
    void         SetEscapeSmart()                           { nEscDir=SDRESC_SMART; }
    FASTBOOL     IsEscapeLeft() const                       { return (nEscDir&SDRESC_LEFT)!=0; }
    void         SetEscapeLeft(FASTBOOL bOn)                { nEscDir=bOn ? (nEscDir|SDRESC_LEFT) : (nEscDir&~SDRESC_LEFT); }
    FASTBOOL     IsEscapeRight() const                      { return (nEscDir&SDRESC_RIGHT)!=0; }
    void         SetEscapeRight(FASTBOOL bOn)               { nEscDir=bOn ? (nEscDir|SDRESC_RIGHT) : (nEscDir&~SDRESC_RIGHT); }
    FASTBOOL     IsEscapeTop() const                        { return (nEscDir&SDRESC_TOP)!=0; }
    void         SetEscapeTop(FASTBOOL bOn)                 { nEscDir=bOn ? (nEscDir|SDRESC_TOP) : (nEscDir&~SDRESC_TOP); }
    FASTBOOL     IsEscapeBottom() const                     { return (nEscDir&SDRESC_BOTTOM)!=0; }
    void         SetEscapeBottom(FASTBOOL bOn)              { nEscDir=bOn ? (nEscDir|SDRESC_BOTTOM) : (nEscDir&~SDRESC_BOTTOM); }*/
    USHORT       GetId() const                              { return nId; }
    void         SetId(USHORT nNewId)                       { nId=nNewId; }
    FASTBOOL     IsPercent() const                          { return !bNoPercent; }
    void         SetPercent(FASTBOOL bOn)                   { bNoPercent=!bOn; }
    // Temporaer zu setzen fuer Transformationen am Bezugsobjekt
    FASTBOOL     IsReallyAbsolute() const                   { return bReallyAbsolute; }
    USHORT       GetAlign() const                           { return nAlign; }
    void         SetAlign(USHORT nAlg)                      { nAlign=nAlg; }
    USHORT       GetHorzAlign() const                       { return nAlign&0x00FF; }
    void         SetHorzAlign(USHORT nAlg)                  { nAlign=(nAlign&0xFF00)|(nAlg&0x00FF); }
    USHORT       GetVertAlign() const                       { return nAlign&0xFF00; }
    void         SetVertAlign(USHORT nAlg)                  { nAlign=(nAlign&0x00FF)|(nAlg&0xFF00); }
    Point        GetAbsolutePos(const SdrObject& rObj) const;
    friend SvStream& operator<<(SvStream& rOut, const SdrGluePoint& rGP);
    friend SvStream& operator>>(SvStream& rIn, SdrGluePoint& rGP);
};

#define SDRGLUEPOINT_NOTFOUND 0xFFFF

class SdrGluePointList {
    Container aList;
//    FASTBOOL bSorted; // Muss noch implementiert werden oder?
    // Eigentlich sollte die GluePointList stets sortiert sein (solange die
    // Id's ausreichen)
protected:
    SdrGluePoint* GetObject(USHORT i) const { return (SdrGluePoint*)(aList.GetObject(i)); }
public:
    SdrGluePointList(): aList(1024,4,4) {}
    SdrGluePointList(const SdrGluePointList& rSrcList): aList(1024,4,4)     { *this=rSrcList; }
    ~SdrGluePointList()                                                     { Clear(); }
    void                Clear();
    USHORT              GetCount() const                                    { return USHORT(aList.Count()); }
    // Beim Insert wird dem Objekt (also dem GluePoint) automatisch eine Id zugewiesen.
    // ReturnCode ist der Index des neuen GluePoints in der Liste
    USHORT              Insert(const SdrGluePoint& rGP);
    void                Delete(USHORT nPos)                                 { delete (SdrGluePoint*)aList.Remove(nPos); }
    SdrGluePoint&       operator[](USHORT nPos)                             { return *GetObject(nPos); }
    const SdrGluePoint& operator[](USHORT nPos) const                       { return *GetObject(nPos); }
    USHORT              FindGluePoint(USHORT nId) const;
    // Temporaer zu setzen fuer Transformationen am Bezugsobjekt
    friend SvStream& operator<<(SvStream& rOut, const SdrGluePointList& rGPL);
    friend SvStream& operator>>(SvStream& rIn, SdrGluePointList& rGPL);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDGLUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
