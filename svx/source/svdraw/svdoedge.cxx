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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdoedge.hxx>
#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdattrx.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/svddrag.hxx>
#include <svx/svddrgv.hxx>
#include "svddrgm1.hxx"
#include <svx/svdhdl.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // Objektname
#include <svl/style.hxx>
#include <svl/smplhint.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdr/properties/connectorproperties.hxx>
#include <svx/sdr/contact/viewcontactofsdredgeobj.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/svdobj.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

#define SDRESC_VERT (sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP|sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM)
#define SDRESC_HORZ (sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT|sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT)
#define SDRESC_ALL (SDRESC_HORZ|SDRESC_VERT)

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrObjConnection

class SdrObjConnection : public SfxListener
{
private:
    SdrEdgeObj*                 mpOwner;                // connector this belongs to

    basegfx::B2DPoint           maPosition;             // connector position
    SdrObject*                  mpConnectedSdrObject;   // referenced SdrOebjct
    sal_uInt32                  mnConnectorId;          // Connector ID

    /// bitfield
    /// when set, connector will find best connection itself
    bool                        mbBestConnection : 1;

    // connected to automatic gluepoint (AutoGluePoint)
    bool                        mbAutoVertex : 1;

protected:
    // object notifies; e.g. connected SDrObjects changed
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    // if preconditions for positions change, calculate current and set, The
    // position depends on the object connected to and the GluePointID, but also
    // needs to be checked on object change
    void checkPositionFromObject();
    bool getPositionFromObject(basegfx::B2DPoint& rPosition);

public:
    // costruct with object connection, position is derived if
    // pConnectedSdrObject and nConnectorId is given
    SdrObjConnection(SdrEdgeObj* pOwner);

    // copy constructor will ignore Owner, but copy SdrObject connection
    SdrObjConnection(const SdrObjConnection&);

    virtual ~SdrObjConnection();

    /// assignment operator; will not copy mpOwner
    SdrObjConnection& operator=(const SdrObjConnection&);

    /// called when page access of owner changes (owner gets removed/
    /// inserted, e.g deleted but put to undo stack)
    void ownerPageChange();

    // get a copy of the GluePoint referenced; returns true if
    // a GluePoint is referenced and when it got copied to rGP
    bool TakeGluePoint(sdr::glue::GluePoint& rGP) const;

    // allow SdrEdgeObj to adapt position in two cases: user defined
    // EdgeTrack or result of BestConnection being available
    void adaptBestConnectionPosition(const basegfx::B2DPoint& rPosition);

    // data write access
    void SetPosition(const basegfx::B2DPoint& rPosition);
    void SetConnectedSdrObject(SdrObject* pConnectedSdrObject);
    void SetConnectorID(sal_uInt32 nNew);
    void setBestConnection(bool bNew) { mbBestConnection = bNew; }
    void setAutoVertex(bool bNew) { mbAutoVertex = bNew; }

    // data read access
    SdrEdgeObj* getOwner() const { return mpOwner; }
    const basegfx::B2DPoint& GetPosition() const { return maPosition; }
    SdrObject* GetConnectedSdrObject() const { return mpConnectedSdrObject; }
    sal_uInt32 GetConnectorId() const { return mnConnectorId; };
    bool IsBestConnection() const { return mbBestConnection; };
    bool IsAutoVertex() const { return mbAutoVertex; };
};


void SdrObjConnection::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if(mpOwner)
    {
        if(mpConnectedSdrObject && mpConnectedSdrObject == &rBC)
        {
            const SdrBaseHint* pBase = dynamic_cast< const SdrBaseHint* >( &rHint);

            if(pBase)
            {
                const SdrHintKind eSdrHintKind(pBase->GetSdrHintKind());

                // object listening to is delete and/or removed from page, break linkage
                if(HINT_SDROBJECTDYING == eSdrHintKind || HINT_OBJREMOVED == eSdrHintKind)
                {
                    EndListening(*mpConnectedSdrObject);
                    mpConnectedSdrObject = 0;
                }
            }

            if(mpConnectedSdrObject)
            {
                checkPositionFromObject();
            }
        }
    }
}

void SdrObjConnection::checkPositionFromObject()
{
    if(mpConnectedSdrObject)
    {
        basegfx::B2DPoint aNewPosition(maPosition);

        if(getPositionFromObject(aNewPosition) && !maPosition.equal(aNewPosition))
        {
            maPosition = aNewPosition;

            if(mpOwner)
            {
                mpOwner->geometryChange();
            }
        }
    }
}

bool SdrObjConnection::getPositionFromObject(basegfx::B2DPoint& rPosition)
{
    if(mpConnectedSdrObject)
    {
        const sdr::glue::GluePointProvider& rProvider = mpConnectedSdrObject->GetGluePointProvider();
        bool bFound(false);

        if(mbAutoVertex)
        {
            rPosition = rProvider.getAutoGluePointByIndex(mnConnectorId).getUnitPosition();
            bFound = true;
        }
        else
        {
            if(rProvider.hasUserGluePoints())
            {
                const sdr::glue::GluePoint* pCandidate = rProvider.findUserGluePointByID(mnConnectorId);

                if(pCandidate)
                {
                    rPosition = pCandidate->getUnitPosition();
                    bFound = true;
                }
            }
        }

        if(bFound)
        {
            rPosition = mpConnectedSdrObject->getSdrObjectTransformation() * rPosition;
            return true;
        }
    }

    return false;
}

SdrObjConnection::SdrObjConnection(SdrEdgeObj* pOwner)
:   SfxListener(),
    mpOwner(pOwner),
    maPosition(0.0, 0.0),
    mpConnectedSdrObject(0),
    mnConnectorId(0),
    mbBestConnection(true),
    mbAutoVertex(false)
{
    SetConnectedSdrObject(mpConnectedSdrObject);
}

SdrObjConnection::SdrObjConnection(const SdrObjConnection& rSource)
:   SfxListener(),
    mpOwner(0), // ignore owner; this will make copy-constructed objects inactive
    maPosition(rSource.maPosition),
    mpConnectedSdrObject(rSource.mpConnectedSdrObject),
    mnConnectorId(rSource.mnConnectorId),
    mbBestConnection(rSource.mbBestConnection),
    mbAutoVertex(rSource.mbAutoVertex)
{
}

SdrObjConnection::~SdrObjConnection()
{
    if(mpConnectedSdrObject)
    {
        EndListening(*mpConnectedSdrObject);
    }
}

SdrObjConnection& SdrObjConnection::operator=(const SdrObjConnection& rSource)
{
    if(mpConnectedSdrObject)
    {
        EndListening(*mpConnectedSdrObject);
    }

    maPosition = rSource.maPosition;
    mpConnectedSdrObject = rSource.mpConnectedSdrObject;
    mnConnectorId = rSource.mnConnectorId;
    mbBestConnection = rSource.mbBestConnection;
    mbAutoVertex = rSource.mbAutoVertex;

    if(mpOwner && mpConnectedSdrObject && mpOwner->getSdrPageFromSdrObject())
    {
        StartListening(*mpConnectedSdrObject);
    }

    checkPositionFromObject();
    return *this;
}

void SdrObjConnection::adaptBestConnectionPosition(const basegfx::B2DPoint& rPosition)
{
    if(IsBestConnection() || (mpOwner && mpOwner->mbEdgeTrackUserDefined))
    {
        maPosition = rPosition;
    }
    else
    {
        OSL_ENSURE(false, "SdrObjConnection::adaptBestConnectionPosition only allowed for BestConnection or UserDefined (!)");
    }
}

void SdrObjConnection::SetPosition(const basegfx::B2DPoint& rPosition)
{
    basegfx::B2DPoint aNewPosition(rPosition);

    if(mpConnectedSdrObject)
    {
        getPositionFromObject(aNewPosition);
    }

    if(!aNewPosition.equal(maPosition))
    {
        maPosition = aNewPosition;

        if(mpOwner)
        {
            mpOwner->geometryChange();
        }
    }
}

void SdrObjConnection::SetConnectedSdrObject(SdrObject* pConnectedSdrObject)
{
    if(pConnectedSdrObject != mpConnectedSdrObject)
    {
        if(mpConnectedSdrObject)
        {
            EndListening(*mpConnectedSdrObject);
        }

        mpConnectedSdrObject = pConnectedSdrObject;

        if(mpOwner && mpConnectedSdrObject && mpOwner->getSdrPageFromSdrObject())
        {
            StartListening(*mpConnectedSdrObject);
        }

        checkPositionFromObject();
    }
}

void SdrObjConnection::SetConnectorID(sal_uInt32 nNew)
{
    if(nNew != mnConnectorId)
    {
        mnConnectorId = nNew;
        checkPositionFromObject();
    }
}

void SdrObjConnection::ownerPageChange()
{
    if(mpOwner && mpConnectedSdrObject)
    {
        if(mpOwner->getSdrPageFromSdrObject())
        {
            StartListening(*mpConnectedSdrObject);
        }
        else
        {
            EndListening(*mpConnectedSdrObject);
        }
    }
}

bool SdrObjConnection::TakeGluePoint(sdr::glue::GluePoint& rGP) const
{
    if(mpConnectedSdrObject)
    {
        // Ein Obj muss schon angedockt sein!
        const sdr::glue::GluePointProvider& rProvider = mpConnectedSdrObject->GetGluePointProvider();

        if(mbAutoVertex)
        {
            rGP = rProvider.getAutoGluePointByIndex(mnConnectorId);

            return true;
        }
        else
        {

            if(rProvider.hasUserGluePoints())
            {
                const sdr::glue::GluePoint* pCandidate = rProvider.findUserGluePointByID(mnConnectorId);

                if(pCandidate)
                {
                    rGP = *pCandidate;

                    return true;
                }
            }
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrEdgeInfoRec

class SdrEdgeInfoRec
{
public:
    // Die 5 Distanzen werden beim draggen bzw. per SetAttr gesetzt und von
    // ImpCalcEdgeTrack ausgewertet. Per Get/SetAttr/Get/SetStyleSh werden
    // jedoch nur 0-3 longs transportiert.
    basegfx::B2DPoint           aObj1Line2;
    basegfx::B2DPoint           aObj1Line3;
    basegfx::B2DPoint           aObj2Line2;
    basegfx::B2DPoint           aObj2Line3;
    basegfx::B2DPoint           aMiddleLine;

    // Nachfolgende Werte werden von ImpCalcEdgeTrack gesetzt
    long                        nAngle1;           // Austrittswinkel am Obj1
    long                        nAngle2;           // Austrittswinkel am Obj2
    sal_uInt16                  nObj1Lines;        // 1..3
    sal_uInt16                  nObj2Lines;        // 1..3
    sal_uInt16                  nMiddleLine;       // 0xFFFF=keine, sonst Punktnummer des Linienbeginns
    char                        cOrthoForm;        // Form des Ortho-Verbindes, z.B. 'Z','U',I','L','S',...

    SdrEdgeInfoRec()
    :   nAngle1(0),
        nAngle2(0),
        nObj1Lines(0),
        nObj2Lines(0),
        nMiddleLine(0xFFFF),
        cOrthoForm(0)
    {}

    basegfx::B2DPoint& ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode);
    const basegfx::B2DPoint& ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode) const { return const_cast< SdrEdgeInfoRec* >(this)->ImpGetLineVersatzPoint(eLineCode); }
    sal_uInt32 ImpGetPolyIdx(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount) const;
    bool ImpIsHorzLine(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount) const;
    void ImpSetLineVersatz(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount, long nVal);
    long ImpGetLineVersatz(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount) const;
    void ImpResetUserDistances() { aObj1Line2 = aObj1Line3 = aObj2Line2 = aObj2Line3 = aMiddleLine = basegfx::B2DPoint(); }
    bool ImpUsesUserDistances() const;

    bool operator==(const SdrEdgeInfoRec& rCompare) const;
    bool operator!=(const SdrEdgeInfoRec& rCompare) const { return !operator==(rCompare); }
};

basegfx::B2DPoint& SdrEdgeInfoRec::ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode)
{
    switch (eLineCode)
    {
        case OBJ1LINE2 : return aObj1Line2;
        case OBJ1LINE3 : return aObj1Line3;
        case OBJ2LINE2 : return aObj2Line2;
        case OBJ2LINE3 : return aObj2Line3;
        case MIDDLELINE: return aMiddleLine;
    } // switch
    return aMiddleLine;
}

sal_uInt32 SdrEdgeInfoRec::ImpGetPolyIdx(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount) const
{
    switch (eLineCode)
    {
        case OBJ1LINE2 : return 1;
        case OBJ1LINE3 : return 2;
        case OBJ2LINE2 : return nPointCount - 3;
        case OBJ2LINE3 : return nPointCount - 4;
        case MIDDLELINE: return nMiddleLine;
    } // switch
    return 0;
}

bool SdrEdgeInfoRec::ImpIsHorzLine(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount) const
{
    sal_uInt32 nIdx(ImpGetPolyIdx(eLineCode, nPointCount));
    bool bHorz(0 == nAngle1 || 18000 == nAngle1);

    if(OBJ2LINE2 == eLineCode || OBJ2LINE3 == eLineCode)
    {
        nIdx = nPointCount - nIdx;
        bHorz = (0 == nAngle2 || 18000 == nAngle2);
    }

    if(1 == (nIdx & 1))
    {
        bHorz = !bHorz;
    }

    return bHorz;
}

void SdrEdgeInfoRec::ImpSetLineVersatz(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount, long nVal)
{
    basegfx::B2DPoint& rPt=ImpGetLineVersatzPoint(eLineCode);

    if(ImpIsHorzLine(eLineCode, nPointCount))
    {
        rPt.setY(nVal);
    }
    else
    {
        rPt.setX(nVal);
    }
}

long SdrEdgeInfoRec::ImpGetLineVersatz(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount) const
{
    const basegfx::B2DPoint& rPt = ImpGetLineVersatzPoint(eLineCode);

    if(ImpIsHorzLine(eLineCode, nPointCount))
    {
        return rPt.getY();
    }
    else
    {
        return rPt.getX();
    }
}

bool SdrEdgeInfoRec::ImpUsesUserDistances() const
{
    if(!aObj1Line2.equal(basegfx::B2DPoint::getEmptyPoint())) return true;
    if(!aObj1Line3.equal(basegfx::B2DPoint::getEmptyPoint())) return true;
    if(!aObj2Line2.equal(basegfx::B2DPoint::getEmptyPoint())) return true;
    if(!aObj2Line3.equal(basegfx::B2DPoint::getEmptyPoint())) return true;
    if(!aMiddleLine.equal(basegfx::B2DPoint::getEmptyPoint())) return true;
    return false;
}

