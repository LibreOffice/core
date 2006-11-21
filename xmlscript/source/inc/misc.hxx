/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: misc.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 17:41:38 $
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

#if ! defined INCLUDED_XMLSCRIPT_MISC_HXX
#define INCLUDED_XMLSCRIPT_MISC_HXX

#include "com/sun/star/uno/Any.hxx"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


namespace xmlscript
{

//==============================================================================
template< typename T >
inline void extract_throw( T * p, ::com::sun::star::uno::Any const & a )
{
    if (! (a >>= *p))
    {
        throw ::com::sun::star::uno::RuntimeException(
            OUSTR("expected ") + ::getCppuType( p ).getTypeName(),
            ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface>() );
    }
}

//==============================================================================
template< typename T >
inline T extract_throw( ::com::sun::star::uno::Any const & a )
{
    T v = T();
    extract_throw<T>( &v, a );
    return v;
}

}

#endif
