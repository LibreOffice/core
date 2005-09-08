/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rschash.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:31:52 $
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

