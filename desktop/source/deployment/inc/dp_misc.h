/*************************************************************************
 *
 *  $RCSfile: dp_misc.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 15:29:08 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if ! defined INCLUDED_DP_MISC_H
#define INCLUDED_DP_MISC_H

#include "rtl/ustrbuf.hxx"
#include "osl/mutex.hxx"
#include "osl/process.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/deployment/XPackageRegistry.hpp"
#include "com/sun/star/awt/XWindow.hpp"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define ARLEN(x) (sizeof (x) / sizeof *(x))


namespace css = ::com::sun::star;

namespace dp_misc
{

const sal_Char CR = 0x0d;
const sal_Char LF = 0x0a;

//==============================================================================
class MutexHolder
{
    mutable ::osl::Mutex m_mutex;
protected:
    inline ::osl::Mutex & getMutex() const { return m_mutex; }
};

//==============================================================================
inline void try_dispose( css::uno::Reference<css::uno::XInterface> const & x )
{
    css::uno::Reference<css::lang::XComponent> xComp( x, css::uno::UNO_QUERY );
    if (xComp.is())
        xComp->dispose();
}

//##############################################################################

//==============================================================================
template< typename T >
inline void extract_throw( T * p, css::uno::Any const & a )
{
    if (! (a >>= *p))
    {
        throw css::uno::RuntimeException(
            OUSTR("expected ") + ::getCppuType( p ).getTypeName(),
            css::uno::Reference<css::uno::XInterface>() );
    }
}

//==============================================================================
template< typename T >
inline T extract_throw( css::uno::Any const & a )
{
    T v;
    extract_throw<T>( &v, a );
    return v;
}

//##############################################################################

//==============================================================================
::rtl::OUString const & getPlatformString();

//==============================================================================
bool platform_fits( ::rtl::OUString const & platform_string );

//==============================================================================
::rtl::OUString make_url(
    ::rtl::OUString const & base_url, ::rtl::OUString const & url );

//==============================================================================
::rtl::OUString expand_url( ::rtl::OUString const & url );

//==============================================================================
::rtl::OUString generateRandomPipeId();

class AbortChannel;
//==============================================================================
css::uno::Reference<css::uno::XInterface> resolveUnoURL(
    ::rtl::OUString const & connectString,
    css::uno::Reference<css::uno::XComponentContext> const & xLocalContext,
    AbortChannel * abortChannel = 0 );

//==============================================================================
bool office_is_running();

//==============================================================================
oslProcess raiseProcess( ::rtl::OUString const & appURL,
                         css::uno::Sequence< ::rtl::OUString > const & args );

}

#endif
