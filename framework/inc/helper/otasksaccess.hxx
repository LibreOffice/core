/*************************************************************************
 *
 *  $RCSfile: otasksaccess.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:22 $
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

#ifndef __FRAMEWORK_HELPER_OTASKSACCESS_HXX_
#define __FRAMEWORK_HELPER_OTASKSACCESS_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_
#include <classes/framecontainer.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_OMUTEXMEMBER_HXX_
#include <helper/omutexmember.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XELEMENTACCESS_HPP_
#include <com/sun/star/container/XElementAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

#define ANY                                 ::com::sun::star::uno::Any
#define EVENTOBJECT                         ::com::sun::star::lang::EventObject
#define NOSUCHELEMENTEXCEPTION              ::com::sun::star::container::NoSuchElementException
#define OWEAKOBJECT                         ::cppu::OWeakObject
#define REFERENCE                           ::com::sun::star::uno::Reference
#define RUNTIMEEXCEPTION                    ::com::sun::star::uno::RuntimeException
#define UNOTYPE                             ::com::sun::star::uno::Type
#define WEAKREFERENCE                       ::com::sun::star::uno::WeakReference
#define WRAPPEDTARGETEXCEPTION              ::com::sun::star::lang::WrappedTargetException
#define XDESKTOP                            ::com::sun::star::frame::XDesktop
#define XELEMENTACCESS                      ::com::sun::star::container::XElementAccess
#define XENUMERATION                        ::com::sun::star::container::XEnumeration
#define XENUMERATIONACCESS                  ::com::sun::star::container::XEnumerationAccess
#define XTASK                               ::com::sun::star::frame::XTask
#define MUTEX                               ::osl::Mutex

//_________________________________________________________________________________________________________________
//  switches
//  Use these to de/activate some features of this implementation.
//  (for debugging or testing!)
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          implement XEnumerationAccess interface as helper to create many oneway enumeration of tasks
    @descr          We share mutex and framecontainer with ouer owner and have full access to his child tasks.
                    (Ouer owner can be the Desktop only!) We create oneway enumerations on demand. These "lists"
                    can be used for one time only. Step during the list from first to last element.
                    (The type of created enumerations is OTasksEnumeration.)

    @implements     XInterface
                    XEnumerationAccess
                    XElementAccess
                    [ XDebugging if TEST_TREE is defined! ]
    @base           OWeakObject

    @devstatus      deprecated
*//*-*************************************************************************************************************/

//class OTasksAccess    :   DERIVE_FROM_XSPECIALDEBUGINTERFACE      // => These macro will expand to nothing, if no testmode is set in debug.h!
class OTasksAccess  :   public XENUMERATIONACCESS           ,   // => XElementAccess
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
            @short      constructor to initialize this instance
            @descr      A desktop will create an enumeration-access-object. An enumeration is a oneway-list and a
                        snapshot of the tasklist of current tasks of desktop.
                        But we need a instance to create more then one enumerations to the same tasklist!

            @seealso    class Desktop
            @seealso    class OTasksEnumeration

            @param      "xOwner" is a reference to ouer owner and must be the desktop!
            @param      "pTasks" is a pointer to the taskcontainer of the desktop. We need it to create a new enumeration.
            @param      "aMutex" is a reference to the shared mutex of ouer owner(the desktop).
            @return     -

            @onerror    Do nothing and reset this object to default with an empty list.
        *//*-*****************************************************************************************************/

         OTasksAccess(  const   REFERENCE< XDESKTOP >&      xOwner  ,
                                FrameContainer*             pTasks  ,
                                MUTEX&                      aMutex  );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE
//      DECLARE_XSPECIALDEBUGINTERFACE  // => These macro will expand to nothing, if no testmode is set in debug.h!

        //---------------------------------------------------------------------------------------------------------
        //  XEnumerationAccess
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      create a new enumeration of tasks
            @descr      You can call this method to get a new snapshot to all tasks of the desktop as an enumeration.

            @seealso    interface XEnumerationAccess
            @seealso    interface XEnumeration
            @seealso    class Desktop

            @param      -
            @return     If the desktop and some tasks exist => a valid reference to an enumeration<BR>
                        An NULL-reference, other way.

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XENUMERATION > SAL_CALL createEnumeration() throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //  XElementAccess
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      get the type of elements in enumeration
            @descr      -

            @seealso    interface XElementAccess
            @seealso    class TasksEnumeration

            @param      -
            @return     The uno-type XTask.

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual UNOTYPE SAL_CALL getElementType() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      get state of tasklist of enumeration.
            @descr      -

            @seealso    interface XElementAccess

            @param      -
            @return     sal_True  ,if more then 0 elements exist.
            @return     sal_False ,otherwise.

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL hasElements() throw( RUNTIMEEXCEPTION );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

        /*-****************************************************************************************************//**
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.
                        Don't use an instance of this class as normal member. Use it dynamicly with a pointer.
                        We hold a weakreference to ouer owner and not to ouer superclass!
                        Thats the reason for a protected dtor.

            @seealso    class Desktop

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~OTasksAccess();

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
            @return     sal_False ,on invalid parameter.
            @return     sal_True  ,otherwise

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_ASSERTIONS

    private:

        sal_Bool impldbg_checkParameter_OTasksAccessCtor(   const   REFERENCE< XDESKTOP >&      xOwner  ,
                                                                    FrameContainer*             pTasks  ,
                                                                    MUTEX&                      aMutex  ) const;

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        MUTEX&                              m_aMutex            ;   /// shared mutex with owner
        WEAKREFERENCE< XDESKTOP >           m_xOwner            ;   /// weak reference to the desktop object!
        FrameContainer*                     m_pTasks            ;   /// pointer to list of current tasks on desktop (is a member of class Desktop!)
                                                                    /// This pointer is valid only, if weakreference can be locked.

};      //  class OTasksAccess

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_HELPER_OTASKSACCESS_HXX_
