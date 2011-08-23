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
#ifdef _MSC_VER
#pragma hdrstop
#endif



#include <stdlib.h>  // strtod()

#include <bf_so3/linkmgr.hxx>
#include <tools/datetime.hxx>

#include "doc.hxx"
#include "rdswg.hxx"
#include "fldbas.hxx"
#include "docufld.hxx"
#include "chpfld.hxx"
#include "ddefld.hxx"
#include "expfld.hxx"
#include "reffld.hxx"
#include "usrfld.hxx"
#include "dbfld.hxx"
#include "flddat.hxx"
#include "numrule.hxx"
namespace binfilter {

static UINT32  nNewFldFmt;                       // Aktueller Format-Wert


static SwDBFieldType* In_SwDBFieldType( SwSwgReader& rPar )
{
    // MS: An den Typen DBName und FeldName uebergeben
    SwDBFieldType aType( rPar.pDoc, rPar.GetText(), rPar.pDoc->GetDBData() );
    // altes Expand wegwerfen
    if( rPar.aHdr.nVersion < SWG_VER_POOLIDS )
        rPar.GetText( FALSE );

    //JP 13.02.96: Bug 25321 - Felder ohne Namen nie einlesen
    if( !aType.GetName().Len() )
        return 0;

    return (SwDBFieldType*) rPar.pDoc->InsertFldType( aType );
}

static SwUserFieldType* In_SwUserFieldType( SwSwgReader& rPar )
{
    SwUserFieldType aType( rPar.pDoc, rPar.GetText() );
    aType.SetContent( rPar.GetText() );

    //JP 13.02.96: Bug 25321 - Felder ohne Namen nie einlesen
    if( !aType.GetName().Len() )
        return 0;

    SwUserFieldType* p = (SwUserFieldType*) rPar.pDoc->InsertFldType( aType );

    sal_Char* dummy;
    ByteString aVal = rPar.GetAsciiText();

    USHORT nType;
    if( rPar.aHdr.nVersion >= SWG_VER_POOLIDS )
        rPar.r >> nType;

    if( !rPar.bNew && p->GetDepends() )
        return p;

    p->SetValue( strtod( aVal.GetBuffer(), &dummy ) );

    if( rPar.aHdr.nVersion >= SWG_VER_POOLIDS )
    {
        // hot fix
        if( !nType )
            nType = GSE_STRING;
    }
    else
    {
        nType = (USHORT)nNewFldFmt & 3;
        nNewFldFmt &= ~3;

        // Anfuehrungsstriche alter Vars wegschmeissen
        String aTmp(aType.GetContent());
        if(nType & GSE_STRING)
        {
            aTmp = aTmp.Copy(1, aTmp.Len() - 2);
            p->SetContent(aTmp);
        }
    }
    p->SetType( nType );
    return p;
}

/* MS: Diese Felder gibt es nicht mehr

static SwRegFieldType* In_SwRegFieldType( SwSwgReader& rPar )
{
    BYTE nCount;
    long nEditTime;
    rPar.r.long4();
    rPar.r >> nCount;
    aType.aTitle = rPar.GetText();
    rPar.r >> nEditTime;
    SwRegFieldType aType;
    aType.aEditTime.SetTime( nEditTime );
    for( BYTE i = 0; ( i < nCount ) && rPar.r.good(); i++ )
    {
        aType.aInfo[ i ].aAuthor = rPar.GetText();
        ULONG nDate;
        long  nTime;
        rPar.r >> nDate >> nTime;
        aType.aInfo[ i ].aDateTime.SetDate( nDate );
        aType.aInfo[ i ].aDateTime.SetTime( nTime );
    }
    rPar.r.long3();
    return (SwRegFieldType*) rPar.pDoc->InsertFldType( aType );
}


static SwVarRegFieldType* In_SwVarRegFieldType( SwSwgReader& rPar )
{
    USHORT nType;
    rPar.r >> nType;
    SwVarRegFieldType aType( rPar.GetText(), nType );
    aType.SetValue( rPar.GetText() );
    return (SwVarRegFieldType*) rPar.pDoc->InsertFldType( aType );
}

End Of Comment MS: */

static SwDDEFieldType* In_SwDDEFieldType( SwSwgReader& rPar )
{
    USHORT nType;
    rPar.r >> nType;
    String aName( rPar.GetText() );
    String aCmd( rPar.GetText() );
    if( !nType )
        nType = ::binfilter::LINKUPDATE_ONCALL;
    else
        nType = ::binfilter::LINKUPDATE_ALWAYS;

    // die ersten beiden Blanks gegen den neuen Trenner austauschen
    xub_StrLen nFnd = aCmd.Search( ' ' );
    if( STRING_NOTFOUND != nFnd )
        aCmd.SetChar( nFnd, ::binfilter::cTokenSeperator );
    nFnd = aCmd.Search( ' ', nFnd );
    if( STRING_NOTFOUND != nFnd )
        aCmd.SetChar( nFnd, ::binfilter::cTokenSeperator );

    //JP 13.02.96: Bug 25321 - Felder ohne Namen nie einlesen
    if( !aName.Len() || !aCmd.Len() )
        return 0;

    SwDDEFieldType aType( aName, aCmd, nType );
    return (SwDDEFieldType*) rPar.pDoc->InsertFldType( aType );
}

static SwSetExpFieldType* In_SwSetExpFieldType( SwSwgReader& rPar )
{
    USHORT nType;
    rPar.r >> nType;
    String aName( rPar.GetText() );

    //JP 13.02.96: Bug 25321 - Felder ohne Namen nie einlesen
    if( !aName.Len() )
        return 0;

    SwSetExpFieldType aType( rPar.pDoc, aName, nType );
    return (SwSetExpFieldType*) rPar.pDoc->InsertFldType( aType );
}

/////////////////////////////////////////////////////////////////////////////

static SwField* In_SwDBField( SwSwgReader& rPar, SwDBFieldType* pType, USHORT nSub )
{
    pType = In_SwDBFieldType( rPar );

    //JP 13.02.96: Bug 25321 - Felder ohne Namen nie einlesen
    if( !pType )
        return 0;

    SwDBField* pFld = new SwDBField( pType );
    pFld->SetSubType(nSub);
    if( rPar.aHdr.nVersion >= SWG_VER_POOLIDS )
    {   // Expansion laden
        pFld->InitContent(rPar.GetText( FALSE ));
    }
    return pFld;
}

static SwField* In_SwUserField( SwSwgReader& rPar, SwUserFieldType* pType,
                                USHORT nSub )
{
    pType = In_SwUserFieldType( rPar );

    //JP 13.02.96: Bug 25321 - Felder ohne Namen nie einlesen
    if( !pType )
        return 0;

    return new SwUserField( pType, nSub );    // MS: Hier kann auch das Format mit
                                        // uebergeben werden
    if( UF_STRING & pType->GetType() )
        nNewFldFmt = 0; // Warum auch immer!

}

static SwField* In_SwFileNameField( SwSwgReader&, SwFileNameFieldType* pType )
{
    return new SwFileNameField( pType );
}

static SwField* In_SwDBNameField( SwSwgReader& rPar, SwDBNameFieldType* pType )
{
    return new SwDBNameField( pType, rPar.pDoc->GetDBData() );
}

static SwField* In_SwDateField( SwSwgReader&, SwDateTimeFieldType* pType )
{
    return new SwDateTimeField( pType, DATEFLD );
}

static SwField* In_SwTimeField( SwSwgReader&, SwDateTimeFieldType* pType )
{
    return new SwDateTimeField( pType, TIMEFLD );
}

static SwField* In_SwPageNumberField( SwSwgReader& rPar, SwPageNumberFieldType* pType )
{
    USHORT nOff = 0;
    USHORT nSub = PG_RANDOM;
    if( rPar.aHdr.nVersion >= SWG_VER_POOLIDS )
    {   rPar.r >> nOff;
        rPar.r >> nSub;
    }
    return new SwPageNumberField( pType, nSub, nNewFldFmt, nOff );
}

static SwField* In_SwAuthorField( SwSwgReader&, SwAuthorFieldType* pType )
{
    return new SwAuthorField( pType );
}

static SwField* In_SwChapterField( SwSwgReader&, SwChapterFieldType* pType )
{
    return new SwChapterField( pType );
}

static SwField* In_SwDocStatField( SwSwgReader& rPar, SwDocStatFieldType* pType )
{
    USHORT nSubType = 0;
    if( rPar.aHdr.nVersion < SWG_VER_POOLIDS )
    {
        // Im Format stand der Subtyp:
        nSubType = (USHORT)nNewFldFmt;
        nNewFldFmt = SVX_NUM_ARABIC;
    } else rPar.r >> nSubType;
    return new SwDocStatField( pType, nSubType, nNewFldFmt );
}

static SwField* In_SwHiddenTxtField( SwSwgReader& rPar, SwHiddenTxtFieldType* pType )
{
    BYTE cFlags;
    USHORT nSubType = TYP_HIDDENTXTFLD;
    rPar.r >> cFlags;
    String aText( rPar.GetText() );
    String aCond;
    aCond = rPar.GetText();
    if( rPar.aHdr.nVersion >= SWG_VER_FRAMES3 )
        rPar.r >> nSubType;
    BOOL bCond = BOOL( ( cFlags & 0x02 ) != 0 );
    BOOL bIsHidden = BOOL( ( cFlags & 0x01 ) != 0 );

    if( rPar.aHdr.nVersion < SWG_VER_POOLIDS )
        bCond = BOOL( !bCond );

    if( bCond && TYP_CONDTXTFLD != nSubType )
    {
// steht im sw3field.cxx
extern void lcl_sw3io_ChkHiddenExp( String& );
        lcl_sw3io_ChkHiddenExp( aCond );
        bIsHidden = !bIsHidden;
    }

    SwHiddenTxtField* pFld = new SwHiddenTxtField( pType,
                bCond,
                aEmptyStr, aText,
                bIsHidden, nSubType );
    pFld->SetPar1( aCond );
    return pFld;
}

static SwField* In_SwGetRefField( SwSwgReader& rPar, SwGetRefFieldType* pType )
{
    String aName( rPar.GetText() );
    String aText( rPar.GetText( FALSE ) );
    SwGetRefField* pFld = new SwGetRefField( pType, aName, 0, 0, 0 );
    if( aText.Len() )
        pFld->SetExpand( aText );
    return pFld;
}

static SwField* In_SwPostItField( SwSwgReader& rPar, SwPostItFieldType* pType )
{
    ULONG nDate;
    rPar.r.long4();
    rPar.r >> nDate;
    rPar.r.long3();
    String aAuthor( rPar.GetText() );
    String aText( rPar.GetText() );
    return new SwPostItField( pType, aAuthor, aText, Date( nDate ) );
}

static SwField* In_SwFixDateField( SwSwgReader& rPar, SwDateTimeFieldType* pType )
{
    long nVal;
    rPar.r.long4();
    rPar.r >> nVal;
    rPar.r.long3();
    SwDateTimeField* pFld = new SwDateTimeField( pType, DATEFLD|FIXEDFLD );
    Time aTmpTime;
    Date aTmpDate(nVal);
    DateTime aDT(aTmpDate, aTmpTime);
    pFld->SetDateTime( aDT );
    return pFld;
}

static SwField* In_SwFixTimeField( SwSwgReader& rPar, SwDateTimeFieldType* pType )
{
    long nVal;
    rPar.r.long4();
    rPar.r >> nVal;
    rPar.r.long3();
    SwDateTimeField* pFld = new SwDateTimeField( pType );
    Date aTmpDate;
    DateTime aDT(aTmpDate, Time(nVal));
    pFld->SetDateTime( aDT );
    return pFld;
}

/* MS: RegFields werden nicht mehr gebraucht

static SwField* In_SwRegField( SwSwgReader& rPar, SwRegFieldType* pType )
{
    pType = In_SwRegFieldType( rPar );
    if( !pType ) return NULL;
    USHORT nType;
    rPar.r >> nType;
    return new SwRegField( pType, nType );
}

static SwField* In_SwVarRegField( SwSwgReader& rPar, SwVarRegFieldType* pType )
{
    pType = In_SwVarRegFieldType( rPar );
    return new SwVarRegField( pType );
}
*/

static SwField* In_SwDDEField( SwSwgReader& rPar, SwDDEFieldType* pType )
{
    pType = In_SwDDEFieldType( rPar );
    //JP 13.02.96: Bug 25321 - Felder ohne Namen nie einlesen
    if( !pType )
        return 0;
    return new SwDDEField( pType );
}

static SwField* In_SwInputField( SwSwgReader& rPar, SwInputFieldType* pType )
{
    SwInputField* pFld = new SwInputField( pType, aEmptyStr, aEmptyStr );
    // Prompt
    pFld->SetPar2( rPar.GetText() );
    // Content
    pFld->SetPar1( rPar.GetText() );
    USHORT nSubType;

    if( rPar.aHdr.nVersion < SWG_VER_POOLIDS )
    {   // Im Format stand der Subtyp:
        nSubType   = (USHORT)nNewFldFmt & 0x03;
        nNewFldFmt = 0;
    }
    else
        rPar.r >> nSubType;
    pFld->SetSubType( nSubType );
    return pFld;
}

static SwField* In_SwMacroField( SwSwgReader& rPar, SwMacroFieldType* pType )
{
    String aName;
    String aText;
    if( rPar.aHdr.nVersion < SWG_VER_POOLIDS ) {
        aName = rPar.GetText();
        String aLib( rPar.GetText() );
        String aTmp(aLib);
        aTmp += ':';
        aTmp += aName;
        aName = aTmp;
    } else {
        aName = rPar.GetText();
        aText = rPar.GetText();
    }
    return new SwMacroField( pType, aName, aText );
}

static SwField* In_SwTblField( SwSwgReader& rPar, SwTblFieldType* pType, USHORT nSub )
{
    BYTE bInTbl; // wird ignoriert
    rPar.r >> bInTbl;
    String aFormel = rPar.GetText();
    String aTxt    = rPar.GetText( FALSE );
    SwTblField* pFld = new SwTblField( pType, aFormel );
    pFld->SetSubType(nSub);
    if( !bInTbl && aTxt.Len() )
        pFld->ChgExpStr( aTxt );
    return pFld;
}

static SwField* In_SwGetExpField( SwSwgReader& rPar, SwGetExpFieldType* pType, USHORT nSub )
{
    SwGetExpField* p = new SwGetExpField( pType, rPar.GetText() );
    USHORT nSubType = GSE_EXPR;
    if( rPar.aHdr.nVersion >= SWG_VER_POOLIDS )
    {
        String s = rPar.GetText();
        p->ChgExpStr( s );
        if( rPar.aHdr.nVersion >= SWG_VER_GETEXP )
            rPar.r >> nSubType;
    }
    else
    {   nSubType = (USHORT)nNewFldFmt & 3;
        nNewFldFmt &= ~3;
    }
    p->SetSubType(nSubType|nSub);
    if( GSE_STRING & nSubType )
        nNewFldFmt = 0;

    // Falls sich das Feld in einem Shared-Format befindet,
    // mus es nach dem Lesen ge-updated werden
//  if( rPar.nStatus & SWGSTAT_SHAREDFMT )
//      rPar.nStatus |= SWGSTAT_UPDATEEXPR;
    return p;
}

static SwField* In_SwSetExpField( SwSwgReader& rPar, SwSetExpFieldType* pType, USHORT nSub )
{
    pType = In_SwSetExpFieldType( rPar );
    //JP 13.02.96: Bug 25321 - Felder ohne Namen nie einlesen
    if( !pType )
        return 0;
    BYTE cFlags;
    rPar.r >> cFlags;
    String aFrml( rPar.GetText() );
    SwSetExpField* pFld = new SwSetExpField( pType, aFrml, pType->GetType());
    if( cFlags & 0x02 )
        pFld->ChgExpStr( rPar.GetText() );
    if( cFlags & 0x01 )
        pFld->SetInputFlag(TRUE);

    if( rPar.aHdr.nVersion < SWG_VER_POOLIDS )
    {
        USHORT nSubType = (USHORT)nNewFldFmt & 3;
        nNewFldFmt &= ~3;
        pFld->SetSubType( nSubType | (cFlags & 1 ) );

        // bei alten Vars Anfuehrungsstriche entfernen
        if(nSubType & GSE_STRING)
        {
            aFrml = aFrml.Copy(1, aFrml.Len() - 2);
            pFld->ChgExpStr( aFrml );
            pFld->SetPar2( aFrml );
        }
    }

    aFrml = pFld->GetExpStr();
//  pFld->SetValue( strtod( (const sal_Char*)aFrml, &dummy ) );
    // TODO: unicode: is this correct?
    pFld->SetValue( aFrml.ToDouble() );

    // Hack: fuer Seq-Felder wurde das Original-Format im Subtyp uebergeben,
    // aber nur, wenn es auch als entsprechendes Format in Frage kommt.
    // (SUB_VISIBLE und SUB_CMD sind disjunkt).
    USHORT nSubType = ((SwSetExpField *)pFld)->GetSubType();
    if( nSub >= (USHORT)SVX_NUM_CHARS_UPPER_LETTER &&
        nSub <= (USHORT)SVX_NUM_BITMAP )
    {
        if( GSE_SEQ & nSubType )
            nNewFldFmt = nSub;
        nSub = 0;
    }
    nSubType |= nSub;
    pFld->SetSubType( nSubType );
    if( GSE_STRING & nSubType )
        nNewFldFmt = 0; // Warum auch immer

    // neue Felder: Subtyp bereits im Feldtyp!
    // Falls sich das Feld in einem Shared-Format befindet,
    // mus es nach dem Lesen ge-updated werden
//  if( rPar.nStatus & SWGSTAT_SHAREDFMT )
//      rPar.nStatus |= SWGSTAT_UPDATEEXPR;
    return pFld;
}

static SwField* In_SwHiddenParaField( SwSwgReader& rPar, SwHiddenParaFieldType* pType )
{
    BYTE bHidden;
    rPar.r >> bHidden;
    String aCond( rPar.GetText() );
    SwHiddenParaField* pFld = new SwHiddenParaField( pType, aCond );
    pFld->SetHidden( (BOOL) bHidden );
    return pFld;
}

static SwField* In_SwDocInfoField( SwSwgReader& rPar, SwDocInfoFieldType* pType,
                                   USHORT nSubType   )
{
    BYTE nType;
    rPar.r >> nType;
    return new SwDocInfoField( pType, (USHORT)nType | nSubType );
}

static SwField* In_SwTemplNameField( SwSwgReader& rPar, SwTemplNameFieldType* pType )
{
    return new SwTemplNameField( pType, nNewFldFmt );
}

static SwField* In_SwDBNextSetField( SwSwgReader& rPar, SwDBNextSetFieldType* pType )
{
    String aCond(rPar.GetText());
    String aName(rPar.GetText());
    return new SwDBNextSetField( pType, aCond, aName, rPar.pDoc->GetDBData() );
}

static SwField* In_SwDBNumSetField( SwSwgReader& rPar, SwDBNumSetFieldType* pType )
{
    String aCond(rPar.GetText());
    String aPar2(rPar.GetText());
    return new SwDBNumSetField( pType, aCond, aPar2, rPar.pDoc->GetDBData());
}

static SwField* In_SwDBSetNumberField( SwSwgReader& rPar, SwDBSetNumberFieldType* pType )
{
    SwDBSetNumberField* pFld = new SwDBSetNumberField( pType, rPar.pDoc->GetDBData() );
    if( rPar.aHdr.nVersion >= SWG_VER_FRAMES3 )
    {
        long n;
        rPar.r.long4();
        rPar.r >> n;
        rPar.r.long3();
        pFld->SetSetNumber( n );
    }
    return pFld;
}

static SwField* In_SwExtUserField( SwSwgReader& rPar, SwExtUserFieldType* pType )
{
    String aData = rPar.GetText();
    USHORT nSubType;
    rPar.r >> nSubType;

    SwExtUserField* pFld = new SwExtUserField( pType, nSubType );
    pType->SetData( aData );
    return pFld;
}

/////////////////////////////////////////////////////////////////////////////

extern void sw3io_ConvertFromOldField( SwDoc& rDoc, USHORT& rWhich,
                                       USHORT& rSubType, UINT32 &rFmt,
                                       USHORT nVersion );

SwField* SwSwgReader::InField()
{
    long endfld = r.getskip();
    short nFld;
    USHORT nOldFldFmt;
    USHORT nSubType = 0;

    r >> nFld >> nOldFldFmt;
    nNewFldFmt = nOldFldFmt;

    if( nFld == -1 )
    {
        // war unbekanntes Feld, ignorieren
        r.skip(); return NULL;
    }
    nFld += RES_FIELDS_BEGIN;

    UINT16 nRealWhich = nFld;
    sw3io_ConvertFromOldField( *pDoc, nRealWhich, nSubType, nNewFldFmt, 0 );

    SwField* pFld = NULL;
    SwFieldType* pType = pDoc->GetSysFldType( (const RES_FIELDS) nRealWhich );
    switch( nFld )
    {
        case RES_DBFLD:
            pFld = In_SwDBField( *this, NULL, nSubType );
            break;
        case RES_USERFLD:
            pFld = In_SwUserField( *this, NULL, nSubType );
            break;
        case RES_FILENAMEFLD:
            pFld = In_SwFileNameField( *this, (SwFileNameFieldType*) pType );
            break;
        case RES_DBNAMEFLD:
            pFld = In_SwDBNameField( *this, (SwDBNameFieldType*) pType );
            break;
        case RES_DATEFLD:
            pFld = In_SwDateField( *this, (SwDateTimeFieldType*) pType );
            break;
        case RES_TIMEFLD:
            pFld = In_SwTimeField( *this, (SwDateTimeFieldType*) pType );
            break;
        case RES_PAGENUMBERFLD:
            pFld = In_SwPageNumberField( *this, (SwPageNumberFieldType*) pType );
            break;
        case RES_AUTHORFLD:
            pFld = In_SwAuthorField( *this, (SwAuthorFieldType*) pType );
            break;
        case RES_CHAPTERFLD:
            pFld = In_SwChapterField( *this, (SwChapterFieldType*) pType );
            break;
        case RES_DOCSTATFLD:
            pFld = In_SwDocStatField( *this, (SwDocStatFieldType*) pType );
            break;
        case RES_GETREFFLD:
            pFld = In_SwGetRefField( *this, (SwGetRefFieldType*) pType );
            break;
        case RES_HIDDENTXTFLD:
            pFld = In_SwHiddenTxtField( *this, (SwHiddenTxtFieldType*) pType );
            break;
        case RES_POSTITFLD:
            pFld = In_SwPostItField( *this, (SwPostItFieldType*) pType );
            break;
        case RES_FIXDATEFLD:
            pFld = In_SwFixDateField( *this, (SwDateTimeFieldType*) pType );
            break;
        case RES_FIXTIMEFLD:
            pFld = In_SwFixTimeField( *this, (SwDateTimeFieldType*) pType );
            break;
        /*
            MS: gibst nicht mehr

        case RES_REGFLD:
            pFld = In_SwRegField( *this, NULL );
            break;
        case RES_VARREGFLD:
            pFld = In_SwVarRegField( *this, NULL );
            break;
        */
        case RES_DDEFLD:
            pFld = In_SwDDEField( *this, NULL );
            break;
        case RES_INPUTFLD:
            pFld = In_SwInputField( *this, (SwInputFieldType*) pType );
            break;
        case RES_MACROFLD:
            pFld = In_SwMacroField( *this, (SwMacroFieldType*) pType );
            break;
        case RES_TABLEFLD:
            pFld = In_SwTblField( *this, (SwTblFieldType*) pType, nSubType );
            break;
        case RES_GETEXPFLD:
            pFld = In_SwGetExpField( *this, (SwGetExpFieldType*) pType, nSubType );
            break;
        case RES_SETEXPFLD:
            pFld = In_SwSetExpField( *this, NULL, nSubType );
            break;
        case RES_HIDDENPARAFLD:
            pFld = In_SwHiddenParaField( *this, (SwHiddenParaFieldType*) pType );
            break;
        case RES_DOCINFOFLD:
            pFld = In_SwDocInfoField( *this, (SwDocInfoFieldType*) pType, nSubType );
            break;
        case RES_TEMPLNAMEFLD:
            pFld = In_SwTemplNameField( *this, (SwTemplNameFieldType*) pType );
            break;
        case RES_DBNEXTSETFLD:
            pFld = In_SwDBNextSetField( *this, (SwDBNextSetFieldType*) pType );
            break;
        case RES_DBNUMSETFLD:
            pFld = In_SwDBNumSetField( *this, (SwDBNumSetFieldType*) pType );
            break;
        case RES_DBSETNUMBERFLD:
            pFld = In_SwDBSetNumberField( *this, (SwDBSetNumberFieldType*) pType );
            break;
        case RES_EXTUSERFLD:
            pFld = In_SwExtUserField( *this, (SwExtUserFieldType*) pType );
            break;
    }
    r.skip( endfld );

    if( !r )
        delete pFld, pFld = NULL;
    if( pFld )
        pFld->ChangeFormat( nNewFldFmt );
    return pFld;
}

// Einlesen aller Feldtypen, die keine Systemtypen sind

void SwSwgReader::InFieldTypes()
{
    USHORT nFields;
    r >> nFields;
    for( USHORT i = 0; i < nFields && r.good(); i++ )
    {
        if( aHdr.nVersion >= SWG_VER_POOLIDS )
        {
            if( r.next() != SWG_DATA )
            {
                Error(); break;
            }
            long nextrec = r.getskip();
            InFieldType();
            r.skip( nextrec );
        } else InFieldType();
    }
    if( r.good() )
        r.next();
}

SwFieldType* SwSwgReader::InFieldType()
{
    BYTE ch;
    SwFieldType* p = NULL;
    r >> ch;
    ch += RES_FIELDS_BEGIN;
    switch( ch )
    {
        case RES_DBFLD:
            p = In_SwDBFieldType( *this ); break;
        case RES_USERFLD:
            p = In_SwUserFieldType( *this ); break;
        /*
            MS: Gibts nicht mehr

        case RES_REGFLD:
            p = In_SwRegFieldType( *this ); break;
        case RES_VARREGFLD:
            p = In_SwVarRegFieldType( *this ); break;
        */
        case RES_DDEFLD:
            p = In_SwDDEFieldType( *this ); break;
        case RES_SETEXPFLD:
            p = In_SwSetExpFieldType( *this ); break;
    }
    return p;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
