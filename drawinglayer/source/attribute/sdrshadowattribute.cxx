/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
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

#include <drawinglayer/attribute/sdrshadowattribute.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrShadowAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // shadow definitions
            basegfx::B2DVector                  maOffset;                   // shadow offset 1/100th mm
            double                              mfTransparence;             // [0.0 .. 1.0], 0.0==no transp.
            basegfx::BColor                     maColor;                    // color of shadow

            ImpSdrShadowAttribute(
                const basegfx::B2DVector& rOffset,
                double fTransparence,
                const basegfx::BColor& rColor)
            :   mnRefCount(0),
                maOffset(rOffset),
                mfTransparence(fTransparence),
                maColor(rColor)
            {
            }

            // data read access
            const basegfx::B2DVector& getOffset() const { return maOffset; }
            double getTransparence() const { return mfTransparence; }
            const basegfx::BColor& getColor() const { return maColor; }

            bool operator==(const ImpSdrShadowAttribute& rCandidate) const
            {
                return (getOffset() == rCandidate.getOffset()
                    && getTransparence() == rCandidate.getTransparence()
                    && getColor() == rCandidate.getColor());
            }

            static ImpSdrShadowAttribute* get_global_default()
            {
                static ImpSdrShadowAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrShadowAttribute(
                        basegfx::B2DVector(),
                        0.0,
                        basegfx::BColor());

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        SdrShadowAttribute::SdrShadowAttribute(
            const basegfx::B2DVector& rOffset,
            double fTransparence,
            const basegfx::BColor& rColor)
        :   mpSdrShadowAttribute(new ImpSdrShadowAttribute(
                rOffset, fTransparence, rColor))
        {
        }

        SdrShadowAttribute::SdrShadowAttribute()
        :   mpSdrShadowAttribute(ImpSdrShadowAttribute::get_global_default())
        {
            mpSdrShadowAttribute->mnRefCount++;
        }

        SdrShadowAttribute::SdrShadowAttribute(const SdrShadowAttribute& rCandidate)
        :   mpSdrShadowAttribute(rCandidate.mpSdrShadowAttribute)
        {
            mpSdrShadowAttribute->mnRefCount++;
        }

        SdrShadowAttribute::~SdrShadowAttribute()
        {
            if(mpSdrShadowAttribute->mnRefCount)
            {
                mpSdrShadowAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrShadowAttribute;
            }
        }

        bool SdrShadowAttribute::isDefault() const
        {
            return mpSdrShadowAttribute == ImpSdrShadowAttribute::get_global_default();
        }

        SdrShadowAttribute& SdrShadowAttribute::operator=(const SdrShadowAttribute& rCandidate)
        {
            if(rCandidate.mpSdrShadowAttribute != mpSdrShadowAttribute)
            {
                if(mpSdrShadowAttribute->mnRefCount)
                {
                    mpSdrShadowAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrShadowAttribute;
                }

                mpSdrShadowAttribute = rCandidate.mpSdrShadowAttribute;
                mpSdrShadowAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrShadowAttribute::operator==(const SdrShadowAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrShadowAttribute == mpSdrShadowAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrShadowAttribute == *mpSdrShadowAttribute);
        }

        const basegfx::B2DVector& SdrShadowAttribute::getOffset() const
        {
            return mpSdrShadowAttribute->getOffset();
        }

        double SdrShadowAttribute::getTransparence() const
        {
            return mpSdrShadowAttribute->getTransparence();
        }

        const basegfx::BColor& SdrShadowAttribute::getColor() const
        {
            return mpSdrShadowAttribute->getColor();
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
