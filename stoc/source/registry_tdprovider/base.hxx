/*************************************************************************
 *
 *  $RCSfile: base.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 15:03:23 $
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

#ifndef _STOC_RDBTDP_BASE_HXX
#define _STOC_RDBTDP_BASE_HXX

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

#include "registry/refltype.hxx"

#include <list>
#include <memory>
#include <vector>

#include <com/sun/star/reflection/XTypeDescription.hpp>
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include <com/sun/star/reflection/XInterfaceTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceTypeDescription2.hpp>
#include <com/sun/star/reflection/XCompoundTypeDescription.hpp>
#include <com/sun/star/reflection/XConstantTypeDescription.hpp>
#include <com/sun/star/reflection/XConstantsTypeDescription.hpp>
#include <com/sun/star/reflection/XEnumTypeDescription.hpp>
#include <com/sun/star/reflection/XIndirectTypeDescription.hpp>
#include <com/sun/star/reflection/XServiceConstructorDescription.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription2.hpp>
#include <com/sun/star/reflection/XSingletonTypeDescription2.hpp>
#include <com/sun/star/reflection/XModuleTypeDescription.hpp>
#include <com/sun/star/reflection/XPublished.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

using namespace std;
using namespace rtl;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::reflection;

namespace stoc_rdbtdp
{

com::sun::star::uno::Reference< XTypeDescription > resolveTypedefs(
    com::sun::star::uno::Reference< XTypeDescription > const & type);


::osl::Mutex & getMutex();

//--------------------------------------------------------------------------------------------------

typedef ::std::list< ::com::sun::star::uno::Reference<
            ::com::sun::star::registry::XRegistryKey > > RegistryKeyList;

//--------------------------------------------------------------------------------------------------

class RegistryKeyCloser
{
public:
    RegistryKeyCloser( const ::com::sun::star::uno::Reference<
        ::com::sun::star::registry::XRegistryKey > & xKey )
    : m_xKey( xKey ) {}
    ~RegistryKeyCloser()
    { if ( m_xKey.is() ) { try { if ( m_xKey->isValid() ) m_xKey->closeKey(); } catch (...) {} } }

    void reset() { m_xKey.clear(); }
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::registry::XRegistryKey > m_xKey;
};

//--------------------------------------------------------------------------------------------------

// helper to create XTypeDescription instances using typereg::Reader
// (used from Type Provider and Type Description Enumeration implementation)
::com::sun::star::uno::Reference<
    ::com::sun::star::reflection::XTypeDescription >
createTypeDescription(
    const ::com::sun::star::uno::Sequence< sal_Int8 > & rData,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XHierarchicalNameAccess > & xNameAccess,
    bool bReturnEmptyRefForUnknownType );

//--------------------------------------------------------------------------------------------------

extern rtl_StandardModuleCount g_moduleCount;

//--------------------------------------------------------------------------------------------------
inline sal_Int32 getRTValueAsInt32( const RTConstValue & rVal )
{
    switch (rVal.m_type)
    {
    case RT_TYPE_BYTE:
        return rVal.m_value.aByte;
    case RT_TYPE_INT16:
        return rVal.m_value.aShort;
    case RT_TYPE_UINT16:
        return rVal.m_value.aUShort;
    case RT_TYPE_INT32:
        return rVal.m_value.aLong;
    case RT_TYPE_UINT32:
        return rVal.m_value.aULong;
    }
    OSL_ENSURE( sal_False, "### unexpected value type!" );
    return 0;
}
//--------------------------------------------------------------------------------------------------
inline Any getRTValue( const RTConstValue & rVal )
{
    switch (rVal.m_type)
    {
    case RT_TYPE_BOOL:
        return Any( &rVal.m_value.aBool, ::getCppuBooleanType() );
    case RT_TYPE_BYTE:
        return Any( &rVal.m_value.aByte, ::getCppuType( (const sal_Int8 *)0 ) );
    case RT_TYPE_INT16:
        return Any( &rVal.m_value.aShort, ::getCppuType( (const sal_Int16 *)0 ) );
    case RT_TYPE_UINT16:
        return Any( &rVal.m_value.aUShort, ::getCppuType( (const sal_uInt16 *)0 ) );
    case RT_TYPE_INT32:
        return Any( &rVal.m_value.aLong, ::getCppuType( (const sal_Int32 *)0 ) );
    case RT_TYPE_UINT32:
        return Any( &rVal.m_value.aULong, ::getCppuType( (const sal_uInt32 *)0 ) );
    case RT_TYPE_INT64:
        return Any( &rVal.m_value.aHyper, ::getCppuType( (const sal_Int64 *)0 ) );
    case RT_TYPE_UINT64:
        return Any( &rVal.m_value.aUHyper, ::getCppuType( (const sal_uInt64 *)0 ) );
    case RT_TYPE_FLOAT:
        return Any( &rVal.m_value.aFloat, ::getCppuType( (const float *)0 ) );
    case RT_TYPE_DOUBLE:
        return Any( &rVal.m_value.aDouble, ::getCppuType( (const double *)0 ) );
    case RT_TYPE_STRING:
    {
        OUString aStr( rVal.m_value.aString );
        return Any( &aStr, ::getCppuType( (const OUString *)0 ) );
    }
    }
    OSL_ENSURE( sal_False, "### unexpected RTValue!" );
    return Any();
}

//==================================================================================================
class TypeDescriptionImpl : public WeakImplHelper1< XTypeDescription >
{
    TypeClass           _eTypeClass;
    OUString            _aName;

public:
    TypeDescriptionImpl( TypeClass eTypeClass, const OUString & rName )
        : _eTypeClass( eTypeClass )
        , _aName( rName )
        {
            g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        }
    virtual ~TypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class InterfaceTypeDescriptionImpl:
    public WeakImplHelper2< XInterfaceTypeDescription2, XPublished >
{
    com::sun::star::uno::Reference< XHierarchicalNameAccess >  _xTDMgr;
    Sequence< sal_Int8 >                  _aBytes;

    OUString                              _aName;

    Sequence< OUString >                  _aBaseTypes;
    Sequence< com::sun::star::uno::Reference< XTypeDescription > > _xBaseTDs;
    Sequence< OUString >                  _aOptionalBaseTypes;
    Sequence< com::sun::star::uno::Reference< XTypeDescription > >
    _xOptionalBaseTDs;

    sal_Int32                             _nBaseOffset;
    Sequence<
        com::sun::star::uno::Reference< XInterfaceMemberTypeDescription > >
    _members;
    bool _membersInit;

    bool _published;

    void checkInterfaceType(
        com::sun::star::uno::Reference< XTypeDescription > const & type);

public:
    InterfaceTypeDescriptionImpl(
        const com::sun::star::uno::Reference< XHierarchicalNameAccess > &
        xTDMgr,
        const OUString & rName, const Sequence< OUString > & rBaseTypes,
        const Sequence< OUString > & rOptionalBaseTypes,
        const Sequence< sal_Int8 > & rBytes, bool published );
    virtual ~InterfaceTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XInterfaceTypeDescription2
    virtual Uik SAL_CALL getUik() throw(::com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Reference< XTypeDescription > SAL_CALL
    getBaseType() throw(::com::sun::star::uno::RuntimeException);
    virtual
    Sequence<
        com::sun::star::uno::Reference< XInterfaceMemberTypeDescription > >
    SAL_CALL getMembers() throw(::com::sun::star::uno::RuntimeException);

    virtual Sequence< com::sun::star::uno::Reference< XTypeDescription > >
    SAL_CALL getBaseTypes() throw (RuntimeException);

    virtual Sequence< com::sun::star::uno::Reference< XTypeDescription > >
    SAL_CALL getOptionalBaseTypes() throw (RuntimeException);

    virtual sal_Bool SAL_CALL isPublished()
        throw (::com::sun::star::uno::RuntimeException)
    { return _published; }
};

//==================================================================================================
class CompoundTypeDescriptionImpl:
    public WeakImplHelper2< XCompoundTypeDescription, XPublished >
{
    com::sun::star::uno::Reference< XHierarchicalNameAccess >  _xTDMgr;
    TypeClass                             _eTypeClass;
    Sequence< sal_Int8 >                  _aBytes;
    OUString                              _aName;

    OUString                              _aBaseType;
    com::sun::star::uno::Reference< XTypeDescription >        _xBaseTD;

    Sequence< com::sun::star::uno::Reference< XTypeDescription > > * _pMembers;
    Sequence< OUString > *                _pMemberNames;

    bool _published;

public:
    CompoundTypeDescriptionImpl(
        const com::sun::star::uno::Reference< XHierarchicalNameAccess > &
        xTDMgr,
        TypeClass eTypeClass,
        const OUString & rName, const OUString & rBaseName,
        const Sequence< sal_Int8 > & rBytes,
        bool published )
        : _xTDMgr( xTDMgr )
        , _eTypeClass( eTypeClass )
        , _aBytes( rBytes )
        , _aName( rName )
        , _aBaseType( rBaseName )
        , _pMembers( 0 )
        , _pMemberNames( 0 )
        , _published( published )
        {
            g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        }
    virtual ~CompoundTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XCompoundTypeDescription
    virtual com::sun::star::uno::Reference< XTypeDescription > SAL_CALL
    getBaseType() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< com::sun::star::uno::Reference< XTypeDescription > >
    SAL_CALL getMemberTypes() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getMemberNames() throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isPublished()
        throw (::com::sun::star::uno::RuntimeException)
    { return _published; }
};

//==================================================================================================
class EnumTypeDescriptionImpl:
    public WeakImplHelper2< XEnumTypeDescription, XPublished >
{
    com::sun::star::uno::Reference< XHierarchicalNameAccess >  _xTDMgr;
    Sequence< sal_Int8 >                  _aBytes;

    OUString                              _aName;
    sal_Int32                             _nDefaultValue;

    Sequence< OUString > *                _pEnumNames;
    Sequence< sal_Int32 > *               _pEnumValues;

    bool _published;

public:
    EnumTypeDescriptionImpl(
        const com::sun::star::uno::Reference< XHierarchicalNameAccess > &
        xTDMgr,
        const OUString & rName, sal_Int32 nDefaultValue,
        const Sequence< sal_Int8 > & rBytes, bool published )
        : _xTDMgr( xTDMgr )
        , _aName( rName )
        , _nDefaultValue( nDefaultValue )
        , _aBytes( rBytes )
        , _pEnumNames( 0 )
        , _pEnumValues( 0 )
        , _published( published )
        {
            g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        }
    virtual ~EnumTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XEnumTypeDescription
    virtual sal_Int32 SAL_CALL getDefaultEnumValue() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getEnumNames() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int32 > SAL_CALL getEnumValues() throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isPublished()
        throw (::com::sun::star::uno::RuntimeException)
    { return _published; }
};

//==================================================================================================
class TypedefTypeDescriptionImpl:
    public WeakImplHelper2< XIndirectTypeDescription, XPublished >
{
    com::sun::star::uno::Reference< XHierarchicalNameAccess > _xTDMgr;
    OUString                              _aName;

    OUString                              _aRefName;
    com::sun::star::uno::Reference< XTypeDescription > _xRefTD;

    bool _published;

public:
    TypedefTypeDescriptionImpl(
        const com::sun::star::uno::Reference< XHierarchicalNameAccess > &
        xTDMgr,
        const OUString & rName, const OUString & rRefName, bool published )
        : _xTDMgr( xTDMgr )
        , _aName( rName )
        , _aRefName( rRefName )
        , _published( published )
        {
            g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        }
    virtual ~TypedefTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XIndirectTypeDescription
    virtual com::sun::star::uno::Reference< XTypeDescription > SAL_CALL
    getReferencedType() throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isPublished()
        throw (::com::sun::star::uno::RuntimeException)
    { return _published; }
};

//==================================================================================================
class ServiceTypeDescriptionImpl:
    public WeakImplHelper2< XServiceTypeDescription2, XPublished >
{
    OUString                              _aName;
    Sequence< sal_Int8 >                  _aBytes;
    com::sun::star::uno::Reference< XHierarchicalNameAccess > _xTDMgr;
    bool _bInitReferences;

    com::sun::star::uno::Reference< XTypeDescription > _xInterfaceTD;
    std::auto_ptr<
        Sequence<
            com::sun::star::uno::Reference< XServiceConstructorDescription > > >
    _pCtors;
    Sequence< com::sun::star::uno::Reference< XServiceTypeDescription > >
    _aMandatoryServices;
    Sequence< com::sun::star::uno::Reference< XServiceTypeDescription > >
    _aOptionalServices;
    Sequence< com::sun::star::uno::Reference< XInterfaceTypeDescription > >
    _aMandatoryInterfaces;
    Sequence< com::sun::star::uno::Reference< XInterfaceTypeDescription > >
    _aOptionalInterfaces;
    std::auto_ptr<
        Sequence< com::sun::star::uno::Reference< XPropertyTypeDescription > > >
    _pProps;

    bool _published;

public:
    ServiceTypeDescriptionImpl(
        const com::sun::star::uno::Reference< XHierarchicalNameAccess > &
        xTDMgr,
        const OUString & rName, const Sequence< sal_Int8 > & rBytes,
        bool published)
    : _aName( rName ), _aBytes( rBytes ), _xTDMgr( xTDMgr ),
      _bInitReferences( false ), _published( published )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    }
    virtual ~ServiceTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL
    getTypeClass()
        throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL
    getName()
        throw(::com::sun::star::uno::RuntimeException);

    // XServiceTypeDescription
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::reflection::XServiceTypeDescription > > SAL_CALL
    getMandatoryServices()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::reflection::XServiceTypeDescription > > SAL_CALL
    getOptionalServices()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::reflection::XInterfaceTypeDescription > > SAL_CALL
    getMandatoryInterfaces()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::reflection::XInterfaceTypeDescription > > SAL_CALL
    getOptionalInterfaces()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::reflection::XPropertyTypeDescription > > SAL_CALL
    getProperties()
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceTypeDescription2
    virtual sal_Bool SAL_CALL isSingleInterfaceBased()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< XTypeDescription > SAL_CALL
    getInterface() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::reflection::XServiceConstructorDescription > >
    SAL_CALL getConstructors()
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isPublished()
        throw (::com::sun::star::uno::RuntimeException)
    { return _published; }

private:
    void getReferences()
        throw (::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class ModuleTypeDescriptionImpl : public WeakImplHelper1< XModuleTypeDescription >
{
    OUString                                        _aName;
    com::sun::star::uno::Reference< XTypeDescriptionEnumerationAccess > _xTDMgr;

    Sequence< com::sun::star::uno::Reference< XTypeDescription > > * _pMembers;

public:
    ModuleTypeDescriptionImpl(
        const com::sun::star::uno::Reference<
        XTypeDescriptionEnumerationAccess > & xTDMgr,
        const OUString & rName )
    : _aName( rName ), _xTDMgr( xTDMgr ), _pMembers( 0 )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    }
    virtual ~ModuleTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL
    getTypeClass()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual OUString SAL_CALL
    getName()
        throw( ::com::sun::star::uno::RuntimeException );

    // XModuleTypeDescription
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::reflection::XTypeDescription > > SAL_CALL
    getMembers()
        throw ( ::com::sun::star::uno::RuntimeException );
};

//==================================================================================================
class ConstantTypeDescriptionImpl : public WeakImplHelper1< XConstantTypeDescription >
{
    OUString _aName;
    Any      _aValue;

public:
    ConstantTypeDescriptionImpl( const OUString & rName,
                                 const Any & rValue )
    : _aName( rName ), _aValue( rValue )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    }
    virtual ~ConstantTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL
    getTypeClass()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual OUString SAL_CALL
    getName()
        throw( ::com::sun::star::uno::RuntimeException );

    // XConstantTypeDescription
    virtual ::com::sun::star::uno::Any SAL_CALL
    getConstantValue()
        throw ( ::com::sun::star::uno::RuntimeException );
};

//==================================================================================================
class ConstantsTypeDescriptionImpl:
    public WeakImplHelper2< XConstantsTypeDescription, XPublished >
{
    OUString             _aName;
    Sequence< sal_Int8 > _aBytes;
    Sequence< com::sun::star::uno::Reference< XConstantTypeDescription > > *
    _pMembers;

    bool _published;

public:
    ConstantsTypeDescriptionImpl( const OUString & rName,
                                  const Sequence< sal_Int8 > & rBytes,
                                  bool published )
    : _aName( rName ), _aBytes( rBytes), _pMembers( 0 ), _published( published )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    }
    virtual ~ConstantsTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL
    getTypeClass()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual OUString SAL_CALL
    getName()
        throw( ::com::sun::star::uno::RuntimeException );

    // XConstantsTypeDescription
    virtual
    Sequence< com::sun::star::uno::Reference< XConstantTypeDescription > >
    SAL_CALL getConstants() throw ( RuntimeException );

    virtual sal_Bool SAL_CALL isPublished()
        throw (::com::sun::star::uno::RuntimeException)
    { return _published; }
};

//==================================================================================================
class SingletonTypeDescriptionImpl:
    public WeakImplHelper2< XSingletonTypeDescription2, XPublished >
{
    OUString _aName;
    OUString _aBaseName;
    com::sun::star::uno::Reference< XHierarchicalNameAccess > _xTDMgr;
    com::sun::star::uno::Reference< XTypeDescription > _xInterfaceTD;
    com::sun::star::uno::Reference< XServiceTypeDescription > _xServiceTD;

    bool _published;

    void init();

public:
    SingletonTypeDescriptionImpl(
        const com::sun::star::uno::Reference< XHierarchicalNameAccess > &
        xTDMgr,
        const OUString & rName, const OUString & rBaseName, bool published )
    : _aName( rName ), _aBaseName( rBaseName), _xTDMgr( xTDMgr ),
      _published( published )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    }
    virtual ~SingletonTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL
    getTypeClass()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual OUString SAL_CALL
    getName()
        throw( ::com::sun::star::uno::RuntimeException );

    // XSingletonTypeDescription
    virtual com::sun::star::uno::Reference< XServiceTypeDescription > SAL_CALL
    getService() throw ( ::com::sun::star::uno::RuntimeException );

    // XSingletonTypeDescription2
    virtual sal_Bool SAL_CALL isInterfaceBased()
        throw (::com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Reference< XTypeDescription > SAL_CALL
    getInterface() throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isPublished()
        throw (::com::sun::star::uno::RuntimeException)
    { return _published; }
};

}

#endif /* _STOC_RDBTDP_BASE_HXX */
