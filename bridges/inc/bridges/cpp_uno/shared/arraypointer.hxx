/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: arraypointer.hxx,v $
 * $Revision: 1.4 $
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

#ifndef INCLUDED_BRIDGES_CPP_UNO_SHARED_ARRAYPOINTER_HXX
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_ARRAYPOINTER_HXX

#include "sal/config.h"

namespace bridges { namespace cpp_uno { namespace shared {

/**
 * A simple smart pointer that holds an array until it is being released.
 */
template< typename T > class ArrayPointer {
public:
    ArrayPointer(T * p): p_(p) {}

    ~ArrayPointer() { delete[] p_; }

    T * release() { T * t = p_; p_ = 0; return t; }

private:
    ArrayPointer(ArrayPointer &); // not defined
    void operator =(ArrayPointer &); // not defined

    T * p_;
};

} } }

#endif
