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
#include "precompiled_sw.hxx"
#include "hintids.hxx"
#include <svx/svdtrans.hxx>
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdograf.hxx>
#include <svx/svddrgv.hxx>
#include <fmtclds.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmturl.hxx>
#include "viewsh.hxx"
#include "viewimp.hxx"
#include "cntfrm.hxx"
#include "frmatr.hxx"
#include "doc.hxx"
#include <IDocumentUndoRedo.hxx>
#include "dview.hxx"
#include "dflyobj.hxx"
#include "flyfrm.hxx"
#include "frmfmt.hxx"
#include "viewopt.hxx"
#include "frmtool.hxx"
#include "flyfrms.hxx"
#include "ndnotxt.hxx"
#include "grfatr.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"

using namespace ::com::sun::star;
// --> OD 2004-11-22 #117958#

#include <svx/sdr/properties/defaultproperties.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <sw_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <svx/svdlegacy.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/svdetc.hxx>

using namespace ::com::sun::star;

static sal_Bool bInResize = sal_False;

/*************************************************************************
|*
|*  SwFlyDrawObj::Ctor
|*
|*  Ersterstellung      MA 18. Apr. 95
|*  Letzte Aenderung    MA 28. May. 96
|*
*************************************************************************/

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // #i95264# currently needed since createViewIndependentPrimitive2DSequence()
        // is called when the view independent range of the object is used. There should currently no VOCs being
        // constructed since it gets not visualized (instead the corresponding SwVirtFlyDrawObj's
        // referencing this one are visualized).
        class VCOfSwFlyDrawObj : public ViewContactOfSdrObj
        {
        protected:
            // This method is responsible for creating the graphical visualisation data
            // ONLY based on model data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            // basic constructor, used from SdrObject.
            VCOfSwFlyDrawObj(SwFlyDrawObj& rObj)
            :   ViewContactOfSdrObj(rObj)
            {
            }
            virtual ~VCOfSwFlyDrawObj();
        };

        drawinglayer::primitive2d::Primitive2DSequence VCOfSwFlyDrawObj::createViewIndependentPrimitive2DSequence() const
        {
            // currently gets not visualized, return empty sequence
            return drawinglayer::primitive2d::Primitive2DSequence();
        }

        VCOfSwFlyDrawObj::~VCOfSwFlyDrawObj()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* SwFlyDrawObj::CreateObjectSpecificProperties()
{
    // --> OD 2004-11-22 #117958# - create default properties
    return new sdr::properties::DefaultProperties(*this);
    // <--
}

sdr::contact::ViewContact* SwFlyDrawObj::CreateObjectSpecificViewContact()
{
    // #i95264# needs an own VC since createViewIndependentPrimitive2DSequence()
    // is called when the view independent range of the object is used
    return new sdr::contact::VCOfSwFlyDrawObj(*this);
}

SwFlyDrawObj::SwFlyDrawObj(SdrModel& rSdrModel)
:   SdrObject(rSdrModel)
{
}

SwFlyDrawObj::~SwFlyDrawObj()
{
}

void SwFlyDrawObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SwFlyDrawObj* pSource = dynamic_cast< const SwFlyDrawObj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrObject::copyDataFromSdrObject(rSource);

            // nothing to do on own data
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SwFlyDrawObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    SwFlyDrawObj* pClone = new SwFlyDrawObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

/*************************************************************************
|*
|*  SwFlyDrawObj::Factory-Methoden
|*
|*  Ersterstellung      MA 23. Feb. 95
|*  Letzte Aenderung    MA 23. Feb. 95
|*
*************************************************************************/

sal_uInt32 __EXPORT SwFlyDrawObj::GetObjInventor() const
{
    return SWGInventor;
}


sal_uInt16 __EXPORT SwFlyDrawObj::GetObjIdentifier()    const
{
    return SwFlyDrawObjIdentifier;
}


sal_uInt16 __EXPORT SwFlyDrawObj::GetObjVersion() const
{
    return SwDrawFirst;
}

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::CToren, Dtor
|*
|*  Ersterstellung      MA 08. Dec. 94
|*  Letzte Aenderung    MA 28. May. 96
|*
*************************************************************************/

//////////////////////////////////////////////////////////////////////////////////////
// AW: Need own primitive to get the FlyFrame paint working

namespace drawinglayer
{
    namespace primitive2d
    {
        class SwVirtFlyDrawObjPrimitive : public BufferedDecompositionPrimitive2D
        {
        private:
            const SwVirtFlyDrawObj&                 mrSwVirtFlyDrawObj;
            const basegfx::B2DRange                 maOuterRange;

