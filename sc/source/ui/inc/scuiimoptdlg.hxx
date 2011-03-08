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

#ifndef SCUI_IMOPTDLG_HXX
#define SCUI_IMOPTDLG_HXX

#include "imoptdlg.hxx"

//===================================================================


class ScDelimiterTable;

class ScImportOptionsDlg : public ModalDialog
{
public:
                ScImportOptionsDlg( Window*                 pParent,
                                    BOOL                    bAscii = TRUE,
                                    const ScImportOptions*  pOptions = NULL,
                                    const String*           pStrTitle = NULL,
                                    BOOL                    bMultiByte = FALSE,
                                    BOOL                    bOnlyDbtoolsEncodings = FALSE,
                                    BOOL                    bImport = TRUE );

                ~ScImportOptionsDlg();

    void GetImportOptions( ScImportOptions& rOptions ) const;

private:
    FixedLine           aFlFieldOpt;
    FixedText           aFtFont;
    SvxTextEncodingBox  aLbFont;
    FixedText           aFtFieldSep;
    ComboBox            aEdFieldSep;
    FixedText           aFtTextSep;
    ComboBox            aEdTextSep;
    CheckBox            aCbFixed;
    OKButton            aBtnOk;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    CheckBox            aCbShown;

    ScDelimiterTable*   pFieldSepTab;
    ScDelimiterTable*   pTextSepTab;

private:
    USHORT GetCodeFromCombo( const ComboBox& rEd ) const;

    DECL_LINK( FixedWidthHdl, CheckBox* );
    DECL_LINK( DoubleClickHdl, ListBox* );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
