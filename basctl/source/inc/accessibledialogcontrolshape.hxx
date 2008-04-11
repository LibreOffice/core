/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessibledialogcontrolshape.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BASCTL_ACCESSIBLEDIALOGCONTROLSHAPE_HXX_
#define _BASCTL_ACCESSIBLEDIALOGCONTROLSHAPE_HXX_

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <vector>

class Window;
class DialogWindow;
class DlgEdObj;
class VCLExternalSolarLock;

namespace utl {
class AccessibleStateSetHelper;
}


//  ----------------------------------------------------
//  class AccessibleDialogControlShape
//  ----------------------------------------------------

typedef ::comphelper::OAccessibleExtendedComponentHelper    AccessibleExtendedComponentHelper_BASE;

typedef ::cppu::ImplHelper3<
    ::com::sun::star::accessibility::XAccessible,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::beans::XPropertyChangeListener > AccessibleDialogControlShape_BASE;

class AccessibleDialogControlShape :    public AccessibleExtendedComponentHelper_BASE,
                                        public AccessibleDialogControlShape_BASE
{
    friend class AccessibleDialogWindow;

private:
    VCLExternalSolarLock*   m_pExternalLock;
    DialogWindow*           m_pDialogWindow;
    DlgEdObj*               m_pDlgEdObj;
    sal_Bool                m_bFocused;
    sal_Bool                m_bSelected;

    ::com::sun::star::awt::Rectangle                                            m_aBounds;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xControlModel;

protected:
    sal_Bool                IsFocused();
    sal_Bool                IsSelected();

    void                    SetFocused( sal_Bool bFocused );
    void                    SetSelected( sal_Bool bSelected );

    ::com::sun::star::awt::Rectangle GetBounds();
    void                    SetBounds( const ::com::sun::star::awt::Rectangle& aBounds );

    Window*                 GetWindow() const;

    ::rtl::OUString         GetModelStringProperty( const sal_Char* pPropertyName );

    virtual void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    // OCommonAccessibleComponent
    virtual ::com::sun::star::awt::Rectangle SAL_CALL   implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL   disposing();

public:
    AccessibleDialogControlShape( DialogWindow* pDialogWindow, DlgEdObj* pDlgEdObj );
    virtual ~AccessibleDialogControlShape();

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& rSource ) throw (::com::sun::star::uno::RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

    // XAccessibleComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleExtendedComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTitledBorderText(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getToolTipText(  ) throw (::com::sun::star::uno::RuntimeException);
};


#endif // _BASCTL_ACCESSIBLEDIALOGCONTROLSHAPE_HXX_
