/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: debug.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_BASEBMP_DEBUG_HXX
#define INCLUDED_BASEBMP_DEBUG_HXX

#include <iostream>
#include <boost/shared_ptr.hpp>

namespace basebmp
{
    class BitmapDevice;

    /** Dump content of BitmapDevice to given output stream.

        @param rDevice
        Device whose content should be dumped.

        @param rOutputStream
        Stream to write output to.
    */
    void debugDump( const boost::shared_ptr< BitmapDevice >& rDevice,
                    ::std::ostream&                          rOutputStream );
}

#endif /* INCLUDED_BASEBMP_DEBUG_HXX */
