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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SCUIASCIIOPT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SCUIASCIIOPT_HXX

#include "asciiopt.hxx"
#include <svx/langbox.hxx>

class ScImportAsciiDlg : public ModalDialog
{
    SvStream*                   mpDatStream;
    sal_uLong                       mnStreamPos;
    sal_uLong*                      mpRowPosArray;
    sal_uLong                       mnRowPosCount;

    OUString               maPreviewLine[ CSV_PREVIEW_LINES ];

    VclPtr<FixedText>                  pFtCharSet;

    VclPtr<SvxTextEncodingBox>         pLbCharSet;

    VclPtr<FixedText>                  pFtCustomLang;

    VclPtr<SvxLanguageBox>             pLbCustomLang;

    VclPtr<FixedText>                  pFtRow;
    VclPtr<NumericField>               pNfRow;

    VclPtr<RadioButton>                pRbFixed;
    VclPtr<RadioButton>                pRbSeparated;

    VclPtr<CheckBox>                   pCkbTab;
    VclPtr<CheckBox>                   pCkbSemicolon;
    VclPtr<CheckBox>                   pCkbComma;
    VclPtr<CheckBox>                   pCkbSpace;
    VclPtr<CheckBox>                   pCkbOther;
    VclPtr<Edit>                       pEdOther;
    VclPtr<CheckBox>                   pCkbAsOnce;

    VclPtr<FixedText>                  pFtTextSep;
    VclPtr<ComboBox>                   pCbTextSep;

    VclPtr<CheckBox>                   pCkbQuotedAsText;
    VclPtr<CheckBox>                   pCkbDetectNumber;

    VclPtr<FixedText>                  pFtType;
    VclPtr<ListBox>                    pLbType;

    VclPtr<ScCsvTableBox>              mpTableBox;

    OUString                    aCharSetUser;
    OUString                    aColumnUser;
    OUString                    aTextSepList;
    OUString                    maFieldSeparators;  // selected field separators
    sal_Unicode                 mcTextSep;

    rtl_TextEncoding            meCharSet;          /// Selected char set.
    bool                        mbCharSetSystem;    /// Is System char set selected?
    ScImportAsciiCall           meCall;             /// How the dialog is called (see asciiopt.hxx)

public:
                                ScImportAsciiDlg(
                                    vcl::Window* pParent, const OUString& aDatName,
                                    SvStream* pInStream, ScImportAsciiCall eCall );
                                virtual ~ScImportAsciiDlg();
    virtual void                dispose() override;

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

    bool                        GetLine( sal_uLong nLine, OUString &rText );
    void                        UpdateVertical();
    inline bool                 Seek( sal_uLong nPos ); // synced to and from mnStreamPos

                                DECL_LINK_TYPED( CharSetHdl, ListBox&, void );
                                DECL_LINK_TYPED( FirstRowHdl, Edit&, void );
                                DECL_LINK_TYPED( RbSepFixHdl, Button*, void );
                                DECL_LINK_TYPED( SeparatorEditHdl, Edit&, void );
                                DECL_LINK_TYPED( SeparatorClickHdl, Button*, void );
                                DECL_LINK_TYPED( SeparatorComboBoxHdl, ComboBox&, void );
                                void SeparatorHdl(Control*);
                                DECL_LINK_TYPED( LbColTypeHdl, ListBox&, void );
                                DECL_LINK_TYPED( UpdateTextHdl, ScCsvTableBox&, void );
                                DECL_LINK_TYPED( ColTypeHdl, ScCsvTableBox&, void );
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
