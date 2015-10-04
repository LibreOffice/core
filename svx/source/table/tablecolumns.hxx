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

#ifndef INCLUDED_SVX_SOURCE_TABLE_TABLECOLUMNS_HXX
#define INCLUDED_SVX_SOURCE_TABLE_TABLECOLUMNS_HXX

#include <com/sun/star/table/XTableColumns.hpp>
#include <cppuhelper/implbase.hxx>

#include "tablemodel.hxx"


namespace sdr { namespace table {

class TableColumns : public ::cppu::WeakAggImplHelper< css::table::XTableColumns >
{
public:
    explicit TableColumns( const TableModelRef& xTableModel );
    virtual ~TableColumns() override;

    void dispose();
    void throwIfDisposed() const throw (css::uno::RuntimeException);

    // XTableColumns
    virtual void SAL_CALL insertByIndex( sal_Int32 nIndex, sal_Int32 nCount ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByIndex( sal_Int32 nIndex, sal_Int32 nCount ) throw (css::uno::RuntimeException, std::exception) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // Methods
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException, std::exception) override;

private:
    TableModelRef   mxTableModel;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
