/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyperlinkarea.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:08:21 $
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

#ifndef INCLUDED_SLIDESHOW_HYPERLINKAREA_HXX
#define INCLUDED_SLIDESHOW_HYPERLINKAREA_HXX

#include <boost/shared_ptr.hpp>
#include <vector>
#include <utility>

namespace rtl {
    class OUString;
}
namespace basegfx {
    class B2DRange;
}

/* Definition of HyperlinkArea interface */

namespace slideshow
{
    namespace internal
    {
        /** HyperlinkArea interface

            Implementers of this interface provide information for
            hyperlink sensitive areas.
         */
        class HyperlinkArea
        {
        public:
            typedef std::pair< ::basegfx::B2DRange,
                               ::rtl::OUString >    HyperlinkRegion;

            typedef std::vector<HyperlinkRegion>    HyperlinkRegions;

            /** Request hyperlink-sensitive areas.

                @return a vector of hyperlink-sensitive areas, plus
                the URI associated to them.
             */
            virtual HyperlinkRegions getHyperlinkRegions() const = 0;

            /** Retrieve priority of link area

                @return the priority of the link area. Link areas with
                higher priority will receive hyperlink clicks in favor
                of areas with less priority, if they cover the same
                place on screen.
             */
            virtual double getHyperlinkPriority() const = 0;

            /** Functor struct, for area ordering

                This defines a strict weak ordering of areas, sort key
                is the object ptr value. Most typical use is for
                associative containers holding areas.
             */
            struct lessThanArea
            {
                // make functor adaptable (to boost::bind)
                typedef bool result_type;

                bool operator()(const boost::shared_ptr< HyperlinkArea >& rLHS,
                                const boost::shared_ptr< HyperlinkArea >& rRHS) const
                {
                    const double nPrioL( rLHS->getHyperlinkPriority() );
                    const double nPrioR( rRHS->getHyperlinkPriority() );

                    // if prios are equal, tie-break on ptr value
                    return nPrioL == nPrioR ? rLHS.get() < rRHS.get() : nPrioL < nPrioR;
                }
            };
       };

        typedef boost::shared_ptr< HyperlinkArea > HyperlinkAreaSharedPtr;
    }
}

#endif /* INCLUDED_SLIDESHOW_HYPERLINKAREA_HXX */
