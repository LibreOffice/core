/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DAVProperties.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:33:29 $
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
#ifndef _DAVPROPERTIES_HXX_
#define _DAVPROPERTIES_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _NEONTYPES_HXX_
#include "NeonTypes.hxx"
#endif

namespace webdav_ucp
{

struct DAVProperties
{
    static const ::rtl::OUString CREATIONDATE;
    static const ::rtl::OUString DISPLAYNAME;
    static const ::rtl::OUString GETCONTENTLANGUAGE;
    static const ::rtl::OUString GETCONTENTLENGTH;
    static const ::rtl::OUString GETCONTENTTYPE;
    static const ::rtl::OUString GETETAG;
    static const ::rtl::OUString GETLASTMODIFIED;
    static const ::rtl::OUString LOCKDISCOVERY;
    static const ::rtl::OUString RESOURCETYPE;
    static const ::rtl::OUString SOURCE;
    static const ::rtl::OUString SUPPORTEDLOCK;
    static const ::rtl::OUString EXECUTABLE;

    static void createNeonPropName( const rtl::OUString & rFullName,
                                    NeonPropName & rName );
    static void createUCBPropName ( const char * nspace,
                                    const char * name,
                                    rtl::OUString & rFullName );

    static bool isUCBDeadProperty( const NeonPropName & rName );
};

} // namespace webdav_ucp

#endif // _DAVPROPERTIES_HXX_