bool SdrEdgeInfoRec::operator==(const SdrEdgeInfoRec& rCompare) const
{
    return aObj1Line2.equal(rCompare.aObj1Line2)
        && aObj1Line3.equal(rCompare.aObj1Line3)
        && aObj2Line2.equal(rCompare.aObj2Line2)
        && aObj2Line3.equal(rCompare.aObj2Line3)
        && aMiddleLine.equal(rCompare.aMiddleLine)
        && nAngle1 == rCompare.nAngle1
        && nAngle2 == rCompare.nAngle2
        && nObj1Lines == rCompare.nObj1Lines
        && nObj2Lines == rCompare.nObj2Lines
        && nMiddleLine == rCompare.nMiddleLine
        && cOrthoForm == rCompare.cOrthoForm;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrEdgeObjGeoData : public SdrObjGeoData
{
public:
    SdrObjConnection*           mpCon1;  // Verbindungszustand des Linienanfangs
    SdrObjConnection*           mpCon2;  // Verbindungszustand des Linienendes
    basegfx::B2DPolygon         maEdgeTrack;
    SdrEdgeInfoRec*             mpEdgeInfo;

    /// bitfield
    bool                        mbEdgeTrackUserDefined : 1;

    SdrEdgeObjGeoData()
    :   mpCon1(new SdrObjConnection(0)),
        mpCon2(new SdrObjConnection(0)),
        maEdgeTrack(),
        mpEdgeInfo(new SdrEdgeInfoRec()),
        mbEdgeTrackUserDefined(false)
    {
    }

    virtual ~SdrEdgeObjGeoData();
};

SdrEdgeObjGeoData::~SdrEdgeObjGeoData()
{
    delete mpCon1;
    delete mpCon2;
    delete mpEdgeInfo;
}

//////////////////////////////////////////////////////////////////////////////
// GluePoint section

namespace sdr
{
    namespace glue
    {
        // SdrEdgeObj implements it's own GluePointProvider since
        // - it does not support UserGluePoints -> use GluePointProvider
        // - it supports AutoGluePoints, but different from standard -> replace
        //   AutoGluePoint methods

        class SdrEdgeObjGluePointProvider : public GluePointProvider
        {
        private:
            // we need a reference to the owner object, given as reference in
            // the constructor to express that it will always be set
            const SdrEdgeObj*           mpSource;

        protected:
            SdrEdgeObjGluePointProvider(const SdrEdgeObjGluePointProvider& rCandidate);
            virtual GluePointProvider& operator=(const GluePointProvider& rCandidate);

        public:
            // construction, destruction, copying
            SdrEdgeObjGluePointProvider(const SdrEdgeObj& rSource);
            virtual ~SdrEdgeObjGluePointProvider();

            // copying
            virtual GluePointProvider* Clone() const;

            // AutoGluePoint read access (read only)
            virtual sal_uInt32 getAutoGluePointCount() const;
            virtual GluePoint getAutoGluePointByIndex(sal_uInt32 nIndex) const;
        };

        SdrEdgeObjGluePointProvider::SdrEdgeObjGluePointProvider(const SdrEdgeObj& rSource)
        :   GluePointProvider(),
            mpSource(&rSource)
        {
        }

        SdrEdgeObjGluePointProvider::~SdrEdgeObjGluePointProvider()
        {
        }

        SdrEdgeObjGluePointProvider::SdrEdgeObjGluePointProvider(const SdrEdgeObjGluePointProvider& rCandidate)
        :   GluePointProvider(),
            mpSource(rCandidate.mpSource)
        {
        }

        GluePointProvider& SdrEdgeObjGluePointProvider::operator=(const GluePointProvider& rCandidate)
        {
            // call parent to copy UserGluePoints
            GluePointProvider::operator=(rCandidate);

            const SdrEdgeObjGluePointProvider* pSource = dynamic_cast< const SdrEdgeObjGluePointProvider* >(&rCandidate);

            if(pSource)
            {
                mpSource = pSource->mpSource;
            }

            return *this;
        }

        GluePointProvider* SdrEdgeObjGluePointProvider::Clone() const
        {
            return new SdrEdgeObjGluePointProvider(*this);
        }

        sal_uInt32 SdrEdgeObjGluePointProvider::getAutoGluePointCount() const
        {
            // to stay compatible, also use four points (anyways UNO API uses a fixed count of four
            // currently). Seel below, 0,1 create the same point, 2 the start, 3, the end
            return 4;
        }

        GluePoint SdrEdgeObjGluePointProvider::getAutoGluePointByIndex(sal_uInt32 nIndex) const
        {
            // no error with indices, just repeatedly return last GluePoint as fallback
            const basegfx::B2DPolygon aEdgeTrack(mpSource->GetEdgeTrackPath());
            const sal_uInt32 nPntAnz(aEdgeTrack.count());
            basegfx::B2DPoint aOldPoint(0.0, 0.0);

            if(nPntAnz)
            {
                switch(nIndex)
                {
                    case 2:
                    {
                        if(!mpSource->GetSdrObjectConnection(true))
                        {
                            aOldPoint = aEdgeTrack.getB2DPoint(0);
                        }
                        break;
                    }
                    case 3:
                    {
                        if(!mpSource->GetSdrObjectConnection(false))
                        {
                            aOldPoint = aEdgeTrack.getB2DPoint(nPntAnz - 1);
                        }
                        break;
                    }
                    default:
                    {
                        if(1 == (nPntAnz & 1))
                        {
                            aOldPoint = aEdgeTrack.getB2DPoint(nPntAnz / 2);
                        }
                        else
                        {
                            aOldPoint = (aEdgeTrack.getB2DPoint((nPntAnz/2) - 1) + aEdgeTrack.getB2DPoint(nPntAnz/2)) * 0.5;
                        }
                        break;
                    }
                }
            }

            // need to make absolute position relative. temporarily correct zero sizes for invert
            basegfx::B2DHomMatrix aTransform(basegfx::tools::guaranteeMinimalScaling(mpSource->getSdrObjectTransformation()));

            aTransform.invert();
            aOldPoint = aTransform * aOldPoint;

            return sdr::glue::GluePoint(
                aOldPoint,
                sdr::glue::GluePoint::ESCAPE_DIRECTION_SMART,
                sdr::glue::GluePoint::Alignment_Center,
                sdr::glue::GluePoint::Alignment_Center,
                true,   // mbRelative
                false); // mbUserDefined
        }
    } // end of namespace glue
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// old connector geometry stuff, still needed but isolated

namespace
{
    sal_uInt16 impOldCalcEscAngle(const basegfx::B2DRange& aRange, const basegfx::B2DPoint& rPt)
    {
        const double dxl(rPt.getX() - aRange.getMinX());
        const double dyo(rPt.getY() - aRange.getMinY());
        const double dxr(aRange.getMaxX() - rPt.getX());
        const double dyu(aRange.getMaxY() - rPt.getY());
        const bool bxMitt(fabs(dxl - dxr) < 2.0);
        const bool byMitt(fabs(dyo - dyu) < 2.0);

        if(bxMitt && byMitt)
        {
            return SDRESC_ALL;
        }

        const double dx(std::min(dxl, dxr));
        const double dy(std::min(dyo, dyu));
        const bool bDiag(fabs(dx - dy) < 2.0);

        if (bDiag)
        {
            sal_uInt16 nRet(0);

            if(byMitt)
            {
                nRet |= SDRESC_VERT;
            }

            if(bxMitt)
            {
                nRet |= SDRESC_HORZ;
            }

            if(dxl < dxr)
            {
                if(dyo<dyu)
                {
                    nRet |= sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT | sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP;
                }
                else
                {
                    nRet |= sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT | sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM;
                }
            }
            else
            {
                if(dyo<dyu)
                {
                    nRet |= sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT | sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP;
                }
                else
                {
                    nRet|=sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT | sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM;
                }
            }

            return nRet;
        }

        if(dx < dy)
        {
            if(bxMitt)
            {
                return SDRESC_HORZ;
            }
            if(dxl<dxr)
            {
                return sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT;
            }
            else
            {
                return sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT;
            }
        }
        else
        {
            if(byMitt)
            {
                return SDRESC_VERT;
            }
            if(dyo<dyu)
            {
                return sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP;
            }
            else
            {
                return sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM;
            }
        }
    }

    XPolygon impOldCalcObjToCenter(const Point& rStPt, long nEscAngle, const Rectangle& rRect, const Point& rMeeting)
    {
        XPolygon aXP;
        aXP.Insert(XPOLY_APPEND,rStPt,XPOLY_NORMAL);
        bool bRts=nEscAngle==0;
        bool bObn=nEscAngle==9000;
        bool bLks=nEscAngle==18000;
        bool bUnt=nEscAngle==27000;

        Point aP1(rStPt); // erstmal den Pflichtabstand
        if (bLks) aP1.X()=rRect.Left();
        if (bRts) aP1.X()=rRect.Right();
        if (bObn) aP1.Y()=rRect.Top();
        if (bUnt) aP1.Y()=rRect.Bottom();

        bool bFinish=false;
        if (!bFinish) {
            Point aP2(aP1); // Und nun den Pflichtabstand ggf. bis auf Meetinghoehe erweitern
            if (bLks && rMeeting.X()<=aP2.X()) aP2.X()=rMeeting.X();
            if (bRts && rMeeting.X()>=aP2.X()) aP2.X()=rMeeting.X();
            if (bObn && rMeeting.Y()<=aP2.Y()) aP2.Y()=rMeeting.Y();
            if (bUnt && rMeeting.Y()>=aP2.Y()) aP2.Y()=rMeeting.Y();
            aXP.Insert(XPOLY_APPEND,aP2,XPOLY_NORMAL);

            Point aP3(aP2);
            if ((bLks && rMeeting.X()>aP2.X()) || (bRts && rMeeting.X()<aP2.X())) { // Aussenrum
                if (rMeeting.Y()<aP2.Y()) {
                    aP3.Y()=rRect.Top();
                    if (rMeeting.Y()<aP3.Y()) aP3.Y()=rMeeting.Y();
                } else {
                    aP3.Y()=rRect.Bottom();
                    if (rMeeting.Y()>aP3.Y()) aP3.Y()=rMeeting.Y();
                }
                aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
                if (aP3.Y()!=rMeeting.Y()) {
                    aP3.X()=rMeeting.X();
                    aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
                }
            }
            if ((bObn && rMeeting.Y()>aP2.Y()) || (bUnt && rMeeting.Y()<aP2.Y())) { // Aussenrum
                if (rMeeting.X()<aP2.X()) {
                    aP3.X()=rRect.Left();
                    if (rMeeting.X()<aP3.X()) aP3.X()=rMeeting.X();
                } else {
                    aP3.X()=rRect.Right();
                    if (rMeeting.X()>aP3.X()) aP3.X()=rMeeting.X();
                }
                aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
                if (aP3.X()!=rMeeting.X()) {
                    aP3.Y()=rMeeting.Y();
                    aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
                }
            }
        }
#ifdef DBG_UTIL
        if (aXP.GetPointCount()>4) {
            DBG_ERROR("SdrEdgeObj::impOldCalcObjToCenter(): Polygon hat mehr als 4 Punkte!");
        }
#endif
        return aXP;
    }

    basegfx::B2DPolygon impOldCalcEdgeTrack(
        const Point& rPt1, long nAngle1, const Rectangle& rBoundRect1, const Rectangle& rBewareRect1,
        const Point& rPt2, long nAngle2, const Rectangle& rBoundRect2, const Rectangle& rBewareRect2,
        SdrEdgeKind eKind,
        sal_uInt32* pnQuality, SdrEdgeInfoRec* pInfo)
    {
        bool bRts1=nAngle1==0;
        bool bObn1=nAngle1==9000;
        bool bLks1=nAngle1==18000;
        bool bUnt1=nAngle1==27000;
        bool bHor1=bLks1 || bRts1;
        bool bVer1=bObn1 || bUnt1;
        bool bRts2=nAngle2==0;
        bool bObn2=nAngle2==9000;
        bool bLks2=nAngle2==18000;
        bool bUnt2=nAngle2==27000;
        bool bHor2=bLks2 || bRts2;
        bool bVer2=bObn2 || bUnt2;
        bool bInfo=pInfo!=NULL;
        if (bInfo) {
            pInfo->cOrthoForm=0;
            pInfo->nAngle1=nAngle1;
            pInfo->nAngle2=nAngle2;
            pInfo->nObj1Lines=1;
            pInfo->nObj2Lines=1;
            pInfo->nMiddleLine=0xFFFF;
        }
        Point aPt1(rPt1);
        Point aPt2(rPt2);
        Rectangle aBoundRect1 (rBoundRect1 );
        Rectangle aBoundRect2 (rBoundRect2 );
        Rectangle aBewareRect1(rBewareRect1);
        Rectangle aBewareRect2(rBewareRect2);
        Point aMeeting((aPt1.X()+aPt2.X()+1)/2,(aPt1.Y()+aPt2.Y()+1)/2);
        bool bMeetingXMid=true;
        bool bMeetingYMid=true;
        if (eKind==SDREDGE_ONELINE) {
            XPolygon aXP(2);
            aXP[0]=rPt1;
            aXP[1]=rPt2;
            if (pnQuality!=NULL) {
                *pnQuality=Abs(rPt1.X()-rPt2.X())+Abs(rPt1.Y()-rPt2.Y());
            }
            return aXP.getB2DPolygon();
        } else if (eKind==SDREDGE_THREELINES) {
            XPolygon aXP(4);
            aXP[0]=rPt1;
            aXP[1]=rPt1;
            aXP[2]=rPt2;
            aXP[3]=rPt2;
            if (bRts1) aXP[1].X()=aBewareRect1.Right();  //+=500;
            if (bObn1) aXP[1].Y()=aBewareRect1.Top();    //-=500;
            if (bLks1) aXP[1].X()=aBewareRect1.Left();   //-=500;
            if (bUnt1) aXP[1].Y()=aBewareRect1.Bottom(); //+=500;
            if (bRts2) aXP[2].X()=aBewareRect2.Right();  //+=500;
            if (bObn2) aXP[2].Y()=aBewareRect2.Top();    //-=500;
            if (bLks2) aXP[2].X()=aBewareRect2.Left();   //-=500;
            if (bUnt2) aXP[2].Y()=aBewareRect2.Bottom(); //+=500;
            if (pnQuality!=NULL) {
                long nQ=Abs(aXP[1].X()-aXP[0].X())+Abs(aXP[1].Y()-aXP[0].Y());
                    nQ+=Abs(aXP[2].X()-aXP[1].X())+Abs(aXP[2].Y()-aXP[1].Y());
                    nQ+=Abs(aXP[3].X()-aXP[2].X())+Abs(aXP[3].Y()-aXP[2].Y());
                *pnQuality=nQ;
            }
            if (bInfo) {
                pInfo->nObj1Lines=2;
                pInfo->nObj2Lines=2;
                if (bHor1) {
                    aXP[1].X()+=pInfo->aObj1Line2.getX();
                } else {
                    aXP[1].Y()+=pInfo->aObj1Line2.getY();
                }
                if (bHor2) {
                    aXP[2].X()+=pInfo->aObj2Line2.getX();
                } else {
                    aXP[2].Y()+=pInfo->aObj2Line2.getY();
                }
            }
            return aXP.getB2DPolygon();
        }
        sal_uInt16 nIntersections=0;
        bool bForceMeeting=false; // Muss die Linie durch den MeetingPoint laufen?
        {
            Point aC1(aBewareRect1.Center());
            Point aC2(aBewareRect2.Center());
            if (aBewareRect1.Left()<=aBewareRect2.Right() && aBewareRect1.Right()>=aBewareRect2.Left()) {
                // Ueberschneidung auf der X-Achse
                long n1=Max(aBewareRect1.Left(),aBewareRect2.Left());
                long n2=Min(aBewareRect1.Right(),aBewareRect2.Right());
                aMeeting.X()=(n1+n2+1)/2;
            } else {
                // Ansonsten den Mittelpunkt des Freiraums
                if (aC1.X()<aC2.X()) {
                    aMeeting.X()=(aBewareRect1.Right()+aBewareRect2.Left()+1)/2;
                } else {
                    aMeeting.X()=(aBewareRect1.Left()+aBewareRect2.Right()+1)/2;
                }
            }
            if (aBewareRect1.Top()<=aBewareRect2.Bottom() && aBewareRect1.Bottom()>=aBewareRect2.Top()) {
                // Ueberschneidung auf der Y-Achse
                long n1=Max(aBewareRect1.Top(),aBewareRect2.Top());
                long n2=Min(aBewareRect1.Bottom(),aBewareRect2.Bottom());
                aMeeting.Y()=(n1+n2+1)/2;
            } else {
                // Ansonsten den Mittelpunkt des Freiraums
                if (aC1.Y()<aC2.Y()) {
                    aMeeting.Y()=(aBewareRect1.Bottom()+aBewareRect2.Top()+1)/2;
                } else {
                    aMeeting.Y()=(aBewareRect1.Top()+aBewareRect2.Bottom()+1)/2;
                }
            }
            // Im Prinzip gibt es 3 zu unterscheidene Faelle:
            //   1. Beide in die selbe Richtung
            //   2. Beide in genau entgegengesetzte Richtungen
            //   3. Einer waagerecht und der andere senkrecht
            long nXMin=Min(aBewareRect1.Left(),aBewareRect2.Left());
            long nXMax=Max(aBewareRect1.Right(),aBewareRect2.Right());
            long nYMin=Min(aBewareRect1.Top(),aBewareRect2.Top());
            long nYMax=Max(aBewareRect1.Bottom(),aBewareRect2.Bottom());
            //bool bBoundOverlap=aBoundRect1.Right()>aBoundRect2.Left() && aBoundRect1.Left()<aBoundRect2.Right() &&
            //                     aBoundRect1.Bottom()>aBoundRect2.Top() && aBoundRect1.Top()<aBoundRect2.Bottom();
            bool bBewareOverlap=aBewareRect1.Right()>aBewareRect2.Left() && aBewareRect1.Left()<aBewareRect2.Right() &&
                                    aBewareRect1.Bottom()>aBewareRect2.Top() && aBewareRect1.Top()<aBewareRect2.Bottom();
            unsigned nMainCase=3;
            if (nAngle1==nAngle2) nMainCase=1;
            else if ((bHor1 && bHor2) || (bVer1 && bVer2)) nMainCase=2;
            if (nMainCase==1) { // Fall 1: Beide in eine Richtung moeglich.
                if (bVer1) aMeeting.X()=(aPt1.X()+aPt2.X()+1)/2; // ist hier besser, als der
                if (bHor1) aMeeting.Y()=(aPt1.Y()+aPt2.Y()+1)/2; // Mittelpunkt des Freiraums
                // bX1Ok bedeutet, dass die Vertikale, die aus Obj1 austritt, keinen Konflikt mit Obj2 bildet, ...
                bool bX1Ok=aPt1.X()<=aBewareRect2.Left() || aPt1.X()>=aBewareRect2.Right();
                bool bX2Ok=aPt2.X()<=aBewareRect1.Left() || aPt2.X()>=aBewareRect1.Right();
                bool bY1Ok=aPt1.Y()<=aBewareRect2.Top() || aPt1.Y()>=aBewareRect2.Bottom();
                bool bY2Ok=aPt2.Y()<=aBewareRect1.Top() || aPt2.Y()>=aBewareRect1.Bottom();
                if (bLks1 && (bY1Ok || aBewareRect1.Left()<aBewareRect2.Right()) && (bY2Ok || aBewareRect2.Left()<aBewareRect1.Right())) {
                    aMeeting.X()=nXMin;
                    bMeetingXMid=false;
                }
                if (bRts1 && (bY1Ok || aBewareRect1.Right()>aBewareRect2.Left()) && (bY2Ok || aBewareRect2.Right()>aBewareRect1.Left())) {
                    aMeeting.X()=nXMax;
                    bMeetingXMid=false;
                }
                if (bObn1 && (bX1Ok || aBewareRect1.Top()<aBewareRect2.Bottom()) && (bX2Ok || aBewareRect2.Top()<aBewareRect1.Bottom())) {
                    aMeeting.Y()=nYMin;
                    bMeetingYMid=false;
                }
                if (bUnt1 && (bX1Ok || aBewareRect1.Bottom()>aBewareRect2.Top()) && (bX2Ok || aBewareRect2.Bottom()>aBewareRect1.Top())) {
                    aMeeting.Y()=nYMax;
                    bMeetingYMid=false;
                }
            } else if (nMainCase==2) {
                // Fall 2:
                bForceMeeting=true;
                if (bHor1) { // beide waagerecht
                    /* 9 Moeglichkeiten:                   ù ù ù                    */
                    /*   2.1 Gegenueber, Ueberschneidung   Ã ´ ù                    */
                    /*       nur auf der Y-Achse           ù ù ù                    */
                    /*   2.2, 2.3 Gegenueber, vertikal versetzt. Ã ù ù   ù ù ù      */
                    /*            Ueberschneidung weder auf der  ù ´ ù   ù ´ ù      */
                    /*            X- noch auf der Y-Achse        ù ù ù   Ã ù ù      */
                    /*   2.4, 2.5 Untereinander,   ù Ã ù   ù ù ù                    */
                    /*            Ueberschneidung  ù ´ ù   ù ´ ù                    */
                    /*            nur auf X-Achse  ù ù ù   ù Ã ù                    */
                    /*   2.6, 2.7 Gegeneinander, vertikal versetzt. ù ù Ã   ù ù ù   */
                    /*            Ueberschneidung weder auf der     ù ´ ù   ù ´ ù   */
                    /*            X- noch auf der Y-Achse.          ù ù ù   ù ù Ã   */
                    /*   2.8 Gegeneinander.       ù ù ù                             */
                    /*       Ueberschneidung nur  ù ´ Ã                             */
                    /*       auf der Y-Achse.     ù ù ù                             */
                    /*   2.9 Die BewareRects der Objekte ueberschneiden             */
                    /*       sich auf X- und Y-Achse.                               */
                    /* Die Faelle gelten entsprechend umgesetzt auch fuer           */
                    /* senkrechte Linienaustritte.                                  */
                    /* Die Faelle 2.1-2.7 werden mit dem Default-Meeting ausreichend*/
                    /* gut behandelt. Spezielle MeetingPoints werden hier also nur  */
                    /* fuer 2.8 und 2.9 bestimmt.                                   */

                    // Normalisierung. aR1 soll der nach rechts und
                    // aR2 der nach links austretende sein.
                    Rectangle aBewR1(bRts1 ? aBewareRect1 : aBewareRect2);
                    Rectangle aBewR2(bRts1 ? aBewareRect2 : aBewareRect1);
                    Rectangle aBndR1(bRts1 ? aBoundRect1 : aBoundRect2);
                    Rectangle aBndR2(bRts1 ? aBoundRect2 : aBoundRect1);
                    if (aBewR1.Bottom()>aBewR2.Top() && aBewR1.Top()<aBewR2.Bottom()) {
                        // Ueberschneidung auf der Y-Achse. Faelle 2.1, 2.8, 2.9
                        if (aBewR1.Right()>aBewR2.Left()) {
                            // Faelle 2.8, 2.9
                            // Fall 2.8 ist immer Aussenrumlauf (bDirect=false).
                            // Fall 2.9 kann auch Direktverbindung sein (bei geringer
                            // Ueberschneidung der BewareRects ohne Ueberschneidung der
                            // Boundrects wenn die Linienaustritte sonst das BewareRect
                            // des jeweils anderen Objekts verletzen wuerden.
                            bool bCase29Direct=false;
                            bool bCase29=aBewR1.Right()>aBewR2.Left();
                            if (aBndR1.Right()<=aBndR2.Left()) { // Fall 2.9 und keine Boundrectueberschneidung
                                if ((aPt1.Y()>aBewareRect2.Top() && aPt1.Y()<aBewareRect2.Bottom()) ||
                                    (aPt2.Y()>aBewareRect1.Top() && aPt2.Y()<aBewareRect1.Bottom())) {
                                   bCase29Direct=true;
                                }
                            }
                            if (!bCase29Direct) {
                                bool bObenLang=Abs(nYMin-aMeeting.Y())<=Abs(nYMax-aMeeting.Y());
                                if (bObenLang) {
                                    aMeeting.Y()=nYMin;
                                } else {
                                    aMeeting.Y()=nYMax;
                                }
                                bMeetingYMid=false;
                                if (bCase29) {
                                    // und nun noch dafuer sorgen, dass das
                                    // umzingelte Obj nicht durchquert wird
                                    if ((aBewR1.Center().Y()<aBewR2.Center().Y()) != bObenLang) {
                                        aMeeting.X()=aBewR2.Right();
                                    } else {
                                        aMeeting.X()=aBewR1.Left();
                                    }
                                    bMeetingXMid=false;
                                }
                            } else {
                                // Direkte Verbindung (3-Linien Z-Verbindung), da
                                // Verletzung der BewareRects unvermeidlich ist.
                                // Via Dreisatz werden die BewareRects nun verkleinert.
                                long nWant1=aBewR1.Right()-aBndR1.Right(); // Abstand bei Obj1
                                long nWant2=aBndR2.Left()-aBewR2.Left();   // Abstand bei Obj2
                                long nSpace=aBndR2.Left()-aBndR1.Right(); // verfuegbarer Platz
                                long nGet1=BigMulDiv(nWant1,nSpace,nWant1+nWant2);
                                long nGet2=nSpace-nGet1;
                                if (bRts1) { // Normalisierung zurueckwandeln
                                    aBewareRect1.Right()+=nGet1-nWant1;
                                    aBewareRect2.Left()-=nGet2-nWant2;
                                } else {
                                    aBewareRect2.Right()+=nGet1-nWant1;
                                    aBewareRect1.Left()-=nGet2-nWant2;
                                }
                                nIntersections++; // Qualitaet herabsetzen
                            }
                        }
                    }
                } else if (bVer1) { // beide senkrecht
                    Rectangle aBewR1(bUnt1 ? aBewareRect1 : aBewareRect2);
                    Rectangle aBewR2(bUnt1 ? aBewareRect2 : aBewareRect1);
                    Rectangle aBndR1(bUnt1 ? aBoundRect1 : aBoundRect2);
                    Rectangle aBndR2(bUnt1 ? aBoundRect2 : aBoundRect1);
                    if (aBewR1.Right()>aBewR2.Left() && aBewR1.Left()<aBewR2.Right()) {
                        // Ueberschneidung auf der Y-Achse. Faelle 2.1, 2.8, 2.9
                        if (aBewR1.Bottom()>aBewR2.Top()) {
                            // Faelle 2.8, 2.9
                            // Fall 2.8 ist immer Aussenrumlauf (bDirect=false).
                            // Fall 2.9 kann auch Direktverbindung sein (bei geringer
                            // Ueberschneidung der BewareRects ohne Ueberschneidung der
                            // Boundrects wenn die Linienaustritte sonst das BewareRect
                            // des jeweils anderen Objekts verletzen wuerden.
                            bool bCase29Direct=false;
                            bool bCase29=aBewR1.Bottom()>aBewR2.Top();
                            if (aBndR1.Bottom()<=aBndR2.Top()) { // Fall 2.9 und keine Boundrectueberschneidung
                                if ((aPt1.X()>aBewareRect2.Left() && aPt1.X()<aBewareRect2.Right()) ||
                                    (aPt2.X()>aBewareRect1.Left() && aPt2.X()<aBewareRect1.Right())) {
                                   bCase29Direct=true;
                                }
                            }
                            if (!bCase29Direct) {
                                bool bLinksLang=Abs(nXMin-aMeeting.X())<=Abs(nXMax-aMeeting.X());
                                if (bLinksLang) {
                                    aMeeting.X()=nXMin;
                                } else {
                                    aMeeting.X()=nXMax;
                                }
                                bMeetingXMid=false;
                                if (bCase29) {
                                    // und nun noch dafuer sorgen, dass das
                                    // umzingelte Obj nicht durchquert wird
                                    if ((aBewR1.Center().X()<aBewR2.Center().X()) != bLinksLang) {
                                        aMeeting.Y()=aBewR2.Bottom();
                                    } else {
                                        aMeeting.Y()=aBewR1.Top();
                                    }
                                    bMeetingYMid=false;
                                }
                            } else {
                                // Direkte Verbindung (3-Linien Z-Verbindung), da
                                // Verletzung der BewareRects unvermeidlich ist.
                                // Via Dreisatz werden die BewareRects nun verkleinert.
                                long nWant1=aBewR1.Bottom()-aBndR1.Bottom(); // Abstand bei Obj1
                                long nWant2=aBndR2.Top()-aBewR2.Top();   // Abstand bei Obj2
                                long nSpace=aBndR2.Top()-aBndR1.Bottom(); // verfuegbarer Platz
                                long nGet1=BigMulDiv(nWant1,nSpace,nWant1+nWant2);
                                long nGet2=nSpace-nGet1;
                                if (bUnt1) { // Normalisierung zurueckwandeln
                                    aBewareRect1.Bottom()+=nGet1-nWant1;
                                    aBewareRect2.Top()-=nGet2-nWant2;
                                } else {
                                    aBewareRect2.Bottom()+=nGet1-nWant1;
                                    aBewareRect1.Top()-=nGet2-nWant2;
                                }
                                nIntersections++; // Qualitaet herabsetzen
                            }
                        }
                    }
                }
            } else if (nMainCase==3) { // Fall 3: Einer waagerecht und der andere senkrecht. Sehr viele Fallunterscheidungen
                /* Kleine Legende: ù ú ù ú ù -> Ohne Ueberschneidung, maximal Beruehrung.                   */
                /*                 ú ú ú ú ú -> Ueberschneidung                                             */
                /*                 ù ú Ã ú ù -> Selbe Hoehe                                                 */
                /*                 ú ú ú ú ú -> Ueberschneidung                                             */
                /*                 ù ú ù ú ù -> Ohne Ueberschneidung, maximal Beruehrung.                   */
                /* Linienaustritte links ´, rechts Ã, oben Á und unten Â.                                   */
                /* Insgesamt sind 96 Konstellationen moeglich, wobei einige nicht einmal                    */
                /* eindeutig einem Fall und damit einer Behandlungsmethode zugeordnet werden                */
                /* koennen.                                                                                 */
                /* 3.1: Hierzu moegen alle Konstellationen zaehlen, die durch den                           */
                /*      Default-MeetingPoint zufriedenstellend abgedeckt sind (20+12).                      */
                /*   Â Â Â ú Á    Á ú Â Â Â   Diese 12  ù ú ù Â ù    ù ú ù ú ù    ù Â ù ú ù    ù ú ù ú ù    */
                /*   ú ú ú ú Á    Á ú ú ú ú   Konstel.  ú ú ú ú ú    ú ú ú ú Â    ú ú ú ú ú    Â ú ú ú ú    */
                /*   ù ú Ã ú ù    ù ú ´ ú ù   jedoch    ù ú Ã ú Á    ù ú Ã ú Â    Á ú ´ ú ù    Â ú ´ ú ù    */
                /*   ú ú ú ú Â    Â ú ú ú ú   nur zum   ú ú ú ú Á    ú ú ú ú ú    Á ú ú ú ú    ú ú ú ú ú    */
                /*   Á Á Á ú Â    Â ú Á Á Á   Teil:     ù ú ù Á ù    ù ú ù ú ù    ù Á ù ú ù    ù ú ù ú ù    */
                /*   Letztere 16 Faelle scheiden aus, sobald sich die Objekte offen                         */
                /*   gegenueberstehen (siehe Fall 3.2).                                                     */
                /* 3.2: Die Objekte stehen sich offen gegenueber und somit ist eine                         */
                /*      Verbindung mit lediglich 2 Linien moeglich (4+20).                                  */
                /*      Dieser Fall hat 1. Prioritaet.                                                      */
                /*   ù ú ù ú Â   Â ú ù ú ù   Diese 20  ù ú ù Â ù   ù Â ù ú ù   ù ú ù ú ù   ù ú ù ú ù        */
                /*   ú ú ú ú ú   ú ú ú ú ú   Konstel.  ú ú ú Â Â   Â Â ú ú ú   ú ú ú ú ú   ú ú ú ú ú        */
                /*   ù ú Ã ú ù   ù ú ´ ú ù   jedoch    ù ú Ã Á Á   Á Á ´ ú ù   ù ú Ã Â Â   Â Â ´ ú ù        */
                /*   ú ú ú ú ú   ú ú ú ú ú   nur zum   ú ú ú Á Á   Á Á ú ú ú   ú ú ú ú ú   ú ú ú ú ú        */
                /*   ù ú ù ú Á   Á ú ù ú ù   Teil:     ù ú ù Á ù   ù Á ù ú ù   ù ú ù ú ù   ù ú ù ú ù        */
                /* 3.3: Die Linienaustritte zeigen vom anderen Objekt weg bzw. hinter                       */
                /*      dessen Ruecken vorbei (52+4).                                                       */
                /*   Á Á Á Á ù   ù Á Á Á Á   ù ú ú ú ù   ù ú ù ú ù   Diese 4   ù ú ù ú ù   ù ú ù ú ù        */
                /*   Á Á Á Á ú   ú Á Á Á Á   Â Â Â ú ú   ú ú Â Â Â   Konstel.  ú ú ú Â ú   ú Â ú ú ú        */
                /*   Á Á Ã ú ù   ù ú ´ Á Á   Â Â Ã ú ù   ù ú ´ Â Â   jedoch    ù ú Ã ú ù   ù ú ´ ú ù        */
                /*   Á Á Á ú ú   ú ú Á Á Á   Â Â Â Â ú   ú Â Â Â Â   nur zum   ú ú ú Á ú   ú Á ú ú ú        */
                /*   ù ú ù ú ù   ù ú ù ú ù   Â Â Â Â ù   ù Â Â Â Â   Teil:     ù ú ù ú ù   ù ú ù ú ù        */

                // Fall 3.2
                Rectangle aTmpR1(aBewareRect1);
                Rectangle aTmpR2(aBewareRect2);
                if (bBewareOverlap) {
                    // Ueberschneidung der BewareRects: BoundRects fuer Check auf Fall 3.2 verwenden.
                    aTmpR1=aBoundRect1;
                    aTmpR2=aBoundRect2;
                }
                if ((((bRts1 && aTmpR1.Right ()<=aPt2.X()) || (bLks1 && aTmpR1.Left()>=aPt2.X())) &&
                     ((bUnt2 && aTmpR2.Bottom()<=aPt1.Y()) || (bObn2 && aTmpR2.Top ()>=aPt1.Y()))) ||
                    (((bRts2 && aTmpR2.Right ()<=aPt1.X()) || (bLks2 && aTmpR2.Left()>=aPt1.X())) &&
                     ((bUnt1 && aTmpR1.Bottom()<=aPt2.Y()) || (bObn1 && aTmpR1.Top ()>=aPt2.Y())))) {
                    // Fall 3.2 trifft zu: Verbindung mit lediglich 2 Linien
                    bForceMeeting=true;
                    bMeetingXMid=false;
                    bMeetingYMid=false;
                    if (bHor1) {
                        aMeeting.X()=aPt2.X();
                        aMeeting.Y()=aPt1.Y();
                    } else {
                        aMeeting.X()=aPt1.X();
                        aMeeting.Y()=aPt2.Y();
                    }
                    // Falls Ueberschneidung der BewareRects:
                    aBewareRect1=aTmpR1;
                    aBewareRect2=aTmpR2;
                } else if ((((bRts1 && aBewareRect1.Right ()>aBewareRect2.Left  ()) ||
                             (bLks1 && aBewareRect1.Left  ()<aBewareRect2.Right ())) &&
                            ((bUnt2 && aBewareRect2.Bottom()>aBewareRect1.Top   ()) ||
                             (bObn2 && aBewareRect2.Top   ()<aBewareRect1.Bottom()))) ||
                           (((bRts2 && aBewareRect2.Right ()>aBewareRect1.Left  ()) ||
                             (bLks2 && aBewareRect2.Left  ()<aBewareRect1.Right ())) &&
                            ((bUnt1 && aBewareRect1.Bottom()>aBewareRect2.Top   ()) ||
                             (bObn1 && aBewareRect1.Top   ()<aBewareRect2.Bottom())))) {
                    // Fall 3.3
                    bForceMeeting=true;
                    if (bRts1 || bRts2) { aMeeting.X()=nXMax; bMeetingXMid=false; }
                    if (bLks1 || bLks2) { aMeeting.X()=nXMin; bMeetingXMid=false; }
                    if (bUnt1 || bUnt2) { aMeeting.Y()=nYMax; bMeetingYMid=false; }
                    if (bObn1 || bObn2) { aMeeting.Y()=nYMin; bMeetingYMid=false; }
                }
            }
        }

        XPolygon aXP1(impOldCalcObjToCenter(aPt1,nAngle1,aBewareRect1,aMeeting));
        XPolygon aXP2(impOldCalcObjToCenter(aPt2,nAngle2,aBewareRect2,aMeeting));
        sal_uInt16 nXP1Anz=aXP1.GetPointCount();
        sal_uInt16 nXP2Anz=aXP2.GetPointCount();
        if (bInfo) {
            pInfo->nObj1Lines=nXP1Anz; if (nXP1Anz>1) pInfo->nObj1Lines--;
            pInfo->nObj2Lines=nXP2Anz; if (nXP2Anz>1) pInfo->nObj2Lines--;
        }
        Point aEP1(aXP1[nXP1Anz-1]);
        Point aEP2(aXP2[nXP2Anz-1]);
        bool bInsMeetingPoint=aEP1.X()!=aEP2.X() && aEP1.Y()!=aEP2.Y();
        bool bHorzE1=aEP1.Y()==aXP1[nXP1Anz-2].Y(); // letzte Linie von XP1 horizontal?
        bool bHorzE2=aEP2.Y()==aXP2[nXP2Anz-2].Y(); // letzte Linie von XP2 horizontal?
        if (aEP1==aEP2 && (bHorzE1 && bHorzE2 && aEP1.Y()==aEP2.Y()) || (!bHorzE1 && !bHorzE2 && aEP1.X()==aEP2.X())) {
            // Sonderbehandlung fuer 'I'-Verbinder
            nXP1Anz--; aXP1.Remove(nXP1Anz,1);
            nXP2Anz--; aXP2.Remove(nXP2Anz,1);
            bMeetingXMid=false;
            bMeetingYMid=false;
        }
        if (bInsMeetingPoint) {
            aXP1.Insert(XPOLY_APPEND,aMeeting,XPOLY_NORMAL);
            if (bInfo) {
                // Durch einfuegen des MeetingPoints kommen 2 weitere Linie hinzu.
                // Evtl. wird eine von diesen die Mittellinie.
                if (pInfo->nObj1Lines==pInfo->nObj2Lines) {
                    pInfo->nObj1Lines++;
                    pInfo->nObj2Lines++;
                } else {
                    if (pInfo->nObj1Lines>pInfo->nObj2Lines) {
                        pInfo->nObj2Lines++;
                        pInfo->nMiddleLine=nXP1Anz-1;
                    } else {
                        pInfo->nObj1Lines++;
                        pInfo->nMiddleLine=nXP1Anz;
                    }
                }
            }
        } else if (bInfo && aEP1!=aEP2 && nXP1Anz+nXP2Anz>=4) {
            // Durch Verbinden der beiden Enden kommt eine weitere Linie hinzu.
            // Dies wird die Mittellinie.
            pInfo->nMiddleLine=nXP1Anz-1;
        }
        sal_uInt16 nNum=aXP2.GetPointCount();
        if (aXP1[nXP1Anz-1]==aXP2[nXP2Anz-1] && nXP1Anz>1 && nXP2Anz>1) nNum--;
        while (nNum>0) {
            nNum--;
            aXP1.Insert(XPOLY_APPEND,aXP2[nNum],XPOLY_NORMAL);
        }
        sal_uInt16 nPntAnz=aXP1.GetPointCount();
        char cForm=0;
        if (bInfo || pnQuality!=NULL) {
            cForm='?';
            if (nPntAnz==2) cForm='I';
            else if (nPntAnz==3) cForm='L';
            else if (nPntAnz==4) { // Z oder U
                if (nAngle1==nAngle2) cForm='U';
                else cForm='Z';
            } else if (nPntAnz==4) { /* Ú-¿  Ú-¿  */
                /* ...                 -Ù     -Ù  */
            } else if (nPntAnz==6) { // S oder C oder ...
                if (nAngle1!=nAngle2) {
                    // Fuer Typ S hat Linie2 dieselbe Richtung wie Linie4.
                    // Bei Typ C sind die beiden genau entgegengesetzt.
                    Point aP1(aXP1[1]);
                    Point aP2(aXP1[2]);
                    Point aP3(aXP1[3]);
                    Point aP4(aXP1[4]);
                    if (aP1.Y()==aP2.Y()) { // beide Linien Horz
                        if ((aP1.X()<aP2.X())==(aP3.X()<aP4.X())) cForm='S';
                        else cForm='C';
                    } else { // sonst beide Linien Vert
                        if ((aP1.Y()<aP2.Y())==(aP3.Y()<aP4.Y())) cForm='S';
                        else cForm='C';
                    }
                } else cForm='4'; // sonst der 3. Fall mit 5 Linien
            } else cForm='?';  //
            // Weitere Formen:
            if (bInfo) {
                pInfo->cOrthoForm=cForm;
                if (cForm=='I' || cForm=='L' || cForm=='Z' || cForm=='U') {
                    pInfo->nObj1Lines=1;
                    pInfo->nObj2Lines=1;
                    if (cForm=='Z' || cForm=='U') {
                        pInfo->nMiddleLine=1;
                    } else {
                        pInfo->nMiddleLine=0xFFFF;
                    }
                } else if (cForm=='S' || cForm=='C') {
                    pInfo->nObj1Lines=2;
                    pInfo->nObj2Lines=2;
                    pInfo->nMiddleLine=2;
                }
            }
        }
        if (pnQuality!=NULL) {
            sal_uInt32 nQual=0;
            sal_uInt32 nQual0=nQual; // Ueberlaeufe vorbeugen
            bool bOverflow=false;
            Point aPt0(aXP1[0]);
            for (sal_uInt16 nPntNum=1; nPntNum<nPntAnz; nPntNum++) {
                Point aPt1b(aXP1[nPntNum]);
                nQual+=Abs(aPt1b.X()-aPt0.X())+Abs(aPt1b.Y()-aPt0.Y());
                if (nQual<nQual0) bOverflow=true;
                nQual0=nQual;
                aPt0=aPt1b;
            }

            sal_uInt16 nTmp=nPntAnz;
            if (cForm=='Z') {
                nTmp=2; // Z-Form hat gute Qualitaet (nTmp=2 statt 4)
                sal_uInt32 n1=Abs(aXP1[1].X()-aXP1[0].X())+Abs(aXP1[1].Y()-aXP1[0].Y());
                sal_uInt32 n2=Abs(aXP1[2].X()-aXP1[1].X())+Abs(aXP1[2].Y()-aXP1[1].Y());
                sal_uInt32 n3=Abs(aXP1[3].X()-aXP1[2].X())+Abs(aXP1[3].Y()-aXP1[2].Y());
                // fuer moeglichst gleichlange Linien sorgen
                sal_uInt32 nBesser=0;
                n1+=n3;
                n3=n2/4;
                if (n1>=n2) nBesser=6;
                else if (n1>=3*n3) nBesser=4;
                else if (n1>=2*n3) nBesser=2;
                if (aXP1[0].Y()!=aXP1[1].Y()) nBesser++; // Senkrechte Startlinie kriegt auch noch einen Pluspunkt (fuer H/V-Prio)
                if (nQual>nBesser) nQual-=nBesser; else nQual=0;
            }
            if (nTmp>=3) {
                nQual0=nQual;
                nQual+=(sal_uInt32)nTmp*0x01000000;
                if (nQual<nQual0 || nTmp>15) bOverflow=true;
            }
            if (nPntAnz>=2) { // Austrittswinkel nochmal pruefen
                Point aP1(aXP1[1]); aP1-=aXP1[0];
                Point aP2(aXP1[nPntAnz-2]); aP2-=aXP1[nPntAnz-1];
                long nAng1=0; if (aP1.X()<0) nAng1=18000; if (aP1.Y()>0) nAng1=27000;
                if (aP1.Y()<0) nAng1=9000; if (aP1.X()!=0 && aP1.Y()!=0) nAng1=1; // Schraeg!?!
                long nAng2=0; if (aP2.X()<0) nAng2=18000; if (aP2.Y()>0) nAng2=27000;
                if (aP2.Y()<0) nAng2=9000; if (aP2.X()!=0 && aP2.Y()!=0) nAng2=1; // Schraeg!?!
                if (nAng1!=nAngle1) nIntersections++;
                if (nAng2!=nAngle2) nIntersections++;
            }

            // Fuer den Qualitaetscheck wieder die Original-Rects verwenden und
            // gleichzeitig checken, ob eins fuer die Edge-Berechnung verkleinert
            // wurde (z.B. Fall 2.9)
            aBewareRect1=rBewareRect1;
            aBewareRect2=rBewareRect2;

            for (sal_uInt16 i=0; i<nPntAnz; i++) {
                Point aPt1b(aXP1[i]);
                bool b1=aPt1b.X()>aBewareRect1.Left() && aPt1b.X()<aBewareRect1.Right() &&
                            aPt1b.Y()>aBewareRect1.Top() && aPt1b.Y()<aBewareRect1.Bottom();
                bool b2=aPt1b.X()>aBewareRect2.Left() && aPt1b.X()<aBewareRect2.Right() &&
                            aPt1b.Y()>aBewareRect2.Top() && aPt1b.Y()<aBewareRect2.Bottom();
                sal_uInt16 nInt0=nIntersections;
                if (i==0 || i==nPntAnz-1) {
                    if (b1 && b2) nIntersections++;
                } else {
                    if (b1) nIntersections++;
                    if (b2) nIntersections++;
                }
                // und nun noch auf Ueberschneidungen checken
                if (i>0 && nInt0==nIntersections) {
                    if (aPt0.Y()==aPt1b.Y()) { // Horizontale Linie
                        if (aPt0.Y()>aBewareRect1.Top() && aPt0.Y()<aBewareRect1.Bottom() &&
                            ((aPt0.X()<=aBewareRect1.Left() && aPt1b.X()>=aBewareRect1.Right()) ||
                             (aPt1b.X()<=aBewareRect1.Left() && aPt0.X()>=aBewareRect1.Right()))) nIntersections++;
                        if (aPt0.Y()>aBewareRect2.Top() && aPt0.Y()<aBewareRect2.Bottom() &&
                            ((aPt0.X()<=aBewareRect2.Left() && aPt1b.X()>=aBewareRect2.Right()) ||
                             (aPt1b.X()<=aBewareRect2.Left() && aPt0.X()>=aBewareRect2.Right()))) nIntersections++;
                    } else { // Vertikale Linie
                        if (aPt0.X()>aBewareRect1.Left() && aPt0.X()<aBewareRect1.Right() &&
                            ((aPt0.Y()<=aBewareRect1.Top() && aPt1b.Y()>=aBewareRect1.Bottom()) ||
                             (aPt1b.Y()<=aBewareRect1.Top() && aPt0.Y()>=aBewareRect1.Bottom()))) nIntersections++;
                        if (aPt0.X()>aBewareRect2.Left() && aPt0.X()<aBewareRect2.Right() &&
                            ((aPt0.Y()<=aBewareRect2.Top() && aPt1b.Y()>=aBewareRect2.Bottom()) ||
                             (aPt1b.Y()<=aBewareRect2.Top() && aPt0.Y()>=aBewareRect2.Bottom()))) nIntersections++;
                    }
                }
                aPt0=aPt1b;
            }
            if (nPntAnz<=1) nIntersections++;
            nQual0=nQual;
            nQual+=(sal_uInt32)nIntersections*0x10000000;
            if (nQual<nQual0 || nIntersections>15) bOverflow=true;

            if (bOverflow || nQual==0xFFFFFFFF) nQual=0xFFFFFFFE;
            *pnQuality=nQual;
        }
        if (bInfo) { // nun die Linienversaetze auf aXP1 anwenden
            if (pInfo->nMiddleLine!=0xFFFF) {
                const sal_uInt16 nIdx = (sal_uInt16)pInfo->ImpGetPolyIdx(MIDDLELINE,aXP1.GetPointCount());
                if (pInfo->ImpIsHorzLine(MIDDLELINE,aXP1.GetPointCount())) {
                    aXP1[nIdx].Y()+=pInfo->aMiddleLine.getY();
                    aXP1[nIdx+1].Y()+=pInfo->aMiddleLine.getY();
                } else {
                    aXP1[nIdx].X()+=pInfo->aMiddleLine.getX();
                    aXP1[nIdx+1].X()+=pInfo->aMiddleLine.getX();
                }
            }
            if (pInfo->nObj1Lines>=2) {
                const sal_uInt16 nIdx = (sal_uInt16)pInfo->ImpGetPolyIdx(OBJ1LINE2,aXP1.GetPointCount());
                if (pInfo->ImpIsHorzLine(OBJ1LINE2,aXP1.GetPointCount())) {
                    aXP1[nIdx].Y()+=pInfo->aObj1Line2.getY();
                    aXP1[nIdx+1].Y()+=pInfo->aObj1Line2.getY();
                } else {
                    aXP1[nIdx].X()+=pInfo->aObj1Line2.getX();
                    aXP1[nIdx+1].X()+=pInfo->aObj1Line2.getX();
                }
            }
            if (pInfo->nObj1Lines>=3) {
                const sal_uInt16 nIdx = (sal_uInt16)pInfo->ImpGetPolyIdx(OBJ1LINE3,aXP1.GetPointCount());
                if (pInfo->ImpIsHorzLine(OBJ1LINE3,aXP1.GetPointCount())) {
                    aXP1[nIdx].Y()+=pInfo->aObj1Line3.getY();
                    aXP1[nIdx+1].Y()+=pInfo->aObj1Line3.getY();
                } else {
                    aXP1[nIdx].X()+=pInfo->aObj1Line3.getX();
                    aXP1[nIdx+1].X()+=pInfo->aObj1Line3.getX();
                }
            }
            if (pInfo->nObj2Lines>=2) {
                const sal_uInt16 nIdx = (sal_uInt16)pInfo->ImpGetPolyIdx(OBJ2LINE2,aXP1.GetPointCount());
                if (pInfo->ImpIsHorzLine(OBJ2LINE2,aXP1.GetPointCount())) {
                    aXP1[nIdx].Y()+=pInfo->aObj2Line2.getY();
                    aXP1[nIdx+1].Y()+=pInfo->aObj2Line2.getY();
                } else {
                    aXP1[nIdx].X()+=pInfo->aObj2Line2.getX();
                    aXP1[nIdx+1].X()+=pInfo->aObj2Line2.getX();
                }
            }
            if (pInfo->nObj2Lines>=3) {
                const sal_uInt16 nIdx = (sal_uInt16)pInfo->ImpGetPolyIdx(OBJ2LINE3,aXP1.GetPointCount());
                if (pInfo->ImpIsHorzLine(OBJ2LINE3,aXP1.GetPointCount())) {
                    aXP1[nIdx].Y()+=pInfo->aObj2Line3.getY();
                    aXP1[nIdx+1].Y()+=pInfo->aObj2Line3.getY();
                } else {
                    aXP1[nIdx].X()+=pInfo->aObj2Line3.getX();
                    aXP1[nIdx+1].X()+=pInfo->aObj2Line3.getX();
                }
            }
        }
        // Nun mache ich ggf. aus dem Verbinder eine Bezierkurve
        if (eKind==SDREDGE_BEZIER && nPntAnz>2) {
            Point* pPt1=&aXP1[0];
            Point* pPt2=&aXP1[1];
            Point* pPt3=&aXP1[nPntAnz-2];
            Point* pPt4=&aXP1[nPntAnz-1];
            long dx1=pPt2->X()-pPt1->X();
            long dy1=pPt2->Y()-pPt1->Y();
            long dx2=pPt3->X()-pPt4->X();
            long dy2=pPt3->Y()-pPt4->Y();
            if (cForm=='L') { // nPntAnz==3
                aXP1.SetFlags(1,XPOLY_CONTROL);
                Point aPt3(*pPt2);
                aXP1.Insert(2,aPt3,XPOLY_CONTROL);
                nPntAnz=aXP1.GetPointCount();
                pPt1=&aXP1[0];
                pPt2=&aXP1[1];
                pPt3=&aXP1[nPntAnz-2];
                pPt4=&aXP1[nPntAnz-1];
                pPt2->X()-=dx1/3;
                pPt2->Y()-=dy1/3;
                pPt3->X()-=dx2/3;
                pPt3->Y()-=dy2/3;
            } else if (nPntAnz>=4 && nPntAnz<=6) { // Z oder U oder ...
                // fuer Alle Anderen werden die Endpunkte der Ausgangslinien
                // erstmal zu Kontrollpunkten. Bei nPntAnz>4 ist also noch
                // Nacharbeit erforderlich!
                aXP1.SetFlags(1,XPOLY_CONTROL);
                aXP1.SetFlags(nPntAnz-2,XPOLY_CONTROL);
                // Distanz x1.5
                pPt2->X()+=dx1/2;
                pPt2->Y()+=dy1/2;
                pPt3->X()+=dx2/2;
                pPt3->Y()+=dy2/2;
                if (nPntAnz==5) {
                    // Vor und hinter dem Mittelpunkt jeweils
                    // noch einen Kontrollpunkt einfuegen
                    Point aCenter(aXP1[2]);
                    long dx1b=aCenter.X()-aXP1[1].X();
                    long dy1b=aCenter.Y()-aXP1[1].Y();
                    long dx2b=aCenter.X()-aXP1[3].X();
                    long dy2b=aCenter.Y()-aXP1[3].Y();
                    aXP1.Insert(2,aCenter,XPOLY_CONTROL);
                    aXP1.SetFlags(3,XPOLY_SYMMTR);
                    aXP1.Insert(4,aCenter,XPOLY_CONTROL);
                    aXP1[2].X()-=dx1b/2;
                    aXP1[2].Y()-=dy1b/2;
                    aXP1[3].X()-=(dx1b+dx2b)/4;
                    aXP1[3].Y()-=(dy1b+dy2b)/4;
                    aXP1[4].X()-=dx2b/2;
                    aXP1[4].Y()-=dy2b/2;
                }
                if (nPntAnz==6) {
                    Point aPt1b(aXP1[2]);
                    Point aPt2b(aXP1[3]);
                    aXP1.Insert(2,aPt1b,XPOLY_CONTROL);
                    aXP1.Insert(5,aPt2b,XPOLY_CONTROL);
                    long dx=aPt1b.X()-aPt2b.X();
                    long dy=aPt1b.Y()-aPt2b.Y();
                    aXP1[3].X()-=dx/2;
                    aXP1[3].Y()-=dy/2;
                    aXP1.SetFlags(3,XPOLY_SYMMTR);
                    //aXP1[4].X()+=dx/2;
                    //aXP1[4].Y()+=dy/2;
                    aXP1.Remove(4,1); // weil identisch mit aXP1[3]
                }
            }
        }
        return aXP1.getB2DPolygon();
    }
}

//////////////////////////////////////////////////////////////////////////////
// SdrEdgeObj

sdr::properties::BaseProperties* SdrEdgeObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::ConnectorProperties(*this);
}

sdr::contact::ViewContact* SdrEdgeObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrEdgeObj(*this);
}

