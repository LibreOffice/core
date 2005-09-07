/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: guardedarray.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:34:42 $
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

#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_SHARED_GUARDEDARRAY_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_SHARED_GUARDEDARRAY_HXX

namespace bridges { namespace cpp_uno { namespace shared {

template< typename T > class GuardedArray {
public:
    explicit GuardedArray(T * thePointer): pointer(thePointer) {}

    ~GuardedArray() { delete[] pointer; }

    T * get() const { return pointer; }

    T * release() { T * p = pointer; pointer = 0; return p; }

private:
    GuardedArray(GuardedArray &); // not implemented
    void operator =(GuardedArray); // not implemented

    T * pointer;
};

} } }

#endif
