/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