sdr::glue::GluePointProvider* SdrEdgeObj::CreateObjectSpecificGluePointProvider()
{
    return new sdr::glue::SdrEdgeObjGluePointProvider(*this);
}

SdrEdgeObj::SdrEdgeObj(SdrModel& rSdrModel)
:   SdrTextObj(rSdrModel),
    mpCon1(new SdrObjConnection(this)),
    mpCon2(new SdrObjConnection(this)),
    maEdgeTrack(),
    mpEdgeInfo(new SdrEdgeInfoRec()),
    mbEdgeTrackUserDefined(false),
    mbSuppressDefaultConnect(false),
    mbBoundRectCalculationRunning(false),
    mbSuppressed(false)
{
    // give an initial position to the points so that the initial transformation
    // gets set
    mpCon2->SetPosition(basegfx::B2DPoint(0.0, 100.0));
}

SdrEdgeObj::~SdrEdgeObj()
{
}

void SdrEdgeObj::geometryChange()
{
    const basegfx::B2DHomMatrix aCurrent(
        SdrTextObj::getSdrObjectTransformation());
    const basegfx::B2DRange aBaseRange(
        mpCon1->GetPosition(),
        mpCon2->GetPosition());
    const basegfx::B2DHomMatrix aNew(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aBaseRange.getRange(),
            aBaseRange.getMinimum()));

    if(aNew != aCurrent)
    {
        maSdrObjectTransformation.setB2DHomMatrix(aNew);
        SetEdgeTrackDirty();
        SetChanged();

        // change implies interactive or listener-related change, so this
        // can no longer be a user-defined EdgeTrack
        mbEdgeTrackUserDefined = false;

        // Broadcasting nur, wenn auf der selben Page
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
    }
}

void SdrEdgeObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SdrEdgeObj* pSource = dynamic_cast< const SdrEdgeObj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrTextObj::copyDataFromSdrObject(rSource);

            // copy connection (without changing owner), but do not
            // keep object connection
            *mpCon1 = *pSource->mpCon1;
            mpCon1->SetConnectedSdrObject(0);

            *mpCon2 = *pSource->mpCon2;
            mpCon2->SetConnectedSdrObject(0);

            // copy local data
            maEdgeTrack = pSource->maEdgeTrack;
            *mpEdgeInfo = *pSource->mpEdgeInfo;
            mbEdgeTrackUserDefined = pSource->mbEdgeTrackUserDefined;
            mbSuppressDefaultConnect = pSource->mbSuppressDefaultConnect;

            // set internal bolleans to defaults
            mbSuppressed = mbBoundRectCalculationRunning = false;
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrEdgeObj* SdrEdgeObj::checkIfUsesListener(SfxListener& rCandidate)
{
    SdrObjConnection* pSdrObjConnection = dynamic_cast< SdrObjConnection* >(&rCandidate);

    if(pSdrObjConnection)
    {
        return pSdrObjConnection->getOwner();
    }

    return 0;
}

SdrObject* SdrEdgeObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrEdgeObj* pClone = new SdrEdgeObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

