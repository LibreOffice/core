/*************************************************************************
 *
 *  $RCSfile: AccessibleDocumentViewBase.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:04:28 $
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

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_DOCUMENT_VIEW_BASE_HXX
#define _SD_ACCESSIBILITY_ACCESSIBLE_DOCUMENT_VIEW_BASE_HXX

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_CONTEXT_BASE_HXX
#include <svx/AccessibleContextBase.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_COMPONENT_BASE_HXX
#include <svx/AccessibleComponentBase.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SELECTION_BASE_HXX
#include <svx/AccessibleSelectionBase.hxx>
#endif
#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_VIEW_FORWARDER_HXX
#include "AccessibleViewForwarder.hxx"
#endif
#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_PAGE_SHAPE_HXX
#include "AccessiblePageShape.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_CHILDREN_MANAGER_HXX
#include <svx/ChildrenManager.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XWindowListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XTopWindowListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

class SdWindow;
class SdViewShell;
class VclSimpleEvent;

namespace accessibility {


/** Base class for the various document views of the Draw and
    Impress applications.

    <p>The different view modes of the Draw and Impress applications
    are made accessible by derived classes.  When the view mode is
    changed than the object representing the document view is
    disposed and replaced by a new instance of the then appropriate
    derived class.</p>

    <p>This base class also manages an optionally active accessible OLE
    object.  If you overwrite the <member>getAccessibleChildCount</member>
    and <member>getAccessibleChild</member> methods then make sure to first
    call the corresponding method of this class and adapt your child count
    and indices accordingly.  Only one active OLE object is allowed at a
    time.  This class does not listen for disposing calls at the moment
    because it does not use the accessible OLE object directly and trusts on
    getting informed through VCL window events.</p>

    <p>This class implements three kinds of listeners:
    <ol><li>The property change listener is not used directly but exists as
    convenience for derived classes.  May be moved to those classes
    instead.</li>
    <li>As window listener it waits for changes of the window geometry and
    forwards those as view forwarder changes.</li>
    <li>As top window listener it waits for activation and deactivation of
    the top window containing the document view to give this class and
    derived classes the oportunity to set and remove the focus to/from
    shapes.</li>
    </ol>
    </p>
*/
class AccessibleDocumentViewBase
    :   public AccessibleContextBase,
        public AccessibleComponentBase,
        public AccessibleSelectionBase,
        public IAccessibleViewForwarderListener,
        public ::com::sun::star::beans::XPropertyChangeListener,
        public ::com::sun::star::awt::XWindowListener,
        public ::com::sun::star::awt::XTopWindowListener
{
public:
    //=====  internal  ========================================================

    /** Create a new object.  Note that the caller has to call the
        Init method directly after this constructor has finished.
    @param pSdWindow
        The window whose content is to be made accessible.
    @param pViewShell
        The view shell associated with the given window.
    @param rxController
        The controller from which to get the model.
    @param rxParent
        The accessible parent of the new object.  Note that this parent does
        not necessarily correspond with the parent of the given window.
     */
    AccessibleDocumentViewBase (
        SdWindow* pSdWindow,
        SdViewShell* pViewShell,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XController>& rxController,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent);

    virtual ~AccessibleDocumentViewBase (void);

    /** Initialize a new object.  Call this method directly after creating a
        new object.  It finished the initialization begun in the constructor
        but which needs a fully created object.
     */
    virtual void Init (void);

    /** Define callback for listening to window child events of VCL.
        Listen for creation or destruction of OLE objects.
    */
    DECL_LINK (WindowChildEventListener, VclSimpleEvent*);

    //=====  IAccessibleViewForwarderListener  ================================

    /** A view forwarder change is signalled for instance when any of the
        window events is recieved.  Thus, instead of overloading the four
        windowResized... methods it will be sufficient in most cases just to
        overload this method.
     */
    virtual void ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder);

    //=====  XAccessibleContext  ==============================================

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** This implementation returns either 1 or 0 depending on whether there
        is an active accessible OLE object or not.
    */
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** This implementation either returns the active accessible OLE object
        if it exists and the given index is 0 or throws an exception.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (long nIndex)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint (const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Point SAL_CALL getLocation (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Size SAL_CALL getSize (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XInterface  ======================================================

    virtual com::sun::star::uno::Any SAL_CALL
        queryInterface (const com::sun::star::uno::Type & rType)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        acquire (void)
        throw ();

    virtual void SAL_CALL
        release (void)
        throw ();


    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XTypeProvider  ===================================================

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
        getTypes (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  lang::XEventListener  ============================================

    virtual void SAL_CALL
        disposing (const ::com::sun::star::lang::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XPropertyChangeListener  =========================================

    virtual void SAL_CALL
        propertyChange (const ::com::sun::star::beans::PropertyChangeEvent& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XWindowListener  =================================================

    virtual void SAL_CALL
        windowResized (const ::com::sun::star::awt::WindowEvent& e)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        windowMoved (const ::com::sun::star::awt::WindowEvent& e)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        windowShown (const ::com::sun::star::lang::EventObject& e)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        windowHidden (const ::com::sun::star::lang::EventObject& e)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XTopWindowListener  ==============================================

    virtual void SAL_CALL windowOpened( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowClosing( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowClosed( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowMinimized( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowNormalized( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowActivated( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowDeactivated( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

protected:

    // return the member maMutex;
    virtual ::osl::Mutex&
        implGetMutex();

    // return ourself as context in default case
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
        implGetAccessibleContext()
        throw ( ::com::sun::star::uno::RuntimeException );

    // return sal_False in default case
    virtual sal_Bool
        implIsSelected( sal_Int32 nAccessibleChildIndex )
        throw (::com::sun::star::uno::RuntimeException);

    // return nothing in default case
    virtual void
        implSelect( sal_Int32 nAccessibleChildIndex, sal_Bool bSelect )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

protected:
    /// The core window that is made accessible.
    SdWindow* mpWindow;

    /// The API window that is made accessible.
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>
         mxWindow;

    /// The controller of the window in which this view is displayed.
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController>
         mxController;

    /// Model of the document.
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XModel>
        mxModel;

    // Bundle of information that is passed down the shape tree.
    AccessibleShapeTreeInfo maShapeTreeInfo;

    /// The view forwarder passed to the children manager.
    AccessibleViewForwarder maViewForwarder;

    /** Accessible OLE object.  Set or removed by the
        <member>SetAccessibleOLEObject</member> method.
    */
    ::com::sun::star::uno::Reference <
        ::com::sun::star::accessibility::XAccessible>
        mxAccessibleOLEObject;

    // This method is called from the component helper base class while
    // disposing.
    virtual void SAL_CALL disposing (void);

    /** Create a name string.  The current name is not modified and,
        therefore, no events are send.  This method is usually called once
        by the <member>getAccessibleName</member> method of the base class.
        @return
           A name string.
    */
    virtual ::rtl::OUString
        CreateAccessibleName ()
        throw (::com::sun::star::uno::RuntimeException);

    /** Create a description string.  The current description is not
        modified and, therefore, no events are send.  This method is usually
        called once by the <member>getAccessibleDescription</member> method
        of the base class.
        @return
           A description string.
    */
    virtual ::rtl::OUString
        CreateAccessibleDescription ()
        throw (::com::sun::star::uno::RuntimeException);

    /** This method is called when (after) the frame containing this
        document has been activated.  Can be used to send FOCUSED state
        changes for the currently selected element.
    */
    virtual void Activated (void);

    /** This method is called when (before or after?) the frame containing
        this document has been deactivated.  Can be used to send FOCUSED
        state changes for the currently selected element.
    */
    virtual void Deactivated (void);

    /** Set or remove the currently active accessible OLE object.
        @param xOLEObject
            If this is a valid reference then a child event is send that
            informs the listeners of a new child.  If there has already been
            an active accessible OLE object then this is removed first and
            appropriate events are send.

            If this is an empty reference then the currently active
            accessible OLE object (if there is one) is removed.
    */
    virtual void SetAccessibleOLEObject (
        const ::com::sun::star::uno::Reference <
        ::com::sun::star::accessibility::XAccessible>& xOLEObject);
};

} // end of namespace accessibility

#endif
