/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pdfuno.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 11:14:51 $
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

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#include <pdffilter.hxx>
#include <pdfdialog.hxx>

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

                xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( PDFFilter_getImplementationName() );
                xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
                const Sequence< OUString > & rSNL1 = PDFFilter_getSupportedServiceNames();
                const OUString * pArray1 = rSNL1.getConstArray();
                for ( nPos = rSNL1.getLength(); nPos--; )
                    xNewKey->createKey( pArray1[nPos] );

                xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( PDFDialog_getImplementationName() );
                xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
                const Sequence< OUString > & rSNL2 = PDFDialog_getSupportedServiceNames();
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
            Reference< XSingleServiceFactory > xFactory;

            if( aImplName.equals( PDFFilter_getImplementationName() ) )
            {
                xFactory = createSingleFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ),
                                                OUString::createFromAscii( pImplName ),
                                                PDFFilter_createInstance, PDFFilter_getSupportedServiceNames() );

            }
            else if( aImplName.equals( PDFDialog_getImplementationName() ) )
            {
                xFactory = createSingleFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ),
                                                OUString::createFromAscii( pImplName ),
                                                PDFDialog_createInstance, PDFDialog_getSupportedServiceNames() );

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
