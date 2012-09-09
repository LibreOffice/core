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

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABBARPAGE_HXX_
#define ACCESSIBILITY_EXT_ACCESSIBLETABBARPAGE_HXX_

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase2.hxx>
#include "accessibility/extended/accessibletabbarbase.hxx"

#include <vector>

namespace utl {
class AccessibleStateSetHelper;
}

//.........................................................................
namespace accessibility
{
//.........................................................................

    //  ----------------------------------------------------
    //  class AccessibleTabBarPage
    //  ----------------------------------------------------

    typedef ::cppu::ImplHelper2<
        ::com::sun::star::accessibility::XAccessible,
        ::com::sun::star::lang::XServiceInfo > AccessibleTabBarPage_BASE;

    class AccessibleTabBarPage :    public AccessibleTabBarBase,
                                    public AccessibleTabBarPage_BASE
    {
        friend class AccessibleTabBarPageList;

    private:
        sal_uInt16              m_nPageId;
        sal_Bool                m_bEnabled;
        sal_Bool                m_bShowing;
        sal_Bool                m_bSelected;
        OUString                m_sPageText;

        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >        m_xParent;

    protected:
        sal_Bool                IsEnabled();
        sal_Bool                IsShowing();
        sal_Bool                IsSelected();

        void                    SetEnabled( sal_Bool bEnabled );
        void                    SetShowing( sal_Bool bShowing );
        void                    SetSelected( sal_Bool bSelected );
        void                    SetPageText( const OUString& sPageText );

        sal_uInt16              GetPageId() const { return m_nPageId; }

        virtual void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

        // OCommonAccessibleComponent
        virtual ::com::sun::star::awt::Rectangle SAL_CALL   implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL   disposing();

    public:
        AccessibleTabBarPage( TabBar* pTabBar, sal_uInt16 nPageId,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent );
        virtual ~AccessibleTabBarPage();

        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

        // XAccessible
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
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
        virtual OUString SAL_CALL getTitledBorderText(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getToolTipText(  ) throw (::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace accessibility
//.........................................................................

#endif // ACCESSIBILITY_EXT_ACCESSIBLETABBARPAGE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
