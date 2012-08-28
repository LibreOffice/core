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
#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXCHECKBOXCELL_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXCHECKBOXCELL_HXX

#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include "accessibility/extended/accessiblebrowseboxcell.hxx"
#include <cppuhelper/implbase2.hxx>
#include <tools/wintypes.hxx>
// ============================================================================
namespace accessibility
{
// ============================================================================
    typedef ::cppu::ImplHelper2 <   ::com::sun::star::accessibility::XAccessible,
                                    ::com::sun::star::accessibility::XAccessibleValue
                                >   AccessibleCheckBoxCell_BASE;

    class AccessibleCheckBoxCell :   public AccessibleBrowseBoxCell
                                    ,public AccessibleCheckBoxCell_BASE
    {
    private:
        TriState m_eState;
        sal_Bool m_bIsTriState;

    protected:
        virtual ~AccessibleCheckBoxCell() {}

        virtual ::utl::AccessibleStateSetHelper* implCreateStateSetHelper();

    public:
        AccessibleCheckBoxCell(const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
                                ::svt::IAccessibleTableProvider& _rBrowseBox,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
                                sal_Int32 _nRowPos,
                                sal_uInt16 _nColPos,
                                const TriState& _eState,
                                sal_Bool _bIsTriState);

        // XInterface
        DECLARE_XINTERFACE( )
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessible
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw ( ::com::sun::star::uno::RuntimeException );
        virtual ::sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);


        // XAccessibleValue
        virtual ::com::sun::star::uno::Any SAL_CALL getCurrentValue(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL setCurrentValue( const ::com::sun::star::uno::Any& aNumber ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getMaximumValue(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getMinimumValue(  ) throw (::com::sun::star::uno::RuntimeException);

        // internal
        void        SetChecked( sal_Bool _bChecked );
    };
}
#endif // ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXCHECKBOXCELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
