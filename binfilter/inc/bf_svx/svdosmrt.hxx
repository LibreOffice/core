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

#ifndef _SVDSMRT_HXX
#define _SVDSMRT_HXX

#include <bf_svx/xpoly.hxx>

#include <bf_svx/svdosmev.hxx>
class SvStream;
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrObject;
class SdrPathObj;
class ImpSdrExprTokenList;

enum SdrSmartValueFieldKind {SDRSMARTFIELD_NORMAL,
                             SDRSMARTFIELD_METRIC,
                             SDRSMARTFIELD_METRICX,
                             SDRSMARTFIELD_METRICY,
                             SDRSMARTFIELD_BOOL,
                             SDRSMARTFIELD_ANGLE,
                             SDRSMARTFIELD_GEOKIND};

class SdrSmartValueField: public SdrExprValue {
    ImpSdrExprTokenList*   pTokenList;
    ULONG                  nErrorTokPos;
    FASTBOOL               bCalculating;
    FASTBOOL               bCircRef;
    SdrExprErrKind         eError;
    SdrSmartValueFieldKind eKind;
private:
    virtual void Undirty(const SdrObject* pObj);
public:
    SdrSmartValueField(SdrSmartValueFieldKind eKind_);
    SdrSmartValueField(const SdrSmartValueField& rSource): pTokenList(NULL) { bIsField=TRUE; *this=rSource; }
    virtual ~SdrSmartValueField();
    void operator=(const SdrSmartValueField& rSource);
    SdrSmartValueFieldKind GetFieldKind() const             { return eKind; }
    FASTBOOL        IsMetric() const                          { return eKind==SDRSMARTFIELD_METRIC || eKind==SDRSMARTFIELD_METRICX || eKind==SDRSMARTFIELD_METRICY; }
    void            SetFormulaString(const XubString& rStr, const SdrObject* pObj);
    const XubString& GetFormulaString(const SdrObject* pObj) const;
    XubString        GetValueString(const SdrObject* pObj) const;
    void            SetUnitsAbsolute(long nUnits, const SdrObject* pObj);
    void            SetUnitsRelative(long nUnits, const SdrObject* pObj);
    void            MakeUnitsAbsolute(const SdrObject* pObj);
    void            MakeUnitsRelative(const SdrObject* pObj);
    FASTBOOL        IsError() const                           { return eError!=SDREXPRERR_NONE; }
    SdrExprErrKind  GetError() const                          { return eError; }
    XubString        GetErrorStr() const;
    ULONG           GetErrorTokPos() const                    { return nErrorTokPos; }
    USHORT          GetErrorCharPos() const;
    USHORT          GetErrorCharAnz() const;
    friend SvStream& operator<<(SvStream& rOut, const SdrSmartValueField& rSVF);
    friend SvStream& operator>>(SvStream& rIn, SdrSmartValueField& rSVF);
};

class SdrSmartPointField {
    SdrSmartValueField aX,aY;
public:
    SdrSmartPointField(): aX(SDRSMARTFIELD_METRICX), aY(SDRSMARTFIELD_METRICY) {}
    const SdrSmartValueField& X() const { return aX; }
    const SdrSmartValueField& Y() const { return aY; }
    SdrSmartValueField& X() { return aX; }
    SdrSmartValueField& Y() { return aY; }
    void SetAllFieldsDirty() { aX.SetDirty(); aY.SetDirty(); }
    ULONG GetMSecsUsed(const SdrObject* pObj) const;
    friend SvStream& operator<<(SvStream& rOut, const SdrSmartPointField& rSPF);
    friend SvStream& operator>>(SvStream& rIn, SdrSmartPointField& rSPF);
};

class SdrSmartMetricField: public SdrSmartValueField {
public:
    SdrSmartMetricField(): SdrSmartValueField(SDRSMARTFIELD_METRIC) {}
};

class SdrSmartBoolField: public SdrSmartValueField {
public:
    SdrSmartBoolField(): SdrSmartValueField(SDRSMARTFIELD_BOOL) {}
    FASTBOOL GetValue(const SdrObject* pObj) const { return GetLong(pObj)!=0; }
    FASTBOOL QuickGetValue() const                 { return QuickGetLong()!=0; }
};

class SdrSmartAngleField: public SdrSmartValueField {
public:
    SdrSmartAngleField(): SdrSmartValueField(SDRSMARTFIELD_ANGLE) {}
};

