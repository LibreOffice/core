/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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
#ifndef SW_WORD_VBA_TABLEHELPER_HXX
#define SW_WORD_VBA_TABLEHELPER_HXX

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
    SwTableBox* GetTabBox( sal_Int32 nCol, sal_Int32 nRow ) throw (css::uno::RuntimeException);
    void    InitTabCols( SwTabCols& rCols, const SwTableBox *pStart, sal_Bool bCurRowOnly  = sal_False );
    sal_Int32 GetRightSeparator( SwTabCols& rCols, sal_Int32 nNum) const;
    sal_Int32 GetColCount( SwTabCols& rCols ) const;
    sal_Int32 GetColWidth( SwTabCols& rCols, sal_Int32 nNum ) throw (css::uno::RuntimeException);

public:
    SwVbaTableHelper( const css::uno::Reference< css::text::XTextTable >& xTextTable ) throw (css::uno::RuntimeException);
    ~SwVbaTableHelper() {}
    sal_Int32 getTabColumnsCount( sal_Int32 nRowIndex ) throw (css::uno::RuntimeException);
    sal_Int32 getTabColumnsMaxCount( ) throw (css::uno::RuntimeException);
    sal_Int32 getTabRowIndex( const rtl::OUString& sCellName ) throw (css::uno::RuntimeException);
    sal_Int32 getTabColIndex( const rtl::OUString& sCellName ) throw (css::uno::RuntimeException);
    sal_Int32 getTableWidth( ) throw (css::uno::RuntimeException);

    sal_Int32 GetColWidth( sal_Int32 nCol, sal_Int32 nRow = 0, sal_Bool bCurRowOnly  = sal_False ) throw (css::uno::RuntimeException);
    void SetColWidth( sal_Int32 _width, sal_Int32 nCol, sal_Int32 nRow = 0, sal_Bool bCurRowOnly  = sal_False ) throw (css::uno::RuntimeException);

    static SwTable* GetSwTable( const css::uno::Reference< css::text::XTextTable >& xTextTable ) throw (css::uno::RuntimeException);
    static rtl::OUString getColumnStr( sal_Int32 nCol );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
