/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treesegment.hxx,v $
 * $Revision: 1.7 $
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

#ifndef INCLUDED_CONFIGMGR_SOURCE_INC_TREESEGMENT_HXX
#define INCLUDED_CONFIGMGR_SOURCE_INC_TREESEGMENT_HXX

#include "sal/config.h"

#include <memory>

#include "rtl/ref.hxx"
#include "salhelper/simplereferenceobject.hxx"

namespace rtl { class OUString; }

namespace configmgr {
    class INode;
    namespace sharable { struct TreeFragment; }

    namespace data {
        // rtl::Reference< TreeSegment > is a reference counted
        // sharable::TreeFragment *.  A null reference is always modeled as an
        // empty rtl::Reference< TreeSegment >, never as a null fragment.
        class TreeSegment: public salhelper::SimpleReferenceObject {
        public:
            static rtl::Reference< TreeSegment > create(
                std::auto_ptr< INode > tree, rtl::OUString const & type);

            static rtl::Reference< TreeSegment > create(
                rtl::OUString const & name, std::auto_ptr< INode > tree);

            static rtl::Reference< TreeSegment > create(
                sharable::TreeFragment * tree);

            static rtl::Reference< TreeSegment > create(
                rtl::Reference< TreeSegment > const & tree)
            { return create(tree.is() ? tree->fragment : 0); }

            sharable::TreeFragment * const fragment; // non-null

        private:
            TreeSegment(sharable::TreeFragment * tree);

            virtual ~TreeSegment();
        };
    }
}

#endif
