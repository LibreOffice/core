/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DAVTypes.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:35:00 $
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

#ifndef _DAVTYPES_HXX_
#define _DAVTYPES_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

namespace webdav_ucp
{
/* RFC 2518

15.1 Class 1

   A class 1 compliant resource MUST meet all "MUST" requirements in all
   sections of this document.

   Class 1 compliant resources MUST return, at minimum, the value "1" in
   the DAV header on all responses to the OPTIONS method.

15.2 Class 2

   A class 2 compliant resource MUST meet all class 1 requirements and
   support the LOCK method, the supportedlock property, the
   lockdiscovery property, the Time-Out response header and the Lock-
   Token request header.  A class "2" compliant resource SHOULD also
   support the Time-Out request header and the owner XML element.

   Class 2 compliant resources MUST return, at minimum, the values "1"
   and "2" in the DAV header on all responses to the OPTIONS method.
*/

struct DAVCapabilities
{
    bool class1;
    bool class2;
    bool executable; // supports "executable" property (introduced by mod_dav)

    DAVCapabilities() : class1( false ), class2( false ), executable( false ) {}
};

enum Depth { DAVZERO = 0, DAVONE = 1, DAVINFINITY = -1 };

enum ProppatchOperation { PROPSET = 0, PROPREMOVE = 1 };

struct ProppatchValue
{
    ProppatchOperation       operation;
    rtl::OUString            name;
    com::sun::star::uno::Any value;

    ProppatchValue( const ProppatchOperation o,
                    const rtl::OUString & n,
                    const com::sun::star::uno::Any & v )
    : operation( o ), name( n ), value( v ) {}
};

} // namespace webdav_ucp

#endif // _DAVTYPES_HXX_
