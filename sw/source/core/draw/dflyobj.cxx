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

#include <hintids.hxx>
#include <comphelper/lok.hxx>
#include <osl/diagnose.h>
#include <tools/mapunit.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdtrans.hxx>
#include <editeng/protitem.hxx>
#include <svx/svdpage.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/ptrstyle.hxx>

#include <fmtclds.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmturl.hxx>
#include <viewsh.hxx>
#include <frmatr.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <dflyobj.hxx>
#include <flyfrm.hxx>
#include <frmfmt.hxx>
#include <viewopt.hxx>
#include <frmtool.hxx>
#include <flyfrms.hxx>
#include <ndnotxt.hxx>
#include <grfatr.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <textboxhelper.hxx>
#include <wrtsh.hxx>
#include <ndgrf.hxx>
#include <frmmgr.hxx>

#include <svx/sdr/properties/defaultproperties.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

// AW: For VCOfDrawVirtObj and stuff
#include <svx/sdr/contact/viewcontactofvirtobj.hxx>
#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <sw_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <notxtfrm.hxx>

using namespace ::com::sun::star;

static bool bInResize = false;


namespace sdr::contact
{
        namespace {

        /**
         * @see #i95264#
         *
         * currently needed since createViewIndependentPrimitive2DSequence() is called when
         * RecalcBoundRect() is used. There should currently no VOCs being constructed since it
         * gets not visualized (instead the corresponding SwVirtFlyDrawObj's referencing this one
         * are visualized).
         */
        class VCOfSwFlyDrawObj : public ViewContactOfSdrObj
        {
        protected:
            /** This method is responsible for creating the graphical visualisation data
             *
             * @note ONLY based on model data
             */
            virtual void createViewIndependentPrimitive2DSequence(drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const override;

        public:
            /// basic constructor, used from SdrObject.
            explicit VCOfSwFlyDrawObj(SwFlyDrawObj& rObj)
            :   ViewContactOfSdrObj(rObj)
            {
            }
        };

        }

        void VCOfSwFlyDrawObj::createViewIndependentPrimitive2DSequence(drawinglayer::primitive2d::Primitive2DDecompositionVisitor&) const
        {
            // currently gets not visualized, return empty sequence
        }

} // end of namespace sdr::contact

std::unique_ptr<sdr::properties::BaseProperties> SwFlyDrawObj::CreateObjectSpecificProperties()
{
    // create default properties
    return std::make_unique<sdr::properties::DefaultProperties>(*this);
}

std::unique_ptr<sdr::contact::ViewContact> SwFlyDrawObj::CreateObjectSpecificViewContact()
{
    // needs an own VC since createViewIndependentPrimitive2DSequence()
    // is called when RecalcBoundRect() is used
    return std::make_unique<sdr::contact::VCOfSwFlyDrawObj>(*this);
}

SwFlyDrawObj::SwFlyDrawObj(SdrModel& rSdrModel)
:   SdrObject(rSdrModel),
    mbIsTextBox(false)
{
}

SwFlyDrawObj::~SwFlyDrawObj()
{
}

// SwFlyDrawObj - Factory-Methods
SdrInventor SwFlyDrawObj::GetObjInventor() const
{
    return SdrInventor::Swg;
}

SdrObjKind SwFlyDrawObj::GetObjIdentifier() const
{
    return SwFlyDrawObjIdentifier;
}

// TODO: Need own primitive to get the FlyFrame paint working
namespace drawinglayer::primitive2d
{
        namespace {

        class SwVirtFlyDrawObjPrimitive : public BufferedDecompositionPrimitive2D
        {
        private:
            const SwVirtFlyDrawObj&                 mrSwVirtFlyDrawObj;
            const basegfx::B2DRange                 maOuterRange;

        protected:
            /// method which is to be used to implement the local decomposition of a 2D primitive
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            SwVirtFlyDrawObjPrimitive(
                const SwVirtFlyDrawObj& rSwVirtFlyDrawObj,
                const basegfx::B2DRange &rOuterRange)
            :   mrSwVirtFlyDrawObj(rSwVirtFlyDrawObj),
                maOuterRange(rOuterRange)
            {
            }

            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            // override to allow callbacks to wrap_DoPaintObject
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const override;

            // data read access
            const SwVirtFlyDrawObj& getSwVirtFlyDrawObj() const { return mrSwVirtFlyDrawObj; }
            const basegfx::B2DRange& getOuterRange() const { return maOuterRange; }

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };

        }
} // end of namespace drawinglayer::primitive2d

namespace drawinglayer::primitive2d
{
        void SwVirtFlyDrawObjPrimitive::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(getOuterRange().isEmpty())
                return;

            // currently this SW object has no primitive representation. As long as this is the case,
            // create invisible geometry to allow correct HitTest and BoundRect calculations for the
            // object. Use a filled primitive to get 'inside' as default object hit. The special cases from
            // the old SwVirtFlyDrawObj::CheckHit implementation are handled now in SwDrawView::PickObj;
            // this removed the 'hack' to get a view from inside model data or to react on null-tolerance
            // as it was done in the old implementation
            rContainer.push_back(
                createHiddenGeometryPrimitives2D(
                    true,
                    getOuterRange()));
        }

        bool SwVirtFlyDrawObjPrimitive::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SwVirtFlyDrawObjPrimitive& rCompare = static_cast<const SwVirtFlyDrawObjPrimitive&>(rPrimitive);

                return (&getSwVirtFlyDrawObj() == &rCompare.getSwVirtFlyDrawObj()
                    && getOuterRange() == rCompare.getOuterRange());
            }

            return false;
        }

        basegfx::B2DRange SwVirtFlyDrawObjPrimitive::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            return getOuterRange();
        }

        void SwVirtFlyDrawObjPrimitive::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            // This is the callback to keep the FlyFrame painting in SW alive as long as it
            // is not changed to primitives. This is the method which will be called by the processors
            // when they do not know this primitive (and they do not). Inside wrap_DoPaintObject
            // there needs to be a test that paint is only done during SW repaints (see there).
            // Using this mechanism guarantees the correct Z-Order of the VirtualObject-based FlyFrames.
            getSwVirtFlyDrawObj().wrap_DoPaintObject(rViewInformation);

            // call parent
            BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        }

        // provide unique ID
        sal_uInt32 SwVirtFlyDrawObjPrimitive::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SWVIRTFLYDRAWOBJPRIMITIVE2D;
        }

} // end of namespace drawinglayer::primitive2d