// Alle Koordinaten beziehen sich auf das unrotated Ref-Rect des Objekts
// Dieses wird dargestellt durch SdrObjSmartData::Pos,Size,Winkel. Positive
// Koordinaten gehen nach unten rechts. Der Winkel dagegen entgegen dem
// Uhrzeigersinn.

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum SdrSmartGeoKind {SDRSMARTGEO_NONE,
                      SDRSMARTGEO_START,
                      SDRSMARTGEO_LINETO,
                      SDRSMARTGEO_ARCTO,
                      SDRSMARTGEO_ELARCTO,
                      SDRSMARTGEO_BEZIERTO,
                      SDRSMARTGEO_SPLINETO,
                      SDRSMARTGEO_CLOSE};

class SdrSmartGeoKindField: public SdrSmartValueField {
public:
    SdrSmartGeoKindField(): SdrSmartValueField(SDRSMARTFIELD_GEOKIND) {}
    SdrSmartGeoKind GetValue(const SdrObject* pObj) const { return (SdrSmartGeoKind)GetLong(pObj); }
    SdrSmartGeoKind QuickGetValue() const                 { return (SdrSmartGeoKind)QuickGetLong(); }
    XubString GetString(SdrSmartGeoKind eKind) const;
};

class SdrSmartGeometricLine {
    SdrSmartPointField aPos;
    SdrSmartPointField aCtrl1,aCtrl2; // Fuer Bezier/ElArc
    SdrSmartGeoKindField aKind;
public:
    SdrSmartGeometricLine() {}
    SdrSmartGeometricLine(const SdrSmartGeoKindField& rKind): aKind(rKind) {}
    SdrSmartGeometricLine(const XubString& rKind, const SdrObject* pObj) { aKind.SetFormulaString(rKind,pObj); }
    SdrSmartGeometricLine(SdrSmartGeoKind eKind, const SdrObject* pObj) { aKind.SetFormulaString((USHORT)eKind,pObj); }
    SdrSmartGeometricLine(const SdrSmartGeoKindField& rKind, const SdrSmartPointField& rPos): aKind(rKind), aPos(rPos) {}
    SdrSmartGeometricLine(const XubString& rKind, const XubString& rX, const XubString& rY, const SdrObject* pObj) { aKind.SetFormulaString(rKind,pObj); aPos.X().SetFormulaString(rX,pObj); aPos.Y().SetFormulaString(rY,pObj); }
    SdrSmartGeometricLine(SdrSmartGeoKind eKind, const XubString& rX, const XubString& rY, const SdrObject* pObj) { aKind.SetFormulaString((USHORT)eKind,pObj); aPos.X().SetFormulaString(rX,pObj); aPos.Y().SetFormulaString(rY,pObj); }
    void AddPolygon(XPolygon& rXP, const SdrObject* pObj) const;
    void SetAllFieldsDirty();
    const SdrSmartGeoKindField& GetKind() const { return aKind; }
    SdrSmartGeoKindField&       Kind()         { return aKind; }
    FASTBOOL HasPos(const SdrObject* pObj) const;
    FASTBOOL HasCtrl(const SdrObject* pObj) const;
    const SdrSmartPointField& GetPos() const   { return aPos; }
    const SdrSmartPointField& GetCtrl1() const { return aCtrl1; }
    const SdrSmartPointField& GetCtrl2() const { return aCtrl2; }
    SdrSmartPointField& Pos()                  { return aPos; }
    SdrSmartPointField& Ctrl1()                { return aCtrl1; }
    SdrSmartPointField& Ctrl2()                { return aCtrl2; }
    ULONG GetMSecsUsed(const SdrObject* pObj) const;
    const SdrSmartValueField* GetField(long nColumn) const;
    SdrSmartValueField* GetField(long nColumn) { return (SdrSmartValueField*)((const SdrSmartGeometricLine*)this)->GetField(nColumn); }
    friend SvStream& operator<<(SvStream& rOut, const SdrSmartGeometricLine& rSGL);
    friend SvStream& operator>>(SvStream& rIn, SdrSmartGeometricLine& rSGL);
};

