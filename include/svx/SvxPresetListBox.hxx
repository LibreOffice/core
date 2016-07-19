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

#ifndef INCLUDED_SVX_SVXPRESETLISTBOX_HXX
#define INCLUDED_SVX_SVXPRESETLISTBOX_HXX

#include <svtools/svtdllapi.h>
#include <svx/XPropertyTable.hxx>
#include <vcl/ctrl.hxx>
#include <svtools/valueset.hxx>
#include <svx/xtable.hxx>
#include <tools/gen.hxx>

class SVX_DLLPUBLIC SvxPresetListBox : public ValueSet
{
private:
    sal_uInt32 nColCount;
    sal_uInt32 nRowCount;
    Size       aIconSize;
    Link<SvxPresetListBox*,void> maRenameHdl;
    Link<SvxPresetListBox*,void> maDeleteHdl;

    DECL_LINK_TYPED( OnMenuItemSelected, Menu*, bool );

    template< typename ListType, typename EntryType >
    void FillPresetListBoxImpl(ListType& pList, sal_uInt32 nStartIndex = 1);

public:
    SvxPresetListBox(vcl::Window* pParent, WinBits nWinStyle = WB_ITEMBORDER);

    virtual void Resize() override;
    virtual void Command( const CommandEvent& rEvt ) override;
    sal_uInt32 getColumnCount() const { return nColCount; }
    Size const & GetIconSize() const { return aIconSize; }

    void setColumnCount( const sal_uInt32 nCount ) { nColCount = nCount; }
    void setRowCount( const sal_uInt32 nRow ) { nRowCount = nRow; }
    void SetRenameHdl( const Link<SvxPresetListBox*,void>& rLink )
    {
        maRenameHdl = rLink;
    }
    void SetDeleteHdl( const Link<SvxPresetListBox*,void>& rLink )
    {
        maDeleteHdl = rLink;
    }

    void FillPresetListBox(XGradientList& pList, sal_uInt32 nStartIndex = 1);
    void FillPresetListBox(XHatchList& pList, sal_uInt32 nStartIndex = 1);
    void FillPresetListBox(XBitmapList& pList, sal_uInt32 nStartIndex = 1);
    void FillPresetListBox(XPatternList& pList, sal_uInt32 nStartIndex = 1);
    void DrawLayout();

};

#endif // INCLUDED_SVX_SVXPRESETLISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