void SdrEdgeObj::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
    if(pOldPage != pNewPage)
    {
        // call parent
        SdrTextObj::handlePageChange(pOldPage, pNewPage);

        // check broadcasters; when we are not inserted we do not need broadcasters
        mpCon1->ownerPageChange();
        mpCon2->ownerPageChange();
    }
}

bool SdrEdgeObj::IsClosedObj() const
{
    return false;
}

void SdrEdgeObj::ImpSetAttrToEdgeInfo()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    const SdrEdgeKind eKind((static_cast< const SdrEdgeKindItem& >(rSet.Get(SDRATTR_EDGEKIND))).GetValue());
    sal_Int32 nVal1 = ((SdrEdgeLine1DeltaItem&)rSet.Get(SDRATTR_EDGELINE1DELTA)).GetValue();
    sal_Int32 nVal2 = ((SdrEdgeLine2DeltaItem&)rSet.Get(SDRATTR_EDGELINE2DELTA)).GetValue();
    sal_Int32 nVal3 = ((SdrEdgeLine3DeltaItem&)rSet.Get(SDRATTR_EDGELINE3DELTA)).GetValue();

    if(eKind == SDREDGE_ORTHOLINES || eKind == SDREDGE_BEZIER)
    {
        sal_Int32 nVals[3] = { nVal1, nVal2, nVal3 };
        sal_uInt16 n = 0;

        // force recalc EdgeTrack, we need evtl. the EdgeTrack PointCount (see below)
        ImpRecalcEdgeTrack();

        if(mpEdgeInfo->nObj1Lines >= 2 && n < 3)
        {
            mpEdgeInfo->ImpSetLineVersatz(OBJ1LINE2, maEdgeTrack.count(), nVals[n]);
            n++;
        }

        if(mpEdgeInfo->nObj1Lines >= 3 && n < 3)
        {
            mpEdgeInfo->ImpSetLineVersatz(OBJ1LINE3, maEdgeTrack.count(), nVals[n]);
            n++;
        }

        if(mpEdgeInfo->nMiddleLine != 0xFFFF && n < 3)
        {
            mpEdgeInfo->ImpSetLineVersatz(MIDDLELINE, maEdgeTrack.count(), nVals[n]);
            n++;
        }

        if(mpEdgeInfo->nObj2Lines >= 3 && n < 3)
        {
            mpEdgeInfo->ImpSetLineVersatz(OBJ2LINE3, maEdgeTrack.count(), nVals[n]);
            n++;
        }

        if(mpEdgeInfo->nObj2Lines >= 2 && n < 3)
        {
            mpEdgeInfo->ImpSetLineVersatz(OBJ2LINE2, maEdgeTrack.count(), nVals[n]);
            n++;
        }
    }
    else if(eKind == SDREDGE_THREELINES)
    {
        bool bHor1 = mpEdgeInfo->nAngle1 == 0 || mpEdgeInfo->nAngle1 == 18000;
        bool bHor2 = mpEdgeInfo->nAngle2 == 0 || mpEdgeInfo->nAngle2 == 18000;

        if(bHor1)
        {
            mpEdgeInfo->aObj1Line2.setX(nVal1);
        }
        else
        {
            mpEdgeInfo->aObj1Line2.setY(nVal1);
        }

        if(bHor2)
        {
            mpEdgeInfo->aObj2Line2.setX(nVal2);
        }
        else
        {
            mpEdgeInfo->aObj2Line2.setY(nVal2);
        }
    }

    // if values have changed, a different EdgeTrack needs to be created
    SetEdgeTrackDirty();
}