// AW: own sdr::contact::ViewContact (VC) sdr::contact::ViewObjectContact (VOC) needed
// since offset is defined different from SdrVirtObj's sdr::contact::ViewContactOfVirtObj.
// For paint, that offset is used by setting at the OutputDevice; for primitives this is
// not possible since we have no OutputDevice, but define the geometry itself.

namespace sdr::contact
{
        namespace {

        class VCOfSwVirtFlyDrawObj : public ViewContactOfVirtObj
        {
        protected:
            /** This method is responsible for creating the graphical visualisation data
             *
             * @note ONLY based on model data
             */
            virtual void createViewIndependentPrimitive2DSequence(drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const override;

        public:
            /// basic constructor, used from SdrObject.
            explicit VCOfSwVirtFlyDrawObj(SwVirtFlyDrawObj& rObj)
            :   ViewContactOfVirtObj(rObj)
            {
            }

            /// access to SwVirtFlyDrawObj
            SwVirtFlyDrawObj& GetSwVirtFlyDrawObj() const
            {
                return static_cast<SwVirtFlyDrawObj&>(mrObject);
            }
        };

        }
} // end of namespace sdr::contact

namespace sdr::contact
{
        void VCOfSwVirtFlyDrawObj::createViewIndependentPrimitive2DSequence(drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
        {
            const SdrObject& rReferencedObject = GetSwVirtFlyDrawObj().GetReferencedObj();

            if(dynamic_cast<const SwFlyDrawObj*>( &rReferencedObject) !=  nullptr)
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

                    rVisitor.visit(xPrimitive);
                }
            }
       }

} // end of namespace sdr::contact