        protected:
            // method which is to be used to implement the local decomposition of a 2D primitive
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            SwVirtFlyDrawObjPrimitive(
                const SwVirtFlyDrawObj& rSwVirtFlyDrawObj,
                const basegfx::B2DRange &rOuterRange)
            :   BufferedDecompositionPrimitive2D(),
                mrSwVirtFlyDrawObj(rSwVirtFlyDrawObj),
                maOuterRange(rOuterRange)
            {
            }

            // get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            // overloaded to allow callbacks to wrap_DoPaintObject
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            // data read access
            const SwVirtFlyDrawObj& getSwVirtFlyDrawObj() const { return mrSwVirtFlyDrawObj; }
            const basegfx::B2DRange& getOuterRange() const { return maOuterRange; }

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SwVirtFlyDrawObjPrimitive::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            if(!getOuterRange().isEmpty())
            {
                // currently this SW object has no primitive representation. As long as this is the case,
                // create invisible geometry to allow corfect HitTest and BoundRect calculations for the
                // object. Use a filled primitive to get 'inside' as default object hit. The special cases from
                // the old SwVirtFlyDrawObj::CheckHit implementation are handled now in SwDrawView::PickObj;
                // this removed the 'hack' to get a view from inside model data or to react on null-tolerance
                // as it was done in the old implementation
                const Primitive2DReference aHitTestReference(
                    createHiddenGeometryPrimitives2D(
                        true,
                        getOuterRange()));

                aRetval = Primitive2DSequence(&aHitTestReference, 1);
            }

            return aRetval;
        }

        basegfx::B2DRange SwVirtFlyDrawObjPrimitive::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            return getOuterRange();
        }

        Primitive2DSequence SwVirtFlyDrawObjPrimitive::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // This is the callback to keep the FlyFrame painting in SW alive as long as it
            // is not changed to primitives. This is the method which will be called by the processors
            // when they do not know this primitive (and they do not). Inside wrap_DoPaintObject
            // there needs to be a test that paint is only done during SW repaints (see there).
            // Using this mechanism guarantees the correct Z-Order of the VirtualObject-based FlyFrames.
            getSwVirtFlyDrawObj().wrap_DoPaintObject();

            // call parent
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SwVirtFlyDrawObjPrimitive, PRIMITIVE2D_ID_SWVIRTFLYDRAWOBJPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////////////
// AW: own sdr::contact::ViewContact (VC) sdr::contact::ViewObjectContact (VOC) needed
// since offset is defined different
// For paint, that offset is used by setting at the OutputDevice; for primitives this is
// not possible since we have no OutputDevice, but define the geometry itself.

namespace sdr
{
    namespace contact
    {
        class VCOfSwVirtFlyDrawObj : public ViewContactOfSdrObj
        {
        protected:
            // This method is responsible for creating the graphical visualisation data
            // ONLY based on model data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            // basic constructor, used from SdrObject.
            VCOfSwVirtFlyDrawObj(SwVirtFlyDrawObj& rObj)
            :   ViewContactOfSdrObj(rObj)
            {
            }
            virtual ~VCOfSwVirtFlyDrawObj();

            // access to SwVirtFlyDrawObj
            SwVirtFlyDrawObj& GetSwVirtFlyDrawObj() const
            {
                return (SwVirtFlyDrawObj&)mrObject;
            }
        };
    } // end of namespace contact
} // end of namespace sdr

namespace sdr
{
    namespace contact
    {
        drawinglayer::primitive2d::Primitive2DSequence VCOfSwVirtFlyDrawObj::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SdrObject& rReferencedObject = GetSwVirtFlyDrawObj().GetReferencedObj();

            if(dynamic_cast< const SwFlyDrawObj* >(&rReferencedObject))
            {
                // create an own specialized primitive which is used as repaint callpoint and HitTest
                // for HitTest processor (see primitive implementation above)
                const basegfx::B2DRange aOuterRange(GetSwVirtFlyDrawObj().getOuterBound());

                if(!aOuterRange.isEmpty())
                {
                    const drawinglayer::primitive2d::Primitive2DReference xPrimitive(
                        new drawinglayer::primitive2d::SwVirtFlyDrawObjPrimitive(
                            GetSwVirtFlyDrawObj(),
                            aOuterRange));

                    xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xPrimitive, 1);
                }
            }

            return xRetval;
        }

