/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_IMOPTDLG_HXX
#define SC_IMOPTDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <svx/txencbox.hxx>
#include "scdllapi.h"
#include "global.hxx"

//===================================================================

class SC_DLLPUBLIC ScImportOptions
{
public:
        ScImportOptions()
            : nFieldSepCode(0), nTextSepCode(0),
            eCharSet(RTL_TEXTENCODING_DONTKNOW), bFixedWidth(sal_False),
            bSaveAsShown(sal_False), bQuoteAllText(sal_False)
        {}
        ScImportOptions( const String& rStr );

        ScImportOptions( sal_Unicode nFieldSep, sal_Unicode nTextSep, const String& rStr )
            : nFieldSepCode(nFieldSep), nTextSepCode(nTextSep), aStrFont(rStr),
            bFixedWidth(sal_False), bSaveAsShown(sal_False), bQuoteAllText(sal_False)
        { eCharSet = ScGlobal::GetCharsetValue(aStrFont); }

        ScImportOptions( sal_Unicode nFieldSep, sal_Unicode nTextSep, rtl_TextEncoding nEnc )
            : nFieldSepCode(nFieldSep), nTextSepCode(nTextSep),
            bFixedWidth(sal_False), bSaveAsShown(sal_False), bQuoteAllText(sal_False)
        { SetTextEncoding( nEnc ); }

        ScImportOptions( const ScImportOptions& rCpy )
            : nFieldSepCode     (rCpy.nFieldSepCode),
              nTextSepCode      (rCpy.nTextSepCode),
              aStrFont          (rCpy.aStrFont),
              eCharSet          (rCpy.eCharSet),
              bFixedWidth       (rCpy.bFixedWidth),
              bSaveAsShown      (rCpy.bSaveAsShown),
              bQuoteAllText     (rCpy.bQuoteAllText)
        {}

    ScImportOptions& operator=( const ScImportOptions& rCpy )
                        {
                            nFieldSepCode   = rCpy.nFieldSepCode;
                            nTextSepCode    = rCpy.nTextSepCode;
                            aStrFont        = rCpy.aStrFont;
                            eCharSet        = rCpy.eCharSet;
                            bFixedWidth     = rCpy.bFixedWidth;
                            bSaveAsShown    = rCpy.bSaveAsShown;
                            bQuoteAllText   = rCpy.bQuoteAllText;
                            return *this;
                        }

    sal_Bool             operator==( const ScImportOptions& rCmp )
                        {
                            return
                                   nFieldSepCode    == rCmp.nFieldSepCode
                                && nTextSepCode     == rCmp.nTextSepCode
                                && eCharSet         == rCmp.eCharSet
                                && aStrFont         == rCmp.aStrFont
                                && bFixedWidth      == rCmp.bFixedWidth
                                && bSaveAsShown     == rCmp.bSaveAsShown
                                && bQuoteAllText    == rCmp.bQuoteAllText;
                        }
    String  BuildString() const;

    void    SetTextEncoding( rtl_TextEncoding nEnc );

    sal_Unicode nFieldSepCode;
    sal_Unicode nTextSepCode;
    String      aStrFont;
    CharSet     eCharSet;
    sal_Bool    bFixedWidth;
    sal_Bool    bSaveAsShown;
    sal_Bool    bQuoteAllText;
};


#endif // SC_IMOPTDLG_HXX



