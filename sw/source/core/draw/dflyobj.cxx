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
#include <comphelper/lok.hxx>
#include <tools/mapunit.hxx>
#include <svx/svdtrans.hxx>
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <svx/svdpage.hxx>
#include <vcl/svapp.hxx>

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
#include "wrtsh.hxx"

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
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const override;

        public:
            /// basic constructor, used from SdrObject.
            explicit VCOfSwFlyDrawObj(SwFlyDrawObj& rObj)
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
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            SwVirtFlyDrawObjPrimitive(
                const SwVirtFlyDrawObj& rSwVirtFlyDrawObj,
                const basegfx::B2DRange &rOuterRange)
            :   BufferedDecompositionPrimitive2D(),
                mrSwVirtFlyDrawObj(rSwVirtFlyDrawObj),
                maOuterRange(rOuterRange)
            {
            }

            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            // override to allow callbacks to wrap_DoPaintObject
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

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
                // create invisible geometry to allow correct HitTest and BoundRect calculations for the
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

        Primitive2DSequence SwVirtFlyDrawObjPrimitive::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // This is the callback to keep the FlyFrame painting in SW alive as long as it
            // is not changed to primitives. This is the method which will be called by the processors
            // when they do not know this primitive (and they do not). Inside wrap_DoPaintObject
            // there needs to be a test that paint is only done during SW repaints (see there).
            // Using this mechanism guarantees the correct Z-Order of the VirtualObject-based FlyFrames.
            getSwVirtFlyDrawObj().wrap_DoPaintObject(rViewInformation);

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
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const override;

        public:
            /// basic constructor, used from SdrObject.
            explicit VCOfSwVirtFlyDrawObj(SwVirtFlyDrawObj& rObj)
            :   ViewContactOfVirtObj(rObj)
            {
            }
            virtual ~VCOfSwVirtFlyDrawObj();

            /// access to SwVirtFlyDrawObj
            SwVirtFlyDrawObj& GetSwVirtFlyDrawObj() const
            {
                return static_cast<SwVirtFlyDrawObj&>(mrObject);
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

    if(dynamic_cast<const SwFlyDrawObj*>( &rReferencedObject) !=  nullptr)
    {
        const SwFlyFrame* pFlyFrame = GetFlyFrame();

        if(pFlyFrame)
        {
            const Rectangle aOuterRectangle(pFlyFrame->Frame().Pos(), pFlyFrame->Frame().SSize());

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
            const Rectangle aInnerRectangle(pFlyFrame->Frame().Pos() + pFlyFrame->Prt().Pos(), pFlyFrame->Prt().SSize());

            if(!aInnerRectangle.IsEmpty())
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

SwVirtFlyDrawObj::SwVirtFlyDrawObj(SdrObject& rNew, SwFlyFrame* pFly) :
    SdrVirtObj( rNew ),
    m_pFlyFrame( pFly )
{
    const SvxProtectItem &rP = m_pFlyFrame->GetFormat()->GetProtect();
    bMovProt = rP.IsPosProtected();
    bSizProt = rP.IsSizeProtected();
}

SwVirtFlyDrawObj::~SwVirtFlyDrawObj()
{
    if ( GetPage() )    //Withdraw SdrPage the responsibility.
        GetPage()->RemoveObject( GetOrdNum() );
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
            explicit RestoreMapMode( SwViewShell* pViewShell )
                : mbMapModeRestored( false )
                , mpOutDev( pViewShell->GetOut() )
            {
                if ( pViewShell->getPrePostMapMode() != mpOutDev->GetMapMode() )
                {
                    mpOutDev->Push(PushFlags::MAPMODE);

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
    if ( pShell && pShell->IsDrawingLayerPaintInProgress() )
    {
        bool bDrawObject(true);

        if ( !SwFlyFrame::IsPaint( const_cast<SwVirtFlyDrawObj*>(this), pShell ) )
        {
            bDrawObject = false;
        }

        if ( bDrawObject )
        {
            // if there's no viewport set, all fly-frames will be painted,
            // which is slow, wastes memory, and can cause other trouble.
            (void) rViewInformation; // suppress "unused parameter" warning
            assert(comphelper::LibreOfficeKit::isActive() || !rViewInformation.getViewport().isEmpty());
            if ( !m_pFlyFrame->IsFlyInContentFrame() )
            {
                // it is also necessary to restore the VCL MapMode from ViewInformation since e.g.
                // the VCL PixelRenderer resets it at the used OutputDevice. Unfortunately, this
                // excludes shears and rotates which are not expressable in MapMode.
                // OD #i102707#
                // new helper class to restore MapMode - restoration, only if
                // needed and consideration of paint for meta file creation .
                RestoreMapMode aRestoreMapModeIfNeeded( pShell );

                // paint the FlyFrame (use standard VCL-Paint)
                m_pFlyFrame->Paint( *pShell->GetOut(), GetFlyFrame()->Frame() );
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
    if ( GetFlyFrame()->Frame().HasArea() )
        const_cast<SwVirtFlyDrawObj*>(this)->aOutRect = GetFlyFrame()->Frame().SVRect();
    else
        const_cast<SwVirtFlyDrawObj*>(this)->aOutRect = Rectangle();
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
    if (pUserCall!=nullptr)
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
    if (pUserCall!=nullptr)
        pUserCall->Changed(*this, SDRUSERCALL_RESIZE, aTmp);
}

void SwVirtFlyDrawObj::NbcSetLogicRect(const Rectangle& )
{
    SetRect();
}

::basegfx::B2DPolyPolygon SwVirtFlyDrawObj::TakeXorPoly() const
{
    const Rectangle aSourceRectangle(GetFlyFrame()->Frame().SVRect());
    const ::basegfx::B2DRange aSourceRange(aSourceRectangle.Left(), aSourceRectangle.Top(), aSourceRectangle.Right(), aSourceRectangle.Bottom());
    ::basegfx::B2DPolyPolygon aRetval;

    aRetval.append(::basegfx::tools::createPolygonFromRect(aSourceRange));

    return aRetval;
}

//  SwVirtFlyDrawObj::Move() und Resize()

void SwVirtFlyDrawObj::NbcMove(const Size& rSiz)
{
    MoveRect( aOutRect, rSiz );
    const Point aOldPos( GetFlyFrame()->Frame().Pos() );
    const Point aNewPos( aOutRect.TopLeft() );
    const SwRect aFlyRect( aOutRect );

    //If the Fly has a automatic align (right or top),
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
        static_cast<SwFlyAtContentFrame*>(GetFlyFrame())->SetAbsPos( aNewPos );
    else
    {
        const SwFrameFormat *pTmpFormat = GetFormat();
        const SwFormatVertOrient &rVert = pTmpFormat->GetVertOrient();
        const SwFormatHoriOrient &rHori = pTmpFormat->GetHoriOrient();
        long lXDiff = aNewPos.X() - aOldPos.X();
        if( rHori.IsPosToggle() && text::HoriOrientation::NONE == eHori &&
            !GetFlyFrame()->FindPageFrame()->OnRightPage() )
            lXDiff = -lXDiff;

        if( GetFlyFrame()->GetAnchorFrame()->IsRightToLeft() &&
            text::HoriOrientation::NONE == eHori )
            lXDiff = -lXDiff;

        long lYDiff = aNewPos.Y() - aOldPos.Y();
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
            lXDiff = GetFlyFrame()->GetAnchorFrame()->Frame().Width() -
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
                    aFlyRect.Left() < pAnch->Frame().Left() + pAnch->Prt().Left(),
                    bLeftPrt = aFlyRect.Left() + aFlyRect.Width() <
                               pAnch->Frame().Left() + pAnch->Prt().Width()/2;
                if ( bLeftFrame || bLeftPrt )
                {
                    aHori.SetHoriOrient( text::HoriOrientation::LEFT );
                    aHori.SetRelationOrient( bLeftFrame ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
                }
                else
                {
                    const bool bRightFrame = aFlyRect.Left() >
                                       pAnch->Frame().Left() + pAnch->Prt().Width();
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


void SwVirtFlyDrawObj::NbcCrop(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    // Get Wrt Shell
    SwWrtShell *pSh = dynamic_cast<SwWrtShell*>( GetFlyFrame()->getRootFrame()->GetCurrShell() );
    if (!pSh || dynamic_cast<const SwWrtShell*>( pSh) ==  nullptr)
        return;

    // Compute old and new rect. This will give us the deformation to apply to
    // the object to crop
    Rectangle aOldRect( aOutRect );

    const long nOldWidth = aOldRect.GetWidth();
    const long nOldHeight = aOldRect.GetHeight();

    if (!nOldWidth || !nOldHeight)
        return;

    Rectangle aNewRect( aOutRect );
    ResizeRect( aNewRect, rRef, xFact, yFact );

    // Get graphic object size in 100th of mm
    GraphicObject const *pGraphicObject = pSh->GetGraphicObj();
    if (!pGraphicObject)
        return;
    const MapMode aMapMode100thmm(MAP_100TH_MM);
    Size aGraphicSize(pGraphicObject->GetPrefSize());
    if( MAP_PIXEL == pGraphicObject->GetPrefMapMode().GetMapUnit() )
        aGraphicSize = Application::GetDefaultDevice()->PixelToLogic( aGraphicSize, aMapMode100thmm );
    else
        aGraphicSize = OutputDevice::LogicToLogic( aGraphicSize, pGraphicObject->GetPrefMapMode(), aMapMode100thmm);
    if( aGraphicSize.A() == 0 || aGraphicSize.B() == 0 )
        return ;

    // Get old values for crop in 10th of mm
    SfxItemSet aSet( pSh->GetAttrPool(), RES_GRFATR_CROPGRF, RES_GRFATR_CROPGRF );
    pSh->GetCurAttr( aSet );
    SwCropGrf aCrop( static_cast<const SwCropGrf&>(aSet.Get(RES_GRFATR_CROPGRF)) );

    Rectangle aCropRectangle(
        convertTwipToMm100(aCrop.GetLeft()),
        convertTwipToMm100(aCrop.GetTop()),
        convertTwipToMm100(aCrop.GetRight()),
        convertTwipToMm100(aCrop.GetBottom()) );

    // Compute delta to apply
    double fScaleX = ( aGraphicSize.Width() - aCropRectangle.Left() - aCropRectangle.Right() ) / (double)nOldWidth;
    double fScaleY = ( aGraphicSize.Height() - aCropRectangle.Top() - aCropRectangle.Bottom() ) / (double)nOldHeight;

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
//    pSh->StartUndo(UNDO_START);

    // Set new crop values in twips
    aCrop.SetLeft  (convertMm100ToTwip(nLeftCrop));
    aCrop.SetTop   (convertMm100ToTwip(nTopCrop));
    aCrop.SetRight (convertMm100ToTwip(nRightCrop));
    aCrop.SetBottom(convertMm100ToTwip(nBottomCrop));
    pSh->SetAttrItem(aCrop);

    // Set new frame size
    SwFrameFormat *pFormat = GetFormat();
    SwFormatFrameSize aSz( pFormat->GetFrameSize() );
    aSz.SetWidth(aNewRect.GetWidth());
    aSz.SetHeight(aNewRect.GetHeight());
    pFormat->GetDoc()->SetAttr( aSz, *pFormat );

//    pSh->EndUndo(UNDO_END);
    pSh->EndAllAction();

}

void SwVirtFlyDrawObj::NbcResize(const Point& rRef,
            const Fraction& xFact, const Fraction& yFact)
{
    ResizeRect( aOutRect, rRef, xFact, yFact );

    const SwFrame* pTmpFrame = GetFlyFrame()->GetAnchorFrame();
    if( !pTmpFrame )
        pTmpFrame = GetFlyFrame();
    const bool bVertX = pTmpFrame->IsVertical();

    const bool bRTL = pTmpFrame->IsRightToLeft();

    const bool bVertL2RX = pTmpFrame->IsVertLR();
    const Point aNewPos( ( bVertX && !bVertL2RX ) || bRTL ?
                         aOutRect.Right() + 1 :
                         aOutRect.Left(),
                         aOutRect.Top() );

    Size aSz( aOutRect.Right() - aOutRect.Left() + 1,
              aOutRect.Bottom()- aOutRect.Top()  + 1 );
    if( aSz != GetFlyFrame()->Frame().SSize() )
    {
        //The width of the columns should not be too narrow
        if ( GetFlyFrame()->Lower() && GetFlyFrame()->Lower()->IsColumnFrame() )
        {
            SwBorderAttrAccess aAccess( SwFrame::GetCache(), GetFlyFrame() );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            long nMin = rAttrs.CalcLeftLine()+rAttrs.CalcRightLine();
            const SwFormatCol& rCol = rAttrs.GetAttrSet().GetCol();
            if ( rCol.GetColumns().size() > 1 )
            {
                for ( const auto &rC : rCol.GetColumns() )
                {
                    nMin += rC.GetLeft() + rC.GetRight() + MINFLY;
                }
                nMin -= MINFLY;
            }
            aSz.Width() = std::max( aSz.Width(), nMin );
        }

        SwFrameFormat *pFormat = GetFormat();
        const SwFormatFrameSize aOldFrameSz( pFormat->GetFrameSize() );
        GetFlyFrame()->ChgSize( aSz );
        SwFormatFrameSize aFrameSz( pFormat->GetFrameSize() );
        if ( aFrameSz.GetWidthPercent() || aFrameSz.GetHeightPercent() )
        {
            long nRelWidth, nRelHeight;
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
                nRelWidth  = pRel->Prt().Width();
                nRelHeight = pRel->Prt().Height();
            }
            if ( aFrameSz.GetWidthPercent() && aFrameSz.GetWidthPercent() != SwFormatFrameSize::SYNCED &&
                 aOldFrameSz.GetWidth() != aFrameSz.GetWidth() )
                aFrameSz.SetWidthPercent( sal_uInt8(aSz.Width() * 100.0 / nRelWidth + 0.5) );
            if ( aFrameSz.GetHeightPercent() && aFrameSz.GetHeightPercent() != SwFormatFrameSize::SYNCED &&
                 aOldFrameSz.GetHeight() != aFrameSz.GetHeight() )
                aFrameSz.SetHeightPercent( sal_uInt8(aSz.Height() * 100.0 / nRelHeight + 0.5) );
            pFormat->GetDoc()->SetAttr( aFrameSz, *pFormat );
        }
    }

    //Position can also be changed!
    const Point aOldPos( ( bVertX && !bVertL2RX ) || bRTL ?
                         GetFlyFrame()->Frame().TopRight() :
                         GetFlyFrame()->Frame().Pos() );
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
    GetFormat()->GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false);
}

void SwVirtFlyDrawObj::Resize(const Point& rRef,
                    const Fraction& xFact, const Fraction& yFact, bool /*bUnsetRelative*/)
{
    NbcResize( rRef, xFact, yFact );
    SetChanged();
    GetFormat()->GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false);
}

void SwVirtFlyDrawObj::Crop(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    NbcCrop( rRef, xFact, yFact );
    SetChanged();
    GetFormat()->GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false);
}

void SwVirtFlyDrawObj::addCropHandles(SdrHdlList& rTarget) const
{
    Rectangle aRect(GetSnapRect());

    if(!aRect.IsEmpty())
    {
       rTarget.AddHdl(new SdrCropHdl(aRect.TopLeft()     , HDL_UPLFT, 0, 0));
       rTarget.AddHdl(new SdrCropHdl(aRect.TopCenter()   , HDL_UPPER, 0, 0));
       rTarget.AddHdl(new SdrCropHdl(aRect.TopRight()    , HDL_UPRGT, 0, 0));
       rTarget.AddHdl(new SdrCropHdl(aRect.LeftCenter()  , HDL_LEFT , 0, 0));
       rTarget.AddHdl(new SdrCropHdl(aRect.RightCenter() , HDL_RIGHT, 0, 0));
       rTarget.AddHdl(new SdrCropHdl(aRect.BottomLeft()  , HDL_LWLFT, 0, 0));
       rTarget.AddHdl(new SdrCropHdl(aRect.BottomCenter(), HDL_LOWER, 0, 0));
       rTarget.AddHdl(new SdrCropHdl(aRect.BottomRight() , HDL_LWRGT, 0, 0));
    }
}

// Macro

Pointer  SwVirtFlyDrawObj::GetMacroPointer(
    const SdrObjMacroHitRec& ) const
{
    return Pointer( PointerStyle::RefHand );
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
            aRect = m_pFlyFrame->Prt();
            aRect += m_pFlyFrame->Frame().Pos();
        }
        else
            aRect = m_pFlyFrame->Frame();

        if( aRect.IsInside( rRec.aPos ) )
        {
            aRect.Pos().setX(aRect.Pos().getX() + rRec.nTol);
            aRect.Pos().setY(aRect.Pos().getY() + rRec.nTol);
            aRect.SSize().Height()-= 2 * rRec.nTol;
            aRect.SSize().Width() -= 2 * rRec.nTol;

            if( aRect.IsInside( rRec.aPos ) )
            {
                if( !rURL.GetMap() ||
                    m_pFlyFrame->GetFormat()->GetIMapObject( rRec.aPos, m_pFlyFrame ))
                    return const_cast<SdrObject*>(static_cast<SdrObject const *>(this));

                return nullptr;
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
