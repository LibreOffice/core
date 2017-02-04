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

#ifndef INCLUDED_SVX_SVDOEDGE_HXX
#define INCLUDED_SVX_SVDOEDGE_HXX

#include <svx/svdotext.hxx>
#include <svx/svdglue.hxx>
#include <svx/svxdllapi.h>


class SdrDragMethod;
class SdrPageView;

namespace sdr { namespace properties {
    class ConnectorProperties;
}}


/// Utility class SdrObjConnection
class SdrObjConnection
{
    friend class                SdrEdgeObj;
    friend class                ImpEdgeHdl;
    friend class                SdrCreateView;

protected:
    Point                       aObjOfs;       // set during dragging of a node
    SdrObject*                  pObj;          // referenced object
    sal_uInt16                  nConId;        // connector number

    bool                        bBestConn : 1;   // true -> the best-matching connector is searched for
    bool                        bBestVertex : 1; // true -> the best-matching vertex to connect is searched for
    bool                        bAutoVertex : 1; // autoConnector at apex nCon
    bool                        bAutoCorner : 1; // autoConnector at corner nCon

public:
    SdrObjConnection() { ResetVars(); }
    SVX_DLLPUBLIC ~SdrObjConnection();

    void ResetVars();
    bool TakeGluePoint(SdrGluePoint& rGP) const;

    inline void SetBestConnection( bool rB ) { bBestConn = rB; };
    inline void SetBestVertex( bool rB ) { bBestVertex = rB; };
    inline void SetAutoVertex( bool rB ) { bAutoVertex = rB; };
    inline void SetConnectorId( sal_uInt16 nId ) { nConId = nId; };

    inline bool IsBestConnection() const { return bBestConn; };
    inline bool IsAutoVertex() const { return bAutoVertex; };
    inline sal_uInt16 GetConnectorId() const { return nConId; };
    inline SdrObject* GetObject() const { return pObj; }
};


enum class SdrEdgeLineCode { Obj1Line2, Obj1Line3, Obj2Line2, Obj2Line3, MiddleLine };

/// Utility class SdrEdgeInfoRec
class SdrEdgeInfoRec
{
public:
    // The 5 distances are set on dragging or via SetAttr and are
    // evaluated by ImpCalcEdgeTrack. Only 0-3 longs are transpeorted
    // via Get/SetAttr/Get/SetStyleSh though.
    Point                       aObj1Line2;
    Point                       aObj1Line3;
    Point                       aObj2Line2;
    Point                       aObj2Line3;
    Point                       aMiddleLine;

    // Following values are set by ImpCalcEdgeTrack
    long                        nAngle1;           // exit angle at Obj1
    long                        nAngle2;           // exit angle at Obj2
    sal_uInt16                  nObj1Lines;        // 1..3
    sal_uInt16                  nObj2Lines;        // 1..3
    sal_uInt16                  nMiddleLine;       // 0xFFFF=none, otherwiese point number of the beginning of the line
    char                        cOrthoForm;        // form of the right-angled connector, e.g., 'Z','U',I','L','S',...

public:
    SdrEdgeInfoRec()
    :   nAngle1(0),
        nAngle2(0),
        nObj1Lines(0),
        nObj2Lines(0),
        nMiddleLine(0xFFFF),
        cOrthoForm(0)
    {}

