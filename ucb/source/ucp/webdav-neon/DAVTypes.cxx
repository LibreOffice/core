/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "osl/time.h"

#include "DAVTypes.hxx"
#include "../inc/urihelper.hxx"
#include "NeonUri.hxx"

using namespace webdav_ucp;
using namespace com::sun::star;

// DAVCapabilities implementation

DAVOptions::DAVOptions() :
    m_isResourceFound( false ),
    m_isClass1( false ),
    m_isClass2( false ),
    m_isClass3( false ),
    m_aAllowedMethods(),
    m_nStaleTime( 0 ),
    m_sURL(),
    m_sRedirectedURL()
{
}


DAVOptions::DAVOptions( const DAVOptions & rOther ) :
    m_isResourceFound( rOther.m_isResourceFound ),
    m_isClass1( rOther.m_isClass1 ),
    m_isClass2( rOther.m_isClass2 ),
    m_isClass3( rOther.m_isClass3 ),
    m_aAllowedMethods( rOther.m_aAllowedMethods ),
    m_nStaleTime( rOther.m_nStaleTime ),
    m_sURL( rOther.m_sURL ),
    m_sRedirectedURL( rOther.m_sRedirectedURL)
{
}


DAVOptions::~DAVOptions()
{
}


bool DAVOptions::operator==( const DAVOptions& rOpts ) const
{
    return
        m_isResourceFound == rOpts.m_isResourceFound &&
        m_isClass1 == rOpts.m_isClass1 &&
        m_isClass2 == rOpts.m_isClass2 &&
        m_isClass3 == rOpts.m_isClass3 &&
        m_aAllowedMethods == rOpts.m_aAllowedMethods &&
        m_nStaleTime == rOpts.m_nStaleTime &&
        m_sURL == rOpts.m_sURL &&
        m_sRedirectedURL == rOpts.m_sRedirectedURL;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
