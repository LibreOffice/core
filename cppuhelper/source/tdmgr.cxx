/*************************************************************************
 *
 *  $RCSfile: tdmgr.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 10:54:46 $
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

#ifndef INCLUDED_SAL_ALLOCA_H
#include <sal/alloca.h>
#endif

#include <osl/diagnose.h>
#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

#include <cppuhelper/implbase1.hxx>
#include <typelib/typedescription.h>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XTypeDescription.hpp>
#include <com/sun/star/reflection/XEnumTypeDescription.hpp>
#include <com/sun/star/reflection/XArrayTypeDescription.hpp>
#include <com/sun/star/reflection/XIndirectTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceMemberTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceAttributeTypeDescription.hpp>
#include <com/sun/star/reflection/XMethodParameter.hpp>
#include <com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceTypeDescription.hpp>
#include <com/sun/star/reflection/XCompoundTypeDescription.hpp>
#include <com/sun/star/reflection/XUnionTypeDescription.hpp>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::reflection;


namespace cppu
{

static typelib_TypeDescription * createCTD( const Reference< XTypeDescription > & xType );

//==================================================================================================
inline static sal_Int64 coerceToInt64( const Any & rVal )
{
    switch (rVal.getValueTypeClass())
    {
    case TypeClass_CHAR:
        return *(sal_Unicode *)rVal.getValue();
    case TypeClass_BOOLEAN:
        return (*(sal_Bool *)rVal.getValue() ? 1 : 0);
    case TypeClass_BYTE:
        return *(sal_Int8 *)rVal.getValue();
    case TypeClass_SHORT:
        return *(sal_Int16 *)rVal.getValue();
    case TypeClass_UNSIGNED_SHORT:
        return *(sal_uInt16 *)rVal.getValue();
    case TypeClass_LONG:
        return *(sal_Int32 *)rVal.getValue();
    case TypeClass_UNSIGNED_LONG:
        return *(sal_uInt32 *)rVal.getValue();
    case TypeClass_HYPER:
        return *(sal_Int64 *)rVal.getValue();
    case TypeClass_UNSIGNED_HYPER:
        return *(sal_uInt64 *)rVal.getValue();
    case TypeClass_ENUM:
        return *(int *)rVal.getValue();
    }
    OSL_ENSURE( sal_False, "### cannot read union discriminant: no ordinal type!" );
    return 0; // dummy
}
//==================================================================================================
inline static typelib_TypeDescription * createCTD(
    const Reference< XUnionTypeDescription > & xType )
{
    typelib_TypeDescription * pRet = 0;
    if (xType.is())
    {
        OUString aTypeName( xType->getName() );

        // discriminant type
        Reference< XTypeDescription > xDiscrTD( xType->getDiscriminantType() );
        OUString aDiscrTypeName( xDiscrTD->getName() );
        typelib_TypeDescriptionReference * pDiscrTypeRef = 0;
        typelib_typedescriptionreference_new( &pDiscrTypeRef,
                                              (typelib_TypeClass)xDiscrTD->getTypeClass(),
                                              aDiscrTypeName.pData );
        // default member type
        Reference< XTypeDescription > xDefaultMemberTD( xType->getDefaultMemberType() );
        OUString aDefMemberTypeName( xDefaultMemberTD->getName() );
        typelib_TypeDescriptionReference * pDefMemberTypeRef = 0;
        typelib_typedescriptionreference_new( &pDefMemberTypeRef,
                                              (typelib_TypeClass)xDefaultMemberTD->getTypeClass(),
                                              aDefMemberTypeName.pData );
        // init array
        Sequence< Any > aDiscriminants( xType->getDiscriminants() );
        Sequence< Reference< XTypeDescription > > aMemberTypes( xType->getMemberTypes() );
        Sequence< OUString > aMemberNames( xType->getMemberNames() );
        sal_Int32 nMembers = aDiscriminants.getLength();
        OSL_ASSERT( nMembers == aMemberNames.getLength() && nMembers == aMemberTypes.getLength() );

        const Any * pDiscriminants                          = aDiscriminants.getConstArray();
        const Reference< XTypeDescription > * pMemberTypes  = aMemberTypes.getConstArray();
        const OUString * pMemberNames                       = aMemberNames.getConstArray();

        typelib_Union_Init * pMembers = (typelib_Union_Init *)alloca( nMembers * sizeof(typelib_Union_Init) );

        sal_Int32 nPos;
        for ( nPos = nMembers; nPos--; )
        {
            typelib_Union_Init & rEntry = pMembers[nPos];
            // member discriminant
            rEntry.nDiscriminant = coerceToInt64( pDiscriminants[nPos] );
            // member type
            OUString aMemberTypeName( pMemberTypes[nPos]->getName() );
            rEntry.pTypeRef = 0;
            typelib_typedescriptionreference_new( &rEntry.pTypeRef,
                                                  (typelib_TypeClass)pMemberTypes[nPos]->getTypeClass(),
                                                  aMemberTypeName.pData );
            // member name
            rEntry.pMemberName = pMemberNames[nPos].pData;
        }

        typelib_typedescription_newUnion( &pRet, aTypeName.pData,
                                          pDiscrTypeRef,
                                          coerceToInt64( xType->getDefaultDiscriminant() ),
                                          pDefMemberTypeRef,
                                          nMembers, pMembers );

        for ( nPos = nMembers; nPos--; )
        {
            typelib_typedescriptionreference_release( pMembers[nPos].pTypeRef );
        }

        typelib_typedescriptionreference_release( pDiscrTypeRef );
        typelib_typedescriptionreference_release( pDefMemberTypeRef );
    }
    return pRet;
}
//==================================================================================================
inline static typelib_TypeDescription * createCTD(
    const Reference< XCompoundTypeDescription > & xType )
{
    typelib_TypeDescription * pRet = 0;
    if (xType.is())
    {
        typelib_TypeDescription * pBaseType = createCTD(
            Reference< XCompoundTypeDescription >::query( xType->getBaseType() ) );
        if (pBaseType)
            typelib_typedescription_register( &pBaseType );

        // construct member init array
        const Sequence<Reference< XTypeDescription > > & rMemberTypes = xType->getMemberTypes();
        const Sequence< OUString > & rMemberNames                     = xType->getMemberNames();

        const Reference< XTypeDescription > * pMemberTypes = rMemberTypes.getConstArray();
        const OUString * pMemberNames                      = rMemberNames.getConstArray();

        sal_Int32 nMembers = rMemberTypes.getLength();
        OSL_ENSURE( nMembers == rMemberNames.getLength(), "### lens differ!" );

        OUString aTypeName( xType->getName() );

        typelib_CompoundMember_Init * pMemberInits = (typelib_CompoundMember_Init *)alloca(
            sizeof(typelib_CompoundMember_Init) * nMembers );

        sal_Int32 nPos;
        for ( nPos = nMembers; nPos--; )
        {
            typelib_CompoundMember_Init & rInit = pMemberInits[nPos];
            rInit.eTypeClass = (typelib_TypeClass)pMemberTypes[nPos]->getTypeClass();

            OUString aMemberTypeName( pMemberTypes[nPos]->getName() );
            rtl_uString_acquire( rInit.pTypeName = aMemberTypeName.pData );

            // string is held by rMemberNames
            rInit.pMemberName = pMemberNames[nPos].pData;
        }

        typelib_typedescription_new(
            &pRet,
            (typelib_TypeClass)xType->getTypeClass(),
            aTypeName.pData,
            (pBaseType ? pBaseType->pWeakRef : 0),
            nMembers, pMemberInits );

        // cleanup
        for ( nPos = nMembers; nPos--; )
        {
            rtl_uString_release( pMemberInits[nPos].pTypeName );
        }
        if (pBaseType)
            typelib_typedescription_release( pBaseType );
    }
    return pRet;
}
//==================================================================================================
inline static typelib_TypeDescription * createCTD(
    const Reference< XInterfaceAttributeTypeDescription > & xAttribute )
{
    typelib_TypeDescription * pRet = 0;
    if (xAttribute.is())
    {
        OUString aMemberName( xAttribute->getName() );
        Reference< XTypeDescription > xType( xAttribute->getType() );
        OUString aMemberTypeName( xType->getName() );

        typelib_typedescription_newInterfaceAttribute(
            (typelib_InterfaceAttributeTypeDescription **)&pRet,
            xAttribute->getPosition(),
            aMemberName.pData, // name
            (typelib_TypeClass)xType->getTypeClass(),
            aMemberTypeName.pData, // type name
            xAttribute->isReadOnly() );
    }
    return pRet;
}
//==================================================================================================
static typelib_TypeDescription * createCTD(
    const Reference< XInterfaceMethodTypeDescription > & xMethod )
{
    typelib_TypeDescription * pRet = 0;
    if (xMethod.is())
    {
        Reference< XTypeDescription > xReturnType( xMethod->getReturnType() );

        // init all params
        const Sequence<Reference< XMethodParameter > > & rParams = xMethod->getParameters();
        const Reference< XMethodParameter > * pParams            = rParams.getConstArray();
        sal_Int32 nParams = rParams.getLength();

        typelib_Parameter_Init * pParamInit = (typelib_Parameter_Init *)alloca(
            sizeof(typelib_Parameter_Init) * nParams );

        sal_Int32 nPos;
        for ( nPos = nParams; nPos--; )
        {
            const Reference< XMethodParameter > & xParam = pParams[nPos];
            const Reference< XTypeDescription > & xType  = xParam->getType();
            typelib_Parameter_Init & rInit = pParamInit[xParam->getPosition()];

            rInit.eTypeClass = (typelib_TypeClass)xType->getTypeClass();
            OUString aParamTypeName( xType->getName() );
            rtl_uString_acquire( rInit.pTypeName = aParamTypeName.pData );
            OUString aParamName( xParam->getName() );
            rtl_uString_acquire( rInit.pParamName = aParamName.pData );
            rInit.bIn  = xParam->isIn();
            rInit.bOut = xParam->isOut();
        }

        // init all exception strings
        const Sequence<Reference< XTypeDescription > > & rExceptions = xMethod->getExceptions();
        const Reference< XTypeDescription > * pExceptions = rExceptions.getConstArray();
        sal_Int32 nExceptions = rExceptions.getLength();
        rtl_uString ** ppExceptionNames = (rtl_uString **)alloca(
            sizeof(rtl_uString *) * nExceptions );

        for ( nPos = nExceptions; nPos--; )
        {
            OUString aExceptionTypeName( pExceptions[nPos]->getName() );
            rtl_uString_acquire( ppExceptionNames[nPos] = aExceptionTypeName.pData );
        }

        OUString aTypeName( xMethod->getName() );
        OUString aReturnTypeName( xReturnType->getName() );

        typelib_typedescription_newInterfaceMethod(
            (typelib_InterfaceMethodTypeDescription **)&pRet,
            xMethod->getPosition(),
            xMethod->isOneway(),
            aTypeName.pData,
            (typelib_TypeClass)xReturnType->getTypeClass(),
            aReturnTypeName.pData,
            nParams, pParamInit,
            nExceptions, ppExceptionNames );

        for ( nPos = nParams; nPos--; )
        {
            rtl_uString_release( pParamInit[nPos].pTypeName );
            rtl_uString_release( pParamInit[nPos].pParamName );
        }
        for ( nPos = nExceptions; nPos--; )
        {
            rtl_uString_release( ppExceptionNames[nPos] );
        }
    }
    return pRet;
}
//==================================================================================================
inline static typelib_TypeDescription * createCTD(
    const Reference< XInterfaceTypeDescription > & xType )
{
    typelib_TypeDescription * pRet = 0;
    if (xType.is())
    {
        typelib_TypeDescription * pBaseType = createCTD(
            Reference< XInterfaceTypeDescription >::query( xType->getBaseType() ) );
        if (pBaseType)
            typelib_typedescription_register( &pBaseType );

        // construct all member refs
        const Sequence<Reference< XInterfaceMemberTypeDescription > > & rMembers = xType->getMembers();
        sal_Int32 nMembers = rMembers.getLength();

        typelib_TypeDescriptionReference ** ppMemberRefs = (typelib_TypeDescriptionReference **)alloca(
            sizeof(typelib_TypeDescriptionReference *) * nMembers );

        const Reference< XInterfaceMemberTypeDescription > * pMembers = rMembers.getConstArray();

        OUString aTypeName( xType->getName() );

        sal_Int32 nPos;
        for ( nPos = nMembers; nPos--; )
        {
            OUString aMemberTypeName( pMembers[nPos]->getName() );
            ppMemberRefs[nPos] = 0;
            typelib_typedescriptionreference_new(
                ppMemberRefs + nPos,
                (typelib_TypeClass)pMembers[nPos]->getTypeClass(),
                aMemberTypeName.pData );
        }

        Uik uik = xType->getUik();

        typelib_typedescription_newInterface(
            (typelib_InterfaceTypeDescription **)&pRet,
            aTypeName.pData,
            uik.m_Data1, uik.m_Data2, uik.m_Data3, uik.m_Data4, uik.m_Data5,
            (pBaseType ? pBaseType->pWeakRef : 0),
            nMembers, ppMemberRefs );

        // cleanup refs and base type
        if (pBaseType)
            typelib_typedescription_release( pBaseType );

        for ( nPos = nMembers; nPos--; )
        {
            typelib_typedescriptionreference_release( ppMemberRefs[nPos] );
        }
    }
    return pRet;
}
//==================================================================================================
inline static typelib_TypeDescription * createCTD( const Reference< XEnumTypeDescription > & xType )
{
    typelib_TypeDescription * pRet = 0;
    if (xType.is())
    {
        OUString aTypeName( xType->getName() );
        Sequence< OUString > aNames( xType->getEnumNames() );
        OSL_ASSERT( sizeof(OUString) == sizeof(rtl_uString *) ); // !!!
        Sequence< sal_Int32 > aValues( xType->getEnumValues() );

        typelib_typedescription_newEnum(
            &pRet, aTypeName.pData, xType->getDefaultEnumValue(),
            aNames.getLength(),
            (rtl_uString **)aNames.getConstArray(),
            const_cast< sal_Int32 * >( aValues.getConstArray() ) );
    }
    return pRet;
}
//==================================================================================================
inline static typelib_TypeDescription * createCTD(
    const Reference< XArrayTypeDescription > & xType )
{
    typelib_TypeDescription * pRet = 0;
    if (xType.is())
    {
        typelib_TypeDescription * pElementType = createCTD( xType->getType() );
        typelib_typedescription_register( &pElementType );

        Sequence< sal_Int32 > dims( xType->getDimensions() );
        OSL_ASSERT( dims.getLength() == xType->getNumberOfDimensions() );

        OUString aTypeName( xType->getName() );

        typelib_typedescription_newArray(
            &pRet, pElementType->pWeakRef,
            dims.getLength(), const_cast< sal_Int32 * >( dims.getConstArray() ) );

        // cleanup
        if (pElementType)
            typelib_typedescription_release( pElementType );
    }
    return pRet;
}
//==================================================================================================
inline static typelib_TypeDescription * createCTD(
    const Reference< XIndirectTypeDescription > & xType )
{
    typelib_TypeDescription * pRet = 0;
    if (xType.is())
    {
        typelib_TypeDescription * pRefType = createCTD( xType->getReferencedType() );
        typelib_typedescription_register( &pRefType );

        OUString aTypeName( xType->getName() );

        typelib_typedescription_new(
            &pRet,
            (typelib_TypeClass)xType->getTypeClass(),
            aTypeName.pData,
            pRefType->pWeakRef,
            0, 0 );

        // cleanup
        if (pRefType)
            typelib_typedescription_release( pRefType );
    }
    return pRet;
}

//==================================================================================================
static typelib_TypeDescription * createCTD( const Reference< XTypeDescription > & xType )
{
    typelib_TypeDescription * pRet = 0;

    if (xType.is())
    {
        switch (xType->getTypeClass())
        {
            // built in types
        case TypeClass_VOID:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("void") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_VOID, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_CHAR:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("char") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_CHAR, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_BOOLEAN:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("boolean") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_BOOLEAN, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_BYTE:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("byte") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_BYTE, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_SHORT:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("short") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_SHORT, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_UNSIGNED_SHORT:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("unsigned short") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_UNSIGNED_SHORT, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_LONG:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("long") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_LONG, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_UNSIGNED_LONG:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("unsigned long") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_UNSIGNED_LONG, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_HYPER:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("hyper") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_HYPER, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_UNSIGNED_HYPER:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("unsigned hyper") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_UNSIGNED_HYPER, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_FLOAT:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("float") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_FLOAT, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_DOUBLE:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("double") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_DOUBLE, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_STRING:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("string") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_STRING, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_TYPE:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("type") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_TYPE, aTypeName.pData, 0, 0, 0 );
            break;
        }
        case TypeClass_ANY:
        {
            OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("any") );
            typelib_typedescription_new( &pRet, typelib_TypeClass_ANY, aTypeName.pData, 0, 0, 0 );
            break;
        }

        case TypeClass_UNION:
            pRet = createCTD( Reference< XUnionTypeDescription >::query( xType ) );
            break;
        case TypeClass_EXCEPTION:
        case TypeClass_STRUCT:
            pRet = createCTD( Reference< XCompoundTypeDescription >::query( xType ) );
            break;
        case TypeClass_ENUM:
            pRet = createCTD( Reference< XEnumTypeDescription >::query( xType ) );
            break;
        case TypeClass_TYPEDEF:
        {
            Reference< XIndirectTypeDescription > xTypedef( xType, UNO_QUERY );
            if (xTypedef.is())
                pRet = createCTD( xTypedef->getReferencedType() );
            break;
        }
        case TypeClass_ARRAY:
            pRet = createCTD( Reference< XArrayTypeDescription >::query( xType ) );
            break;
        case TypeClass_SEQUENCE:
            pRet = createCTD( Reference< XIndirectTypeDescription >::query( xType ) );
            break;
        case TypeClass_INTERFACE:
            pRet = createCTD( Reference< XInterfaceTypeDescription >::query( xType ) );
            break;
        case TypeClass_INTERFACE_METHOD:
            pRet = createCTD( Reference< XInterfaceMethodTypeDescription >::query( xType ) );
            break;
        case TypeClass_INTERFACE_ATTRIBUTE:
            pRet = createCTD( Reference< XInterfaceAttributeTypeDescription >::query( xType ) );
            break;
        }
    }

    return pRet;
}


//==================================================================================================
extern "C"
{
static void SAL_CALL typelib_callback(
    void * pContext, typelib_TypeDescription ** ppRet, rtl_uString * pTypeName )
{
    OSL_ENSURE( pContext && ppRet && pTypeName, "### null ptr!" );
    if (ppRet)
    {
        if (*ppRet)
        {
            ::typelib_typedescription_release( *ppRet );
            *ppRet = 0;
        }
        if (pContext && pTypeName)
        {
            try
            {
                OUString const & rTypeName = OUString::unacquired( &pTypeName );
                Reference< XTypeDescription > xTD;
                if (reinterpret_cast< container::XHierarchicalNameAccess * >(
                        pContext )->getByHierarchicalName(
                            rTypeName ) >>= xTD)
                {
                    *ppRet = createCTD( xTD );
                }
            }
            catch (Exception & exc)
            {
                OSL_ENSURE(
                    0, OUStringToOString(
                        exc.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            }
        }
#if OSL_DEBUG_LEVEL > 0
        if (0 == *ppRet)
        {
            OString msg(
                OString("typelibrary type not accessable: ") +
                OUStringToOString( pTypeName, RTL_TEXTENCODING_ASCII_US ) );
            OSL_ENSURE( 0, msg.getStr() );
        }
#endif
    }
}
}

//==================================================================================================
class EventListenerImpl
    : public WeakImplHelper1< lang::XEventListener >
{
    Reference< container::XHierarchicalNameAccess > m_xTDMgr;

public:
    inline EventListenerImpl(
        Reference< container::XHierarchicalNameAccess > const & xTDMgr )
        SAL_THROW( () )
        : m_xTDMgr( xTDMgr )
        {}

    // XEventListener
    virtual void SAL_CALL disposing( lang::EventObject const & rEvt )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
void EventListenerImpl::disposing( lang::EventObject const & rEvt )
    throw (RuntimeException)
{
    OSL_ASSERT( rEvt.Source == m_xTDMgr );
    // deregister of c typelib callback
    ::typelib_typedescription_revokeCallback( m_xTDMgr.get(), typelib_callback );
}

//==================================================================================================
sal_Bool SAL_CALL installTypeDescriptionManager(
    Reference< container::XHierarchicalNameAccess > const & xTDMgr )
    SAL_THROW( () )
{
    Reference< lang::XComponent > xComp( xTDMgr, UNO_QUERY );
    if (xComp.is())
    {
        xComp->addEventListener( new EventListenerImpl( xTDMgr ) );
        // register c typelib callback
        ::typelib_typedescription_registerCallback( xTDMgr.get(), typelib_callback );
        return sal_True;
    }
    return sal_False;
}

} // end namespace cppu

