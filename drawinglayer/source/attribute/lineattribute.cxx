/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lineattribute.cxx,v $
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

#include <drawinglayer/attribute/lineattribute.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpLineAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            basegfx::BColor                         maColor;                // color
            double                                  mfWidth;                // absolute line width
            basegfx::B2DLineJoin                    meLineJoin;             // type of LineJoin

            ImpLineAttribute(
                const basegfx::BColor& rColor,
                double fWidth,
                basegfx::B2DLineJoin aB2DLineJoin)
            :   mnRefCount(0),
                maColor(rColor),
                mfWidth(fWidth),
                meLineJoin(aB2DLineJoin)
            {
            }

            // data read access
            const basegfx::BColor& getColor() const { return maColor; }
            double getWidth() const { return mfWidth; }
            basegfx::B2DLineJoin getLineJoin() const { return meLineJoin; }

            bool operator==(const ImpLineAttribute& rCandidate) const
            {
                return (getColor() == rCandidate.getColor()
                    && getWidth() == rCandidate.getWidth()
                    && getLineJoin() == rCandidate.getLineJoin());
            }

            static ImpLineAttribute* get_global_default()
            {
                static ImpLineAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpLineAttribute(
                        basegfx::BColor(),
                        0.0,
                        basegfx::B2DLINEJOIN_ROUND);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        LineAttribute::LineAttribute(
            const basegfx::BColor& rColor,
            double fWidth,
            basegfx::B2DLineJoin aB2DLineJoin)
        :   mpLineAttribute(new ImpLineAttribute(
                rColor, fWidth, aB2DLineJoin))
        {
        }

        LineAttribute::LineAttribute()
        :   mpLineAttribute(ImpLineAttribute::get_global_default())
        {
            mpLineAttribute->mnRefCount++;
        }

        LineAttribute::LineAttribute(const LineAttribute& rCandidate)
        :   mpLineAttribute(rCandidate.mpLineAttribute)
        {
            mpLineAttribute->mnRefCount++;
        }

        LineAttribute::~LineAttribute()
        {
            if(mpLineAttribute->mnRefCount)
            {
                mpLineAttribute->mnRefCount--;
            }
            else
            {
                delete mpLineAttribute;
            }
        }

        bool LineAttribute::isDefault() const
        {
            return mpLineAttribute == ImpLineAttribute::get_global_default();
        }

        LineAttribute& LineAttribute::operator=(const LineAttribute& rCandidate)
        {
            if(rCandidate.mpLineAttribute != mpLineAttribute)
            {
                if(mpLineAttribute->mnRefCount)
                {
                    mpLineAttribute->mnRefCount--;
                }
                else
                {
                    delete mpLineAttribute;
                }

                mpLineAttribute = rCandidate.mpLineAttribute;
                mpLineAttribute->mnRefCount++;
            }

            return *this;
        }

        bool LineAttribute::operator==(const LineAttribute& rCandidate) const
        {
            if(rCandidate.mpLineAttribute == mpLineAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpLineAttribute == *mpLineAttribute);
        }

        const basegfx::BColor& LineAttribute::getColor() const
        {
            return mpLineAttribute->getColor();
        }

        double LineAttribute::getWidth() const
        {
            return mpLineAttribute->getWidth();
        }

        basegfx::B2DLineJoin LineAttribute::getLineJoin() const
        {
            return mpLineAttribute->getLineJoin();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
