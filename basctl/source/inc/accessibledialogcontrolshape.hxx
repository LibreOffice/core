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

#ifndef INCLUDED_BASCTL_SOURCE_INC_ACCESSIBLEDIALOGCONTROLSHAPE_HXX
#define INCLUDED_BASCTL_SOURCE_INC_ACCESSIBLEDIALOGCONTROLSHAPE_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/implbase3.hxx>

namespace vcl { class Window; }
class VCLExternalSolarLock;

namespace utl {
    class AccessibleStateSetHelper;
}

namespace basctl
{

class DlgEdObj;
class DialogWindow;


//  class AccessibleDialogControlShape


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
    bool                    m_bFocused;
    bool                    m_bSelected;

    ::com::sun::star::awt::Rectangle                                            m_aBounds;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xControlModel;

protected:
    bool                    IsFocused();
    bool                    IsSelected();

    void                    SetFocused (bool bFocused);
    void                    SetSelected (bool bSelected);

    ::com::sun::star::awt::Rectangle GetBounds();
    void                    SetBounds( const ::com::sun::star::awt::Rectangle& aBounds );

    vcl::Window*                 GetWindow() const;

    OUString                GetModelStringProperty( const sal_Char* pPropertyName );

    virtual void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    // OCommonAccessibleComponent
    virtual ::com::sun::star::awt::Rectangle implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL   disposing() SAL_OVERRIDE;

public:
    AccessibleDialogControlShape (DialogWindow*, DlgEdObj*);
    virtual ~AccessibleDialogControlShape();

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& rSource ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleExtendedComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getTitledBorderText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getToolTipText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_ACCESSIBLEDIALOGCONTROLSHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
