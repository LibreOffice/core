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


#include <svx/sdr/attribute/sdrformtextoutlineattribute.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>



namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrFormTextOutlineAttribute
        {
        public:
            
            sal_uInt32                          mnRefCount;

            
            LineAttribute                       maLineAttribute;
            StrokeAttribute                     maStrokeAttribute;
            sal_uInt8                           mnTransparence;

            ImpSdrFormTextOutlineAttribute(
                const LineAttribute& rLineAttribute,
                const StrokeAttribute& rStrokeAttribute,
                sal_uInt8 nTransparence)
            :   mnRefCount(0),
                maLineAttribute(rLineAttribute),
                maStrokeAttribute(rStrokeAttribute),
                mnTransparence(nTransparence)
            {
            }

            
            const LineAttribute& getLineAttribute() const { return maLineAttribute; }
            const StrokeAttribute& getStrokeAttribute() const { return maStrokeAttribute; }
            sal_uInt8 getTransparence() const { return mnTransparence; }

            
            bool operator==(const ImpSdrFormTextOutlineAttribute& rCandidate) const
            {
                return (getLineAttribute() == rCandidate.getLineAttribute()
                    && getStrokeAttribute() == rCandidate.getStrokeAttribute()
                    && getTransparence() == rCandidate.getTransparence());
            }

            static ImpSdrFormTextOutlineAttribute* get_global_default()
            {
                static ImpSdrFormTextOutlineAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrFormTextOutlineAttribute(
                        LineAttribute(),
                        StrokeAttribute(),
                        0);

                    
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        SdrFormTextOutlineAttribute::SdrFormTextOutlineAttribute(
            const LineAttribute& rLineAttribute,
            const StrokeAttribute& rStrokeAttribute,
            sal_uInt8 nTransparence)
        :   mpSdrFormTextOutlineAttribute(new ImpSdrFormTextOutlineAttribute(
                rLineAttribute, rStrokeAttribute, nTransparence))
        {
        }

        SdrFormTextOutlineAttribute::SdrFormTextOutlineAttribute()
        :   mpSdrFormTextOutlineAttribute(ImpSdrFormTextOutlineAttribute::get_global_default())
        {
            mpSdrFormTextOutlineAttribute->mnRefCount++;
        }

        SdrFormTextOutlineAttribute::SdrFormTextOutlineAttribute(const SdrFormTextOutlineAttribute& rCandidate)
        :   mpSdrFormTextOutlineAttribute(rCandidate.mpSdrFormTextOutlineAttribute)
        {
            mpSdrFormTextOutlineAttribute->mnRefCount++;
        }

        SdrFormTextOutlineAttribute::~SdrFormTextOutlineAttribute()
        {
            if(mpSdrFormTextOutlineAttribute->mnRefCount)
            {
                mpSdrFormTextOutlineAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrFormTextOutlineAttribute;
            }
        }

        bool SdrFormTextOutlineAttribute::isDefault() const
        {
            return mpSdrFormTextOutlineAttribute == ImpSdrFormTextOutlineAttribute::get_global_default();
        }

        SdrFormTextOutlineAttribute& SdrFormTextOutlineAttribute::operator=(const SdrFormTextOutlineAttribute& rCandidate)
        {
            if(rCandidate.mpSdrFormTextOutlineAttribute != mpSdrFormTextOutlineAttribute)
            {
                if(mpSdrFormTextOutlineAttribute->mnRefCount)
                {
                    mpSdrFormTextOutlineAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrFormTextOutlineAttribute;
                }

                mpSdrFormTextOutlineAttribute = rCandidate.mpSdrFormTextOutlineAttribute;
                mpSdrFormTextOutlineAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrFormTextOutlineAttribute::operator==(const SdrFormTextOutlineAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrFormTextOutlineAttribute == mpSdrFormTextOutlineAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrFormTextOutlineAttribute == *mpSdrFormTextOutlineAttribute);
        }

        const LineAttribute& SdrFormTextOutlineAttribute::getLineAttribute() const
        {
            return mpSdrFormTextOutlineAttribute->getLineAttribute();
        }

        const StrokeAttribute& SdrFormTextOutlineAttribute::getStrokeAttribute() const
        {
            return mpSdrFormTextOutlineAttribute->getStrokeAttribute();
        }

        sal_uInt8 SdrFormTextOutlineAttribute::getTransparence() const
        {
            return mpSdrFormTextOutlineAttribute->getTransparence();
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
