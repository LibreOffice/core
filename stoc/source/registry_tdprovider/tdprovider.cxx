/*************************************************************************
 *
 *  $RCSfile: tdprovider.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-07 14:48:04 $
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
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <list>

#include "base.hxx"

using namespace com::sun::star::beans;
using namespace com::sun::star::registry;


namespace stoc_rdbtdp
{

#define SERVICENAME "com.sun.star.reflection.TypeDescriptionProvider"
#define IMPLNAME    "com.sun.star.comp.stoc.RegistryTypeDescriptionProvider"

//  typedef list< Reference< XSimpleRegistry > > RegistryList;
typedef list< Reference< XRegistryKey > > RegistryKeyList;

//--------------------------------------------------------------------------------------------------
inline static Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
}

//==================================================================================================
class ProviderImpl
    : public OComponentHelper
    , public XServiceInfo
    , public XHierarchicalNameAccess
{
    Mutex                               _aComponentMutex;
    Reference< XMultiServiceFactory >   _xSMgr;
    Reference< XHierarchicalNameAccess > _xTDMgr;

    Mutex                               _aListsMutex;
//      RegistryList                        _aOpenRegistries;
    RegistryKeyList                     _aBaseKeys;
    RegistryTypeReaderLoader            _aLoader;

public:
    ProviderImpl( const Reference< XMultiServiceFactory > & xMgr );
    virtual ~ProviderImpl();

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // some XComponent part from OComponentHelper
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // XHierarchicalNameAccess
    virtual Any SAL_CALL getByHierarchicalName( const OUString & rName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);
};
//__________________________________________________________________________________________________
ProviderImpl::ProviderImpl( const Reference< XMultiServiceFactory > & xSMgr )
    : OComponentHelper( _aComponentMutex )
    , _xSMgr( xSMgr )
    , _xTDMgr( _xSMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.TypeDescriptionManager") ) ), UNO_QUERY )
{
    OSL_ENSHURE( _xTDMgr.is(), "### cannot get service instance \"com.sun.star.reflection.TypeDescriptionManager\"!" );
}
//__________________________________________________________________________________________________
ProviderImpl::~ProviderImpl()
{
}

// XInterface
//__________________________________________________________________________________________________
Any ProviderImpl::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( cppu::queryInterface(
        rType,
        static_cast< XHierarchicalNameAccess * >( this ),
        static_cast< XServiceInfo * >( this ) ) );

    return (aRet.hasValue() ? aRet : OComponentHelper::queryInterface( rType ));
}
//__________________________________________________________________________________________________
void ProviderImpl::acquire() throw()
{
    OComponentHelper::acquire();
}
//__________________________________________________________________________________________________
void ProviderImpl::release() throw()
{
    OComponentHelper::release();
}

// XTypeProvider
//__________________________________________________________________________________________________
Sequence< Type > ProviderImpl::getTypes()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OTypeCollection * s_pTypes = 0;
    if (! s_pTypes)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pTypes)
        {
            static OTypeCollection s_aTypes(
                ::getCppuType( (const Reference< XServiceInfo > *)0 ),
                ::getCppuType( (const Reference< XHierarchicalNameAccess > *)0 ),
                OComponentHelper::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}
//__________________________________________________________________________________________________
Sequence< sal_Int8 > ProviderImpl::getImplementationId()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OImplementationId * s_pId = 0;
    if (! s_pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pId)
        {
            static OImplementationId s_aId;
            s_pId = &s_aId;
        }
    }
    return s_pId->getImplementationId();
}

// XComponent
//__________________________________________________________________________________________________
void ProviderImpl::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    OComponentHelper::dispose();

    MutexGuard aGuard( _aListsMutex );
    for ( RegistryKeyList::const_iterator iPos( _aBaseKeys.begin() );
          iPos != _aBaseKeys.end(); ++iPos )
    {
        (*iPos)->closeKey();
    }
    _aBaseKeys.clear();
//      for ( RegistryList::const_iterator iRPos( _aOpenRegistries.begin() );
//            iRPos != _aOpenRegistries.end(); ++iRPos )
//      {
//          (*iRPos)->close();
//      }
//      _aOpenRegistries.clear();

    MutexGuard aGuard2( _aComponentMutex );
    _xTDMgr.clear();
    _xSMgr.clear();
}

// XServiceInfo
//__________________________________________________________________________________________________
OUString ProviderImpl::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
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
    return stoc_rdbtdp::getSupportedServiceNames();
}

// XHierarchicalNameAccess
//__________________________________________________________________________________________________
Any SAL_CALL ProviderImpl::getByHierarchicalName( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException, com::sun::star::container::NoSuchElementException)
{
    Any aRet;

    if (_aLoader.isLoaded()) // dll is loaded?
    {
        MutexGuard aGuard( _aListsMutex );
        if (! _aBaseKeys.size())
        {
            // DBO TODO:
            // listen for registry changes

            // reading from service manager registry for all typelib data
            Reference< XPropertySet > xProps( _xSMgr, UNO_QUERY );
            if (xProps.is())
            {
                Any aReg( xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Registry") ) ) );
                if (aReg.hasValue() && aReg.getValueTypeClass() == TypeClass_INTERFACE)
                {
                    Reference< XSimpleRegistry > xSource(
                        *(const Reference< XInterface > *)aReg.getValue(), UNO_QUERY );
                    if (xSource->isValid())
                    {
                        Reference< XRegistryKey > xKey( xSource->getRootKey()->openKey(
                            OUString( RTL_CONSTASCII_USTRINGPARAM("/UCR") ) ) );
                        if (xKey.is())
                            _aBaseKeys.push_back( xKey );
                    }
                }
            }
        }

        // read from registry
        OUString aKey( rName.replace( '.', '/' ) );
        for ( RegistryKeyList::const_iterator iPos( _aBaseKeys.begin() );
              !aRet.hasValue() && iPos != _aBaseKeys.end(); ++iPos )
        {
            Reference< XRegistryKey > xBaseKey( *iPos );
            Reference< XRegistryKey > xKey( xBaseKey->openKey( aKey ) );
            if (xKey.is())
            {
                if (xKey->getValueType() == RegistryValueType_BINARY)
                {
                    Sequence< sal_Int8 > aBytes( xKey->getBinaryValue() );
                    RegistryTypeReader aReader( _aLoader,
                                                (const sal_uInt8 *)aBytes.getConstArray(),
                                                aBytes.getLength(),
                                                sal_False );

                    OUString aName( aReader.getTypeName().replace( '/', '.' ) );

                    switch (aReader.getTypeClass())
                    {
                    case RT_TYPE_INTERFACE:
                    {
                        RTUik aUik;
                        aReader.getUik( aUik );
                        aRet <<= Reference< XTypeDescription >( new InterfaceTypeDescriptionImpl(
                            _xTDMgr, aName,
                            aReader.getSuperTypeName().replace( '/', '.' ),
                            aUik, aBytes ) );
                        break;
                    }
                    case RT_TYPE_EXCEPTION:
                        aRet <<= Reference< XTypeDescription >( new CompoundTypeDescriptionImpl(
                            _xTDMgr, TypeClass_EXCEPTION, aName,
                            aReader.getSuperTypeName().replace( '/', '.' ),
                            aBytes ) );
                        break;
                    case RT_TYPE_STRUCT:
                        aRet <<= Reference< XTypeDescription >( new CompoundTypeDescriptionImpl(
                            _xTDMgr, TypeClass_STRUCT, aName,
                            aReader.getSuperTypeName().replace( '/', '.' ),
                            aBytes ) );
                        break;
                    case RT_TYPE_ENUM:
                        aRet <<= Reference< XTypeDescription >( new EnumTypeDescriptionImpl(
                            _xTDMgr, aName,
                            getRTValueAsInt32( aReader.getFieldConstValue( 0 ) ),
                            aBytes ) );
                        break;
                    case RT_TYPE_TYPEDEF:
                        aRet <<= Reference< XTypeDescription >( new TypedefTypeDescriptionImpl(
                            _xTDMgr, aName,
                            aReader.getSuperTypeName().replace( '/', '.' ) ) );
                        break;

/*                      // these following are in question
                    case RT_TYPE_MODULE:
                        aRet <<= Reference< XTypeDescription >( new TypeDescriptionImpl(
                            TypeClass_MODULE, aName ) );
                        break;
                    case RT_TYPE_SERVICE:
                        aRet <<= Reference< XTypeDescription >( new TypeDescriptionImpl(
                            TypeClass_SERVICE, aName ) );
                        break;
//                      case RT_TYPE_INVALID:
//                      case RT_TYPE_CONSTANTS:
//                      case RT_TYPE_OBJECT:
                    default: // existing registry node
                        aRet <<= Reference< XTypeDescription >( new TypeDescriptionImpl(
                            TypeClass_UNKNOWN, aName ) );
                        break;
*/
                    }
                }
                xKey->closeKey();
            }
            else // might be a constant
            {
                sal_Int32 nIndex = aKey.lastIndexOf( '/' );
                if (nIndex > 0)
                {
                    // open module
                    Reference< XRegistryKey > xKey( xBaseKey->openKey( aKey.copy( 0, nIndex ) ) );
                    if (xKey.is())
                    {
                        if (xKey->getValueType() == RegistryValueType_BINARY)
                        {
                            Sequence< sal_Int8 > aBytes( xKey->getBinaryValue() );
                            RegistryTypeReader aReader(
                                _aLoader, (const sal_uInt8 *)aBytes.getConstArray(),
                                aBytes.getLength(), sal_False );

                            if (aReader.getTypeClass() == RT_TYPE_MODULE ||
                                aReader.getTypeClass() == RT_TYPE_CONSTANTS ||
                                aReader.getTypeClass() == RT_TYPE_ENUM)
                            {
                                OUString aFieldName( aKey.copy( nIndex+1, aKey.getLength() - nIndex -1 ) );
                                sal_Int32 nPos = aReader.getFieldCount();
                                while (nPos--)
                                {
                                    if ( aFieldName.equals( aReader.getFieldName( (sal_uInt16)nPos )) )
                                        break;
                                }
                                if (nPos >= 0)
                                    aRet = getRTValue( aReader.getFieldConstValue( (sal_uInt16)nPos ) );
                            }
                        }
                        xKey->closeKey();
                    }
                }
            }
        }
    }

    if (! aRet.hasValue())
    {
        NoSuchElementException aExc;
        aExc.Message = rName;
        throw aExc;
    }
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

//==================================================================================================
static Reference< XInterface > SAL_CALL ProviderImpl_create( const Reference< XMultiServiceFactory > & xMgr )
    throw(::com::sun::star::uno::Exception)
{
    return Reference< XInterface >( *new ProviderImpl( xMgr ) );
}

}


//##################################################################################################
//##################################################################################################
//##################################################################################################


extern "C"
{
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
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );

            const Sequence< OUString > & rSNL = stoc_rdbtdp::getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createOneInstanceFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
            stoc_rdbtdp::ProviderImpl_create,
            stoc_rdbtdp::getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


