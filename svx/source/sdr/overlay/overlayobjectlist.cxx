/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlayobjectlist.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_svx.hxx"
#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <tools/debug.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

// for SOLARIS compiler include of algorithm part of _STL is necesary to
// get access to basic algos like ::std::find
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        OverlayObjectList::~OverlayObjectList()
        {
            clear();
        }

        void OverlayObjectList::clear()
        {
            OverlayObjectVector::iterator aStart(maVector.begin());

            for(; aStart != maVector.end(); aStart++)
            {
                ::sdr::overlay::OverlayObject* pCandidate = *aStart;

                if(pCandidate->getOverlayManager())
                {
                    pCandidate->getOverlayManager()->remove(*pCandidate);
                }

                delete pCandidate;
            }

            maVector.clear();
        }

        void OverlayObjectList::remove(OverlayObject& rOverlayObject)
        {
            const OverlayObjectVector::iterator aFindResult = ::std::find(maVector.begin(), maVector.end(), &rOverlayObject);
            DBG_ASSERT((aFindResult != maVector.end()),
                "OverlayObjectList::remove: Could not find given object in list (!)");
            maVector.erase(aFindResult);
        }

        sal_Bool OverlayObjectList::isHit(const basegfx::B2DPoint& rPos, double fTol) const
        {
            if(maVector.size())
            {
                OverlayObjectVector::const_iterator aStart(maVector.begin());

                if(0.0 == fTol)
                {
                    ::sdr::overlay::OverlayObject* pCandidate = *aStart;
                    OverlayManager* pManager = pCandidate->getOverlayManager();

                    if(pManager)
                    {
                        Size aSizeLogic(pManager->getOutputDevice().PixelToLogic(
                            Size(DEFAULT_VALUE_FOR_HITTEST_PIXEL, DEFAULT_VALUE_FOR_HITTEST_PIXEL)));
                        fTol = aSizeLogic.Width();
                    }
                }

                for(; aStart != maVector.end(); aStart++)
                {
                    ::sdr::overlay::OverlayObject* pCandidate = *aStart;

                    if(pCandidate->isHit(rPos, fTol))
                    {
                        return sal_True;
                    }
                }
            }

            return sal_False;
        }

        sal_Bool OverlayObjectList::isHitPixel(const Point& rPos, sal_uInt32 nTol) const
        {
            if(maVector.size())
            {
                OverlayObjectVector::const_iterator aStart(maVector.begin());
                ::sdr::overlay::OverlayObject* pCandidate = *aStart;
                OverlayManager* pManager = pCandidate->getOverlayManager();

                if(pManager)
                {
                    Point aPosLogic(pManager->getOutputDevice().PixelToLogic(rPos));
                    Size aSizeLogic(pManager->getOutputDevice().PixelToLogic(Size(nTol, nTol)));
                    basegfx::B2DPoint aPosition(aPosLogic.X(), aPosLogic.Y());

                    return isHit(aPosition, (double)aSizeLogic.Width());
                }
            }

            return sal_False;
        }

        basegfx::B2DRange OverlayObjectList::getBaseRange() const
        {
            basegfx::B2DRange aRetval;

            if(maVector.size())
            {
                OverlayObjectVector::const_iterator aStart(maVector.begin());

                for(; aStart != maVector.end(); aStart++)
                {
                    ::sdr::overlay::OverlayObject* pCandidate = *aStart;
                    aRetval.expand(pCandidate->getBaseRange());
                }
            }

            return aRetval;
        }

        void OverlayObjectList::transform(const basegfx::B2DHomMatrix& rMatrix)
        {
            if(!rMatrix.isIdentity() && maVector.size())
            {
                OverlayObjectVector::iterator aStart(maVector.begin());

                for(; aStart != maVector.end(); aStart++)
                {
                    ::sdr::overlay::OverlayObject* pCandidate = *aStart;
                    pCandidate->transform(rMatrix);
                }
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
