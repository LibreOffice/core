/*************************************************************************
 *
 *  $RCSfile: base.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: svesik $ $Date: 2000-11-23 01:39:07 $
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
#ifndef __REGISTRY_REFLREAD_HXX__
#include <registry/reflread.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

#include <vector>

#include <com/sun/star/reflection/XTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceTypeDescription.hpp>
#include <com/sun/star/reflection/XCompoundTypeDescription.hpp>
#include <com/sun/star/reflection/XEnumTypeDescription.hpp>
#include <com/sun/star/reflection/XIndirectTypeDescription.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>


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
    OSL_ENSHURE( sal_False, "### unexpected value type!" );
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
    OSL_ENSHURE( sal_False, "### unexpected RTValue!" );
    return Any();
}

//==================================================================================================
struct MethodInit
{
//      WeakReference< XInterfaceMemberTypeDescription > wxMember;
    //
    OUString    aTypeName;
    OUString    aMemberName;
    OUString    aReturnTypeName;
    sal_uInt16  nMethodIndex;
    sal_Bool    bOneWay;
};
//==================================================================================================
struct AttributeInit
{
//      WeakReference< XInterfaceMemberTypeDescription > wxMember;
    //
    OUString    aTypeName;
    OUString    aMemberName;
    OUString    aMemberTypeName;
    sal_Bool    bReadOnly;
};

//==================================================================================================
class TypeDescriptionImpl : public WeakImplHelper1< XTypeDescription >
{
    TypeClass           _eTypeClass;
    OUString            _aName;

public:
    TypeDescriptionImpl( TypeClass eTypeClass, const OUString & rName )
        : _eTypeClass( eTypeClass )
        , _aName( rName )
        {}

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class InterfaceTypeDescriptionImpl : public WeakImplHelper1< XInterfaceTypeDescription >
{
    Reference< XHierarchicalNameAccess >  _xTDMgr;
    Sequence< sal_Int8 >                  _aBytes;

    OUString                              _aName;
    Uik                                   _aUik;

    OUString                              _aBaseType;
    Mutex                                 _aBaseTypeMutex;
    Reference< XTypeDescription >         _xBaseTD;

    Mutex                                 _aMembersMutex;
    sal_Int32                             _nBaseOffset;
    vector< AttributeInit > *             _pAttributes;
    vector< MethodInit > *                _pMethods;

public:
    InterfaceTypeDescriptionImpl( const Reference< XHierarchicalNameAccess > & xTDMgr,
                                  const OUString & rName, const OUString & rBaseType,
                                  const RTUik & rUik, const Sequence< sal_Int8 > & rBytes );
    virtual ~InterfaceTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XInterfaceTypeDescription
    virtual Uik SAL_CALL getUik() throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XTypeDescription > SAL_CALL getBaseType() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XInterfaceMemberTypeDescription > > SAL_CALL getMembers() throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class CompoundTypeDescriptionImpl : public WeakImplHelper1< XCompoundTypeDescription >
{
    Reference< XHierarchicalNameAccess >  _xTDMgr;
    TypeClass                             _eTypeClass;
    Sequence< sal_Int8 >                  _aBytes;
    OUString                              _aName;

    OUString                              _aBaseType;
    Mutex                                 _aBaseTypeMutex;
    Reference< XTypeDescription >         _xBaseTD;

    Mutex                                 _aMembersMutex;
    Sequence< Reference< XTypeDescription > > * _pMembers;

    Mutex                                 _aMemberNamesMutex;
    Sequence< OUString > *                _pMemberNames;

public:
    CompoundTypeDescriptionImpl( const Reference< XHierarchicalNameAccess > & xTDMgr,
                                 TypeClass eTypeClass,
                                 const OUString & rName, const OUString & rBaseName,
                                 const Sequence< sal_Int8 > & rBytes )
        : _xTDMgr( xTDMgr )
        , _eTypeClass( eTypeClass )
        , _aBytes( rBytes )
        , _aName( rName )
        , _aBaseType( rBaseName )
        , _pMembers( 0 )
        , _pMemberNames( 0 )
        {}
    virtual ~CompoundTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XCompoundTypeDescription
    virtual Reference< XTypeDescription > SAL_CALL getBaseType() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XTypeDescription > > SAL_CALL getMemberTypes() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getMemberNames() throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class EnumTypeDescriptionImpl : public WeakImplHelper1< XEnumTypeDescription >
{
    Reference< XHierarchicalNameAccess >  _xTDMgr;
    Sequence< sal_Int8 >                  _aBytes;

    OUString                              _aName;
    sal_Int32                             _nDefaultValue;

    Mutex                                 _aEnumNamesMutex;
    Sequence< OUString > *                _pEnumNames;
    Mutex                                 _aEnumValuesMutex;
    Sequence< sal_Int32 > *               _pEnumValues;

public:
    EnumTypeDescriptionImpl( const Reference< XHierarchicalNameAccess > & xTDMgr,
                             const OUString & rName, sal_Int32 nDefaultValue,
                             const Sequence< sal_Int8 > & rBytes )
        : _xTDMgr( xTDMgr )
        , _aName( rName )
        , _nDefaultValue( nDefaultValue )
        , _aBytes( rBytes )
        , _pEnumNames( 0 )
        , _pEnumValues( 0 )
        {}
    virtual ~EnumTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XEnumTypeDescription
    virtual sal_Int32 SAL_CALL getDefaultEnumValue() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getEnumNames() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int32 > SAL_CALL getEnumValues() throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class TypedefTypeDescriptionImpl : public WeakImplHelper1< XIndirectTypeDescription >
{
    Reference< XHierarchicalNameAccess >  _xTDMgr;
    OUString                              _aName;

    Mutex                                 _aRefTDMutex;
    OUString                              _aRefName;
    Reference< XTypeDescription >         _xRefTD;

public:
    TypedefTypeDescriptionImpl( const Reference< XHierarchicalNameAccess > & xTDMgr,
                                const OUString & rName, const OUString & rRefName )
        : _xTDMgr( xTDMgr )
        , _aName( rName )
        , _aRefName( rRefName )
        {}

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XIndirectTypeDescription
    virtual Reference< XTypeDescription > SAL_CALL getReferencedType() throw(::com::sun::star::uno::RuntimeException);
};

}


