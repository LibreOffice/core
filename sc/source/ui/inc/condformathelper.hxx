/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <address.hxx>
#include <svx/fntctrl.hxx>

class ScConditionalFormat;
class ScFormatEntry;

enum ScCondFormatEntryType
{
    CONDITION,
    COLORSCALE,
    DATABAR,
    FORMULA,
    ICONSET,
    DATE
};

class ScCondFormatHelper
{
public:
    static SC_DLLPUBLIC OUString GetExpression(const ScFormatEntry* rEntry, const ScAddress& rPos);
    static SC_DLLPUBLIC OUString GetExpression(const ScConditionalFormat& rFormat, const ScAddress& rPos);

    static SC_DLLPUBLIC OUString GetExpression( ScCondFormatEntryType eType, sal_Int32 nIndex,
            std::u16string_view aStr1 = std::u16string_view(), std::u16string_view aStr2 = std::u16string_view() );
    static SC_DLLPUBLIC void StyleSelect(weld::Window* pDialogParent, weld::ComboBox& rLbStyle,
                                         const ScDocument* pDoc, SvxFontPrevWindow& rWdPreview);
    static SC_DLLPUBLIC void FillStyleListBox(const ScDocument* pDocument, weld::ComboBox& rCombo);
    static SC_DLLPUBLIC void UpdateStyleList(weld::ComboBox& rLbStyle, const ScDocument* pDoc);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
