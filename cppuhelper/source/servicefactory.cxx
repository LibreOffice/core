/*************************************************************************
 *
 *  $RCSfile: servicefactory.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: kr $ $Date: 2001-09-03 14:28:17 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _RTL_PROCESS_H
#include <rtl/process.h>
#endif
#ifndef _RTL_BOOTSTRAP_H
#include <rtl/bootstrap.h>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <string.h>
#include <osl/security.hxx>

#include <rtl/bootstrap.hxx>

#include <cppuhelper/shlib.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/component_context.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;


namespace cppu
{

//==================================================================================================
static Reference< XInterface > SAL_CALL createInstance(
    Reference< XInterface > const & xFactory,
    Reference< XComponentContext > const & xContext = Reference< XComponentContext >() )
{
    Reference< lang::XSingleComponentFactory > xFac( xFactory, UNO_QUERY );
    if (xFac.is())
    {
        return xFac->createInstanceWithContext( xContext );
    }
    else
    {
        Reference< lang::XSingleServiceFactory > xFac( xFactory, UNO_QUERY );
        if (xFac.is())
        {
            OSL_ENSURE( !xContext.is(), "### ignoring context!" );
            return xFac->createInstance();
        }
    }
    throw Exception(
        OUString( RTL_CONSTASCII_USTRINGPARAM("no factory object given!") ),
        Reference< XInterface >() );
}
//==================================================================================================
Reference< registry::XSimpleRegistry > SAL_CALL createSimpleRegistry(
    OUString const & rBootstrapPath )
    SAL_THROW( () )
{
    try
    {
        return Reference< registry::XSimpleRegistry >(
            createInstance( loadSharedLibComponentFactory(
                OUString( RTL_CONSTASCII_USTRINGPARAM("simreg") ), rBootstrapPath,
                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.SimpleRegistry") ),
                Reference< lang::XMultiServiceFactory >(), Reference< registry::XRegistryKey >() ) ), UNO_QUERY );
    }
    catch (Exception &)
    {
    }

    OSL_ENSURE( 0, "### cannot instanciate simple registry!" );
    return Reference< registry::XSimpleRegistry >();
}
//==================================================================================================
Reference< registry::XSimpleRegistry > SAL_CALL createNestedRegistry(
    OUString const & rBootstrapPath )
    SAL_THROW( () )
{
    try
    {
        return Reference< registry::XSimpleRegistry >(
            createInstance( loadSharedLibComponentFactory(
                OUString( RTL_CONSTASCII_USTRINGPARAM("defreg") ), rBootstrapPath,
                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.NestedRegistry") ),
                Reference< lang::XMultiServiceFactory >(), Reference< registry::XRegistryKey >() ) ), UNO_QUERY );
    }
    catch (Exception &)
    {
    }

    OSL_ENSURE( 0, "### cannot instanciate simple registry!" );
    return Reference< registry::XSimpleRegistry >();
}

// private forward decl
Reference< XComponentContext > SAL_CALL createInitialComponentContext(
    ContextEntry_Init const * pEntries, sal_Int32 nEntries,
    Reference< registry::XSimpleRegistry > const & xRegistry )
    SAL_THROW( () );

//--------------------------------------------------------------------------------------------------
static Reference< lang::XMultiComponentFactory > bootstrapInitialSF(
    OUString const & rBootstrapPath )
    SAL_THROW( (Exception) )
{
    Reference< lang::XMultiServiceFactory > xSF( createInstance( loadSharedLibComponentFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("smgr") ), rBootstrapPath,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.ORegistryServiceManager") ),
        Reference< lang::XMultiServiceFactory >(), Reference< registry::XRegistryKey >() ) ), UNO_QUERY );

    // write initial shared lib loader, simple registry, default registry, impl reg
    Reference< container::XSet > xSet( xSF, UNO_QUERY );

    // loader
    {
    Any aFac( makeAny( loadSharedLibComponentFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("cpld") ), rBootstrapPath,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.DLLComponentLoader") ),
        xSF, Reference< registry::XRegistryKey >() ) ) );
    xSet->insert( aFac );
    OSL_ENSURE( xSet->has( aFac ), "### failed registering loader!" );
    }
    // simple registry
    {
    Any aFac( makeAny( loadSharedLibComponentFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("simreg") ), rBootstrapPath,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.SimpleRegistry") ),
        xSF, Reference< registry::XRegistryKey >() ) ) );
    xSet->insert( aFac );
    OSL_ENSURE( xSet->has( aFac ), "### failed registering simple registry!" );
    }
    // nested registry
    {
    Any aFac( makeAny( loadSharedLibComponentFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("defreg") ), rBootstrapPath,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.NestedRegistry") ),
        xSF, Reference< registry::XRegistryKey >() ) ) );
    xSet->insert( aFac );
    OSL_ENSURE( xSet->has( aFac ), "### failed registering default registry!" );
    }
    // td manager
    {
    Any aFac( makeAny( loadSharedLibComponentFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("tdmgr") ), rBootstrapPath,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.TypeDescriptionManager") ),
        xSF, Reference< registry::XRegistryKey >() ) ) );
    xSet->insert( aFac );
    OSL_ENSURE( xSet->has( aFac ), "### failed registering registry td provider!" );
    }
    // implementation registration
    {
    Any aFac( makeAny( loadSharedLibComponentFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("impreg") ), rBootstrapPath,
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.ImplementationRegistration") ),
        xSF, Reference< registry::XRegistryKey >() ) ) );
    xSet->insert( aFac );
    OSL_ENSURE( xSet->has( aFac ), "### failed registering impl reg!" );
    }

    return Reference< lang::XMultiComponentFactory >( xSF, UNO_QUERY );
}
//--------------------------------------------------------------------------------------------------
static Reference< XComponentContext > initializeSF(
    Reference< lang::XMultiComponentFactory > const & xSF,
    Reference< registry::XSimpleRegistry > const & types_xRegistry,
    Reference< registry::XSimpleRegistry > const & services_xRegistry,
    OUString const & rBootstrapPath )
    SAL_THROW( (Exception) )
{
    Reference< lang::XInitialization > xSFInit( xSF, UNO_QUERY );
    if (! xSFInit.is())
    {
        throw Exception(
            OUString( RTL_CONSTASCII_USTRINGPARAM("servicemanager does not support XInitialization!") ),
            Reference< XInterface >() );
    }

    // default initial context
    ContextEntry_Init context_values[ 4 ];
    // smgr
    context_values[ 0 ].bLateInitService = false;
    context_values[ 0 ].name = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.theServiceManager") );
    context_values[ 0 ].value <<= xSF;
    // tdmgr
    context_values[ 1 ].bLateInitService = true;
    context_values[ 1 ].name = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.theTypeDescriptionManager") );
    context_values[ 1 ].value <<= OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.TypeDescriptionManager") );
    // tdmgr: cache size
    context_values[ 2 ].bLateInitService = false;
    context_values[ 2 ].name = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.TypeDescriptionManager.CacheSize") );
    context_values[ 2 ].value <<= (sal_Int32)512;
    sal_Int32 nEntries = 3;

    if (types_xRegistry.is())
    {
        // add registry td provider
        Reference< container::XSet > xSet( xSF, UNO_QUERY );
        Any aFac( makeAny( loadSharedLibComponentFactory(
            OUString( RTL_CONSTASCII_USTRINGPARAM("rdbtdp") ), rBootstrapPath,
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.RegistryTypeDescriptionProvider") ),
            Reference< lang::XMultiServiceFactory >( xSF, UNO_QUERY ), Reference< registry::XRegistryKey >() ) ) );
        xSet->insert( aFac );
        OSL_ENSURE( xSet->has( aFac ), "### failed registering registry td provider!" );

        // rdbtdp: registries to be used
        context_values[ 3 ].bLateInitService = false;
        context_values[ 3 ].name = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.RegistryTypeDescriptionProvider.Registries") );
        context_values[ 3 ].value = makeAny( Sequence< Reference< registry::XSimpleRegistry > >( &types_xRegistry, 1 ) );
        nEntries = 4;
    }

    Reference< XComponentContext > xContext( createInitialComponentContext(
        context_values, nEntries, services_xRegistry ) );

    Sequence< Any > aSFInit( 2 );
    aSFInit[ 0 ] <<= services_xRegistry;
    aSFInit[ 1 ] <<= xContext; // default context
    xSFInit->initialize( aSFInit );

    Reference< container::XHierarchicalNameAccess > xTDMgr;
    if (xContext->getValueByName( OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.reflection.theTypeDescriptionManager") ) ) >>= xTDMgr)
    {
        installTypeDescriptionManager( xTDMgr );
    }

    return xContext;
}

//==================================================================================================
static Reference< lang::XMultiComponentFactory > createImplServiceFactory(
    const OUString & rWriteRegistry,
    const OUString & rReadRegistry,
    sal_Bool bReadOnly,
    const OUString & rBootstrapPath )
    SAL_THROW( (Exception) )
{
    Reference< lang::XMultiComponentFactory > xSF( bootstrapInitialSF( rBootstrapPath ) );

    Reference< registry::XSimpleRegistry > xRegistry;

    // open a registry
    sal_Bool bRegistryShouldBeValid = sal_False;
    if (rWriteRegistry.getLength() && !rReadRegistry.getLength())
    {
        bRegistryShouldBeValid = sal_True;
        xRegistry.set( createSimpleRegistry( rBootstrapPath ) );
        if (xRegistry.is())
        {
            if (bReadOnly)
            {
                xRegistry->open( rWriteRegistry, sal_True, sal_False );
            }
            else
            {
                xRegistry->open( rWriteRegistry, sal_False, sal_True );
            }
        }
    }
    else if (rWriteRegistry.getLength() && rReadRegistry.getLength()) // default registry
    {
        bRegistryShouldBeValid = sal_True;
        xRegistry.set( createNestedRegistry( rBootstrapPath ) );

        Reference< registry::XSimpleRegistry > xWriteReg( createSimpleRegistry( rBootstrapPath ) );
        if (xWriteReg.is())
        {
            if (bReadOnly)
            {
                try
                {
                    xWriteReg->open( rWriteRegistry, sal_True, sal_False );
                }
                catch (registry::InvalidRegistryException &)
                {
                }

                if (! xWriteReg->isValid())
                {
                    throw Exception(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("specified first registry could not be open readonly!") ),
                        Reference< XInterface >() );
                }
            }
            else
            {
                xWriteReg->open( rWriteRegistry, sal_False, sal_True );
            }
        }

        Reference< registry::XSimpleRegistry > xReadReg( createSimpleRegistry( rBootstrapPath ) );
        if (xReadReg.is())
        {
            xReadReg->open( rReadRegistry, sal_True, sal_False );
        }

        Reference< lang::XInitialization > xInit( xRegistry, UNO_QUERY );
        Sequence< Any > aInitSeq( 2 );
        aInitSeq[ 0 ] <<= xWriteReg;
        aInitSeq[ 1 ] <<= xReadReg;
        xInit->initialize( aInitSeq );
    }

    if (bRegistryShouldBeValid && (!xRegistry.is() || !xRegistry->isValid()))
    {
        throw Exception(
            OUString( RTL_CONSTASCII_USTRINGPARAM("specified registry could not be initialized") ),
            Reference< XInterface >() );
    }

    initializeSF( xSF, xRegistry, xRegistry, rBootstrapPath );

    return xSF;
}

//==================================================================================================
Reference< lang::XMultiServiceFactory > SAL_CALL createRegistryServiceFactory(
    const OUString & rWriteRegistry,
    const OUString & rReadRegistry,
    sal_Bool bReadOnly,
    const OUString & rBootstrapPath )
    SAL_THROW( (::com::sun::star::uno::Exception) )
{
    return Reference< lang::XMultiServiceFactory >( createImplServiceFactory(
        rWriteRegistry, rReadRegistry, bReadOnly, rBootstrapPath ), UNO_QUERY );
}

//==================================================================================================
Reference< XComponentContext > SAL_CALL bootstrap_InitialComponentContext(
    Reference< registry::XSimpleRegistry > const & xRegistry,
    OUString const & rBootstrapPath )
    SAL_THROW( (Exception) )
{
    Reference< lang::XMultiComponentFactory > xSF( bootstrapInitialSF( rBootstrapPath ) );
    return initializeSF( xSF, xRegistry, xRegistry, rBootstrapPath );
}



//==================================================================================================


static OUString findBoostrapArgument(const Bootstrap & bootstrap, const OUString & arg_name, sal_Bool * pFallenBack) SAL_THROW(()) {
    OUString result;

    OUString prefixed_arg_name = OUString(RTL_CONSTASCII_USTRINGPARAM("UNO_"));
    prefixed_arg_name += arg_name.toAsciiUpperCase();

    if(!bootstrap.getFrom(prefixed_arg_name, result)) // environment not set -> try relative to executable
    {
        if(pFallenBack)
            *pFallenBack = sal_True;

        OUString fileName;
        bootstrap.getIniName(fileName);

        result = fileName.copy(0, fileName.getLength() - strlen(SAL_CONFIGFILE(""))); // cut the rc extension
        result += OUString(RTL_CONSTASCII_USTRINGPARAM("_"));
        result += arg_name.toAsciiLowerCase();
        result += OUString(RTL_CONSTASCII_USTRINGPARAM(".rdb"));

#ifdef DEBUG
        OString result_dbg = OUStringToOString(result, RTL_TEXTENCODING_ASCII_US);
        OString arg_name_dbg = OUStringToOString(arg_name, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("cppuhelper::findBoostrapArgument - setting %s relative to executable: %s\n",
                  arg_name_dbg.getStr(),
                  result_dbg.getStr());
#endif
    }
    else
    {
        if(pFallenBack)
            *pFallenBack = sal_False;

#ifdef DEBUG
        OString prefixed_arg_name_dbg = OUStringToOString(prefixed_arg_name, RTL_TEXTENCODING_ASCII_US);
        OString result_dbg = OUStringToOString(result, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("cppuhelper::findBoostrapArgument - found %s in env: %s",
                  prefixed_arg_name_dbg.getStr(),
                  result_dbg.getStr());
#endif
    }

    return result;
}


static Reference<XSimpleRegistry> nestRegistries(const OUString baseDir,
                                                 const Reference<XSingleServiceFactory> & xSimRegFac,
                                                 const Reference<XSingleServiceFactory> & xNesRegFac,
                                                 OUString csl_rdbs,
                                                 const OUString & write_rdb,
                                                 sal_Bool forceWrite_rdb)
    SAL_THROW((Exception))
{
    sal_Int32 index;
    Reference<XSimpleRegistry> lastRegistry;

    if(write_rdb.getLength()) // is there a write registry given?
    {
        lastRegistry = Reference<XSimpleRegistry>::query(xSimRegFac->createInstance());

        try
        {
            OSL_TRACE("opening xxxxx");
              lastRegistry->open(write_rdb, sal_False, forceWrite_rdb);
            OSL_TRACE("opening yyyy");

        }
        catch (InvalidRegistryException & invalidRegistryException)
        {
#ifdef DEBUG
            OString rdb_name_tmp = OUStringToOString(write_rdb, RTL_TEXTENCODING_ASCII_US);
            OString message_dbg = OUStringToOString(invalidRegistryException.Message, RTL_TEXTENCODING_ASCII_US);
            OSL_TRACE("warning: couldn't open %s cause of %s", rdb_name_tmp.getStr(), message_dbg.getStr());
#endif
        }

        if(!lastRegistry->isValid())
            lastRegistry = Reference<XSimpleRegistry>();
    }

    do
    {
        index = csl_rdbs.indexOf((sal_Unicode)' ');
        OUString rdb_name = (index == -1) ? csl_rdbs : csl_rdbs.copy(0, index);
        csl_rdbs = (index == -1) ? OUString() : csl_rdbs.copy(index + 1);

        try
        {
            Reference<XSimpleRegistry> simpleRegistry = Reference<XSimpleRegistry>::query(xSimRegFac->createInstance());

            osl::FileBase::getAbsoluteFileURL(baseDir, rdb_name, rdb_name);
            simpleRegistry->open(rdb_name, sal_True, sal_False);

            if(lastRegistry.is())
            {
                Reference<XSimpleRegistry> nestedRegistry = Reference<XSimpleRegistry>::query(xNesRegFac->createInstance());
                Reference<XInitialization> nestedRegistry_xInit(nestedRegistry, UNO_QUERY);

                Sequence<Any> aArgs(2);
                aArgs[0] = makeAny(lastRegistry);
                aArgs[1] = makeAny(simpleRegistry);

                nestedRegistry_xInit->initialize(aArgs);

                lastRegistry = nestedRegistry;
            }
            else
                lastRegistry = simpleRegistry;
        }
        catch(InvalidRegistryException & invalidRegistryException)
        {
#ifdef DEBUG
            OString rdb_name_tmp = OUStringToOString(rdb_name, RTL_TEXTENCODING_ASCII_US);
            OString message_dbg = OUStringToOString(invalidRegistryException.Message, RTL_TEXTENCODING_ASCII_US);
            OSL_TRACE("warning: couldn't open %s cause of %s", rdb_name_tmp.getStr(), message_dbg.getStr());
#endif
        }
    }
    while(index != -1 && csl_rdbs.getLength()); // are there more rdbs in list?

    return lastRegistry;
}

/**
 * Bootstrap an <code>InitialContext</code>, try to find the
 * neccessarey types and services rdbs.
 *
 * @return the XComponentContext
 */
