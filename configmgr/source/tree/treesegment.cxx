/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treesegment.cxx,v $
 * $Revision: 1.8 $
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

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include <memory>

#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "salhelper/simplereferenceobject.hxx"

#include "builddata.hxx"
#include "treesegment.hxx"

namespace configmgr { namespace data {

rtl::Reference< TreeSegment > TreeSegment::create(
    std::auto_ptr< INode > tree, rtl::OUString const & type)
{
    rtl::Reference< TreeSegment > r;
    if (tree.get() != 0) {
        std::auto_ptr< sharable::TreeFragment > p(
            buildElementTree(*tree, type, false));
        r = new TreeSegment(p.get());
        p.release();
    }
    return r;
}

rtl::Reference< TreeSegment > TreeSegment::create(
    rtl::OUString const & name, std::auto_ptr< INode > tree)
{
    rtl::Reference< TreeSegment > r;
    if (tree.get() != 0) {
        std::auto_ptr< sharable::TreeFragment > p(
            buildTree(name, *tree, false));
        r = new TreeSegment(p.get());
        p.release();
    }
    return r;
}

rtl::Reference< TreeSegment > TreeSegment::create(
    sharable::TreeFragment * tree)
{
    rtl::Reference< TreeSegment > r;
    if (tree != 0) {
        std::auto_ptr< sharable::TreeFragment > p(data::buildTree(tree));
        r = new TreeSegment(p.get());
        p.release();
    }
    return r;
}

TreeSegment::TreeSegment(sharable::TreeFragment * tree): fragment(tree) {
    OSL_ASSERT(tree != 0);
}

TreeSegment::~TreeSegment() {
    destroyTree(fragment);
}

} }
