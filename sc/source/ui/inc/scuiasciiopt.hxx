/*************************************************************************
 *
 *  $RCSfile: scuiasciiopt.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 14:12:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// ============================================================================

#ifndef SCUI_ASCIIOPT_HXX
#define SCUI_ASCIIOPT_HXX


#include "asciiopt.hxx"
// ============================================================================

class ScImportAsciiDlg : public ModalDialog
{
    SvStream*                   pDatStream;
    ULONG*                      pRowPosArray;
    ULONG*                      pRowPosArrayUnicode;
    USHORT                      nArrayEndPos;
    USHORT                      nArrayEndPosUnicode;
    ULONG                       nStreamPos;
    ULONG                       nStreamPosUnicode;
    BOOL                        bVFlag;

    FixedLine                   aFlFieldOpt;
    FixedText                   aFtCharSet;
    SvxTextEncodingBox          aLbCharSet;

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
    FixedText                   aFtTextSep;
    ComboBox                    aCbTextSep;

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

    // aPreviewLine contains the byte string as read from the file
    ByteString                  aPreviewLine[ CSV_PREVIEW_LINES ];
    // same for Unicode
    String                      aPreviewLineUnicode[ CSV_PREVIEW_LINES ];

    CharSet                     meCharSet;          /// Selected char set.
    bool                        mbCharSetSystem;    /// Is System char set selected?

public:
                                ScImportAsciiDlg(
                                    Window* pParent, String aDatName,
                                    SvStream* pInStream, sal_Unicode cSep = '\t' );
                                ~ScImportAsciiDlg();

    void                        GetOptions( ScAsciiOptions& rOpt );

private:
    /** Sets the selected char set data to meCharSet and mbCharSetSystem. */
    void                        SetSelectedCharSet();
    /** Returns all separator characters in a string. */
    String                      GetSeparators() const;

    /** Enables or disables all separator checkboxes and edit fields. */
    void                        SetupSeparatorCtrls();

    void                        UpdateVertical( bool bSwitchToFromUnicode = false );

                                DECL_LINK( CharSetHdl, SvxTextEncodingBox* );
                                DECL_LINK( FirstRowHdl, NumericField* );
                                DECL_LINK( RbSepFixHdl, RadioButton* );
                                DECL_LINK( SeparatorHdl, Control* );
                                DECL_LINK( LbColTypeHdl, ListBox* );
                                DECL_LINK( UpdateTextHdl, ScCsvTableBox* );
                                DECL_LINK( ColTypeHdl, ScCsvTableBox* );
};

#endif

