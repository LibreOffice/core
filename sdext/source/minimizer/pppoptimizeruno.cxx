/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pppoptimizeruno.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2007-05-11 14:00:33 $
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

#include <stdio.h>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <pppoptimizer.hxx>
#include <pppoptimizerdialog.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment ** )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    // -------------------------------------------------------------------------

    sal_Bool SAL_CALL component_writeInfo( void* /*pServiceManager*/, void* pRegistryKey )
    {
        if (pRegistryKey)
        {
            try
            {
                Reference< XRegistryKey >   xNewKey;
                sal_Int32                   nPos;

                xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( PPPOptimizer_getImplementationName() );
                xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
                const Sequence< OUString > & rSNL1 = PPPOptimizer_getSupportedServiceNames();
                const OUString * pArray1 = rSNL1.getConstArray();
                for ( nPos = rSNL1.getLength(); nPos--; )
                    xNewKey->createKey( pArray1[nPos] );

                xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( PPPOptimizerDialog_getImplementationName() );
                xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
                const Sequence< OUString > & rSNL2 = PPPOptimizerDialog_getSupportedServiceNames();
                const OUString * pArray2 = rSNL2.getConstArray();
                for ( nPos = rSNL2.getLength(); nPos--; )
                    xNewKey->createKey( pArray2[nPos] );

                return sal_True;
            }
            catch (InvalidRegistryException &)
            {
                OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
            }
        }
        return sal_False;
    }

    // -------------------------------------------------------------------------

    void* SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
    {
        OUString    aImplName( OUString::createFromAscii( pImplName ) );
        void*       pRet = 0;

        if( pServiceManager )
        {
            Reference< XSingleComponentFactory > xFactory;
            if( aImplName.equals( PPPOptimizer_getImplementationName() ) )
            {
                xFactory = createSingleComponentFactory(
                        PPPOptimizer_createInstance,
                        OUString::createFromAscii( pImplName ),
                        PPPOptimizer_getSupportedServiceNames() );

            }
            else if( aImplName.equals( PPPOptimizerDialog_getImplementationName() ) )
            {
                xFactory = createSingleComponentFactory(
                        PPPOptimizerDialog_createInstance,
                        OUString::createFromAscii( pImplName ),
                        PPPOptimizerDialog_getSupportedServiceNames() );
            }
            if( xFactory.is() )
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        return pRet;
    }
}