void SdrEdgeObj::ImpSetEdgeInfoToAttr()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    const SdrEdgeKind eKind((static_cast< const SdrEdgeKindItem& >(rSet.Get(SDRATTR_EDGEKIND))).GetValue());
    sal_Int32 nValAnz = ((SfxUInt16Item&)rSet.Get(SDRATTR_EDGELINEDELTAANZ)).GetValue();
    sal_Int32 nVal1 = ((SdrEdgeLine1DeltaItem&)rSet.Get(SDRATTR_EDGELINE1DELTA)).GetValue();
    sal_Int32 nVal2 = ((SdrEdgeLine2DeltaItem&)rSet.Get(SDRATTR_EDGELINE2DELTA)).GetValue();
    sal_Int32 nVal3 = ((SdrEdgeLine3DeltaItem&)rSet.Get(SDRATTR_EDGELINE3DELTA)).GetValue();
    sal_Int32 nVals[3] = { nVal1, nVal2, nVal3 };
    sal_uInt16 n = 0;

    // force recalc EdgeTrack, we need evtl. changes to EdgeInfo before
    // we write it back to the SfxItemSet
    ImpRecalcEdgeTrack();

    if(eKind == SDREDGE_ORTHOLINES || eKind == SDREDGE_BEZIER)
    {
        if(mpEdgeInfo->nObj1Lines >= 2 && n < 3)
        {
            nVals[n] = mpEdgeInfo->ImpGetLineVersatz(OBJ1LINE2, maEdgeTrack.count());
            n++;
        }

        if(mpEdgeInfo->nObj1Lines >= 3 && n < 3)
        {
            nVals[n] = mpEdgeInfo->ImpGetLineVersatz(OBJ1LINE3, maEdgeTrack.count());
            n++;
        }

        if(mpEdgeInfo->nMiddleLine != 0xFFFF && n < 3)
        {
            nVals[n] = mpEdgeInfo->ImpGetLineVersatz(MIDDLELINE, maEdgeTrack.count());
            n++;
        }

        if(mpEdgeInfo->nObj2Lines >= 3 && n < 3)
        {
            nVals[n] = mpEdgeInfo->ImpGetLineVersatz(OBJ2LINE3, maEdgeTrack.count());
            n++;
        }

        if(mpEdgeInfo->nObj2Lines >= 2 && n < 3)
        {
            nVals[n] = mpEdgeInfo->ImpGetLineVersatz(OBJ2LINE2, maEdgeTrack.count());
            n++;
        }
    }
    else if(eKind == SDREDGE_THREELINES)
    {
        bool bHor1 = mpEdgeInfo->nAngle1 == 0 || mpEdgeInfo->nAngle1 == 18000;
        bool bHor2 = mpEdgeInfo->nAngle2 == 0 || mpEdgeInfo->nAngle2 == 18000;

        n = 2;
        nVals[0] = bHor1 ? mpEdgeInfo->aObj1Line2.getX() : mpEdgeInfo->aObj1Line2.getY();
        nVals[1] = bHor2 ? mpEdgeInfo->aObj2Line2.getX() : mpEdgeInfo->aObj2Line2.getY();
    }

    if(n != nValAnz || nVals[0] != nVal1 || nVals[1] != nVal2 || nVals[2] != nVal3)
    {
        // #75371# Here no more notifying is necessary, just local changes are OK.
        if(n != nValAnz)
        {
            GetProperties().SetObjectItemDirect(SfxUInt16Item(SDRATTR_EDGELINEDELTAANZ, n));
        }

        if(nVals[0] != nVal1)
        {
            GetProperties().SetObjectItemDirect(SdrEdgeLine1DeltaItem(nVals[0]));
        }

        if(nVals[1] != nVal2)
        {
            GetProperties().SetObjectItemDirect(SdrEdgeLine2DeltaItem(nVals[1]));
        }

        if(nVals[2] != nVal3)
        {
            GetProperties().SetObjectItemDirect(SdrEdgeLine3DeltaItem(nVals[2]));
        }

        if(n < 3)
        {
            GetProperties().ClearObjectItemDirect(SDRATTR_EDGELINE3DELTA);
        }

        if(n < 2)
        {
            GetProperties().ClearObjectItemDirect(SDRATTR_EDGELINE2DELTA);
        }

        if(n < 1)
        {
            GetProperties().ClearObjectItemDirect(SDRATTR_EDGELINE1DELTA);
        }
    }
}

void SdrEdgeObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    // #54102# allow rotation, mirror and shear
    rInfo.mbRotateFreeAllowed = true;
    rInfo.mbRotate90Allowed = true;
    rInfo.mbMirrorFreeAllowed = true;
    rInfo.mbMirror45Allowed = true;
    rInfo.mbMirror90Allowed = true;
    rInfo.mbTransparenceAllowed = false;
    rInfo.mbGradientAllowed = false;
    rInfo.mbShearAllowed = true;
    rInfo.mbEdgeRadiusAllowed = false;
    const bool bCanConv(!HasText() || ImpCanConvTextToCurve());
    rInfo.mbCanConvToPath = bCanConv;
    rInfo.mbCanConvToPoly = bCanConv;
    rInfo.mbCanConvToContour = (rInfo.mbCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrEdgeObj::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_EDGE);
}

void SdrEdgeObj::SetEdgeTrackDirty()
{
    if(!mbEdgeTrackUserDefined)
    {
        maEdgeTrack.clear();
    }
}

void SdrEdgeObj::ConnectToSdrObject(bool bTail, SdrObject* pObj)
{
    SdrObjConnection& rCon(bTail ? *mpCon1 : *mpCon2);

    rCon.SetConnectedSdrObject(pObj);
}

SdrObject* SdrEdgeObj::GetSdrObjectConnection(bool bTail) const
{
    const SdrObjConnection& rCon(bTail ? *mpCon1 : *mpCon2);
    SdrObject* pObj = rCon.GetConnectedSdrObject();

    if(!pObj)
    {
        return 0;
    }

    if(!pObj->IsObjectInserted())
    {
        return 0;
    }

    const SdrPage* pOwningPage = getSdrPageFromSdrObject();

    if(pOwningPage && pObj->getSdrPageFromSdrObject() != pOwningPage)
    {
        return 0;
    }

    return pObj;
}

bool SdrEdgeObj::CheckSdrObjectConnection(bool bTail) const
{
    const SdrObject* pConnectedNode = GetSdrObjectConnection(bTail);

    if(!pConnectedNode)
    {
        // not connected to any object -> no valid connection
        return false;
    }

    const SdrObjConnection& rCon(bTail ? *mpCon1 : *mpCon2);
    const sdr::glue::GluePointProvider& rProvider = pConnectedNode->GetGluePointProvider();

    if(rCon.IsBestConnection() && (rProvider.hasUserGluePoints() || rProvider.getAutoGluePointCount()))
    {
        // best connection is always valid when there are any GluePoints
        return true;
    }

    const basegfx::B2DPoint aCurrentPosition(rCon.GetPosition());

    if(rCon.IsAutoVertex())
    {
        // AutoGluePoints
        const sal_uInt32 nConAnz(rProvider.getAutoGluePointCount());

        for(sal_uInt32 i(0); i < nConAnz; i++)
        {
            const sdr::glue::GluePoint aPt(rProvider.getAutoGluePointByIndex(i));
            const basegfx::B2DPoint aGluePos(pConnectedNode->getSdrObjectTransformation() * aPt.getUnitPosition());

            if(aCurrentPosition.equal(aGluePos))
            {
                return true;
            }
        }
    }
    else
    {
        // User-Defined (or CustomShape) GluePoints
        const sdr::glue::GluePointVector aGluePointVector(rProvider.getUserGluePointVector());
        const sal_uInt32 nConAnz(aGluePointVector.size());

        for(sal_uInt32 i(0); i < nConAnz; i++)
        {
            const sdr::glue::GluePoint* pCandidate = aGluePointVector[i];

            if(pCandidate)
            {
                const basegfx::B2DPoint aGluePos(pConnectedNode->getSdrObjectTransformation() * pCandidate->getUnitPosition());

                if(aCurrentPosition.equal(aGluePos))
                {
                    return true;
                }
            }
            else
            {
                OSL_ENSURE(false, "Got sdr::glue::PointVector with emty entries (!)");
            }
        }
    }

    return false;
}

void SdrEdgeObj::SetConnectorId(bool bTail, sal_uInt32 nId)
{
    SdrObjConnection& rCon(bTail ? *mpCon1 : *mpCon2);

    rCon.SetConnectorID(nId);
}

sal_uInt32 SdrEdgeObj::GetConnectorId(bool bTail) const
{
    const SdrObjConnection& rCon(bTail ? *mpCon1 : *mpCon2);

    return rCon.GetConnectorId();
}

void SdrEdgeObj::ImpRecalcEdgeTrack()
{
    // #120437# if bEdgeTrackUserDefined, do not recalculate
    if(mbEdgeTrackUserDefined)
    {
        return;
    }

    // not dirty at all
    if(maEdgeTrack.count())
    {
        return;
    }

    // #120437# also not when model locked during import, but remember
    if(getSdrModelFromSdrObject().isLocked())
    {
        mbSuppressed = true;
        return;
    }

    // #110649#
    if(IsBoundRectCalculationRunning())
    {
        // this object is involved into another ImpRecalcEdgeTrack() call
        // from another SdrEdgeObj. Do not calculate again to avoid loop.
        // Also, do not change mbEdgeTrackDirty so that it gets recalculated
        // later at the first non-looping call.
    }
    else
    {
        if(mbSuppressed)
        {
            // #123048# If layouting was ever suppressed, it needs to be done once
            // and the attr need to be set at EdgeInfo, else these attr *will be lost*
            // in the following call to ImpSetEdgeInfoToAttr() sice they were never
            // set before (!)
            maEdgeTrack = ImpCalcEdgeTrack(*mpCon1, *mpCon2, mpEdgeInfo, 0, 0);
            ImpSetAttrToEdgeInfo();
            mbSuppressed = false;
        }

        // To not run in a depth loop, use a coloring algorythm on
        // SdrEdgeObj BoundRect calculations
        mbBoundRectCalculationRunning = true;

        // remember current SdrEdgeInfoRec and new polygon
        const SdrEdgeInfoRec aPreserved(*mpEdgeInfo);
        const basegfx::B2DPolygon aNew(ImpCalcEdgeTrack(*mpCon1, *mpCon2, mpEdgeInfo, 0, 0));
        bool bBroadcastChange(false);

        if(aNew != maEdgeTrack)
        {
            maEdgeTrack = aNew;

            // if connections are in 'BestConnection' mode, update their oposition to the
            // computed one, just to make sure when that data is fetched from the connection
            // itself that it is correct
            if(maEdgeTrack.count())
            {
                const basegfx::B2DPoint aStart(maEdgeTrack.getB2DPoint(0));
                const basegfx::B2DPoint aEnd(maEdgeTrack.getB2DPoint(maEdgeTrack.count() - 1));

                if(mpCon1->IsBestConnection())
                {
                    mpCon1->adaptBestConnectionPosition(aStart);
                }

                if(mpCon2->IsBestConnection())
                {
                    mpCon2->adaptBestConnectionPosition(aEnd);
                }
            }

            bBroadcastChange = true;
        }

        if(aPreserved != *mpEdgeInfo)
        {
            ImpSetEdgeInfoToAttr();
            bBroadcastChange = true;
        }

        if(bBroadcastChange)
        {
            // use local scope to trigger locally
            const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        }

        // #110649#
        mbBoundRectCalculationRunning = false;
    }
}

