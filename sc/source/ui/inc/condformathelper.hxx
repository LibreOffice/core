/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
#include "conditio.hxx"

#include "global.hxx"

enum ScCondFormatEntryType
{
    CONDITION,
    COLORSCALE,
    DATABAR,
    FORMULA,
    ICONSET,
    COLLAPSED
};

class ScCondFormatHelper
{
public:
    static SC_DLLPUBLIC rtl::OUString GetExpression(const ScConditionalFormat& rFormat, const ScAddress& rPos);

    static SC_DLLPUBLIC rtl::OUString GetExpression( ScCondFormatEntryType eType, sal_Int32 nIndex,
            rtl::OUString aStr1 = rtl::OUString(), rtl::OUString aStr2 = rtl::OUString() );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
