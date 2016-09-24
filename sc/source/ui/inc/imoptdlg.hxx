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

#ifndef INCLUDED_SC_SOURCE_UI_INC_IMOPTDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_IMOPTDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <svx/txencbox.hxx>
#include "scdllapi.h"
#include "global.hxx"

class SC_DLLPUBLIC ScImportOptions
{
public:
        ScImportOptions( const OUString& rStr );

        ScImportOptions( sal_Unicode nFieldSep, sal_Unicode nTextSep, rtl_TextEncoding nEnc )
            : nFieldSepCode(nFieldSep), nTextSepCode(nTextSep),
            bFixedWidth(false), bSaveAsShown(false), bQuoteAllText(false), bSaveFormulas(false)
        { SetTextEncoding( nEnc ); }

    ScImportOptions& operator=( const ScImportOptions& rCpy )
                        {
                            nFieldSepCode   = rCpy.nFieldSepCode;
                            nTextSepCode    = rCpy.nTextSepCode;
                            aStrFont        = rCpy.aStrFont;
                            eCharSet        = rCpy.eCharSet;
                            bFixedWidth     = rCpy.bFixedWidth;
                            bSaveAsShown    = rCpy.bSaveAsShown;
                            bQuoteAllText   = rCpy.bQuoteAllText;
                            bSaveFormulas   = rCpy.bSaveFormulas;
                            return *this;
                        }

    OUString  BuildString() const;

    void    SetTextEncoding( rtl_TextEncoding nEnc );

    sal_Unicode nFieldSepCode;
    sal_Unicode nTextSepCode;
    OUString    aStrFont;
    rtl_TextEncoding eCharSet;
    bool        bFixedWidth;
    bool        bSaveAsShown;
    bool        bQuoteAllText;
    bool        bSaveFormulas;
};

#endif // INCLUDED_SC_SOURCE_UI_INC_IMOPTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
