/*************************************************************************
 *
 *  $RCSfile: hierarchyuri.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2001-07-03 11:13:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
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
    HierarchyUri() {}
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
