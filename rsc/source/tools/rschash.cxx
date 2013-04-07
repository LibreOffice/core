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

#include <rschash.hxx>


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
