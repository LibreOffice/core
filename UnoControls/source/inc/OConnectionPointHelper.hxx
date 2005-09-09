/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OConnectionPointHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:19:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OCONNECTIONPOINTHELPER_HXX
#define _OCONNECTIONPOINTHELPER_HXX

//______________________________________________________________________________________________________________
//  includes of other projects
//______________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XCONNECTIONPOINTCONTAINER_HPP_
#include <com/sun/star/lang/XConnectionPointContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCONNECTIONPOINT_HPP_
#include <com/sun/star/lang/XConnectionPoint.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif

//______________________________________________________________________________________________________________
//  includes of my own project
//______________________________________________________________________________________________________________

#ifndef _OCONNECTIONPOINTCONTAINERHELPER_HXX
#include "OConnectionPointContainerHelper.hxx"
#endif

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
