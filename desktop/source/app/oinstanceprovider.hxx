/*************************************************************************
 *
 *  $RCSfile: oinstanceprovider.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 15:46:17 $
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

#ifndef __SFX_OINSTANCEPROVIDER_HXX_
#define __SFX_OINSTANCEPROVIDER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include "omutexmember.hxx"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_BRIDGE_XINSTANCEPROVIDER_HPP_
#include <com/sun/star/bridge/XInstanceProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

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
