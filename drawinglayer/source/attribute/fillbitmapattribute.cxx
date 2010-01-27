/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillbitmapattribute.cxx,v $
 *
 *  $Revision: 1.4 $
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

#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpFillBitmapAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            BitmapEx                                maBitmapEx;
            basegfx::B2DPoint                       maTopLeft;
            basegfx::B2DVector                      maSize;

            // bitfield
            unsigned                                mbTiling : 1;

            ImpFillBitmapAttribute(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DPoint& rTopLeft,
                const basegfx::B2DVector& rSize,
                bool bTiling)
            :   mnRefCount(0),
                maBitmapEx(rBitmapEx),
                maTopLeft(rTopLeft),
                maSize(rSize),
                mbTiling(bTiling)
            {
            }

            bool operator==(const ImpFillBitmapAttribute& rCandidate) const
            {
                return (maBitmapEx == rCandidate.maBitmapEx
                    && maTopLeft == rCandidate.maTopLeft
                    && maSize == rCandidate.maSize
                    && mbTiling == rCandidate.mbTiling);
            }

            // data read access
            const BitmapEx& getBitmapEx() const { return maBitmapEx; }
            const basegfx::B2DPoint& getTopLeft() const { return maTopLeft; }
            const basegfx::B2DVector& getSize() const { return maSize; }
            bool getTiling() const { return mbTiling; }

            static ImpFillBitmapAttribute* get_global_default()
            {
                static ImpFillBitmapAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpFillBitmapAttribute(
                        BitmapEx(),
                        basegfx::B2DPoint(),
                        basegfx::B2DVector(),
                        false);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        FillBitmapAttribute::FillBitmapAttribute(
            const BitmapEx& rBitmapEx,
            const basegfx::B2DPoint& rTopLeft,
            const basegfx::B2DVector& rSize,
            bool bTiling)
        :   mpFillBitmapAttribute(new ImpFillBitmapAttribute(
                rBitmapEx, rTopLeft, rSize, bTiling))
        {
        }

        FillBitmapAttribute::FillBitmapAttribute()
        :   mpFillBitmapAttribute(ImpFillBitmapAttribute::get_global_default())
        {
            mpFillBitmapAttribute->mnRefCount++;
        }

        FillBitmapAttribute::FillBitmapAttribute(const FillBitmapAttribute& rCandidate)
        :   mpFillBitmapAttribute(rCandidate.mpFillBitmapAttribute)
        {
            mpFillBitmapAttribute->mnRefCount++;
        }

        FillBitmapAttribute::~FillBitmapAttribute()
        {
            if(mpFillBitmapAttribute->mnRefCount)
            {
                mpFillBitmapAttribute->mnRefCount--;
            }
            else
            {
                delete mpFillBitmapAttribute;
            }
        }

        bool FillBitmapAttribute::isDefault() const
        {
            return mpFillBitmapAttribute == ImpFillBitmapAttribute::get_global_default();
        }

        FillBitmapAttribute& FillBitmapAttribute::operator=(const FillBitmapAttribute& rCandidate)
        {
            if(rCandidate.mpFillBitmapAttribute != mpFillBitmapAttribute)
            {
                if(mpFillBitmapAttribute->mnRefCount)
                {
                    mpFillBitmapAttribute->mnRefCount--;
                }
                else
                {
                    delete mpFillBitmapAttribute;
                }

                mpFillBitmapAttribute = rCandidate.mpFillBitmapAttribute;
                mpFillBitmapAttribute->mnRefCount++;
            }

            return *this;
        }

        bool FillBitmapAttribute::operator==(const FillBitmapAttribute& rCandidate) const
        {
            if(rCandidate.mpFillBitmapAttribute == mpFillBitmapAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpFillBitmapAttribute == *mpFillBitmapAttribute);
        }

        const BitmapEx& FillBitmapAttribute::getBitmapEx() const
        {
            return mpFillBitmapAttribute->getBitmapEx();
        }

        const basegfx::B2DPoint& FillBitmapAttribute::getTopLeft() const
        {
            return mpFillBitmapAttribute->getTopLeft();
        }

        const basegfx::B2DVector& FillBitmapAttribute::getSize() const
        {
            return mpFillBitmapAttribute->getSize();
        }

        bool FillBitmapAttribute::getTiling() const
        {
            return mpFillBitmapAttribute->getTiling();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
