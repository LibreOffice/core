/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

    rtl::OUString               maPreviewLine[ CSV_PREVIEW_LINES ];

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
    String                      aTextSepList;
    String                      maFieldSeparators;  // selected field separators
    sal_Unicode                 mcTextSep;
    String                      maStrTextToColumns;

    CharSet                     meCharSet;          /// Selected char set.
    bool                        mbCharSetSystem;    /// Is System char set selected?
    ScImportAsciiCall           meCall;             /// How the dialog is called (see asciiopt.hxx)

public:
                                ScImportAsciiDlg(
                                    Window* pParent, String aDatName,
                                    SvStream* pInStream, ScImportAsciiCall eCall );
                                ~ScImportAsciiDlg();

    void                        GetOptions( ScAsciiOptions& rOpt );
    void                        SaveParameters();

private:
    /** Sets the selected char set data to meCharSet and mbCharSetSystem. */
    void                        SetSelectedCharSet();
    /** Set separators in ui from maFieldSeparators    */
    void                        SetSeparators();
    /** Returns all separator characters in a string. */
    String                      GetSeparators() const;

    /** Enables or disables all separator checkboxes and edit fields. */
    void                        SetupSeparatorCtrls();


    bool                        GetLine( sal_uLong nLine, rtl::OUString &rText );
    void                        UpdateVertical();
    inline bool                 Seek( sal_uLong nPos ); // synced to and from mnStreamPos

                                DECL_LINK( CharSetHdl, SvxTextEncodingBox* );
                                DECL_LINK( FirstRowHdl, NumericField* );
                                DECL_LINK( RbSepFixHdl, RadioButton* );
                                DECL_LINK( SeparatorHdl, Control* );
                                DECL_LINK( LbColTypeHdl, ListBox* );
                                DECL_LINK(UpdateTextHdl, void *);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
