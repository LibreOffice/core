/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: oinstanceprovider.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __SFX_OINSTANCEPROVIDER_HXX_
#define __SFX_OINSTANCEPROVIDER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include "omutexmember.hxx"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/bridge/XInstanceProvider.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

#define XINSTANCEPROVIDER                   ::com::sun::star::bridge::XInstanceProvider
#define OWEAKOBJECT                         ::cppu::OWeakObject
#define NOSUCHELEMENTEXCEPTION              ::com::sun::star::container::NoSuchElementException
#define XMULTISERVICEFACTORY                ::com::sun::star::lang::XMultiServiceFactory
#define REFERENCE                           ::com::sun::star::uno::Reference
#define RUNTIMEEXCEPTION                    ::com::sun::star::uno::RuntimeException
#define ANY                                 ::com::sun::star::uno::Any
#define UNOTYPE                             ::com::sun::star::uno::Type
#define XINTERFACE                          ::com::sun::star::uno::XInterface
#define OUSTRING                            ::rtl::OUString

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          create factories corresponding to special name
    @descr          If you need a factory for creation of other factories (zB. neccessary for bridges) you can use
                    these implementation. You will give us a factory name and we will give you these factory (if it exist).

    @implements     XInterface
                    XInstanceProvider
                    [ XDebugging if ENABLE_SERVICEDEBUG is defined! ]
    @base           OMutexMember
                    OWeakObject

    @devstatus      deprecated
*//*-*************************************************************************************************************/

class OInstanceProvider :   public XINSTANCEPROVIDER            ,
                            public OMutexMember                 ,   // Order of baseclasses is important for right initialization!
                            public OWEAKOBJECT
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard ctor
            @descr      These initialize a new instance of ths class with needed informations for work.

            @seealso    using at owner

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         OInstanceProvider( const REFERENCE< XMULTISERVICEFACTORY >& xFactory );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface
        //---------------------------------------------------------------------------------------------------------

        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();
        virtual ANY SAL_CALL queryInterface( const UNOTYPE& aType ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //  XInstanceProvider
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      return reference to a factory for specified instance
            @descr      These object is a selector for different factories. You will give me a factory name -
                        and I will give you a reference to the right factory object (if any exist!).

            @seealso    -

            @param      "sInstanceName", name of searched factory.
            @return     A reference to corresponding factory.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XINTERFACE > SAL_CALL getInstance( const OUSTRING& sInstanceName ) throw(    NOSUCHELEMENTEXCEPTION  ,
                                                                                                        RUNTIMEEXCEPTION        );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

        /*-****************************************************************************************************//**
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.
                        This method is protected, because its not allowed to use an instance of this class as a member!
                        You MUST use a pointer.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~OInstanceProvider();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      debug-method to check incoming parameter of some other mehods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @seealso    ASSERTs in implementation!

            @param      references to checking variables
            @return     sal_False on invalid parameter<BR>
                        sal_True  otherway

            @onerror    -
        *//*-*****************************************************************************************************/

    #if OSL_DEBUG_LEVEL > 0

    private:

        sal_Bool impldbg_checkParameter_getInstance( const OUSTRING& sInstanceName );

    #endif  // #ifdef OSL_DEBUG_LEVEL

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        REFERENCE< XMULTISERVICEFACTORY >       m_xFactory      ;

};      //  class OInstanceProvider

#endif  //  #ifndef __SFX_OINSTANCEPROVIDER_HXX_
