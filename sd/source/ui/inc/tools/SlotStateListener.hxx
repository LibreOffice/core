/*************************************************************************
 *
 *  $RCSfile: SlotStateListener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-03-18 16:49:58 $
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

#ifndef SD_TOOLS_SLOT_STATE_LISTENER_HXX
#define SD_TOOLS_SLOT_STATE_LISTENER_HXX

#include "MutexOwner.hxx"

#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHP_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTIOIN_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#include <tools/link.hxx>

namespace sd { namespace tools {


typedef cppu::WeakComponentImplHelper1<
    ::com::sun::star::frame::XStatusListener
    > SlotStateListenerInterfaceBase;


/** Listen for state changes of slots.  This class has been created in order
    to be informed when the support for vertical writing changes but it can
    be used to relay state changes of other slots as well.
*/
class SlotStateListener
    : protected MutexOwner,
      public SlotStateListenerInterfaceBase
{
public:
    /** Create a new object that has yet to be told about the the frame and
        slots to listen to and the callback that state changes are relayed
        to.  When you want to listen to one single slot then you can use the
        three argument variant of this constructor.
    */
    SlotStateListener (void);

    /** This convenience version of the constructor takes all parameters
        that are necessary to observe a single slot.  See descriptions of
        the SetCallback(), ConnectToFrame(), and ObserveSlot() methods for
        explanations about the parameters.
    */
    SlotStateListener (
        Link& rCallback,
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame> xFrame,
        const ::rtl::OUString& rSlotName);

    /** The constructor de-registers all remaining listeners.  Usually a prior
        dispose() call should have done that already.
    */
    virtual ~SlotStateListener (void);

    /** Set the callback to the given value.  Whenever one of the observed
        slots changes its state this callback is informed about it.
        Changing the callback does not release the listeners.
        @throws DisposedException
    */
    void SetCallback (const Link& rCallback);

    /** Set the frame whose slots shall be observed.  When an object of this
        class is already observing slots of another frame then these
        listeners are released first.
        @throws DisposedException
    */
    void ConnectToFrame (::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame> xFrame);

    /** Observe the slot specified by the given name.  Note that
        ConnectToFrame() has to have been called earlier.
        @param rSlotName
            The name of the slot to observe.  An example is
            ".uno:VerticalTextState".
        @throws DisposedException
    */
    void ObserveSlot (const ::rtl::OUString& rSlotName);


    //=====  frame::XStatusListener  ==========================================

    /** Called by slot state change broadcasters.  In turn the callback is
        informed about the state chage.
        @throws DisposedException
    */
    virtual void SAL_CALL
        statusChanged (
            const ::com::sun::star::frame::FeatureStateEvent& rState)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  lang::XEventListener  ============================================

    virtual void SAL_CALL
        disposing(const com::sun::star::lang::EventObject& rEvent)
        throw(com::sun::star::uno::RuntimeException);

protected:
    /** This method is called by the WeakComponentImplHelper base class in
        reaction to a XComponent::dispose() call.  It releases all currently
        active listeners.
    */
    virtual void SAL_CALL disposing (void);

private:
    Link maCallback;

    /** Remember the URLs that describe slots whose state changes we are
        listening to.
    */
    typedef ::std::vector<com::sun::star::util::URL> RegisteredURLList;
    RegisteredURLList maRegisteredURLList;

    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::frame::XFrame> mxFrameWeak;

    /** Deregister all currently active state change listeners.
    */
    void ReleaseListeners (void);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (::com::sun::star::lang::DisposedException);

    /** Transform the given string into a URL object.
    */
    ::com::sun::star::util::URL MakeURL (const ::rtl::OUString& rSlotName) const;

    /** Return an XDispatch object for the given URL.
    */
    ::com::sun::star::uno::Reference<com::sun::star::frame::XDispatch>
        GetDispatch (
            const ::com::sun::star::util::URL& rURL) const;
};

} } // end of namespace ::sd::tools

#endif