class SdrSmartGeometric {
    Container aList;
    SdrSmartBoolField aLineEnd1;
    SdrSmartBoolField aLineEnd2;
    SdrSmartBoolField aDisabled;
    SdrSmartBoolField aNoFill;
protected:
    SdrSmartGeometricLine* GetObject(ULONG i) const { return (SdrSmartGeometricLine*)(aList.GetObject(i)); }
public:
    SdrSmartGeometric(): aList(1024,4,4) {}
    SdrSmartGeometric(const SdrSmartGeometric& rSource): aList(1024,4,4)     { *this=rSource; }
    ~SdrSmartGeometric()                                                     { Clear(); }
    void                Clear();
    void                operator=(const SdrSmartGeometric& rSource);
    ULONG               GetCount() const                                { return aList.Count(); }
    void                Insert(const SdrSmartGeometricLine& rSGL, ULONG nPos=CONTAINER_APPEND) { aList.Insert(new SdrSmartGeometricLine(rSGL),nPos); }
    void                Delete(ULONG nPos)                              { delete (SdrSmartGeometricLine*)aList.Remove(nPos); }
    SdrSmartGeometricLine&       operator[](ULONG nPos)                 { return *GetObject(nPos); }
    const SdrSmartGeometricLine& operator[](ULONG nPos) const           { return *GetObject(nPos); }
    const SdrSmartBoolField& GetLineEnd1() const                        { return aLineEnd1; }
    const SdrSmartBoolField& GetLineEnd2() const                        { return aLineEnd2; }
    const SdrSmartBoolField& GetDisabled() const                        { return aDisabled; }
    const SdrSmartBoolField& GetNoFill() const                          { return aNoFill; }
    SdrSmartBoolField&       LineEnd1()                                 { return aLineEnd1; }
    SdrSmartBoolField&       LineEnd2()                                 { return aLineEnd2; }
    SdrSmartBoolField&       Disabled()                                 { return aDisabled; }
    SdrSmartBoolField&       NoFill()                                   { return aNoFill; }
    XPolygon                 GetPolygon(const SdrObject* pObj) const;
    SdrSmartGeometricLine*   GetGeometricLine(const SdrObject* pObj, USHORT nPnt) const;
    void SetAllFieldsDirty();
    ULONG GetMSecsUsed(const SdrObject* pObj) const;
    const SdrSmartValueField* GetField(long nColumn) const;
    SdrSmartValueField* GetField(long nColumn) { return (SdrSmartValueField*)((const SdrSmartGeometric*)this)->GetField(nColumn); }
    friend SvStream& operator<<(SvStream& rOut, const SdrSmartGeometric& rSGeo);
    friend SvStream& operator>>(SvStream& rIn, SdrSmartGeometric& rSGeo);
};

