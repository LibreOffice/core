/*************************************************************************
 *
 *  $RCSfile: tdprovider.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 15:04:44 $
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
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif
#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include "com/sun/star/uno/RuntimeException.hpp"

#include "registry/reader.hxx"
#include "registry/version.h"

#ifndef _STOC_RDBTDP_BASE_HXX
#include "base.hxx"
#endif
#ifndef _STOC_RDBTDP_TDENUMERATION_HXX
#include "rdbtdp_tdenumeration.hxx"
#endif
#include "structtypedescription.hxx"

#define SERVICENAME "com.sun.star.reflection.TypeDescriptionProvider"
#define IMPLNAME    "com.sun.star.comp.stoc.RegistryTypeDescriptionProvider"

using namespace com::sun::star::beans;
using namespace com::sun::star::registry;

namespace stoc_rdbtdp
{
rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

static Sequence< OUString > rdbtdp_getSupportedServiceNames()
{
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(1);
            seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICENAME));
            pNames = &seqNames;
        }
    }
    return *pNames;
}

static OUString rdbtdp_getImplementationName()
{
    static OUString *pImplName = 0;
    if( ! pImplName )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pImplName )
        {
            static OUString implName( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME ) );
            pImplName = &implName;
        }
    }
    return *pImplName;
}

struct MutexHolder
{
    Mutex _aComponentMutex;
};
//==================================================================================================
class ProviderImpl
    : public MutexHolder
    , public WeakComponentImplHelper4< XServiceInfo,
                                       XHierarchicalNameAccess,
                                       XTypeDescriptionEnumerationAccess,
                                       XInitialization >
{
    com::sun::star::uno::Reference< XComponentContext >              _xContext;
    com::sun::star::uno::Reference< XHierarchicalNameAccess >        _xTDMgr;
    com::sun::star::uno::Reference< XHierarchicalNameAccess > getTDMgr() SAL_THROW( () );

    RegistryKeyList                             _aBaseKeys;

protected:
    virtual void SAL_CALL disposing();

public:
    ProviderImpl( const com::sun::star::uno::Reference< XComponentContext > & xContext );
    virtual ~ProviderImpl();

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any > & args ) throw (Exception, RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XHierarchicalNameAccess
    virtual Any SAL_CALL getByHierarchicalName( const OUString & rName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);

    // XTypeDescriptionEnumerationAccess
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::reflection::XTypeDescriptionEnumeration > SAL_CALL
    createTypeDescriptionEnumeration(
        const ::rtl::OUString& moduleName,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::TypeClass >& types,
        ::com::sun::star::reflection::TypeDescriptionSearchDepth depth )
            throw ( ::com::sun::star::reflection::NoSuchTypeNameException,
                    ::com::sun::star::reflection::InvalidTypeNameException,
                    ::com::sun::star::uno::RuntimeException );
};
//__________________________________________________________________________________________________
ProviderImpl::ProviderImpl( const com::sun::star::uno::Reference< XComponentContext > & xContext )
    : WeakComponentImplHelper4<
        XServiceInfo, XHierarchicalNameAccess,
        XTypeDescriptionEnumerationAccess, XInitialization >( _aComponentMutex )
    , _xContext( xContext )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
ProviderImpl::~ProviderImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
com::sun::star::uno::Reference< XHierarchicalNameAccess > ProviderImpl::getTDMgr()
    SAL_THROW( () )
{
    if (! _xTDMgr.is())
    {
        com::sun::star::uno::Reference< XHierarchicalNameAccess > xTDMgr;
        _xContext->getValueByName( OUString( RTL_CONSTASCII_USTRINGPARAM(
            "/singletons/com.sun.star.reflection.theTypeDescriptionManager") ) ) >>= xTDMgr;
        OSL_ENSURE( xTDMgr.is(), "### cannot get singleton \"TypeDescriptionManager\" from context!" );

        {
        MutexGuard guard( _aComponentMutex );
        if (! _xTDMgr.is())
        {
            _xTDMgr = xTDMgr;
        }
        }
    }
    return _xTDMgr;
}

//__________________________________________________________________________________________________
void ProviderImpl::disposing()
{
    _xTDMgr.clear();
    _xContext.clear();

    for ( RegistryKeyList::const_iterator iPos( _aBaseKeys.begin() );
          iPos != _aBaseKeys.end(); ++iPos )
    {
        (*iPos)->closeKey();
    }
    _aBaseKeys.clear();
}

// XInitialization
//__________________________________________________________________________________________________
void ProviderImpl::initialize(
    const Sequence< Any > & args )
    throw (Exception, RuntimeException)
{
    // registries to read from
    Any const * pRegistries = args.getConstArray();
    for ( sal_Int32 nPos = 0; nPos < args.getLength(); ++nPos )
    {
        com::sun::star::uno::Reference< XSimpleRegistry > xRegistry( pRegistries[ nPos ], UNO_QUERY );
        if (xRegistry.is() && xRegistry->isValid())
        {
            com::sun::star::uno::Reference< XRegistryKey > xKey( xRegistry->getRootKey()->openKey(
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UCR") ) ) );
            if (xKey.is() && xKey->isValid())
            {
                _aBaseKeys.push_back( xKey );
            }
        }
    }
}

// XServiceInfo
//__________________________________________________________________________________________________
OUString ProviderImpl::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return rdbtdp_getImplementationName();
}
//__________________________________________________________________________________________________
sal_Bool ProviderImpl::supportsService( const OUString & rServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    const Sequence< OUString > & rSNL = getSupportedServiceNames();
    const OUString * pArray = rSNL.getConstArray();
    for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
    {
        if (pArray[nPos] == rServiceName)
            return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< OUString > ProviderImpl::getSupportedServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    return rdbtdp_getSupportedServiceNames();
}

// XHierarchicalNameAccess
//__________________________________________________________________________________________________
Any SAL_CALL ProviderImpl::getByHierarchicalName( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException, com::sun::star::container::NoSuchElementException)
{
    Any aRet;

    // read from registry
    OUString aKey( rName.replace( '.', '/' ) );
    for ( RegistryKeyList::const_iterator iPos( _aBaseKeys.begin() );
          !aRet.hasValue() && iPos != _aBaseKeys.end(); ++iPos )
    {
        try
        {
            com::sun::star::uno::Reference< XRegistryKey > xBaseKey( *iPos );
            com::sun::star::uno::Reference< XRegistryKey > xKey( xBaseKey->openKey( aKey ) );
            if (xKey.is())
            {
                // closes key in it's dtor (which is
                // called even in case of exceptions).
                RegistryKeyCloser aCloser( xKey );

                if ( xKey->isValid() )
                {
                    if (xKey->getValueType() == RegistryValueType_BINARY)
                    {
                        Sequence< sal_Int8 > aBytes( xKey->getBinaryValue() );
                        com::sun::star::uno::Reference< XTypeDescription > xTD(
                            createTypeDescription( aBytes,
                                                   getTDMgr(),
                                                   true ) );
                        if ( xTD.is() )
                            aRet <<= xTD;
                    }
                }
            }
            else // might be a constant
            {
                sal_Int32 nIndex = aKey.lastIndexOf( '/' );
                if (nIndex > 0)
                {
                    // open module
                    com::sun::star::uno::Reference< XRegistryKey > xKey( xBaseKey->openKey( aKey.copy( 0, nIndex ) ) );
                    if (xKey.is())
                    {
                        // closes key in it's dtor (which is
                        // called even in case of exceptions).
                        RegistryKeyCloser aCloser( xKey );

                        if ( xKey->isValid() )
                        {
                            if (xKey->getValueType() == RegistryValueType_BINARY)
                            {
                                Sequence< sal_Int8 > aBytes( xKey->getBinaryValue() );
                                typereg::Reader aReader(
                                    aBytes.getConstArray(), aBytes.getLength(),
                                    false, TYPEREG_VERSION_1);

                                if (aReader.getTypeClass() == RT_TYPE_MODULE ||
                                    aReader.getTypeClass() == RT_TYPE_CONSTANTS ||
                                    aReader.getTypeClass() == RT_TYPE_ENUM)
                                {
                                    OUString aFieldName( aKey.copy( nIndex+1, aKey.getLength() - nIndex -1 ) );
                                    sal_Int16 nPos = aReader.getFieldCount();
                                    while (nPos--)
                                    {
                                        if (aFieldName.equals(
                                                aReader.getFieldName(nPos)))
                                            break;
                                    }
                                    if (nPos >= 0)
                                        aRet = getRTValue(
                                            aReader.getFieldValue(nPos));
                                }
                            }
                        }
                    }
                }
            }
        }
        catch ( InvalidRegistryException const & )
        {
            OSL_ENSURE( sal_False,
                        "ProviderImpl::getByHierarchicalName "
                        "- Caught InvalidRegistryException!" );

            // openKey, closeKey, getValueType, getBinaryValue, isValid

            // Don't stop iteration in this case.
        }
    }

    if ( !aRet.hasValue() )
        throw NoSuchElementException(
            rName, static_cast< cppu::OWeakObject * >( this  ) );

    return aRet;
}

//__________________________________________________________________________________________________
sal_Bool ProviderImpl::hasByHierarchicalName( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    try
    {
        return getByHierarchicalName( rName ).hasValue();
    }
    catch (NoSuchElementException &)
    {
    }
    return sal_False;
}

// XTypeDescriptionEnumerationAccess
//__________________________________________________________________________________________________
// virtual
com::sun::star::uno::Reference< XTypeDescriptionEnumeration > SAL_CALL
ProviderImpl::createTypeDescriptionEnumeration(
        const OUString & moduleName,
        const Sequence< TypeClass > & types,
        TypeDescriptionSearchDepth depth )
    throw ( NoSuchTypeNameException,
            InvalidTypeNameException,
            RuntimeException )
{
    return com::sun::star::uno::Reference< XTypeDescriptionEnumeration >(
        TypeDescriptionEnumerationImpl::createInstance( _xContext,
                                                        moduleName,
                                                        types,
                                                        depth,
                                                        _aBaseKeys ).get() );
}

//==================================================================================================
static com::sun::star::uno::Reference< XInterface > SAL_CALL ProviderImpl_create(
    com::sun::star::uno::Reference< XComponentContext > const & xContext )
    throw(::com::sun::star::uno::Exception)
{
    return com::sun::star::uno::Reference< XInterface >( *new ProviderImpl( xContext ) );
}

//__________________________________________________________________________________________________
// global helper function

com::sun::star::uno::Reference< XTypeDescription > resolveTypedefs(
    com::sun::star::uno::Reference< XTypeDescription > const & type)
{
    com::sun::star::uno::Reference< XTypeDescription > resolved(type);
    while (resolved->getTypeClass() == TypeClass_TYPEDEF) {
        resolved = com::sun::star::uno::Reference< XIndirectTypeDescription >(
            resolved, UNO_QUERY_THROW)->getReferencedType();
    }
    return resolved;
}

com::sun::star::uno::Reference< XTypeDescription > createTypeDescription(
    const Sequence< sal_Int8 > & rData,
    const com::sun::star::uno::Reference< XHierarchicalNameAccess > & xNameAccess,
    bool bReturnEmptyRefForUnknownType )
{
    typereg::Reader aReader(
        rData.getConstArray(), rData.getLength(), false, TYPEREG_VERSION_1);

    OUString aName( aReader.getTypeName().replace( '/', '.' ) );

    switch (aReader.getTypeClass())
    {
        case RT_TYPE_INTERFACE:
        {
            sal_uInt16 n = aReader.getSuperTypeCount();
            com::sun::star::uno::Sequence< rtl::OUString > aBaseTypeNames(n);
            {for (sal_uInt16 i = 0; i < n; ++i) {
                aBaseTypeNames[i] = aReader.getSuperTypeName(i).replace(
                    '/', '.');
            }}
            sal_uInt16 n2 = aReader.getReferenceCount();
            com::sun::star::uno::Sequence< rtl::OUString >
                aOptionalBaseTypeNames(n2);
            {for (sal_uInt16 i = 0; i < n2; ++i) {
                OSL_ASSERT(
                    aReader.getReferenceSort(i) == RT_REF_SUPPORTS
                    && aReader.getReferenceFlags(i) == RT_ACCESS_OPTIONAL);
                aOptionalBaseTypeNames[i] = aReader.getReferenceTypeName(i);
            }}
            return com::sun::star::uno::Reference< XTypeDescription >(
                new InterfaceTypeDescriptionImpl( xNameAccess,
                                                  aName,
                                                  aBaseTypeNames,
                                                  aOptionalBaseTypeNames,
                                                  rData,
                                                  aReader.isPublished() ) );
        }

        case RT_TYPE_MODULE:
        {
            com::sun::star::uno::Reference<
                XTypeDescriptionEnumerationAccess > xTDEA(
                    xNameAccess, UNO_QUERY );

            OSL_ENSURE( xTDEA.is(),
                        "No XTypeDescriptionEnumerationAccess!" );

            return com::sun::star::uno::Reference< XTypeDescription >(
                new ModuleTypeDescriptionImpl( xTDEA, aName ) );
        }

        case RT_TYPE_STRUCT:
            {
                rtl::OUString superTypeName;
                if (aReader.getSuperTypeCount() == 1) {
                    superTypeName = aReader.getSuperTypeName(0).replace(
                        '/', '.');
                }
                return com::sun::star::uno::Reference< XTypeDescription >(
                    new stoc::registry_tdprovider::StructTypeDescription(
                        xNameAccess, aName, superTypeName, rData,
                        aReader.isPublished()));
            }

        case RT_TYPE_ENUM:
            return com::sun::star::uno::Reference< XTypeDescription >(
                new EnumTypeDescriptionImpl( xNameAccess,
                                             aName,
                                             getRTValueAsInt32(
                                                aReader.getFieldValue( 0 ) ),
                                             rData, aReader.isPublished() ) );

        case RT_TYPE_EXCEPTION:
            {
                rtl::OUString superTypeName;
                if (aReader.getSuperTypeCount() == 1) {
                    superTypeName = aReader.getSuperTypeName(0).replace(
                        '/', '.');
                }
                return com::sun::star::uno::Reference< XTypeDescription >(
                    new CompoundTypeDescriptionImpl(
                        xNameAccess, TypeClass_EXCEPTION, aName, superTypeName,
                        rData, aReader.isPublished()));
            }

        case RT_TYPE_TYPEDEF:
            return com::sun::star::uno::Reference< XTypeDescription >(
                new TypedefTypeDescriptionImpl( xNameAccess,
                                                aName,
                                                aReader.getSuperTypeName(0)
                                                    .replace( '/', '.' ),
                                                aReader.isPublished() ) );
        case RT_TYPE_SERVICE:
            return com::sun::star::uno::Reference< XTypeDescription >(
                new ServiceTypeDescriptionImpl(
                    xNameAccess, aName, rData, aReader.isPublished() ) );

        case RT_TYPE_CONSTANTS:
            return com::sun::star::uno::Reference< XTypeDescription >(
                new ConstantsTypeDescriptionImpl(
                    aName, rData, aReader.isPublished() ) );

        case RT_TYPE_SINGLETON:
            return com::sun::star::uno::Reference< XTypeDescription >(
                new SingletonTypeDescriptionImpl( xNameAccess,
                                                  aName,
                                                  aReader.getSuperTypeName(0)
                                                    .replace( '/', '.' ),
                                                  aReader.isPublished() ) );
        case RT_TYPE_INVALID:
        case RT_TYPE_OBJECT:      // deprecated and not used
        case RT_TYPE_UNION:       // deprecated and not used
            OSL_ENSURE( sal_False, "createTypeDescription - Unsupported Type!" );
            break;

        default:
            OSL_ENSURE( sal_False, "createTypeDescription - Unknown Type!" );
            break;
    }

    // Unknown type.

    if ( bReturnEmptyRefForUnknownType )
        return com::sun::star::uno::Reference< XTypeDescription >();

    return com::sun::star::uno::Reference< XTypeDescription >(
                new TypeDescriptionImpl( TypeClass_UNKNOWN, aName ) );
}

}


//##################################################################################################
//##################################################################################################
//##################################################################################################
using namespace stoc_rdbtdp;

static struct ImplementationEntry g_entries[] =
{
    {
        ProviderImpl_create, rdbtdp_getImplementationName,
        rdbtdp_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{
sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return component_writeInfoHelper( pServiceManager, pRegistryKey, g_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}
}
