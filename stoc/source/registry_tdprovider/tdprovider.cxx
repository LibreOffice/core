/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <uno/dispatcher.h>
#include <uno/mapping.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/compbase4.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <cppuhelper/weakref.hxx>

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
#include "base.hxx"
#include "rdbtdp_tdenumeration.hxx"
#include "structtypedescription.hxx"

#define SERVICENAME "com.sun.star.reflection.TypeDescriptionProvider"
#define IMPLNAME    "com.sun.star.comp.stoc.RegistryTypeDescriptionProvider"

using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::registry;

extern rtl_StandardModuleCount g_moduleCount;

namespace stoc_bootstrap
{
uno::Sequence< OUString > rdbtdp_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(1);
    seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICENAME));
    return seqNames;
}

OUString rdbtdp_getImplementationName()
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(IMPLNAME));
}
}

namespace stoc_rdbtdp
{
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
    // XHierarchicalNameAccess + XTypeDescriptionEnumerationAccess wrapper
    // first asking the tdmgr instance, then looking up locally
    class TypeDescriptionManagerWrapper
        : public ::cppu::WeakImplHelper2<
            container::XHierarchicalNameAccess,
            reflection::XTypeDescriptionEnumerationAccess>
    {
        com::sun::star::uno::Reference<container::XHierarchicalNameAccess>
        m_xTDMgr;
        com::sun::star::uno::Reference<container::XHierarchicalNameAccess>
        m_xThisProvider;
    public:
        TypeDescriptionManagerWrapper( ProviderImpl * pProvider )
            : m_xTDMgr( pProvider->_xContext->getValueByName(
                            OUString( RTL_CONSTASCII_USTRINGPARAM(
                                          "/singletons/com.sun.star.reflection."
                                          "theTypeDescriptionManager") ) ),
                        UNO_QUERY_THROW ),
              m_xThisProvider( pProvider )
            {}
        // XHierarchicalNameAccess
        virtual Any SAL_CALL getByHierarchicalName( OUString const & name )
            throw (container::NoSuchElementException, RuntimeException);
        virtual sal_Bool SAL_CALL hasByHierarchicalName( OUString const & name )
            throw (RuntimeException);

        // XTypeDescriptionEnumerationAccess
        virtual uno::Reference<
            reflection::XTypeDescriptionEnumeration > SAL_CALL
        createTypeDescriptionEnumeration(
            const OUString& moduleName,
            const uno::Sequence< uno::TypeClass >& types,
            reflection::TypeDescriptionSearchDepth depth )
                throw ( reflection::NoSuchTypeNameException,
                        reflection::InvalidTypeNameException,
                        uno::RuntimeException );
    };
    friend class TypeDescriptionManagerWrapper;

    com::sun::star::uno::Reference< XComponentContext >              _xContext;
    com::sun::star::uno::WeakReference<XHierarchicalNameAccess> _xTDMgr;
    com::sun::star::uno::Reference< XHierarchicalNameAccess > getTDMgr() SAL_THROW(());

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
    Any getByHierarchicalNameImpl( const OUString & rName );

    virtual Any SAL_CALL getByHierarchicalName( const OUString & rName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);

    // XTypeDescriptionEnumerationAccess
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::reflection::XTypeDescriptionEnumeration > SAL_CALL
    createTypeDescriptionEnumeration(
        const OUString& moduleName,
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

//______________________________________________________________________________
Any ProviderImpl::TypeDescriptionManagerWrapper::getByHierarchicalName(
    OUString const & name ) throw (container::NoSuchElementException,
                                   RuntimeException)
{
    try
    {
        // first try tdmgr:
        return m_xTDMgr->getByHierarchicalName( name );
    }
    catch (container::NoSuchElementException &)
    {
        // then lookup locally:
        return m_xThisProvider->getByHierarchicalName( name );
    }
}

//______________________________________________________________________________
sal_Bool ProviderImpl::TypeDescriptionManagerWrapper::hasByHierarchicalName(
    OUString const & name ) throw (RuntimeException)
{
    return m_xTDMgr->hasByHierarchicalName( name ) || m_xThisProvider->hasByHierarchicalName( name );
}

//______________________________________________________________________________
uno::Reference< reflection::XTypeDescriptionEnumeration > SAL_CALL
ProviderImpl::TypeDescriptionManagerWrapper::createTypeDescriptionEnumeration(
        const OUString& moduleName,
        const uno::Sequence< uno::TypeClass >& types,
        reflection::TypeDescriptionSearchDepth depth )
    throw ( reflection::NoSuchTypeNameException,
            reflection::InvalidTypeNameException,
            uno::RuntimeException )
{
    try
    {
        // first try tdmgr:
        uno::Reference< reflection::XTypeDescriptionEnumerationAccess > xTDEA(
            m_xTDMgr, uno::UNO_QUERY_THROW );
        return
            xTDEA->createTypeDescriptionEnumeration( moduleName, types, depth );
    }
    catch (reflection::NoSuchTypeNameException &)
    {
        // then lookup locally:
        uno::Reference< reflection::XTypeDescriptionEnumerationAccess > xTDEA(
            m_xThisProvider, uno::UNO_QUERY_THROW );
        return
            xTDEA->createTypeDescriptionEnumeration( moduleName, types, depth );
    }
}

//__________________________________________________________________________________________________
com::sun::star::uno::Reference< XHierarchicalNameAccess > ProviderImpl::getTDMgr()
    SAL_THROW(())
{
    // harden weak reference:
    com::sun::star::uno::Reference<container::XHierarchicalNameAccess> xTDMgr(
        _xTDMgr );
    if (! xTDMgr.is())
    {
        xTDMgr.set( new TypeDescriptionManagerWrapper(this) );
        {
        MutexGuard guard( _aComponentMutex );
        _xTDMgr = xTDMgr;
        }
    }
    return xTDMgr;
}

//__________________________________________________________________________________________________
void ProviderImpl::disposing()
{
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
                OUString("/UCR") ) );
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
    return stoc_bootstrap::rdbtdp_getImplementationName();
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
    return stoc_bootstrap::rdbtdp_getSupportedServiceNames();
}

