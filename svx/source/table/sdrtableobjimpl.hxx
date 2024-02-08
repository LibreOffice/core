/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <memory>
#include <vector>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XModifyListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <sal/types.h>
#include <svx/svdotable.hxx>
#include <svx/svxdllapi.h>

#include <celltypes.hxx>

namespace sdr::table {

class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SdrTableObjImpl : public ::cppu::WeakImplHelper< css::util::XModifyListener >
{
public:
    CellRef mxActiveCell;
    TableModelRef mxTable;
    SdrTableObj* mpTableObj;
    std::unique_ptr<TableLayouter> mpLayouter;
    CellPos maEditPos;
    TableStyleSettings maTableStyle;
    css::uno::Reference< css::container::XIndexAccess > mxTableStyle;
    std::vector<std::unique_ptr<SdrUndoAction>> maUndos;
    bool mbSkipChangeLayout;

    void CropTableModelToSelection(const CellPos& rStart, const CellPos& rEnd);

    CellRef getCell( const CellPos& rPos ) const;
    void LayoutTable( tools::Rectangle& rArea, bool bFitWidth, bool bFitHeight );

    void ApplyCellStyles();
    void UpdateCells( tools::Rectangle const & rArea );

    SdrTableObjImpl();
    virtual ~SdrTableObjImpl() override;

    void init( SdrTableObj* pTable, sal_Int32 nColumns, sal_Int32 nRows );
    void dispose();

    sal_Int32 getColumnCount() const;
    /// Get widths of the columns in the table.
    std::vector<sal_Int32> getColumnWidths() const;
    sal_Int32 getRowCount() const;

    void DragEdge( bool mbHorizontal, int nEdge, sal_Int32 nOffset );

    SdrTableObjImpl& operator=( const SdrTableObjImpl& rSource );

    // XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    void update();

    void connectTableStyle();
    void disconnectTableStyle();
    bool isInUse();
    void dumpAsXml(xmlTextWriterPtr pWriter) const;
private:
    static SdrTableObjImpl* lastLayoutTable;
    static tools::Rectangle lastLayoutInputRectangle;
    static tools::Rectangle lastLayoutResultRectangle;
    static bool lastLayoutFitWidth;
    static bool lastLayoutFitHeight;
    static css::text::WritingMode lastLayoutMode;
    static sal_Int32 lastRowCount;
    static sal_Int32 lastColCount;
    static std::vector<sal_Int32> lastColWidths;
    static bool rowSizeChanged;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