basegfx::B2DRange SwVirtFlyDrawObj::getOuterBound() const
{
    basegfx::B2DRange aOuterRange;
    const SdrObject& rReferencedObject = GetReferencedObj();

    if(dynamic_cast<const SwFlyDrawObj*>( &rReferencedObject) !=  nullptr)
    {
        const SwFlyFrame* pFlyFrame = GetFlyFrame();

        if(pFlyFrame)
        {
            const tools::Rectangle aOuterRectangle(pFlyFrame->getFrameArea().Pos(), pFlyFrame->getFrameArea().SSize());

            if(!aOuterRectangle.IsEmpty())
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

    if(dynamic_cast<const SwFlyDrawObj*>( &rReferencedObject) !=  nullptr)
    {
        const SwFlyFrame* pFlyFrame = GetFlyFrame();

        if(pFlyFrame)
        {
            const tools::Rectangle aInnerRectangle(pFlyFrame->getFrameArea().Pos() + pFlyFrame->getFramePrintArea().Pos(), pFlyFrame->getFramePrintArea().SSize());

            if(!aInnerRectangle.IsEmpty())
            {
                aInnerRange.expand(basegfx::B2DTuple(aInnerRectangle.Left(), aInnerRectangle.Top()));
                aInnerRange.expand(basegfx::B2DTuple(aInnerRectangle.Right(), aInnerRectangle.Bottom()));
            }
        }
    }

    return aInnerRange;
}

bool SwVirtFlyDrawObj::ContainsSwGrfNode() const
{
    // RotGrfFlyFrame: Check if this is a SwGrfNode
    const SwFlyFrame* pFlyFrame(GetFlyFrame());

    if(nullptr != pFlyFrame && pFlyFrame->Lower() && pFlyFrame->Lower()->IsNoTextFrame())
    {
        const SwNoTextFrame *const pNTF(static_cast<const SwNoTextFrame*>(pFlyFrame->Lower()));

        const SwGrfNode *const pGrfNd(pNTF->GetNode()->GetGrfNode());

        return nullptr != pGrfNd;
    }

    return false;
}

bool SwVirtFlyDrawObj::HasLimitedRotation() const
{
    // RotGrfFlyFrame: If true, this SdrObject supports only limited rotation.
    // This is the case for SwGrfNode instances
    return ContainsSwGrfNode();
}

void SwVirtFlyDrawObj::Rotate(const Point& rRef, Degree100 nAngle100, double sn, double cs)
{
    if(ContainsSwGrfNode())
    {
        // RotGrfFlyFrame: Here is where the positively completed rotate interaction is executed.
        // Rotation is in 1/100th degree and may be signed (!)
        Degree10 nAngle10 = to<Degree10>(nAngle100);

        while(nAngle10 < 0_deg10)
        {
            nAngle10 += 3600_deg10;
        }

        SwWrtShell *pShForAngle = nAngle10 ? dynamic_cast<SwWrtShell*>(GetFlyFrame()->getRootFrame()->GetCurrShell()) : nullptr;
        if (pShForAngle)
        {
            // RotGrfFlyFrame: Add transformation to placeholder object
            Size aSize;
            const Degree10 nOldRot(SwVirtFlyDrawObj::getPossibleRotationFromFraphicFrame(aSize));
            SwFlyFrameAttrMgr aMgr(false, pShForAngle, Frmmgr_Type::NONE, nullptr);

            aMgr.SetRotation(nOldRot, (nOldRot + nAngle10) % 3600_deg10, aSize);
        }
    }
    else
    {
        // call parent
        SdrVirtObj::Rotate(rRef, nAngle100, sn, cs);
    }
}

std::unique_ptr<sdr::contact::ViewContact> SwVirtFlyDrawObj::CreateObjectSpecificViewContact()
{
    // need an own ViewContact (VC) to allow creation of a specialized primitive
    // for being able to visualize the FlyFrames in primitive renderers
    return std::make_unique<sdr::contact::VCOfSwVirtFlyDrawObj>(*this);
}

SwVirtFlyDrawObj::SwVirtFlyDrawObj(
    SdrModel& rSdrModel,
    SdrObject& rNew,
    SwFlyFrame* pFly)
:   SdrVirtObj(rSdrModel, rNew),
    m_pFlyFrame(pFly)
{
    const SvxProtectItem &rP = m_pFlyFrame->GetFormat()->GetProtect();
    m_bMovProt = rP.IsPosProtected();
    m_bSizProt = rP.IsSizeProtected();
}

SwVirtFlyDrawObj::~SwVirtFlyDrawObj()
{
    if ( getSdrPageFromSdrObject() )    //Withdraw SdrPage the responsibility.
        getSdrPageFromSdrObject()->RemoveObject( GetOrdNum() );
}

const SwFrameFormat *SwVirtFlyDrawObj::GetFormat() const
{
    return GetFlyFrame()->GetFormat();
}
SwFrameFormat *SwVirtFlyDrawObj::GetFormat()
{
    return GetFlyFrame()->GetFormat();
}

// --> OD #i102707#
namespace
{
    class RestoreMapMode
    {
        public:
            explicit RestoreMapMode( SwViewShell const * pViewShell )
                : mbMapModeRestored( false )
                , mpOutDev( pViewShell->GetOut() )
            {
                if ( pViewShell->getPrePostMapMode() == mpOutDev->GetMapMode() )
                    return;

                mpOutDev->Push(vcl::PushFlags::MAPMODE);

                GDIMetaFile* pMetaFile = mpOutDev->GetConnectMetaFile();
                if ( pMetaFile &&
                     pMetaFile->IsRecord() && !pMetaFile->IsPause() )
                {
                    OSL_FAIL( "MapMode restoration during meta file creation is somehow suspect - using <SetRelativeMapMode(..)>, but not sure, if correct." );
                    mpOutDev->SetRelativeMapMode( pViewShell->getPrePostMapMode() );
                }
                else
                {
                    mpOutDev->SetMapMode( pViewShell->getPrePostMapMode() );
                }

                mbMapModeRestored = true;
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
            VclPtr<OutputDevice> mpOutDev;
    };
}
// <--

void SwVirtFlyDrawObj::wrap_DoPaintObject(
    drawinglayer::geometry::ViewInformation2D const& rViewInformation) const
{
    SwViewShell* pShell = m_pFlyFrame->getRootFrame()->GetCurrShell();

    // Only paint when we have a current shell and a DrawingLayer paint is in progress.
    // This avoids evtl. problems with renderers which do processing stuff,
    // but no paints. IsPaintInProgress() depends on SW repaint, so, as long
    // as SW paints self and calls DrawLayer() for Heaven and Hell, this will
    // be correct
    if ( !(pShell && pShell->IsDrawingLayerPaintInProgress()) )
        return;

    bool bDrawObject(true);

    if ( !SwFlyFrame::IsPaint( const_cast<SwVirtFlyDrawObj*>(this), pShell ) )
    {
        bDrawObject = false;
    }

    if ( !bDrawObject )
        return;

    // if there's no viewport set, all fly-frames will be painted,
    // which is slow, wastes memory, and can cause other trouble.
    (void) rViewInformation; // suppress "unused parameter" warning
    assert(comphelper::LibreOfficeKit::isActive() || !rViewInformation.getViewport().isEmpty());
    if ( m_pFlyFrame->IsFlyInContentFrame() )
        return;

    // it is also necessary to restore the VCL MapMode from ViewInformation since e.g.
    // the VCL PixelRenderer resets it at the used OutputDevice. Unfortunately, this
    // excludes shears and rotates which are not expressible in MapMode.
    // OD #i102707#
    // new helper class to restore MapMode - restoration, only if
    // needed and consideration of paint for meta file creation .
    RestoreMapMode aRestoreMapModeIfNeeded( pShell );

    // paint the FlyFrame (use standard VCL-Paint)
    m_pFlyFrame->PaintSwFrame( *pShell->GetOut(), m_pFlyFrame->GetPageFrame()->getFrameArea());
}

void SwVirtFlyDrawObj::TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const
{
    rInfo.bMoveAllowed =
    rInfo.bResizeFreeAllowed = rInfo.bResizePropAllowed = true;

    // RotGrfFlyFrame: Some rotation may be allowed
    rInfo.bRotateFreeAllowed = rInfo.bRotate90Allowed = HasLimitedRotation();

    rInfo.bMirrorFreeAllowed = rInfo.bMirror45Allowed =
    rInfo.bMirror90Allowed   = rInfo.bShearAllowed    =
    rInfo.bCanConvToPath     = rInfo.bCanConvToPoly   =
    rInfo.bCanConvToPathLineToArea = rInfo.bCanConvToPolyLineToArea = false;
}

// SwVirtFlyDrawObj - Size Determination

void SwVirtFlyDrawObj::SetRect() const
{
    if ( GetFlyFrame()->getFrameArea().HasArea() )
        const_cast<SwVirtFlyDrawObj*>(this)->m_aOutRect = GetFlyFrame()->getFrameArea().SVRect();
    else
        const_cast<SwVirtFlyDrawObj*>(this)->m_aOutRect = tools::Rectangle();
}

const tools::Rectangle& SwVirtFlyDrawObj::GetCurrentBoundRect() const
{
    SetRect();
    return m_aOutRect;
}

const tools::Rectangle& SwVirtFlyDrawObj::GetLastBoundRect() const
{
    return GetCurrentBoundRect();
}

void SwVirtFlyDrawObj::RecalcBoundRect()
{
    SetRect();
}

void SwVirtFlyDrawObj::RecalcSnapRect()
{
    SetRect();
}

const tools::Rectangle& SwVirtFlyDrawObj::GetSnapRect()  const
{
    SetRect();
    return m_aOutRect;
}

void SwVirtFlyDrawObj::SetSnapRect(const tools::Rectangle& )
{
    tools::Rectangle aTmp( GetLastBoundRect() );
    SetRect();
    SetChanged();
    BroadcastObjectChange();
    if (m_pUserCall!=nullptr)
        m_pUserCall->Changed(*this, SdrUserCallType::Resize, aTmp);
}

void SwVirtFlyDrawObj::NbcSetSnapRect(const tools::Rectangle& )
{
    SetRect();
}

const tools::Rectangle& SwVirtFlyDrawObj::GetLogicRect() const
{
    SetRect();
    return m_aOutRect;
}

void SwVirtFlyDrawObj::SetLogicRect(const tools::Rectangle& )
{
    tools::Rectangle aTmp( GetLastBoundRect() );
    SetRect();
    SetChanged();
    BroadcastObjectChange();
    if (m_pUserCall!=nullptr)
        m_pUserCall->Changed(*this, SdrUserCallType::Resize, aTmp);
}

void SwVirtFlyDrawObj::NbcSetLogicRect(const tools::Rectangle& )
{
    SetRect();
}

::basegfx::B2DPolyPolygon SwVirtFlyDrawObj::TakeXorPoly() const
{
    const tools::Rectangle aSourceRectangle(GetFlyFrame()->getFrameArea().SVRect());
    const ::basegfx::B2DRange aSourceRange = vcl::unotools::b2DRectangleFromRectangle(aSourceRectangle);
    ::basegfx::B2DPolyPolygon aRetval;

    aRetval.append(::basegfx::utils::createPolygonFromRect(aSourceRange));

    return aRetval;
}

//  SwVirtFlyDrawObj::Move() and Resize()
void SwVirtFlyDrawObj::NbcMove(const Size& rSiz)
{
    if(GetFlyFrame()->IsFlyFreeFrame() && static_cast< SwFlyFreeFrame* >(GetFlyFrame())->isTransformableSwFrame())
    {
        // RotateFlyFrame3: When we have a change and are in transformed state (e.g. rotation used),
        // we need to fall back to the un-transformed state to keep the old code below
        // working properly. Restore FrameArea and use aOutRect from old FrameArea.
        TransformableSwFrame* pTransformableSwFrame(static_cast<SwFlyFreeFrame*>(GetFlyFrame())->getTransformableSwFrame());
        pTransformableSwFrame->restoreFrameAreas();
        m_aOutRect = GetFlyFrame()->getFrameArea().SVRect();
    }

    m_aOutRect.Move( rSiz );
    const Point aOldPos( GetFlyFrame()->getFrameArea().Pos() );
    const Point aNewPos( m_aOutRect.TopLeft() );
    const SwRect aFlyRect( m_aOutRect );

    //If the Fly has an automatic align (right or top),
    //so preserve the automatic.
    SwFrameFormat *pFormat = GetFlyFrame()->GetFormat();
    const sal_Int16 eHori = pFormat->GetHoriOrient().GetHoriOrient();
    const sal_Int16 eVert = pFormat->GetVertOrient().GetVertOrient();
    const sal_Int16 eRelHori = pFormat->GetHoriOrient().GetRelationOrient();
    const sal_Int16 eRelVert = pFormat->GetVertOrient().GetRelationOrient();
    //On paragraph bound Flys starting from the new position a new
    //anchor must be set. Anchor and the new RelPos is calculated and
    //placed by the Fly itself.
    if( GetFlyFrame()->IsFlyAtContentFrame() )
    {
        static_cast<SwFlyAtContentFrame*>(GetFlyFrame())->SetAbsPos( aNewPos );
    }
    else
    {
        const SwFrameFormat *pTmpFormat = GetFormat();
        const SwFormatVertOrient &rVert = pTmpFormat->GetVertOrient();
        const SwFormatHoriOrient &rHori = pTmpFormat->GetHoriOrient();
        tools::Long lXDiff = aNewPos.X() - aOldPos.X();
        if( rHori.IsPosToggle() && text::HoriOrientation::NONE == eHori &&
            !GetFlyFrame()->FindPageFrame()->OnRightPage() )
            lXDiff = -lXDiff;

        if( GetFlyFrame()->GetAnchorFrame()->IsRightToLeft() &&
            text::HoriOrientation::NONE == eHori )
            lXDiff = -lXDiff;

        tools::Long lYDiff = aNewPos.Y() - aOldPos.Y();
        if( GetFlyFrame()->GetAnchorFrame()->IsVertical() )
        {
            //lXDiff -= rVert.GetPos();
            //lYDiff += rHori.GetPos();

            if ( GetFlyFrame()->GetAnchorFrame()->IsVertLR() )
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

        if( GetFlyFrame()->GetAnchorFrame()->IsRightToLeft() &&
            text::HoriOrientation::NONE != eHori )
            lXDiff = GetFlyFrame()->GetAnchorFrame()->getFrameArea().Width() -
                     aFlyRect.Width() - lXDiff;

        const Point aTmp( lXDiff, lYDiff );
        GetFlyFrame()->ChgRelPos( aTmp );
    }

    SwAttrSet aSet( pFormat->GetDoc()->GetAttrPool(),
                                            RES_VERT_ORIENT, RES_HORI_ORIENT );
    SwFormatHoriOrient aHori( pFormat->GetHoriOrient() );
    SwFormatVertOrient aVert( pFormat->GetVertOrient() );
    bool bPut = false;

    if( !GetFlyFrame()->IsFlyLayFrame() &&
        ::GetHtmlMode(pFormat->GetDoc()->GetDocShell()) )
    {
        //In HTML-Mode only automatic aligns are allowed.
        //Only we can try a snap to left/right respectively left-/right border
        const SwFrame* pAnch = GetFlyFrame()->GetAnchorFrame();
        bool bNextLine = false;

        if( !GetFlyFrame()->IsAutoPos() || text::RelOrientation::PAGE_FRAME != aHori.GetRelationOrient() )
        {
            if( text::RelOrientation::CHAR == eRelHori )
            {
                aHori.SetHoriOrient( text::HoriOrientation::LEFT );
                aHori.SetRelationOrient( text::RelOrientation::CHAR );
            }
            else
            {
                bNextLine = true;
                //Horizontal Align:
                const bool bLeftFrame =
                    aFlyRect.Left() < pAnch->getFrameArea().Left() + pAnch->getFramePrintArea().Left(),
                    bLeftPrt = aFlyRect.Left() + aFlyRect.Width() <
                               pAnch->getFrameArea().Left() + pAnch->getFramePrintArea().Width()/2;
                if ( bLeftFrame || bLeftPrt )
                {
                    aHori.SetHoriOrient( text::HoriOrientation::LEFT );
                    aHori.SetRelationOrient( bLeftFrame ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
                }
                else
                {
                    const bool bRightFrame = aFlyRect.Left() >
                                       pAnch->getFrameArea().Left() + pAnch->getFramePrintArea().Width();
                    aHori.SetHoriOrient( text::HoriOrientation::RIGHT );
                    aHori.SetRelationOrient( bRightFrame ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
                }
            }
            aSet.Put( aHori );
        }
        //Vertical alignment simply is retained principally,
        //only on manual align will be switched over.
        bool bRelChar = text::RelOrientation::CHAR == eRelVert;
        aVert.SetVertOrient( eVert != text::VertOrientation::NONE ? eVert :
                GetFlyFrame()->IsFlyInContentFrame() ? text::VertOrientation::CHAR_CENTER :
                bRelChar && bNextLine ? text::VertOrientation::CHAR_TOP : text::VertOrientation::TOP );
        if( bRelChar )
            aVert.SetRelationOrient( text::RelOrientation::CHAR );
        else
            aVert.SetRelationOrient( text::RelOrientation::PRINT_AREA );
        aSet.Put( aVert );
        bPut = true;
    }

    //We want preferably not to lose the automatic alignments.
    if ( !bPut && bInResize )
    {
        if ( text::HoriOrientation::NONE != eHori )
        {
            aHori.SetHoriOrient( eHori );
            aHori.SetRelationOrient( eRelHori );
            aSet.Put( aHori );
            bPut = true;
        }
        if ( text::VertOrientation::NONE != eVert )
        {
            aVert.SetVertOrient( eVert );
            aVert.SetRelationOrient( eRelVert );
            aSet.Put( aVert );
            bPut = true;
        }
    }
    if ( bPut )
        pFormat->SetFormatAttr( aSet );
}


void SwVirtFlyDrawObj::NbcCrop(const basegfx::B2DPoint& rRef, double fxFact, double fyFact)
{
    // Get Wrt Shell
    SwWrtShell *pSh = dynamic_cast<SwWrtShell*>( GetFlyFrame()->getRootFrame()->GetCurrShell() );

    if (!pSh)
    {
        return;
    }

    GraphicObject const *pGraphicObject = pSh->GetGraphicObj();

    if (!pGraphicObject)
    {
        return;
    }

    // Get graphic object size in 100th of mm
    const MapMode aMapMode100thmm(MapUnit::Map100thMM);
    Size aGraphicSize(pGraphicObject->GetPrefSize());

    if( MapUnit::MapPixel == pGraphicObject->GetPrefMapMode().GetMapUnit() )
    {
        aGraphicSize = Application::GetDefaultDevice()->PixelToLogic( aGraphicSize, aMapMode100thmm );
    }
    else
    {
        aGraphicSize = OutputDevice::LogicToLogic( aGraphicSize, pGraphicObject->GetPrefMapMode(), aMapMode100thmm);
    }

    if( aGraphicSize.IsEmpty() )
    {
        return ;
    }

    const bool bIsTransformableSwFrame(
        GetFlyFrame()->IsFlyFreeFrame() &&
        static_cast< SwFlyFreeFrame* >(GetFlyFrame())->isTransformableSwFrame());

    if(bIsTransformableSwFrame)
    {
        // When we have a change and are in transformed state (e.g. rotation used),
        // we need to fall back to the un-transformed state to keep the old code below
        // working properly. Restore FrameArea and use aOutRect from old FrameArea.
        TransformableSwFrame* pTransformableSwFrame(static_cast<SwFlyFreeFrame*>(GetFlyFrame())->getTransformableSwFrame());
        pTransformableSwFrame->restoreFrameAreas();
        m_aOutRect = GetFlyFrame()->getFrameArea().SVRect();
    }

    // Compute old and new rect. This will give us the deformation to apply to
    // the object to crop. OldRect is the inner frame, see getFullDragClone()
    // below where getFramePrintAreaTransformation is used as object geometry for Crop
    const tools::Rectangle aOldRect(
        GetFlyFrame()->getFrameArea().TopLeft() + GetFlyFrame()->getFramePrintArea().TopLeft(),
        GetFlyFrame()->getFramePrintArea().SSize());
    const tools::Long nOldWidth(aOldRect.GetWidth());
    const tools::Long nOldHeight(aOldRect.GetHeight());

    if (!nOldWidth || !nOldHeight)
    {
        return;
    }

    // rRef is relative to the Crop-Action, si in X/Y-Ranges of [0.0 .. 1.0],
    // to get the correct absolute position, transform using the old Rect
    const Point aRef(
        aOldRect.Left() + basegfx::fround(aOldRect.GetWidth() * rRef.getX()),
        aOldRect.Top() + basegfx::fround(aOldRect.GetHeight() * rRef.getY()));

    // apply transformation, use old ResizeRect for now
    tools::Rectangle aNewRect( aOldRect );
    ResizeRect(
        aNewRect,
        aRef,
        Fraction(fxFact),
        Fraction(fyFact));

    // Get old values for crop in 10th of mm
    SfxItemSetFixed<RES_GRFATR_CROPGRF, RES_GRFATR_CROPGRF> aSet( pSh->GetAttrPool() );
    pSh->GetCurAttr( aSet );
    SwCropGrf aCrop( aSet.Get(RES_GRFATR_CROPGRF) );

    tools::Rectangle aCropRectangle(
        convertTwipToMm100(aCrop.GetLeft()),
        convertTwipToMm100(aCrop.GetTop()),
        convertTwipToMm100(aCrop.GetRight()),
        convertTwipToMm100(aCrop.GetBottom()) );

    // Compute delta to apply
    double fScaleX = ( aGraphicSize.Width() - aCropRectangle.Left() - aCropRectangle.Right() ) / static_cast<double>(nOldWidth);
    double fScaleY = ( aGraphicSize.Height() - aCropRectangle.Top() - aCropRectangle.Bottom() ) / static_cast<double>(nOldHeight);

    sal_Int32 nDiffLeft = aNewRect.Left() - aOldRect.Left();
    sal_Int32 nDiffTop = aNewRect.Top() - aOldRect.Top();
    sal_Int32 nDiffRight = aNewRect.Right() - aOldRect.Right();
    sal_Int32 nDiffBottom = aNewRect.Bottom() - aOldRect.Bottom();

    // Compute new values in 10th of mm
    sal_Int32 nLeftCrop = static_cast<sal_Int32>( aCropRectangle.Left() + nDiffLeft * fScaleX );
    sal_Int32 nTopCrop = static_cast<sal_Int32>( aCropRectangle.Top() + nDiffTop * fScaleY );
    sal_Int32 nRightCrop = static_cast<sal_Int32>( aCropRectangle.Right() - nDiffRight * fScaleX );
    sal_Int32 nBottomCrop = static_cast<sal_Int32>( aCropRectangle.Bottom() - nDiffBottom * fScaleY );

    // Apply values
    pSh->StartAllAction();
    // pSh->StartUndo(SwUndoId::START);

    // Set new crop values in twips
    aCrop.SetLeft  (o3tl::toTwips(nLeftCrop, o3tl::Length::mm100));
    aCrop.SetTop   (o3tl::toTwips(nTopCrop, o3tl::Length::mm100));
    aCrop.SetRight (o3tl::toTwips(nRightCrop, o3tl::Length::mm100));
    aCrop.SetBottom(o3tl::toTwips(nBottomCrop, o3tl::Length::mm100));
    pSh->SetAttrItem(aCrop);

    // Set new frame size
    SwFrameFormat *pFormat = GetFormat();
    SwFormatFrameSize aSz( pFormat->GetFrameSize() );
    const tools::Long aNewWidth(aNewRect.GetWidth() + (m_aOutRect.GetWidth() - aOldRect.GetWidth()));
    const tools::Long aNewHeight(aNewRect.GetHeight() + (m_aOutRect.GetHeight() - aOldRect.GetHeight()));
    aSz.SetWidth(aNewWidth);
    aSz.SetHeight(aNewHeight);
    pFormat->GetDoc()->SetAttr( aSz, *pFormat );

    // add move - to make result look better. Fill with defaults
    // for the untransformed case
    Point aNewTopLeft(aNewRect.TopLeft());
    const Point aOldTopLeft(aOldRect.TopLeft());

    if(bIsTransformableSwFrame)
    {
        // Need to correct the NewTopLeft position in transformed state to make
        // the interaction look correct. First, extract rotation
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;
        GetFlyFrame()->getFrameAreaTransformation().decompose(aScale, aTranslate, fRotate, fShearX);

        // calc the center of the unchanged object
        const basegfx::B2DPoint aFormerCenter(
            GetFlyFrame()->getFrameAreaTransformation() * basegfx::B2DPoint(0.5, 0.5));

        // define the existing rotation around that former center
        const basegfx::B2DHomMatrix aRotFormerCenter(
            basegfx::utils::createRotateAroundPoint(
                aFormerCenter.getX(),
                aFormerCenter.getY(),
                fRotate));

        // use the new center of the unrotated object, rotate it around the
        // former center
        const Point aNewCenter(aNewRect.Center());
        const basegfx::B2DPoint aRotNewCenter(
            aRotFormerCenter * basegfx::B2DPoint(aNewCenter.X(), aNewCenter.Y()));

        // Create the new TopLeft of the unrotated, cropped object by creating
        // as if re-creating the unrotated geometry
        aNewTopLeft = Point(
            basegfx::fround(aRotNewCenter.getX() - (0.5 * aNewRect.getWidth())),
            basegfx::fround(aRotNewCenter.getY() - (0.5 * aNewRect.getHeight())));
    }

    // check if we have movement and execute if yes
    const Size aDeltaMove(
        aNewTopLeft.X() - aOldTopLeft.X(),
        aNewTopLeft.Y() - aOldTopLeft.Y());

    if(0 != aDeltaMove.Width() || 0 != aDeltaMove.Height())
    {
        NbcMove(aDeltaMove);
    }

    // pSh->EndUndo(SwUndoId::END);
    pSh->EndAllAction();
}

void SwVirtFlyDrawObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    const SwFrame* pTmpFrame = GetFlyFrame()->GetAnchorFrame();

    if( !pTmpFrame )
    {
        pTmpFrame = GetFlyFrame();
    }

    const bool bVertX(pTmpFrame->IsVertical());
    const bool bRTL(pTmpFrame->IsRightToLeft());
    const bool bVertL2RX(pTmpFrame->IsVertLR());
    const bool bUseRightEdge((bVertX && !bVertL2RX ) || bRTL);
    const bool bIsTransformableSwFrame(
        GetFlyFrame()->IsFlyFreeFrame() &&
        static_cast< SwFlyFreeFrame* >(GetFlyFrame())->isTransformableSwFrame());

    if(bIsTransformableSwFrame)
    {
        // When we have a change in transformed state, we need to fall back to the
        // state without possible transformations.
        // In the Resize case to correctly handle the changes, apply to the transformation
        // and extract the new, untransformed state from that modified transformation
        basegfx::B2DHomMatrix aNewMat(GetFlyFrame()->getFrameAreaTransformation());
        const basegfx::B2DPoint aRef(rRef.X(), rRef.Y());

        // apply state to already valid transformation
        aNewMat.translate(-aRef.getX(), -aRef.getY());
        aNewMat.scale(double(xFact), double(yFact));
        aNewMat.translate(aRef.getX(), aRef.getY());

        // get center of transformed state
        const basegfx::B2DPoint aCenter(aNewMat * basegfx::B2DPoint(0.5, 0.5));

        // decompose to extract scale
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;
        aNewMat.decompose(aScale, aTranslate, fRotate, fShearX);
        const basegfx::B2DVector aAbsScale(basegfx::absolute(aScale));

        // create new modified, but untransformed OutRect
        m_aOutRect = tools::Rectangle(
            basegfx::fround(aCenter.getX() - (0.5 * aAbsScale.getX())),
            basegfx::fround(aCenter.getY() - (0.5 * aAbsScale.getY())),
            basegfx::fround(aCenter.getX() + (0.5 * aAbsScale.getX())),
            basegfx::fround(aCenter.getY() + (0.5 * aAbsScale.getY())));

        // restore FrameAreas so that actions below not adapted to new
        // full transformations take the correct actions
        TransformableSwFrame* pTransformableSwFrame(static_cast<SwFlyFreeFrame*>(GetFlyFrame())->getTransformableSwFrame());
        pTransformableSwFrame->restoreFrameAreas();
    }
    else
    {
        ResizeRect( m_aOutRect, rRef, xFact, yFact );
    }

    // Position may also change, remember old one. This is now already
    // the one in the unrotated, old coordinate system
    Point aOldPos(bUseRightEdge ? GetFlyFrame()->getFrameArea().TopRight() : GetFlyFrame()->getFrameArea().Pos());

    // get target size in old coordinate system
    Size aSz( m_aOutRect.Right() - m_aOutRect.Left() + 1, m_aOutRect.Bottom()- m_aOutRect.Top()  + 1 );

    // compare with restored FrameArea
    if( aSz != GetFlyFrame()->getFrameArea().SSize() )
    {
        //The width of the columns should not be too narrow
        if ( GetFlyFrame()->Lower() && GetFlyFrame()->Lower()->IsColumnFrame() )
        {
            SwBorderAttrAccess aAccess( SwFrame::GetCache(), GetFlyFrame() );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            tools::Long nMin = rAttrs.CalcLeftLine()+rAttrs.CalcRightLine();
            const SwFormatCol& rCol = rAttrs.GetAttrSet().GetCol();
            if ( rCol.GetColumns().size() > 1 )
            {
                for ( const auto &rC : rCol.GetColumns() )
                {
                    nMin += rC.GetLeft() + rC.GetRight() + MINFLY;
                }
                nMin -= MINFLY;
            }
            aSz.setWidth( std::max( aSz.Width(), nMin ) );
        }

        SwFrameFormat *pFormat = GetFormat();
        const SwFormatFrameSize aOldFrameSz( pFormat->GetFrameSize() );
        GetFlyFrame()->ChgSize( aSz );
        SwFormatFrameSize aFrameSz( pFormat->GetFrameSize() );

        if ( aFrameSz.GetWidthPercent() || aFrameSz.GetHeightPercent() )
        {
            tools::Long nRelWidth, nRelHeight;
            const SwFrame *pRel = GetFlyFrame()->IsFlyLayFrame() ?
                                GetFlyFrame()->GetAnchorFrame() :
                                GetFlyFrame()->GetAnchorFrame()->GetUpper();
            const SwViewShell *pSh = GetFlyFrame()->getRootFrame()->GetCurrShell();

            if ( pSh && pRel->IsBodyFrame() &&
                 pSh->GetViewOptions()->getBrowseMode() &&
                 pSh->VisArea().HasArea() )
            {
                nRelWidth  = pSh->GetBrowseWidth();
                nRelHeight = pSh->VisArea().Height();
                const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
                nRelHeight -= 2*aBorder.Height();
            }
            else
            {
                nRelWidth  = pRel->getFramePrintArea().Width();
                nRelHeight = pRel->getFramePrintArea().Height();
            }

            if ( aFrameSz.GetWidthPercent() && aFrameSz.GetWidthPercent() != SwFormatFrameSize::SYNCED &&
                 aOldFrameSz.GetWidth() != aFrameSz.GetWidth() )
            {
                aFrameSz.SetWidthPercent( sal_uInt8(aSz.Width() * 100.0 / nRelWidth + 0.5) );
            }

            if ( aFrameSz.GetHeightPercent() && aFrameSz.GetHeightPercent() != SwFormatFrameSize::SYNCED &&
                 aOldFrameSz.GetHeight() != aFrameSz.GetHeight() )
            {
                aFrameSz.SetHeightPercent( sal_uInt8(aSz.Height() * 100.0 / nRelHeight + 0.5) );
            }

            pFormat->GetDoc()->SetAttr( aFrameSz, *pFormat );
        }
    }

    //Position can also be changed, get new one
    const Point aNewPos(bUseRightEdge ? m_aOutRect.Right() + 1 : m_aOutRect.Left(), m_aOutRect.Top());

    if ( aNewPos == aOldPos )
        return;

    // Former late change in aOutRect by ChgSize
    // is now taken into account directly by calculating
    // aNewPos *after* calling ChgSize (see old code).
    // Still need to adapt aOutRect since the 'Move' is already applied
    // here (see ResizeRect) and it's the same SdrObject
    const Size aDeltaMove(
            aNewPos.X() - aOldPos.X(),
            aNewPos.Y() - aOldPos.Y());
    m_aOutRect.Move(-aDeltaMove.Width(), -aDeltaMove.Height());

    // Now, move as needed (no empty delta which was a hack anyways)
    if(bIsTransformableSwFrame)
    {
        // need to save aOutRect to FrameArea, will be restored to aOutRect in
        // SwVirtFlyDrawObj::NbcMove currently for TransformableSwFrames
        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*GetFlyFrame());
        aFrm.setSwRect(SwRect(m_aOutRect));
    }

    // keep old hack - not clear what happens here
    bInResize = true;
    NbcMove(aDeltaMove);
    bInResize = false;
}

void SwVirtFlyDrawObj::Move(const Size& rSiz)
{
    NbcMove( rSiz );
    SetChanged();
    GetFormat()->GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false);
}

void SwVirtFlyDrawObj::Resize(const Point& rRef,
                    const Fraction& xFact, const Fraction& yFact, bool /*bUnsetRelative*/)
{
    NbcResize( rRef, xFact, yFact );
    SetChanged();
    GetFormat()->GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false);
}

void SwVirtFlyDrawObj::Crop(const basegfx::B2DPoint& rRef, double fxFact, double fyFact)
{
    NbcCrop( rRef, fxFact, fyFact );
    SetChanged();
    GetFormat()->GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false);
}

// RotGrfFlyFrame: Helper to access possible rotation of Graphic contained in FlyFrame
Degree10 SwVirtFlyDrawObj::getPossibleRotationFromFraphicFrame(Size& rSize) const
{
    Degree10 nRetval;
    const SwNoTextFrame* pNoTx = dynamic_cast< const SwNoTextFrame* >(GetFlyFrame()->Lower());

    if(pNoTx)
    {
        SwNoTextNode& rNoTNd = const_cast< SwNoTextNode& >(*static_cast<const SwNoTextNode*>(pNoTx->GetNode()));
        SwGrfNode* pGrfNd = rNoTNd.GetGrfNode();

        if(nullptr != pGrfNd)
        {
            const SwAttrSet& rSet = pGrfNd->GetSwAttrSet();
            const SwRotationGrf& rRotation = rSet.GetRotationGrf();

            rSize = rRotation.GetUnrotatedSize();
            nRetval = rRotation.GetValue();
        }
    }

    return nRetval;
}

Degree100 SwVirtFlyDrawObj::GetRotateAngle() const
{
    if(ContainsSwGrfNode())
    {
        Size aSize;
        return to<Degree100>(getPossibleRotationFromFraphicFrame(aSize));
    }
    else
    {
        return SdrVirtObj::GetRotateAngle();
    }
}

SdrObjectUniquePtr SwVirtFlyDrawObj::getFullDragClone() const
{
    // call parent
    SdrObjectUniquePtr pRetval = SdrVirtObj::getFullDragClone();

    if(pRetval && GetFlyFrame() && ContainsSwGrfNode())
    {
        // RotGrfFlyFrame3: get inner bounds/transformation
        const basegfx::B2DHomMatrix aTargetTransform(GetFlyFrame()->getFramePrintAreaTransformation());

        pRetval->TRSetBaseGeometry(aTargetTransform, basegfx::B2DPolyPolygon());
    }

    return pRetval;
}

void SwVirtFlyDrawObj::addCropHandles(SdrHdlList& rTarget) const
{
    // RotGrfFlyFrame: Adapt to possible rotated Graphic contained in FlyFrame
    if(!GetFlyFrame()->getFrameArea().HasArea())
        return;

    // Use InnerBound, OuterBound (same as GetFlyFrame()->getFrameArea().SVRect())
    // may have a distance to InnerBound which needs to be taken into account.
    // The Graphic is mapped to InnerBound, as is the rotated Graphic.
    const basegfx::B2DRange aTargetRange(getInnerBound());

    if(aTargetRange.isEmpty())
        return;

    // RotGrfFlyFrame3: get inner bounds/transformation
    const basegfx::B2DHomMatrix aTargetTransform(GetFlyFrame()->getFramePrintAreaTransformation());

    // break up matrix
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate(0.0);
    double fShearX(0.0);
    aTargetTransform.decompose(aScale, aTranslate, fRotate, fShearX);
    basegfx::B2DPoint aPos;

    aPos = aTargetTransform * basegfx::B2DPoint(0.0, 0.0);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::UpperLeft, fShearX, fRotate));
    aPos = aTargetTransform * basegfx::B2DPoint(0.5, 0.0);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::Upper, fShearX, fRotate));
    aPos = aTargetTransform * basegfx::B2DPoint(1.0, 0.0);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::UpperRight, fShearX, fRotate));
    aPos = aTargetTransform * basegfx::B2DPoint(0.0, 0.5);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::Left , fShearX, fRotate));
    aPos = aTargetTransform * basegfx::B2DPoint(1.0, 0.5);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::Right, fShearX, fRotate));
    aPos = aTargetTransform * basegfx::B2DPoint(0.0, 1.0);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::LowerLeft, fShearX, fRotate));
    aPos = aTargetTransform * basegfx::B2DPoint(0.5, 1.0);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::Lower, fShearX, fRotate));
    aPos = aTargetTransform * basegfx::B2DPoint(1.0, 1.0);
    rTarget.AddHdl(std::make_unique<SdrCropHdl>(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), SdrHdlKind::LowerRight, fShearX, fRotate));
}

