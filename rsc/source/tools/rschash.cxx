/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rschash.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 16:04:07 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_rsc.hxx"

#ifndef _RSCHASH_HXX
#include <rschash.hxx>
#endif

using namespace rtl;

AtomContainer::AtomContainer()
{
    m_aStringToID[ OString() ] = 0;
    m_aIDToString[ 0 ] = OString();
    m_nNextID = 1;
}

AtomContainer::~AtomContainer()
{
}

Atom AtomContainer::getID( const OString& rStr, bool bOnlyIfExists )
{
    OString aKey = rStr.toAsciiLowerCase();
    std::hash_map< OString, Atom, OStringHash >::const_iterator it =
        m_aStringToID.find( aKey );
    if( it != m_aStringToID.end() )
        return it->second;

    if( bOnlyIfExists )
        return InvalidAtom;

    Atom aRet = m_nNextID;
    m_aStringToID[ aKey ] = m_nNextID;
    m_aIDToString[ m_nNextID ] = rStr;
    m_nNextID++;
    return aRet;
}

const OString& AtomContainer::getString( Atom nAtom )
{
    std::hash_map< Atom, OString >::const_iterator it =
        m_aIDToString.find( nAtom );
    return (it != m_aIDToString.end()) ? it->second : m_aIDToString[0];
}