Reference<XComponentContext> SAL_CALL defaultBootstrap_InitialComponentContext(const OUString & iniFile) SAL_THROW((Exception))
{
    Bootstrap bootstrap(iniFile);

      OUString cls_uno_types = findBoostrapArgument(bootstrap, OUString(RTL_CONSTASCII_USTRINGPARAM("TYPES")), NULL);
      OUString cls_uno_services = findBoostrapArgument(bootstrap, OUString(RTL_CONSTASCII_USTRINGPARAM("SERVICES")), NULL);

    sal_Bool fallenBack;
      OUString write_rdb = findBoostrapArgument(bootstrap, OUString(RTL_CONSTASCII_USTRINGPARAM("WRITERDB")), &fallenBack);

    OUString bootstrapPath;

    Reference<XMultiComponentFactory> smgr_XMultiComponentFactory(bootstrapInitialSF(bootstrapPath));
    Reference<XMultiServiceFactory> smgr_XMultiServiceFactory(smgr_XMultiComponentFactory, UNO_QUERY);

    Reference<XRegistryKey> xEmptyKey;

    Reference<XSingleServiceFactory> xSimRegFac(
        loadSharedLibComponentFactory(
            OUString(RTL_CONSTASCII_USTRINGPARAM("simreg")),
            bootstrapPath,
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.SimpleRegistry")),
            smgr_XMultiServiceFactory,
            xEmptyKey),
        UNO_QUERY);

    Reference<XSingleServiceFactory> xNesRegFac(
        loadSharedLibComponentFactory(
            OUString( RTL_CONSTASCII_USTRINGPARAM("defreg")),
            bootstrapPath,
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.NestedRegistry")),
            smgr_XMultiServiceFactory,
            xEmptyKey),
        UNO_QUERY);

    OUString iniDir;
    bootstrap.getIniName(iniDir);
    iniDir = iniDir.copy(0, iniDir.lastIndexOf('/'));

    Reference<XSimpleRegistry> types_xRegistry = nestRegistries(iniDir, xSimRegFac, xNesRegFac, cls_uno_types, OUString(), sal_False);
    Reference<XSimpleRegistry> services_xRegistry = nestRegistries(iniDir, xSimRegFac, xNesRegFac, cls_uno_services, write_rdb, !fallenBack);

    return initializeSF(smgr_XMultiComponentFactory, types_xRegistry, services_xRegistry, bootstrapPath);
}

Reference<XComponentContext> SAL_CALL defaultBootstrap_InitialComponentContext() SAL_THROW((Exception))
{
    return defaultBootstrap_InitialComponentContext(OUString());
}
}
