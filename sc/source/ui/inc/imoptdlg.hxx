/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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