class SdrSmartGeometricList {
    Container aList;
protected:
    SdrSmartGeometric* GetObject(ULONG i) const { return (SdrSmartGeometric*)(aList.GetObject(i)); }
public:
    SdrSmartGeometricList(): aList(1024,4,4) {}
    SdrSmartGeometricList(const SdrSmartGeometricList& rSrcList): aList(1024,4,4) { *this=rSrcList; }
    ~SdrSmartGeometricList()                                                      { Clear(); }
    void                Clear();
    void                operator=(const SdrSmartGeometricList& rSrcList);
    ULONG               GetCount() const                                { return aList.Count(); }
    void                Insert(const SdrSmartGeometric& rSGeo, ULONG nPos=CONTAINER_APPEND) { aList.Insert(new SdrSmartGeometric(rSGeo),nPos); }
    void                Delete(ULONG nPos)                              { delete (SdrSmartGeometric*)aList.Remove(nPos); }
    SdrSmartGeometric&       operator[](ULONG nPos)                     { return *GetObject(nPos); }
    const SdrSmartGeometric& operator[](ULONG nPos) const               { return *GetObject(nPos); }
    XPolyPolygon           GetPolygon(const SdrObject* pObj) const;
    SdrSmartGeometricLine* GetGeometricLine(const SdrObject* pObj, USHORT nPol, USHORT nPnt) const;
    void SetAllFieldsDirty();
    ULONG GetMSecsUsed(const SdrObject* pObj) const;
    friend SvStream& operator<<(SvStream& rOut, const SdrSmartGeometricList& rSGL);
    friend SvStream& operator>>(SvStream& rIn, SdrSmartGeometricList& rSGL);
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class SdrSmartDraftLine {
    SdrSmartPointField aPos;
    SdrSmartMetricField aA,aB,aC,aD;
public:
    const SdrSmartPointField& GetPos() const   { return aPos; }
    const SdrSmartMetricField& GetA() const    { return aA; }
    const SdrSmartMetricField& GetB() const    { return aB; }
    const SdrSmartMetricField& GetC() const    { return aC; }
    const SdrSmartMetricField& GetD() const    { return aD; }
    SdrSmartPointField& Pos()                  { return aPos; }
    SdrSmartMetricField& A()                   { return aA; }
    SdrSmartMetricField& B()                   { return aB; }
    SdrSmartMetricField& C()                   { return aC; }
    SdrSmartMetricField& D()                   { return aD; }
    void SetAllFieldsDirty();
    ULONG GetMSecsUsed(const SdrObject* pObj) const;
    const SdrSmartValueField* GetField(long nColumn) const;
    SdrSmartValueField* GetField(long nColumn) { return (SdrSmartValueField*)((const SdrSmartDraftLine*)this)->GetField(nColumn); }
    friend SvStream& operator<<(SvStream& rOut, const SdrSmartDraftLine& rSDL);
    friend SvStream& operator>>(SvStream& rIn, SdrSmartDraftLine& rSDL);
};

class SdrSmartDraftLineList {
    Container aList;
protected:
    SdrSmartDraftLine* GetObject(ULONG i) const { return (SdrSmartDraftLine*)(aList.GetObject(i)); }
public:
    SdrSmartDraftLineList(): aList(1024,4,4) {}
    SdrSmartDraftLineList(const SdrSmartDraftLineList& rSrcList): aList(1024,4,4) { *this=rSrcList; }
    ~SdrSmartDraftLineList()                                                      { Clear(); }
    void                Clear();
    void                operator=(const SdrSmartDraftLineList& rSrcList);
    ULONG               GetCount() const                                { return aList.Count(); }
    void                Insert(const SdrSmartDraftLine& rSDL, ULONG nPos=CONTAINER_APPEND) { aList.Insert(new SdrSmartDraftLine(rSDL),nPos); }
    void                Delete(ULONG nPos)                              { delete (SdrSmartDraftLine*)aList.Remove(nPos); }
    SdrSmartDraftLine&       operator[](ULONG nPos)                     { return *GetObject(nPos); }
    const SdrSmartDraftLine& operator[](ULONG nPos) const               { return *GetObject(nPos); }
    void SetAllFieldsDirty();
    ULONG GetMSecsUsed(const SdrObject* pObj) const;
    friend SvStream& operator<<(SvStream& rOut, const SdrSmartDraftLineList& rSDLL);
    friend SvStream& operator>>(SvStream& rIn, SdrSmartDraftLineList& rSDLL);
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class SdrSmartGluePoint {
    SdrSmartPointField aPos;
    SdrSmartValueField aEsc;
    SdrSmartBoolField  aDisabled;
    USHORT             nId;
public:
    SdrSmartGluePoint(): aEsc(SDRSMARTFIELD_NORMAL),nId(0) {}
    void SetAllFieldsDirty();
    ULONG GetMSecsUsed(const SdrObject* pObj) const;
    const SdrSmartValueField* GetField(long nColumn) const;
    SdrSmartValueField* GetField(long nColumn) { return (SdrSmartValueField*)((const SdrSmartGluePoint*)this)->GetField(nColumn); }
    friend SvStream& operator<<(SvStream& rOut, const SdrSmartGluePoint& rSGP);
    friend SvStream& operator>>(SvStream& rIn, SdrSmartGluePoint& rSGP);
};

class SdrSmartGluePointList {
    Container aList;
protected:
    SdrSmartGluePoint* GetObject(ULONG i) const { return (SdrSmartGluePoint*)(aList.GetObject(i)); }
public:
    SdrSmartGluePointList(): aList(1024,4,4) {}
    SdrSmartGluePointList(const SdrSmartGluePointList& rSrcList): aList(1024,4,4) { *this=rSrcList; }
    ~SdrSmartGluePointList()                                                      { Clear(); }
    void                Clear();
    void                operator=(const SdrSmartGluePointList& rSrcList);
    ULONG               GetCount() const                                { return aList.Count(); }
    void                Insert(const SdrSmartGluePoint& rSGP, ULONG nPos=CONTAINER_APPEND) { aList.Insert(new SdrSmartGluePoint(rSGP),nPos); }
    void                Delete(ULONG nPos)                              { delete (SdrSmartGluePoint*)aList.Remove(nPos); }
    SdrSmartGluePoint&       operator[](ULONG nPos)                     { return *GetObject(nPos); }
    const SdrSmartGluePoint& operator[](ULONG nPos) const               { return *GetObject(nPos); }
    void SetAllFieldsDirty();
    ULONG GetMSecsUsed(const SdrObject* pObj) const;
    friend SvStream& operator<<(SvStream& rOut, const SdrSmartGluePointList& rSGPL);
    friend SvStream& operator>>(SvStream& rIn, SdrSmartGluePointList& rSGPL);
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class SdrSmartDragPoint {
    SdrSmartPointField aRef;
    SdrSmartAngleField aRefAngle;
    SdrSmartPointField aPos;
    SdrSmartPointField aMin; // Bezogen auf aRef
    SdrSmartPointField nMax; // Bezogen auf aRef
    SdrSmartMetricField aMinLen,aMaxLen;
    SdrSmartAngleField aMinAngle,aMaxAngle;
    SdrSmartMetricField aFixedLen;
    SdrSmartAngleField aFixedAngle;
    // nSnapXY wenn !SnapAngle, ""="View.Fang"
    SdrSmartPointField aSnap;      // Bezogen auf aRef
    // nSnapAngle wenn !SnapXY, ""="View.Winkelfang"
    SdrSmartAngleField aSnapAngle; // Bezogen auf aRefAngle. Hiermit auch Ortho4 und Ortho8 moeglich!
    SdrSmartBoolField  aDisabled;
public:
    void SetAllFieldsDirty();
    ULONG GetMSecsUsed(const SdrObject* pObj) const;
    const SdrSmartValueField* GetField(long nColumn) const;
    SdrSmartValueField* GetField(long nColumn) { return (SdrSmartValueField*)((const SdrSmartDragPoint*)this)->GetField(nColumn); }
    friend SvStream& operator<<(SvStream& rOut, const SdrSmartDragPoint& rSDP);
    friend SvStream& operator>>(SvStream& rIn, SdrSmartDragPoint& rSDP);
};

class SdrSmartDragPointList {
    Container aList;
protected:
    SdrSmartDragPoint* GetObject(ULONG i) const { return (SdrSmartDragPoint*)(aList.GetObject(i)); }
public:
    SdrSmartDragPointList(): aList(1024,4,4) {}
    SdrSmartDragPointList(const SdrSmartDragPointList& rSrcList): aList(1024,4,4) { *this=rSrcList; }
    ~SdrSmartDragPointList()                                                      { Clear(); }
    void                Clear();
    void                operator=(const SdrSmartDragPointList& rSrcList);
    ULONG               GetCount() const                                { return aList.Count(); }
    void                Insert(const SdrSmartDragPoint& rSDP, ULONG nPos=CONTAINER_APPEND) { aList.Insert(new SdrSmartDragPoint(rSDP),nPos); }
    void                Delete(ULONG nPos)                              { delete (SdrSmartDragPoint*)aList.Remove(nPos); }
    void                SetAllFieldsDirty();
    ULONG GetMSecsUsed(const SdrObject* pObj) const;
    SdrSmartDragPoint&       operator[](ULONG nPos)                     { return *GetObject(nPos); }
    const SdrSmartDragPoint& operator[](ULONG nPos) const               { return *GetObject(nPos); }
    friend SvStream& operator<<(SvStream& rOut, const SdrSmartDragPointList& rSDPL);
    friend SvStream& operator>>(SvStream& rIn, SdrSmartDragPointList& rSDPL);
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class SdrObjSmartData {
public:
    // Pos,Size,Winkel
    SdrSmartPointField aStartPt;
    SdrSmartPointField aEndPt;
    SdrSmartPointField aPos;
    SdrSmartPointField aSize;
    SdrSmartAngleField aAngle;
    SdrSmartBoolField  aHMirror;
    SdrSmartBoolField  aVMirror;
    // Geometrie(n)
    SdrSmartGeometricList aGeometrics;
    // Testflaeche
    SdrSmartDraftLineList aDraft;
    // Variablen fuer Iteration
    SdrSmartDraftLineList aVars;
    // Klebepunkte
    SdrSmartGluePointList aGluePoints;
    // Steuerziehpunkte
    SdrSmartDragPointList aDragPoints;
    FASTBOOL bAnimated;
    ULONG    nTimerStep;
//    AutoTimer aAnimator;
//    SdrPathObj* pPathObj;
//private:
//    DECL_LINK(AniHdl,AutoTimer*);
public:
    SdrObjSmartData();
    ~SdrObjSmartData();
    void SetAllFieldsDirty();
    ULONG GetMSecsUsed(const SdrObject* pObj) const;
    void SetPathObj(SdrPathObj* pObj);
    XPolyPolygon GetPolygon(const SdrObject* pObj) const;
    SdrSmartGeometricLine* GetGeometricLine(const SdrObject* pObj, USHORT nPol, USHORT nPnt) const;
    friend SvStream& operator<<(SvStream& rOut, const SdrObjSmartData& rSData);
    friend SvStream& operator>>(SvStream& rIn, SdrObjSmartData& rSData);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDSMRT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
