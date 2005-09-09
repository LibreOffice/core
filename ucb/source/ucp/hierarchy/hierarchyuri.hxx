/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hierarchyuri.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:49:18 $
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

#ifndef _HIERARCHYURI_HXX
#define _HIERARCHYURI_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace hierarchy_ucp {

//=========================================================================

#define HIERARCHY_URL_SCHEME          "vnd.sun.star.hier"
#define HIERARCHY_URL_SCHEME_LENGTH   17

//=========================================================================

class HierarchyUri
{
    mutable ::rtl::OUString m_aUri;
    mutable ::rtl::OUString m_aParentUri;
    mutable ::rtl::OUString m_aService;
    mutable ::rtl::OUString m_aPath;
    mutable ::rtl::OUString m_aName;
    mutable bool            m_bValid;

private:
    void init() const;
    static ::rtl::OUString decodeSegment( const ::rtl::OUString& rSource );

public:
    HierarchyUri() : m_bValid( false ) {}
    HierarchyUri( const ::rtl::OUString & rUri )
    : m_aUri( rUri ), m_bValid( false ) {}

    sal_Bool isValid() const
    { init(); return m_bValid; }

    const ::rtl::OUString & getUri() const
    { init(); return m_aUri; }

    void setUri( const ::rtl::OUString & rUri )
    { m_aPath = ::rtl::OUString(); m_aUri = rUri; m_bValid = false; }

    const ::rtl::OUString & getParentUri() const
    { init(); return m_aParentUri; }

    const ::rtl::OUString & getService() const
    { init(); return m_aService; }

    const ::rtl::OUString & getPath() const
    { init(); return m_aPath; }

    const ::rtl::OUString & getName() const
    { init(); return m_aName; }

    inline sal_Bool isRootFolder() const;

    static ::rtl::OUString encodeURL( const ::rtl::OUString & rURL );
    static ::rtl::OUString encodeSegment( const ::rtl::OUString & rSegment );
};

inline sal_Bool HierarchyUri::isRootFolder() const
{
    init();
    return ( ( m_aPath.getLength() == 1 ) &&
             ( m_aPath.getStr()[ 0 ] == sal_Unicode( '/' ) ) );
}
}

#endif
