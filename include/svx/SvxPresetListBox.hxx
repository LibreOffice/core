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

#include <svtools/valueset.hxx>
#include <svx/xtable.hxx>
#include <tools/gen.hxx>

class SVXCORE_DLLPUBLIC SvxPresetListBox : public ValueSet
{
private:
    static constexpr sal_uInt32  nColCount = 3;
    Size                         aIconSize;
    sal_uInt16 mnContextMenuItemId;
    Link<SvxPresetListBox*,void> maRenameHdl;
    Link<SvxPresetListBox*,void> maDeleteHdl;

    void OnMenuItemSelected(std::u16string_view rIdent);

    template< typename ListType, typename EntryType >
    void FillPresetListBoxImpl(ListType& pList, sal_uInt32 nStartIndex);

public:
    SvxPresetListBox(std::unique_ptr<weld::ScrolledWindow> pWindow);

    virtual void Resize() override;
    virtual bool Command(const CommandEvent& rEvent) override;
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    Size const & GetIconSize() const { return aIconSize; }
    sal_uInt16 GetContextMenuItemId() const { return mnContextMenuItemId; }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
