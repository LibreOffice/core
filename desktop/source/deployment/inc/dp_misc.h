/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_misc.h,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:26:49 $
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

#if ! defined INCLUDED_DP_MISC_H
#define INCLUDED_DP_MISC_H

#include "rtl/ustrbuf.hxx"
#include "rtl/instance.hxx"
#include "osl/mutex.hxx"
#include "osl/process.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/deployment/XPackageRegistry.hpp"
#include "com/sun/star/awt/XWindow.hpp"

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_MISC_API_HXX
#include "dp_misc_api.hxx"
#endif

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define ARLEN(x) (sizeof (x) / sizeof *(x))

namespace dp_misc {

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
inline void try_dispose( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> const & x )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xComp( x, ::com::sun::star::uno::UNO_QUERY );
    if (xComp.is())
        xComp->dispose();
}

//##############################################################################

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::rtl::OUString expandUnoRcTerm( ::rtl::OUString const & term );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::rtl::OUString expandUnoRcUrl( ::rtl::OUString const & url );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC ::rtl::OUString const & getPlatformString();

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
bool platform_fits( ::rtl::OUString const & platform_string );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC ::rtl::OUString makeURL(
    ::rtl::OUString const & baseURL, ::rtl::OUString const & relPath );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC ::rtl::OUString generateRandomPipeId();

class AbortChannel;
//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> resolveUnoURL(
    ::rtl::OUString const & connectString,
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> const & xLocalContext,
    AbortChannel * abortChannel = 0 );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC bool office_is_running();

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
oslProcess raiseProcess( ::rtl::OUString const & appURL,
                         ::com::sun::star::uno::Sequence< ::rtl::OUString > const & args );

}

#endif
