/*************************************************************************
 *
 *  $RCSfile: servicemanager.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: as $ $Date: 2001-02-20 09:58:45 $
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

#ifndef __FRAMEWORK_CLASSES_SERVICEMANAGER_HXX_
#define __FRAMEWORK_CLASSES_SERVICEMANAGER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

#define REFERENCE                   ::com::sun::star::uno::Reference
#define XMULTISERVICEFACTORY        ::com::sun::star::lang::XMultiServiceFactory
#define OUSTRING                    ::rtl::OUString
#define MUTEX                       ::osl::Mutex

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          create a new global servicemanager
    @descr          A global servicemanager is neccessary to instanciate UNO-services. To do this, you need
                    a factory to create new objects with special type. That's the reason for a servicemanager.

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class ServiceManager
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard constructor
            @descr      This method don't initialize the new global servicemanager!
                        But we increase an internal ref count. These is needed in dtor to release
                        all created static references to created service mamanger!

            @seealso    dtor

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         ServiceManager();

        /*-****************************************************************************************************//**
            @short      standard destructor to delete instance
            @descr      Here is a good place to destroy the global manager instances!

            @seealso    ctor

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~ServiceManager();

        /*-****************************************************************************************************//**
            @short      initialize global uno service manager and return it
            @descr      This method create a new manager only at first call. We confiscate this with a static
                        pointer, which will be initialized only, if it NULL!
                        Then you can call this method everytime to get a reference to the manager.
                        If you will initialize an uno application you must set returned reference in ::comphelper::setProcessServiceFactory()!
                        The created manager use "applicat.rdb" and "userXX.rdb" automaticly.

            @seealso    -

            @param      -
            @return     A reference to the global servicemanager. It can be NULL!

            @onerror    We return a null-reference.
        *//*-*****************************************************************************************************/

        REFERENCE< XMULTISERVICEFACTORY > getGlobalUNOServiceManager();

        /*-****************************************************************************************************//**
            @short      initialize global uno service manager and return it
            @descr      Do the same like getGlobalUNOServiceManager() before, but use "applicat.rdb" only!

            @seealso    -

            @param      -
            @return     A reference to the global servicemanager. It can be NULL!

            @onerror    We return a null-reference.
        *//*-*****************************************************************************************************/

        REFERENCE< XMULTISERVICEFACTORY > getSimpleGlobalUNOServiceManager();

        /*-****************************************************************************************************//**
            @short      return a reference to a uno servicemanager instance which use your specified user registry file
            @descr      This do the same like method before - but instead of "userXX.rdb" we use your file.
                        These is neccessary, if you will run more then one uno application at the same time in same environment!
                        All created servicemanager use the same "applicat.rdb" but different user registries.

            @ATTENTION  Given file name must be a full qualified system file name. If file not already exist we create a new one!
                        "applicat.rdb", "userXX.rdb" are not valid values!

            @seealso    method generateGlobalUNOServiceManager()
            @seealso    method generatePrivateUNOServiceManager()

            @param      "sUserRegistryFile", full qualified system file name of user registry
            @return     A reference to the created servicemanager. It can be NULL!

            @onerror    We return a null-reference.
        *//*-*****************************************************************************************************/

        REFERENCE< XMULTISERVICEFACTORY > getSharedUNOServiceManager( const OUSTRING& sUserRegistryFile );

        /*-****************************************************************************************************//**
            @short      return a reference to a uno servicemanager instance which use your specified user registry file only
            @descr      This do the same like methods before - but use your file as the only one registry.
                        "applicat.rdb" is used here!

            @ATTENTION  Given file name must be a full qualified system file name. If file not already exist we create a new one!
                        "applicat.rdb", "userXX.rdb" are not valid values!
                        If file was new created - you must register services at runtime himself.
                        Otherwise no service could be created by these manager ...

            @seealso    method generateGlobalUNOServiceManager()
            @seealso    method generateSharedUNOServiceManager()

            @param      "sUserRegistryFile", full qualified system file name of user registry
            @return     A reference to the created servicemanager. It can be NULL!

            @onerror    We return a null-reference.
        *//*-*****************************************************************************************************/

        REFERENCE< XMULTISERVICEFACTORY > getPrivateUNOServiceManager( const OUSTRING& sUserRegistryFile );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      create our own global mutex to prevent us against multithreaded problems
            @descr      We use some static member. For correct access to it we must use the global osl mutex ...
                        but its not fine to do so! These block ALL other operations, which need these mutex too.
                        That's the reason to create our own static mutex. Only first creation is protected
                        by the global mutex, using isn't it!

            @seealso    using

            @param      -
            @return     reference to created static own global mutex

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        MUTEX& impl_getOwnGlobalMutex();

        /*-****************************************************************************************************//**
            @short      create a new global servicemanager instance
            @descr      Is a helper-method for getManager().

            @seealso    method getManager()

            @param      "sRegistryFile", file name of user registry.
            @return     A reference to a new initialized servicemanager with a valid registry.
                        It can be NULL, if an error occured.

            @onerror    Return a NULL-reference.
        *//*-*****************************************************************************************************/

        REFERENCE< XMULTISERVICEFACTORY > impl_createManager( const OUSTRING& sRegistryFile );

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

    #ifdef ENABLE_ASSERTIONS

    private:

    #endif  //  #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  private variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        static REFERENCE< XMULTISERVICEFACTORY >*   m_pGlobalServiceManager         ;
        static REFERENCE< XMULTISERVICEFACTORY >*   m_pSimpleGlobalServiceManager   ;
        static REFERENCE< XMULTISERVICEFACTORY >*   m_pSharedServiceManager         ;
        static REFERENCE< XMULTISERVICEFACTORY >*   m_pPrivateServiceManager        ;
        static sal_Int32                            m_nRefCount                     ;

};      //  class ServiceManager

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_SERVICEMANAGER_HXX_
