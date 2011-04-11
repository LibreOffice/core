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
#ifndef _TABLEMGR_HXX
#define _TABLEMGR_HXX

#include "swdllapi.h"
#include "swtypes.hxx"
#include "tabcol.hxx"

class SwFrmFmt;
class SwWrtShell;
class Window;
class SfxItemSet;
class SwFlyFrmFmt;

namespace com { namespace sun { namespace star {
    namespace frame {
        class XModel; }
    namespace chart2 {
    namespace data {
        class XDataProvider; } }
}}}

const SwTwips lAutoWidth = INVALID_TWIPS;
const char cParaDelim = 0x0a;

class SW_DLLPUBLIC SwTableFUNC
{
    SwFrmFmt    *pFmt;
    SwWrtShell  *pSh;
    sal_Bool        bCopy;
    SwTabCols   aCols;

private:
    SW_DLLPRIVATE int GetRightSeparator(int nNum) const;

public:
    inline SwTableFUNC(SwFrmFmt &);
           SwTableFUNC(SwWrtShell *pShell, sal_Bool bCopyFmt = sal_False);
           ~SwTableFUNC();

    void    InitTabCols();
    void    ColWidthDlg(Window *pParent );
    SwTwips GetColWidth(sal_uInt16 nNum) const;
    SwTwips GetMaxColWidth(sal_uInt16 nNum) const;
    void    SetColWidth(sal_uInt16 nNum, SwTwips nWidth );
    sal_uInt16  GetColCount() const;
    sal_uInt16  GetCurColNum() const;

    sal_Bool IsTableSelected() const { return pFmt != 0; }

    const SwFrmFmt *GetTableFmt() const { return pFmt; }

    SwWrtShell* GetShell() const { return pSh; }

    // @deprecated
    void UpdateChart();

    /// @return the XModel of the newly inserted chart if successfull
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        InsertChart( ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider > &rxDataProvider, sal_Bool bFillWithData, const rtl::OUString &rCellRange, SwFlyFrmFmt** ppFlyFrmFmt = 0 );
};

inline SwTableFUNC::SwTableFUNC(SwFrmFmt &rFmt) :
    pFmt(&rFmt),
    pSh(0),
    bCopy(sal_False)
{
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
