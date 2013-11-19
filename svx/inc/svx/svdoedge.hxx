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

#ifndef _SVDOEDGE_HXX
#define _SVDOEDGE_HXX

#include <svx/svdotext.hxx>
#include <svx/svxdllapi.h>
#include <basegfx/polygon/b2dpolygon.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

namespace sdr { namespace properties { class ConnectorProperties; }}
class SdrObjConnection;
class SdrEdgeInfoRec;

enum SdrEdgeLineCode
{
    OBJ1LINE2,
    OBJ1LINE3,
    OBJ2LINE2,
    OBJ2LINE3,
    MIDDLELINE
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrEdgeObj

class SVX_DLLPUBLIC SdrEdgeObj : public SdrTextObj
{
private:
    // to allow sdr::properties::ConnectorProperties access to ImpSetAttrToEdgeInfo()
    friend class sdr::properties::ConnectorProperties;
    friend class SdrCreateView;
    friend class SdrObjConnection;

    // geometric definition; one for start and one for end
    SdrObjConnection*           mpCon1;
    SdrObjConnection*           mpCon2;

    // created geometry; dependent from EdgeTrackUserDefined it can be
    // recreated on the fly or *is* the geometry already
    basegfx::B2DPolygon         maEdgeTrack;

    // additionally definitions for shaping; these are buffered ItemContent,
    // transfer is done in ImpSetAttrToEdgeInfo and ImpSetEdgeInfoToAttr
    SdrEdgeInfoRec*             mpEdgeInfo;

    /// bitfield
    /// defines if this edge was set from external and thus is defined by
    /// user; this mode is used for compatibility with other file formats.
    /// This state is not editable; any change on geometry data will reset
    /// this flag
    bool                        mbEdgeTrackUserDefined : 1;

    // Bool to allow supporession of default connects at object
    // inside test (HitTest) and object center test (see FindConnector())
    bool                        mbSuppressDefaultConnect : 1;

    // Flag value for avoiding death loops when calculating BoundRects
    // from circularly connected connectors. A coloring algorythm is used
    // here. When the GetCurrentBoundRect() calculation of a SdrEdgeObj
    // is running, the flag is set, else it is always sal_False.
    unsigned                    mbBoundRectCalculationRunning : 1;

    // need to remember if layouting was suppressed before to get
    // to a correct state for first real layouting
    unsigned                    mbSuppressed : 1;

    /// internal post processing when one of the positions the geometry
    /// is based on changes; this is only to be called from the object
    /// connectons or internally. It will correct the local transformation
    /// and do needed refreshes
    void geometryChange();

    /// helper to recalculate the EdgeTrack based on given information
    basegfx::B2DPolygon ImpCalcEdgeTrack(
        SdrObjConnection& rCon1,
        SdrObjConnection& rCon2,
        SdrEdgeInfoRec* pInfo,
        const basegfx::B2DHomMatrix* pTransA,
        const basegfx::B2DHomMatrix* pTransB) const;

    // recalculate EdgeTrack and put to maEdgeTrack
    void ImpRecalcEdgeTrack();

    // push/pull of data between ItemSet representation and SdrEdgeInfoRec
    void ImpSetAttrToEdgeInfo();
    void ImpSetEdgeInfoToAttr();

protected:
    // object specific handlers
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::glue::GluePointProvider* CreateObjectSpecificGluePointProvider();

    virtual ~SdrEdgeObj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    // check if rCandidate is a listener (in one of the SdrObjConnections) and
    // if yes, return the owner of the SdrObjConnection and thus the SdrEdgeObj
    // which is connected to the given listener (which probably is a SdrObject)
    static SdrEdgeObj* checkIfUsesListener(SfxListener& rCandidate);

    // #109007# Interface to default connect suppression
    void SetSuppressDefaultConnect(sal_Bool bNew) { mbSuppressDefaultConnect = bNew; }
    sal_Bool GetSuppressDefaultConnect() const { return mbSuppressDefaultConnect; }

    // #110649#
    sal_Bool IsBoundRectCalculationRunning() const { return mbBoundRectCalculationRunning; }

    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    // react on model/page change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage);

    virtual bool IsClosedObj() const;

    SdrEdgeObj(SdrModel& rSdrModel);

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;

    void SetEdgeTrackDirty();

    // bTail = true: connector start, else connector end
    // no SdrObject given: disconnect
    void ConnectToSdrObject(bool bTail, SdrObject* pObj = 0);
    SdrObject* GetSdrObjectConnection(bool bTail) const;
    bool CheckSdrObjectConnection(bool bTail) const;

    /// interface to hold a user-defined EdgeTrack (see mbEdgeTrackUserDefined)
    void SetEdgeTrackPath( const basegfx::B2DPolygon& rPoly );
    basegfx::B2DPolygon GetEdgeTrackPath() const;

    /// set/get connector ID (if needed)
    void SetConnectorId(bool bTail, sal_uInt32 nId);
    sal_uInt32 GetConnectorId(bool bTail) const;

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    sal_uInt32 impOldGetHdlCount() const;
    SdrHdl* impOldGetHdl(SdrHdlList& rHdlList, sal_uInt32 nHdlNum) const;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const;
    bool checkHorizontalDrag(SdrEdgeLineCode eCode, bool bObjHdlTwo) const;

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;

    // FullDrag support
    virtual SdrObject* getFullDragClone() const;

    // creation support
    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;

    // convert to polygon support
    virtual SdrObject* DoConvertToPolygonObject(bool bBezier, bool bAddText) const;

    // snap point support
    virtual sal_uInt32 GetSnapPointCount() const;
    virtual basegfx::B2DPoint GetSnapPoint(sal_uInt32 i) const;

    virtual bool IsPolygonObject() const;

    // GeoData (Undo/Redo) support
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);

    // helper methods for the StarOffice api
    basegfx::B2DPoint GetTailPoint( bool bTail ) const;
    void SetTailPoint( bool bTail, const basegfx::B2DPoint& rPt );
    void setGluePointIndex( bool bTail, sal_Int32 nId = -1 );
    sal_Int32 getGluePointIndex( bool bTail );

    // helper method for AddConnectorOverlays. Creates the needed overlay
    // polygon for this connector based on internal data
    basegfx::B2DPolygon CreateConnectorOverlay(
        const basegfx::B2DHomMatrix& rCurrentTransformation,
        bool bTail1,
        bool bTail2,
        bool bDetail) const;

    // get/setSdrObjectTransformation
    virtual const basegfx::B2DHomMatrix& getSdrObjectTransformation() const;
    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOEDGE_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
