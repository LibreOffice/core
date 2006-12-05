/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlayobjectlist.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 12:10:53 $
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
#include "precompiled_svx.hxx"

#ifndef _SDR_OVERLAY_OVERLAYOBJECTLIST_HXX
#include <svx/sdr/overlay/overlayobjectlist.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYMANAGER_HXX
#include <svx/sdr/overlay/overlaymanager.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

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