// Macro

PointerStyle  SwVirtFlyDrawObj::GetMacroPointer(
    const SdrObjMacroHitRec& ) const
{
    return PointerStyle::RefHand;
}

bool SwVirtFlyDrawObj::HasMacro() const
{
    const SwFormatURL &rURL = m_pFlyFrame->GetFormat()->GetURL();
    return rURL.GetMap() || !rURL.GetURL().isEmpty();
}

SdrObject* SwVirtFlyDrawObj::CheckMacroHit( const SdrObjMacroHitRec& rRec ) const
{
    const SwFormatURL &rURL = m_pFlyFrame->GetFormat()->GetURL();
    if( rURL.GetMap() || !rURL.GetURL().isEmpty() )
    {
        SwRect aRect;
        if ( m_pFlyFrame->Lower() && m_pFlyFrame->Lower()->IsNoTextFrame() )
        {
            aRect = m_pFlyFrame->getFramePrintArea();
            aRect += m_pFlyFrame->getFrameArea().Pos();
        }
        else
            aRect = m_pFlyFrame->getFrameArea();

        if( aRect.Contains( rRec.aPos ) )
        {
            aRect.Pos().setX(aRect.Pos().getX() + rRec.nTol);
            aRect.Pos().setY(aRect.Pos().getY() + rRec.nTol);
            aRect.AddHeight( -(2 * rRec.nTol) );
            aRect.AddWidth( -(2 * rRec.nTol) );

            if( aRect.Contains( rRec.aPos ) )
            {
                if( !rURL.GetMap() ||
                    m_pFlyFrame->GetFormat()->GetIMapObject( rRec.aPos, m_pFlyFrame ))
                    return const_cast<SwVirtFlyDrawObj*>(this);

                return nullptr;
            }
        }
    }
    return SdrObject::CheckMacroHit( rRec );
}

bool SwVirtFlyDrawObj::IsTextBox() const
{
    return SwTextBoxHelper::isTextBox(GetFormat(), RES_FLYFRMFMT, this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
