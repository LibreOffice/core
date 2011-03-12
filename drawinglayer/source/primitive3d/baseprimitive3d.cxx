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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <basegfx/tools/canvastools.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        BasePrimitive3D::BasePrimitive3D()
        :   BasePrimitive3DImplBase(m_aMutex)
        {
        }

        BasePrimitive3D::~BasePrimitive3D()
        {
        }

        bool BasePrimitive3D::operator==( const BasePrimitive3D& rPrimitive ) const
        {
            return (getPrimitive3DID() == rPrimitive.getPrimitive3DID());
        }

        basegfx::B3DRange BasePrimitive3D::getB3DRange(const geometry::ViewInformation3D& rViewInformation) const
        {
            return getB3DRangeFromPrimitive3DSequence(get3DDecomposition(rViewInformation), rViewInformation);
        }

        Primitive3DSequence BasePrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            return Primitive3DSequence();
        }

        Primitive3DSequence SAL_CALL BasePrimitive3D::getDecomposition( const uno::Sequence< beans::PropertyValue >& rViewParameters ) throw ( uno::RuntimeException )
        {
            const geometry::ViewInformation3D aViewInformation(rViewParameters);
            return get3DDecomposition(rViewParameters);
        }

        com::sun::star::geometry::RealRectangle3D SAL_CALL BasePrimitive3D::getRange( const uno::Sequence< beans::PropertyValue >& rViewParameters ) throw ( uno::RuntimeException )
        {
            const geometry::ViewInformation3D aViewInformation(rViewParameters);
            return basegfx::unotools::rectangle3DFromB3DRectangle(getB3DRange(aViewInformation));
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence BufferedDecompositionPrimitive3D::create3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            return Primitive3DSequence();
        }

        BufferedDecompositionPrimitive3D::BufferedDecompositionPrimitive3D()
        :   BasePrimitive3D(),
            maBuffered3DDecomposition()
        {
        }

        Primitive3DSequence BufferedDecompositionPrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(!getBuffered3DDecomposition().hasElements())
            {
                const Primitive3DSequence aNewSequence(create3DDecomposition(rViewInformation));
                const_cast< BufferedDecompositionPrimitive3D* >(this)->setBuffered3DDecomposition(aNewSequence);
            }

            return getBuffered3DDecomposition();
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// tooling

namespace drawinglayer
{
    namespace primitive3d
    {
        // get range3D from a given Primitive3DReference
        basegfx::B3DRange getB3DRangeFromPrimitive3DReference(const Primitive3DReference& rCandidate, const geometry::ViewInformation3D& aViewInformation)
        {
            basegfx::B3DRange aRetval;

            if(rCandidate.is())
            {
                // try to get C++ implementation base
                const BasePrimitive3D* pCandidate(dynamic_cast< BasePrimitive3D* >(rCandidate.get()));

                if(pCandidate)
                {
                    // use it if possible
                    aRetval.expand(pCandidate->getB3DRange(aViewInformation));
                }
                else
                {
                    // use UNO API call instead
                    const uno::Sequence< beans::PropertyValue >& rViewParameters(aViewInformation.getViewInformationSequence());
                    aRetval.expand(basegfx::unotools::b3DRectangleFromRealRectangle3D(rCandidate->getRange(rViewParameters)));
                }
            }

            return aRetval;
        }

        // get range3D from a given Primitive3DSequence
        basegfx::B3DRange getB3DRangeFromPrimitive3DSequence(const Primitive3DSequence& rCandidate, const geometry::ViewInformation3D& aViewInformation)
        {
            basegfx::B3DRange aRetval;

            if(rCandidate.hasElements())
            {
                const sal_Int32 nCount(rCandidate.getLength());

                for(sal_Int32 a(0L); a < nCount; a++)
                {
                    aRetval.expand(getB3DRangeFromPrimitive3DReference(rCandidate[a], aViewInformation));
                }
            }

            return aRetval;
        }

        bool arePrimitive3DReferencesEqual(const Primitive3DReference& rxA, const Primitive3DReference& rxB)
        {
            const sal_Bool bAIs(rxA.is());

            if(bAIs != rxB.is())
            {
                return false;
            }

            if(!bAIs)
            {
                return true;
            }

            const BasePrimitive3D* pA(dynamic_cast< const BasePrimitive3D* >(rxA.get()));
            const BasePrimitive3D* pB(dynamic_cast< const BasePrimitive3D* >(rxB.get()));
            const bool bAEqualZero(pA == 0L);

            if(bAEqualZero != (pB == 0L))
            {
                return false;
            }

            if(bAEqualZero)
            {
                return false;
            }

            return (pA->operator==(*pB));
        }

        bool arePrimitive3DSequencesEqual(const Primitive3DSequence& rA, const Primitive3DSequence& rB)
        {
            const sal_Bool bAHasElements(rA.hasElements());

            if(bAHasElements != rB.hasElements())
            {
                return false;
            }

            if(!bAHasElements)
            {
                return true;
            }

            const sal_Int32 nCount(rA.getLength());

            if(nCount != rB.getLength())
            {
                return false;
            }

            for(sal_Int32 a(0L); a < nCount; a++)
            {
                if(!arePrimitive3DReferencesEqual(rA[a], rB[a]))
                {
                    return false;
                }
            }

            return true;
        }

        // concatenate sequence
        void appendPrimitive3DSequenceToPrimitive3DSequence(Primitive3DSequence& rDest, const Primitive3DSequence& rSource)
        {
            if(rSource.hasElements())
            {
                if(rDest.hasElements())
                {
                    const sal_Int32 nSourceCount(rSource.getLength());
                    const sal_Int32 nDestCount(rDest.getLength());
                    const sal_Int32 nTargetCount(nSourceCount + nDestCount);
                    sal_Int32 nInsertPos(nDestCount);

                    rDest.realloc(nTargetCount);

                    for(sal_Int32 a(0L); a < nSourceCount; a++)
                    {
                        if(rSource[a].is())
                        {
                            rDest[nInsertPos++] = rSource[a];
                        }
                    }

                    if(nInsertPos != nTargetCount)
                    {
                        rDest.realloc(nInsertPos);
                    }
                }
                else
                {
                    rDest = rSource;
                }
            }
        }

        // concatenate single Primitive3D
        void appendPrimitive3DReferenceToPrimitive3DSequence(Primitive3DSequence& rDest, const Primitive3DReference& rSource)
        {
            if(rSource.is())
            {
                const sal_Int32 nDestCount(rDest.getLength());
                rDest.realloc(nDestCount + 1L);
                rDest[nDestCount] = rSource;
            }
        }

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
