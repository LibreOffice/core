/*************************************************************************
 *
 *  $RCSfile: tdiface.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 15:36:44 $
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
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <com/sun/star/reflection/XInterfaceMemberTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceAttributeTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp>
#include <com/sun/star/reflection/XMethodParameter.hpp>

#include "base.hxx"


namespace stoc_rdbtdp
{

//==================================================================================================
class MethodParameterImpl : public WeakImplHelper1< XMethodParameter >
{
    Reference< XHierarchicalNameAccess > _xTDMgr;

    OUString                        _aName;
    OUString                        _aTypeName;
    Mutex                           _aTypeMutex;
    Reference< XTypeDescription >   _xType;

    sal_Bool                        _bIn;
    sal_Bool                        _bOut;
    sal_Int32                       _nPosition;

public:
    MethodParameterImpl( const Reference< XHierarchicalNameAccess > & xTDMgr,
                         const OUString & rParamName, const OUString & rParamType,
                         sal_Bool bIn, sal_Bool bOut, sal_Int32 nPosition )
        : _xTDMgr( xTDMgr )
        , _aName( rParamName )
        , _aTypeName( rParamType )
        , _bIn( bIn )
        , _bOut( bOut )
        , _nPosition( nPosition )
        {}

    // XMethodParameter
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XTypeDescription > SAL_CALL getType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isIn() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isOut() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException);
};

// XMethodParameter
//__________________________________________________________________________________________________
OUString MethodParameterImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}
//__________________________________________________________________________________________________
Reference<XTypeDescription > MethodParameterImpl::getType()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (!_xType.is() && _aTypeName.getLength())
    {
        MutexGuard aGuard( _aTypeMutex );
        if (!_xType.is() && _aTypeName.getLength())
        {
            try
            {
                if (_xTDMgr->getByHierarchicalName( _aTypeName ) >>= _xType)
                    return _xType;
            }
            catch (NoSuchElementException &)
            {
            }
            // never try again, if no td was found
            _aTypeName = OUString();
        }
    }
    return _xType;
}
//__________________________________________________________________________________________________
sal_Bool MethodParameterImpl::isIn()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _bIn;
}
//__________________________________________________________________________________________________
sal_Bool MethodParameterImpl::isOut()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _bOut;
}
//__________________________________________________________________________________________________
sal_Int32 MethodParameterImpl::getPosition()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _nPosition;
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//==================================================================================================
class InterfaceMethodImpl : public WeakImplHelper1< XInterfaceMethodTypeDescription >
{
    Reference< XHierarchicalNameAccess >  _xTDMgr;

    OUString                              _aTypeName;
    OUString                              _aMemberName;

    OUString                              _aReturnType;
    Mutex                                 _aReturnTypeMutex;
    Reference< XTypeDescription >         _xReturnTD;

    Sequence< sal_Int8 >                  _aBytes;
    sal_uInt16                            _nMethodIndex;
    Mutex                                 _aParamsMutex;
    Sequence< Reference< XMethodParameter > > * _pParams;
    Mutex                                 _aExcMutex;
    Sequence< Reference< XTypeDescription > > * _pExceptions;

    sal_Bool                              _bIsOneWay;
    sal_Int32                             _nPosition;

public:
    InterfaceMethodImpl( const Reference< XHierarchicalNameAccess > & xTDMgr,
                         const OUString & rTypeName,
                         const OUString & rMemberName,
                         const OUString & rReturnType,
                         const Sequence< sal_Int8 > & rBytes,
                         sal_uInt16 nMethodIndex,
                         sal_Bool bIsOneWay,
                         sal_Int32 nPosition )
        : _xTDMgr( xTDMgr )
        , _aTypeName( rTypeName )
        , _aMemberName( rMemberName )
        , _aReturnType( rReturnType )
        , _aBytes( rBytes )
        , _nMethodIndex( nMethodIndex )
        , _pParams( 0 )
        , _pExceptions( 0 )
        , _bIsOneWay( bIsOneWay )
        , _nPosition( nPosition )
        {}
    virtual ~InterfaceMethodImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XInterfaceMemberTypeDescription
    virtual OUString SAL_CALL getMemberName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException);

    // XInterfaceMethodTypeDescription
    virtual Reference< XTypeDescription > SAL_CALL getReturnType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isOneway() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XMethodParameter > > SAL_CALL getParameters() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XTypeDescription > > SAL_CALL getExceptions() throw(::com::sun::star::uno::RuntimeException);
};
//__________________________________________________________________________________________________
InterfaceMethodImpl::~InterfaceMethodImpl()
{
    delete _pParams;
    delete _pExceptions;
}

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass InterfaceMethodImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_INTERFACE_METHOD;
}
//__________________________________________________________________________________________________
OUString InterfaceMethodImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aTypeName;
}

// XInterfaceMemberTypeDescription
//__________________________________________________________________________________________________
OUString InterfaceMethodImpl::getMemberName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aMemberName;
}
//__________________________________________________________________________________________________
sal_Int32 InterfaceMethodImpl::getPosition()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _nPosition;
}

// XInterfaceMethodTypeDescription
//__________________________________________________________________________________________________
Reference<XTypeDescription > InterfaceMethodImpl::getReturnType()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (!_xReturnTD.is() && _aReturnType.getLength())
    {
        MutexGuard aGuard( _aReturnTypeMutex );
        if (!_xReturnTD.is() && _aReturnType.getLength())
        {
            try
            {
                if (_xTDMgr->getByHierarchicalName( _aReturnType ) >>= _xReturnTD)
                    return _xReturnTD;
            }
            catch (NoSuchElementException &)
            {
            }
            // never try again, if no td was found
            _aReturnType = OUString();
        }
    }
    return _xReturnTD;
}
//__________________________________________________________________________________________________
sal_Bool InterfaceMethodImpl::isOneway()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _bIsOneWay;
}
//__________________________________________________________________________________________________
Sequence<Reference<XMethodParameter > > InterfaceMethodImpl::getParameters()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pParams)
    {
        MutexGuard aGuard( _aParamsMutex );
        if (! _pParams)
        {
            RegistryTypeReaderLoader aLoader;
            RegistryTypeReader aReader( aLoader, (const sal_uInt8 *)_aBytes.getConstArray(),
                                        _aBytes.getLength(), sal_False );

            sal_uInt16 nParams = (sal_uInt16)aReader.getMethodParamCount( _nMethodIndex );
            Sequence< Reference< XMethodParameter > > * pTempParams =
                new Sequence< Reference< XMethodParameter > >( nParams );
            Reference< XMethodParameter > * pParams = pTempParams->getArray();

            while (nParams--)
            {
                RTParamMode eMode = aReader.getMethodParamMode( _nMethodIndex, nParams );

                pParams[nParams] = new MethodParameterImpl(
                    _xTDMgr,
                    aReader.getMethodParamName( _nMethodIndex, nParams ),
                    aReader.getMethodParamType( _nMethodIndex, nParams ).replace( '/', '.' ),
                    (eMode == RT_PARAM_IN || eMode == RT_PARAM_INOUT),
                    (eMode == RT_PARAM_OUT || eMode == RT_PARAM_INOUT),
                    nParams );
            }

            _pParams = pTempParams;
        }
    }
    return *_pParams;
}
//__________________________________________________________________________________________________
Sequence<Reference<XTypeDescription > > InterfaceMethodImpl::getExceptions()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pExceptions)
    {
        MutexGuard aGuard( _aExcMutex );
        if (! _pExceptions)
        {
            RegistryTypeReaderLoader aLoader;
            RegistryTypeReader aReader( aLoader, (const sal_uInt8 *)_aBytes.getConstArray(),
                                        _aBytes.getLength(), sal_False );

            sal_uInt16 nExc = (sal_uInt16)aReader.getMethodExcCount( _nMethodIndex );
            Sequence< Reference< XTypeDescription > > * pExceptions =
                new Sequence< Reference< XTypeDescription > >( nExc );
            Reference< XTypeDescription > * pExc = pExceptions->getArray();

            while (nExc--)
            {
                try
                {
                    OUString aMethodExcName( aReader.getMethodExcType( _nMethodIndex, nExc ) );
                    _xTDMgr->getByHierarchicalName( aMethodExcName.replace( '/', '.' ) )
                        >>= pExc[ nExc ];
                }
                catch (NoSuchElementException &)
                {
                }
                OSL_ENSURE( pExc[nExc].is(), "### exception type unknown!" );
            }

            _pExceptions = pExceptions;
        }
    }
    return *_pExceptions;
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//==================================================================================================
class InterfaceAttributeImpl : public WeakImplHelper1< XInterfaceAttributeTypeDescription >
{
    Reference< XHierarchicalNameAccess >  _xTDMgr;

    OUString                              _aTypeName;
    OUString                              _aMemberName;

    OUString                              _aMemberTypeName;
    Mutex                                 _aMemberTypeMutex;
    Reference< XTypeDescription >         _xMemberTD;

    sal_Bool                              _bReadOnly;
    sal_Int32                             _nPosition;

public:
    InterfaceAttributeImpl( const Reference< XHierarchicalNameAccess > & xTDMgr,
                            const OUString & rTypeName,
                            const OUString & rMemberName,
                            const OUString & rMemberTypeName,
                            sal_Bool bReadOnly,
                            sal_Int32 nPosition )
        : _xTDMgr( xTDMgr )
        , _aTypeName( rTypeName )
        , _aMemberName( rMemberName )
        , _aMemberTypeName( rMemberTypeName )
        , _bReadOnly( bReadOnly )
        , _nPosition( nPosition )
        {}

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XInterfaceMemberTypeDescription
    virtual OUString SAL_CALL getMemberName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException);

    // XInterfaceAttributeTypeDescription
    virtual sal_Bool SAL_CALL isReadOnly() throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XTypeDescription > SAL_CALL getType() throw(::com::sun::star::uno::RuntimeException);
};

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass InterfaceAttributeImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_INTERFACE_ATTRIBUTE;
}
//__________________________________________________________________________________________________
OUString InterfaceAttributeImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aTypeName;
}

// XInterfaceMemberTypeDescription
//__________________________________________________________________________________________________
OUString InterfaceAttributeImpl::getMemberName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aMemberName;
}
//__________________________________________________________________________________________________
sal_Int32 InterfaceAttributeImpl::getPosition()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _nPosition;
}

// XInterfaceAttributeTypeDescription
//__________________________________________________________________________________________________
sal_Bool InterfaceAttributeImpl::isReadOnly()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _bReadOnly;
}
//__________________________________________________________________________________________________
Reference<XTypeDescription > InterfaceAttributeImpl::getType()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (!_xMemberTD.is() && _aMemberTypeName.getLength())
    {
        MutexGuard aGuard( _aMemberTypeMutex );
        if (!_xMemberTD.is() && _aMemberTypeName.getLength())
        {
            try
            {
                if (_xTDMgr->getByHierarchicalName( _aMemberTypeName ) >>= _xMemberTD)
                    return _xMemberTD;
            }
            catch (NoSuchElementException &)
            {
            }
            // never try again, if no td was found
            _aMemberTypeName = OUString();
        }
    }
    return _xMemberTD;
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//__________________________________________________________________________________________________
InterfaceTypeDescriptionImpl::InterfaceTypeDescriptionImpl(
    const Reference< XHierarchicalNameAccess > & xTDMgr,
    const OUString & rName, const OUString & rBaseType,
    const RTUik & rUik, const Sequence< sal_Int8 > & rBytes )
    : _xTDMgr( xTDMgr )
    , _aName( rName )
    , _aBaseType( rBaseType )
    , _aBytes( rBytes )
    , _pAttributes( 0 )
    , _pMethods( 0 )
{
    // uik
    _aUik.m_Data1 = rUik.m_Data1;
    _aUik.m_Data2 = rUik.m_Data2;
    _aUik.m_Data3 = rUik.m_Data3;
    _aUik.m_Data4 = rUik.m_Data4;
    _aUik.m_Data5 = rUik.m_Data5;
}
//__________________________________________________________________________________________________
InterfaceTypeDescriptionImpl::~InterfaceTypeDescriptionImpl()
{
    delete _pAttributes;
    delete _pMethods;
}

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass InterfaceTypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_INTERFACE;
}
//__________________________________________________________________________________________________
OUString InterfaceTypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

// XInterfaceTypeDescription
//__________________________________________________________________________________________________
Reference< XTypeDescription > InterfaceTypeDescriptionImpl::getBaseType()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (!_xBaseTD.is() && _aBaseType.getLength())
    {
        MutexGuard aGuard( _aBaseTypeMutex );
        if (!_xBaseTD.is() && _aBaseType.getLength())
        {
            try
            {
                if (_xTDMgr->getByHierarchicalName( _aBaseType ) >>= _xBaseTD)
                    return _xBaseTD;
            }
            catch (NoSuchElementException &)
            {
            }
            // never try again, if no base td was found
            _aBaseType = OUString();
        }
    }
    return _xBaseTD;
}
//__________________________________________________________________________________________________
Uik SAL_CALL InterfaceTypeDescriptionImpl::getUik()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aUik;
}
//__________________________________________________________________________________________________
Sequence< Reference< XInterfaceMemberTypeDescription > > InterfaceTypeDescriptionImpl::getMembers()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pMethods)
    {
        MutexGuard aGuard( _aMembersMutex );
        if (! _pMethods)
        {
            RegistryTypeReaderLoader aLoader;
            RegistryTypeReader aReader( aLoader, (const sal_uInt8 *)_aBytes.getConstArray(),
                                        _aBytes.getLength(), sal_False );
            sal_uInt16 nMethods = (sal_uInt16)aReader.getMethodCount();
            sal_uInt16 nFields  = (sal_uInt16)aReader.getFieldCount();

            vector< AttributeInit > * pAttributes = new vector< AttributeInit >( nFields );
            vector< MethodInit > * pMethods       = new vector< MethodInit >( nMethods );

            OUString aInterfaceName( getName() );

            // base offsets
            _nBaseOffset = 0;

            Reference< XTypeDescription > xBase( getBaseType(), UNO_QUERY );
            while (xBase.is())
            {
                Reference< XInterfaceTypeDescription > xBaseInterface( xBase, UNO_QUERY );
                Sequence< Reference< XInterfaceMemberTypeDescription > > aBaseMembers( xBaseInterface->getMembers() );
                if (aBaseMembers.getLength())
                {
                    _nBaseOffset = aBaseMembers[aBaseMembers.getLength()-1]->getPosition() +1;
                    break;
                }
                xBase = xBaseInterface->getBaseType();
            }

            // all methods
            while (nMethods--)
            {
                OUString aMemberName( aReader.getMethodName( nMethods ) );
                OUStringBuffer aTypeName( aInterfaceName );
                aTypeName.appendAscii( RTL_CONSTASCII_STRINGPARAM("::") );
                aTypeName.append( aMemberName );

                RTMethodMode eMode = aReader.getMethodMode( nMethods );

                MethodInit & rInit = pMethods->operator[]( nMethods );

                rInit.aTypeName    = aTypeName.makeStringAndClear();
                rInit.aMemberName  = aMemberName;
                rInit.aReturnTypeName = aReader.getMethodReturnType( nMethods ).replace( '/', '.' );
                rInit.nMethodIndex = nMethods;
                rInit.bOneWay      = (eMode == RT_MODE_ONEWAY || eMode == RT_MODE_ONEWAY_CONST);
            }

            // all fields
            while (nFields--)
            {
                OUString aMemberName( aReader.getFieldName( nFields ) );
                OUString aMemberType( aReader.getFieldType( nFields ).replace( '/', '.' ) );
                OUStringBuffer aTypeName( aInterfaceName );
                aTypeName.appendAscii( RTL_CONSTASCII_STRINGPARAM("::") );
                aTypeName.append( aMemberName );

                AttributeInit & rInit = pAttributes->operator[]( nFields );

                rInit.aTypeName       = aTypeName.makeStringAndClear();
                rInit.aMemberName     = aMemberName;
                rInit.aMemberTypeName = aMemberType;
                rInit.bReadOnly       = (aReader.getFieldAccess( nFields ) == RT_ACCESS_READONLY);
            }

            _pAttributes = pAttributes;
            _pMethods    = pMethods;
        }
    }

    // collect members
    sal_Int32 nAttributes = _pAttributes->size();
    sal_Int32 nMethods    = _pMethods->size();

    Sequence< Reference< XInterfaceMemberTypeDescription > > aMembers( nAttributes + nMethods );
    Reference< XInterfaceMemberTypeDescription > * pMembers = aMembers.getArray();

    while (nMethods--)
    {
        MethodInit & rInit = _pMethods->operator[]( nMethods );
/*      if (! (pMembers[nAttributes+nMethods] = rInit.wxMember).is())
        {
            rInit.wxMember = */pMembers[nAttributes+nMethods] = new InterfaceMethodImpl(
                _xTDMgr, rInit.aTypeName, rInit.aMemberName,
                rInit.aReturnTypeName, _aBytes, rInit.nMethodIndex,
                rInit.bOneWay, _nBaseOffset+nAttributes+nMethods );
//      }
    }
    while (nAttributes--)
    {
        AttributeInit & rInit = _pAttributes->operator[]( nAttributes );
/*      if (! (pMembers[nAttributes] = rInit.wxMember).is())
        {
            rInit.wxMember = */pMembers[nAttributes] = new InterfaceAttributeImpl(
                _xTDMgr, rInit.aTypeName, rInit.aMemberName, rInit.aMemberTypeName,
                rInit.bReadOnly, _nBaseOffset+nAttributes );
//      }
    }

    return aMembers;
}

}


