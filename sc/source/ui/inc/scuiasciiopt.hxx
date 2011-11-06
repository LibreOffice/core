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



// ============================================================================

#ifndef SCUI_ASCIIOPT_HXX
#define SCUI_ASCIIOPT_HXX


#include "asciiopt.hxx"
#include "svx/langbox.hxx"

// ============================================================================

class ScImportAsciiDlg : public ModalDialog
{
    SvStream*                   mpDatStream;
    sal_uLong                       mnStreamPos;
    sal_uLong*                      mpRowPosArray;
    sal_uLong                       mnRowPosCount;

    String                      maPreviewLine[ CSV_PREVIEW_LINES ];

    FixedLine                   aFlFieldOpt;
    FixedText                   aFtCharSet;
    SvxTextEncodingBox          aLbCharSet;
    FixedText                   aFtCustomLang;
    SvxLanguageBox              aLbCustomLang;

    FixedText                   aFtRow;
    NumericField                aNfRow;

    FixedLine                   aFlSepOpt;
    RadioButton                 aRbFixed;
    RadioButton                 aRbSeparated;

    CheckBox                    aCkbTab;
    CheckBox                    aCkbSemicolon;
    CheckBox                    aCkbComma;
    CheckBox                    aCkbSpace;
    CheckBox                    aCkbOther;
    Edit                        aEdOther;
    CheckBox                    aCkbAsOnce;

    FixedLine                   aFlOtherOpt;

    FixedText                   aFtTextSep;
    ComboBox                    aCbTextSep;

    CheckBox                    aCkbQuotedAsText;
    CheckBox                    aCkbDetectNumber;

    FixedLine                   aFlWidth;
    FixedText                   aFtType;
    ListBox                     aLbType;

    ScCsvTableBox               maTableBox;

    OKButton                    aBtnOk;
    CancelButton                aBtnCancel;
    HelpButton                  aBtnHelp;

    String                      aCharSetUser;
    String                      aColumnUser;
    String                      aFldSepList;
    String                      aTextSepList;
    String                      maFieldSeparators;  // selected field separators
    sal_Unicode                 mcTextSep;
    String                      maStrTextToColumns;

    CharSet                     meCharSet;          /// Selected char set.
    bool                        mbCharSetSystem;    /// Is System char set selected?
    bool                        mbFileImport;       /// Is this dialog involked for csv file import ?

public:
                                ScImportAsciiDlg(
                                    Window* pParent, String aDatName,
                                    SvStream* pInStream, sal_Unicode cSep = '\t' );
                                ~ScImportAsciiDlg();

    void                        GetOptions( ScAsciiOptions& rOpt );
    void                        SetTextToColumnsMode();
    void                        SaveParameters();

private:
    /** Sets the selected char set data to meCharSet and mbCharSetSystem. */
    void                        SetSelectedCharSet();
    /** Returns all separator characters in a string. */
    String                      GetSeparators() const;

    /** Enables or disables all separator checkboxes and edit fields. */
    void                        SetupSeparatorCtrls();


    bool                        GetLine( sal_uLong nLine, String &rText );
    void                        UpdateVertical();
    inline bool                 Seek( sal_uLong nPos ); // synced to and from mnStreamPos

                                DECL_LINK( CharSetHdl, SvxTextEncodingBox* );
                                DECL_LINK( FirstRowHdl, NumericField* );
                                DECL_LINK( RbSepFixHdl, RadioButton* );
                                DECL_LINK( SeparatorHdl, Control* );
                                DECL_LINK( LbColTypeHdl, ListBox* );
                                DECL_LINK( UpdateTextHdl, ScCsvTableBox* );
                                DECL_LINK( ColTypeHdl, ScCsvTableBox* );
};


inline bool ScImportAsciiDlg::Seek(sal_uLong nPos)
{
    bool bSuccess = true;
    if (nPos != mnStreamPos && mpDatStream)
    {
        if (mpDatStream->Seek( nPos ) != nPos)
            bSuccess = false;
        else
            mnStreamPos = nPos;
    }
    return bSuccess;
}

#endif

