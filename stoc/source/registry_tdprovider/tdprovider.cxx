/*************************************************************************
 *
 *  $RCSfile: tdprovider.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dbo $ $Date: 2001-06-25 14:13:13 $
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
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
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

typedef ::std::list< Reference< XRegistryKey > > RegistryKeyList;

struct MutexHolder
{
    Mutex _aComponentMutex;
};
//==================================================================================================
class ProviderImpl
    : public MutexHolder
    , public WeakComponentImplHelper2< XServiceInfo, XHierarchicalNameAccess >
{
    Reference< XComponentContext >              _xContext;
    Reference< XHierarchicalNameAccess >        _xTDMgr;

    RegistryKeyList                             _aBaseKeys;
    RegistryTypeReaderLoader                    _aLoader;

protected:
    virtual void SAL_CALL disposing();

public:
    ProviderImpl( const Reference< XComponentContext > & xContext );
    virtual ~ProviderImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XHierarchicalNameAccess
    virtual Any SAL_CALL getByHierarchicalName( const OUString & rName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);
};
//__________________________________________________________________________________________________
ProviderImpl::ProviderImpl( const Reference< XComponentContext > & xContext )
    : WeakComponentImplHelper2< XServiceInfo, XHierarchicalNameAccess >( _aComponentMutex )
    , _xContext( xContext )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    xContext->getValueByName(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.theTypeDescriptionManager") ) ) >>= _xTDMgr;
    OSL_ENSURE( _xTDMgr.is(), "### cannot get single instance \"TypeDescriptionManager\" from context!" );

    // registries to read from
    Sequence< Reference< XSimpleRegistry > > registries;
    _xContext->getValueByName( OUString( RTL_CONSTASCII_USTRINGPARAM(
        IMPLNAME ".Registries") ) ) >>= registries;

    Reference< XSimpleRegistry > const * pRegistries = registries.getConstArray();
    for ( sal_Int32 nPos = registries.getLength(); nPos--; )
    {
        Reference< XSimpleRegistry > const & xRegistry = pRegistries[ nPos ];
        if (xRegistry.is() && xRegistry->isValid())
        {
            Reference< XRegistryKey > xKey( xRegistry->getRootKey()->openKey(
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UCR") ) ) );
            if (xKey.is() && xKey->isValid())
            {
                _aBaseKeys.push_back( xKey );
            }
        }
    }
}
//__________________________________________________________________________________________________
ProviderImpl::~ProviderImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
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

    if (_aLoader.isLoaded()) // dll is loaded?
    {
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
                    RegistryTypeReader aReader(
                        _aLoader, (const sal_uInt8 *)aBytes.getConstArray(), aBytes.getLength(), sal_False );

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
static Reference< XInterface > SAL_CALL ProviderImpl_create(
    Reference< XComponentContext > const & xContext )
    throw(::com::sun::star::uno::Exception)
{
    return Reference< XInterface >( *new ProviderImpl( xContext ) );
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
