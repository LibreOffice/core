/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_rsc.hxx"
#include <rschash.hxx>

using ::rtl::OString;
using ::rtl::OStringHash;

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
    boost::unordered_map< OString, Atom, OStringHash >::const_iterator it =
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
    boost::unordered_map< Atom, OString >::const_iterator it =
        m_aIDToString.find( nAtom );
    return (it != m_aIDToString.end()) ? it->second : m_aIDToString[0];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
