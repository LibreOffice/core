/*************************************************************************
 *
 *  $RCSfile: ocomponentaccess.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2000-09-26 13:01:14 $
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

#ifndef __FRAMEWORK_HELPER_OCOMPONENTACCESS_HXX_
#define __FRAMEWORK_HELPER_OCOMPONENTACCESS_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_HELPER_OMUTEXMEMBER_HXX_
#include <helper/omutexmember.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif

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

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
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
#define MUTEX                               ::osl::Mutex
#define NOSUCHELEMENTEXCEPTION              ::com::sun::star::container::NoSuchElementException
#define OWEAKOBJECT                         ::cppu::OWeakObject
#define REFERENCE                           ::com::sun::star::uno::Reference
#define RUNTIMEEXCEPTION                    ::com::sun::star::uno::RuntimeException
#define SEQUENCE                            ::com::sun::star::uno::Sequence
#define UNOTYPE                             ::com::sun::star::uno::Type
#define WEAKREFERENCE                       ::com::sun::star::uno::WeakReference
#define WRAPPEDTARGETEXCEPTION              ::com::sun::star::lang::WrappedTargetException
#define XDESKTOP                            ::com::sun::star::frame::XDesktop
#define XELEMENTACCESS                      ::com::sun::star::container::XElementAccess
#define XENUMERATION                        ::com::sun::star::container::XEnumeration
#define XENUMERATIONACCESS                  ::com::sun::star::container::XEnumerationAccess
#define XFRAMESSUPPLIER                     ::com::sun::star::frame::XFramesSupplier
#define XFRAME                              ::com::sun::star::frame::XFrame
#define XTASK                               ::com::sun::star::frame::XTask
#define XTYPEPROVIDER                       ::com::sun::star::lang::XTypeProvider
#define XCOMPONENT                          ::com::sun::star::lang::XComponent

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          implement XEnumerationAccess interface as helper to create many oneway enumeration of components
    @descr          We share mutex and framecontainer with ouer owner and have full access to his child tasks.
                    (Ouer owner can be the Desktop only!) We create oneway enumerations on demand. These "lists"
                    can be used for one time only. Step during the list from first to last element.
                    (The type of created enumerations is OComponentEnumeration.)

    @implements     XInterface
                    XTypeProvider
                    XEnumerationAccess
                    XElementAccess

    @base           OWeakObject

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class OComponentAccess  :   public XTYPEPROVIDER                ,
                            public XENUMERATIONACCESS           ,   // => XElementAccess
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
                        snapshot of the components of current tasks under the desktop.
                        But we need a instance to create more then one enumerations at different times!

            @seealso    class Desktop
            @seealso    class OComponentEnumeration

            @param      "xOwner" is a reference to ouer owner and must be the desktop!
            @param      "aMutex" is a reference to the shared mutex of ouer owner(the desktop).
            @return     -

            @onerror    Do nothing and reset this object to default with an empty list.
        *//*-*****************************************************************************************************/

         OComponentAccess(  const   REFERENCE< XDESKTOP >&      xOwner  ,
                                    MUTEX&                      aMutex  );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        //---------------------------------------------------------------------------------------------------------
        //  XEnumerationAccess
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      create a new enumeration of components
            @descr      You can call this method to get a new snapshot from all components of all tasks of the desktop as an enumeration.

            @seealso    interface XEnumerationAccess
            @seealso    interface XEnumeration
            @seealso    class Desktop

            @param      -
            @return     If the desktop and some components exist => a valid reference to an enumeration<BR>
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
            @seealso    class OComponentEnumeration

            @param      -
            @return     The uno-type XComponent.

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual UNOTYPE SAL_CALL getElementType() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      get state of componentlist of enumeration.
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

        virtual ~OComponentAccess();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      recursive method (!) to collect all components of all frames from the subtree of given node
            @descr      This is neccessary to create the enumeration.

            @seealso    method createEnumeration

            @param      "xNode"        , root of subtree and start point of search
            @param      "seqComponents", result list of search. We cant use a return value, we search recursive
                                         and must collect all informations.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_collectAllChildComponents(    const   REFERENCE< XFRAMESSUPPLIER >&           xNode           ,
                                                          SEQUENCE< REFERENCE< XCOMPONENT > >&  seqComponents   );

        /*-****************************************************************************************************//**
            @short      get the component of a frame
            @descr      The component of a frame can be the window, the controller or the model.

            @seealso    method createEnumeration

            @param      "xFrame", frame which contains the component
            @return     A reference to the component of given frame.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        REFERENCE< XCOMPONENT > impl_getFrameComponent( const REFERENCE< XFRAME >& xFrame ) const;

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

        sal_Bool impldbg_checkParameter_OComponentAccessCtor(   const   REFERENCE< XDESKTOP >&      xOwner  ,
                                                                        MUTEX&                      aMutex  ) const;

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        MUTEX&                              m_aMutex            ;   /// shared mutex with owner
        WEAKREFERENCE< XDESKTOP >           m_xOwner            ;   /// weak reference to the desktop object!

};      //  class OComponentAccess

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_HELPER_OCOMPONENTACCESS_HXX_