        VCOfSwVirtFlyDrawObj::~VCOfSwVirtFlyDrawObj()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////////////

basegfx::B2DRange SwVirtFlyDrawObj::getOuterBound() const
{
    basegfx::B2DRange aOuterRange;
    const SdrObject& rReferencedObject = GetReferencedObj();

    if(dynamic_cast< const SwFlyDrawObj* >(&rReferencedObject))
    {
        const SwFlyFrm* pFlyFrame = GetFlyFrm();

        if(pFlyFrame)
        {
            const Rectangle aOuterRectangle(pFlyFrame->Frm().Pos(), pFlyFrame->Frm().SSize());

            if(!aOuterRectangle.IsEmpty()
                && RECT_EMPTY != aOuterRectangle.Right()
                && RECT_EMPTY != aOuterRectangle.Bottom())
            {
                aOuterRange.expand(basegfx::B2DTuple(aOuterRectangle.Left(), aOuterRectangle.Top()));
                aOuterRange.expand(basegfx::B2DTuple(aOuterRectangle.Right(), aOuterRectangle.Bottom()));
            }
        }
    }

    return aOuterRange;
}

basegfx::B2DRange SwVirtFlyDrawObj::getInnerBound() const
{
    basegfx::B2DRange aInnerRange;
    const SdrObject& rReferencedObject = GetReferencedObj();

    if(dynamic_cast< const SwFlyDrawObj* >(&rReferencedObject))
    {
        const SwFlyFrm* pFlyFrame = GetFlyFrm();

        if(pFlyFrame)
        {
            const Rectangle aInnerRectangle(pFlyFrame->Frm().Pos() + pFlyFrame->Prt().Pos(), pFlyFrame->Prt().SSize());

            if(!aInnerRectangle.IsEmpty()
                && RECT_EMPTY != aInnerRectangle.Right()
                && RECT_EMPTY != aInnerRectangle.Bottom())
            {
                aInnerRange.expand(basegfx::B2DTuple(aInnerRectangle.Left(), aInnerRectangle.Top()));
                aInnerRange.expand(basegfx::B2DTuple(aInnerRectangle.Right(), aInnerRectangle.Bottom()));
            }
        }
    }

    return aInnerRange;
}

sdr::contact::ViewContact* SwVirtFlyDrawObj::CreateObjectSpecificViewContact()
{
    // need an own ViewContact (VC) to allow creation of a specialized primitive
    // for being able to visualize the FlyFrames in primitive renderers
    return new sdr::contact::VCOfSwVirtFlyDrawObj(*this);
}

void SwVirtFlyDrawObj::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& /*rHint*/)
{
//  mbClosedObject = rRefObj.IsClosedObj();
    ActionChanged();
}

SwVirtFlyDrawObj::SwVirtFlyDrawObj(
    SdrObject& rNew,
    SwFlyFrm* pFly)
:   SdrObject(rNew.getSdrModelFromSdrObject()),
    pFlyFrm(pFly),
    rRefObj(rNew)
{
    const SvxProtectItem &rP = pFlyFrm->GetFmt()->GetProtect();
    mbMoveProtect = rP.IsPosProtected();
    mbSizeProtect = rP.IsSizeProtected();

    StartListening(rRefObj);
//  mbClosedObject = rRefObj.IsClosedObj();
}


__EXPORT SwVirtFlyDrawObj::~SwVirtFlyDrawObj()
{
    EndListening(rRefObj);

    if ( getParentOfSdrObject() )   //Der SdrPage die Verantwortung entziehen.
        getParentOfSdrObject()->RemoveObjectFromSdrObjList( GetNavigationPosition() );
}

void SwVirtFlyDrawObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SwVirtFlyDrawObj* pSource = dynamic_cast< const SwVirtFlyDrawObj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrObject::copyDataFromSdrObject(rSource);

            // copy AnchorPos
            SetAnchorPos(pSource->GetAnchorPos());
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SwVirtFlyDrawObj::CloneSdrObject(SdrModel* /*pTargetModel*/) const
{
    SwVirtFlyDrawObj* pClone = new SwVirtFlyDrawObj(
        const_cast< SdrObject& >(GetReferencedObj()),
        const_cast< SwFlyFrm* >(GetFlyFrm()));
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

bool SwVirtFlyDrawObj::IsClosedObj() const
{
    return rRefObj.IsClosedObj();
}

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::GetFmt()
|*
|*  Ersterstellung      MA 08. Dec. 94
|*  Letzte Aenderung    MA 08. Dec. 94
|*
*************************************************************************/

const SwFrmFmt *SwVirtFlyDrawObj::GetFmt() const
{
    return GetFlyFrm()->GetFmt();
}


SwFrmFmt *SwVirtFlyDrawObj::GetFmt()
{
    return GetFlyFrm()->GetFmt();
}

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::Paint()
|*
|*  Ersterstellung      MA 20. Dec. 94
|*  Letzte Aenderung    MA 18. Dec. 95
|*
*************************************************************************/

// --> OD #i102707#
namespace
{
    class RestoreMapMode
    {
        public:
            explicit RestoreMapMode( ViewShell* pViewShell )
                : mbMapModeRestored( false )
                , mpOutDev( pViewShell->GetOut() )
            {
                if ( pViewShell->getPrePostMapMode() != mpOutDev->GetMapMode() )
                {
                    mpOutDev->Push(PUSH_MAPMODE);

                    GDIMetaFile* pMetaFile = mpOutDev->GetConnectMetaFile();
                    if ( pMetaFile &&
                         pMetaFile->IsRecord() && !pMetaFile->IsPause() )
                    {
                        ASSERT( false,
                                "MapMode restoration during meta file creation is somehow suspect - using <SetRelativeMapMode(..)>, but not sure, if correct." )
                        mpOutDev->SetRelativeMapMode( pViewShell->getPrePostMapMode() );
                    }
                    else
                    {
                        mpOutDev->SetMapMode( pViewShell->getPrePostMapMode() );
                    }

                    mbMapModeRestored = true;
                }
            };

            ~RestoreMapMode()
            {
                if ( mbMapModeRestored )
                {
                    mpOutDev->Pop();
                }
            };

        private:
            bool mbMapModeRestored;
            OutputDevice* mpOutDev;
    };
}
// <--

void SwVirtFlyDrawObj::wrap_DoPaintObject() const
{
    ViewShell* pShell = pFlyFrm->getRootFrm()->GetCurrShell();

    // Only paint when we have a current shell and a DrawingLayer paint is in progress.
    // This avcoids evtl. problems with renderers which do processing stuff,
    // but no paints. IsPaintInProgress() depends on SW repaint, so, as long
    // as SW paints self and calls DrawLayer() for Heaven and Hell, this will
    // be correct
    if ( pShell && pShell->IsDrawingLayerPaintInProgress() )
    {
        sal_Bool bDrawObject(sal_True);

        if ( !SwFlyFrm::IsPaint( (SdrObject*)this, pShell ) )
        {
            bDrawObject = sal_False;
        }

        if ( bDrawObject )
        {
            if ( !pFlyFrm->IsFlyInCntFrm() )
            {
                // it is also necessary to restore the VCL MapMode from ViewInformation since e.g.
                // the VCL PixelRenderer resets it at the used OutputDevice. Unfortunately, this
                // excludes shears and rotates which are not expressable in MapMode.
                // OD #i102707#
                // new helper class to restore MapMode - restoration, only if
                // needed and consideration of paint for meta file creation .
                RestoreMapMode aRestoreMapModeIfNeeded( pShell );

                // paint the FlyFrame (use standard VCL-Paint)
                pFlyFrm->Paint( GetFlyFrm()->Frm() );
            }
        }
    }
}

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::TakeObjInfo()
|*
|*  Ersterstellung      MA 03. May. 95
|*  Letzte Aenderung    MA 03. May. 95
|*
*************************************************************************/

void __EXPORT SwVirtFlyDrawObj::TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const
{
    rInfo.mbSelectAllowed = rInfo.mbMoveAllowed = rInfo.mbResizeFreeAllowed = rInfo.mbResizePropAllowed = true;
    rInfo.mbRotateFreeAllowed = rInfo.mbRotate90Allowed = rInfo.mbMirrorFreeAllowed = rInfo.mbMirror45Allowed =
    rInfo.mbMirror90Allowed = rInfo.mbShearAllowed = rInfo.mbCanConvToPath = rInfo.mbCanConvToPoly =
    rInfo.mbCanConvToPathLineToArea = rInfo.mbCanConvToPolyLineToArea = false;
}


/*************************************************************************
|*
|*  SwVirtFlyDrawObj::Groessenermittlung
|*
|*  Ersterstellung      MA 12. Jan. 95
|*  Letzte Aenderung    MA 10. Nov. 95
|*
*************************************************************************/

void SwVirtFlyDrawObj::SetRect() const
{
    if ( GetFlyFrm()->Frm().HasArea() )
    {
        const Rectangle aFrameRect(GetFlyFrm()->Frm().SVRect());

        ((SwVirtFlyDrawObj*)this)->setSdrObjectTransformation(
            basegfx::tools::createScaleTranslateB2DHomMatrix(
                aFrameRect.GetWidth(), aFrameRect.GetHeight(),
                aFrameRect.Left(), aFrameRect.Top()));
    }
    else
    {
        ((SwVirtFlyDrawObj*)this)->setSdrObjectTransformation(basegfx::B2DHomMatrix());
    }
}


//void __EXPORT SwVirtFlyDrawObj::recalculateObjectRange()
//{
//  SetRect();
//  maObjectRange = getSdrObjectTransformation() * basegfx::B2DRange::getUnitB2DRange();
//}


::basegfx::B2DPolyPolygon SwVirtFlyDrawObj::TakeXorPoly() const
{
    const Rectangle aSourceRectangle(GetFlyFrm()->Frm().SVRect());
    const ::basegfx::B2DRange aSourceRange(aSourceRectangle.Left(), aSourceRectangle.Top(), aSourceRectangle.Right(), aSourceRectangle.Bottom());
    ::basegfx::B2DPolyPolygon aRetval;

    aRetval.append(::basegfx::tools::createPolygonFromRect(aSourceRange));

    return aRetval;
}

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::Move() und Resize()
|*
|*  Ersterstellung      MA 12. Jan. 95
|*  Letzte Aenderung    MA 26. Jul. 96
|*
*************************************************************************/

void __EXPORT SwVirtFlyDrawObj::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    SdrObject::setSdrObjectTransformation(rTransformation);

    impReactOnGeometryChange();
}

