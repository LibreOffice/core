/*************************************************************************
 *
 *  $RCSfile: asciiopt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:57 $
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

#ifndef SC_ASCIIOPT_HXX
#define SC_ASCIIOPT_HXX

#include <tools/solar.h>
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
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#ifndef _SVSTDARR_ULONGS
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>
#endif

#include "tabrul.hxx"

//===================================================================

//  Spaltenformate (Werte wie bei OpenText in Excel)

#define SC_COL_STANDARD     1
#define SC_COL_TEXT         2
#define SC_COL_MDY          3
#define SC_COL_DMY          4
#define SC_COL_YMD          5
#define SC_COL_SKIP         9
#define SC_COL_ENGLISH      10

#define SC_ASCIIOPT_PREVIEW_LINES 5     // Anzahl Preview Lines

//  Import-Optionen

class ScAsciiOptions
{
private:
    BOOL        bFixedLen;
    String      aFieldSeps;
    BOOL        bMergeFieldSeps;
    sal_Unicode cTextSep;
    CharSet     eCharSet;
    long        nStartRow;
    USHORT      nInfoCount;
    xub_StrLen* pColStart;
    BYTE*       pColFormat;

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
    const String&       GetFieldSeps() const    { return aFieldSeps; }
    BOOL                IsMergeSeps() const     { return bMergeFieldSeps; }
    sal_Unicode         GetTextSep() const      { return cTextSep; }
    BOOL                IsFixedLen() const      { return bFixedLen; }
    USHORT              GetInfoCount() const    { return nInfoCount; }
    const xub_StrLen*   GetColStart() const     { return pColStart; }
    const BYTE*         GetColFormat() const    { return pColFormat; }
    long                GetStartRow() const     { return nStartRow; }

    void    SetCharSet( CharSet eNew )          { eCharSet = eNew; }
    void    SetFixedLen( BOOL bSet )            { bFixedLen = bSet; }
    void    SetFieldSeps( const String& rStr )  { aFieldSeps = rStr; }
    void    SetMergeSeps( BOOL bSet )           { bMergeFieldSeps = bSet; }
    void    SetTextSep( sal_Unicode c )         { cTextSep = c; }
    void    SetStartRow( long nRow)             { nStartRow= nRow; }

    void    SetColInfo( USHORT nCount, const xub_StrLen* pStart, const BYTE* pFormat );
};


//  Import-Dialog

class ScImportAsciiDlg : public ModalDialog
{
    SvStream*       pDatStream;
    ULONG*          pRowPosArray;
    USHORT          nArrayEndPos;

    BOOL            bVFlag;

    FixedText       aFtRow;
    NumericField    aNfRow;

    FixedText       aFtCharSet;
    ListBox         aLbCharSet;

    RadioButton     aRbFixed;
    RadioButton     aRbSeparated;

    CheckBox        aCkbTab;
    CheckBox        aCkbSemicolon;
    CheckBox        aCkbComma;
    CheckBox        aCkbSpace;
    CheckBox        aCkbOther;
    Edit            aEdOther;
    CheckBox        aCkbAsOnce;
    FixedText       aFtTextSep;
    ComboBox        aCbTextSep;

    ScTableWithRuler aTableBox;
    FixedText       aFtType;
    ListBox         aLbType;
    ScrollBar       aScrollbar;
    ScrollBar       aVScroll;

    GroupBox        aGbSepOpt;
    GroupBox        aGbFieldOpt;
    GroupBox        aGbWidth;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    String          aStringCol;
    String          aStringTo;
    String          aCharSetUser;
    String          aColumnUser;
    String          aFldSepList;
    String          aTextSepList;

    // aPreviewLine contains the byte string as read from the file
    ByteString      aPreviewLine[SC_ASCIIOPT_PREVIEW_LINES];

    USHORT          nScrollPos;
    USHORT          nUsedCols;
    USHORT*         pEndValues;
    BYTE*           pFlags;
    CharSet         eCharSet;

    void        CheckDisable();
    void        CheckValues(BOOL bReadVal = FALSE, USHORT nEditField = USHRT_MAX);
    void        CheckColTypes(BOOL bReadVal,void *pCtr=NULL);
    void        CheckScrollRange();
    void        CheckScrollPos();
    void        UpdateVertical();
    void        DelimitedPreview();
    void        GetCharSet();

    DECL_LINK( VarFixHdl, void* );
    DECL_LINK( ScrollHdl, void* );
    DECL_LINK( SelectHdl, ScTableWithRuler* );
    DECL_LINK( ColTypeHdl, void* );
    DECL_LINK( CharSetHdl, void* );
    DECL_LINK( VarSepHdl, void* );

public:
                ScImportAsciiDlg( Window* pParent, String aDatName,
                                    SvStream* pInStream, sal_Unicode cSep = '\t' );
                ~ScImportAsciiDlg();

    void        GetOptions( ScAsciiOptions& rOpt );
};



#endif


