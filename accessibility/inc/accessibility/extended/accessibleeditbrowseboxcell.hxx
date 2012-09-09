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
#ifndef ACCESSIBILITY_EXT_ACCESSIBILEEDITBROWSEBOXTABLECELL_HXX
#define ACCESSIBILITY_EXT_ACCESSIBILEEDITBROWSEBOXTABLECELL_HXX

#include "accessiblebrowseboxcell.hxx"
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/compbase1.hxx>
#include <comphelper/accessiblewrapper.hxx>

namespace accessibility
{
    // =============================================================================
    // = EditBrowseBoxTableCell
    // =============================================================================
    class EditBrowseBoxTableCell    :public AccessibleBrowseBoxCell
                                    ,public ::comphelper::OAccessibleContextWrapperHelper
    {
    public:
        EditBrowseBoxTableCell(
            const com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
            const com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxOwningAccessible,
            const com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& _xControlChild,
            ::svt::IAccessibleTableProvider& _rBrowseBox,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
            sal_Int32 _nRowPos,
            sal_uInt16 _nColPos
        );

    protected:
        virtual ~EditBrowseBoxTableCell();

    protected:
        // XAccessibleComponent
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException) ;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw ( ::com::sun::star::uno::RuntimeException );

        // XInterface
        DECLARE_XINTERFACE( )
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleContext
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);

        sal_Int16 SAL_CALL getAccessibleRole() throw ( ::com::sun::star::uno::RuntimeException );

        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    protected:
        // OComponentHelper
        virtual void SAL_CALL disposing();

        // XComponent/OComponentProxyAggregationHelper (needs to be disambiguated)
        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );

        // OAccessibleContextWrapperHelper();
        void notifyTranslatedEvent( const ::com::sun::star::accessibility::AccessibleEventObject& _rEvent ) throw (::com::sun::star::uno::RuntimeException);

    private:
        EditBrowseBoxTableCell();                                           // never implemented
        EditBrowseBoxTableCell( const EditBrowseBoxTableCell& );            // never implemented
        EditBrowseBoxTableCell& operator=( const EditBrowseBoxTableCell& ); // never implemented
    };

    // =============================================================================
    // = EditBrowseBoxTableCell
    // =============================================================================
    typedef ::cppu::WeakComponentImplHelper1    <   ::com::sun::star::accessibility::XAccessible
                                                >   EditBrowseBoxTableCellAccess_Base;
    // XAccessible providing an EditBrowseBoxTableCell
    class EditBrowseBoxTableCellAccess
                        :public ::comphelper::OBaseMutex
                        ,public EditBrowseBoxTableCellAccess_Base
    {
    protected:
        ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessibleContext >
                                            m_aContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                            m_xParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                            m_xControlAccessible;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                                            m_xFocusWindow;
        ::svt::IAccessibleTableProvider*    m_pBrowseBox;
        sal_Int32                           m_nRowPos;
        sal_uInt16                          m_nColPos;

    public:
        EditBrowseBoxTableCellAccess(
            const ::com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessible >& _rxParent,
            const ::com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessible > _xControlAccessible,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
            ::svt::IAccessibleTableProvider& _rBrowseBox,
            sal_Int32 _nRowPos,
            sal_uInt16 _nColPos
        );

    protected:
        virtual ~EditBrowseBoxTableCellAccess();

        // XAccessible
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent/OComponentHelper
        virtual void SAL_CALL disposing();

    private:
        EditBrowseBoxTableCellAccess();                                                 // never implemented
        EditBrowseBoxTableCellAccess( const EditBrowseBoxTableCellAccess& );            // never implemented
        EditBrowseBoxTableCellAccess& operator=( const EditBrowseBoxTableCellAccess& ); // never implemented
    };
}

#endif // ACCESSIBILITY_EXT_ACCESSIBILEEDITBROWSEBOXTABLECELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
