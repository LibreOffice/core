/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
                               OUString >    HyperlinkRegion;

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

        protected:
            ~HyperlinkArea() {}
       };

        typedef boost::shared_ptr< HyperlinkArea > HyperlinkAreaSharedPtr;
    }
}

#endif /* INCLUDED_SLIDESHOW_HYPERLINKAREA_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
