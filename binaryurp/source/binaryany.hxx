/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
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

#ifndef INCLUDED_BINARYURP_SOURCE_BINARYANY_HXX
#define INCLUDED_BINARYURP_SOURCE_BINARYANY_HXX

#include "sal/config.h"

#include "uno/any2.h"

namespace com { namespace sun { namespace star { namespace uno {
    class TypeDescription;
} } } }

namespace binaryurp {

class BinaryAny {
public:
    BinaryAny() throw ();

    BinaryAny(com::sun::star::uno::TypeDescription const & type, void * value)
        throw ();

    explicit BinaryAny(uno_Any const & raw) throw ();
        // takes over raw.pData (but copies raw.pType); raw must not be passed
        // to uno_any_destruct

    BinaryAny(BinaryAny const & other) throw ();

    ~BinaryAny() throw ();

    BinaryAny & operator =(BinaryAny const & other) throw ();

    uno_Any * get() throw ();

    com::sun::star::uno::TypeDescription getType() const throw ();

    void * getValue(com::sun::star::uno::TypeDescription const & type) const
        throw ();

private:
    mutable uno_Any data_;
        // mutable so that getValue() can return a non-const void *, as in turn
        // required at various places in binary UNO
};

}

#endif
