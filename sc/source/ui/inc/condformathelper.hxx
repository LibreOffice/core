/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_CONDFORMATHELPER_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CONDFORMATHELPER_HXX

#include <rtl/ustring.hxx>
#include <address.hxx>

class ScConditionalFormat;

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
    static SC_DLLPUBLIC OUString GetExpression(const ScConditionalFormat& rFormat, const ScAddress& rPos);

    static SC_DLLPUBLIC OUString GetExpression( ScCondFormatEntryType eType, sal_Int32 nIndex,
            std::u16string_view aStr1 = std::u16string_view(), std::u16string_view aStr2 = std::u16string_view() );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
