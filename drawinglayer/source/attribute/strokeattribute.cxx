/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strokeattribute.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:19 $
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

#include <drawinglayer/attribute/strokeattribute.hxx>
#include <numeric>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpStrokeAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            ::std::vector< double >                     maDotDashArray;         // array of double which defines the dot-dash pattern
            double                                      mfFullDotDashLen;       // sum of maDotDashArray (for convenience)

            ImpStrokeAttribute(
                const ::std::vector< double >& rDotDashArray,
                double fFullDotDashLen)
            :   mnRefCount(0),
                maDotDashArray(rDotDashArray),
                mfFullDotDashLen(fFullDotDashLen)
            {
            }

            // data read access
            const ::std::vector< double >& getDotDashArray() const { return maDotDashArray; }
            double getFullDotDashLen() const
            {
                if(0.0 == mfFullDotDashLen && maDotDashArray.size())
                {
                    // calculate length on demand
                    const double fAccumulated(::std::accumulate(maDotDashArray.begin(), maDotDashArray.end(), 0.0));
                    const_cast< ImpStrokeAttribute* >(this)->mfFullDotDashLen = fAccumulated;
                }

                return mfFullDotDashLen;
            }

            bool operator==(const ImpStrokeAttribute& rCandidate) const
            {
                return (getDotDashArray() == rCandidate.getDotDashArray()
                    && getFullDotDashLen() == rCandidate.getFullDotDashLen());
            }

            static ImpStrokeAttribute* get_global_default()
            {
                static ImpStrokeAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpStrokeAttribute(
                        std::vector< double >(),
                        0.0);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        StrokeAttribute::StrokeAttribute(
            const ::std::vector< double >& rDotDashArray,
            double fFullDotDashLen)
        :   mpStrokeAttribute(new ImpStrokeAttribute(
                rDotDashArray, fFullDotDashLen))
        {
        }

        StrokeAttribute::StrokeAttribute()
        :   mpStrokeAttribute(ImpStrokeAttribute::get_global_default())
        {
            mpStrokeAttribute->mnRefCount++;
        }

        StrokeAttribute::StrokeAttribute(const StrokeAttribute& rCandidate)
        :   mpStrokeAttribute(rCandidate.mpStrokeAttribute)
        {
            mpStrokeAttribute->mnRefCount++;
        }

        StrokeAttribute::~StrokeAttribute()
        {
            if(mpStrokeAttribute->mnRefCount)
            {
                mpStrokeAttribute->mnRefCount--;
            }
            else
            {
                delete mpStrokeAttribute;
            }
        }

        bool StrokeAttribute::isDefault() const
        {
            return mpStrokeAttribute == ImpStrokeAttribute::get_global_default();
        }

        StrokeAttribute& StrokeAttribute::operator=(const StrokeAttribute& rCandidate)
        {
            if(rCandidate.mpStrokeAttribute != mpStrokeAttribute)
            {
                if(mpStrokeAttribute->mnRefCount)
                {
                    mpStrokeAttribute->mnRefCount--;
                }
                else
                {
                    delete mpStrokeAttribute;
                }

                mpStrokeAttribute = rCandidate.mpStrokeAttribute;
                mpStrokeAttribute->mnRefCount++;
            }

            return *this;
        }

        bool StrokeAttribute::operator==(const StrokeAttribute& rCandidate) const
        {
            if(rCandidate.mpStrokeAttribute == mpStrokeAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpStrokeAttribute == *mpStrokeAttribute);
        }

        const ::std::vector< double >& StrokeAttribute::getDotDashArray() const
        {
            return mpStrokeAttribute->getDotDashArray();
        }

        double StrokeAttribute::getFullDotDashLen() const
        {
            return mpStrokeAttribute->getFullDotDashLen();
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
