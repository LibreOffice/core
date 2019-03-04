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

#include <sal/config.h>

#include <iterator>
#include <utility>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <comphelper/sequence.hxx>


using namespace com::sun::star;


namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DDecompositionVisitor::~Primitive2DDecompositionVisitor() {}

        BasePrimitive2D::BasePrimitive2D()
        :   BasePrimitive2DImplBase(m_aMutex)
        {
        }

        BasePrimitive2D::~BasePrimitive2D()
        {
        }

        bool BasePrimitive2D::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            return (getPrimitive2DID() == rPrimitive.getPrimitive2DID());
        }

        // Visitor class to get the B2D range from a tree of Primitive2DReference's
        //
        class B2DRangeVisitor : public Primitive2DDecompositionVisitor {
        public:
            const geometry::ViewInformation2D& mrViewInformation;
            basegfx::B2DRange maRetval;
            B2DRangeVisitor(const geometry::ViewInformation2D& rViewInformation) : mrViewInformation(rViewInformation) {}
            virtual void append(const Primitive2DReference& r) override {
                maRetval.expand(getB2DRangeFromPrimitive2DReference(r, mrViewInformation));
            }
            virtual void append(const Primitive2DContainer& r) override {
                maRetval.expand(r.getB2DRange(mrViewInformation));
            }
            virtual void append(Primitive2DContainer&& r) override {
                maRetval.expand(r.getB2DRange(mrViewInformation));
            }
        };
        basegfx::B2DRange BasePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            B2DRangeVisitor aVisitor(rViewInformation);
            get2DDecomposition(aVisitor, rViewInformation);
            return aVisitor.maRetval;
        }

        void BasePrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& /*rVisitor*/, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
        }

        css::uno::Sequence< ::css::uno::Reference< ::css::graphic::XPrimitive2D > > SAL_CALL BasePrimitive2D::getDecomposition( const uno::Sequence< beans::PropertyValue >& rViewParameters )
        {
            const geometry::ViewInformation2D aViewInformation(rViewParameters);
            Primitive2DContainer aContainer;
            get2DDecomposition(aContainer, aViewInformation);
            return comphelper::containerToSequence(aContainer);
        }

        css::geometry::RealRectangle2D SAL_CALL BasePrimitive2D::getRange( const uno::Sequence< beans::PropertyValue >& rViewParameters )
        {
            const geometry::ViewInformation2D aViewInformation(rViewParameters);
            return basegfx::unotools::rectangle2DFromB2DRectangle(getB2DRange(aViewInformation));
        }

        sal_Int64 SAL_CALL BasePrimitive2D::estimateUsage()
        {
            return 0; // for now ignore the objects themselves
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace primitive2d
    {
        void BufferedDecompositionPrimitive2D::create2DDecomposition(Primitive2DContainer& /*rContainer*/, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
        }

        BufferedDecompositionPrimitive2D::BufferedDecompositionPrimitive2D()
        :   BasePrimitive2D(),
            maBuffered2DDecomposition()
        {
        }

        void BufferedDecompositionPrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(getBuffered2DDecomposition().empty())
            {
                Primitive2DContainer aNewSequence;
                create2DDecomposition(aNewSequence, rViewInformation);
                const_cast< BufferedDecompositionPrimitive2D* >(this)->setBuffered2DDecomposition(aNewSequence);
            }

            rVisitor.append(getBuffered2DDecomposition());
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// tooling

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DContainer Primitive2DContainer::maybeInvert(bool bInvert) const
        {
            const sal_uInt32 nSize(size());
            Primitive2DContainer aRetval;

            aRetval.resize(nSize);

            for(sal_uInt32 a(0); a < nSize; a++)
            {
                aRetval[bInvert ? nSize - 1 - a : a] = (*this)[a];
            }

            // all entries taken over to Uno References as owners. To avoid
            // errors with users of this mechanism to delete pointers to BasePrimitive2D
            // itself, clear given vector
            const_cast< Primitive2DContainer& >(*this).clear();

            return aRetval;
        }

        // get B2DRange from a given Primitive2DReference
        basegfx::B2DRange getB2DRangeFromPrimitive2DReference(const Primitive2DReference& rCandidate, const geometry::ViewInformation2D& aViewInformation)
        {
            basegfx::B2DRange aRetval;

            if(rCandidate.is())
            {
                // try to get C++ implementation base
                const BasePrimitive2D* pCandidate(dynamic_cast< BasePrimitive2D* >(rCandidate.get()));

                if(pCandidate)
                {
                    // use it if possible
                    aRetval.expand(pCandidate->getB2DRange(aViewInformation));
                }
                else
                {
                    // use UNO API call instead
                    const uno::Sequence< beans::PropertyValue >& rViewParameters(aViewInformation.getViewInformationSequence());
                    aRetval.expand(basegfx::unotools::b2DRectangleFromRealRectangle2D(rCandidate->getRange(rViewParameters)));
                }
            }

            return aRetval;
        }

        // get B2DRange from a given Primitive2DSequence
        basegfx::B2DRange Primitive2DContainer::getB2DRange(const geometry::ViewInformation2D& aViewInformation) const
        {
            basegfx::B2DRange aRetval;

            if(!empty())
            {
                const sal_Int32 nCount(size());

                for(sal_Int32 a(0); a < nCount; a++)
                {
                    aRetval.expand(getB2DRangeFromPrimitive2DReference((*this)[a], aViewInformation));
                }
            }

            return aRetval;
        }

        bool arePrimitive2DReferencesEqual(const Primitive2DReference& rxA, const Primitive2DReference& rxB)
        {
            const bool bAIs(rxA.is());

            if(bAIs != rxB.is())
            {
                return false;
            }

            if(!bAIs)
            {
                return true;
            }

            const BasePrimitive2D* pA(dynamic_cast< const BasePrimitive2D* >(rxA.get()));
            const BasePrimitive2D* pB(dynamic_cast< const BasePrimitive2D* >(rxB.get()));
            const bool bAEqualZero(pA == nullptr);

            if(bAEqualZero != (pB == nullptr))
            {
                return false;
            }

            if(bAEqualZero)
            {
                return false;
            }

            return pA->operator==(*pB);
        }

        bool Primitive2DContainer::operator==(const Primitive2DContainer& rB) const
        {
            const bool bAHasElements(!empty());

            if(bAHasElements != !rB.empty())
            {
                return false;
            }

            if(!bAHasElements)
            {
                return true;
            }

            const size_t nCount(size());

            if(nCount != rB.size())
            {
                return false;
            }

            for(size_t a(0); a < nCount; a++)
            {
                if(!arePrimitive2DReferencesEqual((*this)[a], rB[a]))
                {
                    return false;
                }
            }

            return true;
        }

        Primitive2DContainer::~Primitive2DContainer() {}

        void Primitive2DContainer::append(const Primitive2DReference& rSource)
        {
            push_back(rSource);
        }

        void Primitive2DContainer::append(const Primitive2DContainer& rSource)
        {
            insert(end(), rSource.begin(), rSource.end());
        }

        void Primitive2DContainer::append(Primitive2DContainer&& rSource)
        {
            size_t n = size();
            resize(n + rSource.size());
            for (size_t i = 0; i<rSource.size(); ++i)
            {
                (*this)[n + i] = std::move( rSource[i] );
            }
        }

        void Primitive2DContainer::append(const Primitive2DSequence& rSource)
        {
            std::copy(rSource.begin(), rSource.end(), std::back_inserter(*this));
        }

        OUString idToString(sal_uInt32 nId)
        {
            switch( nId )
            {
            case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D: return OUString("TRANSPARENCE");
            case PRIMITIVE2D_ID_ANIMATEDSWITCHPRIMITIVE2D: return OUString("ANIMATEDSWITCH");
            case PRIMITIVE2D_ID_ANIMATEDBLINKPRIMITIVE2D: return OUString("ANIMATEDBLINK");
            case PRIMITIVE2D_ID_ANIMATEDINTERPOLATEPRIMITIVE2D: return OUString("ANIMATEDINTERPOLATE");
            case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D: return OUString("BACKGROUNDCOLOR");
            case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D: return OUString("BITMAP");
            case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D: return OUString("CONTROL");
            case PRIMITIVE2D_ID_EMBEDDED3DPRIMITIVE2D: return OUString("EMBEDDED3D");
            case PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D: return OUString("FILLGRAPHIC");
            case PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D: return OUString("FILLGRADIENT");
            case PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D: return OUString("FILLHATCH");
            case PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D: return OUString("GRAPHIC");
            case PRIMITIVE2D_ID_GRIDPRIMITIVE2D: return OUString("GRID");
            case PRIMITIVE2D_ID_GROUPPRIMITIVE2D: return OUString("GROUP");
            case PRIMITIVE2D_ID_HELPLINEPRIMITIVE2D: return OUString("HELPLINE");
            case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D: return OUString("MARKERARRAY");
            case PRIMITIVE2D_ID_MASKPRIMITIVE2D: return OUString("MASK");
            case PRIMITIVE2D_ID_MEDIAPRIMITIVE2D: return OUString("MEDIA");
            case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D: return OUString("METAFILE");
            case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D: return OUString("MODIFIEDCOLOR");
            case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D: return OUString("POLYGONHAIRLINE");
            case PRIMITIVE2D_ID_POLYGONMARKERPRIMITIVE2D: return OUString("POLYGONMARKER");
            case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D: return OUString("POLYGONSTROKE");
            case PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D: return OUString("POLYGONSTROKEARROW");
            case PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D: return OUString("POLYPOLYGONSTROKE");
            case PRIMITIVE2D_ID_POLYPOLYGONSTROKEARROWPRIMITIVE2D: return OUString("POLYPOLYGONSTROKEARROW");
            case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D: return OUString("POLYPOLYGONCOLOR");
            case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D: return OUString("POLYPOLYGONGRADIENT");
            case PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D: return OUString("POLYPOLYGONHATCH");
            case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D: return OUString("POLYPOLYGONGRAPHIC");
            case PRIMITIVE2D_ID_SCENEPRIMITIVE2D: return OUString("SCENE");
            case PRIMITIVE2D_ID_SHADOWPRIMITIVE2D: return OUString("SHADOW");
            case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D: return OUString("TEXTSIMPLEPORTION");
            case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D: return OUString("TEXTDECORATEDPORTION");
            case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D: return OUString("TRANSFORM");
            case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D: return OUString("UNIFIEDTRANSPARENCE");
            case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D: return OUString("POINTARRAY");
            case PRIMITIVE2D_ID_TEXTHIERARCHYFIELDPRIMITIVE2D: return OUString("TEXTHIERARCHYFIELD");
            case PRIMITIVE2D_ID_TEXTHIERARCHYLINEPRIMITIVE2D: return OUString("TEXTHIERARCHYLINE");
            case PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D: return OUString("TEXTHIERARCHYPARAGRAPH");
            case PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D: return OUString("TEXTHIERARCHYBLOCK");
            case PRIMITIVE2D_ID_TEXTHIERARCHYEDITPRIMITIVE2D: return OUString("TEXTHIERARCHYEDIT");
            case PRIMITIVE2D_ID_POLYGONWAVEPRIMITIVE2D: return OUString("POLYGONWAVE");
            case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D: return OUString("WRONGSPELL");
            case PRIMITIVE2D_ID_TEXTEFFECTPRIMITIVE2D: return OUString("TEXTEFFECT");
            case PRIMITIVE2D_ID_TEXTHIERARCHYBULLETPRIMITIVE2D: return OUString("TEXTHIERARCHYBULLET");
            case PRIMITIVE2D_ID_POLYPOLYGONHAIRLINEPRIMITIVE2D: return OUString("POLYPOLYGONHAIRLINE");
            case PRIMITIVE2D_ID_EXECUTEPRIMITIVE2D: return OUString("EXECUTE");
            case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D: return OUString("PAGEPREVIEW");
            case PRIMITIVE2D_ID_STRUCTURETAGPRIMITIVE2D: return OUString("STRUCTURETAG");
            case PRIMITIVE2D_ID_BORDERLINEPRIMITIVE2D: return OUString("BORDERLINE");
            case PRIMITIVE2D_ID_POLYPOLYGONMARKERPRIMITIVE2D: return OUString("POLYPOLYGONMARKER");
            case PRIMITIVE2D_ID_HITTESTPRIMITIVE2D: return OUString("HITTEST");
            case PRIMITIVE2D_ID_INVERTPRIMITIVE2D: return OUString("INVERT");
            case PRIMITIVE2D_ID_DISCRETEBITMAPPRIMITIVE2D: return OUString("DISCRETEBITMAP");
            case PRIMITIVE2D_ID_WALLPAPERBITMAPPRIMITIVE2D: return OUString("WALLPAPERBITMAP");
            case PRIMITIVE2D_ID_TEXTLINEPRIMITIVE2D: return OUString("TEXTLINE");
            case PRIMITIVE2D_ID_TEXTCHARACTERSTRIKEOUTPRIMITIVE2D: return OUString("TEXTCHARACTERSTRIKEOUT");
            case PRIMITIVE2D_ID_TEXTGEOMETRYSTRIKEOUTPRIMITIVE2D: return OUString("TEXTGEOMETRYSTRIKEOUT");
            case PRIMITIVE2D_ID_EPSPRIMITIVE2D: return OUString("EPS");
            case PRIMITIVE2D_ID_DISCRETESHADOWPRIMITIVE2D: return OUString("DISCRETESHADOW");
            case PRIMITIVE2D_ID_HIDDENGEOMETRYPRIMITIVE2D: return OUString("HIDDENGEOMETRY");
            case PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D: return OUString("SVGLINEARGRADIENT");
            case PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D: return OUString("SVGRADIALGRADIENT");
            case PRIMITIVE2D_ID_SVGLINEARATOMPRIMITIVE2D: return OUString("SVGLINEARATOM");
            case PRIMITIVE2D_ID_SVGRADIALATOMPRIMITIVE2D: return OUString("SVGRADIALATOM");
            case PRIMITIVE2D_ID_CROPPRIMITIVE2D: return OUString("CROP");
            case PRIMITIVE2D_ID_PATTERNFILLPRIMITIVE2D: return OUString("PATTERNFILL");
            case PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D: return OUString("OBJECTINFO");
            case PRIMITIVE2D_ID_POLYPOLYGONSELECTIONPRIMITIVE2D: return OUString("POLYPOLYGONSELECTION");
            case PRIMITIVE2D_ID_PAGEHIERARCHYPRIMITIVE2D: return OUString("PAGEHIERARCHY");
            default: return OUString::number((nId >> 16) & 0xFF) + "|" + OUString::number(nId & 0xFF);
            }
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