basegfx::B2DPolygon SdrEdgeObj::ImpCalcEdgeTrack(
    SdrObjConnection& rCon1,
    SdrObjConnection& rCon2,
    SdrEdgeInfoRec* pInfo,
    const basegfx::B2DHomMatrix* pTransA,
    const basegfx::B2DHomMatrix* pTransB) const
{
    basegfx::B2DPolygon aRetval;
    basegfx::B2DPoint aPt1(rCon1.GetPosition());
    basegfx::B2DPoint aPt2(rCon2.GetPosition());
    const basegfx::B2DRange aBaseRange(aPt1, aPt2);
    sdr::glue::GluePoint aGP1,aGP2;
    sal_uInt16 nEsc1(SDRESC_ALL);
    sal_uInt16 nEsc2(SDRESC_ALL);
    basegfx::B2DRange aBoundRange1;
    basegfx::B2DRange aBoundRange2;
    basegfx::B2DRange aBewareRange1;
    basegfx::B2DRange aBewareRange2;
    const SdrPage* pOwningPage = getSdrPageFromSdrObject();
    const bool bCon1(rCon1.GetConnectedSdrObject() && (!pOwningPage || rCon1.GetConnectedSdrObject()->getSdrPageFromSdrObject() == pOwningPage));
    const bool bCon2(rCon2.GetConnectedSdrObject() && (!pOwningPage || rCon2.GetConnectedSdrObject()->getSdrPageFromSdrObject() == pOwningPage));
    const SfxItemSet& rSet = GetObjectItemSet();

    if(bCon1)
    {
        if(rCon1.GetConnectedSdrObject() == this)
        {
            aBoundRange1 = aBaseRange;
        }
        else
        {
            aBoundRange1 = rCon1.GetConnectedSdrObject()->getObjectRange(0);
        }

        const sal_Int32 nH(((SdrEdgeNode1HorzDistItem&)rSet.Get(SDRATTR_EDGENODE1HORZDIST)).GetValue());
        const sal_Int32 nV(((SdrEdgeNode1VertDistItem&)rSet.Get(SDRATTR_EDGENODE1VERTDIST)).GetValue());

        aBewareRange1 = basegfx::B2DRange(
            aBoundRange1.getMinX() - nH, aBoundRange1.getMinY() - nV,
            aBoundRange1.getMaxX() + nH, aBoundRange1.getMaxY() + nV);
    }
    else
    {
        aBewareRange1 = aBoundRange1 = basegfx::B2DRange(aPt1);
    }

    if(pTransA)
    {
        aBoundRange1.transform(*pTransA);
        aBewareRange1.transform(*pTransA);
        aPt1 = *pTransA * aPt1;
    }

    if(bCon2)
    {
        if(rCon2.GetConnectedSdrObject() == this)
        {
            aBoundRange2 = aBaseRange;
        }
        else
        {
            aBoundRange2 = rCon2.GetConnectedSdrObject()->getObjectRange(0);
        }

        const sal_Int32 nH(((SdrEdgeNode2HorzDistItem&)rSet.Get(SDRATTR_EDGENODE2HORZDIST)).GetValue());
        const sal_Int32 nV(((SdrEdgeNode2VertDistItem&)rSet.Get(SDRATTR_EDGENODE2VERTDIST)).GetValue());

        aBewareRange2 = basegfx::B2DRange(
            aBoundRange2.getMinX() - nH, aBoundRange2.getMinY() - nV,
            aBoundRange2.getMaxX() + nH, aBoundRange2.getMaxY() + nV);
    }
    else
    {
        aBewareRange2 = aBoundRange2 = basegfx::B2DRange(aPt2);
    }

    if(pTransB)
    {
        aBoundRange2.transform(*pTransB);
        aBewareRange2.transform(*pTransB);
        aPt2 = *pTransB * aPt2;
    }

    sal_uInt32 nBestQual=0xFFFFFFFF;
    SdrEdgeInfoRec aBestInfo;
    const bool bAuto1(bCon1 && rCon1.IsBestConnection());
    const bool bAuto2(bCon2 && rCon2.IsBestConnection());

    if(bAuto1)
    {
        rCon1.setAutoVertex(true);
    }

    if(bAuto2)
    {
        rCon2.setAutoVertex(true);
    }

    sal_uInt16 nBestAuto1(0);
    sal_uInt16 nBestAuto2(0);
    sal_uInt16 nAnz1(bAuto1 ? 4 : 1);
    sal_uInt16 nAnz2(bAuto2 ? 4 : 1);

    // prepare vars for old interface stuff
    const Rectangle aBoundRect1(
        basegfx::fround(aBoundRange1.getMinX()), basegfx::fround(aBoundRange1.getMinY()),
        basegfx::fround(aBoundRange1.getMaxX()), basegfx::fround(aBoundRange1.getMaxY()));
    const Rectangle aBoundRect2(
        basegfx::fround(aBoundRange2.getMinX()), basegfx::fround(aBoundRange2.getMinY()),
        basegfx::fround(aBoundRange2.getMaxX()), basegfx::fround(aBoundRange2.getMaxY()));
    const Rectangle aBewareRect1(
        basegfx::fround(aBewareRange1.getMinX()), basegfx::fround(aBewareRange1.getMinY()),
        basegfx::fround(aBewareRange1.getMaxX()), basegfx::fround(aBewareRange1.getMaxY()));
    const Rectangle aBewareRect2(
        basegfx::fround(aBewareRange2.getMinX()), basegfx::fround(aBewareRange2.getMinY()),
        basegfx::fround(aBewareRange2.getMaxX()), basegfx::fround(aBewareRange2.getMaxY()));

    for(sal_uInt16 nNum1(0); nNum1 < nAnz1; nNum1++)
    {
        if(bAuto1)
        {
            rCon1.SetConnectorID(nNum1);
        }

        if(bCon1 && rCon1.TakeGluePoint(aGP1))
        {
            aPt1 = rCon1.GetConnectedSdrObject()->getSdrObjectTransformation() * aGP1.getUnitPosition();
            nEsc1 = aGP1.getEscapeDirections();

            if(pTransA)
            {
                aPt1 = *pTransA * aPt1;
            }

            if(sdr::glue::GluePoint::ESCAPE_DIRECTION_SMART == nEsc1)
            {
                nEsc1 = impOldCalcEscAngle(aBoundRange1, aPt1);
            }
        }

        const Point aPoint1(basegfx::fround(aPt1.getX()), basegfx::fround(aPt1.getY()));

        for(sal_uInt16 nNum2(0); nNum2 < nAnz2; nNum2++)
        {
            if(bAuto2)
            {
                rCon2.SetConnectorID(nNum2);
            }

            if(bCon2 && rCon2.TakeGluePoint(aGP2))
            {
                aPt2 = rCon2.GetConnectedSdrObject()->getSdrObjectTransformation() * aGP2.getUnitPosition();
                nEsc2 = aGP2.getEscapeDirections();

                if(pTransB)
                {
                    aPt2 = *pTransB * aPt2;
                }

                if(sdr::glue::GluePoint::ESCAPE_DIRECTION_SMART == nEsc2)
                {
                    nEsc2 = impOldCalcEscAngle(aBoundRange2, aPt2);
                }
            }

            const Point aPoint2(basegfx::fround(aPt2.getX()), basegfx::fround(aPt2.getY()));

            for(long nA1(0); nA1 < 36000; nA1 += 9000)
            {
                const sal_uInt16 nE1(!nA1 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT : 9000 == nA1 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP : 18000 == nA1 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT : 27000 == nA1 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM : 0);

                for(long nA2(0); nA2 < 36000; nA2 += 9000)
                {
                    const sal_uInt16 nE2(!nA2 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT : 9000 == nA2 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP : 18000 == nA2 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT : 27000 == nA2 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM : 0);

                    if((nEsc1 & nE1) && (nEsc2 & nE2))
                    {
                        sal_uInt32 nQual(0);
                        SdrEdgeInfoRec aInfo;

                        if(pInfo)
                        {
                            aInfo = *pInfo;
                        }

                        basegfx::B2DPolygon aXP(
                            impOldCalcEdgeTrack(
                                aPoint1, nA1, aBoundRect1, aBewareRect1,
                                aPoint2, nA2, aBoundRect2, aBewareRect2,
                                static_cast< const SdrEdgeKindItem& >(GetObjectItem(SDRATTR_EDGEKIND)).GetValue(),
                                &nQual,
                                &aInfo));

                        if(aXP.isClosed())
                        {
                            // closed because correction (closeWithGeometryChange) was done when
                            // converted from Polygon class to B2DPolygon class using Polygon::getB2DPolygon()
                            // which by default corrects using basegfx::tools::checkClosed. This is
                            // not wanted in this case; e.g. when starting to construct a connector.
                            // I will correct it here since rewriting impOldCalcEdgeTrack and avoiding
                            // usage of old XPolygon/Polygon would be nice, but is dangerous and nearly
                            // impossible
                            basegfx::tools::openWithGeometryChange(aXP);
                        }

                        if(nQual < nBestQual)
                        {
                            aRetval = aXP;
                            nBestQual = nQual;
                            aBestInfo = aInfo;
                            nBestAuto1 = nNum1;
                            nBestAuto2 = nNum2;
                        }
                    }
                }
            }
        }
    }

    if(bAuto1)
    {
        rCon1.SetConnectorID(nBestAuto1);
    }

    if(bAuto2)
    {
        rCon2.SetConnectorID(nBestAuto2);
    }

    if(pInfo)
    {
        *pInfo = aBestInfo;
    }

    return aRetval;
}

void SdrEdgeObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulEDGE);

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SdrEdgeObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralEDGE);
}

basegfx::B2DPolyPolygon SdrEdgeObj::TakeXorPoly() const
{
    return basegfx::B2DPolyPolygon(GetEdgeTrackPath());
}

void SdrEdgeObj::SetEdgeTrackPath(const basegfx::B2DPolygon& rPoly)
{
    if(rPoly.count() > 1)
    {
        maEdgeTrack = rPoly;
        mbEdgeTrackUserDefined = true;

        mpCon1->SetConnectedSdrObject(0);
        mpCon1->adaptBestConnectionPosition(maEdgeTrack.getB2DPoint(0));

        mpCon2->SetConnectedSdrObject(0);
        mpCon2->adaptBestConnectionPosition(maEdgeTrack.getB2DPoint(maEdgeTrack.count() - 1));
    }
    else
    {
        mbEdgeTrackUserDefined = false;
    }
}

basegfx::B2DPolygon SdrEdgeObj::GetEdgeTrackPath() const
{
    const_cast< SdrEdgeObj* >(this)->ImpRecalcEdgeTrack();

    return maEdgeTrack;
}

void SdrEdgeObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    // due to old object re-use ol methods
    const sal_uInt32 nCount(impOldGetHdlCount());

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        impOldGetHdl(rHdlList, a);
    }
}

bool SdrEdgeObj::checkHorizontalDrag(SdrEdgeLineCode eLineCode, bool bObjHdlTwo) const
{
    const SdrEdgeKind eEdgeKind((static_cast< const SdrEdgeKindItem& >(GetObjectItem(SDRATTR_EDGEKIND))).GetValue());

    if(SDREDGE_ORTHOLINES == eEdgeKind || SDREDGE_BEZIER == eEdgeKind)
    {
        const_cast< SdrEdgeObj* >(this)->ImpRecalcEdgeTrack();

        return !mpEdgeInfo->ImpIsHorzLine(eLineCode, maEdgeTrack.count());
    }
    else if(SDREDGE_THREELINES == eEdgeKind)
    {
        const sal_Int32 nWink(bObjHdlTwo ? mpEdgeInfo->nAngle1 : mpEdgeInfo->nAngle2);

        if(!nWink || 18000 == nWink)
        {
            return true;
        }
    }

    return false;
}

sal_uInt32 SdrEdgeObj::impOldGetHdlCount() const
{
    const_cast< SdrEdgeObj* >(this)->ImpRecalcEdgeTrack();
    const SdrEdgeKind eKind((static_cast< const SdrEdgeKindItem& >(GetObjectItem(SDRATTR_EDGEKIND))).GetValue());
    const sal_uInt32 nPntAnz(maEdgeTrack.count());
    sal_uInt32 nHdlAnz(0);

    if(nPntAnz)
    {
        nHdlAnz = 2;

        if((SDREDGE_ORTHOLINES == eKind || SDREDGE_BEZIER == eKind) &&  4 <= nPntAnz)
        {
            const sal_uInt32 nO1(mpEdgeInfo->nObj1Lines > 0 ? mpEdgeInfo->nObj1Lines - 1 : 0);
            const sal_uInt32 nO2(mpEdgeInfo->nObj2Lines > 0 ? mpEdgeInfo->nObj2Lines - 1 : 0);
            const sal_uInt32 nM(mpEdgeInfo->nMiddleLine != 0xFFFF ? 1 : 0);

            nHdlAnz += nO1 + nO2 + nM;
        }
        else if(SDREDGE_THREELINES == eKind && 4 == nPntAnz)
        {
            if(GetSdrObjectConnection(true))
            {
                nHdlAnz++;
            }

            if(GetSdrObjectConnection(false))
            {
                nHdlAnz++;
            }
        }
    }

    return nHdlAnz;
}

SdrHdl* SdrEdgeObj::impOldGetHdl(SdrHdlList& rHdlList, sal_uInt32 nHdlNum) const
{
    SdrHdl* pHdl = 0;
    const_cast< SdrEdgeObj* >(this)->ImpRecalcEdgeTrack();
    const sal_uInt32 nPntAnz(maEdgeTrack.count());

    if(nPntAnz)
    {
        if(!nHdlNum)
        {
            pHdl = new ImpEdgeHdl(rHdlList, *this, HDL_POLY, mpCon1->GetPosition());

            if(mpCon1->GetConnectedSdrObject() && mpCon1->IsBestConnection())
            {
                pHdl->Set1PixMore(true);
            }
        }
        else if(1 == nHdlNum)
        {
            pHdl = new ImpEdgeHdl(rHdlList, *this, HDL_POLY, mpCon2->GetPosition());

            if(mpCon2->GetConnectedSdrObject() && mpCon2->IsBestConnection())
            {
                pHdl->Set1PixMore(true);
            }
        }
        else
        {
            const SdrEdgeKind eKind((static_cast< const SdrEdgeKindItem& >(GetObjectItem(SDRATTR_EDGEKIND))).GetValue());

            if(SDREDGE_ORTHOLINES == eKind || SDREDGE_BEZIER == eKind)
            {
                const sal_uInt32 nO1(mpEdgeInfo->nObj1Lines > 0 ? mpEdgeInfo->nObj1Lines - 1 : 0);
                const sal_uInt32 nO2(mpEdgeInfo->nObj2Lines > 0 ? mpEdgeInfo->nObj2Lines - 1 : 0);
                const sal_uInt32 nM(mpEdgeInfo->nMiddleLine != 0xFFFF ? 1 : 0);
                sal_uInt32 nNum(nHdlNum - 2);
                sal_Int32 nPt(0);
                SdrEdgeLineCode aSdrEdgeLineCode(OBJ1LINE2);

                if(nNum < nO1)
                {
                    nPt = nNum + 1;

                    if(0 == nNum)
                    {
                        aSdrEdgeLineCode = OBJ1LINE2;
                    }

                    if(1 == nNum)
                    {
                        aSdrEdgeLineCode = OBJ1LINE3;
                    }
                }
                else
                {
                    nNum = nNum - nO1;

                    if(nNum < nO2)
                    {
                        nPt = nPntAnz - 3 - nNum;

                        if(0 == nNum)
                        {
                            aSdrEdgeLineCode = OBJ2LINE2;
                        }

                        if(1 == nNum)
                        {
                            aSdrEdgeLineCode = OBJ2LINE3;
                        }
                    }
                    else
                    {
                        nNum = nNum - nO2;

                        if(nNum < nM)
                        {
                            nPt = mpEdgeInfo->nMiddleLine;
                            aSdrEdgeLineCode = MIDDLELINE;
                        }
                    }
                }

                if(nPt > 0)
                {
                    ImpEdgeHdl* pImpEdgeHdl = new ImpEdgeHdl(rHdlList, *this, HDL_POLY, (maEdgeTrack.getB2DPoint(nPt) + maEdgeTrack.getB2DPoint(nPt + 1)) * 0.5);
                    pHdl = pImpEdgeHdl;
                    pImpEdgeHdl->SetLineCode(aSdrEdgeLineCode);
                }
            }
            else if(SDREDGE_THREELINES == eKind)
            {
                sal_uInt32 nNum(nHdlNum);

                if(!GetSdrObjectConnection(true))
                {
                    nNum++;
                }

                ImpEdgeHdl* pImpEdgeHdl = new ImpEdgeHdl(rHdlList, *this, HDL_POLY, maEdgeTrack.getB2DPoint(nNum - 1));
                pHdl = pImpEdgeHdl;

                if(2 == nNum)
                {
                    pImpEdgeHdl->SetLineCode(OBJ1LINE2);
                }

                if(3 == nNum)
                {
                    pImpEdgeHdl->SetLineCode(OBJ2LINE2);
                }
            }
        }

        if(pHdl)
        {
            pHdl->SetPointNum(nHdlNum);
        }
    }

    return pHdl;
}

bool SdrEdgeObj::hasSpecialDrag() const
{
    return true;
}

SdrObject* SdrEdgeObj::getFullDragClone() const
{
    // use Clone operator
    SdrEdgeObj* pRetval = static_cast< SdrEdgeObj* >(CloneSdrObject());

    // copy connections for clone, clone itself does not do this for good reason
    pRetval->ConnectToSdrObject(true, GetSdrObjectConnection(true));
    pRetval->ConnectToSdrObject(false, GetSdrObjectConnection(false));

    return pRetval;
}

bool SdrEdgeObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    if(!rDrag.GetActiveHdl())
        return false;

    rDrag.SetEndDragChangesAttributes(true);

    if(rDrag.GetActiveHdl()->GetPointNum() < 2)
    {
        rDrag.SetNoSnap(true);
    }

    return true;
}

