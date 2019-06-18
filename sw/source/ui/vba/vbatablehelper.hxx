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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBATABLEHELPER_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBATABLEHELPER_HXX

#include <com/sun/star/text/XTextTable.hpp>
#include "wordvbahelper.hxx"
#include <swtable.hxx>
#include <tabcol.hxx>

class SwVbaTableHelper
{
private:
    css::uno::Reference< css::text::XTextTable > mxTextTable;
    SwTable* pTable;

private:
    /// @throws css::uno::RuntimeException
    SwTableBox* GetTabBox( sal_Int32 nCol, sal_Int32 nRow );
    void    InitTabCols( SwTabCols& rCols, const SwTableBox *pStart );
    static sal_Int32 GetRightSeparator( SwTabCols const & rCols, sal_Int32 nNum);
    static sal_Int32 GetColCount( SwTabCols const & rCols );
    /// @throws css::uno::RuntimeException
    static sal_Int32 GetColWidth( SwTabCols& rCols, sal_Int32 nNum );

public:
    /// @throws css::uno::RuntimeException
    explicit SwVbaTableHelper( const css::uno::Reference< css::text::XTextTable >& xTextTable );
    /// @throws css::uno::RuntimeException
    sal_Int32 getTabColumnsCount( sal_Int32 nRowIndex );
    /// @throws css::uno::RuntimeException
    sal_Int32 getTabColumnsMaxCount( );
    /// @throws css::uno::RuntimeException
    sal_Int32 getTabRowIndex( const OUString& sCellName );
    /// @throws css::uno::RuntimeException
    sal_Int32 getTabColIndex( const OUString& sCellName );
    /// @throws css::uno::RuntimeException
    sal_Int32 getTableWidth( ) const;

    /// @throws css::uno::RuntimeException
    sal_Int32 GetColWidth( sal_Int32 nCol, sal_Int32 nRow = 0 );
    /// @throws css::uno::RuntimeException
    void SetColWidth( sal_Int32 _width, sal_Int32 nCol, sal_Int32 nRow = 0, bool bCurRowOnly  = false );

    /// @throws css::uno::RuntimeException
    static SwTable* GetSwTable( const css::uno::Reference< css::text::XTextTable >& xTextTable );
    static OUString getColumnStr( sal_Int32 nCol );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
