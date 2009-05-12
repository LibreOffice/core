/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rschash.hxx,v $
 * $Revision: 1.6 $
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
#ifndef _RSCHASH_HXX
#define _RSCHASH_HXX

#include <sal/types.h>
#include <rtl/string.hxx>
#include <hash_map>

typedef sal_uInt32 Atom;

#define InvalidAtom Atom( ~0 )

class AtomContainer
{
    Atom m_nNextID;
    std::hash_map< rtl::OString, Atom, rtl::OStringHash > m_aStringToID;
    std::hash_map< Atom, rtl::OString > m_aIDToString;

    public:
    AtomContainer();
    ~AtomContainer();

    Atom getID( const rtl::OString& rStr, bool bOnlyIfExists = false );
    const rtl::OString& getString( Atom nAtom );

};

#endif // _RSCHASH_HXX