void SwVirtFlyDrawObj::impReactOnGeometryChange()
{
    const basegfx::B2DPoint aCurrentTopLeft(getSdrObjectTransformation() * basegfx::B2DPoint(0.0, 0.0));
    const Point aOldPos( GetFlyFrm()->Frm().Pos() );
    const Point aNewPos(basegfx::fround(aCurrentTopLeft.getX()), basegfx::fround(aCurrentTopLeft.getY()));
    const SwRect aFlyRect( sdr::legacy::GetSnapRect(*this) );

    //Wenn der Fly eine automatische Ausrichtung hat (rechts oder oben),
    //so soll die Automatik erhalten bleiben
    SwFrmFmt *pFmt = GetFlyFrm()->GetFmt();
    const sal_Int16 eHori = pFmt->GetHoriOrient().GetHoriOrient();
    const sal_Int16 eVert = pFmt->GetVertOrient().GetVertOrient();
    const sal_Int16 eRelHori = pFmt->GetHoriOrient().GetRelationOrient();
    const sal_Int16 eRelVert = pFmt->GetVertOrient().GetRelationOrient();
    //Bei Absatzgebundenen Flys muss ausgehend von der neuen Position ein
    //neuer Anker gesetzt werden. Anker und neue RelPos werden vom Fly selbst
    //berechnet und gesetzt.
    if( GetFlyFrm()->IsFlyAtCntFrm() )
        ((SwFlyAtCntFrm*)GetFlyFrm())->SetAbsPos( aNewPos );
    else
    {
        const SwFrmFmt *pTmpFmt = GetFmt();
        const SwFmtVertOrient &rVert = pTmpFmt->GetVertOrient();
        const SwFmtHoriOrient &rHori = pTmpFmt->GetHoriOrient();
        long lXDiff = aNewPos.X() - aOldPos.X();
        if( rHori.IsPosToggle() && text::HoriOrientation::NONE == eHori &&
            !GetFlyFrm()->FindPageFrm()->OnRightPage() )
            lXDiff = -lXDiff;

        if( GetFlyFrm()->GetAnchorFrm()->IsRightToLeft() &&
            text::HoriOrientation::NONE == eHori )
            lXDiff = -lXDiff;

        long lYDiff = aNewPos.Y() - aOldPos.Y();
        if( GetFlyFrm()->GetAnchorFrm()->IsVertical() )
        {
            //lXDiff -= rVert.GetPos();
            //lYDiff += rHori.GetPos();
            //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
            if ( GetFlyFrm()->GetAnchorFrm()->IsVertLR() )
            {
                lXDiff += rVert.GetPos();
                lXDiff = -lXDiff;
            }
            else
            {
                lXDiff -= rVert.GetPos();
                lYDiff += rHori.GetPos();
            }
        }
        else
        {
            lXDiff += rHori.GetPos();
            lYDiff += rVert.GetPos();
        }

        if( GetFlyFrm()->GetAnchorFrm()->IsRightToLeft() &&
            text::HoriOrientation::NONE != eHori )
            lXDiff = GetFlyFrm()->GetAnchorFrm()->Frm().Width() -
                     aFlyRect.Width() - lXDiff;

        const Point aTmp( lXDiff, lYDiff );
        GetFlyFrm()->ChgRelPos( aTmp );
    }

    SwAttrSet aSet( pFmt->GetDoc()->GetAttrPool(),
                                            RES_VERT_ORIENT, RES_HORI_ORIENT );
    SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
    SwFmtVertOrient aVert( pFmt->GetVertOrient() );
    sal_Bool bPut = sal_False;

    if( !GetFlyFrm()->IsFlyLayFrm() &&
        ::GetHtmlMode(pFmt->GetDoc()->GetDocShell()) )
    {
        //Im HTML-Modus sind nur automatische Ausrichtungen erlaubt.
        //Einzig einen Snap auf Links/Rechts bzw. Linker-/Rechter-Rand koennen
        //wir versuchen.
        const SwFrm* pAnch = GetFlyFrm()->GetAnchorFrm();
        sal_Bool bNextLine = sal_False;

        if( !GetFlyFrm()->IsAutoPos() || text::RelOrientation::PAGE_FRAME != aHori.GetRelationOrient() )
        {
            if( text::RelOrientation::CHAR == eRelHori )
            {
                aHori.SetHoriOrient( text::HoriOrientation::LEFT );
                aHori.SetRelationOrient( text::RelOrientation::CHAR );
            }
            else
            {
                bNextLine = sal_True;
                //Horizontale Ausrichtung:
                const sal_Bool bLeftFrm =
                    aFlyRect.Left() < pAnch->Frm().Left() + pAnch->Prt().Left(),
                    bLeftPrt = aFlyRect.Left() + aFlyRect.Width() <
                               pAnch->Frm().Left() + pAnch->Prt().Width()/2;
                if ( bLeftFrm || bLeftPrt )
                {
                    aHori.SetHoriOrient( text::HoriOrientation::LEFT );
                    aHori.SetRelationOrient( bLeftFrm ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
                }
                else
                {
                    const sal_Bool bRightFrm = aFlyRect.Left() >
                                       pAnch->Frm().Left() + pAnch->Prt().Width();
                    aHori.SetHoriOrient( text::HoriOrientation::RIGHT );
                    aHori.SetRelationOrient( bRightFrm ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
                }
            }
            aSet.Put( aHori );
        }
        //Vertikale Ausrichtung bleibt grundsaetzlich schlicht erhalten,
        //nur bei nicht automatischer Ausrichtung wird umgeschaltet.
        sal_Bool bRelChar = text::RelOrientation::CHAR == eRelVert;
        aVert.SetVertOrient( eVert != text::VertOrientation::NONE ? eVert :
                GetFlyFrm()->IsFlyInCntFrm() ? text::VertOrientation::CHAR_CENTER :
                bRelChar && bNextLine ? text::VertOrientation::CHAR_TOP : text::VertOrientation::TOP );
        if( bRelChar )
            aVert.SetRelationOrient( text::RelOrientation::CHAR );
        else
            aVert.SetRelationOrient( text::RelOrientation::PRINT_AREA );
        aSet.Put( aVert );
        bPut = sal_True;
    }

    //Automatische Ausrichtungen wollen wir moeglichst nicht verlieren.
    if ( !bPut && bInResize )
    {
        if ( text::HoriOrientation::NONE != eHori )
        {
            aHori.SetHoriOrient( eHori );
            aHori.SetRelationOrient( eRelHori );
            aSet.Put( aHori );
            bPut = sal_True;
        }
        if ( text::VertOrientation::NONE != eVert )
        {
            aVert.SetVertOrient( eVert );
            aVert.SetRelationOrient( eRelVert );
            aSet.Put( aVert );
            bPut = sal_True;
        }
    }
    if ( bPut )
        pFmt->SetFmtAttr( aSet );
}


Pointer  __EXPORT SwVirtFlyDrawObj::GetMacroPointer(
    const SdrObjMacroHitRec& ) const
{
    return Pointer( POINTER_REFHAND );
}


bool __EXPORT SwVirtFlyDrawObj::HasMacro() const
{
    const SwFmtURL &rURL = pFlyFrm->GetFmt()->GetURL();
    return rURL.GetMap() || rURL.GetURL().Len();
}


SdrObject* SwVirtFlyDrawObj::CheckMacroHit( const SdrObjMacroHitRec& rRec ) const
{
    const SwFmtURL &rURL = pFlyFrm->GetFmt()->GetURL();
    if( rURL.GetMap() || rURL.GetURL().Len() )
    {
        SwRect aRect;
        if ( pFlyFrm->Lower() && pFlyFrm->Lower()->IsNoTxtFrm() )
        {
            aRect = pFlyFrm->Prt();
            aRect += pFlyFrm->Frm().Pos();
        }
        else
            aRect = pFlyFrm->Frm();

        const Point aHitRecPos(basegfx::fround(rRec.maPos.getX()), basegfx::fround(rRec.maPos.getY()));

        if( aRect.IsInside( aHitRecPos ) )
        {
            SwRect aActRect( aRect );
            Size aActSz( aRect.SSize() );
            const long nRecTol(basegfx::fround(rRec.mfTol));
            aRect.Pos().X() += nRecTol;
            aRect.Pos().Y() += nRecTol;
            aRect.SSize().Height()-= 2 * nRecTol;
            aRect.SSize().Width() -= 2 * nRecTol;

            if( aRect.IsInside( aHitRecPos ) )
            {
                if( !rURL.GetMap() ||
                    pFlyFrm->GetFmt()->GetIMapObject( aHitRecPos, pFlyFrm ))
                    return (SdrObject*)this;

                return 0;
            }
        }
    }
    return SdrObject::CheckMacroHit( rRec );
}

bool SwVirtFlyDrawObj::supportsFullDrag() const
{
    return false;
}

SdrObject* SwVirtFlyDrawObj::getFullDragClone() const
{
    SdrObject& rReferencedObject = const_cast< SdrObject& >(GetReferencedObj());

    return new SdrGrafObj(
        getSdrModelFromSdrObject(),
        GetObjGraphic(rReferencedObject),
        getSdrObjectTransformation());
}

//////////////////////////////////////////////////////////////////////////
// methods which were missing from SdrVirtObj which do something

SdrObject& SwVirtFlyDrawObj::ReferencedObj()
{
    return rRefObj;
}

const SdrObject& SwVirtFlyDrawObj::GetReferencedObj() const
{
    return rRefObj;
}

sdr::properties::BaseProperties& SwVirtFlyDrawObj::GetProperties() const
{
    return rRefObj.GetProperties();
}

sal_uInt32 SwVirtFlyDrawObj::GetObjInventor() const
{
    return rRefObj.GetObjInventor();
}

sal_uInt16 SwVirtFlyDrawObj::GetObjIdentifier() const
{
    return rRefObj.GetObjIdentifier();
}

SdrObjList* SwVirtFlyDrawObj::getChildrenOfSdrObject() const
{
    return rRefObj.getChildrenOfSdrObject();
}

void SwVirtFlyDrawObj::SetChanged()
{
    SdrObject::SetChanged();
}

void SwVirtFlyDrawObj::TakeObjNameSingul(XubString& rName) const
{
    rRefObj.TakeObjNameSingul(rName);
    rName.Insert(sal_Unicode('['), 0);
    rName += sal_Unicode(']');

    String aName( GetName() );
    if(aName.Len())
        {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SwVirtFlyDrawObj::TakeObjNamePlural(XubString& rName) const
{
    rRefObj.TakeObjNamePlural(rName);
    rName.Insert(sal_Unicode('['), 0);
    rName += sal_Unicode(']');
}

sal_uInt32 SwVirtFlyDrawObj::GetPlusHdlCount(const SdrHdl& rHdl) const
{
    return rRefObj.GetPlusHdlCount(rHdl);
}

void SwVirtFlyDrawObj::GetPlusHdl(SdrHdlList& rHdlList, SdrObject& rSdrObject, const SdrHdl& rHdl, sal_uInt32 nPlNum) const
{
    rRefObj.GetPlusHdl(rHdlList, rSdrObject, rHdl, nPlNum);
    OSL_ENSURE(rHdlList.GetHdlCount(), "No PlusHdl added (!)");
    SdrHdl* pHdl = rHdlList.GetHdlByIndex(rHdlList.GetHdlCount() - 1);
    pHdl->setPosition(pHdl->getPosition() + GetAnchorPos());
}

void SwVirtFlyDrawObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    // #i73248#
    // SdrObject::AddToHdlList(rHdlList) is not a good thing to call
    // since at SdrPathObj, only AddToHdlList may be used and the call
    // will instead use the standard implementation which uses GetHdlCount()
    // and GetHdl instead. This is not wrong, but may be much less effective
    // and may not be prepared to GetHdl returning NULL

    // add handles from refObj to list and transform them
    const sal_uInt32 nStart(rHdlList.GetHdlCount());
    rRefObj.AddToHdlList(rHdlList);
    const sal_uInt32 nEnd(rHdlList.GetHdlCount());

    if(nEnd > nStart)
    {
        const basegfx::B2DPoint aOffset(GetOffset().X(), GetOffset().Y());

        for(sal_uInt32 a(nStart); a < nEnd; a++)
        {
            SdrHdl* pCandidate = rHdlList.GetHdlByIndex(a);

            pCandidate->setPosition(pCandidate->getPosition() + aOffset);
        }
    }
}

bool SwVirtFlyDrawObj::hasSpecialDrag() const
{
    return rRefObj.hasSpecialDrag();
}

bool SwVirtFlyDrawObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    return rRefObj.beginSpecialDrag(rDrag);
}

bool SwVirtFlyDrawObj::applySpecialDrag(SdrDragStat& rDrag)
{
    return rRefObj.applySpecialDrag(rDrag);
}

String SwVirtFlyDrawObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    return rRefObj.getSpecialDragComment(rDrag);
}

basegfx::B2DPolyPolygon SwVirtFlyDrawObj::getSpecialDragPoly(const SdrDragStat& rDrag) const
{
    return rRefObj.getSpecialDragPoly(rDrag);
    // Offset handlen !!!!!! fehlt noch !!!!!!!
}

bool SwVirtFlyDrawObj::BegCreate(SdrDragStat& rStat)
{
    return rRefObj.BegCreate(rStat);
}

bool SwVirtFlyDrawObj::MovCreate(SdrDragStat& rStat)
{
    return rRefObj.MovCreate(rStat);
}

bool SwVirtFlyDrawObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    return rRefObj.EndCreate(rStat,eCmd);
}

