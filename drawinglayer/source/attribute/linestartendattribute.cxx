/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linestartendattribute.cxx,v $
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

#include <drawinglayer/attribute/linestartendattribute.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpLineStartEndAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            double                                  mfWidth;                // absolute line StartEndGeometry base width
            basegfx::B2DPolyPolygon                 maPolyPolygon;          // the StartEndGeometry PolyPolygon

            // bitfield
            unsigned                                mbCentered : 1;         // use centered to ineStart/End point?

            ImpLineStartEndAttribute(
                double fWidth,
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                bool bCentered)
            :   mnRefCount(0),
                mfWidth(fWidth),
                maPolyPolygon(rPolyPolygon),
                mbCentered(bCentered)
            {
            }

            // data read access
            double getWidth() const { return mfWidth; }
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
            bool isCentered() const { return mbCentered; }

            bool operator==(const ImpLineStartEndAttribute& rCandidate) const
            {
                return (basegfx::fTools::equal(getWidth(), rCandidate.getWidth())
                    && getB2DPolyPolygon() == rCandidate.getB2DPolyPolygon()
                    && isCentered() == rCandidate.isCentered());
            }

            static ImpLineStartEndAttribute* get_global_default()
            {
                static ImpLineStartEndAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpLineStartEndAttribute(
                        0.0,
                        basegfx::B2DPolyPolygon(),
                        false);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        LineStartEndAttribute::LineStartEndAttribute(
            double fWidth,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            bool bCentered)
        :   mpLineStartEndAttribute(new ImpLineStartEndAttribute(
                fWidth, rPolyPolygon, bCentered))
        {
        }

        LineStartEndAttribute::LineStartEndAttribute()
        :   mpLineStartEndAttribute(ImpLineStartEndAttribute::get_global_default())
        {
            mpLineStartEndAttribute->mnRefCount++;
        }

        LineStartEndAttribute::LineStartEndAttribute(const LineStartEndAttribute& rCandidate)
        :   mpLineStartEndAttribute(rCandidate.mpLineStartEndAttribute)
        {
            mpLineStartEndAttribute->mnRefCount++;
        }

        LineStartEndAttribute::~LineStartEndAttribute()
        {
            if(mpLineStartEndAttribute->mnRefCount)
            {
                mpLineStartEndAttribute->mnRefCount--;
            }
            else
            {
                delete mpLineStartEndAttribute;
            }
        }

        bool LineStartEndAttribute::isDefault() const
        {
            return mpLineStartEndAttribute == ImpLineStartEndAttribute::get_global_default();
        }

        LineStartEndAttribute& LineStartEndAttribute::operator=(const LineStartEndAttribute& rCandidate)
        {
            if(rCandidate.mpLineStartEndAttribute != mpLineStartEndAttribute)
            {
                if(mpLineStartEndAttribute->mnRefCount)
                {
                    mpLineStartEndAttribute->mnRefCount--;
                }
                else
                {
                    delete mpLineStartEndAttribute;
                }

                mpLineStartEndAttribute = rCandidate.mpLineStartEndAttribute;
                mpLineStartEndAttribute->mnRefCount++;
            }

            return *this;
        }

        bool LineStartEndAttribute::operator==(const LineStartEndAttribute& rCandidate) const
        {
            if(rCandidate.mpLineStartEndAttribute == mpLineStartEndAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpLineStartEndAttribute == *mpLineStartEndAttribute);
        }

        double LineStartEndAttribute::getWidth() const
        {
            return mpLineStartEndAttribute->getWidth();
        }

        const basegfx::B2DPolyPolygon& LineStartEndAttribute::getB2DPolyPolygon() const
        {
            return mpLineStartEndAttribute->getB2DPolyPolygon();
        }

        bool LineStartEndAttribute::isCentered() const
        {
            return mpLineStartEndAttribute->isCentered();
        }

        bool LineStartEndAttribute::isActive() const
        {
            return (0.0 != getWidth()
                && 0 != getB2DPolyPolygon().count()
                && 0 != getB2DPolyPolygon().getB2DPolygon(0).count());
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
