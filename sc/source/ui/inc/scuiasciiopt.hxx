/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scuiasciiopt.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-08-20 16:52:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// ============================================================================

#ifndef SCUI_ASCIIOPT_HXX
#define SCUI_ASCIIOPT_HXX


#include "asciiopt.hxx"
// ============================================================================

class ScImportAsciiDlg : public ModalDialog
{
    SvStream*                   mpDatStream;
    ULONG                       mnStreamPos;
    ULONG*                      mpRowPosArray;
    ULONG                       mnRowPosCount;

    String                      maPreviewLine[ CSV_PREVIEW_LINES ];

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
    String                      maFieldSeparators;  // selected field separators
    sal_Unicode                 mcTextSep;
    String                      maStrTextToColumns;

    CharSet                     meCharSet;          /// Selected char set.
    bool                        mbCharSetSystem;    /// Is System char set selected?

public:
                                ScImportAsciiDlg(
                                    Window* pParent, String aDatName,
                                    SvStream* pInStream, sal_Unicode cSep = '\t' );
                                ~ScImportAsciiDlg();

    void                        GetOptions( ScAsciiOptions& rOpt );
    void                        SetTextToColumnsMode();

private:
    /** Sets the selected char set data to meCharSet and mbCharSetSystem. */
    void                        SetSelectedCharSet();
    /** Returns all separator characters in a string. */
    String                      GetSeparators() const;

    /** Enables or disables all separator checkboxes and edit fields. */
    void                        SetupSeparatorCtrls();


    bool                        GetLine( ULONG nLine, String &rText );
    void                        UpdateVertical();
    inline bool                 Seek( ULONG nPos ); // synced to and from mnStreamPos

                                DECL_LINK( CharSetHdl, SvxTextEncodingBox* );
                                DECL_LINK( FirstRowHdl, NumericField* );
                                DECL_LINK( RbSepFixHdl, RadioButton* );
                                DECL_LINK( SeparatorHdl, Control* );
                                DECL_LINK( LbColTypeHdl, ListBox* );
                                DECL_LINK( UpdateTextHdl, ScCsvTableBox* );
                                DECL_LINK( ColTypeHdl, ScCsvTableBox* );
};


inline bool ScImportAsciiDlg::Seek(ULONG nPos)
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

