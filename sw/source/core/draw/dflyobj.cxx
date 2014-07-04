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

#include "hintids.hxx"
#include <svx/svdtrans.hxx>
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <svx/svdpage.hxx>

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

#include <svx/sdr/properties/defaultproperties.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

// AW: For VCOfDrawVirtObj and stuff
#include <svx/sdr/contact/viewcontactofvirtobj.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <sw_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>

using namespace ::com::sun::star;

static bool bInResize = false;

TYPEINIT1( SwFlyDrawObj, SdrObject )
TYPEINIT1( SwVirtFlyDrawObj, SdrVirtObj )

namespace sdr
{
    namespace contact
    {
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
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const SAL_OVERRIDE;

        public:
            /// basic constructor, used from SdrObject.
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

sdr::properties::BaseProperties* SwFlyDrawObj::CreateObjectSpecificProperties()
{
    // create default properties
    return new sdr::properties::DefaultProperties(*this);
}

sdr::contact::ViewContact* SwFlyDrawObj::CreateObjectSpecificViewContact()
{
    // needs an own VC since createViewIndependentPrimitive2DSequence()
    // is called when RecalcBoundRect() is used
    return new sdr::contact::VCOfSwFlyDrawObj(*this);
}

SwFlyDrawObj::SwFlyDrawObj()
{
}

SwFlyDrawObj::~SwFlyDrawObj()
{
}

// SwFlyDrawObj - Factory-Methods

sal_uInt32 SwFlyDrawObj::GetObjInventor() const
{
    return SWGInventor;
}

sal_uInt16 SwFlyDrawObj::GetObjIdentifier() const
{
    return SwFlyDrawObjIdentifier;
}

// TODO: Need own primitive to get the FlyFrame paint working

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
            /// method which is to be used to implement the local decomposition of a 2D primitive
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const SAL_OVERRIDE;

        public:
            SwVirtFlyDrawObjPrimitive(
                const SwVirtFlyDrawObj& rSwVirtFlyDrawObj,
                const basegfx::B2DRange &rOuterRange)
            :   BufferedDecompositionPrimitive2D(),
                mrSwVirtFlyDrawObj(rSwVirtFlyDrawObj),
                maOuterRange(rOuterRange)
            {
            }

            virtual bool operator==(const BasePrimitive2D& rPrimitive) const SAL_OVERRIDE;

            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const SAL_OVERRIDE;

            // overloaded to allow callbacks to wrap_DoPaintObject
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const SAL_OVERRIDE;

            // data read access
            const SwVirtFlyDrawObj& getSwVirtFlyDrawObj() const { return mrSwVirtFlyDrawObj; }
            const basegfx::B2DRange& getOuterRange() const { return maOuterRange; }

            /// provide unique ID
            DeclPrimitive2DIDBlock()
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

        bool SwVirtFlyDrawObjPrimitive::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SwVirtFlyDrawObjPrimitive& rCompare = (SwVirtFlyDrawObjPrimitive&)rPrimitive;

                return (&getSwVirtFlyDrawObj() == &rCompare.getSwVirtFlyDrawObj()
                    && getOuterRange() == rCompare.getOuterRange());
            }

