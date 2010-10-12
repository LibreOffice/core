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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