bool SdrEdgeObj::applySpecialDrag(SdrDragStat& rDragStat)
{
    const SdrEdgeObj* pOriginalEdge = dynamic_cast< const SdrEdgeObj* >(rDragStat.GetActiveHdl()->GetObj());
    const bool bOriginalEdgeModified(pOriginalEdge == this);

    // not user defined if dragging
    mbEdgeTrackUserDefined = false;

    if(!bOriginalEdgeModified && pOriginalEdge)
    {
        // copy connections when clone is modified. This is needed because
        // as preparation to this modification the data from the original object
        // was copied to the clone using the operator=. As can be seen there,
        // that operator does not copy the connections (for good reason)
        ConnectToSdrObject(true, pOriginalEdge->mpCon1->GetConnectedSdrObject());
        ConnectToSdrObject(false, pOriginalEdge->mpCon2->GetConnectedSdrObject());
    }

    if(rDragStat.GetActiveHdl()->GetPointNum() < 2)
    {
        // start or end point connector drag
        const bool bDragA(0 == rDragStat.GetActiveHdl()->GetPointNum());
        SdrView& rSdrView = rDragStat.GetSdrViewFromSdrDragStat();

        if(rSdrView.GetSdrPageView())
        {
            SdrObjConnection& rDraggedOne(bDragA ? *mpCon1 : *mpCon2);
            sal_uInt32 nID(0);
            bool bBest(false);
            bool bAuto(false);

            // look for new connection
            SdrObject* pNewContact = rSdrView.FindConnector(rDragStat.GetNow(), nID, bBest, bAuto, pOriginalEdge);

            // set parameters
            rDraggedOne.SetConnectedSdrObject(0);
            rDraggedOne.SetPosition(rDragStat.GetNow());
            rDraggedOne.SetConnectorID(nID);
            rDraggedOne.setBestConnection(bBest);
            rDraggedOne.setAutoVertex(bAuto);
            rDraggedOne.SetConnectedSdrObject(pNewContact);

            if(!bOriginalEdgeModified)
            {
                // show IA helper, but only do this during IA, so not when the original
                // Edge gets modified in the last call
                rDragStat.GetSdrViewFromSdrDragStat().SetConnectMarker(rDraggedOne.GetConnectedSdrObject());
            }
        }

        // reset edge info's offsets, this is a end point drag
        if(mpEdgeInfo->ImpUsesUserDistances())
        {
            mpEdgeInfo->ImpResetUserDistances();
        }
    }
    else
    {
        // control point connector drag
        const ImpEdgeHdl* pEdgeHdl = dynamic_cast< const ImpEdgeHdl* >(rDragStat.GetActiveHdl());

        if(pEdgeHdl)
        {
            const SdrEdgeLineCode eLineCode = pEdgeHdl->GetLineCode();
            const basegfx::B2DPoint aDist(rDragStat.GetNow() - rDragStat.GetStart());
            sal_Int32 nDist(basegfx::fround(pEdgeHdl->IsHorzDrag() ? aDist.getX() : aDist.getY()));

            ImpRecalcEdgeTrack();
            nDist += mpEdgeInfo->ImpGetLineVersatz(eLineCode, maEdgeTrack.count());
            mpEdgeInfo->ImpSetLineVersatz(eLineCode, maEdgeTrack.count(), nDist);

            SetEdgeTrackDirty();
            SetChanged();
        }
    }

    // save EdgeInfos and mark object as user modified
    ImpSetEdgeInfoToAttr();

    if(bOriginalEdgeModified)
    {
        // hide connect marker helper again when original gets changed.
        // This happens at the end of the interaction
        rDragStat.GetSdrViewFromSdrDragStat().HideConnectMarker();

        // modified is needed at the end of interaction, too
        SetEdgeTrackDirty();
        SetChanged();
    }

    return true;
}

String SdrEdgeObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    const bool bCreateComment(this == rDrag.GetSdrViewFromSdrDragStat().GetCreateObj());

    if(bCreateComment)
    {
        return String();
    }
    else
    {
        XubString aStr;
        TakeMarkedDescriptionString(STR_DragEdgeTail, aStr);

        return aStr;
    }
}

basegfx::B2DPolygon SdrEdgeObj::CreateConnectorOverlay(
    const basegfx::B2DHomMatrix& rCurrentTransformation,
    bool bTail1,
    bool bTail2,
    bool bDetail) const
{
    basegfx::B2DPolygon aResult;

    if(bDetail)
    {
        // to calculate connector overlay in interactive mode when a SdrObject
        // is modified to which we are connected; this is not optimal and requires
        // two extra transformations to ImpCalcEdgeTrack to implicitely transform
        // the source and target point object information. It will be better to
        // clone the whole involved structure in the future, including the involved
        // connectors. This will then do everything automatically. For now - since
        // SdrDragMove does not use Clones but transforms the original primitives -
        // stay with the implicit transform(offset).
        if(bTail1 && bTail2)
        {
            // start and end are influenced, just transform current EdgeTrack
            aResult = GetEdgeTrackPath();

            aResult.transform(rCurrentTransformation);
        }
        else
        {
            SdrObjConnection aMyCon1(*mpCon1);
            SdrObjConnection aMyCon2(*mpCon2);
            SdrEdgeInfoRec aInfo(*mpEdgeInfo);

            aResult = ImpCalcEdgeTrack(
                aMyCon1,
                aMyCon2,
                &aInfo,
                bTail1 ? &rCurrentTransformation : 0,
                bTail2 ? &rCurrentTransformation : 0);
        }
    }
    else
    {
        basegfx::B2DPoint aPt1(mpCon1->GetPosition());
        basegfx::B2DPoint aPt2(mpCon2->GetPosition());

        if(mpCon1->GetConnectedSdrObject() && mpCon1->IsBestConnection())
        {
            aPt1 = mpCon1->GetConnectedSdrObject()->getSdrObjectTransformation() * basegfx::B2DPoint(0.5, 0.5);
        }

        if(mpCon2->GetConnectedSdrObject() && mpCon2->IsBestConnection())
        {
            aPt2 = mpCon2->GetConnectedSdrObject()->getSdrObjectTransformation() * basegfx::B2DPoint(0.5, 0.5);
        }

        if(bTail1)
        {
            aPt1 *= rCurrentTransformation;
        }

        if(bTail2)
        {
            aPt2 *= rCurrentTransformation;
        }

        aResult.append(aPt1);
        aResult.append(aPt2);
    }

    return aResult;
}

bool SdrEdgeObj::BegCreate(SdrDragStat& rDragStat)
{
    rDragStat.SetNoSnap(true);

    // prepare connector with new positions
    mpCon1->SetConnectedSdrObject(0);
    mpCon1->SetPosition(rDragStat.GetStart());

    mpCon2->SetConnectedSdrObject(0);
    mpCon2->SetPosition(rDragStat.GetNow());

    SdrView& rSdrView = rDragStat.GetSdrViewFromSdrDragStat();

    if(rSdrView.GetSdrPageView())
    {
        // check for start connection to SdrObject and evtl. connect to it
        sal_uInt32 nID(0);
        bool bBest(false);
        bool bAuto(false);

        // look for new connection
        SdrObject* pNewContact = rSdrView.FindConnector(rDragStat.GetStart(), nID, bBest, bAuto, this);

        mpCon1->SetConnectorID(nID);
        mpCon1->setBestConnection(bBest);
        mpCon1->setAutoVertex(bAuto);
        mpCon1->SetConnectedSdrObject(pNewContact);
    }

    return true;
}

bool SdrEdgeObj::MovCreate(SdrDragStat& rDragStat)
{
    // prepare connector end with new position
    mpCon2->SetConnectedSdrObject(0);
    mpCon2->SetPosition(rDragStat.GetNow());

    SdrView& rSdrView = rDragStat.GetSdrViewFromSdrDragStat();

    if(rSdrView.GetSdrPageView())
    {
        // check for end connection to SdrObject and evtl. connect to it
        sal_uInt32 nID(0);
        bool bBest(false);
        bool bAuto(false);

        // look for new connection
        SdrObject* pNewContact = rSdrView.FindConnector(rDragStat.GetNow(), nID, bBest, bAuto, this);

        mpCon2->SetConnectorID(nID);
        mpCon2->setBestConnection(bBest);
        mpCon2->setAutoVertex(bAuto);
        mpCon2->SetConnectedSdrObject(pNewContact);

        rDragStat.GetSdrViewFromSdrDragStat().SetConnectMarker(pNewContact);
    }

    return true;
}

bool SdrEdgeObj::EndCreate(SdrDragStat& rDragStat, SdrCreateCmd eCmd)
{
    const bool bOk(SDRCREATE_FORCEEND == eCmd || rDragStat.GetPointAnz() >= 2);

    if(bOk)
    {
        rDragStat.GetSdrViewFromSdrDragStat().HideConnectMarker();
        ImpSetEdgeInfoToAttr();
    }

    return bOk;
}

bool SdrEdgeObj::BckCreate(SdrDragStat& rDragStat)
{
    rDragStat.GetSdrViewFromSdrDragStat().HideConnectMarker();

    return false;
}

void SdrEdgeObj::BrkCreate(SdrDragStat& rDragStat)
{
    rDragStat.GetSdrViewFromSdrDragStat().HideConnectMarker();
}

basegfx::B2DPolyPolygon SdrEdgeObj::TakeCreatePoly(const SdrDragStat& /*rStatDrag*/) const
{
    const_cast< SdrEdgeObj* >(this)->ImpRecalcEdgeTrack();

    return basegfx::B2DPolyPolygon(maEdgeTrack);
}

const basegfx::B2DHomMatrix& SdrEdgeObj::getSdrObjectTransformation() const
{
    // nothing to do (evtl. add a check in debug mode, but the transformation
    // should be up to date all the time as long as all changes to the geometry
    // of this object use impChangeEdgeTrackData to change maEdgeTrack)
    return SdrTextObj::getSdrObjectTransformation();
}

void SdrEdgeObj::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    const basegfx::B2DHomMatrix aCurrentTransformation(getSdrObjectTransformation());

    if(rTransformation != aCurrentTransformation)
    {
        // #54102# handle start and end point if not connected
        const bool bCon1(GetSdrObjectConnection(true));
        const bool bCon2(GetSdrObjectConnection(false));
        const bool bUserDefined(maEdgeTrack.count() && mbEdgeTrackUserDefined);
        const bool bApplyTransform(bUserDefined || !bCon1 || !bCon2);
        const bool bCheckUserDistaces(!IsPasteResize() && mpEdgeInfo->ImpUsesUserDistances());
        bool bScale(false);

        if(bApplyTransform)
        {
            // get old transform and invert, multiply with new transform to get full transform
            basegfx::B2DHomMatrix aCompleteTransform(basegfx::tools::guaranteeMinimalScaling(aCurrentTransformation));
            aCompleteTransform.invert();
            aCompleteTransform = rTransformation * aCompleteTransform;

            if(bCheckUserDistaces)
            {
                const basegfx::B2DVector aDiagonal(1.0, 1.0);

                bScale = !basegfx::fTools::equal(aDiagonal.getLength(), (aCompleteTransform * aDiagonal).getLength());
            }

            if(bUserDefined)
            {
                // #120437# special handling when track is user defined
                maEdgeTrack.transform(aCompleteTransform);

                mpCon1->SetConnectedSdrObject(0);
                mpCon1->adaptBestConnectionPosition(maEdgeTrack.getB2DPoint(0));

                mpCon2->SetConnectedSdrObject(0);
                mpCon2->adaptBestConnectionPosition(maEdgeTrack.getB2DPoint(maEdgeTrack.count() - 1));
            }
            else
            {
                if(!bCon1)
                {
                    // transform first point
                    mpCon1->SetPosition(aCompleteTransform * mpCon1->GetPosition());
                }

                if(!bCon2)
                {
                    // transform last point
                    mpCon2->SetPosition(aCompleteTransform * mpCon2->GetPosition());
                }
            }
        }
        else
        {
            // both ends are connected to some SdrObject, thus transforming this
            // object will do nothing, so no need to call parent and set the new
            // transform at the underlying SdrObject.
            // Reactions on changes of the connected SdrObjects is done in Notify.
        }

        // if resize contains scale and is not from paste, forget user distances
        if(bCheckUserDistaces && bScale)
        {
            mpEdgeInfo->ImpResetUserDistances();
            ImpSetEdgeInfoToAttr();
            SetEdgeTrackDirty();
            SetChanged();
        }
    }
}

SdrObject* SdrEdgeObj::DoConvertToPolygonObject(bool bBezier, bool bAddText) const
{
    const_cast< SdrEdgeObj* >(this)->ImpRecalcEdgeTrack();
    SdrObject* pRet = ImpConvertMakeObj(basegfx::B2DPolyPolygon(maEdgeTrack), bBezier);

    if(bAddText)
    {
        pRet = ImpConvertAddText(pRet, bBezier);
    }

    return pRet;
}

sal_uInt32 SdrEdgeObj::GetSnapPointCount() const
{
    return 2;
}

basegfx::B2DPoint SdrEdgeObj::GetSnapPoint(sal_uInt32 i) const
{
    if(!i)
    {
        return mpCon1->GetPosition();
    }
    else
    {
        return mpCon2->GetPosition();
    }
}

bool SdrEdgeObj::IsPolygonObject() const
{
    return false;
}

SdrObjGeoData* SdrEdgeObj::NewGeoData() const
{
    return new SdrEdgeObjGeoData;
}

void SdrEdgeObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    // call parent
    SdrTextObj::SaveGeoData(rGeo);

    SdrEdgeObjGeoData* pEGeo = dynamic_cast< SdrEdgeObjGeoData* >(&rGeo);

    if(pEGeo)
    {
        *pEGeo->mpCon1 = *mpCon1; // this will ignore owner and make copy inactive
        *pEGeo->mpCon2 = *mpCon2; // this will ignore owner and make copy inactive
        pEGeo->maEdgeTrack = maEdgeTrack;
        *pEGeo->mpEdgeInfo = *mpEdgeInfo;
        pEGeo->mbEdgeTrackUserDefined = mbEdgeTrackUserDefined;
    }
    else
    {
        OSL_ENSURE(false, "SaveGeoData with wrong data type (!)");
    }
}

void SdrEdgeObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    // call parent
    SdrTextObj::RestGeoData(rGeo);

    const SdrEdgeObjGeoData* pEGeo = dynamic_cast< const SdrEdgeObjGeoData* >(&rGeo);

    if(pEGeo)
    {
        *mpCon1 = *pEGeo->mpCon1;
        *mpCon2 = *pEGeo->mpCon2;
        maEdgeTrack = pEGeo->maEdgeTrack;
        *mpEdgeInfo = *pEGeo->mpEdgeInfo;
        mbEdgeTrackUserDefined = pEGeo->mbEdgeTrackUserDefined;
    }
    else
    {
        OSL_ENSURE(false, "RestGeoData with wrong data type (!)");
    }
}

basegfx::B2DPoint SdrEdgeObj::GetTailPoint(bool bTail) const
{
    if(bTail)
    {
        return mpCon1->GetPosition();
    }
    else
    {
        return mpCon2->GetPosition();
    }
}

void SdrEdgeObj::SetTailPoint(bool bTail, const basegfx::B2DPoint& rPt)
{
    if(bTail)
    {
        mpCon1->SetPosition(rPt);
    }
    else
    {
        mpCon2->SetPosition(rPt);
    }
}

/** this method is used by the api to set a glue point for a connection
    nId == -1 :     The best default point is automaticly choosen
    0 <= nId <= 3 : One of the default points is choosen
    nId >= 4 :      A user defined glue point is choosen
*/
void SdrEdgeObj::setGluePointIndex(bool bTail, sal_Int32 nIndex /* = -1 */ )
{
    SdrObjConnection& rConn1(bTail ? *mpCon1 : *mpCon2);

    if( nIndex > 3 )
    {
        nIndex -= 4; // The start api index and the implementation index is now both 0

        // for user defined glue points we have
        // to get the id for this index first
        const SdrObject* pCandidate = rConn1.GetConnectedSdrObject();

        if(pCandidate)
        {
            const sdr::glue::GluePointProvider& rProvider = pCandidate->GetGluePointProvider();

            if(!rProvider.hasUserGluePoints() || !rProvider.findUserGluePointByID(nIndex))
            {
                return;
            }
        }
    }
    else if( nIndex < 0 )
    {
        nIndex = 0;
    }

    rConn1.SetConnectorID(nIndex);
    rConn1.setBestConnection(nIndex < 0);
    rConn1.setAutoVertex(nIndex >= 0 && nIndex <= 3);
}

/** this method is used by the api to return a glue point id for a connection.
    See setGluePointId for possible return values */
sal_Int32 SdrEdgeObj::getGluePointIndex(bool bTail)
{
    SdrObjConnection& rConn1(bTail ? *mpCon1 : *mpCon2);
    sal_Int32 nId = -1;

    if(!rConn1.IsBestConnection())
    {
        nId = rConn1.GetConnectorId();

        if(!rConn1.IsAutoVertex())
        {
            nId += 4; // The start api index and the implementation index is now both 0
        }
    }

    return nId;
}

//////////////////////////////////////////////////////////////////////////////
// eof
