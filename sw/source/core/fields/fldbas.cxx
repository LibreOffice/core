/*************************************************************************
 *
 *  $RCSfile: fldbas.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:19 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop


#include <math.h>
#ifdef MAC
#include <stdlib.h>
#endif
#include <float.h>

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _ZFORMAT_HXX //autogen
#include <svtools/zformat.hxx>
#endif
#ifndef _TOOLS_SOLMATH_HXX //autogen wg. SolarMath
#include <tools/solmath.hxx>
#endif
#ifndef _TOOLS_INTN_HXX //autogen wg. International
#include <tools/intn.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _FRAME_HXX
#include <frame.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
using namespace ::com::sun::star;

USHORT lcl_GetLanguageOfFormat( USHORT nLng, ULONG nFmt,
                                const SvNumberFormatter& rFormatter )
{
    if( nLng == LANGUAGE_NONE ) // wegen Bug #60010
        nLng = LANGUAGE_SYSTEM;
    else if( nLng == ::GetSystemLanguage() )
        switch( rFormatter.GetIndexTableOffset( nFmt ))
        {
        case NF_NUMBER_SYSTEM:
        case NF_DATE_SYSTEM_SHORT:
        case NF_DATE_SYSTEM_LONG:
        case NF_DATETIME_SYSTEM_SHORT_HHMM:
            nLng = LANGUAGE_SYSTEM;
            break;
        }
    return nLng;
}

/*--------------------------------------------------------------------
    Beschreibung: Globals
 --------------------------------------------------------------------*/
// Array der Feldname

SvStringsDtor* SwFieldType::pFldNames = 0;

DBG_NAME(SwFieldType);

    USHORT __FAR_DATA aTypeTab[] = {
    /* RES_DBFLD            */      TYP_DBFLD,
    /* RES_USERFLD          */      TYP_USERFLD,
    /* RES_FILENAMEFLD      */      TYP_FILENAMEFLD,
    /* RES_DBNAMEFLD        */      TYP_DBNAMEFLD,
    /* RES_DATEFLD          */      TYP_DATEFLD,
    /* RES_TIMEFLD          */      TYP_TIMEFLD,
    /* RES_PAGENUMBERFLD    */      TYP_PAGENUMBERFLD,  // dynamisch
    /* RES_AUTHORFLD        */      TYP_AUTHORFLD,
    /* RES_CHAPTERFLD       */      TYP_CHAPTERFLD,
    /* RES_DOCSTATFLD       */      TYP_DOCSTATFLD,
    /* RES_GETEXPFLD        */      TYP_GETFLD,         // dynamisch
    /* RES_SETEXPFLD        */      TYP_SETFLD,         // dynamisch
    /* RES_GETREFFLD        */      TYP_GETREFFLD,
    /* RES_HIDDENTXTFLD     */      TYP_HIDDENTXTFLD,
    /* RES_POSTITFLD        */      TYP_POSTITFLD,
    /* RES_FIXDATEFLD       */      TYP_FIXDATEFLD,
    /* RES_FIXTIMEFLD       */      TYP_FIXTIMEFLD,
    /* RES_REGFLD           */      0,                  // alt
    /* RES_VARREGFLD        */      0,                  // alt
    /* RES_SETREFFLD        */      TYP_SETREFFLD,
    /* RES_INPUTFLD         */      TYP_INPUTFLD,
    /* RES_MACROFLD         */      TYP_MACROFLD,
    /* RES_DDEFLD           */      TYP_DDEFLD,
    /* RES_TABLEFLD         */      TYP_FORMELFLD,
    /* RES_HIDDENPARAFLD    */      TYP_HIDDENPARAFLD,
    /* RES_DOCINFOFLD       */      TYP_DOCINFOFLD,
    /* RES_TEMPLNAMEFLD     */      TYP_TEMPLNAMEFLD,
    /* RES_DBNEXTSETFLD     */      TYP_DBNEXTSETFLD,
    /* RES_DBNUMSETFLD      */      TYP_DBNUMSETFLD,
    /* RES_DBSETNUMBERFLD   */      TYP_DBSETNUMBERFLD,
    /* RES_EXTUSERFLD       */      TYP_EXTUSERFLD,
    /* RES_REFPAGESETFLD    */      TYP_SETREFPAGEFLD,
    /* RES_REFPAGEGETFLD    */      TYP_GETREFPAGEFLD,
    /* RES_INTERNETFLD      */      TYP_INTERNETFLD,
    /* RES_JUMPEDITFLD      */      TYP_JUMPEDITFLD,
    /* RES_SCRIPTFLD        */      TYP_SCRIPTFLD,
    /* RES_DATETIMEFLD      */      0,                  // dynamisch
    /* RES_AUTHORITY        */      TYP_AUTHORITY
    };
        // ????? TYP_USRINPFLD,



