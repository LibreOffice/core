/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FRAMEWORK_HELPER_OFRAMES_HXX_
#define __FRAMEWORK_HELPER_OFRAMES_HXX_

#include <classes/framecontainer.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>
#include <general.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weakref.hxx>

namespace framework{

/*-************************************************************************************************************//**
    @short          implement XFrames, XIndexAccess and XElementAccess interfaces as helper for services
    @descr          Use this class as helper for these interfaces. We share mutex and framecontainer with ouer owner.
                    The framecontainer is a member of it from type "FrameContainer". That means;
                    we have the same information as ouer owner. In current implementation we use mutex and lock-mechanism
                    to prevent against compete access. In future we plan support of semaphore!

    @devstatus      deprecated
    @implements     XInterface
                    XFrames
                    XIndexAccess
                    XElementAccess
    @base           OWeakObject

    @ATTENTION      Don't use this class as direct member - use it dynamicly. Do not derive from this class.
                    We hold a weakreference to ouer owner not to ouer superclass.

    @devstatus      deprecated
*//*-*************************************************************************************************************/
class OFrames   :   private ThreadHelpBase      ,   // Must be the first of baseclasses - Is necessary for right initialization of objects!
                    public ::cppu::WeakImplHelper1< ::com::sun::star::frame::XFrames >
{
    public:

        /*-****************************************************************************************************//**
            @short      standard ctor
            @descr      These initialize a new instance of this class with all needed information for work.
                        We share framecontainer with owner implementation! It's a threadsafe container.

            @seealso    -

            @param      xOwner            , reference to ouer owner. We hold a wekreference to prevent us against cross-references!
            @param      pFrameContainer   , pointer to shared framecontainer of owner. It's valid only, if weakreference is valid!
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
         OFrames(   const   css::uno::Reference< css::frame::XFrame >&              xOwner          ,
                            FrameContainer*                                         pFrameContainer );

        //  XFrames

        /*-****************************************************************************************************//**
            @short      append frame to container
            @descr      We share the container with ouer owner. We can do this only, if no lock is set on container.
                        Valid references are accepted only!

            @seealso    class FrameContainer

            @param      "xFrame", reference to an existing frame to append.
            @return     -

            @onerror    We do nothing in release or throw an assert in debug version.
        *//*-*****************************************************************************************************/
        virtual void SAL_CALL append( const css::uno::Reference< css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      remove frame from container
            @descr      This is the companion to append(). We only accept valid references and don't work, if
                        a lock is set.

            @seealso    class FrameContainer

            @param      "xFrame", reference to an existing frame to remove.
            @return     -

            @onerror    We do nothing in release or throw an assert in debug version.
        *//*-*****************************************************************************************************/
        virtual void SAL_CALL remove( const css::uno::Reference< css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      return list of all applicable frames for given flags
            @descr      Call these to get a list of all frames, which are match with given search flags.

            @seealso    -

            @param      "nSearchFlag", flags to search right frames.
            @return     A list of founded frames.

            @onerror    An empty list is returned.
        *//*-*****************************************************************************************************/
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XFrame > > SAL_CALL queryFrames( sal_Int32 nSearchFlags ) throw( css::uno::RuntimeException );

        //  XIndexAccess

        /*-****************************************************************************************************//**
            @short      get count of all current frames in container
            @descr      This is the beginning of full index-access. With a count you can step over all items in container.
                        Next call should be getByIndex(). But these mechanism works only, if no lock in container is set!

            @seealso    class FrameContainer
            @seealso    method getByIndex()

            @param      -
            @return     Count of current items in container.

            @onerror    If a lock is set, we return 0 for prevent further access!
        *//*-*****************************************************************************************************/
        virtual sal_Int32 SAL_CALL getCount() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      get specified container item by index
            @descr      If you called getCount() successful - this method return the specified element as an Any.
                        You must observe the range from 0 to count-1! Otherwise an IndexOutOfBoundsException is thrown.

            @seealso    class FrameContainer
            @seealso    method getCount()

            @param      "nIndex", valid index to get container item.
            @return     A container item (specified by index) wrapped in an Any.

            @onerror    If a lock is set, we return an empty Any!
            @onerror    If index out of range, an IndexOutOfBoundsException is thrown.
        *//*-*****************************************************************************************************/
        virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 nIndex ) throw(    css::lang::IndexOutOfBoundsException    ,
                                                                                css::lang::WrappedTargetException       ,
                                                                                css::uno::RuntimeException              );

        //  XElementAccess

        /*-****************************************************************************************************//**
            @short      get uno-type of all container items
            @descr      In current implementation type is fixed to XFrame!
                        (container-lock is ignored)

            @seealso    -

            @param      -
            @return     A uno-type descriptor.

            @onerror    -
        *//*-*****************************************************************************************************/
        virtual css::uno::Type SAL_CALL getElementType() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      get fill state of current container
            @descr      Call these to get information about, if items exist in container or not.
                        (container-lock is ignored)

            @seealso    -

            @param      -
            @return     sal_True, if container contains some items.
            @return     sal_False, otherwise.

            @onerror    We return sal_False.
        *//*-*****************************************************************************************************/
        virtual sal_Bool SAL_CALL hasElements() throw( css::uno::RuntimeException );

    protected:

        /*-****************************************************************************************************//**
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.
                        This method is protected, because its not allowed to use this class as a member!
                        You MUST use a dynamical instance (pointer). That's the reason for a protected dtor.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        virtual ~OFrames();

        /*-****************************************************************************************************//**
            @short      reset instance to default values
            @descr      There are two ways to delete an instance of this class.<BR>
                        1) delete with destructor<BR>
                        2) dispose from parent or factory ore ...<BR>
                        This method do the same for both ways! It free used memory and release references ...

            @seealso    method dispose() (if it exist!)
            @seealso    destructor ~TaskEnumeration()

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        virtual void impl_resetObject();

    private:

        /*-****************************************************************************************************//**
            @short      append one sequence to another
            @descr      There is no operation to add to sequences! Use this helper-method to do this.

            @seealso    class Sequence

            @param      "seqDestination", reference to sequence on which operation will append the other sequence.
            @param      "seqSource"     , reference to sequence for append.
            @return     "seqDestination" is parameter AND return value at the same time.

            @onerror    -
        *//*-*****************************************************************************************************/
        void impl_appendSequence(           css::uno::Sequence< css::uno::Reference< css::frame::XFrame > >&    seqDestination  ,
                                     const  css::uno::Sequence< css::uno::Reference< css::frame::XFrame > >&    seqSource       );

    //  debug methods
    //  (should be private everyway!)

        /*-****************************************************************************************************//**
            @short      debug-method to check incoming parameter of some other mehods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @seealso    ASSERTs in implementation!

            @param      references to checking variables
            @return     sal_False ,on invalid parameter
            @return     sal_True  ,otherwise

            @onerror    -
        *//*-*****************************************************************************************************/

    private:
        static sal_Bool impldbg_checkParameter_OFramesCtor  (   const   css::uno::Reference< css::frame::XFrame >&              xOwner          ,
                                                                        FrameContainer*                                         pFrameContainer );
        static sal_Bool impldbg_checkParameter_append       (   const   css::uno::Reference< css::frame::XFrame >&              xFrame          );
        static sal_Bool impldbg_checkParameter_remove       (   const   css::uno::Reference< css::frame::XFrame >&              xFrame          );
        static sal_Bool impldbg_checkParameter_queryFrames  (           sal_Int32                                               nSearchFlags    );

    //  variables
    //  (should be private everyway!)

    private:
        css::uno::WeakReference< css::frame::XFrame >               m_xOwner                        ;   /// reference to owner of this instance (Hold no hard reference!)
        FrameContainer*                                             m_pFrameContainer               ;   /// with owner shared list to hold all direct children of an XFramesSupplier
        sal_Bool                                                    m_bRecursiveSearchProtection    ;   /// flag to protect against recursive searches of frames at parents

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
