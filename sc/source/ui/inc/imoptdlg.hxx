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

#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <scdllapi.h>

class SC_DLLPUBLIC ScImportOptions
{
public:
        ScImportOptions( std::u16string_view rStr );

        ScImportOptions( sal_Unicode nFieldSep, sal_Unicode nTextSep, rtl_TextEncoding nEnc )
            : nFieldSepCode(nFieldSep), nTextSepCode(nTextSep),
            bFixedWidth(false), bSaveAsShown(false), bQuoteAllText(false),
            bSaveNumberAsSuch(true), bSaveFormulas(false), bRemoveSpace(false),
            bEvaluateFormulas(true), nSheetToExport(0)
        { SetTextEncoding( nEnc ); }

    ScImportOptions& operator=( const ScImportOptions& rCpy ) = default;

    OUString  BuildString() const;

    void    SetTextEncoding( rtl_TextEncoding nEnc );

    sal_Unicode nFieldSepCode;
    sal_Unicode nTextSepCode;
    OUString    aStrFont;
    rtl_TextEncoding eCharSet;
    bool        bFixedWidth;
    bool        bSaveAsShown;
    bool        bQuoteAllText;
    bool        bSaveNumberAsSuch;
    bool        bSaveFormulas;
    bool        bRemoveSpace;
    bool        bEvaluateFormulas;
    // "0" for 'current sheet', "-1" for all sheets (each to a separate file),
    // or 1-based specific sheet number (to a separate file).
    sal_Int32   nSheetToExport;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
