/*************************************************************************
 *
 *  $RCSfile: accessibledialogwindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-03-26 12:50:21 $
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

#ifndef _BASCTL_ACCESSIBLEDIALOGWINDOW_HXX_
#define _BASCTL_ACCESSIBLEDIALOGWINDOW_HXX_

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef COMPHELPER_ACCESSIBLE_COMPONENT_HELPER_HXX
#include <comphelper/accessiblecomponenthelper.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

#include <vector>

class DialogWindow;
class DlgEditor;
class DlgEdModel;
class DlgEdObj;
class VCLExternalSolarLock;
class VclSimpleEvent;
class VclWindowEvent;

namespace utl {
class AccessibleStateSetHelper;
}


//  ----------------------------------------------------
//  class AccessibleDialogWindow
//  ----------------------------------------------------

typedef ::comphelper::OAccessibleExtendedComponentHelper    AccessibleExtendedComponentHelper_BASE;

typedef ::cppu::ImplHelper3 <
    ::drafts::com::sun::star::accessibility::XAccessible,
    ::drafts::com::sun::star::accessibility::XAccessibleSelection,
    ::com::sun::star::lang::XServiceInfo > AccessibleDialogWindow_BASE;

class AccessibleDialogWindow :  public AccessibleExtendedComponentHelper_BASE,
                                public AccessibleDialogWindow_BASE,
                                public SfxListener
{
private:

    class ChildDescriptor
    {
    public:
        DlgEdObj*                                                                                   pDlgEdObj;
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >    rxAccessible;

        ChildDescriptor();
        ChildDescriptor( DlgEdObj* _pDlgEdObj );
        ChildDescriptor( DlgEdObj* _pDlgEdObj, const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& _rxAccessible );
        ~ChildDescriptor();

        ChildDescriptor( const ChildDescriptor& rDesc );
        ChildDescriptor& operator=( const ChildDescriptor& rDesc );

        bool operator==( const ChildDescriptor& rDesc );
        bool operator<( const ChildDescriptor& rDesc ) const;
    };

    typedef ::std::vector< ChildDescriptor > AccessibleChildren;

    AccessibleChildren      m_aAccessibleChildren;
    VCLExternalSolarLock*   m_pExternalLock;
    DialogWindow*           m_pDialogWindow;
    DlgEditor*              m_pDlgEditor;
    DlgEdModel*             m_pDlgEdModel;

protected:
    void                    UpdateFocused();
    void                    UpdateSelected();
    void                    UpdateBounds();

    sal_Bool                IsChildVisible( const ChildDescriptor& rDesc );

    void                    InsertChild( const ChildDescriptor& rDesc );
    void                    RemoveChild( const ChildDescriptor& rDesc );
    void                    UpdateChild( const ChildDescriptor& rDesc );
    void                    UpdateChildren();
    void                    SortChildren();

    DECL_LINK( WindowEventListener, VclSimpleEvent* );

    virtual void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    // OCommonAccessibleComponent
    virtual ::com::sun::star::awt::Rectangle SAL_CALL   implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL   disposing();

public:
    AccessibleDialogWindow( DialogWindow* pDialogWindow );
    ~AccessibleDialogWindow();

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

    // XAccessibleComponent
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAt( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleExtendedComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTitledBorderText(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getToolTipText(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL selectAllAccessible(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deselectSelectedAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
};


#endif // _BASCTL_ACCESSIBLEDIALOGWINDOW_HXX_