const String& SwFieldType::GetTypeStr(USHORT nTypeId)
{
    if( !pFldNames )
        _GetFldName();

    if( nTypeId < SwFieldType::pFldNames->Count() )
        return *SwFieldType::pFldNames->GetObject( nTypeId );
    else
        return aEmptyStr;
}

/*---------------------------------------------------
 Jedes Feld referenziert einen Feldtypen, der fuer
 jedes Dokument einmalig ist.
 --------------------------------------------------*/

SwFieldType::SwFieldType( USHORT nWhichId )
    : SwModify(0),
    nWhich( nWhichId )
{
    DBG_CTOR( SwFieldType, 0 );
}

#ifndef PRODUCT

SwFieldType::~SwFieldType()
{
    DBG_DTOR( SwFieldType, 0 );
}

#endif

const String& SwFieldType::GetName() const
{
    return aEmptyStr;
}

BOOL    SwFieldType::QueryValue( uno::Any& rVal, const String& rProperty ) const
{
    return FALSE;
}
BOOL    SwFieldType::PutValue( const uno::Any& rVal, const String& rProperty )
{
    return FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung:   Basisklasse aller Felder
                    Felder referenzieren einen Feldtyp
                    Felder sind n-mal vorhanden, Feldtypen nur einmal
 --------------------------------------------------------------------*/

SwField::SwField(SwFieldType* pTyp, ULONG nFmt, USHORT nLng) :
    nFormat(nFmt),
    nLang(nLng)
{
    ASSERT( pTyp, "SwField: ungueltiger SwFieldType" );
    pType = pTyp;
}

SwField::~SwField()
{
}

/*--------------------------------------------------------------------
    Beschreibung: Statt Umweg ueber den Typ
 --------------------------------------------------------------------*/

#ifndef PRODUCT
USHORT SwField::Which() const
{
    ASSERT(pType, "Kein Typ vorhanden");
    return pType->Which();
}
#endif

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwField::GetResId(USHORT nTypeId, BOOL& bAmbigous)
{
    static USHORT __READONLY_DATA aTypeTab[] = {
     RES_DATETIMEFLD   ,/*      TYP_DATEFLD,        */
     RES_DATETIMEFLD   ,/*      TYP_TIMEFLD,        */
     RES_FILENAMEFLD   ,/*      TYP_FILENAMEFLD,    */
     RES_DBNAMEFLD     ,/*      TYP_DBNAMEFLD,      */
     RES_CHAPTERFLD    ,/*      TYP_CHAPTERFLD,     */
     RES_PAGENUMBERFLD ,/*      TYP_PAGENUMBERFLD,  */// dynamisch
     RES_DOCSTATFLD    ,/*      TYP_DOCSTATFLD,     */
     RES_AUTHORFLD     ,/*      TYP_AUTHORFLD,      */
     RES_SETEXPFLD     ,/*      TYP_SETFLD,         */// dynamisch
     RES_GETEXPFLD     ,/*      TYP_GETFLD,         */// dynamisch
     RES_TABLEFLD      ,/*      TYP_FORMELFLD,      */
     RES_HIDDENTXTFLD  ,/*      TYP_HIDDENTXTFLD,   */
     RES_SETREFFLD     ,/*      TYP_SETREFFLD,      */
     RES_GETREFFLD     ,/*      TYP_GETREFFLD,      */
     RES_DDEFLD        ,/*      TYP_DDEFLD,         */
     RES_MACROFLD      ,/*      TYP_MACROFLD,       */
     RES_INPUTFLD      ,/*      TYP_INPUTFLD,       */
     RES_HIDDENPARAFLD ,/*      TYP_HIDDENPARAFLD,  */
     RES_DOCINFOFLD    ,/*      TYP_DOCINFOFLD,     */
     RES_DBFLD         ,/*      TYP_DBFLD,          */
     RES_USERFLD       ,/*      TYP_USERFLD,        */
     RES_POSTITFLD     ,/*      TYP_POSTITFLD,      */
     RES_TEMPLNAMEFLD  ,/*      TYP_TEMPLNAMEFLD,   */
     RES_SETEXPFLD     ,/*      TYP_SEQFLD          */
     RES_DBNEXTSETFLD  ,/*      TYP_DBNEXTSETFLD,   */
     RES_DBNUMSETFLD   ,/*      TYP_DBNUMSETFLD,    */
     RES_DBSETNUMBERFLD,/*      TYP_DBSETNUMBERFLD, */
     RES_HIDDENTXTFLD  ,/*      TYP_CONDTXTFLD      */
     RES_PAGENUMBERFLD ,/*      TYP_NEXTPAGEFLD     */
     RES_PAGENUMBERFLD ,/*      TYP_PREVPAGEFLD     */
     RES_EXTUSERFLD    ,/*      TYP_EXTUSERFLD      */
     RES_DATETIMEFLD   ,/*      TYP_FIXDATEFLD,     */
     RES_DATETIMEFLD   ,/*      TYP_FIXTIMEFLD,     */
     RES_SETEXPFLD     ,/*      TYP_SETINPFLD       */
     USHRT_MAX         ,/*      TYP_USRINPFLD       */
     RES_REFPAGESETFLD ,/*      TYP_SETREFPAGEFLD   */
     RES_REFPAGEGETFLD ,/*      TYP_GETREFPAGEFLD   */
     RES_INTERNETFLD   ,/*      TYP_INTERNETFLD     */
     RES_JUMPEDITFLD   ,/*      TYP_JUMPEDITFLD     */
     RES_SCRIPTFLD,     /*      TYP_SCRIPTFLD       */
     RES_AUTHORITY      /*      TYP_AUTHORITY       */
    };

    switch( nTypeId )
    {
        case TYP_CONDTXTFLD:
        case TYP_HIDDENTXTFLD:
        case TYP_USRINPFLD:
        case TYP_PAGENUMBERFLD:
        case TYP_PREVPAGEFLD:
        case TYP_NEXTPAGEFLD:
        case TYP_SEQFLD:
        case TYP_SETFLD:
        case TYP_SETINPFLD:
        case TYP_FORMELFLD:
            bAmbigous = TRUE;
            break;
        default: bAmbigous = FALSE;
    }

    return aTypeTab[ nTypeId ];
}



/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwField::GetTypeId() const
{

    USHORT nRet;
    switch( pType->Which() )
    {
    case RES_DATETIMEFLD:
        if (GetSubType() & FIXEDFLD)
            nRet = GetSubType() & DATEFLD ? TYP_FIXDATEFLD : TYP_FIXTIMEFLD;
        else
            nRet = GetSubType() & DATEFLD ? TYP_DATEFLD : TYP_TIMEFLD;
        break;
    case RES_GETEXPFLD:
        nRet = GSE_FORMULA & GetSubType() ? TYP_FORMELFLD : TYP_GETFLD;
        break;

    case RES_HIDDENTXTFLD:
        nRet = GetSubType();
        break;

    case RES_SETEXPFLD:
        if( GSE_SEQ & GetSubType() )
            nRet = TYP_SEQFLD;
        else if( ((SwSetExpField*)this)->GetInputFlag() )
            nRet = TYP_SETINPFLD;
        else
            nRet = TYP_SETFLD;
        break;

    case RES_PAGENUMBERFLD:
        nRet = GetSubType();
        if( PG_NEXT == nRet )
            nRet = TYP_NEXTPAGEFLD;
        else if( PG_PREV == nRet )
            nRet = TYP_PREVPAGEFLD;
        else
            nRet = TYP_PAGENUMBERFLD;
        break;

    default:
        nRet = aTypeTab[ pType->Which() ];
    }
    return nRet;
}


/*--------------------------------------------------------------------
    Beschreibung: liefert den Namen oder den Inhalt
 --------------------------------------------------------------------*/

String SwField::GetCntnt( BOOL bName ) const
{
    String sRet;
    if( bName )
    {
        USHORT nTypeId = GetTypeId();
        if( RES_DATETIMEFLD == GetTyp()->Which() )
            nTypeId = GetSubType() & DATEFLD ? TYP_DATEFLD : TYP_TIMEFLD;

        sRet = SwFieldType::GetTypeStr( nTypeId );
        if( IsFixed() )
            ( sRet += ' ' ) += ViewShell::GetShellRes()->aFixedStr;
    }
    else
        sRet = Expand();
    return sRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Parameter setzen auslesen
 --------------------------------------------------------------------*/

const String& SwField::GetPar1() const
{
    return aEmptyStr;
}

String SwField::GetPar2() const
{
    return aEmptyStr;
}

String SwField::GetFormula() const
{
    return GetPar2();
}

void SwField::SetPar1(const String& rStr)
{}

void SwField::SetPar2(const String& rStr)
{}

USHORT SwField::GetSubType() const
{
//  ASSERT(0, "Sorry Not implemented");
    return 0;
}

void SwField::SetSubType(USHORT nType)
{
//  ASSERT(0, "Sorry Not implemented");
}

BOOL  SwField::QueryValue( uno::Any& rVal, const String& rProperty ) const
{
    return FALSE;
}
BOOL SwField::PutValue( const uno::Any& rVal, const String& rProperty )
{
    return FALSE;
}


/*--------------------------------------------------------------------
    Beschreibung:   neuen Typ setzen
                    (wird fuer das Kopieren zwischen Dokumenten benutzt)
                    muss immer vom gleichen Typ sein.
 --------------------------------------------------------------------*/

SwFieldType* SwField::ChgTyp( SwFieldType* pNewType )
{
    ASSERT( pNewType && pNewType->Which() == pType->Which(),
            "kein Typ oder ungleiche Typen" );

    SwFieldType* pOld = pType;
    pType = pNewType;
    return pOld;
}

    // hat das Feld eine Action auf dem ClickHandler ? (z.B. INetFelder,..)
FASTBOOL SwField::HasClickHdl() const
{
    FASTBOOL bRet = FALSE;
    switch( pType->Which() )
    {
    case RES_INTERNETFLD:
    case RES_JUMPEDITFLD:
    case RES_GETREFFLD:
    case RES_MACROFLD:
    case RES_INPUTFLD:
        bRet = TRUE;
        break;

    case RES_SETEXPFLD:
        bRet = ((SwSetExpField*)this)->GetInputFlag();
        break;
    }
    return bRet;
}

void SwField::SetLanguage(USHORT nLng)
{
    nLang = nLng;
}

void SwField::ChangeFormat(ULONG n)
{
    nFormat = n;
}

FASTBOOL SwField::IsFixed() const
{
    FASTBOOL bRet = FALSE;
    switch( pType->Which() )
    {
    case RES_FIXDATEFLD:
    case RES_FIXTIMEFLD:
        bRet = TRUE;
        break;

    case RES_DATETIMEFLD:
        bRet = 0 != (GetSubType() & FIXEDFLD);
        break;

    case RES_EXTUSERFLD:
    case RES_AUTHORFLD:
        bRet = 0 != (GetFormat() & AF_FIXED);
        break;

    case RES_FILENAMEFLD:
        bRet = 0 != (GetFormat() & FF_FIXED);
        break;

    case RES_DOCINFOFLD:
        bRet = 0 != (GetSubType() & DI_SUB_FIXED);
        break;
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Sortierte Feldliste aller Felder usw.
 --------------------------------------------------------------------*/

SwFieldList::SwFieldList(SwEditShell* pShell)
    : pSh(pShell)
{
    // Hier die Liste aller Eingabefelder sortiert erstellen
    pSrtLst = new _SetGetExpFlds();
}

void SwFieldList::InsertFields(USHORT nTypeId, const String* pName)
{
    const USHORT nSize = pSh->GetFldTypeCount();

    // Alle Typen abklappern

    for(USHORT i=0; i < nSize; ++i)
    {
        SwFieldType* pFldType = pSh->GetFldType(i);
        if( nTypeId == pFldType->Which() )
        {
            if( pName )
            {
                String aMacTmp( pFldType->GetName() );
                if( *pName != aMacTmp )
                    continue;
            }

            SwClientIter aIter( *pFldType );
            for( SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE(SwFmtFld) );
                    pFld; pFld = (SwFmtFld*)aIter.Next() )
                if( pFld->GetTxtFld() )
                {
                    const SwTxtFld* pTxtFld = pFld->GetTxtFld();
                    const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
                    if( rTxtNode.GetNodes().IsDocNodes() )
                    {
                        SwNodeIndex aIdx( rTxtNode );
                        _SetGetExpFld* pNew = new _SetGetExpFld(aIdx, pTxtFld );
                        pSrtLst->Insert( pNew );
                    }
                }
        }
    }
}

SwFieldList::~SwFieldList()
{
    delete pSrtLst;
}

/*--------------------------------------------------------------------
    Beschreibung: Felder aus der Liste in sortierter Reihenfolge
 --------------------------------------------------------------------*/

USHORT SwFieldList::Count() const
{
    return pSrtLst->Count();
}

/*--------------------------------------------------------------------
    Beschreibung:   von der CursorPos das naechste und das Letzte Feld
                    in der Liste anfahren
 --------------------------------------------------------------------*/

SwField* SwFieldList::GetNextField() const
{
    SwPaM* pCrsr = pSh->GetCrsr();
    ULONG nNdPos = pCrsr->GetPoint()->nNode.GetIndex();
    USHORT nCntPos = pCrsr->GetPoint()->nContent.GetIndex();

    const USHORT nSize = pSrtLst->Count();
    for(USHORT i = 0; i < nSize; ++i )
    {
        _SetGetExpFld* pFnd = (*pSrtLst)[i];
        if( pFnd->GetNode() > nNdPos || ( pFnd->GetNode() == nNdPos &&
            pFnd->GetCntnt() >= nCntPos ))
            return (SwField*)pFnd->GetFld()->GetFld().GetFld();
    }
    return 0;
}

SwField* SwFieldList::GetLastField() const
{
    SwPaM* pCrsr = pSh->GetCrsr();
    ULONG nNdPos = pCrsr->GetPoint()->nNode.GetIndex();
    USHORT nCntPos = pCrsr->GetPoint()->nContent.GetIndex();

    for( USHORT n = pSrtLst->Count(); n; )
    {
        _SetGetExpFld* pFnd = (*pSrtLst)[ --n ];
        if( pFnd->GetNode() < nNdPos || ( pFnd->GetNode() == nNdPos &&
            pFnd->GetCntnt() <= nCntPos ))
            return (SwField*)pFnd->GetFld()->GetFld().GetFld();
    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: Numerierung expandieren
 --------------------------------------------------------------------*/

String FormatNumber(USHORT nNum, ULONG nFormat)
{
    SwNumType aNumber;

    ASSERT(nFormat != SVX_NUM_NUMBER_NONE, "Falsches Nummern-Format" );

    aNumber.eType = (SvxExtNumType)nFormat;
    return aNumber.GetNumStr(nNum);
}

/*--------------------------------------------------------------------
    Beschreibung: CTOR SwValueFieldType
 --------------------------------------------------------------------*/

SwValueFieldType::SwValueFieldType( SwDoc* pDocPtr, USHORT nWhichId )
    : SwFieldType(nWhichId),
    pDoc(pDocPtr),
    bUseFormat(TRUE)
{
}

SwValueFieldType::SwValueFieldType( const SwValueFieldType& rTyp )
    : SwFieldType(rTyp.Which()),
    pDoc(rTyp.GetDoc()),
    bUseFormat(rTyp.UseFormat())
{
}

/*--------------------------------------------------------------------
    Beschreibung: Wert formatiert als String zurueckgeben
 --------------------------------------------------------------------*/

String SwValueFieldType::ExpandValue( const double& rVal,
                                        ULONG nFmt, USHORT nLng) const
{
    if (rVal >= DBL_MAX)        // FehlerString fuer Calculator
        return ViewShell::GetShellRes()->aCalc_Error;

    String sExpand;
    SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
    Color* pCol = 0;

    // wegen Bug #60010
    USHORT nFmtLng = ::lcl_GetLanguageOfFormat( nLng, nFmt, *pFormatter );

    if( nFmt < SV_COUNTRY_LANGUAGE_OFFSET && LANGUAGE_SYSTEM != nFmtLng )
    {
        short nType = NUMBERFORMAT_DEFINED;
        xub_StrLen nDummy;

        const SvNumberformat* pEntry = pFormatter->GetEntry(nFmt);

        if (pEntry && nLng != pEntry->GetLanguage())
        {
            ULONG nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nFmt,
                                                    (LanguageType)nFmtLng);

            if (nNewFormat == nFmt)
            {
                // Warscheinlich benutzerdefiniertes Format
                String sFmt(pEntry->GetFormatstring());

                pFormatter->PutandConvertEntry(sFmt, nDummy, nType, nFmt,
                                        pEntry->GetLanguage(), nFmtLng );
            }
            else
                nFmt = nNewFormat;
        }
        ASSERT(pEntry, "Unbekanntes Zahlenformat!");
    }

    if( pFormatter->IsTextFormat( nFmt ) )
    {
        String sValue;
        DoubleToString(sValue, rVal, nFmtLng);
        pFormatter->GetOutputString(sValue, nFmt, sExpand, &pCol);
    }
    else
        pFormatter->GetOutputString(rVal, nFmt, sExpand, &pCol);

    return sExpand;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwValueFieldType::DoubleToString( String &rValue, const double &rVal,
                                        ULONG nFmt) const
{
    SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
    const SvNumberformat* pEntry = pFormatter->GetEntry(nFmt);

    if (pEntry)
        DoubleToString(rValue, rVal, pEntry->GetLanguage());
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwValueFieldType::DoubleToString( String &rValue, const double &rVal,
                                        USHORT nLng ) const
{
    SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();

    // wegen Bug #60010
    if( nLng == LANGUAGE_NONE ) // wegen Bug #60010
        nLng = LANGUAGE_SYSTEM;

    pFormatter->ChangeIntl( nLng ); // Separator in der richtigen Sprache besorgen
    rValue.Erase();
    SolarMath::DoubleToString( rValue, rVal, 'F', 12,
                                    pFormatter->GetDecSep(), TRUE );
}

/*--------------------------------------------------------------------
    Beschreibung: CTOR SwValueField
 --------------------------------------------------------------------*/

SwValueField::SwValueField( SwValueFieldType* pFldType, ULONG nFmt,
                            USHORT nLang, const double fVal )
    : SwField(pFldType, nFmt, nLang),
    fValue(fVal)
{
}

SwValueField::SwValueField( const SwValueField& rFld )
    : SwField(rFld),
    fValue(rFld.GetValue())
{
}

/*--------------------------------------------------------------------
    Beschreibung:   neuen Typ setzen
                    (wird fuer das Kopieren zwischen Dokumenten benutzt)
                    muss immer vom gleichen Typ sein.
 --------------------------------------------------------------------*/

SwFieldType* SwValueField::ChgTyp( SwFieldType* pNewType )
{
    SwDoc* pNewDoc = ((SwValueFieldType *)pNewType)->GetDoc();
    SwDoc* pDoc    = GetDoc();

    if( pNewDoc && pDoc && pDoc != pNewDoc)
    {
        SvNumberFormatter* pFormatter = pNewDoc->GetNumberFormatter();

        if( pFormatter && pFormatter->HasMergeFmtTbl() &&
            ((SwValueFieldType *)GetTyp())->UseFormat() )
            SetFormat(pFormatter->GetMergeFmtIndex( GetFormat() ));
    }

    return SwField::ChgTyp(pNewType);
}

/*--------------------------------------------------------------------
    Beschreibung: Format aendern
 --------------------------------------------------------------------*/
/*
 Was sollte das denn?
void SwValueField::ChangeFormat(ULONG n)
{
    nFormat = n;
}

/*--------------------------------------------------------------------
    Beschreibung: Format in Office-Sprache ermitteln
 --------------------------------------------------------------------*/

ULONG SwValueField::GetSystemFormat(SvNumberFormatter* pFormatter, ULONG nFmt)
{
    const SvNumberformat* pEntry = pFormatter->GetEntry(nFmt);
    const International& rInter = Application::GetAppInternational();
    USHORT nLng = rInter.GetLanguage();

    if (pEntry && nLng != pEntry->GetLanguage())
    {
        ULONG nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nFmt,
                                                        (LanguageType)nLng);

        if (nNewFormat == nFmt)
        {
            // Warscheinlich benutzerdefiniertes Format
            short nType = NUMBERFORMAT_DEFINED;
            xub_StrLen nDummy;

            String sFmt(pEntry->GetFormatstring());

            ULONG nFormat = nFmt;
            pFormatter->PutandConvertEntry(sFmt, nDummy, nType,
                                nFormat, pEntry->GetLanguage(), nLng);
            nFmt = nFormat;
        }
        else
            nFmt = nNewFormat;
    }

    return nFmt;
}

/*--------------------------------------------------------------------
    Beschreibung: Sprache im Format anpassen
 --------------------------------------------------------------------*/

void SwValueField::SetLanguage( USHORT nLng )
{
    if( ((SwValueFieldType *)GetTyp())->UseFormat() &&
        GetFormat() != ULONG_MAX )
    {
        // wegen Bug #60010
        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
        USHORT nFmtLng = ::lcl_GetLanguageOfFormat( nLng, GetFormat(),
                                                    *pFormatter );

        if( (GetFormat() >= SV_COUNTRY_LANGUAGE_OFFSET ||
             LANGUAGE_SYSTEM != nFmtLng ) &&
            !(Which() == RES_USERFLD && (GetSubType()&SUB_CMD) ) )
        {
            const SvNumberformat* pEntry = pFormatter->GetEntry(GetFormat());

            if( pEntry && nFmtLng != pEntry->GetLanguage() )
            {
                ULONG nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(
                                        GetFormat(), (LanguageType)nFmtLng );

                if( nNewFormat == GetFormat() )
                {
                    // Warscheinlich benutzerdefiniertes Format
                    short nType = NUMBERFORMAT_DEFINED;
                    xub_StrLen nDummy;
                    String sFmt( pEntry->GetFormatstring() );
                    pFormatter->PutandConvertEntry( sFmt, nDummy, nType,
                                                    nNewFormat,
                                                    pEntry->GetLanguage(),
                                                    nFmtLng );
                }
                SetFormat( nNewFormat );
            }
            ASSERT(pEntry, "Unbekanntes Zahlenformat!");
        }
    }

    SwField::SetLanguage(nLng);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

double SwValueField::GetValue() const
{
    return fValue;
}

void SwValueField::SetValue( const double& rVal )
{
    fValue = rVal;
}

/*--------------------------------------------------------------------
    Beschreibung: SwFormulaField
 --------------------------------------------------------------------*/

SwFormulaField::SwFormulaField( SwValueFieldType* pFldType, ULONG nFmt, const double fVal)
    : SwValueField(pFldType, nFmt, LANGUAGE_SYSTEM, fVal)
{
}

SwFormulaField::SwFormulaField( const SwFormulaField& rFld )
    : SwValueField((SwValueFieldType *)rFld.GetTyp(), rFld.GetFormat(),
                    rFld.GetLanguage(), rFld.GetValue())
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

String SwFormulaField::GetFormula() const
{
    return sFormula;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFormulaField::SetFormula(const String& rStr)
{
    sFormula = rStr;

    ULONG nFmt(GetFormat());

    if( nFmt && ULONG_MAX != nFmt )
    {
        // String muss im Systemformat vorliegen!
        // Geht bei Import/Export mit unterschiedlichen Systemformaten
        // voll in die Hose. Ist aber nicht anders moeglich, da der Kalkulator
        // nur im Systemformat rechnen kann und es keine Konvertierungsroutine
        // gibt.
        const International& rInter = Application::GetAppInternational();
        int nErrno;
        double fValue = SolarMath::StringToDouble( rStr.GetBuffer(),
                                                rInter.GetNumThousandSep(),
                                                rInter.GetNumDecimalSep(),
                                                nErrno );
        if( !nErrno )
            SwValueField::SetValue( fValue );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFormulaField::SetExpandedFormula( const String& rStr )
{
    ULONG nFmt(GetFormat());

    if (nFmt && nFmt != ULONG_MAX && ((SwValueFieldType *)GetTyp())->UseFormat())
    {
        double fValue;

        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

        if (pFormatter->IsNumberFormat(rStr, nFmt, fValue))
        {
            SwValueField::SetValue(fValue);
            sFormula.Erase();

            ((SwValueFieldType *)GetTyp())->DoubleToString(sFormula, fValue, nFmt);
            return;
        }
    }
    sFormula = rStr;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

String SwFormulaField::GetExpandedFormula() const
{
    ULONG nFmt(GetFormat());

    if (nFmt && nFmt != ULONG_MAX && ((SwValueFieldType *)GetTyp())->UseFormat())
    {
        String sFormattedValue;
        Color* pCol = 0;

        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

        if (pFormatter->IsTextFormat(nFmt))
        {
            String sValue;
            ((SwValueFieldType *)GetTyp())->DoubleToString(sValue, GetValue(), nFmt);
            pFormatter->GetOutputString(sValue, nFmt, sFormattedValue, &pCol);
        }
        else
            pFormatter->GetOutputString(GetValue(), nFmt, sFormattedValue, &pCol);

        return sFormattedValue;
    }
    else
        return GetFormula();
}
