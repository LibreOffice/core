/*************************************************************************
 *
 *  $RCSfile: asciiopt.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:59:49 $
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

#ifndef SC_ASCIIOPT_HXX
#define SC_ASCIIOPT_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SVX_TXENCBOX_HXX
#include <svx/txencbox.hxx>
#endif

#ifndef _SC_CSVTABLEBOX_HXX
#include "csvtablebox.hxx"
#endif


// ============================================================================

class ScAsciiOptions
{
private:
    BOOL        bFixedLen;
    String      aFieldSeps;
    BOOL        bMergeFieldSeps;
    sal_Unicode cTextSep;
    CharSet     eCharSet;
    BOOL        bCharSetSystem;
    long        nStartRow;
    USHORT      nInfoCount;
    xub_StrLen* pColStart;  //! TODO replace with vector
    BYTE*       pColFormat; //! TODO replace with vector

public:
                    ScAsciiOptions();
                    ScAsciiOptions(const ScAsciiOptions& rOpt);
                    ~ScAsciiOptions();

    ScAsciiOptions& operator=( const ScAsciiOptions& rCpy );

    BOOL            operator==( const ScAsciiOptions& rCmp ) const;

    void            ReadFromString( const String& rString );
    String          WriteToString() const;

    void            InterpretColumnList( const String& rString );

    CharSet             GetCharSet() const      { return eCharSet; }
    BOOL                GetCharSetSystem() const    { return bCharSetSystem; }
    const String&       GetFieldSeps() const    { return aFieldSeps; }
    BOOL                IsMergeSeps() const     { return bMergeFieldSeps; }
    sal_Unicode         GetTextSep() const      { return cTextSep; }
    BOOL                IsFixedLen() const      { return bFixedLen; }
    USHORT              GetInfoCount() const    { return nInfoCount; }
    const xub_StrLen*   GetColStart() const     { return pColStart; }
    const BYTE*         GetColFormat() const    { return pColFormat; }
    long                GetStartRow() const     { return nStartRow; }

    void    SetCharSet( CharSet eNew )          { eCharSet = eNew; }
    void    SetCharSetSystem( BOOL bSet )       { bCharSetSystem = bSet; }
    void    SetFixedLen( BOOL bSet )            { bFixedLen = bSet; }
    void    SetFieldSeps( const String& rStr )  { aFieldSeps = rStr; }
    void    SetMergeSeps( BOOL bSet )           { bMergeFieldSeps = bSet; }
    void    SetTextSep( sal_Unicode c )         { cTextSep = c; }
    void    SetStartRow( long nRow)             { nStartRow= nRow; }

    void    SetColInfo( USHORT nCount, const xub_StrLen* pStart, const BYTE* pFormat );
    void    SetColumnInfo( const ScCsvExpDataVec& rDataVec );
};


//CHINA001 // ============================================================================
//CHINA001
//CHINA001 class ScImportAsciiDlg : public ModalDialog
//CHINA001 {
//CHINA001 SvStream*                   pDatStream;
//CHINA001 ULONG*                      pRowPosArray;
//CHINA001 ULONG*                      pRowPosArrayUnicode;
//CHINA001 USHORT                      nArrayEndPos;
//CHINA001 USHORT                      nArrayEndPosUnicode;
//CHINA001 ULONG                       nStreamPos;
//CHINA001 ULONG                       nStreamPosUnicode;
//CHINA001 BOOL                        bVFlag;
//CHINA001
//CHINA001 FixedLine                   aFlFieldOpt;
//CHINA001 FixedText                   aFtCharSet;
//CHINA001 SvxTextEncodingBox          aLbCharSet;
//CHINA001
//CHINA001 FixedText                   aFtRow;
//CHINA001 NumericField                aNfRow;
//CHINA001
//CHINA001 FixedLine                   aFlSepOpt;
//CHINA001 RadioButton                 aRbFixed;
//CHINA001 RadioButton                 aRbSeparated;
//CHINA001
//CHINA001 CheckBox                    aCkbTab;
//CHINA001 CheckBox                    aCkbSemicolon;
//CHINA001 CheckBox                    aCkbComma;
//CHINA001 CheckBox                    aCkbSpace;
//CHINA001 CheckBox                    aCkbOther;
//CHINA001 Edit                        aEdOther;
//CHINA001 CheckBox                    aCkbAsOnce;
//CHINA001 FixedText                   aFtTextSep;
//CHINA001 ComboBox                    aCbTextSep;
//CHINA001
//CHINA001 FixedLine                   aFlWidth;
//CHINA001 FixedText                   aFtType;
//CHINA001 ListBox                     aLbType;
//CHINA001
//CHINA001 ScCsvTableBox               maTableBox;
//CHINA001
//CHINA001 OKButton                    aBtnOk;
//CHINA001 CancelButton                aBtnCancel;
//CHINA001 HelpButton                  aBtnHelp;
//CHINA001
//CHINA001 String                      aCharSetUser;
//CHINA001 String                      aColumnUser;
//CHINA001 String                      aFldSepList;
//CHINA001 String                      aTextSepList;
//CHINA001
//CHINA001 // aPreviewLine contains the byte string as read from the file
//CHINA001 ByteString                  aPreviewLine[ CSV_PREVIEW_LINES ];
//CHINA001 // same for Unicode
//CHINA001 String                      aPreviewLineUnicode[ CSV_PREVIEW_LINES ];
//CHINA001
//CHINA001 CharSet                     meCharSet;          /// Selected char set.
//CHINA001 bool                        mbCharSetSystem;    /// Is System char set selected?
//CHINA001
//CHINA001 public:
//CHINA001 ScImportAsciiDlg(
//CHINA001 Window* pParent, String aDatName,
//CHINA001 SvStream* pInStream, sal_Unicode cSep = '\t' );
//CHINA001 ~ScImportAsciiDlg();
//CHINA001
//CHINA001 void                        GetOptions( ScAsciiOptions& rOpt );
//CHINA001
//CHINA001 private:
//CHINA001 /** Sets the selected char set data to meCharSet and mbCharSetSystem. */
//CHINA001 void                        SetSelectedCharSet();
//CHINA001 /** Returns all separator characters in a string. */
//CHINA001 String                      GetSeparators() const;
//CHINA001
//CHINA001 /** Enables or disables all separator checkboxes and edit fields. */
//CHINA001 void                        SetupSeparatorCtrls();
//CHINA001
//CHINA001 void                        UpdateVertical( bool bSwitchToFromUnicode = false );
//CHINA001
//CHINA001 DECL_LINK( CharSetHdl, SvxTextEncodingBox* );
//CHINA001 DECL_LINK( FirstRowHdl, NumericField* );
//CHINA001 DECL_LINK( RbSepFixHdl, RadioButton* );
//CHINA001 DECL_LINK( SeparatorHdl, Control* );
//CHINA001 DECL_LINK( LbColTypeHdl, ListBox* );
//CHINA001 DECL_LINK( UpdateTextHdl, ScCsvTableBox* );
//CHINA001 DECL_LINK( ColTypeHdl, ScCsvTableBox* );
//CHINA001 };
//CHINA001
//CHINA001
// ============================================================================

#endif

