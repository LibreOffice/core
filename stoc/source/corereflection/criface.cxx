/*************************************************************************
 *
 *  $RCSfile: criface.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ganaya $ $Date: 2000-10-10 05:35:45 $
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

#include <sal/config.h>
#ifdef SAL_UNX
#ifndef MACOSX
#include <alloca.h>
#endif
#endif
#include <malloc.h>
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#include <typelib/typedescription.hxx>
#endif
#ifndef _UNO_DATA_H_
#include <uno/data.h>
#endif

#include "base.hxx"


namespace stoc_corefl
{

//==================================================================================================
class IdlAttributeFieldImpl
    : public IdlMemberImpl
    , public XIdlField
{
public:
    typelib_InterfaceAttributeTypeDescription * getTypeDescr()
        { return (typelib_InterfaceAttributeTypeDescription *)IdlMemberImpl::getTypeDescr(); }

    IdlAttributeFieldImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                           typelib_TypeDescription * pTypeDescr, typelib_TypeDescription * pDeclTypeDescr )
        : IdlMemberImpl( pReflection, rName, pTypeDescr, pDeclTypeDescr )
        {}

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // XIdlMember
    virtual Reference< XIdlClass > SAL_CALL getDeclaringClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    // XIdlField
    virtual Reference< XIdlClass > SAL_CALL getType() throw(::com::sun::star::uno::RuntimeException);
    virtual FieldAccessMode SAL_CALL getAccessMode() throw(::com::sun::star::uno::RuntimeException);
    virtual Any SAL_CALL get( const Any & rObj ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL set( const Any & rObj, const Any & rValue ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
};

// XInterface
//__________________________________________________________________________________________________
Any IdlAttributeFieldImpl::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( ::cppu::queryInterface( rType, static_cast< XIdlField * >( this ) ) );
    return (aRet.hasValue() ? aRet : IdlMemberImpl::queryInterface( rType ));
}
//__________________________________________________________________________________________________
void IdlAttributeFieldImpl::acquire() throw()
{
    IdlMemberImpl::acquire();
}
//__________________________________________________________________________________________________
void IdlAttributeFieldImpl::release() throw()
{
    IdlMemberImpl::release();
}

// XTypeProvider
//__________________________________________________________________________________________________
Sequence< Type > IdlAttributeFieldImpl::getTypes()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OTypeCollection * s_pTypes = 0;
    if (! s_pTypes)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! s_pTypes)
        {
            static OTypeCollection s_aTypes(
                ::getCppuType( (const Reference< XIdlField > *)0 ),
                IdlMemberImpl::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}
//__________________________________________________________________________________________________
Sequence< sal_Int8 > IdlAttributeFieldImpl::getImplementationId()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OImplementationId * s_pId = 0;
    if (! s_pId)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! s_pId)
        {
            static OImplementationId s_aId;
            s_pId = &s_aId;
        }
    }
    return s_pId->getImplementationId();
}

// XIdlMember
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlAttributeFieldImpl::getDeclaringClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _xDeclClass.is())
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _xDeclClass.is())
        {
            typelib_InterfaceTypeDescription * pTD =
                (typelib_InterfaceTypeDescription *)getDeclTypeDescr();
            while (pTD)
            {
                typelib_TypeDescriptionReference ** ppTypeRefs = pTD->ppMembers;
                for ( sal_Int32 nPos = pTD->nMembers; nPos--; )
                {
                    if (td_equals( (typelib_TypeDescription *)getTypeDescr(), ppTypeRefs[nPos] ))
                    {
                        _xDeclClass = getReflection()->forType( (typelib_TypeDescription *)pTD );
                        return _xDeclClass;
                    }
                }
                pTD = pTD->pBaseTypeDescription;
            }
        }
    }
    return _xDeclClass;
}
//__________________________________________________________________________________________________
OUString IdlAttributeFieldImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return IdlMemberImpl::getName();
}

// XIdlField
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlAttributeFieldImpl::getType()
    throw(::com::sun::star::uno::RuntimeException)
{
    return getReflection()->forType( getTypeDescr()->pAttributeTypeRef );
}
//__________________________________________________________________________________________________
FieldAccessMode IdlAttributeFieldImpl::getAccessMode()
    throw(::com::sun::star::uno::RuntimeException)
{
    return (((typelib_InterfaceAttributeTypeDescription *)getTypeDescr())->bReadOnly
            ? FieldAccessMode_READONLY : FieldAccessMode_READWRITE);
}
//__________________________________________________________________________________________________
Any IdlAttributeFieldImpl::get( const Any & rObj )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    uno_Interface * pUnoI = mapToUno( rObj, (typelib_InterfaceTypeDescription *)getDeclTypeDescr() );
    OSL_ENSHURE( pUnoI, "### illegal destination object given!" );
    if (pUnoI)
    {
        TypeDescription aTD( getTypeDescr()->pAttributeTypeRef );
        typelib_TypeDescription * pTD = aTD.get();

        uno_Any aExc;
        uno_Any * pExc = &aExc;
        void * pReturn = alloca( pTD->nSize );

        (*pUnoI->pDispatcher)( pUnoI, (typelib_TypeDescription *)getTypeDescr(), pReturn, 0, &pExc );
        (*pUnoI->release)( pUnoI );

        Any aRet;
        if (pExc)
        {
            // DBO TODO: throw original exception generically
            uno_any_destruct( pExc, 0 );
            throw RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("exception occured during get of attribute!") ),
                *(const Reference< XInterface > *)rObj.getValue() );
        }
        else
        {
            uno_any_destruct( &aRet, cpp_release );
            uno_any_constructAndConvert( &aRet, pReturn, pTD, getUno2Cpp().get() );
            uno_destructData( pReturn, pTD, 0 );
        }
        return aRet;
    }
    throw IllegalArgumentException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("illegal object given!") ),
        (XWeak *)(OWeakObject *)this, 0 );
    return Any(); // dummy
}
//__________________________________________________________________________________________________
void IdlAttributeFieldImpl::set( const Any & rObj, const Any & rValue )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException)
{
    if (getTypeDescr()->bReadOnly)
    {
        throw IllegalAccessException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("cannot set readonly attribute!") ),
            (XWeak *)(OWeakObject *)this );
    }

    uno_Interface * pUnoI = mapToUno( rObj, (typelib_InterfaceTypeDescription *)getDeclTypeDescr() );
    OSL_ENSHURE( pUnoI, "### illegal destination object given!" );
    if (pUnoI)
    {
        TypeDescription aTD( getTypeDescr()->pAttributeTypeRef );
        typelib_TypeDescription * pTD = aTD.get();

        // construct uno value to be set
        void * pArgs[1];
        void * pArg = pArgs[0] = alloca( pTD->nSize );

        sal_Bool bAssign;
        if (pTD->eTypeClass == typelib_TypeClass_ANY)
        {
            uno_copyAndConvertData( pArg, SAL_CONST_CAST( Any *, &rValue ),
                                    pTD, getCpp2Uno().get() );
            bAssign = sal_True;
        }
        else if (typelib_typedescriptionreference_equals( rValue.getValueTypeRef(), pTD->pWeakRef ))
        {
            uno_copyAndConvertData( pArg, SAL_CONST_CAST( void *, rValue.getValue() ),
                                    pTD, getCpp2Uno().get() );
            bAssign = sal_True;
        }
        else if (pTD->eTypeClass == typelib_TypeClass_INTERFACE)
        {
            Reference< XInterface > xObj;
            if (bAssign = extract( rValue, (typelib_InterfaceTypeDescription *)pTD,
                                   xObj, getReflection() ))
            {
                *(void **)pArg = getCpp2Uno().mapInterface(
                    xObj.get(), (typelib_InterfaceTypeDescription *)pTD );
            }
        }
        else
        {
            typelib_TypeDescription * pValueTD = 0;
            TYPELIB_DANGER_GET( &pValueTD, rValue.getValueTypeRef() );
            // construct temp uno val to do proper assignment: todo opt
            void * pTemp = alloca( pValueTD->nSize );
            uno_copyAndConvertData(
                pTemp, (void *)rValue.getValue(), pValueTD, getCpp2Uno().get() );
            uno_constructData(
                pArg, pTD );
            // assignment does simple conversion
            bAssign = uno_assignData(
                pArg, pTD, pTemp, pValueTD, 0, 0, 0 );
            uno_destructData(
                pTemp, pValueTD, 0 );
            TYPELIB_DANGER_RELEASE( pValueTD );
        }

        if (bAssign)
        {
            uno_Any aExc;
            uno_Any * pExc = &aExc;
            (*pUnoI->pDispatcher)( pUnoI, (typelib_TypeDescription *)getTypeDescr(), 0, pArgs, &pExc );
            (*pUnoI->release)( pUnoI );

            uno_destructData( pArg, pTD, 0 );
            if (pExc)
            {
                // DBO TODO: throw original exception generically
                uno_any_destruct( pExc, 0 );
                throw RuntimeException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("exception occured during get of attribute!") ),
                    *(const Reference< XInterface > *)rObj.getValue() );
            }
            return;
        }
        (*pUnoI->release)( pUnoI );

        throw IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal value given!") ),
            *(const Reference< XInterface > *)rObj.getValue(), 1 );
    }
    throw IllegalArgumentException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("illegal destination object given!") ),
        (XWeak *)(OWeakObject *)this, 0 );
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//==================================================================================================
class IdlInterfaceMethodImpl
    : public IdlMemberImpl
    , public XIdlMethod
{
    Sequence< Reference< XIdlClass > > * _pExceptionTypes;
    Sequence< Reference< XIdlClass > > * _pParamTypes;
    Sequence< ParamInfo > *              _pParamInfos;

public:
    typelib_InterfaceMethodTypeDescription * getTypeDescr()
        { return (typelib_InterfaceMethodTypeDescription *)IdlMemberImpl::getTypeDescr(); }

    IdlInterfaceMethodImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                            typelib_TypeDescription * pTypeDescr, typelib_TypeDescription * pDeclTypeDescr )
        : IdlMemberImpl( pReflection, rName, pTypeDescr, pDeclTypeDescr )
        , _pExceptionTypes( 0 )
        , _pParamTypes( 0 )
        , _pParamInfos( 0 )
        {}
    virtual ~IdlInterfaceMethodImpl();

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // XIdlMember
    virtual Reference< XIdlClass > SAL_CALL getDeclaringClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    // XIdlMethod
    virtual Reference< XIdlClass > SAL_CALL getReturnType() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XIdlClass > > SAL_CALL getParameterTypes() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< ParamInfo > SAL_CALL getParameterInfos() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XIdlClass > > SAL_CALL getExceptionTypes() throw(::com::sun::star::uno::RuntimeException);
    virtual MethodMode SAL_CALL getMode() throw(::com::sun::star::uno::RuntimeException);
    virtual Any SAL_CALL invoke( const Any & rObj, Sequence< Any > & rArgs ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException);
};
//__________________________________________________________________________________________________
IdlInterfaceMethodImpl::~IdlInterfaceMethodImpl()
{
    delete _pParamInfos;
    delete _pParamTypes;
    delete _pExceptionTypes;
}

// XInterface
//__________________________________________________________________________________________________
Any IdlInterfaceMethodImpl::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( ::cppu::queryInterface( rType, static_cast< XIdlMethod * >( this ) ) );
    return (aRet.hasValue() ? aRet : IdlMemberImpl::queryInterface( rType ));
}
//__________________________________________________________________________________________________
void IdlInterfaceMethodImpl::acquire() throw()
{
    IdlMemberImpl::acquire();
}
//__________________________________________________________________________________________________
void IdlInterfaceMethodImpl::release() throw()
{
    IdlMemberImpl::release();
}

// XTypeProvider
//__________________________________________________________________________________________________
Sequence< Type > IdlInterfaceMethodImpl::getTypes()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OTypeCollection * s_pTypes = 0;
    if (! s_pTypes)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! s_pTypes)
        {
            static OTypeCollection s_aTypes(
                ::getCppuType( (const Reference< XIdlMethod > *)0 ),
                IdlMemberImpl::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}
//__________________________________________________________________________________________________
Sequence< sal_Int8 > IdlInterfaceMethodImpl::getImplementationId()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OImplementationId * s_pId = 0;
    if (! s_pId)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! s_pId)
        {
            static OImplementationId s_aId;
            s_pId = &s_aId;
        }
    }
    return s_pId->getImplementationId();
}

// XIdlMember
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlInterfaceMethodImpl::getDeclaringClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _xDeclClass.is())
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _xDeclClass.is())
        {
            typelib_InterfaceTypeDescription * pTD =
                (typelib_InterfaceTypeDescription *)getDeclTypeDescr();
            while (pTD)
            {
                typelib_TypeDescriptionReference ** ppTypeRefs = pTD->ppMembers;
                for ( sal_Int32 nPos = pTD->nMembers; nPos--; )
                {
                    if (td_equals( (typelib_TypeDescription *)getTypeDescr(), ppTypeRefs[nPos] ))
                    {
                        _xDeclClass = getReflection()->forType( (typelib_TypeDescription *)pTD );
                        return _xDeclClass;
                    }
                }
                pTD = pTD->pBaseTypeDescription;
            }
        }
    }
    return _xDeclClass;
}
//__________________________________________________________________________________________________
OUString IdlInterfaceMethodImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return IdlMemberImpl::getName();
}

// XIdlMethod
//__________________________________________________________________________________________________
Reference< XIdlClass > SAL_CALL IdlInterfaceMethodImpl::getReturnType()
    throw(::com::sun::star::uno::RuntimeException)
{
    return getReflection()->forType( getTypeDescr()->pReturnTypeRef );
}
//__________________________________________________________________________________________________
Sequence< Reference< XIdlClass > > IdlInterfaceMethodImpl::getExceptionTypes()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pExceptionTypes)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _pExceptionTypes)
        {
            sal_Int32 nExc = getTypeDescr()->nExceptions;
            Sequence< Reference< XIdlClass > > * pTempExceptionTypes =
                new Sequence< Reference< XIdlClass > >( nExc );
            Reference< XIdlClass > * pExceptionTypes = pTempExceptionTypes->getArray();

            typelib_TypeDescriptionReference ** ppExc = getTypeDescr()->ppExceptions;
            IdlReflectionServiceImpl * pRefl = getReflection();

            while (nExc--)
                pExceptionTypes[nExc] = pRefl->forType( ppExc[nExc] );

            _pExceptionTypes = pTempExceptionTypes;
        }
    }
    return *_pExceptionTypes;
}
//__________________________________________________________________________________________________
Sequence< Reference< XIdlClass > > IdlInterfaceMethodImpl::getParameterTypes()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pParamTypes)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _pParamTypes)
        {
            sal_Int32 nParams = getTypeDescr()->nParams;
            Sequence< Reference< XIdlClass > > * pTempParamTypes =
                new Sequence< Reference< XIdlClass > >( nParams );
            Reference< XIdlClass > * pParamTypes = pTempParamTypes->getArray();

            typelib_MethodParameter * pTypelibParams = getTypeDescr()->pParams;
            IdlReflectionServiceImpl * pRefl = getReflection();

            while (nParams--)
                pParamTypes[nParams] = pRefl->forType( pTypelibParams[nParams].pTypeRef );

            _pParamTypes = pTempParamTypes;
        }
    }
    return *_pParamTypes;
}
//__________________________________________________________________________________________________
Sequence< ParamInfo > IdlInterfaceMethodImpl::getParameterInfos()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pParamInfos)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _pParamInfos)
        {
            sal_Int32 nParams = getTypeDescr()->nParams;
            Sequence< ParamInfo > * pTempParamInfos = new Sequence< ParamInfo >( nParams );
            ParamInfo * pParamInfos = pTempParamInfos->getArray();

            typelib_MethodParameter * pTypelibParams = getTypeDescr()->pParams;

            if (_pParamTypes) // use param types
            {
                const Reference< XIdlClass > * pParamTypes = _pParamTypes->getConstArray();

                while (nParams--)
                {
                    const typelib_MethodParameter & rParam = pTypelibParams[nParams];
                    ParamInfo & rInfo = pParamInfos[nParams];
                    rInfo.aName = rParam.pName;
                    if (rParam.bIn)
                        rInfo.aMode = (rParam.bOut ? ParamMode_INOUT : ParamMode_IN);
                    else
                        rInfo.aMode = ParamMode_OUT;
                    rInfo.aType = pParamTypes[nParams];
                }
            }
            else // make also param types sequence if not already initialized
            {
                Sequence< Reference< XIdlClass > > * pTempParamTypes =
                    new Sequence< Reference< XIdlClass > >( nParams );
                Reference< XIdlClass > * pParamTypes = pTempParamTypes->getArray();

                IdlReflectionServiceImpl * pRefl = getReflection();

                while (nParams--)
                {
                    const typelib_MethodParameter & rParam = pTypelibParams[nParams];
                    ParamInfo & rInfo = pParamInfos[nParams];
                    rInfo.aName = rParam.pName;
                    if (rParam.bIn)
                        rInfo.aMode = (rParam.bOut ? ParamMode_INOUT : ParamMode_IN);
                    else
                        rInfo.aMode = ParamMode_OUT;
                    rInfo.aType = pParamTypes[nParams] = pRefl->forType( rParam.pTypeRef );
                }

                _pParamTypes = pTempParamTypes;
            }

            _pParamInfos = pTempParamInfos;
        }
    }
    return *_pParamInfos;
}
//__________________________________________________________________________________________________
MethodMode SAL_CALL IdlInterfaceMethodImpl::getMode()
    throw(::com::sun::star::uno::RuntimeException)
{
    return (getTypeDescr()->bOneWay ? MethodMode_ONEWAY : MethodMode_TWOWAY);
}
//__________________________________________________________________________________________________
Any SAL_CALL IdlInterfaceMethodImpl::invoke( const Any & rObj, Sequence< Any > & rArgs )
    throw(::com::sun::star::lang::IllegalArgumentException,
          ::com::sun::star::reflection::InvocationTargetException,
          ::com::sun::star::uno::RuntimeException)
{
    if (rObj.getValueTypeClass() == TypeClass_INTERFACE)
    {
        // acquire()/ release()
        if (rtl_ustr_ascii_compare( ((typelib_TypeDescription *)getTypeDescr())->pTypeName->buffer,
                                    "com.sun.star.uno.XInterface::acquire" ) == 0)
        {
            (*(const Reference< XInterface > *)rObj.getValue())->acquire();
            return Any();
        }
        else if (rtl_ustr_ascii_compare( ((typelib_TypeDescription *)getTypeDescr())->pTypeName->buffer,
                                         "com.sun.star.uno.XInterface::release" ) == 0)
        {
            (*(const Reference< XInterface > *)rObj.getValue())->release();
            return Any();
        }
    }

    uno_Interface * pUnoI = mapToUno( rObj, (typelib_InterfaceTypeDescription *)getDeclTypeDescr() );
    OSL_ENSHURE( pUnoI, "### illegal destination object given!" );
    if (pUnoI)
    {
        sal_Int32 nParams = getTypeDescr()->nParams;
        if (rArgs.getLength() != nParams)
        {
            (*pUnoI->release)( pUnoI );
            throw IllegalArgumentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("arguments len differ!") ),
                *(const Reference< XInterface > *)rObj.getValue(), 1 );
        }

        Any * pCppArgs = rArgs.getArray();
        typelib_MethodParameter * pParams = getTypeDescr()->pParams;
        typelib_TypeDescription * pReturnType = 0;
        TYPELIB_DANGER_GET( &pReturnType, getTypeDescr()->pReturnTypeRef );

        void * pUnoReturn = alloca( pReturnType->nSize );
        void ** ppUnoArgs = (void **)alloca( sizeof(void *) * nParams *2 );
        typelib_TypeDescription ** ppParamTypes = (typelib_TypeDescription **)(ppUnoArgs + nParams);

        // convert arguments
        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
            ppParamTypes[nPos] = 0;
            TYPELIB_DANGER_GET( ppParamTypes + nPos, pParams[nPos].pTypeRef );
            typelib_TypeDescription * pTD = ppParamTypes[nPos];

            ppUnoArgs[nPos] = alloca( pTD->nSize );
            if (pParams[nPos].bIn)
            {
                sal_Bool bAssign;
                if (typelib_typedescriptionreference_equals( pCppArgs[nPos].getValueTypeRef(), pTD->pWeakRef ))
                {
                    uno_type_copyAndConvertData(
                        ppUnoArgs[nPos], (void *)pCppArgs[nPos].getValue(),
                        pCppArgs[nPos].getValueTypeRef(), getCpp2Uno().get() );
                    bAssign = sal_True;
                }
                else if (pTD->eTypeClass == typelib_TypeClass_ANY)
                {
                    uno_type_any_constructAndConvert(
                        (uno_Any *)ppUnoArgs[nPos], (void *)pCppArgs[nPos].getValue(),
                        pCppArgs[nPos].getValueTypeRef(), getCpp2Uno().get() );
                    bAssign = sal_True;
                }
                else if (pCppArgs[nPos].getValueTypeClass() == TypeClass_INTERFACE &&
                         pTD->eTypeClass == typelib_TypeClass_INTERFACE)
                {
                    Reference< XInterface > xDest;
                    if (bAssign = extract( pCppArgs[nPos], (typelib_InterfaceTypeDescription *)pTD,
                                           xDest, getReflection() ))
                    {
                        *(void **)ppUnoArgs[nPos] = getCpp2Uno().mapInterface(
                            xDest.get(), (typelib_InterfaceTypeDescription *)pTD );
                    }
                }
                else
                {
                    typelib_TypeDescription * pValueTD = 0;
                    TYPELIB_DANGER_GET( &pValueTD, pCppArgs[nPos].getValueTypeRef() );
                    // construct temp uno val to do proper assignment: todo opt
                    void * pTemp = alloca( pValueTD->nSize );
                    uno_copyAndConvertData(
                        pTemp, (void *)pCppArgs[nPos].getValue(), pValueTD, getCpp2Uno().get() );
                    uno_constructData(
                        ppUnoArgs[nPos], pTD );
                    // assignment does simple conversion
                    bAssign = uno_assignData(
                        ppUnoArgs[nPos], pTD, pTemp, pValueTD, 0, 0, 0 );
                    uno_destructData(
                        pTemp, pValueTD, 0 );
                    TYPELIB_DANGER_RELEASE( pValueTD );
                }

                if (! bAssign)
                {
                    IllegalArgumentException aExc(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("cannot coerce argument type during corereflection call!") ),
                        *(const Reference< XInterface > *)rObj.getValue(), nPos );

                    // cleanup
                    while (nPos--)
                    {
                        if (pParams[nPos].bIn)
                            uno_destructData( ppUnoArgs[nPos], ppParamTypes[nPos], 0 );
                        TYPELIB_DANGER_RELEASE( ppParamTypes[nPos] );
                    }
                    TYPELIB_DANGER_RELEASE( pReturnType );
                    (*pUnoI->release)( pUnoI );

                    throw aExc;
                }
            }
        }

        uno_Any aUnoExc;
        uno_Any * pUnoExc = &aUnoExc;

        (*pUnoI->pDispatcher)(
            pUnoI, (typelib_TypeDescription *)getTypeDescr(), pUnoReturn, ppUnoArgs, &pUnoExc );
        (*pUnoI->release)( pUnoI );

        Any aRet;
        if (pUnoExc)
        {
            // cleanup
            while (nParams--)
            {
                if (pParams[nParams].bIn)
                    uno_destructData( ppUnoArgs[nParams], ppParamTypes[nParams], 0 );
                TYPELIB_DANGER_RELEASE( ppParamTypes[nParams] );
            }
            TYPELIB_DANGER_RELEASE( pReturnType );

            InvocationTargetException aExc;
            aExc.Context = *(const Reference< XInterface > *)rObj.getValue();
            aExc.Message = OUString( RTL_CONSTASCII_USTRINGPARAM("exception occured during invocation!") );
            uno_any_destruct( &aExc.TargetException, cpp_release );
            uno_type_copyAndConvertData(
                &aExc.TargetException, pUnoExc, ::getCppuType( (const Any *)0 ).getTypeLibType(), getUno2Cpp().get() );
            uno_any_destruct( pUnoExc, 0 );
            throw aExc;
        }
        else
        {
            // reconvert arguments and cleanup
            while (nParams--)
            {
                if (pParams[nParams].bOut) // write back
                {
                    uno_any_destruct( &pCppArgs[nParams], cpp_release );
                    uno_any_constructAndConvert(
                        &pCppArgs[nParams], ppUnoArgs[nParams], ppParamTypes[nParams], getUno2Cpp().get() );
                }
                uno_destructData( ppUnoArgs[nParams], ppParamTypes[nParams], 0 );
                TYPELIB_DANGER_RELEASE( ppParamTypes[nParams] );
            }
            uno_any_destruct( &aRet, cpp_release );
            uno_any_constructAndConvert( &aRet, pUnoReturn, pReturnType, getUno2Cpp().get() );
            uno_destructData( pUnoReturn, pReturnType, 0 );
            TYPELIB_DANGER_RELEASE( pReturnType );
        }
        return aRet;
    }
    throw IllegalArgumentException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("illegal destination object given!") ),
        (XWeak *)(OWeakObject *)this, 0 );
    return Any(); // dummy
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//__________________________________________________________________________________________________
InterfaceIdlClassImpl::~InterfaceIdlClassImpl()
{
    for ( sal_Int32 nPos = _nMethods + _nAttributes; nPos--; )
        typelib_typedescription_release( _pSortedMemberInit[nPos].second );

    delete [] _pSortedMemberInit;
}

//__________________________________________________________________________________________________
Sequence< Reference< XIdlClass > > InterfaceIdlClassImpl::getSuperclasses()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _xSuperClass.is())
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _xSuperClass.is())
        {
            typelib_InterfaceTypeDescription * pInterfaceTypeDescr = getTypeDescr()->pBaseTypeDescription;
            if (pInterfaceTypeDescr)
                _xSuperClass = getReflection()->forType( (typelib_TypeDescription *)pInterfaceTypeDescr );
        }
    }

    if (_xSuperClass.is())
        return Sequence< Reference< XIdlClass > >( &_xSuperClass, 1 );
    else
        return Sequence< Reference< XIdlClass > >();
}
//__________________________________________________________________________________________________
void InterfaceIdlClassImpl::initMembers()
{
    sal_Int32 nAll = getTypeDescr()->nAllMembers;
    MemberInit * pSortedMemberInit = new MemberInit[nAll];
    typelib_TypeDescriptionReference ** ppAllMembers = getTypeDescr()->ppAllMembers;

    for ( sal_Int32 nPos = 0; nPos < nAll; ++nPos )
    {
        sal_Int32 nIndex;
        if (ppAllMembers[nPos]->eTypeClass == typelib_TypeClass_INTERFACE_METHOD)
        {
            // methods to front
            nIndex = _nMethods;
            ++_nMethods;
        }
        else
        {
            ++_nAttributes;
            nIndex = (nAll - _nAttributes);
            // attributes at the back
        }

        typelib_TypeDescription * pTD = 0;
        typelib_typedescriptionreference_getDescription( &pTD, ppAllMembers[nPos] );
        OSL_ENSHURE( pTD, "### cannot get type description!" );
        pSortedMemberInit[nIndex].first = ((typelib_InterfaceMemberTypeDescription *)pTD)->pMemberName;
        pSortedMemberInit[nIndex].second = pTD;
    }

    _pSortedMemberInit = pSortedMemberInit;
}
//__________________________________________________________________________________________________
sal_Bool InterfaceIdlClassImpl::isAssignableFrom( const Reference< XIdlClass > & xType )
    throw(::com::sun::star::uno::RuntimeException)
{
    if (xType.is() && xType->getTypeClass() == TypeClass_INTERFACE)
    {
        if (equals( xType ))
            return sal_True;
        else
        {
            const Sequence< Reference< XIdlClass > > & rSeq = xType->getSuperclasses();
            if (rSeq.getLength())
            {
                OSL_ENSHURE( rSeq.getLength() == 1, "### unexpected len of super classes!" );
                return isAssignableFrom( rSeq[0] );
            }
        }
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Uik InterfaceIdlClassImpl::getUik()
    throw(::com::sun::star::uno::RuntimeException)
{
    return *(Uik *)&getTypeDescr()->aUik;
}
//__________________________________________________________________________________________________
Sequence< Reference< XIdlMethod > > InterfaceIdlClassImpl::getMethods()
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( getMutexAccess() );
    if (! _pSortedMemberInit)
        initMembers();

    // create methods sequence
    Sequence< Reference< XIdlMethod > > aRet( _nMethods );
    Reference< XIdlMethod > * pRet = aRet.getArray();
    for ( sal_Int32 nPos = _nMethods; nPos--; )
    {

        /*_aName2Method[_pSortedMemberInit[nPos].first] = */pRet[nPos] = new IdlInterfaceMethodImpl(
            getReflection(), _pSortedMemberInit[nPos].first,
            _pSortedMemberInit[nPos].second, IdlClassImpl::getTypeDescr() );
    }
    return aRet;
}
//__________________________________________________________________________________________________
Sequence< Reference< XIdlField > > InterfaceIdlClassImpl::getFields()
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( getMutexAccess() );
    if (! _pSortedMemberInit)
        initMembers();

    // create fields sequence
    Sequence< Reference< XIdlField > > aRet( _nAttributes );
    Reference< XIdlField > * pRet = aRet.getArray();
    for ( sal_Int32 nPos = _nAttributes; nPos--; )
    {
        /*_aName2Field[_pSortedMemberInit[_nMethods+nPos].first] = */pRet[_nAttributes-nPos-1] =
            new IdlAttributeFieldImpl(
                getReflection(), _pSortedMemberInit[_nMethods+nPos].first,
                _pSortedMemberInit[_nMethods+nPos].second, IdlClassImpl::getTypeDescr() );
    }
    return aRet;
}
//__________________________________________________________________________________________________
Reference< XIdlMethod > InterfaceIdlClassImpl::getMethod( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( getMutexAccess() );
    if (! _pSortedMemberInit)
        initMembers();

    Reference< XIdlMethod > xRet;

    // try weak map
    const OUString2Method::const_iterator iFind( _aName2Method.find( rName ) );
    if (iFind != _aName2Method.end())
        xRet = (*iFind).second; // harden ref

    if (! xRet.is())
    {
        for ( sal_Int32 nPos = _nMethods; nPos--; )
        {
            if (_pSortedMemberInit[nPos].first == rName)
            {
                _aName2Method[rName] = xRet = new IdlInterfaceMethodImpl(
                    getReflection(), rName,
                    _pSortedMemberInit[nPos].second, IdlClassImpl::getTypeDescr() );
                break;
            }
        }
    }
    return xRet;
}
//__________________________________________________________________________________________________
Reference< XIdlField > InterfaceIdlClassImpl::getField( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( getMutexAccess() );
    if (! _pSortedMemberInit)
        initMembers();

    Reference< XIdlField > xRet;

    // try weak map
    const OUString2Field::const_iterator iFind( _aName2Field.find( rName ) );
    if (iFind != _aName2Field.end())
        xRet = (*iFind).second; // harden ref

    if (! xRet.is())
    {
        for ( sal_Int32 nPos = _nAttributes; nPos--; )
        {
            if (_pSortedMemberInit[_nMethods+nPos].first == rName)
            {
                _aName2Field[rName] = xRet = new IdlAttributeFieldImpl(
                    getReflection(), rName,
                    _pSortedMemberInit[_nMethods+nPos].second, IdlClassImpl::getTypeDescr() );
                break;
            }
        }
    }
    return xRet;
}
//__________________________________________________________________________________________________
void InterfaceIdlClassImpl::createObject( Any & rObj )
    throw(::com::sun::star::uno::RuntimeException)
{
    // interfaces cannot be constructed
    rObj.clear();
}

}


