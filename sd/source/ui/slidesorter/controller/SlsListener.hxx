/*************************************************************************
 *
 *  $RCSfile: SlsListener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:14:41 $
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

#ifndef SD_SLIDESORTER_SLIDE_SORTER_LISTENER_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_LISTENER_HXX

#include "MutexOwner.hxx"

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTLISTENER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif

#include <svtools/lstner.hxx>

namespace sd { namespace slidesorter { namespace controller {

typedef cppu::WeakComponentImplHelper4<
    ::com::sun::star::document::XEventListener,
    ::com::sun::star::beans::XPropertyChangeListener,
    ::com::sun::star::accessibility::XAccessibleEventListener,
    ::com::sun::star::frame::XFrameActionListener
    > ListenerInterfaceBase;

class SlideSorterController;

/** Listen for events of various types and sources and react to them.  This
    class is a part of the controller.

    When the view shell in the center pane is replaced by another the
    associated controller is replaced as well.  Therefore we have to
    register at the frame and on certain FrameActionEvents to stop listening
    to the old controller and register as listener at the new one.
*/
class Listener
    : protected MutexOwner,
      public ListenerInterfaceBase,
      public SfxListener
{
public:
    Listener (SlideSorterController& rController);
    virtual ~Listener (void);

    /** Connect to the current controller of the view shell as listener.
        This method is called once during initialization and every time a
        FrameActionEvent signals the current controller being exchanged.
        When the connection is successfull then the flag
        mbListeningToController is set to <TRUE/>.
    */
    void ConnectToController (void);

    /** Disconnect from the current controller of the view shell as
        listener.  This method is called once during initialization and
        every time a FrameActionEvent signals the current controller being
        exchanged.  When this method terminates then mbListeningToController
        is <FALSE/>.
    */
    void DisconnectFromController (void);

    virtual void Notify (
        SfxBroadcaster& rBroadcaster,
        const SfxHint& rHint);

    //=====  lang::XEventListener  ============================================
    virtual void SAL_CALL
        disposing (const ::com::sun::star::lang::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  document::XEventListener  ========================================
    virtual void SAL_CALL
        notifyEvent (
            const ::com::sun::star::document::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  beans::XPropertySetListener  =====================================
    virtual void SAL_CALL
        propertyChange (
            const com::sun::star::beans::PropertyChangeEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException);

    //===== accessibility::XAccessibleEventListener  ==========================
    virtual void SAL_CALL
        notifyEvent (
            const ::com::sun::star::accessibility::AccessibleEventObject&
            rEvent)
        throw (::com::sun::star::uno::RuntimeException);

    //===== frame::XFrameActionListener  ======================================
    /** For certain actions the listener connects to a new controller of the
        frame it is listening to.  This usually happens when the view shell
        in the center pane is replaced by another view shell.
    */
    virtual void SAL_CALL
        frameAction (const ::com::sun::star::frame::FrameActionEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing (void);

private:
    SlideSorterController& mrController;

    /// Remember whether we are listening to the document.
    bool mbListeningToDocument;
    /// Remember whether we are listening to the UNO document.
    bool mbListeningToUNODocument;
    /// Remember whether we are listening to the UNO controller.
    bool mbListeningToController;
    /// Remember whether we are listening to the frame.
    bool mbListeningToFrame;

    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::beans::XPropertySet> mxControllerPropertySetWeak;
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::frame::XFrame> mxFrameWeak;


    void ReleaseListeners (void);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (::com::sun::star::lang::DisposedException);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif
