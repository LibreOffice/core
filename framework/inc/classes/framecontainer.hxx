/*************************************************************************
 *
 *  $RCSfile: framecontainer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2000-10-16 11:51:52 $
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

#ifndef __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_
#define __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_ASYNCQUIT_HXX_
#include <classes/asyncquit.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef __SGI_STL_VECTOR
#include <stl/vector>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

#define XFRAME                  ::com::sun::star::frame::XFrame
#define REFERENCE               ::com::sun::star::uno::Reference
#define SEQUENCE                ::com::sun::star::uno::Sequence
#define STLVECTOR               ::std::vector

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          implement a container to hold childs of frame, task or desktop
    @descr          Every object of frame, task or desktop hold reference to his childs. These container is used as helper
                    to do this. Some helper-classe like OFrames or OTasksAccess use it to. They hold a pointer to an instance
                    of this class, which is a member of a frame, task or desktop! You can append and remove frames.
                    It's possible to set one of these frames as active or deactive. You could have full index-access to
                    container-items. To block append- or remove-calls, its possible to set a lock. Nobody can change the container
                    the lock is set. But don't forget to unlock the container again!
                    These class is NOT threadsafe!

    @implements     -
    @base           -

    @ATTENTION      This class is not threadsafe.

    @devstatus      deprecated
*//*-*************************************************************************************************************/

