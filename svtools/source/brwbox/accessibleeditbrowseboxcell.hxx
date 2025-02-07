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
#pragma once

#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <comphelper/compbase.hxx>
#include <comphelper/accessiblewrapper.hxx>
#include <vcl/accessibility/AccessibleBrowseBoxCell.hxx>


// = EditBrowseBoxTableCell

class EditBrowseBoxTableCell final : public AccessibleBrowseBoxCell
                                ,public ::comphelper::OAccessibleContextWrapperHelper
{
public:
    EditBrowseBoxTableCell(
        const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
        const css::uno::Reference< css::accessibility::XAccessible >& _rxOwningAccessible,
        const css::uno::Reference< css::accessibility::XAccessibleContext >& _xControlChild,
        ::vcl::IAccessibleTableProvider& _rBrowseBox,
        const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
        sal_Int32 _nRowPos,
        sal_uInt16 _nColPos
    );

private:
    virtual ~EditBrowseBoxTableCell() override;

    // XAccessibleComponent
    virtual sal_Int32 SAL_CALL getForeground(  ) override ;
    virtual sal_Int32 SAL_CALL getBackground(  ) override ;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    // XInterface
    DECLARE_XINTERFACE( )
    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )

    // XAccessibleContext
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual sal_Int64 SAL_CALL getAccessibleStateSet(  ) override;

    sal_Int16 SAL_CALL getAccessibleRole() override;

    virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;

    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // XComponent/OComponentProxyAggregationHelper (needs to be disambiguated)
    virtual void SAL_CALL dispose() final override;

    // OAccessibleContextWrapperHelper();
    void notifyTranslatedEvent( const css::accessibility::AccessibleEventObject& _rEvent ) override;

    EditBrowseBoxTableCell( const EditBrowseBoxTableCell& ) = delete;
    EditBrowseBoxTableCell& operator=( const EditBrowseBoxTableCell& ) = delete;
};


// = EditBrowseBoxTableCell

// XAccessible providing an EditBrowseBoxTableCell
class EditBrowseBoxTableCellAccess final
    : public comphelper::WeakComponentImplHelper<css::accessibility::XAccessible>
{
    css::uno::WeakReference< css::accessibility::XAccessibleContext >
                                        m_aContext;
    css::uno::Reference< css::accessibility::XAccessible >
                                        m_xParent;
    css::uno::Reference< css::accessibility::XAccessible >
                                        m_xControlAccessible;
    css::uno::Reference< css::awt::XWindow >
                                        m_xFocusWindow;
    ::vcl::IAccessibleTableProvider*    m_pBrowseBox;
    sal_Int32                           m_nRowPos;
    sal_uInt16                          m_nColPos;

public:
    EditBrowseBoxTableCellAccess(
        css::uno::Reference< css::accessibility::XAccessible > _xParent,
        css::uno::Reference< css::accessibility::XAccessible > _xControlAccessible,
        css::uno::Reference< css::awt::XWindow > _xFocusWindow,
        ::vcl::IAccessibleTableProvider& _rBrowseBox,
        sal_Int32 _nRowPos,
        sal_uInt16 _nColPos
    );

private:
    virtual ~EditBrowseBoxTableCellAccess() override;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // XComponent/WeakComponentImplHelper
    virtual void disposing(std::unique_lock<std::mutex>&) override;

    EditBrowseBoxTableCellAccess( const EditBrowseBoxTableCellAccess& ) = delete;
    EditBrowseBoxTableCellAccess& operator=( const EditBrowseBoxTableCellAccess& ) = delete;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