bool SwVirtFlyDrawObj::BckCreate(SdrDragStat& rStat)
{
    return rRefObj.BckCreate(rStat);
}

void SwVirtFlyDrawObj::BrkCreate(SdrDragStat& rStat)
{
    rRefObj.BrkCreate(rStat);
}

basegfx::B2DPolyPolygon SwVirtFlyDrawObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    return rRefObj.TakeCreatePoly(rDrag);
    // Offset handlen !!!!!! fehlt noch !!!!!!!
}

sal_uInt32 SwVirtFlyDrawObj::GetSnapPointCount() const
{
    return rRefObj.GetSnapPointCount();
}

basegfx::B2DPoint SwVirtFlyDrawObj::GetSnapPoint(sal_uInt32 i) const
{
    return rRefObj.GetSnapPoint(i) + GetAnchorPos();
}

bool SwVirtFlyDrawObj::IsPolygonObject() const
{
    return rRefObj.IsPolygonObject();
}

sal_uInt32 SwVirtFlyDrawObj::GetObjectPointCount() const
{
    return rRefObj.GetObjectPointCount();
}

basegfx::B2DPoint SwVirtFlyDrawObj::GetObjectPoint(sal_uInt32 i) const
{
    return rRefObj.GetObjectPoint(i) + GetAnchorPos();
}

