/*************************************************************************
 *
 *  $RCSfile: imoptdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:54 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <segmentc.hxx>
#endif

#include "imoptdlg.hxx"
#include "scresid.hxx"
#include "imoptdlg.hrc"

SEG_EOFGLOBALS()

//========================================================================
// ScDelimiterTable
//========================================================================

class ScDelimiterTable
{
public:
        ScDelimiterTable( const String& rDelTab )
            :   theDelTab ( rDelTab ),
                cSep      ( '\t' ),
                nCount    ( rDelTab.GetTokenCount('\t') ),
                nIter     ( 0 )
            {}

    USHORT  GetCode( const String& rDelimiter ) const;
    String  GetDelimiter( USHORT nCode ) const;

    String  FirstDel()  { nIter = 0; return theDelTab.GetToken( nIter, cSep ); }
    String  NextDel()   { nIter +=2; return theDelTab.GetToken( nIter, cSep ); }

private:
    const String        theDelTab;
    const sal_Unicode   cSep;
    const xub_StrLen    nCount;
    xub_StrLen          nIter;
};

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(imoptdlg_02)

USHORT ScDelimiterTable::GetCode( const String& rDel ) const
{
    USHORT nCode = 0;
    xub_StrLen i = 0;

    if ( nCount >= 2 )
    {
        while ( i<nCount )
        {
            if ( rDel == theDelTab.GetToken( i, cSep ) )
            {
                nCode = (USHORT)theDelTab.GetToken( i+1, cSep ).ToInt32();
                i     = nCount;
            }
            else
                i += 2;
        }
    }

    return nCode;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(imoptdlg_03)

String ScDelimiterTable::GetDelimiter( USHORT nCode ) const
{
    String aStrDel;
    xub_StrLen i = 0;

    if ( nCount >= 2 )
    {
        while ( i<nCount )
        {
            if ( nCode == (USHORT)theDelTab.GetToken( i+1, cSep ).ToInt32() )
            {
                aStrDel = theDelTab.GetToken( i, cSep );
                i       = nCount;
            }
            else
                i += 2;
        }
    }

    return aStrDel;
}

//========================================================================
// ScImportOptionsDlg
//========================================================================
#pragma SEG_FUNCDEF(imoptdlg_04)

ScImportOptionsDlg::ScImportOptionsDlg( Window*                 pParent,
                                        BOOL                    bAsciiImport,
                                        const ScImportOptions*  pOptions,
                                        const String*           pStrTitle )

    :   ModalDialog ( pParent, ScResId( RID_SCDLG_IMPORTOPT ) ),
        aBtnOk      ( this, ScResId( BTN_OK ) ),
        aBtnCancel  ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp    ( this, ScResId( BTN_HELP ) ),
        aFtFieldSep ( this, ScResId( FT_FIELDSEP ) ),
        aEdFieldSep ( this, ScResId( ED_FIELDSEP ) ),
        aFtTextSep  ( this, ScResId( FT_TEXTSEP ) ),
        aEdTextSep  ( this, ScResId( ED_TEXTSEP ) ),
        aFtFont     ( this, ScResId( FT_FONT ) ),
        aLbFont     ( this, ScResId( bAsciiImport ? DDLB_FONT : LB_FONT ) ),
        aGbFieldOpt ( this, ScResId( GB_FIELDOPT ) ),

        aCharKeyList( ScResId( STR_CHARSETKEYS ) )
{
    // im Ctor-Initializer nicht moeglich (MSC kann das nicht):
    pFieldSepTab = new ScDelimiterTable( String(ScResId(SCSTR_FIELDSEP)) );
    pTextSepTab  = new ScDelimiterTable( String(ScResId(SCSTR_TEXTSEP)) );

    String aStr = pFieldSepTab->FirstDel();
    USHORT nCode;

    while ( aStr.Len() > 0 )
    {
        aEdFieldSep.InsertEntry( aStr );
        aStr = pFieldSepTab->NextDel();
    }

    aStr = pTextSepTab->FirstDel();

    while ( aStr.Len() > 0 )
    {
        aEdTextSep.InsertEntry( aStr );
        aStr = pTextSepTab->NextDel();
    }

    aLbFont.SelectEntryPos( aLbFont.GetEntryCount()-1 );
    aEdFieldSep.SetText( aEdFieldSep.GetEntry(0) );
    aEdTextSep.SetText( aEdTextSep.GetEntry(0) );

    if ( pOptions )
        aLbFont.SelectEntry( pOptions->aStrFont );

    if ( !bAsciiImport )
    {
        aGbFieldOpt.SetText( aFtFont.GetText() );
        aFtFieldSep.Hide();
        aFtTextSep .Hide();
        aFtFont    .Hide();
        aEdFieldSep.Hide();
        aEdTextSep .Hide();
        aLbFont.GrabFocus();
    }
    else
    {
        if ( pOptions )
        {
            nCode = pOptions->nFieldSepCode;
            aStr  = pFieldSepTab->GetDelimiter( nCode );

            if ( !aStr.Len() )
                aEdFieldSep.SetText( String((sal_Unicode)nCode) );
            else
                aEdFieldSep.SetText( aStr );

            nCode = pOptions->nTextSepCode;
            aStr  = pTextSepTab->GetDelimiter( nCode );

            if ( !aStr.Len() )
                aEdTextSep.SetText( String((sal_Unicode)nCode) );
            else
                aEdTextSep.SetText( aStr );
        }
    }

    // optionaler Titel:
    if ( pStrTitle )
        SetText( *pStrTitle );

    FreeResource();
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(imoptdlg_05)

__EXPORT ScImportOptionsDlg::~ScImportOptionsDlg()
{
    delete pFieldSepTab;
    delete pTextSepTab;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(imoptdlg_06)

void ScImportOptionsDlg::GetImportOptions( ScImportOptions& rOptions ) const
{
    USHORT  nKeyIdx = aLbFont.GetEntryCount()-1;

    if ( aLbFont.GetSelectEntryCount() != 0 )
        nKeyIdx = aLbFont.GetSelectEntryPos();

    rOptions.aStrFont = aCharKeyList.GetToken( nKeyIdx );
    rOptions.eCharSet = (CharSet)((long)aLbFont.GetEntryData( nKeyIdx ));

    if ( aFtFieldSep.IsEnabled() )
    {
        rOptions.nFieldSepCode = GetCodeFromCombo( aEdFieldSep );
        rOptions.nTextSepCode  = GetCodeFromCombo( aEdTextSep );
    }
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(imoptdlg_07)

USHORT ScImportOptionsDlg::GetCodeFromCombo( const ComboBox& rEd ) const
{
    ScDelimiterTable* pTab;
    String  aStr( rEd.GetText() );
    USHORT  nCode;

    if ( &rEd == &aEdTextSep )
        pTab = pTextSepTab;
    else
        pTab = pFieldSepTab;

    if ( !aStr.Len() )
    {
        nCode = 0;          // kein Trennzeichen
    }
    else
    {
        nCode = pTab->GetCode( aStr );

        if ( nCode == 0 )
            nCode = (USHORT)aStr.GetChar(0);
    }

    return nCode;
}

//------------------------------------------------------------------------
//  Der Options-String darf kein Semikolon mehr enthalten (wegen Pickliste)
//  darum ab Version 336 Komma stattdessen

#pragma SEG_FUNCDEF(imoptdlg_08)

ScImportOptions::ScImportOptions( const String& rStr )
{
    if ( rStr.GetTokenCount(',') >= 3 )
    {
        nFieldSepCode = (USHORT) rStr.GetToken(0,',').ToInt32();
        nTextSepCode  = (USHORT) rStr.GetToken(1,',').ToInt32();
        aStrFont      = rStr.GetToken(2,',');
        eCharSet      = GetCharsetValue(aStrFont);
    }
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(imoptdlg_0a)

String ScImportOptions::BuildString() const
{
    String  aResult;

    aResult += String::CreateFromInt32(nFieldSepCode);
    aResult += ',';
    aResult += String::CreateFromInt32(nTextSepCode);
    aResult += ',';
    aResult += aStrFont;

    return aResult;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(imoptdlg_09)

String ScImportOptions::BuildParaString( const String& rTyp, const String& rDsn ) const
{
    String  aResult;

    aResult.AppendAscii(RTL_CONSTASCII_STRINGPARAM("Typ="));
    aResult += rTyp;
    aResult.AppendAscii(RTL_CONSTASCII_STRINGPARAM(";Dsn="));
    aResult += rDsn;
    aResult.AppendAscii(RTL_CONSTASCII_STRINGPARAM(";Charset="));
    aResult += aStrFont;
    aResult.AppendAscii(RTL_CONSTASCII_STRINGPARAM(";del="));
    aResult += (sal_Unicode)nFieldSepCode;
    aResult.AppendAscii(RTL_CONSTASCII_STRINGPARAM(";strdel="));
    aResult += (sal_Unicode)nTextSepCode;
    aResult.AppendAscii(RTL_CONSTASCII_STRINGPARAM(";deleted;HDR;"));

    return aResult;
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.16  2000/09/17 14:08:56  willem.vandorp
    OpenOffice header added.

    Revision 1.15  2000/08/31 16:38:20  willem.vandorp
    Header and footer replaced

    Revision 1.14  2000/04/14 17:38:03  nn
    unicode changes

    Revision 1.13  1997/12/05 18:55:10  ANK
    Includes geaendert


      Rev 1.12   05 Dec 1997 19:55:10   ANK
   Includes geaendert

      Rev 1.11   29 Oct 1996 14:03:36   NN
   ueberall ScResId statt ResId

      Rev 1.10   16 Sep 1996 20:05:00   NN
   kein Semikolon in Options-String

      Rev 1.9   05 Jun 1996 15:49:34   NN
   STR_FIELD/TEXTSEP jetzt SCSTR...

      Rev 1.8   15 Jan 1996 11:45:20   NN
   #24293# String mit Ascii-Codes, #24246# leere Trenner erlauben

      Rev 1.7   07 Dec 1995 17:22:32   MO
   Cancel wieder aktiviert, GrabFocus auf Lb im Non-ASCII Modus

      Rev 1.6   06 Dec 1995 14:11:48   MO
   bei nicht-ASCII-Import nur FontListBox (BugId: 21193)

      Rev 1.5   19 Nov 1995 11:54:46   OG
   pragma korrigiert

      Rev 1.4   15 Nov 1995 16:44:08   JN
   Optimierungen

      Rev 1.3   16 Oct 1995 19:19:38   JN
   richtigen Zeichensatz ausgeben

      Rev 1.2   28 Jul 1995 10:27:32   MO
   Feldtrenner-Delimiter auf TAB geandert

      Rev 1.1   28 Jul 1995 09:33:46   MO
   Dialog-Titel als optionalen Parameter

      Rev 1.0   20 Jul 1995 15:33:24   MO
   Initial revision.

------------------------------------------------------------------------ */

#pragma SEG_EOFMODULE

