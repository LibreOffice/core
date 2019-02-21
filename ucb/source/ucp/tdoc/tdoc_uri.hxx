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

#ifndef INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_URI_HXX
#define INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_URI_HXX

#include <rtl/ustring.hxx>

namespace tdoc_ucp {


#define TDOC_URL_SCHEME          "vnd.sun.star.tdoc"
#define TDOC_URL_SCHEME_LENGTH   17


class Uri
{
    enum State { UNKNOWN, INVALID, VALID };

    mutable OUString m_aUri;
    mutable OUString m_aParentUri;
    mutable OUString m_aPath;
    mutable OUString m_aDocId;
    mutable OUString m_aName;
    mutable OUString m_aDecodedName;
    mutable State           m_eState;

private:
    void init() const;

public:
    explicit Uri( const OUString & rUri )
    : m_aUri( rUri ), m_eState( UNKNOWN ) {}

    bool operator== ( const Uri & rOther ) const
    { init(); return m_aUri == rOther.m_aUri; }

    bool operator!= ( const Uri & rOther ) const
    { return !operator==( rOther ); }

    bool isValid() const
    { init(); return m_eState == VALID; }

    const OUString & getUri() const
    { init(); return m_aUri; }

    inline void setUri( const OUString & rUri );

    const OUString & getParentUri() const
    { init(); return m_aParentUri; }

    const OUString & getDocumentId() const
    { init(); return m_aDocId; }

    const OUString & getName() const
    { init(); return m_aName; }

    const OUString & getDecodedName() const
    { init(); return m_aDecodedName; }

    inline bool isRoot() const;

    inline bool isDocument() const;
};

inline void Uri::setUri( const OUString & rUri )
{
    m_eState = UNKNOWN;
    m_aUri = rUri;
    m_aParentUri.clear();
    m_aDocId.clear();
    m_aPath.clear();
    m_aName.clear();
    m_aDecodedName.clear();
}

inline bool Uri::isRoot() const
{
    init();
    return ( m_aPath.getLength() == 1 );
}

inline bool Uri::isDocument() const
{
    init();
    return ( ( !m_aDocId.isEmpty() ) /* not root */
             && ( m_aPath.copy( m_aDocId.getLength() + 1 ).getLength() < 2 ) );
}

} // namespace tdoc_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_URI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