void SwVirtFlyDrawObj::SetObjectPoint(const basegfx::B2DPoint& rPnt, sal_uInt32 i)
{
    const basegfx::B2DPoint aP(rPnt - GetAnchorPos());
    rRefObj.SetObjectPoint(aP,i);
    ActionChanged();
}

SdrObjGeoData* SwVirtFlyDrawObj::GetGeoData() const
{
    return rRefObj.GetGeoData();
}

void SwVirtFlyDrawObj::SetGeoData(const SdrObjGeoData& rGeo)
{
    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
    rRefObj.SetGeoData(rGeo);
    ActionChanged();
}

void SwVirtFlyDrawObj::ReformatText()
{
    rRefObj.ReformatText();
}

void SwVirtFlyDrawObj::PaintMacro(OutputDevice& rOut, const SdrObjMacroHitRec& rRec) const
{
    rRefObj.PaintMacro(rOut,rRec); // Todo: Positionsversatz
}

bool SwVirtFlyDrawObj::DoMacro(const SdrObjMacroHitRec& rRec)
{
    return rRefObj.DoMacro(rRec); // Todo: Positionsversatz
}

XubString SwVirtFlyDrawObj::GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const
{
    return rRefObj.GetMacroPopupComment(rRec); // Todo: Positionsversatz
}

const Point SwVirtFlyDrawObj::GetOffset() const
{
    // #i73248# default offset of SdrVirtObj is maObjectAnchor
    return sdr::legacy::GetAnchorPos(*this);
}

// eof
