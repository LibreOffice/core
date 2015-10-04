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

#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEICONCHOICECTRL_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEICONCHOICECTRL_HXX

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/implbase.hxx>
#include <vcl/vclevent.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>

// class AccessibleListBox -----------------------------------------------

class SvtIconChoiceCtrl;

namespace accessibility
{
    typedef ::cppu::ImplHelper <  css::accessibility::XAccessible
                                , css::accessibility::XAccessibleSelection> AccessibleIconChoiceCtrl_BASE;

    /** the class OAccessibleListBoxEntry represents the base class for an accessible object of a listbox entry
    */
    class AccessibleIconChoiceCtrl  :public AccessibleIconChoiceCtrl_BASE
                                    ,public VCLXAccessibleComponent
    {
    protected:
        css::uno::Reference< css::accessibility::XAccessible > m_xParent;

    protected:
        virtual ~AccessibleIconChoiceCtrl();

        /** this function is called upon disposing the component */
        virtual void SAL_CALL   disposing() override;

        virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
        virtual void    FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet ) override;

        VclPtr< SvtIconChoiceCtrl > getCtrl();

    public:
        /** OAccessibleBase needs a valid view
            @param  _rIconCtrl
                is the box for which we implement an accessible object
            @param  _xParent
                is our parent accessible object
        */
        AccessibleIconChoiceCtrl( SvtIconChoiceCtrl& _rIconCtrl,
                                  const css::uno::Reference< css::accessibility::XAccessible >& _xParent );

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XInterface
        DECLARE_XINTERFACE()

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

        // XServiceInfo - static methods
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw(css::uno::RuntimeException);
        static OUString getImplementationName_Static() throw(css::uno::RuntimeException);

        // XAccessible
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleSelection
        void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        void SAL_CALL clearAccessibleSelection(  ) throw (css::uno::RuntimeException, std::exception) override;
        void SAL_CALL selectAllAccessibleChildren(  ) throw (css::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    };


}// namespace accessibility


#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEICONCHOICECTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
