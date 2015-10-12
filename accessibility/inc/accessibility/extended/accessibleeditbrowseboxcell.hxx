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
#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEEDITBROWSEBOXCELL_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEEDITBROWSEBOXCELL_HXX

#include "accessiblebrowseboxcell.hxx"
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/accessiblewrapper.hxx>

namespace accessibility
{

    // = EditBrowseBoxTableCell

    class EditBrowseBoxTableCell    :public AccessibleBrowseBoxCell
                                    ,public ::comphelper::OAccessibleContextWrapperHelper
    {
    public:
        EditBrowseBoxTableCell(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxOwningAccessible,
            const css::uno::Reference< css::accessibility::XAccessibleContext >& _xControlChild,
            ::svt::IAccessibleTableProvider& _rBrowseBox,
            const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
            sal_Int32 _nRowPos,
            sal_uInt16 _nColPos
        );

    protected:
        virtual ~EditBrowseBoxTableCell();

    protected:
        // XAccessibleComponent
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override ;
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override ;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw ( css::uno::RuntimeException, std::exception ) override;

        // XInterface
        DECLARE_XINTERFACE( )
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleContext
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (css::uno::RuntimeException, std::exception) override;

        sal_Int16 SAL_CALL getAccessibleRole() throw ( css::uno::RuntimeException, std::exception ) override;

        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;
    protected:
        // OComponentHelper
        virtual void SAL_CALL disposing() override;

        // XComponent/OComponentProxyAggregationHelper (needs to be disambiguated)
        virtual void SAL_CALL dispose() throw( css::uno::RuntimeException, std::exception ) override;

        // OAccessibleContextWrapperHelper();
        void notifyTranslatedEvent( const css::accessibility::AccessibleEventObject& _rEvent ) throw (css::uno::RuntimeException) override;

    private:
        EditBrowseBoxTableCell( const EditBrowseBoxTableCell& ) = delete;
        EditBrowseBoxTableCell& operator=( const EditBrowseBoxTableCell& ) = delete;
    };


    // = EditBrowseBoxTableCell

    typedef ::cppu::WeakComponentImplHelper<   css::accessibility::XAccessible
                                           >   EditBrowseBoxTableCellAccess_Base;
    // XAccessible providing an EditBrowseBoxTableCell
    class EditBrowseBoxTableCellAccess
                        :public ::comphelper::OBaseMutex
                        ,public EditBrowseBoxTableCellAccess_Base
    {
    protected:
        css::uno::WeakReference< css::accessibility::XAccessibleContext >
                                            m_aContext;
        css::uno::Reference< css::accessibility::XAccessible >
                                            m_xParent;
        css::uno::Reference< css::accessibility::XAccessible >
                                            m_xControlAccessible;
        css::uno::Reference< css::awt::XWindow >
                                            m_xFocusWindow;
        ::svt::IAccessibleTableProvider*    m_pBrowseBox;
        sal_Int32                           m_nRowPos;
        sal_uInt16                          m_nColPos;

    public:
        EditBrowseBoxTableCellAccess(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxControlAccessible,
            const css::uno::Reference< css::awt::XWindow >& _rxFocusWindow,
            ::svt::IAccessibleTableProvider& _rBrowseBox,
            sal_Int32 _nRowPos,
            sal_uInt16 _nColPos
        );

    protected:
        virtual ~EditBrowseBoxTableCellAccess();

        // XAccessible
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XComponent/OComponentHelper
        virtual void SAL_CALL disposing() override;

    private:
        EditBrowseBoxTableCellAccess( const EditBrowseBoxTableCellAccess& ) = delete;
        EditBrowseBoxTableCellAccess& operator=( const EditBrowseBoxTableCellAccess& ) = delete;
    };
}

#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEEDITBROWSEBOXCELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
