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

