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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
