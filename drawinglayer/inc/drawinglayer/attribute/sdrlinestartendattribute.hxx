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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLINESTARTENDATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLINESTARTENDATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class B2DPolyPolygon;
}

namespace drawinglayer { namespace attribute {
    class ImpSdrLineStartEndAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrLineStartEndAttribute
        {
        private:
            ImpSdrLineStartEndAttribute*               mpSdrLineStartEndAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrLineStartEndAttribute(
                const basegfx::B2DPolyPolygon& rStartPolyPolygon,
                const basegfx::B2DPolyPolygon& rEndPolyPolygon,
                double fStartWidth,
                double fEndWidth,
                bool bStartActive,
                bool bEndActive,
                bool bStartCentered,
                bool bEndCentered);
            SdrLineStartEndAttribute();
            SdrLineStartEndAttribute(const SdrLineStartEndAttribute& rCandidate);
            SdrLineStartEndAttribute& operator=(const SdrLineStartEndAttribute& rCandidate);
            ~SdrLineStartEndAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrLineStartEndAttribute& rCandidate) const;

            // data read access
            const basegfx::B2DPolyPolygon& getStartPolyPolygon() const;
            const basegfx::B2DPolyPolygon& getEndPolyPolygon() const;
            double getStartWidth() const;
            double getEndWidth() const;
            bool isStartActive() const;
            bool isEndActive() const;
            bool isStartCentered() const;
            bool isEndCentered() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLINESTARTENDATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
