/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strokeattribute.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:16 $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX

#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/color/bcolor.hxx>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predefines

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class StrokeAttribute
        {
        private:
            ::std::vector< double >                     maDotDashArray;         // array of double which defines the dot-dash pattern
            double                                      mfFullDotDashLen;       // sum of maDotDashArray (for convenience)

        public:
            StrokeAttribute(
                const ::std::vector< double >& rDotDashArray,
                double fFullDotDashLen = 0.0)
            :   maDotDashArray(rDotDashArray),
                mfFullDotDashLen(fFullDotDashLen)
            {
            }

            StrokeAttribute()
            :   maDotDashArray(),
                mfFullDotDashLen(0.0)
            {
            }

            StrokeAttribute(const StrokeAttribute& rCandidate)
            {
                *this = rCandidate;
            }

            StrokeAttribute& operator=(const StrokeAttribute& rCandidate)
            {
                maDotDashArray = rCandidate.maDotDashArray;
                mfFullDotDashLen = rCandidate.mfFullDotDashLen;
                return *this;
            }

            ~StrokeAttribute()
            {
            }

            // compare operator
            bool operator==(const StrokeAttribute& rCandidate) const
            {
                return (mfFullDotDashLen == rCandidate.mfFullDotDashLen
                    && maDotDashArray == rCandidate.maDotDashArray);
            }

            // data access
            const ::std::vector< double >& getDotDashArray() const { return maDotDashArray; }
            double getFullDotDashLen() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX

// eof
