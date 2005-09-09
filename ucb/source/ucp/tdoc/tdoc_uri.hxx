/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdoc_uri.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:04:39 $
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

#ifndef INCLUDED_TDOC_URI_HXX
#define INCLUDED_TDOC_URI_HXX

#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

namespace tdoc_ucp {

//=========================================================================

#define TDOC_URL_SCHEME          "vnd.sun.star.tdoc"
#define TDOC_URL_SCHEME_LENGTH   17

//=========================================================================

class Uri
{
    enum State { UNKNOWN, INVALID, VALID };

    mutable ::rtl::OUString m_aUri;
    mutable ::rtl::OUString m_aParentUri;
    mutable ::rtl::OUString m_aPath;
    mutable ::rtl::OUString m_aDocId;
    mutable ::rtl::OUString m_aInternalPath;
    mutable ::rtl::OUString m_aName;
    mutable ::rtl::OUString m_aDecodedName;
    mutable State           m_eState;

private:
    void init() const;

public:
    Uri() : m_eState( UNKNOWN ) {}
    Uri( const ::rtl::OUString & rUri )
    : m_aUri( rUri ), m_eState( UNKNOWN ) {}

    bool operator== ( const Uri & rOther ) const
    { init(); return m_aUri == rOther.m_aUri; }

    bool operator!= ( const Uri & rOther ) const
    { return !operator==( rOther ); }

    sal_Bool isValid() const
    { init(); return m_eState == VALID; }

    const ::rtl::OUString & getUri() const
    { init(); return m_aUri; }

    inline void setUri( const ::rtl::OUString & rUri );

    const ::rtl::OUString & getParentUri() const
    { init(); return m_aParentUri; }

    const ::rtl::OUString & getPath() const
    { init(); return m_aPath; }

    const ::rtl::OUString & getDocumentId() const
    { init(); return m_aDocId; }

    const ::rtl::OUString & getInternalPath() const
    { init(); return m_aInternalPath; }

    const ::rtl::OUString & getName() const
    { init(); return m_aName; }

    const ::rtl::OUString & getDecodedName() const
    { init(); return m_aDecodedName; }

    inline sal_Bool isRoot() const;

    inline sal_Bool isDocument() const;

    inline sal_Bool isFolder() const;

    static ::rtl::OUString encodeURL( const ::rtl::OUString & rURL );
    static ::rtl::OUString encodeSegment( const ::rtl::OUString & rSegment );
    static ::rtl::OUString decodeSegment( const rtl::OUString& rSegment );
};

inline void Uri::setUri( const ::rtl::OUString & rUri )
{
    m_eState = UNKNOWN;
    m_aUri = rUri;
    m_aParentUri = m_aDocId = m_aInternalPath = m_aPath = m_aName
        = m_aDecodedName = rtl::OUString();
}

inline sal_Bool Uri::isRoot() const
{
    init();
    return ( m_aPath.getLength() == 1 );
}

inline sal_Bool Uri::isDocument() const
{
    init();
    return ( ( m_aDocId.getLength() > 0 ) /* not root */
             && ( m_aPath.copy( m_aDocId.getLength() + 1 ).getLength() < 2 ) );
}

inline sal_Bool Uri::isFolder() const
{
    init();
    return ( m_aPath.lastIndexOf( '/' ) == m_aPath.getLength() - 1 );
}

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_URI_HXX */
