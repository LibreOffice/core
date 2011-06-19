/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
        sal_Bool m_bEnabled;
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
                                sal_Bool _bEnabled,
                                sal_Bool _bIsTriState = sal_True);

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