// XHierarchicalNameAccess
//__________________________________________________________________________________________________
Any ProviderImpl::getByHierarchicalNameImpl( const OUString & rName )
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
                    com::sun::star::uno::Reference< XRegistryKey > xKey2( xBaseKey->openKey( aKey.copy( 0, nIndex ) ) );
                    if (xKey2.is())
                    {
                        // closes key in it's dtor (which is
                        // called even in case of exceptions).
                        RegistryKeyCloser aCloser( xKey2 );

                        if ( xKey2->isValid() )
                        {
                            if (xKey2->getValueType() == RegistryValueType_BINARY)
                            {
                                Sequence< sal_Int8 > aBytes( xKey2->getBinaryValue() );
                                typereg::Reader aReader(
                                    aBytes.getConstArray(), aBytes.getLength(),
                                    false, TYPEREG_VERSION_1);

                                RTTypeClass tc = aReader.getTypeClass();
                                if (tc == RT_TYPE_MODULE ||
                                    tc == RT_TYPE_CONSTANTS ||
                                    tc == RT_TYPE_ENUM)
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
                                    {
                                        aRet = getRTValue(
                                            aReader.getFieldValue(nPos));
                                        if (tc != RT_TYPE_ENUM)
                                        {
                                            aRet = css::uno::makeAny<
                                                css::uno::Reference<
                                                    css::reflection::XTypeDescription > >(
                                                        new ConstantTypeDescriptionImpl(
                                                            rName, aRet));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        catch ( InvalidRegistryException const & )
        {
            OSL_FAIL( "ProviderImpl::getByHierarchicalName "
                        "- Caught InvalidRegistryException!" );

            // openKey, closeKey, getValueType, getBinaryValue, isValid

            // Don't stop iteration in this case.
        }
        catch ( NoSuchElementException const & )
        {
        }
    }
    return aRet;
}

Any SAL_CALL ProviderImpl::getByHierarchicalName( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException, com::sun::star::container::NoSuchElementException)
{
    Any aRet( getByHierarchicalNameImpl( rName ) );

    if ( !aRet.hasValue() )
        throw NoSuchElementException(
            rName, static_cast< cppu::OWeakObject * >( this  ) );

    return aRet;
}

//__________________________________________________________________________________________________
sal_Bool ProviderImpl::hasByHierarchicalName( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    return getByHierarchicalNameImpl( rName ).hasValue();
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
        TypeDescriptionEnumerationImpl::createInstance( getTDMgr(),
                                                        moduleName,
                                                        types,
                                                        depth,
                                                        _aBaseKeys ).get() );
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
            com::sun::star::uno::Sequence< OUString > aBaseTypeNames(n);
            for (sal_uInt16 i = 0; i < n; ++i) {
                aBaseTypeNames[i] = aReader.getSuperTypeName(i).replace(
                    '/', '.');
            }
            sal_uInt16 n2 = aReader.getReferenceCount();
            com::sun::star::uno::Sequence< OUString >
                aOptionalBaseTypeNames(n2);
            for (sal_uInt16 i = 0; i < n2; ++i) {
                OSL_ASSERT(
                    aReader.getReferenceSort(i) == RT_REF_SUPPORTS
                    && aReader.getReferenceFlags(i) == RT_ACCESS_OPTIONAL);
                aOptionalBaseTypeNames[i] = aReader.getReferenceTypeName(i);
            }
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
                OUString superTypeName;
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
                OUString superTypeName;
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
            OSL_FAIL( "createTypeDescription - Unsupported Type!" );
            break;

        default:
            OSL_FAIL( "createTypeDescription - Unknown Type!" );
            break;
    }

    // Unknown type.

    if ( bReturnEmptyRefForUnknownType )
        return com::sun::star::uno::Reference< XTypeDescription >();

    return com::sun::star::uno::Reference< XTypeDescription >(
                new TypeDescriptionImpl( TypeClass_UNKNOWN, aName ) );
}

}

namespace stoc_bootstrap
{
//==================================================================================================
com::sun::star::uno::Reference< XInterface > SAL_CALL ProviderImpl_create(
    com::sun::star::uno::Reference< XComponentContext > const & xContext )
    throw(::com::sun::star::uno::Exception)
{
    return com::sun::star::uno::Reference< XInterface >( *new stoc_rdbtdp::ProviderImpl( xContext ) );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
