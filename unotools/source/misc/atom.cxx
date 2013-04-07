/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <unotools/atom.hxx>

using namespace utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

AtomProvider::AtomProvider()
{
    m_nAtoms = 1;
}

AtomProvider::~AtomProvider()
{
}

int AtomProvider::getAtom( const OUString& rString, sal_Bool bCreate )
{
    ::boost::unordered_map< OUString, int, OUStringHash >::iterator it = m_aAtomMap.find( rString );
    if( it != m_aAtomMap.end() )
        return it->second;
    if( ! bCreate )
        return INVALID_ATOM;
    m_aAtomMap[ rString ] = m_nAtoms;
    m_aStringMap[ m_nAtoms ] = rString;
    m_nAtoms++;
    return m_nAtoms-1;
}

const OUString& AtomProvider::getString( int nAtom ) const
{
    static OUString aEmpty;
    ::boost::unordered_map< int, OUString, ::boost::hash< int > >::const_iterator it = m_aStringMap.find( nAtom );

    return it == m_aStringMap.end() ? aEmpty : it->second;
}

// -----------------------------------------------------------------------

MultiAtomProvider::MultiAtomProvider()
{
}

MultiAtomProvider::~MultiAtomProvider()
{
    for( ::boost::unordered_map< int, AtomProvider*, ::boost::hash< int > >::iterator it = m_aAtomLists.begin(); it != m_aAtomLists.end(); ++it )
        delete it->second;
}

int MultiAtomProvider::getAtom( int atomClass, const OUString& rString, sal_Bool bCreate )
{
    ::boost::unordered_map< int, AtomProvider*, ::boost::hash< int > >::iterator it =
          m_aAtomLists.find( atomClass );
    if( it != m_aAtomLists.end() )
        return it->second->getAtom( rString, bCreate );

    if( bCreate )
    {
        AtomProvider* pNewClass;
        m_aAtomLists[ atomClass ] = pNewClass = new AtomProvider();
        return pNewClass->getAtom( rString, bCreate );
    }
    return INVALID_ATOM;
}

const OUString& MultiAtomProvider::getString( int atomClass, int atom ) const
{
    ::boost::unordered_map< int, AtomProvider*, ::boost::hash< int > >::const_iterator it =
          m_aAtomLists.find( atomClass );
    if( it != m_aAtomLists.end() )
        return it->second->getString( atom );

    static OUString aEmpty;
    return aEmpty;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
