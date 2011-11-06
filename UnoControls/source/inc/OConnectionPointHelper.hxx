/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _OCONNECTIONPOINTHELPER_HXX
#define _OCONNECTIONPOINTHELPER_HXX

//______________________________________________________________________________________________________________
//  includes of other projects
//______________________________________________________________________________________________________________

#include <com/sun/star/lang/XConnectionPointContainer.hpp>
#include <com/sun/star/lang/XConnectionPoint.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/propshlp.hxx>

//______________________________________________________________________________________________________________
//  includes of my own project
//______________________________________________________________________________________________________________
#include "OConnectionPointContainerHelper.hxx"

//______________________________________________________________________________________________________________
//  namespaces
//______________________________________________________________________________________________________________

namespace unocontrols{

#define UNO3_ANY                                        ::com::sun::star::uno::Any
#define UNO3_SEQUENCE                                   ::com::sun::star::uno::Sequence
#define UNO3_TYPE                                       ::com::sun::star::uno::Type
#define UNO3_REFERENCE                                  ::com::sun::star::uno::Reference
#define UNO3_XCONNECTIONPOINTCONTAINER                  ::com::sun::star::lang::XConnectionPointContainer
#define UNO3_XCONNECTIONPOINT                           ::com::sun::star::lang::XConnectionPoint
#define UNO3_MUTEX                                      ::osl::Mutex
#define UNO3_RUNTIMEEXCEPTION                           ::com::sun::star::uno::RuntimeException
#define UNO3_XINTERFACE                                 ::com::sun::star::uno::XInterface
#define UNO3_OMULTITYPEINTERFACECONTAINERHELPER         ::cppu::OMultiTypeInterfaceContainerHelper
#define UNO3_LISTENEREXISTEXCEPTION                     ::com::sun::star::lang::ListenerExistException
#define UNO3_INVALIDLISTENEREXCEPTION                   ::com::sun::star::lang::InvalidListenerException
#define UNO3_WEAKREFERENCE                              ::com::sun::star::uno::WeakReference
#define UNO3_OWEAKOBJECT                                ::cppu::OWeakObject
#define UNO3_OCONNECTIONPOINTCONTAINERHELPER            OConnectionPointContainerHelper

//______________________________________________________________________________________________________________
//  defines
//______________________________________________________________________________________________________________

//______________________________________________________________________________________________________________
//  class declaration OConnectionPointHelper
//______________________________________________________________________________________________________________

class OConnectionPointHelper    :   public  UNO3_XCONNECTIONPOINT
                                ,   public  UNO3_OWEAKOBJECT
{

//______________________________________________________________________________________________________________
//  public methods
//______________________________________________________________________________________________________________

public:

    //__________________________________________________________________________________________________________
    //  construct/destruct
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    OConnectionPointHelper( UNO3_MUTEX&                             aMutex                      ,
                            UNO3_OCONNECTIONPOINTCONTAINERHELPER*   pContainerImplementation    ,
                            UNO3_TYPE                               aType                       );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ~OConnectionPointHelper();

    //________________________________________________________________________________________________________
    //  XInterface
    //________________________________________________________________________________________________________

    /**_______________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual UNO3_ANY SAL_CALL queryInterface( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @descr      -

        @seealso    XInterface
        @seealso    release()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw();

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @descr      -

        @seealso    XInterface
        @seealso    acquire()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw();

    //__________________________________________________________________________________________________________
    //  XConnectionPoint
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual UNO3_TYPE SAL_CALL getConnectionType() throw( UNO3_RUNTIMEEXCEPTION );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual UNO3_REFERENCE< UNO3_XCONNECTIONPOINTCONTAINER > SAL_CALL getConnectionPointContainer() throw( UNO3_RUNTIMEEXCEPTION );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL advise( const UNO3_REFERENCE< UNO3_XINTERFACE >& xListener ) throw(   UNO3_LISTENEREXISTEXCEPTION     ,
                                                                                                UNO3_INVALIDLISTENEREXCEPTION   ,
                                                                                                UNO3_RUNTIMEEXCEPTION           );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL unadvise( const UNO3_REFERENCE< UNO3_XINTERFACE >& xListener ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual UNO3_SEQUENCE< UNO3_REFERENCE< UNO3_XINTERFACE > > SAL_CALL getConnections() throw( UNO3_RUNTIMEEXCEPTION );

//______________________________________________________________________________________________________________
//  private methods
//______________________________________________________________________________________________________________

private:

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    sal_Bool impl_LockContainer();

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    void impl_UnlockContainer();

//______________________________________________________________________________________________________________
//  private variables
//______________________________________________________________________________________________________________

private:

    UNO3_MUTEX&                                             m_aSharedMutex              ;
    UNO3_WEAKREFERENCE< UNO3_XCONNECTIONPOINTCONTAINER >    m_oContainerWeakReference   ;   // Reference to container-class!. Don't use Reference<...>
                                                                                            // It is a ring-reference => and must be a wekreference!
    UNO3_OCONNECTIONPOINTCONTAINERHELPER*                   m_pContainerImplementation  ;
    UNO3_TYPE                                               m_aInterfaceType            ;
    UNO3_REFERENCE< UNO3_XINTERFACE >                       m_xLock                     ;

};  // class OConnectionPointHelper

}   // namespace unocontrols

#endif  // #ifndef _OCONNECTIONPOINTHELPER_HXX
