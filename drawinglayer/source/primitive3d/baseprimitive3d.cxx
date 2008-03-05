/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprimitive3d.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_BASEPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace
{
    const ::rtl::OUString& getNamePropertyTime()
    {
        static ::rtl::OUString s_sNamePropertyTime(RTL_CONSTASCII_USTRINGPARAM("Time"));
        return s_sNamePropertyTime;
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence BasePrimitive3D::createLocalDecomposition(double /*fTime*/) const
        {
            // default returns an empty sequence
            return Primitive3DSequence();
        }

        BasePrimitive3D::BasePrimitive3D()
        :   BasePrimitive3DImplBase(m_aMutex),
            maLocalDecomposition()
        {
        }

        bool BasePrimitive3D::operator==( const BasePrimitive3D& rPrimitive ) const
        {
            return (getPrimitiveID() == rPrimitive.getPrimitiveID());
        }

        basegfx::B3DRange BasePrimitive3D::getB3DRange(double fTime) const
        {
            return getB3DRangeFromPrimitive3DSequence(get3DDecomposition(fTime), fTime);
        }


        Primitive3DSequence BasePrimitive3D::get3DDecomposition(double fTime) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(!getLocalDecomposition().hasElements())
            {
                const Primitive3DSequence aNewSequence(createLocalDecomposition(fTime));
                const_cast< BasePrimitive3D* >(this)->setLocalDecomposition(aNewSequence);
            }

            return getLocalDecomposition();
        }

        Primitive3DSequence SAL_CALL BasePrimitive3D::getDecomposition( const uno::Sequence< beans::PropertyValue >& rViewParameters ) throw ( uno::RuntimeException )
        {
            return get3DDecomposition(ViewParametersToTime(rViewParameters));
        }

        geometry::RealRectangle3D SAL_CALL BasePrimitive3D::getRange( const uno::Sequence< beans::PropertyValue >& rViewParameters ) throw ( uno::RuntimeException )
        {
            return basegfx::unotools::rectangle3DFromB3DRectangle(getB3DRange(ViewParametersToTime(rViewParameters)));
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
        basegfx::B3DRange getB3DRangeFromPrimitive3DReference(const Primitive3DReference& rCandidate, double fTime)
        {
            basegfx::B3DRange aRetval;

            if(rCandidate.is())
            {
                // try to get C++ implementation base
                const BasePrimitive3D* pCandidate(dynamic_cast< BasePrimitive3D* >(rCandidate.get()));

                if(pCandidate)
                {
                    // use it if possible
                    aRetval.expand(pCandidate->getB3DRange(fTime));
                }
                else
                {
                    // use UNO API call instead
                    const uno::Sequence< beans::PropertyValue > xViewParameters(TimeToViewParameters(fTime));
                    aRetval.expand(basegfx::unotools::b3DRectangleFromRealRectangle3D(rCandidate->getRange(xViewParameters)));
                }
            }

            return aRetval;
        }

        // get range3D from a given Primitive3DSequence
        basegfx::B3DRange getB3DRangeFromPrimitive3DSequence(const Primitive3DSequence& rCandidate, double fTime)
        {
            basegfx::B3DRange aRetval;

            if(rCandidate.hasElements())
            {
                const sal_Int32 nCount(rCandidate.getLength());

                for(sal_Int32 a(0L); a < nCount; a++)
                {
                    aRetval.expand(getB3DRangeFromPrimitive3DReference(rCandidate[a], fTime));
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

        // conversion helpers for 3D ViewParameters (only time used ATM)
        double ViewParametersToTime(const uno::Sequence< beans::PropertyValue >& rViewParameters)
        {
            double fRetval(0.0);

            if(rViewParameters.hasElements())
            {
                const sal_Int32 nCount(rViewParameters.getLength());

                for(sal_Int32 a(0); a < nCount; a++)
                {
                    const beans::PropertyValue& rProp = rViewParameters[a];

                    if(rProp.Name == getNamePropertyTime())
                    {
                        rProp.Value >>= fRetval;
                    }
                }
            }

            return fRetval;
        }

        const uno::Sequence< beans::PropertyValue > TimeToViewParameters(double fTime)
        {
            uno::Sequence< beans::PropertyValue > xViewParameters;

            if(0.0 < fTime)
            {
                xViewParameters.realloc(1);
                xViewParameters[0].Name = getNamePropertyTime();
                xViewParameters[0].Value <<= fTime;
            }

            return xViewParameters;
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