            return false;
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
        ImplPrimitive2DIDBlock(SwVirtFlyDrawObjPrimitive, PRIMITIVE2D_ID_SWVIRTFLYDRAWOBJPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

// AW: own sdr::contact::ViewContact (VC) sdr::contact::ViewObjectContact (VOC) needed
// since offset is defined different from SdrVirtObj's sdr::contact::ViewContactOfVirtObj.
// For paint, that offset is used by setting at the OutputDevice; for primitives this is
// not possible since we have no OutputDevice, but define the geometry itself.

namespace sdr
{
    namespace contact
    {
        class VCOfSwVirtFlyDrawObj : public ViewContactOfVirtObj
        {
        protected:
            /** This method is responsible for creating the graphical visualisation data
             *
             * @note ONLY based on model data
             */
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const SAL_OVERRIDE;

        public:
            /// basic constructor, used from SdrObject.
            VCOfSwVirtFlyDrawObj(SwVirtFlyDrawObj& rObj)
            :   ViewContactOfVirtObj(rObj)
            {
            }
            virtual ~VCOfSwVirtFlyDrawObj();

            /// access to SwVirtFlyDrawObj
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

            if(rReferencedObject.ISA(SwFlyDrawObj))
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

basegfx::B2DRange SwVirtFlyDrawObj::getOuterBound() const
{
    basegfx::B2DRange aOuterRange;
    const SdrObject& rReferencedObject = GetReferencedObj();

    if(rReferencedObject.ISA(SwFlyDrawObj))
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

    if(rReferencedObject.ISA(SwFlyDrawObj))
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

SwVirtFlyDrawObj::SwVirtFlyDrawObj(SdrObject& rNew, SwFlyFrm* pFly) :
    SdrVirtObj( rNew ),
    pFlyFrm( pFly )
{
    //#110094#-1
    // bNotPersistent = bNeedColorRestore = bWriterFlyFrame = sal_True;
    const SvxProtectItem &rP = pFlyFrm->GetFmt()->GetProtect();
    bMovProt = rP.IsPosProtected();
    bSizProt = rP.IsSizeProtected();
}

SwVirtFlyDrawObj::~SwVirtFlyDrawObj()
{
    if ( GetPage() )    //Withdraw SdrPage the responsibility.
        GetPage()->RemoveObject( GetOrdNum() );
}

const SwFrmFmt *SwVirtFlyDrawObj::GetFmt() const
{
    return GetFlyFrm()->GetFmt();
}
SwFrmFmt *SwVirtFlyDrawObj::GetFmt()
{
    return GetFlyFrm()->GetFmt();
}

// --> OD #i102707#
namespace
{
    class RestoreMapMode
    {
        public:
            explicit RestoreMapMode( SwViewShell* pViewShell )
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
                        OSL_FAIL( "MapMode restoration during meta file creation is somehow suspect - using <SetRelativeMapMode(..)>, but not sure, if correct." );
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
    SwViewShell* pShell = pFlyFrm->getRootFrm()->GetCurrShell();

    // Only paint when we have a current shell and a DrawingLayer paint is in progress.
    // This avcoids evtl. problems with renderers which do processing stuff,
    // but no paints. IsPaintInProgress() depends on SW repaint, so, as long
    // as SW paints self and calls DrawLayer() for Heaven and Hell, this will
    // be correct
    if ( pShell && pShell->IsDrawingLayerPaintInProgress() )
    {
        bool bDrawObject(true);

        if ( !SwFlyFrm::IsPaint( (SdrObject*)this, pShell ) )
        {
            bDrawObject = false;
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

void SwVirtFlyDrawObj::TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const
{
    rInfo.bSelectAllowed     = rInfo.bMoveAllowed =
    rInfo.bResizeFreeAllowed = rInfo.bResizePropAllowed = true;

    rInfo.bRotateFreeAllowed = rInfo.bRotate90Allowed =
    rInfo.bMirrorFreeAllowed = rInfo.bMirror45Allowed =
    rInfo.bMirror90Allowed   = rInfo.bShearAllowed    =
    rInfo.bCanConvToPath     = rInfo.bCanConvToPoly   =
    rInfo.bCanConvToPathLineToArea = rInfo.bCanConvToPolyLineToArea = false;
}

// SwVirtFlyDrawObj - Size Determination

void SwVirtFlyDrawObj::SetRect() const
{
    if ( GetFlyFrm()->Frm().HasArea() )
        ((SwVirtFlyDrawObj*)this)->aOutRect = GetFlyFrm()->Frm().SVRect();
    else
        ((SwVirtFlyDrawObj*)this)->aOutRect = Rectangle();
}

const Rectangle& SwVirtFlyDrawObj::GetCurrentBoundRect() const
{
    SetRect();
    return aOutRect;
}

const Rectangle& SwVirtFlyDrawObj::GetLastBoundRect() const
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

const Rectangle& SwVirtFlyDrawObj::GetSnapRect()  const
{
    SetRect();
    return aOutRect;
}

void SwVirtFlyDrawObj::SetSnapRect(const Rectangle& )
{
    Rectangle aTmp( GetLastBoundRect() );
    SetRect();
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall!=NULL)
        pUserCall->Changed(*this, SDRUSERCALL_RESIZE, aTmp);
}

void SwVirtFlyDrawObj::NbcSetSnapRect(const Rectangle& )
{
    SetRect();
}

const Rectangle& SwVirtFlyDrawObj::GetLogicRect() const
{
    SetRect();
    return aOutRect;
}

void SwVirtFlyDrawObj::SetLogicRect(const Rectangle& )
{
    Rectangle aTmp( GetLastBoundRect() );
    SetRect();
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall!=NULL)
        pUserCall->Changed(*this, SDRUSERCALL_RESIZE, aTmp);
}

void SwVirtFlyDrawObj::NbcSetLogicRect(const Rectangle& )
{
    SetRect();
}

::basegfx::B2DPolyPolygon SwVirtFlyDrawObj::TakeXorPoly() const
{
    const Rectangle aSourceRectangle(GetFlyFrm()->Frm().SVRect());
    const ::basegfx::B2DRange aSourceRange(aSourceRectangle.Left(), aSourceRectangle.Top(), aSourceRectangle.Right(), aSourceRectangle.Bottom());
    ::basegfx::B2DPolyPolygon aRetval;

    aRetval.append(::basegfx::tools::createPolygonFromRect(aSourceRange));

    return aRetval;
}

//  SwVirtFlyDrawObj::Move() und Resize()

void SwVirtFlyDrawObj::NbcMove(const Size& rSiz)
{
    MoveRect( aOutRect, rSiz );
    const Point aOldPos( GetFlyFrm()->Frm().Pos() );
    const Point aNewPos( aOutRect.TopLeft() );
    const SwRect aFlyRect( aOutRect );

    //If the Fly has a automatic align (right or top),
    //so preserve the automatic.
    SwFrmFmt *pFmt = GetFlyFrm()->GetFmt();
    const sal_Int16 eHori = pFmt->GetHoriOrient().GetHoriOrient();
    const sal_Int16 eVert = pFmt->GetVertOrient().GetVertOrient();
    const sal_Int16 eRelHori = pFmt->GetHoriOrient().GetRelationOrient();
    const sal_Int16 eRelVert = pFmt->GetVertOrient().GetRelationOrient();
    //On paragraph bound Flys starting from the new position a new
    //anchor must be set. Anchor and the new RelPos is calculated and
    //placed by the Fly itself.
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
    bool bPut = false;

    if( !GetFlyFrm()->IsFlyLayFrm() &&
        ::GetHtmlMode(pFmt->GetDoc()->GetDocShell()) )
    {
        //In HTML-Mode only automatic aligns are allowed.
        //Only we can try a snap to left/right respectively left-/right border
        const SwFrm* pAnch = GetFlyFrm()->GetAnchorFrm();
        bool bNextLine = false;

        if( !GetFlyFrm()->IsAutoPos() || text::RelOrientation::PAGE_FRAME != aHori.GetRelationOrient() )
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
                const bool bLeftFrm =
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
                    const bool bRightFrm = aFlyRect.Left() >
                                       pAnch->Frm().Left() + pAnch->Prt().Width();
                    aHori.SetHoriOrient( text::HoriOrientation::RIGHT );
                    aHori.SetRelationOrient( bRightFrm ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
                }
            }
            aSet.Put( aHori );
        }
        //Vertical alignment simply is retained principally,
        //only on manual align will be switched over.
        bool bRelChar = text::RelOrientation::CHAR == eRelVert;
        aVert.SetVertOrient( eVert != text::VertOrientation::NONE ? eVert :
                GetFlyFrm()->IsFlyInCntFrm() ? text::VertOrientation::CHAR_CENTER :
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
        pFmt->SetFmtAttr( aSet );
}

void SwVirtFlyDrawObj::NbcResize(const Point& rRef,
            const Fraction& xFact, const Fraction& yFact)
{
    ResizeRect( aOutRect, rRef, xFact, yFact );

    const SwFrm* pTmpFrm = GetFlyFrm()->GetAnchorFrm();
    if( !pTmpFrm )
        pTmpFrm = GetFlyFrm();
    const bool bVertX = pTmpFrm->IsVertical();

    const bool bRTL = pTmpFrm->IsRightToLeft();

    const bool bVertL2RX = pTmpFrm->IsVertLR();
    const Point aNewPos( ( bVertX && !bVertL2RX ) || bRTL ?
                         aOutRect.Right() + 1 :
                         aOutRect.Left(),
                         aOutRect.Top() );

    Size aSz( aOutRect.Right() - aOutRect.Left() + 1,
              aOutRect.Bottom()- aOutRect.Top()  + 1 );
    if( aSz != GetFlyFrm()->Frm().SSize() )
    {
        //The width of the columns should not be too narrow
        if ( GetFlyFrm()->Lower() && GetFlyFrm()->Lower()->IsColumnFrm() )
        {
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), GetFlyFrm() );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            long nMin = rAttrs.CalcLeftLine()+rAttrs.CalcRightLine();
            const SwFmtCol& rCol = rAttrs.GetAttrSet().GetCol();
            if ( rCol.GetColumns().size() > 1 )
            {
                for ( sal_uInt16 i = 0; i < rCol.GetColumns().size(); ++i )
                {
                    nMin += rCol.GetColumns()[i].GetLeft() +
                            rCol.GetColumns()[i].GetRight() +
                            MINFLY;
                }
                nMin -= MINFLY;
            }
            aSz.Width() = std::max( aSz.Width(), nMin );
        }

        SwFrmFmt *pFmt = GetFmt();
        const SwFmtFrmSize aOldFrmSz( pFmt->GetFrmSize() );
        GetFlyFrm()->ChgSize( aSz );
        SwFmtFrmSize aFrmSz( pFmt->GetFrmSize() );
        if ( aFrmSz.GetWidthPercent() || aFrmSz.GetHeightPercent() )
        {
            long nRelWidth, nRelHeight;
            const SwFrm *pRel = GetFlyFrm()->IsFlyLayFrm() ?
                                GetFlyFrm()->GetAnchorFrm() :
                                GetFlyFrm()->GetAnchorFrm()->GetUpper();
            const SwViewShell *pSh = GetFlyFrm()->getRootFrm()->GetCurrShell();
            if ( pSh && pRel->IsBodyFrm() &&
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
                nRelWidth  = pRel->Prt().Width();
                nRelHeight = pRel->Prt().Height();
            }
            if ( aFrmSz.GetWidthPercent() && aFrmSz.GetWidthPercent() != 0xFF &&
                 aOldFrmSz.GetWidth() != aFrmSz.GetWidth() )
                aFrmSz.SetWidthPercent( sal_uInt8(aSz.Width() * 100L / nRelWidth + 0.5) );
            if ( aFrmSz.GetHeightPercent() && aFrmSz.GetHeightPercent() != 0xFF &&
                 aOldFrmSz.GetHeight() != aFrmSz.GetHeight() )
                aFrmSz.SetHeightPercent( sal_uInt8(aSz.Height() * 100L / nRelHeight + 0.5) );
            pFmt->GetDoc()->SetAttr( aFrmSz, *pFmt );
        }
    }

    //Position can also be changed!
    const Point aOldPos( ( bVertX && !bVertL2RX ) || bRTL ?
                         GetFlyFrm()->Frm().TopRight() :
                         GetFlyFrm()->Frm().Pos() );
    if ( aNewPos != aOldPos )
    {
        //May have been altered by the ChgSize!
        if( bVertX || bRTL )
        {
            if( aOutRect.TopRight() != aNewPos )
            {
                SwTwips nDeltaX;
                if ( bVertL2RX )
                    nDeltaX = aNewPos.X() - aOutRect.Left();
                else
                    nDeltaX = aNewPos.X() - aOutRect.Right();
                SwTwips nDeltaY = aNewPos.Y() - aOutRect.Top();
                MoveRect( aOutRect, Size( nDeltaX, nDeltaY ) );
            }
        }
        else if ( aOutRect.TopLeft() != aNewPos )
            aOutRect.SetPos( aNewPos );
        bInResize = true;
        NbcMove( Size( 0, 0 ) );
        bInResize = false;
    }
}

void SwVirtFlyDrawObj::Move(const Size& rSiz)
{
    NbcMove( rSiz );
    SetChanged();
    GetFmt()->GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false);
}

void SwVirtFlyDrawObj::Resize(const Point& rRef,
                    const Fraction& xFact, const Fraction& yFact, bool /*bUnsetRelative*/)
{
    NbcResize( rRef, xFact, yFact );
    SetChanged();
    GetFmt()->GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false);
}

// Macro

Pointer  SwVirtFlyDrawObj::GetMacroPointer(
    const SdrObjMacroHitRec& ) const
{
    return Pointer( POINTER_REFHAND );
}

bool SwVirtFlyDrawObj::HasMacro() const
{
    const SwFmtURL &rURL = pFlyFrm->GetFmt()->GetURL();
    return rURL.GetMap() || !rURL.GetURL().isEmpty();
}

SdrObject* SwVirtFlyDrawObj::CheckMacroHit( const SdrObjMacroHitRec& rRec ) const
{
    const SwFmtURL &rURL = pFlyFrm->GetFmt()->GetURL();
    if( rURL.GetMap() || !rURL.GetURL().isEmpty() )
    {
        SwRect aRect;
        if ( pFlyFrm->Lower() && pFlyFrm->Lower()->IsNoTxtFrm() )
        {
            aRect = pFlyFrm->Prt();
            aRect += pFlyFrm->Frm().Pos();
        }
        else
            aRect = pFlyFrm->Frm();

        if( aRect.IsInside( rRec.aPos ) )
        {
            aRect.Pos().setX(aRect.Pos().getX() + rRec.nTol);
            aRect.Pos().setY(aRect.Pos().getY() + rRec.nTol);
            aRect.SSize().Height()-= 2 * rRec.nTol;
            aRect.SSize().Width() -= 2 * rRec.nTol;

            if( aRect.IsInside( rRec.aPos ) )
            {
                if( !rURL.GetMap() ||
                    pFlyFrm->GetFmt()->GetIMapObject( rRec.aPos, pFlyFrm ))
                    return (SdrObject*)this;

                return 0;
            }
        }
    }
    return SdrObject::CheckMacroHit( rRec );
}

// Dragging

bool SwVirtFlyDrawObj::supportsFullDrag() const
{
    // call parent
    return SdrVirtObj::supportsFullDrag();
}

SdrObject* SwVirtFlyDrawObj::getFullDragClone() const
{
    // call parent
    return SdrVirtObj::getFullDragClone();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
