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

#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <basegfx/color/bcolor.hxx>
#include <rtl/instance.hxx>



namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrLineAttribute
        {
        public:
            
            basegfx::B2DLineJoin                    meJoin;             
            double                                  mfWidth;            
            double                                  mfTransparence;     
            basegfx::BColor                         maColor;            
            com::sun::star::drawing::LineCap        meCap;              
            ::std::vector< double >                 maDotDashArray;     
            double                                  mfFullDotDashLen;   

            ImpSdrLineAttribute(
                basegfx::B2DLineJoin eJoin,
                double fWidth,
                double fTransparence,
                const basegfx::BColor& rColor,
                com::sun::star::drawing::LineCap eCap,
                const ::std::vector< double >& rDotDashArray,
                double fFullDotDashLen)
            :   meJoin(eJoin),
                mfWidth(fWidth),
                mfTransparence(fTransparence),
                maColor(rColor),
                meCap(eCap),
                maDotDashArray(rDotDashArray),
                mfFullDotDashLen(fFullDotDashLen)
            {
            }

            explicit ImpSdrLineAttribute(const basegfx::BColor& rColor)
            :   meJoin(basegfx::B2DLINEJOIN_NONE),
                mfWidth(0.0),
                mfTransparence(0.0),
                maColor(rColor),
                meCap(com::sun::star::drawing::LineCap_BUTT),
                maDotDashArray(),
                mfFullDotDashLen(0.0)
            {
            }

            ImpSdrLineAttribute()
            :   meJoin(basegfx::B2DLINEJOIN_ROUND),
                mfWidth(0.0),
                mfTransparence(0.0),
                maColor(basegfx::BColor()),
                meCap(com::sun::star::drawing::LineCap_BUTT),
                maDotDashArray(std::vector< double >()),
                mfFullDotDashLen(0.0)
            {
            }

            
            basegfx::B2DLineJoin getJoin() const { return meJoin; }
            double getWidth() const { return mfWidth; }
            double getTransparence() const { return mfTransparence; }
            const basegfx::BColor& getColor() const { return maColor; }
            com::sun::star::drawing::LineCap getCap() const { return meCap; }
            const ::std::vector< double >& getDotDashArray() const { return maDotDashArray; }
            double getFullDotDashLen() const { return mfFullDotDashLen; }

            bool operator==(const ImpSdrLineAttribute& rCandidate) const
            {
                return (getJoin() == rCandidate.getJoin()
                    && getWidth() == rCandidate.getWidth()
                    && getTransparence() == rCandidate.getTransparence()
                    && getColor() == rCandidate.getColor()
                    && getCap() == rCandidate.getCap()
                    && getDotDashArray() == rCandidate.getDotDashArray());
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< SdrLineAttribute::ImplType, theGlobalDefault > {};
        }

        SdrLineAttribute::SdrLineAttribute(
            basegfx::B2DLineJoin eJoin,
            double fWidth,
            double fTransparence,
            const basegfx::BColor& rColor,
            com::sun::star::drawing::LineCap eCap,
            const ::std::vector< double >& rDotDashArray,
            double fFullDotDashLen)
        :   mpSdrLineAttribute(
                ImpSdrLineAttribute(
                    eJoin,
                    fWidth,
                    fTransparence,
                    rColor,
                    eCap,
                    rDotDashArray,
                    fFullDotDashLen))

        {
        }

        SdrLineAttribute::SdrLineAttribute()
        :   mpSdrLineAttribute(theGlobalDefault::get())
        {
        }

        SdrLineAttribute::SdrLineAttribute(const SdrLineAttribute& rCandidate)
        :   mpSdrLineAttribute(rCandidate.mpSdrLineAttribute)
        {
        }

        SdrLineAttribute::~SdrLineAttribute()
        {
        }

        bool SdrLineAttribute::isDefault() const
        {
            return mpSdrLineAttribute.same_object(theGlobalDefault::get());
        }

        SdrLineAttribute& SdrLineAttribute::operator=(const SdrLineAttribute& rCandidate)
        {
            mpSdrLineAttribute = rCandidate.mpSdrLineAttribute;
            return *this;
        }

        bool SdrLineAttribute::operator==(const SdrLineAttribute& rCandidate) const
        {
            return rCandidate.mpSdrLineAttribute == mpSdrLineAttribute;
        }

        basegfx::B2DLineJoin SdrLineAttribute::getJoin() const
        {
            return mpSdrLineAttribute->getJoin();
        }

        double SdrLineAttribute::getWidth() const
        {
            return mpSdrLineAttribute->getWidth();
        }

        double SdrLineAttribute::getTransparence() const
        {
            return mpSdrLineAttribute->getTransparence();
        }

        const basegfx::BColor& SdrLineAttribute::getColor() const
        {
            return mpSdrLineAttribute->getColor();
        }

        const ::std::vector< double >& SdrLineAttribute::getDotDashArray() const
        {
            return mpSdrLineAttribute->getDotDashArray();
        }

        double SdrLineAttribute::getFullDotDashLen() const
        {
            return mpSdrLineAttribute->getFullDotDashLen();
        }

        com::sun::star::drawing::LineCap SdrLineAttribute::getCap() const
        {
            return mpSdrLineAttribute->getCap();
        }

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
