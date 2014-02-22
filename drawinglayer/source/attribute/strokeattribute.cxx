/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <drawinglayer/attribute/strokeattribute.hxx>
#include <rtl/instance.hxx>
#include <numeric>



namespace drawinglayer
{
    namespace attribute
    {
        class ImpStrokeAttribute
        {
        public:
            
            ::std::vector< double >                     maDotDashArray;         
            double                                      mfFullDotDashLen;       

            ImpStrokeAttribute(
                const ::std::vector< double >& rDotDashArray,
                double fFullDotDashLen)
            :   maDotDashArray(rDotDashArray),
                mfFullDotDashLen(fFullDotDashLen)
            {
            }

            ImpStrokeAttribute()
            :   maDotDashArray(std::vector< double >()),
                mfFullDotDashLen(0.0)
            {
            }

            
            const ::std::vector< double >& getDotDashArray() const { return maDotDashArray; }
            double getFullDotDashLen() const
            {
                if(0.0 == mfFullDotDashLen && maDotDashArray.size())
                {
                    
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
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< StrokeAttribute::ImplType, theGlobalDefault > {};
        }

        StrokeAttribute::StrokeAttribute(
            const ::std::vector< double >& rDotDashArray,
            double fFullDotDashLen)
        :   mpStrokeAttribute(ImpStrokeAttribute(
                rDotDashArray, fFullDotDashLen))
        {
        }

        StrokeAttribute::StrokeAttribute()
        :   mpStrokeAttribute(theGlobalDefault::get())
        {
        }

        StrokeAttribute::StrokeAttribute(const StrokeAttribute& rCandidate)
        :   mpStrokeAttribute(rCandidate.mpStrokeAttribute)
        {
        }

        StrokeAttribute::~StrokeAttribute()
        {
        }

        bool StrokeAttribute::isDefault() const
        {
            return mpStrokeAttribute.same_object(theGlobalDefault::get());
        }

        StrokeAttribute& StrokeAttribute::operator=(const StrokeAttribute& rCandidate)
        {
            mpStrokeAttribute = rCandidate.mpStrokeAttribute;
            return *this;
        }

        bool StrokeAttribute::operator==(const StrokeAttribute& rCandidate) const
        {
            return rCandidate.mpStrokeAttribute == mpStrokeAttribute;
        }

        const ::std::vector< double >& StrokeAttribute::getDotDashArray() const
        {
            return mpStrokeAttribute->getDotDashArray();
        }

        double StrokeAttribute::getFullDotDashLen() const
        {
            return mpStrokeAttribute->getFullDotDashLen();
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
