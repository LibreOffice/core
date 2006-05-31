/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: debug.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2006-05-31 10:12:11 $
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
