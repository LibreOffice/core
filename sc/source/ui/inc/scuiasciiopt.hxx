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

#include <svx/langbox.hxx>
#include <tools/solar.h>
#include <tools/stream.hxx>
#include <vcl/weld.hxx>

#include "asciiopt.hxx"

class ScCsvTableBox;
class SvxTextEncodingBox;

class ScImportAsciiDlg : public weld::GenericDialogController
{
    SvStream*                   mpDatStream;
    sal_uLong                       mnStreamPos;
    std::unique_ptr<sal_uLong[]>    mpRowPosArray;
    sal_uLong                       mnRowPosCount;

    OUString               maPreviewLine[ CSV_PREVIEW_LINES ];

    OUString                    maFieldSeparators;  // selected field separators
    sal_Unicode                 mcTextSep;

    rtl_TextEncoding            meCharSet;          /// Selected char set.
    bool                        mbCharSetSystem;    /// Is System char set selected?
    ScImportAsciiCall           meCall;             /// How the dialog is called (see asciiopt.hxx)
    bool                        mbDetectSpaceSep;   /// Whether to detect a possible space separator.

    std::unique_ptr<weld::Label> mxFtCharSet;
    std::unique_ptr<SvxTextEncodingBox> mxLbCharSet;
    std::unique_ptr<weld::Label> mxFtCustomLang;
    std::unique_ptr<SvxLanguageBox> mxLbCustomLang;

    std::unique_ptr<weld::Label> mxFtRow;
    std::unique_ptr<weld::SpinButton> mxNfRow;

    std::unique_ptr<weld::RadioButton> mxRbFixed;
    std::unique_ptr<weld::RadioButton> mxRbSeparated;

    std::unique_ptr<weld::CheckButton> mxCkbTab;
    std::unique_ptr<weld::CheckButton> mxCkbSemicolon;
    std::unique_ptr<weld::CheckButton> mxCkbComma;
    std::unique_ptr<weld::CheckButton> mxCkbRemoveSpace;
    std::unique_ptr<weld::CheckButton> mxCkbSpace;
    std::unique_ptr<weld::CheckButton> mxCkbOther;
    std::unique_ptr<weld::Entry> mxEdOther;
    std::unique_ptr<weld::CheckButton> mxCkbAsOnce;

    std::unique_ptr<weld::Label> mxFtTextSep;
    std::unique_ptr<weld::ComboBox> mxCbTextSep;

    std::unique_ptr<weld::CheckButton> mxCkbQuotedAsText;
    std::unique_ptr<weld::CheckButton> mxCkbDetectNumber;
    std::unique_ptr<weld::CheckButton> mxCkbSkipEmptyCells;

    std::unique_ptr<weld::Label> mxFtType;
    std::unique_ptr<weld::ComboBox> mxLbType;
    std::unique_ptr<weld::Label> mxAltTitle;

    std::unique_ptr<ScCsvTableBox> mxTableBox;

public:
                                ScImportAsciiDlg(
                                    weld::Window* pParent, const OUString& aDatName,
                                    SvStream* pInStream, ScImportAsciiCall eCall );
                                virtual ~ScImportAsciiDlg() override;

    void                        GetOptions( ScAsciiOptions& rOpt );
    void                        SaveParameters();

private:
    /** Sets the selected char set data to meCharSet and mbCharSetSystem. */
    void                        SetSelectedCharSet();
    /** Set separators in ui from maFieldSeparators    */
    void                        SetSeparators();
    /** Returns all separator characters in a string. */
    OUString                    GetSeparators() const;

    /** Enables or disables all separator checkboxes and edit fields. */
    void                        SetupSeparatorCtrls();

    bool                        GetLine( sal_uLong nLine, OUString &rText, sal_Unicode& rcDetectSep );
    void                        UpdateVertical();
    inline bool                 Seek( sal_uLong nPos ); // synced to and from mnStreamPos

                                DECL_LINK( CharSetHdl, weld::ComboBox&, void );
                                DECL_LINK( FirstRowHdl, weld::SpinButton&, void );
                                DECL_LINK( RbSepFixHdl, weld::Button&, void );
                                DECL_LINK( SeparatorEditHdl, weld::Entry&, void );
                                DECL_LINK( SeparatorClickHdl, weld::Button&, void );
                                DECL_LINK( SeparatorComboBoxHdl, weld::ComboBox&, void );
                                void SeparatorHdl(const weld::Widget*);
                                DECL_LINK( LbColTypeHdl, weld::ComboBox&, void );
                                DECL_LINK( UpdateTextHdl, ScCsvTableBox&, void );
                                DECL_LINK( ColTypeHdl, ScCsvTableBox&, void );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