class FrameContainer
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
            @descr      This will initialize an empty container.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         FrameContainer();

        /*-****************************************************************************************************//**
            @short      standard destructor to delete instance
            @descr      This will clear the container, if programmer forget this.

            @seealso    method clear()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~FrameContainer();

        /*-****************************************************************************************************//**
            @short      append a new frame to the end of container
            @descr      The reference must be valid! If it's not, we do nothing.
                        If a lock is set, we do nothing to.
                        (In debug version an assertion is thrown to show the programmer possible problems!)

            @seealso    -

            @param      "xFrame" is the frame to add in container.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void append( const REFERENCE< XFRAME >& xFrame );

        /*-****************************************************************************************************//**
            @short      remove an existing frame from the container
            @descr      The reference must be valid! If element not exist in container we do nothing.
                        If a lock is set, we do nothing to.
                        (In debug version an assertion is thrown to show the programmer possible problems!)

            @seealso    method clear()

            @param      "xFrame" is the frame to remove from the container.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void remove( const REFERENCE< XFRAME >& xFrame );

        /*-****************************************************************************************************//**
            @short      ask for an existing frame in container
            @descr      Use it to get information about existing items in container.
                        The reference must be valid! The lock is ignored! (We do not change the content of container.)

            @seealso    -

            @param      "xFrame" is the frame to search.
            @return     sal_True , if frame exist<BR>
                        sal_False, other way.

            @onerror    We return sal_False.
        *//*-*****************************************************************************************************/

        sal_Bool exist( const REFERENCE< XFRAME >& xFrame );

        /*-****************************************************************************************************//**
            @short      clear the container and free memory
            @descr      This will clear the container. If you call this method again and the container is already empty, we do nothing!
                        If a lock is set, we do nothing.
                        (In debug version an assertion is thrown to show the programmer this problem!)

            @seealso    method remove()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void clear();

        /*-****************************************************************************************************//**
            @short      lock container
            @descr      You can use some methods of this class to have full access to the container by index!
                        But if someone add or remove frames to/from container we have a problem....
                        With this method you can LOCK the container and all append()-, remove()- and clear()-calls are blocked.
                        But ... don't forget to unlock ... !!!

            @seealso    method unlock()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void lock();

        /*-****************************************************************************************************//**
            @short      unlock container
            @descr      You can use some methods of this class to break full access to the container by index!
                        From now, all access methods by index are blocked. Append(), remove() and clear() are non blocked.

            @seealso    method lock()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void unlock();

        /*-****************************************************************************************************//**
            @short      get count of items in container
            @descr      We ignore the lock - because caller can know these value but he can use it for
                        direct indexaccess only, if a lock is set!

            @seealso    -

            @param      -
            @return     count of container items.

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_uInt32 getCount() const;

        /*-****************************************************************************************************//**
            @short      get item of container by index
            @descr      If no lock is set, we return NULL. The index must in range [0 ... count-1]!

            @seealso    -

            @param      -
            @return     Frame item, if index valid<BR>
                        NULL, other way.

            @onerror    We return NULL!
        *//*-*****************************************************************************************************/

        REFERENCE< XFRAME > operator[]( sal_uInt32 nIndex ) const;

        /*-****************************************************************************************************//**
            @short      get all current items of container as snapshot
            @descr      No lock must set. We return a snapshot only.

            @seealso    -

            @param      -
            @return     Sequence of frames

            @onerror    -
        *//*-*****************************************************************************************************/

        SEQUENCE< REFERENCE< XFRAME > > getAllElements();

        /*-****************************************************************************************************//**
            @short      ask it to get information about existing elements
            @descr      No lock must set. We use current container items only.

            @seealso    -

            @param      -
            @return     sal_True , if one or more elements exist<BR>
                        sal_False, other way.

            @onerror    We return sal_False.
        *//*-*****************************************************************************************************/

        sal_Bool hasElements() const;

        /*-****************************************************************************************************//**
            @short      set the current active frame in container
            @descr      Some implementations like Desktop or Frame need an active frame.
                        But this frame must be a child of these objects! Its not possible to hold an extra reference
                        for these special case. Its better to control this rule by the container himself.
                        He know, which frame is child or not.

            @seealso    method Desktop::setActiveFrame()
            @seealso    method Frame::setActiveFrame()

            @param      "xFrame" must a valid reference to an existing frame in container.
            @return     -

            @onerror    If refrence not valid, we throw an assertion!
        *//*-*****************************************************************************************************/

        void setActive( const REFERENCE< XFRAME >& xFrame );

        /*-****************************************************************************************************//**
            @short      get the current active frame in container.
            @descr      -

            @seealso    method Desktop::getActiveFrame()
            @seealso    method Frame::getActiveFrame()

            @param      -
            @return     A valid reference, if an active one exist.
                        A null-reference, other way.

            @onerror    We return a null-reference.
        *//*-*****************************************************************************************************/

        REFERENCE< XFRAME > getActive() const;

        /*-****************************************************************************************************//**
            @short      Enable or disable automatic termination of desktop if last frame was removed from container
            @descr      Only the desktop should use this functions!

            @seealso    class Desktop
            @seealso    class AsyncQuit

            @param      "aMode", with these mode you can set default time values for different shutdown scenes at timer
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void enableQuitTimer(   const   REFERENCE< XDESKTOP >&  xDesktop    ,
                                const   TAsyncQuitMode&         aMode       );
        void disableQuitTimer();

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

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

    #ifdef ENABLE_ASSERTIONS

    private:

        sal_Bool impldbg_checkParameter_append          (   const   REFERENCE< XFRAME >&    xFrame  ) const;
        sal_Bool impldbg_checkParameter_remove          (   const   REFERENCE< XFRAME >&    xFrame  ) const;
        sal_Bool impldbg_checkParameter_exist           (   const   REFERENCE< XFRAME >&    xFrame  ) const;
        sal_Bool impldbg_checkParameter_IndexOperator   (           sal_uInt32              nIndex  ) const;
        sal_Bool impldbg_checkParameter_setActive       (   const   REFERENCE< XFRAME >&    xFrame  ) const;

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  private variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        sal_Bool                                m_bLock             ;   /// lock to block append()-, remove()- or clear()-calls
        STLVECTOR< REFERENCE< XFRAME > >        m_aContainer        ;   /// list to hold all frames
        REFERENCE< XFRAME >                     m_xActiveFrame      ;   /// one container item can be the current active frame. Its neccessary for Desktop or Frame implementation.
        AsyncQuit*                              m_pQuitTimer        ;   /// if an instance of these class used by desktop and last frame will be removed we must terminate the desktop

};      //  class FrameContainer

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_
