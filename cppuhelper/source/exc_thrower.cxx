/*************************************************************************
 *
 *  $RCSfile: exc_thrower.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2000-10-25 14:44:37 $
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
#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif
#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XInterface.hpp>

using namespace com::sun::star::uno;
using namespace rtl;


namespace cppu
{

//************************************** generated, slightly modified for unicode revision
class XThrower : public ::com::sun::star::uno::XInterface
{
public:

    // Methods
    virtual void SAL_CALL throwException( const ::com::sun::star::uno::Any& rExc ) throw(::com::sun::star::uno::Exception) = 0;
};

static
const ::com::sun::star::uno::Type& getCppuType( const ::com::sun::star::uno::Reference< ::cppu::XThrower >* )
{
    static ::com::sun::star::uno::Type * pType_cppu_XThrower = 0;

    if ( !pType_cppu_XThrower )
    {
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if ( !pType_cppu_XThrower )
    {
        // Start inline typedescription generation
        typelib_InterfaceTypeDescription * pTD = 0;

        const Type & rSuperType = ::getCppuType( ( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)0 );
        typelib_TypeDescriptionReference * pMembers[1] = { 0 };
        OUString aMemberTypeName( RTL_CONSTASCII_USTRINGPARAM("cppu.XThrower::throwException") );
        typelib_typedescriptionreference_new( &pMembers[0],
                                              (typelib_TypeClass)::com::sun::star::uno::TypeClass_INTERFACE_METHOD,
                                              aMemberTypeName.pData );

        OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("cppu.XThrower") );
        typelib_typedescription_newInterface(
            &pTD,
            aTypeName.pData, 0x3313e220, 0xb491, 0xb491, 0x9ede0050, 0x04d7677e,
            rSuperType.getTypeLibType(),
            1,
            pMembers );

        typelib_typedescription_register( (typelib_TypeDescription**)&pTD );
        typelib_typedescriptionreference_release( pMembers[0] );
        typelib_typedescription_release( (typelib_TypeDescription*)pTD );

        static ::com::sun::star::uno::Type aType_cppu_XThrower(
            ::com::sun::star::uno::TypeClass_INTERFACE, aTypeName.pData );
        ::getCppuType( (const ::com::sun::star::uno::Exception*)0 );

        typelib_InterfaceMethodTypeDescription * pMethod = 0;
        {
            typelib_Parameter_Init aParameters[1];
            OUString aParamName( RTL_CONSTASCII_USTRINGPARAM("rExc") );
            aParameters[0].pParamName = aParamName.pData;
            aParameters[0].eTypeClass = (typelib_TypeClass)::com::sun::star::uno::TypeClass_ANY;
            OUString aParamTypeName( RTL_CONSTASCII_USTRINGPARAM("any") );
            aParameters[0].pTypeName = aParamTypeName.pData;
            aParameters[0].bIn = sal_True;
            aParameters[0].bOut = sal_False;
            OUString aExceptionName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.Exception") );
            OUString aVoidName( RTL_CONSTASCII_USTRINGPARAM("void") );
            typelib_typedescription_newInterfaceMethod( &pMethod,
                3, sal_False,
                aMemberTypeName.pData,
                (typelib_TypeClass)::com::sun::star::uno::TypeClass_VOID, aVoidName.pData,
                1, aParameters,
                1, &aExceptionName.pData );
            typelib_typedescription_register( (typelib_TypeDescription**)&pMethod );
        }
        typelib_typedescription_release( (typelib_TypeDescription*)pMethod );
        // End inline typedescription generation
        pType_cppu_XThrower = &aType_cppu_XThrower;
    }
    }
    return *pType_cppu_XThrower;
}
//**************************************


//==================================================================================================
struct ExceptionThrower : public uno_Interface
{
    oslInterlockedCount         nRef;

    inline ExceptionThrower();
};
//--------------------------------------------------------------------------------------------------
void SAL_CALL ExceptionThrower_acquire( uno_Interface * pUnoI )
{
    osl_incrementInterlockedCount( & SAL_STATIC_CAST( ExceptionThrower *, pUnoI )->nRef );
}
//--------------------------------------------------------------------------------------------------
void SAL_CALL ExceptionThrower_release( uno_Interface * pUnoI )
{
    if (! osl_decrementInterlockedCount( & SAL_STATIC_CAST( ExceptionThrower *, pUnoI )->nRef ))
        delete SAL_STATIC_CAST( ExceptionThrower *, pUnoI );
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL ExceptionThrower_dispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberType,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    OSL_ASSERT( pMemberType->eTypeClass == typelib_TypeClass_INTERFACE_METHOD );

    switch (((typelib_InterfaceMemberTypeDescription *)pMemberType)->nPosition)
    {
    case 0: // queryInterace()
    {
        if (::getCppuType( (const Reference< XInterface > *)0 ).equals( *(const Type *)pArgs[0] ))
        {
            const Type & rType = ::getCppuType( (const Reference< XInterface > *)0 );
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, rType.getTypeLibType() );
            uno_any_construct( (uno_Any *)pReturn, &pUnoI, pTD, 0 );
            TYPELIB_DANGER_RELEASE( pTD );
        }
        else if (getCppuType( (const Reference< XThrower > *)0 ).equals( *(const Type *)pArgs[0] ))
        {
            const Type & rType = getCppuType( (const Reference< XThrower > *)0 );
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, rType.getTypeLibType() );
            uno_any_construct( (uno_Any *)pReturn, &pUnoI, pTD, 0 );
            TYPELIB_DANGER_RELEASE( pTD );
        }
        else
        {
            uno_any_construct( (uno_Any *)pReturn, 0, 0, 0 );
        }
        *ppException = 0;
        break;
    }
    case 1: // acquire()
        ExceptionThrower_acquire( pUnoI );
        *ppException = 0;
        break;
    case 2: // release()
        ExceptionThrower_release( pUnoI );
        *ppException = 0;
        break;
    case 3: // throwException()
    {
        OSL_ASSERT( ((uno_Any *)pArgs[0])->pType->eTypeClass == typelib_TypeClass_EXCEPTION );

        uno_type_any_construct(
            *ppException, ((uno_Any *)pArgs[0])->pData, ((uno_Any *)pArgs[0])->pType, 0 );
        break;
    }
    default:
        OSL_ENSHURE( sal_False, "### illegal member called!" );
    }
}
//__________________________________________________________________________________________________
inline ExceptionThrower::ExceptionThrower()
    : nRef( 0 )
{
    uno_Interface::acquire = ExceptionThrower_acquire;
    uno_Interface::release = ExceptionThrower_release;
    uno_Interface::pDispatcher = ExceptionThrower_dispatch;
}

//==================================================================================================
void SAL_CALL throwException( const Any & rExc )
    throw (Exception)
{
    if (rExc.getValueTypeClass() == TypeClass_EXCEPTION)
    {
        OUString aExceptionName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.Exception") );
        typelib_TypeDescription * pComp = 0;
        TYPELIB_DANGER_GET( &pComp, rExc.getValueTypeRef() );
        // must be derived from com.sun.star.uno.Exception
        for ( typelib_CompoundTypeDescription * pTD = (typelib_CompoundTypeDescription *)pComp;
              pTD; pTD = pTD->pBaseTypeDescription )
        {
            if (aExceptionName == rtl::OUString(((typelib_TypeDescription *)pTD)->pTypeName))
            {
                Mapping aUno2Cpp( OUString( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ) );
                OSL_ENSHURE( aUno2Cpp.is(), "### cannot get uno to cpp mapping!" );
                if (aUno2Cpp.is())
                {
                    uno_Interface * pUnoI = new ExceptionThrower();
                    (*pUnoI->acquire)( pUnoI );
                    Reference< XThrower > xThrower;
                    aUno2Cpp.mapInterface(
                        reinterpret_cast< void ** >( &xThrower ),
                        pUnoI, ::cppu::getCppuType( (const Reference< XThrower > *)0 ) );
                    (*pUnoI->release)( pUnoI );

                    if (xThrower.is())
                    {
                        TYPELIB_DANGER_RELEASE( pComp );
                        xThrower->throwException( rExc );
                    }
                }
                break;
            }
        }
        TYPELIB_DANGER_RELEASE( pComp );
    }
    throw RuntimeException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("failed throwing exception generically!") ),
        Reference< XInterface >() );
}

}
