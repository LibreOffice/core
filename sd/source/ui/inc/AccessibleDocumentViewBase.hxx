/*************************************************************************
 *
 *  $RCSfile: AccessibleDocumentViewBase.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2002-04-18 17:51:21 $
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
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessible.hpp>
#endif


class SdWindow;
class SdViewShell;

namespace accessibility {


/** @descr
        This class implements the UAA for views of Draw and Impress
        documents.
*/
class AccessibleDocumentViewBase
    :   public AccessibleContextBase,
        public AccessibleComponentBase,
        public IAccessibleViewForwarderListener,
        public ::com::sun::star::beans::XPropertyChangeListener,
        public ::com::sun::star::frame::XFrameActionListener,
        public ::com::sun::star::awt::XWindowListener
{
public:
    //=====  internal  ========================================================
    AccessibleDocumentViewBase (SdWindow* pSdWindow,
        SdViewShell* pViewShell,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XController>& rxController,
        const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>& rxParent);

    virtual ~AccessibleDocumentViewBase (void);

    void Init (void);

    //=====  IAccessibleViewForwarderListener  ================================
    virtual void ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder);

    //=====  XAccessibleContext  ==============================================

    virtual ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleComponent  ============================================

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


    //=====  XFrameActionListener  ============================================

    virtual void SAL_CALL
        frameAction (const ::com::sun::star::frame::FrameActionEvent& rEventObject)
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
    com::sun::star::uno::Reference < ::com::sun::star::frame::XModel>
        mxModel;

    // Bundle of information that is passed down the shape tree.
    AccessibleShapeTreeInfo maShapeTreeInfo;

    /// The view forwarder passed to the children manager.
    AccessibleViewForwarder maViewForwarder;

    // This method is called from the component helper base class while disposing.
    virtual void SAL_CALL disposing (void);

    /// Set this object's name if is different to the current name.
    virtual ::rtl::OUString
        CreateAccessibleName ()
        throw (::com::sun::star::uno::RuntimeException);

    /// Set this object's description if is different to the current description.
    virtual ::rtl::OUString
        CreateAccessibleDescription ()
        throw (::com::sun::star::uno::RuntimeException);
};

} // end of namespace accessibility

#endif
