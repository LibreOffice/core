/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrallattribute3d.cxx,v $
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

#include <drawinglayer/attribute/sdrallattribute3d.hxx>
#include <drawinglayer/attribute/sdrattribute.hxx>
#include <drawinglayer/attribute/fillattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// pointer compare define
#define pointerOrContentEqual(p, q) ((p == q) || (p && q && *p == *q))

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrLineFillShadowAttribute::SdrLineFillShadowAttribute(
            SdrLineAttribute* pLine,
            SdrFillAttribute* pFill,
            SdrLineStartEndAttribute* pLineStartEnd,
            SdrShadowAttribute* pShadow,
            FillGradientAttribute* pFillFloatTransGradient)
        :   mpShadow(pShadow),
            mpLine(pLine),
            mpLineStartEnd(pLineStartEnd),
            mpFill(pFill),
            mpFillFloatTransGradient(pFillFloatTransGradient)
        {
        }

        SdrLineFillShadowAttribute::SdrLineFillShadowAttribute(
            const SdrLineFillShadowAttribute& rCandidate)
        :   mpShadow(0),
            mpLine(0),
            mpLineStartEnd(0),
            mpFill(0),
            mpFillFloatTransGradient(0)
        {
            *this = rCandidate;
        }

        SdrLineFillShadowAttribute::~SdrLineFillShadowAttribute()
        {
            delete mpShadow;
            delete mpLine;
            delete mpLineStartEnd;
            delete mpFill;
            delete mpFillFloatTransGradient;
        }

        SdrLineFillShadowAttribute& SdrLineFillShadowAttribute::operator=(const SdrLineFillShadowAttribute& rCandidate)
        {
            // handle mpShadow
            {
                // delete local mpShadow if necessary
                if(mpShadow)
                {
                    delete mpShadow;
                    mpShadow = 0;
                }

                // copy mpShadow if necessary
                if(rCandidate.mpShadow)
                {
                    mpShadow = new SdrShadowAttribute(*rCandidate.mpShadow);
                }
            }

            // handle mpLine
            {
                // delete local mpLine if necessary
                if(mpLine)
                {
                    delete mpLine;
                    mpLine = 0;
                }

                // copy mpLine if necessary
                if(rCandidate.mpLine)
                {
                    mpLine = new SdrLineAttribute(*rCandidate.mpLine);
                }
            }

            // handle mpLineStartEnd
            {
                // delete local mpLineStartEnd if necessary
                if(mpLineStartEnd)
                {
                    delete mpLineStartEnd;
                    mpLineStartEnd = 0;
                }

                // copy mpLineStartEnd if necessary
                if(rCandidate.mpLineStartEnd)
                {
                    mpLineStartEnd = new SdrLineStartEndAttribute(*rCandidate.mpLineStartEnd);
                }
            }

            // handle mpFill
            {
                // delete local mpFill if necessary
                if(mpFill)
                {
                    delete mpFill;
                    mpFill = 0;
                }

                // copy mpFill if necessary
                if(rCandidate.mpFill)
                {
                    mpFill = new SdrFillAttribute(*rCandidate.mpFill);
                }
            }

            // handle mpFillFloatTransGradient
            {
                // delete local mpFillFloatTransGradient if necessary
                if(mpFillFloatTransGradient)
                {
                    delete mpFillFloatTransGradient;
                    mpFillFloatTransGradient = 0;
                }

                // copy mpFillFloatTransGradient if necessary
                if(rCandidate.mpFillFloatTransGradient)
                {
                    mpFillFloatTransGradient = new FillGradientAttribute(*rCandidate.mpFillFloatTransGradient);
                }
            }

            return *this;
        }

        bool SdrLineFillShadowAttribute::operator==(const SdrLineFillShadowAttribute& rCandidate) const
        {
            // handle mpShadow
            if(!pointerOrContentEqual(mpShadow, rCandidate.mpShadow))
                return false;

            // handle mpLine
            if(!pointerOrContentEqual(mpLine, rCandidate.mpLine))
                return false;

            // handle mpLineStartEnd
            if(!pointerOrContentEqual(mpLineStartEnd, rCandidate.mpLineStartEnd))
                return false;

            // handle mpFill
            if(!pointerOrContentEqual(mpFill, rCandidate.mpFill))
                return false;

            // handle mpFillFloatTransGradient
            if(!pointerOrContentEqual(mpFillFloatTransGradient, rCandidate.mpFillFloatTransGradient))
                return false;

            return true;
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
