/*************************************************************************
 *
 *  $RCSfile: unohelp.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2000-10-25 09:32:51 $
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

#pragma hdrstop

#include <unohelp.hxx>

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/factory.hxx>

#include <uno/environment.h>
#include <uno/mapping.hxx>
#include <rtl/ustring.hxx>
#include <osl/module.h>

#ifndef _COM_SUN_STAR_TEXT_XBREAKITERATOR_HPP_
#include <com/sun/star/text/XBreakIterator.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCHARACTERCLASSIFICATION_HPP_
#include <com/sun/star/lang/XCharacterClassification.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCOLLATOR_HPP_
#include <com/sun/star/util/XCollator.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::rtl;

#define DOSTRING( x )                       #x
#define STRING( x )                         DOSTRING( x )

#define DOCONCAT4( x, y, z, a )             x##y##z##a
#define CONCAT4( x, y, z, a )               DOCONCAT4(x,y,z,a)
#define DOCONCAT5( x, y, z, a, b )          x##y##z##a##b
#define CONCAT5( x, y, z, a, b )            DOCONCAT5(x,y,z,a,b)

#ifdef UNX
#define LIBNAME(name)                       STRING(CONCAT5(lib,name,SUPD, DLLSUFFIX,.so))
#else
#define LIBNAME(name)                       STRING(CONCAT4( name, SUPD, DLLSUFFIX, .dll))
#endif

Reference< XSingleServiceFactory > ImplLoadLibComponentFactory(
    const OUString & rLibName, const OUString & rImplName,
    const Reference< XMultiServiceFactory > & xSF, const Reference< XRegistryKey > & xKey )
{
    Reference< XSingleServiceFactory > xRet;

    oslModule lib = osl_loadModule( rLibName.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    if (lib)
    {
        void * pSym;

        // ========================= LATEST VERSION =========================
        OUString aGetEnvName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETENV) );
        if (pSym = osl_getSymbol( lib, aGetEnvName.pData ))
        {
            uno_Environment * pCurrentEnv = 0;
            uno_Environment * pEnv = 0;
            const sal_Char * pEnvTypeName = 0;
            (*((component_getImplementationEnvironmentFunc)pSym))( &pEnvTypeName, &pEnv );

            sal_Bool bNeedsMapping =
                (pEnv || 0 != rtl_str_compare( pEnvTypeName, CPPU_CURRENT_LANGUAGE_BINDING_NAME ));

            OUString aEnvTypeName( OUString::createFromAscii( pEnvTypeName ) );

            if (bNeedsMapping)
            {
                if (! pEnv)
                    uno_getEnvironment( &pEnv, aEnvTypeName.pData, 0 );
                if (pEnv)
                {
                    OUString aCppEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
                    uno_getEnvironment( &pCurrentEnv, aCppEnvTypeName.pData, 0 );
                    if (pCurrentEnv)
                        bNeedsMapping = (pEnv != pCurrentEnv);
                }
            }

            OUString aGetFactoryName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETFACTORY) );
            if (pSym = osl_getSymbol( lib, aGetFactoryName.pData ))
            {
                OString aImplName( OUStringToOString( rImplName, RTL_TEXTENCODING_ASCII_US ) );

                if (bNeedsMapping)
                {
                    if (pEnv && pCurrentEnv)
                    {
                        Mapping aCurrent2Env( pCurrentEnv, pEnv );
                        Mapping aEnv2Current( pEnv, pCurrentEnv );

                        if (aCurrent2Env.is() && aEnv2Current.is())
                        {
                            void * pSMgr = aCurrent2Env.mapInterface(
                                xSF.get(), ::getCppuType( (const Reference< XMultiServiceFactory > *)0 ) );
                            void * pKey = aCurrent2Env.mapInterface(
                                xKey.get(), ::getCppuType( (const Reference< XRegistryKey > *)0 ) );

                            void * pSSF = (*((component_getFactoryFunc)pSym))(
                                aImplName.getStr(), pSMgr, pKey );

                            if (pKey)
                                (*pEnv->pExtEnv->releaseInterface)( pEnv->pExtEnv, pKey );
                            if (pSMgr)
                                (*pEnv->pExtEnv->releaseInterface)( pEnv->pExtEnv, pSMgr );

                            if (pSSF)
                            {
                                aEnv2Current.mapInterface(
                                    reinterpret_cast< void ** >( &xRet ),
                                    pSSF, ::getCppuType( (const Reference< XSingleServiceFactory > *)0 ) );
                                (*pEnv->pExtEnv->releaseInterface)( pEnv->pExtEnv, pSSF );
                            }
                        }
                    }
                }
                else
                {
                    XSingleServiceFactory * pRet = (XSingleServiceFactory *)
                        (*((component_getFactoryFunc)pSym))(
                            aImplName.getStr(), xSF.get(), xKey.get() );
                    if (pRet)
                    {
                        xRet = pRet;
                        pRet->release();
                    }
                }
            }

            if (pEnv)
                (*pEnv->release)( pEnv );
            if (pCurrentEnv)
                (*pCurrentEnv->release)( pCurrentEnv );
        }


        if (! xRet.is())
            osl_unloadModule( lib );
    }

    return xRet;
}

uno::Reference < text::XBreakIterator > vcl::unohelper::CreateBreakIterator()
{
    uno::Reference < text::XBreakIterator > xB;
    uno::Reference< lang::XMultiServiceFactory > xMSF = ::utl::getProcessServiceFactory();
    if ( xMSF.is() )
    {
        uno::Reference < uno::XInterface > xI = xMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.text.BreakIterator" ) );
        if ( xI.is() )
        {
            uno::Any x = xI->queryInterface( ::getCppuType((const uno::Reference< text::XBreakIterator >*)0) );
            x >>= xB;
        }
    }
    if( !xB.is() )
    {
        uno::Reference< lang::XSingleServiceFactory > xSSF = ImplLoadLibComponentFactory(
            OUString( RTL_CONSTASCII_USTRINGPARAM( LIBNAME( int ) ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.BreakIterator" ) ),
            Reference< XMultiServiceFactory >(), Reference< XRegistryKey >() );

        uno::Reference < uno::XInterface > xI = xSSF->createInstance();
        if ( xI.is() )
        {
            uno::Any x = xI->queryInterface( ::getCppuType((const uno::Reference< text::XBreakIterator >*)0) );
            x >>= xB;
        }
    }
    return xB;
}

uno::Reference < lang::XCharacterClassification > vcl::unohelper::CreateCharacterClassification()
{
    uno::Reference < lang::XCharacterClassification > xB;
    uno::Reference< lang::XMultiServiceFactory > xMSF = ::utl::getProcessServiceFactory();
    if ( xMSF.is() )
    {
        uno::Reference < uno::XInterface > xI = xMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.lang.CharacterClassification" ) );
        if ( xI.is() )
        {
            uno::Any x = xI->queryInterface( ::getCppuType((const uno::Reference< lang::XCharacterClassification >*)0) );
            x >>= xB;
        }
    }
    if( !xB.is() )
    {
        uno::Reference< lang::XSingleServiceFactory > xSSF = ImplLoadLibComponentFactory(
            OUString( RTL_CONSTASCII_USTRINGPARAM( LIBNAME( int ) ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.lang.CharacterClassification" ) ),
            Reference< XMultiServiceFactory >(), Reference< XRegistryKey >() );

        uno::Reference < uno::XInterface > xI = xSSF->createInstance();
        if ( xI.is() )
        {
            uno::Any x = xI->queryInterface( ::getCppuType((const uno::Reference< lang::XCharacterClassification >*)0) );
            x >>= xB;
        }
    }
    return xB;
}



uno::Reference < util::XCollator > vcl::unohelper::CreateCollator()
{
    uno::Reference < util::XCollator > xB;
    uno::Reference< lang::XMultiServiceFactory > xMSF = ::utl::getProcessServiceFactory();
    if ( xMSF.is() )
    {
        uno::Reference < uno::XInterface > xI = xMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.util.Collator" ) );
        if ( xI.is() )
        {
            uno::Any x = xI->queryInterface( ::getCppuType((const uno::Reference< util::XCollator >*)0) );
            x >>= xB;
        }
    }
    if( !xB.is() )
    {
        uno::Reference< lang::XSingleServiceFactory > xSSF = ImplLoadLibComponentFactory(
            OUString( RTL_CONSTASCII_USTRINGPARAM( LIBNAME( int ) ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.GNUcollator" ) ),
            Reference< XMultiServiceFactory >(), Reference< XRegistryKey >() );

        uno::Reference < uno::XInterface > xI = xSSF->createInstance();
        if ( xI.is() )
        {
            uno::Any x = xI->queryInterface( ::getCppuType((const uno::Reference< util::XCollator >*)0) );
            x >>= xB;
        }
    }
    return xB;
}


