/*************************************************************************
 *
 *  $RCSfile: shellexecentry.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 13:07:09 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//-----------------------------------------------------------------------
//  includes of other projects
//-----------------------------------------------------------------------

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _SHELLEXEC_HXX_
#include "shellexec.hxx"
#endif

//-----------------------------------------------------------------------
// namespace directives
//-----------------------------------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::cppu;
using com::sun::star::system::XSystemShellExecute;

//-----------------------------------------------------------------------
// defines
//-----------------------------------------------------------------------

#define SHELLEXEC_SERVICE_NAME  "com.sun.star.system.SystemShellExecute"
#define SHELLEXEC_IMPL_NAME     "com.sun.star.comp.system.SystemShellExecute"
#define SHELLEXEC_REGKEY_NAME   "/com.sun.star.comp.system.SystemShellExecute/UNO/SERVICES/com.sun.star.system.SystemShellExecute"

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------

namespace
{
    Reference< XInterface > SAL_CALL createInstance(const Reference< XComponentContext >& xContext)
    {
        return Reference< XInterface >( static_cast< XSystemShellExecute* >( new ShellExec(xContext) ) );
    }
}

//-----------------------------------------------------------------------
// the 3 important functions which will be exported
//-----------------------------------------------------------------------

extern "C"
{

//----------------------------------------------------------------------
// component_getImplementationEnvironment
//----------------------------------------------------------------------

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//-----------------------------------------------------------------------
// component_writeInfo
//-----------------------------------------------------------------------

sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > pXNewKey( static_cast< XRegistryKey* >( pRegistryKey ) );
            pXNewKey->createKey( OUString( RTL_CONSTASCII_USTRINGPARAM( SHELLEXEC_REGKEY_NAME ) ) );
            return sal_True;
        }
        catch( InvalidRegistryException& )
        {
            OSL_ENSURE(sal_False, "InvalidRegistryException caught");
    }
    }

    return sal_False;
}

//----------------------------------------------------------------------
// component_getFactory
//----------------------------------------------------------------------

void* SAL_CALL component_getFactory( const sal_Char* pImplName, uno_Interface* pSrvManager, uno_Interface* pRegistryKey )
{
    Reference< XSingleComponentFactory > xFactory;

    if (0 == ::rtl_str_compare( pImplName, SHELLEXEC_IMPL_NAME ))
    {
        OUString serviceName( RTL_CONSTASCII_USTRINGPARAM(SHELLEXEC_SERVICE_NAME) );

        xFactory = ::cppu::createSingleComponentFactory(
            createInstance,
            OUString( RTL_CONSTASCII_USTRINGPARAM(SHELLEXEC_IMPL_NAME) ),
            Sequence< OUString >( &serviceName, 1 ) );

    }

    if (xFactory.is())
    xFactory->acquire();

    return xFactory.get();
}

} // extern "C"
