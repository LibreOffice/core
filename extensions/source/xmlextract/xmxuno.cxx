/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmxuno.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 15:06:27 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "xmxcom.hxx"
#include "xmxtrct.hxx"
#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <uno/mapping.hxx>

// -------------------
// - factory methods -
// -------------------

static REF( NMSP_UNO::XInterface ) SAL_CALL create_XMLExtractor( const REF( NMSP_LANG::XMultiServiceFactory )& rxFact )
{
    return REF( NMSP_UNO::XInterface )( *new XMLExtractor( rxFact ) );
}

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * /*pServiceManager*/, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            REF( NMSP_REGISTRY::XRegistryKey ) xNewKey(
                reinterpret_cast< NMSP_REGISTRY::XRegistryKey * >( pRegistryKey )->createKey(
                    NMSP_RTL::OUString( RTL_CONSTASCII_USTRINGPARAM("/com.sun.star.comp.io.XMLExtractor/UNO/SERVICES/com.sun.star.io.XMLExtractor") ) ) );

            return sal_True;
        }
        catch (NMSP_REGISTRY::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, "com.sun.star.comp.io.XMLExtractor" ) == 0)
    {
        NMSP_RTL::OUString aServiceName( B2UCONST( "com.sum.star.io.XMLExtractor" ) );

        REF( NMSP_LANG::XSingleServiceFactory ) xFactory( NMSP_CPPU::createSingleFactory(
            reinterpret_cast< NMSP_LANG::XMultiServiceFactory * >( pServiceManager ),
            NMSP_RTL::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.io.XMLExtractor") ),
            create_XMLExtractor,
            SEQ( NMSP_RTL::OUString )( &aServiceName, 1 ) ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}
