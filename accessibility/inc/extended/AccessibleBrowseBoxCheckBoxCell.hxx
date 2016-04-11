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
#ifndef INCLUDED_ACCESSIBILITY_INC_EXTENDED_ACCESSIBLEBROWSEBOXCHECKBOXCELL_HXX
#define INCLUDED_ACCESSIBILITY_INC_EXTENDED_ACCESSIBLEBROWSEBOXCHECKBOXCELL_HXX

#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include "extended/accessiblebrowseboxcell.hxx"
#include <cppuhelper/implbase.hxx>
#include <tools/wintypes.hxx>

namespace accessibility
{

    typedef ::cppu::ImplHelper  <   css::accessibility::XAccessible,
                                    css::accessibility::XAccessibleValue
                                >   AccessibleCheckBoxCell_BASE;

    class AccessibleCheckBoxCell :   public AccessibleBrowseBoxCell
                                    ,public AccessibleCheckBoxCell_BASE
    {
    private:
        TriState m_eState;
        bool m_bIsTriState;

    protected:
        virtual ~AccessibleCheckBoxCell() override {}

        virtual ::utl::AccessibleStateSetHelper* implCreateStateSetHelper() override;

    public:
        AccessibleCheckBoxCell(const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                                ::svt::IAccessibleTableProvider& _rBrowseBox,
                                const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
                                sal_Int32 _nRowPos,
                                sal_uInt16 _nColPos,
                                const TriState& _eState,
                                bool _bIsTriState);

        // XInterface
        DECLARE_XINTERFACE( )
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessible
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getImplementationName() throw ( css::uno::RuntimeException, std::exception ) override;
        virtual ::sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;


        // XAccessibleValue
        virtual css::uno::Any SAL_CALL getCurrentValue(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL setCurrentValue( const css::uno::Any& aNumber ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getMaximumValue(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getMinimumValue(  ) throw (css::uno::RuntimeException, std::exception) override;

        // internal
        void        SetChecked( bool _bChecked );
    };
}
#endif // INCLUDED_ACCESSIBILITY_INC_EXTENDED_ACCESSIBLEBROWSEBOXCHECKBOXCELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
