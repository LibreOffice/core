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


#ifndef _ASCFLDLG_HXX
#define _ASCFLDLG_HXX
#include <vcl/fixed.hxx>

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/lstbox.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/txencbox.hxx>
#include <svx/langbox.hxx>

class SwAsciiOptions;
class SvStream;
class SwDocShell;

class SwAsciiFilterDlg : public SfxModalDialog
{
    FixedLine           aFL;
    FixedText           aCharSetFT;
    SvxTextEncodingBox  aCharSetLB;
    FixedText           aFontFT;
    ListBox             aFontLB;
    FixedText           aLanguageFT;
    SvxLanguageBox      aLanguageLB;
    FixedText           aCRLF_FT;
    RadioButton         aCRLF_RB, aCR_RB, aLF_RB;
    OKButton            aOkPB;
    CancelButton        aCancelPB;
    HelpButton          aHelpPB;
    String              sSystemCharSet;
    sal_Bool                bSaveLineStatus;

    DECL_LINK( CharSetSelHdl, SvxTextEncodingBox* );
    DECL_LINK( LineEndHdl, RadioButton* );
    void SetCRLF( LineEnd eEnd );
    LineEnd GetCRLF() const;

public:
    // CTOR:    for import - pStream is the inputstream
    //          for export - pStream must be 0
    SwAsciiFilterDlg( Window* pParent, SwDocShell& rDocSh,
                        SvStream* pStream );
    virtual ~SwAsciiFilterDlg();

    void FillOptions( SwAsciiOptions& rOptions );
};



#endif