    Point& ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode);
    sal_uInt16 ImpGetPolyIdx(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const;
    bool ImpIsHorzLine(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const;
    void ImpSetLineVersatz(SdrEdgeLineCode eLineCode, const XPolygon& rXP, long nVal);
    long ImpGetLineVersatz(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const;
};


/// Utility class SdrEdgeObjGeoData
class SdrEdgeObjGeoData : public SdrTextObjGeoData
{
public:
    SdrObjConnection            aCon1;  // connection status of the beginning of the line
    SdrObjConnection            aCon2;  // connection status of the end of the line
    std::unique_ptr<XPolygon>   pEdgeTrack;
    bool                        bEdgeTrackDirty; // true -> connector track needs to be recalculated
    bool                        bEdgeTrackUserDefined;
    SdrEdgeInfoRec              aEdgeInfo;

public:
    SdrEdgeObjGeoData();
    virtual ~SdrEdgeObjGeoData() override;
};


/// Utility class SdrEdgeObj
class SVX_DLLPUBLIC SdrEdgeObj : public SdrTextObj
{
private:
    // to allow sdr::properties::ConnectorProperties access to ImpSetAttrToEdgeInfo()
    friend class sdr::properties::ConnectorProperties;

    friend class                SdrCreateView;
    friend class                ImpEdgeHdl;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() override;

    SdrObjConnection            aCon1;  // Connection status of the beginning of the line
    SdrObjConnection            aCon2;  // Connection status of the end of the line

    XPolygon*                   pEdgeTrack;
    sal_uInt16                  nNotifyingCount; // Locking
    SdrEdgeInfoRec              aEdgeInfo;

    bool                        bEdgeTrackDirty : 1; // true -> Connection track needs to be recalculated
    bool                        bEdgeTrackUserDefined : 1;

    // Bool to allow suppression of default connects at object
    // inside test (HitTest) and object center test (see ImpFindConnector())
    bool                        mbSuppressDefaultConnect : 1;

    // Flag value for avoiding infinite loops when calculating
    // BoundRects from ring-connected connectors. A coloring algorithm
    // is used here. When the GetCurrentBoundRect() calculation of a
    // SdrEdgeObj is running, the flag is set, else it is always
    // false.
    bool                        mbBoundRectCalculationRunning : 1;

    // #i123048# need to remember if layouting was suppressed before to get
    // to a correct state for first real layouting
    bool                        mbSuppressed : 1;

public:
    // Interface to default connect suppression
    void SetSuppressDefaultConnect(bool bNew) { mbSuppressDefaultConnect = bNew; }
    bool GetSuppressDefaultConnect() const { return mbSuppressDefaultConnect; }

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    static XPolygon ImpCalcObjToCenter(const Point& rStPt, long nEscAngle, const Rectangle& rRect, const Point& rCenter);
    void ImpRecalcEdgeTrack();   // recalculation of the connection track
    XPolygon ImpCalcEdgeTrack(const XPolygon& rTrack0, SdrObjConnection& rCon1, SdrObjConnection& rCon2, SdrEdgeInfoRec* pInfo) const;
    XPolygon ImpCalcEdgeTrack(const Point& rPt1, long nAngle1, const Rectangle& rBoundRect1, const Rectangle& rBewareRect1,
        const Point& rPt2, long nAngle2, const Rectangle& rBoundRect2, const Rectangle& rBewareRect2,
        sal_uIntPtr* pnQuality, SdrEdgeInfoRec* pInfo) const;
    static bool ImpFindConnector(const Point& rPt, const SdrPageView& rPV, SdrObjConnection& rCon, const SdrEdgeObj* pThis, OutputDevice* pOut=nullptr);
    static SdrEscapeDirection ImpCalcEscAngle(SdrObject* pObj, const Point& aPt2);
    void ImpSetTailPoint(bool bTail1, const Point& rPt);
    void ImpUndirtyEdgeTrack();  // potential recalculation of the connection track
    void ImpDirtyEdgeTrack();    // invalidate connector path, so it will be recalculated next time
    void ImpSetAttrToEdgeInfo(); // copying values from the pool to aEdgeInfo
    void ImpSetEdgeInfoToAttr(); // copying values from the aEdgeInfo to the pool

public:

    SdrEdgeObj();
    virtual ~SdrEdgeObj() override;

    SdrObjConnection& GetConnection(bool bTail1) { return *(bTail1 ? &aCon1 : &aCon2); }
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual const Rectangle& GetCurrentBoundRect() const override;
    virtual const Rectangle& GetSnapRect() const override;
    virtual bool IsNode() const override;
    virtual SdrGluePoint GetVertexGluePoint(sal_uInt16 nNum) const override;
    virtual SdrGluePoint GetCornerGluePoint(sal_uInt16 nNum) const override;
    virtual const SdrGluePointList* GetGluePointList() const override;
    virtual SdrGluePointList* ForceGluePointList() override;

    // * for all of the below: bTail1=true: beginning of the line,
    //   otherwise end of the line
    // * pObj=NULL: disconnect connector
    void SetEdgeTrackDirty() { bEdgeTrackDirty=true; }
    void ConnectToNode(bool bTail1, SdrObject* pObj) override;
    void DisconnectFromNode(bool bTail1) override;
    SdrObject* GetConnectedNode(bool bTail1) const override;
    const SdrObjConnection& GetConnection(bool bTail1) const { return *(bTail1 ? &aCon1 : &aCon2); }
    bool CheckNodeConnection(bool bTail1) const;

    virtual void RecalcSnapRect() override;
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const override;
    virtual SdrEdgeObj* Clone() const override;
    SdrEdgeObj& operator=(const SdrEdgeObj& rObj);
    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    void    SetEdgeTrackPath( const basegfx::B2DPolyPolygon& rPoly );
    basegfx::B2DPolyPolygon GetEdgeTrackPath() const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual sal_uInt32 GetHdlCount() const override;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const override;

    // special drag methods
    virtual bool hasSpecialDrag() const override;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const override;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) override;
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const override;

    // FullDrag support
    virtual SdrObject* getFullDragClone() const override;

    virtual void NbcSetSnapRect(const Rectangle& rRect) override;
    virtual void NbcMove(const Size& aSize) override;
    virtual void NbcResize(const Point& rRefPnt, const Fraction& aXFact, const Fraction& aYFact) override;

    // #i54102# added rotate, mirror and shear support
    virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs) override;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
    virtual void NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear) override;

    // #102344# Added missing implementation
    virtual void NbcSetAnchorPos(const Point& rPnt) override;

    virtual bool BegCreate(SdrDragStat& rStat) override;
    virtual bool MovCreate(SdrDragStat& rStat) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;
    virtual bool BckCreate(SdrDragStat& rStat) override;
    virtual void BrkCreate(SdrDragStat& rStat) override;
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const override;
    virtual Pointer GetCreatePointer() const override;
    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    virtual sal_uInt32 GetSnapPointCount() const override;
    virtual Point GetSnapPoint(sal_uInt32 i) const override;
    virtual bool IsPolyObj() const override;
    virtual sal_uInt32 GetPointCount() const override;
    virtual Point GetPoint(sal_uInt32 i) const override;
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i) override;

    virtual SdrObjGeoData* NewGeoData() const override;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) override;

    /** updates edges that are connected to the edges of this object
        as if the connected objects send a repaint broadcast
        #103122#
    */
    void Reformat();

    // helper methods for the StarOffice api
    Point GetTailPoint( bool bTail ) const;
    void SetTailPoint( bool bTail, const Point& rPt );
    void setGluePointIndex( bool bTail, sal_Int32 nId = -1 );
    sal_Int32 getGluePointIndex( bool bTail );

    virtual bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const override;
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon) override;

    // for geometry access
    ::basegfx::B2DPolygon getEdgeTrack() const;

    // helper method for SdrDragMethod::AddConnectorOverlays. Adds a overlay polygon for
    // this connector to rResult.
    basegfx::B2DPolygon ImplAddConnectorOverlay(SdrDragMethod& rDragMethod, bool bTail1, bool bTail2, bool bDetail) const;
};

 // The following item parameters of the SdrItemPool are used to
 // determine the actual connector line routing:
 //
 //  sal_uInt16 EdgeFlowAngle       default 9000 (= 90.00 deg), min 0, max 9000
 //      Clearance angle.
 //      The angle at which the connecting line may run.
 //
 //  sal_uInt16 EdgeEscAngle        default 9000 (= 90.00 Deg), min 0, max 9000
 //      Object exit angle.
 //      The angle at which the connection line may exit from the object.
 //
 //  bool EdgeEscAsRay              default false
 //      true -> the connecting line emerges from the object radially.
 //      Thus, angle specification by the line ObjCenter / connector.
 //
 //  bool EdgeEscUseObjAngle        default false
 //      Object rotation angle is considered
 //      true -> when determining the connector exit angle, angle for
 //      object rotation is taken as an offset.
 //
 //  sal_uIntPtr EdgeFlowDefDist    default 0, min 0, max ?
 //      This is the default minimum distance on calculation of the
 //      connection Line to the docked objects is in logical units.
 //      This distance is overridden within the object, as soon as the
 //      user drags on the lines. When docking onto a new object,
 //      however, this default is used again.
 //
 //
 // General Information About Connectors:
 //
 // There are nodes and edge objects. Two nodes can be joined by an
 // edge. If a connector is connected to a node only at one end, the
 // other end is fixed to an absolute position in the document. It is
 // of course also possible for a connector to be "free" at both ends,
 // i.e. not connected to a node object on each side.
 //
 // A connector object can also theoretically be a node object at the
 // same time. In the first version, however, this will not yet be
 // realized.
 //
 // A connection between node and connector edge can be established by:
 // - Interactive creation of a new edge object at the SdrView where
 //   the beginning or end point of the edge is placed on a connector
 //   (glueing point) of an already existing node object.
 // - Interactive dragging of the beginning or end point of an
 //   existing connector edge object on the SdrView to a connector
 //   (glueing point) of an already existing node object.
 // - Undo/Redo
 //   Moving node objects does not make any connections. Also not the
 //   direct shifting of edge endpoints on the SdrModel... Connections
 //   can also be established, if the connectors are not configured to
 //   be visible in the view.
 //
 // An existing connection between node and edge is retained for:
 // - Dragging (Move/Resize/Rotate/...) of the node object
 // - Moving a connector position in the node object
 // - Simultaneous dragging (Move/Resize/Rotate/...) of the node and the
 //   edge
 //
 // A connection between node and edge can be removed by:
 // - Deleting one of the objects
 // - Dragging the edge object without simultaneously dragging the node
 // - Deleting the connector at the node object
 // - Undo/Redo/Repeat
 // When dragging, the request to remove the connection must be
 // requested from outside of the model (for example, from the
 // SdrView). SdrEdgeObj::Move() itself does not remove the
 // connection.
 //
 // Each node object can have connectors, so-called glue points. These
 // are the geometric points at which the connecting edge object ends
 // when the connection is established. By default, each object has no
 // connectors.  Nevertheless, one can dock an edge in certain view
 // settings since then, e.g., connectors can be automatically
 // generated at the 4 vertices of the node object when needed. Each
 // object provides 2x4 so-called default connector positions, 4 at
 // the vertices and 4 at the corner positions. In the normal case,
 // these are located at the 8 handle positions; exceptions here are
 // ellipses, parallelograms, ... .  In addition, user-specific
 // connectors can be set for each node object.
 //
 // Then there is also the possibility to dock an edge on an object
 // with the attribute "bUseBestConnector". The best-matching
 // connector position for the routing of the connection line is then
 // used from the offering of connectors of the object or/and of the
 // vertices. The user assigns this attribute by docking the node in
 // its center (see, e.g., Visio).
 // 09-06-1996: bUseBestConnector uses vertex glue points only.
 //
 // And here is some terminology:
 //   Connector : The connector object (edge object)
 //   Node      : Any object to which a connector can be glued to, e.g., a rectangle,
 //               etc.
 //   Glue point: The point at which the connector is glued to the node object.
 //               There are:
 //                 Vertex glue points: Each node object presents these glue
 //                     points inherently. Perhaps there is already the option
 //                     "automatically glue to object vertex" in Draw (default is
 //                     on).
 //                 Corner glue points: These glue points, too, are already
 //                     auto-enabled on objects. Similar to the ones above,
 //                     there may already be an option for them in Draw (default is
 //                     off).
 //                 In contrast to Visio, vertex glue points and corner glue
 //                     points are not displayed in the UI; they are simply there (if
 //                     the option is activated).
 //                 Custom glue points: Any number of them are present on each
 //                     node object. They can be made visible using the option
 //                     (always visible when editing). At the moment, however, they
 //                     are not yet fully implemented.
 //                 Automatic glue point selection: If the connector is docked
 //                     to the node object so that the black frame encompasses the
 //                     entire object, then the connector tries to find the most
 //                     convenient of the 4 vertex glue points (and only of those).

#endif // INCLUDED_SVX_SVDOEDGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
