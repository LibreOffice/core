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
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/tools/canvastools.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        BasePrimitive2D::BasePrimitive2D()
        :   BasePrimitive2DImplBase(m_aMutex)
        {
        }

        BasePrimitive2D::~BasePrimitive2D()
        {
        }

        basegfx::B2DRange BasePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            return getB2DRangeFromPrimitive2DSequence(get2DDecomposition(rViewInformation), rViewInformation);
        }

        Primitive2DSequence BasePrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            return Primitive2DSequence();
        }

        Primitive2DSequence SAL_CALL BasePrimitive2D::getDecomposition( const uno::Sequence< beans::PropertyValue >& rViewParameters ) throw ( uno::RuntimeException )
        {
            const geometry::ViewInformation2D aViewInformation(rViewParameters);
            return get2DDecomposition(aViewInformation);
        }

        com::sun::star::geometry::RealRectangle2D SAL_CALL BasePrimitive2D::getRange( const uno::Sequence< beans::PropertyValue >& rViewParameters ) throw ( uno::RuntimeException )
        {
            const geometry::ViewInformation2D aViewInformation(rViewParameters);
            return basegfx::unotools::rectangle2DFromB2DRectangle(getB2DRange(aViewInformation));
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence BufferedDecompositionPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            return Primitive2DSequence();
        }

        BufferedDecompositionPrimitive2D::BufferedDecompositionPrimitive2D()
        :   BasePrimitive2D(),
            maBuffered2DDecomposition()
        {
        }

        Primitive2DSequence BufferedDecompositionPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(!getBuffered2DDecomposition().hasElements())
            {
                const Primitive2DSequence aNewSequence(create2DDecomposition(rViewInformation));
                const_cast< BufferedDecompositionPrimitive2D* >(this)->setBuffered2DDecomposition(aNewSequence);
            }

            return getBuffered2DDecomposition();
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// tooling

namespace drawinglayer
{
    namespace primitive2d
    {
        // convert helper stl vector of primitives to Primitive2DSequence
        Primitive2DSequence Primitive2DVectorToPrimitive2DSequence(const Primitive2DVector& rSource, bool bInvert)
        {
            const sal_uInt32 nSize(rSource.size());
            Primitive2DSequence aRetval;

            aRetval.realloc(nSize);

            for(sal_uInt32 a(0); a < nSize; a++)
            {
                aRetval[bInvert ? nSize - 1 - a : a] = rSource[a];
            }

            // all entries taken over to Uno References as owners. To avoid
            // errors with users of this mechanism to delete pointers to BasePrimitive2D
            // itself, clear given vector
            const_cast< Primitive2DVector& >(rSource).clear();

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
        basegfx::B2DRange getB2DRangeFromPrimitive2DSequence(const Primitive2DSequence& rCandidate, const geometry::ViewInformation2D& aViewInformation)
        {
            basegfx::B2DRange aRetval;

            if(rCandidate.hasElements())
            {
                const sal_Int32 nCount(rCandidate.getLength());

                for(sal_Int32 a(0L); a < nCount; a++)
                {
                    aRetval.expand(getB2DRangeFromPrimitive2DReference(rCandidate[a], aViewInformation));
                }
            }

            return aRetval;
        }

        // concatenate sequence
        void appendPrimitive2DSequenceToPrimitive2DSequence(Primitive2DSequence& rDest, const Primitive2DSequence& rSource)
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

        // concatenate single Primitive2D
        void appendPrimitive2DReferenceToPrimitive2DSequence(Primitive2DSequence& rDest, const Primitive2DReference& rSource)
        {
            if(rSource.is())
            {
                const sal_Int32 nDestCount(rDest.getLength());
                rDest.realloc(nDestCount + 1L);
                rDest[nDestCount] = rSource;
            }
        }

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
