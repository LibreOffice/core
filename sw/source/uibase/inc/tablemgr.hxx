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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_TABLEMGR_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_TABLEMGR_HXX

#include "swdllapi.h"
#include "swtypes.hxx"
#include "tabcol.hxx"

class SwFrameFormat;
class SwWrtShell;
namespace vcl { class Window; }
class SwFlyFrameFormat;

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
    SwFrameFormat    *pFormat;
    SwWrtShell  *pSh;
    bool        bCopy;
    SwTabCols   aCols;

private:
    SAL_DLLPRIVATE int GetRightSeparator(int nNum) const;

public:
    inline SwTableFUNC(SwFrameFormat &);
           SwTableFUNC(SwWrtShell *pShell, bool bCopyFormat = false);
           ~SwTableFUNC();

    void    InitTabCols();
    void    ColWidthDlg(vcl::Window *pParent );
    SwTwips GetColWidth(sal_uInt16 nNum) const;
    SwTwips GetMaxColWidth(sal_uInt16 nNum) const;
    void    SetColWidth(sal_uInt16 nNum, SwTwips nWidth );
    sal_uInt16  GetColCount() const;
    sal_uInt16  GetCurColNum() const;

    SwWrtShell* GetShell() const { return pSh; }

    // @deprecated
    void UpdateChart();

    /// @return the XModel of the newly inserted chart if successful
    css::uno::Reference< css::frame::XModel >
        InsertChart( css::uno::Reference< css::chart2::data::XDataProvider > &rxDataProvider, bool bFillWithData, const OUString &rCellRange, SwFlyFrameFormat** ppFlyFrameFormat = nullptr );
};

inline SwTableFUNC::SwTableFUNC(SwFrameFormat &rFormat) :
    pFormat(&rFormat),
    pSh(nullptr),
    bCopy(false)
{
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
