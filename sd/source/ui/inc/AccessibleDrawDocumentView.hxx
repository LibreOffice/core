/*************************************************************************
 *
 *  $RCSfile: AccessibleDrawDocumentView.hxx,v $
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


#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_DRAW_DOCUMENT_VIEW_HXX
#define _SD_ACCESSIBILITY_ACCESSIBLE_DRAW_DOCUMENT_VIEW_HXX

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_DOCUMENT_VIEW_BASE_HXX
#include "AccessibleDocumentViewBase.hxx"
#endif


namespace accessibility {


/** @descr
        This class implements the UAA for views of Draw and Impress
        documents.
*/
class AccessibleDrawDocumentView
    :   public AccessibleDocumentViewBase
{
public:
    //=====  internal  ========================================================
    AccessibleDrawDocumentView (SdWindow* pSdWindow,
        SdViewShell* pViewShell,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XController>& rxController,
        const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>& rxParent);

    virtual ~AccessibleDrawDocumentView (void);

    void Init (void);

    //=====  IAccessibleViewForwarderListener  ================================
    virtual void ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder);

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (long nIndex)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
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


protected:
    // The visible children of the document view.
    ChildrenManager* mpChildrenManager;

    /// Model of the document.
    com::sun::star::uno::Reference < ::drafts::com::sun::star::accessibility::XAccessible>
        mxPageShape;

    /** Update the draw page shape.  If a draw page shape does not yet exist
        it is created.
    */
    AccessiblePageShape* UpdateDrawPageShape (void);

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
