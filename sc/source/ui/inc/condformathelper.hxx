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
            const OUString& aStr1 = OUString(), const OUString& aStr2 = OUString() );
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
