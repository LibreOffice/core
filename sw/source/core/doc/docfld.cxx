/*************************************************************************
 *
 *  $RCSfile: docfld.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#include <string.h>
#include <float.h>

#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_ULONGS
#include <svtools/svarray.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SO2REF_HXX //autogen
#include <so3/so2ref.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _CALC_HXX
#include <calc.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>   // fuer Expression-Felder
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _CHPFLD_HXX
#include <chpfld.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _AUTHFLD_HXX
#include <authfld.hxx>
#endif

#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>      // fuer InitFldTypes
#endif


#ifndef SO2_DECL_SVLINKNAME_DEFINED
#define SO2_DECL_SVLINKNAME_DEFINED
SO2_DECL_REF(SvLinkName)
#endif

extern BOOL IsFrameBehind( const SwTxtNode& rMyNd, USHORT nMySttPos,
                        const SwTxtNode& rBehindNd, USHORT nSttPos );

SV_IMPL_OP_PTRARR_SORT( _SetGetExpFlds, _SetGetExpFldPtr )


#ifdef UNX
#define DBNAME_COMPARE  0
#define DBNAME_LOWER(s) s
#else
#define DBNAME_COMPARE  INTN_COMPARE_IGNORECASE
#define DBNAME_LOWER(s) rCC.lower( s )
#endif

/*--------------------------------------------------------------------
    Beschreibung: Feldtypen einfuegen
 --------------------------------------------------------------------*/
/*
 *  Implementierung der Feldfunktionen am Doc
 *  Return immer einen gueltigen Pointer auf den Typ. Wenn er also neu
 *  zugefuegt oder schon vorhanden ist.
 */

SwFieldType* SwDoc::InsertFldType(const SwFieldType &rFldTyp)
{
    USHORT nSize = pFldTypes->Count(),
            nFldWhich = rFldTyp.Which();

    USHORT i = INIT_FLDTYPES;

    switch( nFldWhich )
    {
    case RES_SETEXPFLD:
            //JP 29.01.96: SequenceFelder beginnen aber bei INIT_FLDTYPES - 3!!
            //             Sonst gibt es doppelte Nummernkreise!!
            //MIB 14.03.95: Ab sofort verlaesst sich auch der SW3-Reader
            //beim Aufbau der String-Pools und beim Einlesen von SetExp-Feldern
            //hierauf
            if( GSE_SEQ & ((SwSetExpFieldType&)rFldTyp).GetType() )
                i -= INIT_SEQ_FLDTYPES;
        // kein break;
    case RES_DBFLD:
    case RES_USERFLD:
    case RES_DDEFLD:
        {
            String sFldNm( rFldTyp.GetName() );
            const CharClass& rCC = GetAppCharClass();
            rCC.toLower( sFldNm );

            for( ; i < nSize; ++i )
                if( nFldWhich == (*pFldTypes)[i]->Which() )
                {
                    String sCmpStr( (*pFldTypes)[i]->GetName() );
                    rCC.toLower( sCmpStr );
                    if( sCmpStr.Equals( sFldNm ))
                        return (*pFldTypes)[i];
                }
        }
        break;

    case RES_AUTHORITY:
        for( ; i < nSize; ++i )
            if( nFldWhich == (*pFldTypes)[i]->Which() )
                return (*pFldTypes)[i];
        break;

    default:
        for( i = 0; i < nSize; ++i )
            if( nFldWhich == (*pFldTypes)[i]->Which() )
                return (*pFldTypes)[i];
    }

    SwFieldType* pNew = rFldTyp.Copy();
    switch( nFldWhich )
    {
    case RES_DDEFLD:
        ((SwDDEFieldType*)pNew)->SetDoc( this );
        break;

    case RES_DBFLD:
    case RES_TABLEFLD:
    case RES_DATETIMEFLD:
    case RES_GETEXPFLD:
        ((SwValueFieldType*)pNew)->SetDoc( this );
        break;

    case RES_USERFLD:
    case RES_SETEXPFLD:
        ((SwValueFieldType*)pNew)->SetDoc( this );
        // JP 29.07.96: opt. FeldListe fuer den Calculator vorbereiten:
        pUpdtFlds->InsertFldType( *pNew );
        break;
    case RES_AUTHORITY :
        ((SwAuthorityFieldType*)pNew)->SetDoc( this );
        break;
    }

    pFldTypes->Insert( pNew, nSize );
    SetModified();

    return (*pFldTypes)[ nSize ];
}

void SwDoc::InsDeletedFldType( SwFieldType& rFldTyp )
{
    // der FeldTyp wurde als geloescht gekennzeichnet und aus dem
    // Array entfernt. Nun muss man nach diesem wieder suchen.
    // - Ist der nicht vorhanden, dann kann er eingefuegt werden.
    // - Wird genau der gleiche Typ gefunden, dann muss der geloeschte
    //   einen anderen Namen erhalten.

    USHORT nSize = pFldTypes->Count(), nFldWhich = rFldTyp.Which();
    USHORT i = INIT_FLDTYPES;

    ASSERT( RES_SETEXPFLD == nFldWhich ||
            RES_USERFLD == nFldWhich ||
            RES_DDEFLD == nFldWhich, "Falscher FeldTyp" );

    const CharClass& rCC = GetAppCharClass();
    const String& rFldNm = rFldTyp.GetName();
    String sLowerFldNm( rCC.lower( rFldNm ));
    SwFieldType* pFnd;

    for( ; i < nSize; ++i )
        if( nFldWhich == (pFnd = (*pFldTypes)[i])->Which() &&
            sLowerFldNm.Equals( rCC.lower( pFnd->GetName() )) )
        {
            // neuen Namen suchen
            USHORT nNum = 1;
            do {
                String sSrch( sLowerFldNm );
                sSrch.Append( String::CreateFromInt32( nNum ));
                for( i = INIT_FLDTYPES; i < nSize; ++i )
                    if( nFldWhich == (pFnd = (*pFldTypes)[i])->Which() &&
                        sSrch.Equals( rCC.lower( pFnd->GetName() ) ))
                        break;

                if( i >= nSize )        // nicht gefunden
                {
                    ((String&)rFldNm).Append( String::CreateFromInt32( nNum ));
                    break;      // raus aus der While-Schleife
                }
                ++nNum;
            } while( TRUE );
            break;
        }

    // nicht gefunden, also eintragen und Flag loeschen
    pFldTypes->Insert( &rFldTyp, nSize );
    switch( nFldWhich )
    {
    case RES_SETEXPFLD:
        ((SwSetExpFieldType&)rFldTyp).SetDeleted( FALSE );
        break;
    case RES_USERFLD:
        ((SwUserFieldType&)rFldTyp).SetDeleted( FALSE );
        break;
    case RES_DDEFLD:
        ((SwDDEFieldType&)rFldTyp).SetDeleted( FALSE );
        break;
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Feldtypen loeschen
 --------------------------------------------------------------------*/

void SwDoc::RemoveFldType(USHORT nFld)
{
    ASSERT( INIT_FLDTYPES <= nFld,  "keine InitFields loeschen" );
    /*
     * Abheangige Felder vorhanden -> ErrRaise
     */
    USHORT nSize = pFldTypes->Count();
    if(nFld < nSize)
    {
        SwFieldType* pTmp = (*pFldTypes)[nFld];

        // JP 29.07.96: opt. FeldListe fuer den Calculator vorbereiten:
        USHORT nWhich = pTmp->Which();
        switch( nWhich )
        {
        case RES_SETEXPFLD:
        case RES_USERFLD:
            pUpdtFlds->RemoveFldType( *pTmp );
            // kein break;
        case RES_DDEFLD:
            if( pTmp->GetDepends() && !IsUsed( *pTmp ) )
            {
                if( RES_SETEXPFLD == nWhich )
                    ((SwSetExpFieldType*)pTmp)->SetDeleted( TRUE );
                else if( RES_USERFLD == nWhich )
                    ((SwUserFieldType*)pTmp)->SetDeleted( TRUE );
                else
                    ((SwDDEFieldType*)pTmp)->SetDeleted( TRUE );
                nWhich = 0;
            }
            break;
        }

        if( nWhich )
        {
            ASSERT( !pTmp->GetDepends(), "Abhaengige vorh.!" );
            // Feldtype loschen
            delete pTmp;
        }
        pFldTypes->Remove( nFld );
        SetModified();
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Den ersten Typen mit ResId und Namen finden
 --------------------------------------------------------------------*/

SwFieldType* SwDoc::GetFldType( USHORT nResId, const String& rName ) const
{
    USHORT nSize = pFldTypes->Count(), i = 0;
    const CharClass& rCC = GetAppCharClass();

    String aTmp2( rName );
    rCC.toLower( aTmp2 );

    switch( nResId )
    {
    case RES_SETEXPFLD:
            //JP 29.01.96: SequenceFelder beginnen aber bei INIT_FLDTYPES - 3!!
            //             Sonst gibt es doppelte Nummernkreise!!
            //MIB 14.03.95: Ab sofort verlaesst sich auch der SW3-Reader
            //beim Aufbau der String-Pools und beim Einlesen von SetExp-Feldern
            //hierauf
        i = INIT_FLDTYPES - INIT_SEQ_FLDTYPES;
        break;

    case RES_DBFLD:
    case RES_USERFLD:
    case RES_DDEFLD:
    case RES_AUTHORITY:
        i = INIT_FLDTYPES;
        break;
    }

    SwFieldType* pRet = 0;
    for( ; i < nSize; ++i )
    {
        SwFieldType* pFldType = (*pFldTypes)[i];
        if( nResId == pFldType->Which() )
        {
            String aTmp( pFldType->GetName() );
            rCC.toLower( aTmp );

            if( aTmp2.Equals( aTmp ))
            {
                pRet = pFldType;
                break;
            }
        }
    }
    return pRet;
}


/*************************************************************************
|*                SwDoc::UpdateFlds()
|*    Beschreibung      Felder updaten
*************************************************************************/
/*
 *    Alle sollen neu evaluiert werden.
 */

void SwDoc::UpdateFlds( SfxPoolItem *pNewHt, BOOL bCloseDB )
{
    // Modify() fuer jeden Feldtypen rufen,
    // abhaengige SwTxtFld werden benachrichtigt ...

    for( USHORT i=0; i < pFldTypes->Count(); ++i)
    {
        switch( (*pFldTypes)[i]->Which() )
        {
            // Tabellen-Felder als vorletztes Updaten
            // Referenzen als letztes Updaten
        case RES_GETREFFLD:
        case RES_TABLEFLD:
        case RES_DBFLD:
        case RES_JUMPEDITFLD:
        case RES_REFPAGESETFLD:     // werden nie expandiert!
            break;

        case RES_DDEFLD:
        {
            if( !pNewHt )
            {
                SwMsgPoolItem aUpdateDDE( RES_UPDATEDDETBL );
                (*pFldTypes)[i]->Modify( 0, &aUpdateDDE );
            }
            else
                (*pFldTypes)[i]->Modify( 0, pNewHt );
            break;
        }
        case RES_GETEXPFLD:
        case RES_SETEXPFLD:
        case RES_HIDDENTXTFLD:
        case RES_HIDDENPARAFLD:
            // Expression-Felder werden gesondert behandelt
            if( !pNewHt )
                break;
        default:
            (*pFldTypes)[i]->Modify( 0, pNewHt );
        }
    }

    if( !IsExpFldsLocked() )
        UpdateExpFlds( 0, FALSE );      // Expression-Felder Updaten

    // Tabellen
    UpdateTblFlds(pNewHt);

    // Referenzen
    UpdateRefFlds(pNewHt);

    if( bCloseDB )
        GetNewDBMgr()->CloseAll();

    // Nur bei KomplettUpdate evaluieren
    SetModified();
}

/******************************************************************************
 *                      void SwDoc::UpdateUsrFlds()
 ******************************************************************************/

void SwDoc::UpdateUsrFlds()
{
    SwCalc* pCalc = 0;
    const SwFieldType* pFldType;
    for( USHORT i = INIT_FLDTYPES; i < pFldTypes->Count(); ++i )
        if( RES_USERFLD == ( pFldType = (*pFldTypes)[i] )->Which() )
        {
            if( !pCalc )
                pCalc = new SwCalc( *this );
            ((SwUserFieldType*)pFldType)->GetValue( *pCalc );
        }

    if( pCalc )
    {
        delete pCalc;
        SetModified();
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Referenzfelder und TableFelder erneuern
 --------------------------------------------------------------------*/

void SwDoc::UpdateRefFlds( SfxPoolItem* pHt )
{
    SwFieldType* pFldType;
    for( USHORT i = 0; i < pFldTypes->Count(); ++i )
        if( RES_GETREFFLD == ( pFldType = (*pFldTypes)[i] )->Which() )
            pFldType->Modify( 0, pHt );
}

void SwDoc::UpdateTblFlds( SfxPoolItem* pHt )
{
    ASSERT( !pHt || RES_TABLEFML_UPDATE  == pHt->Which(),
            "Was ist das fuer ein MessageItem?" );

    SwFieldType* pFldType;
    for( USHORT i = 0; i < pFldTypes->Count(); ++i )
    {
        if( RES_TABLEFLD == ( pFldType = (*pFldTypes)[i] )->Which() )
        {
            SwTableFmlUpdate* pUpdtFld = 0;
            if( pHt && RES_TABLEFML_UPDATE == pHt->Which() )
                pUpdtFld = (SwTableFmlUpdate*)pHt;

            SwClientIter aIter( *pFldType );
            for( SwFmtFld* pFmtFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
                    pFmtFld; pFmtFld = (SwFmtFld*)aIter.Next() )
                if( pFmtFld->GetTxtFld() )
                {
                    SwTblField* pFld = (SwTblField*)pFmtFld->GetFld();

                    if( pUpdtFld )
                    {
                        // bestimme Tabelle, in der das Feld steht
                        const SwTableNode* pTblNd;
                        const SwTxtNode& rTxtNd = pFmtFld->GetTxtFld()->GetTxtNode();
                        if( !rTxtNd.GetNodes().IsDocNodes() ||
                            0 == ( pTblNd = rTxtNd.FindTableNode() ) )
                            continue;

                        switch( pUpdtFld->eFlags )
                        {
                        case TBL_CALC:
                            // setze das Value-Flag zurueck
                            // JP 17.06.96: interne Darstellung auf alle Formeln
                            //              (Referenzen auf andere Tabellen!!!)
                            if( SUB_CMD & pFld->GetSubType() )
                                pFld->PtrToBoxNm( pUpdtFld->pTbl );
                            else
                                pFld->ChgValid( FALSE );
                            break;
                        case TBL_BOXNAME:
                            // ist es die gesuchte Tabelle ??
                            if( &pTblNd->GetTable() == pUpdtFld->pTbl )
                                // zur externen Darstellung
                                pFld->PtrToBoxNm( pUpdtFld->pTbl );
                            break;
                        case TBL_BOXPTR:
                            // zur internen Darstellung
                            // JP 17.06.96: interne Darstellung auf alle Formeln
                            //              (Referenzen auf andere Tabellen!!!)
                            pFld->BoxNmToPtr( pUpdtFld->pTbl );
                            break;
                        case TBL_RELBOXNAME:
                            // ist es die gesuchte Tabelle ??
                            if( &pTblNd->GetTable() == pUpdtFld->pTbl )
                                // zur relativen Darstellung
                                pFld->ToRelBoxNm( pUpdtFld->pTbl );
                            break;
                        }
                    }
                    else
                        // setze bei allen das Value-Flag zurueck
                        pFld->ChgValid( FALSE );
                }

            break;
        }
        pFldType = 0;
    }

    // und dann noch alle Tabellen Box Formeln abklappern
    const SfxPoolItem* pItem;
    USHORT nMaxItems = GetAttrPool().GetItemCount( RES_BOXATR_FORMULA );
    for( i = 0; i < nMaxItems; ++i )
        if( 0 != (pItem = GetAttrPool().GetItem( RES_BOXATR_FORMULA, i ) ) &&
            ((SwTblBoxFormula*)pItem)->GetDefinedIn() )
        {
            ((SwTblBoxFormula*)pItem)->ChangeState( pHt );
        }


    // alle Felder/Boxen sind jetzt invalide, also kann das Rechnen anfangen
    if( pHt && ( RES_TABLEFML_UPDATE != pHt->Which() ||
                TBL_CALC != ((SwTableFmlUpdate*)pHt)->eFlags ))
        return ;

    SwCalc* pCalc = 0;

    if( pFldType )
    {
        SwClient* pLast;
        SwClientIter aIter( *pFldType );
        // dann rechne mal schoen
        // JP 27.03.97: Beim Berechnen am Ende anfangen - weil neue
        //              Felder immer am Anfang der Modifykette eingefuegt
        //              werden. Beim Import haben wir damit eine bessere/
        //              schnellere Berechnung bei "Kettenformeln"
        if( 0 != ( pLast = aIter.GoEnd() ))
            do {
                SwFmtFld* pFmtFld = (SwFmtFld*)pLast;
                SwTblField* pFld;
                if( !pFmtFld->GetTxtFld() || (SUB_CMD &
                    (pFld = (SwTblField*)pFmtFld->GetFld())->GetSubType() ))
                    continue;

                // muss neu berechnet werden (und ist keine textuelle Anzeige)
                if( !pFld->IsValid() )
                {
                    // bestimme Tabelle, in der das Feld steht
                    const SwTxtNode& rTxtNd = pFmtFld->GetTxtFld()->GetTxtNode();
                    if( !rTxtNd.GetNodes().IsDocNodes() )
                        continue;
                    const SwTableNode* pTblNd = rTxtNd.FindTableNode();
                    if( !pTblNd )
                        continue;

                    // falls dieses Feld nicht in der zu updatenden
                    // Tabelle steht, ueberspringen !!
                    if( pHt && &pTblNd->GetTable() !=
                                            ((SwTableFmlUpdate*)pHt)->pTbl )
                        continue;

                    if( !pCalc )
                        pCalc = new SwCalc( *this );

                    // bestimme die Werte aller SetExpresion Felder, die
                    // bis zur Tabelle gueltig sind
                    SwFrm* pFrm = 0;
                    if( pTblNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        // steht im Sonderbereich, wird teuer !!
                        Point aPt;      // den im Layout 1. Frame returnen - Tab.Kopfzeile !!
                        pFrm = rTxtNd.GetFrm( &aPt );
                        if( pFrm )
                        {
                            SwPosition aPos( *pTblNd );
                            if( GetBodyTxtNode( *this, aPos, *pFrm ) )
                                FldsToCalc( *pCalc, _SetGetExpFld(
                                    aPos.nNode, pFmtFld->GetTxtFld(),
                                    &aPos.nContent ));
                            else
                                pFrm = 0;
                        }
                    }
                    if( !pFrm )
                    {
                        // einen Index fuers bestimmen vom TextNode anlegen
                        SwNodeIndex aIdx( rTxtNd );
                        FldsToCalc( *pCalc,
                            _SetGetExpFld( aIdx, pFmtFld->GetTxtFld() ));
                    }

                    SwTblCalcPara aPara( *pCalc, pTblNd->GetTable() );
                    pFld->CalcField( aPara );
                    if( aPara.IsStackOverFlow() )
                    {
                        if( aPara.CalcWithStackOverflow() )
                            pFld->CalcField( aPara );
#ifndef PRODUCT
                        else
                        {
                            // mind. ein ASSERT
                            ASSERT( !this, "die Kettenformel konnte nicht errechnet werden" );
                        }
#endif
                    }
                    pCalc->SetCalcError( CALC_NOERR );
                }
                pFmtFld->Modify( 0, pHt );
            } while( 0 != ( pLast = aIter-- ));
    }

    // dann berechene noch die Formeln an den Boxen
    for( i = 0; i < nMaxItems; ++i )
        if( 0 != (pItem = GetAttrPool().GetItem( RES_BOXATR_FORMULA, i ) ) &&
            ((SwTblBoxFormula*)pItem)->GetDefinedIn() &&
            !((SwTblBoxFormula*)pItem)->IsValid() )
        {
            SwTblBoxFormula* pFml = (SwTblBoxFormula*)pItem;
            SwTableBox* pBox = pFml->GetTableBox();
            if( pBox && pBox->GetSttNd() &&
                pBox->GetSttNd()->GetNodes().IsDocNodes() )
            {
                const SwTableNode* pTblNd = pBox->GetSttNd()->FindTableNode();
                if( !pHt || &pTblNd->GetTable() ==
                                            ((SwTableFmlUpdate*)pHt)->pTbl )
                {
                    double nValue;
                    if( !pCalc )
                        pCalc = new SwCalc( *this );

                    // bestimme die Werte aller SetExpresion Felder, die
                    // bis zur Tabelle gueltig sind
                    SwFrm* pFrm = 0;
                    if( pTblNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        // steht im Sonderbereich, wird teuer !!
                        Point aPt;      // den im Layout 1. Frame returnen - Tab.Kopfzeile !!
                        SwNodeIndex aCNdIdx( *pTblNd, +2 );
                        SwCntntNode* pCNd = aCNdIdx.GetNode().GetCntntNode();
                        if( !pCNd )
                            pCNd = GetNodes().GoNext( &aCNdIdx );

                        if( pCNd && 0 != (pFrm = pCNd->GetFrm( &aPt )) )
                        {
                            SwPosition aPos( *pCNd );
                            if( GetBodyTxtNode( *this, aPos, *pFrm ) )
                                FldsToCalc( *pCalc, _SetGetExpFld( aPos.nNode ));
                            else
                                pFrm = 0;
                        }
                    }
                    if( !pFrm )
                    {
                        // einen Index fuers bestimmen vom TextNode anlegen
                        SwNodeIndex aIdx( *pTblNd );
                        FldsToCalc( *pCalc, _SetGetExpFld( aIdx ));
                    }

                    SwTblCalcPara aPara( *pCalc, pTblNd->GetTable() );
                    pFml->Calc( aPara, nValue );

                    if( aPara.IsStackOverFlow() )
                    {
                        if( aPara.CalcWithStackOverflow() )
                            pFml->Calc( aPara, nValue );
#ifndef PRODUCT
                        else
                        {
                            // mind. ein ASSERT
                            ASSERT( !this, "die Kettenformel konnte nicht errechnet werden" );
                        }
#endif
                    }

                    SwFrmFmt* pFmt = pBox->ClaimFrmFmt();
                    SfxItemSet aTmp( GetAttrPool(),
                                    RES_BOXATR_BEGIN,RES_BOXATR_END-1 );

                    if( pCalc->IsCalcError() )
                        nValue = DBL_MAX;
                    aTmp.Put( SwTblBoxValue( nValue ));
                    if( SFX_ITEM_SET != pFmt->GetItemState( RES_BOXATR_FORMAT ))
                        aTmp.Put( SwTblBoxNumFormat( 0 ));
                    pFmt->SetAttr( aTmp );

                    pCalc->SetCalcError( CALC_NOERR );
                }
            }
        }

    if( pCalc )
        delete pCalc;
}

void SwDoc::UpdatePageFlds( SfxPoolItem* pMsgHnt )
{
    SwFieldType* pFldType;
    for( USHORT i = 0; i < INIT_FLDTYPES; ++i )
        switch( ( pFldType = (*pFldTypes)[ i ] )->Which() )
        {
        case RES_PAGENUMBERFLD:
        case RES_CHAPTERFLD:
        case RES_GETEXPFLD:
        case RES_REFPAGEGETFLD:
            pFldType->Modify( 0, pMsgHnt );
            break;
        case RES_DOCSTATFLD:
            pFldType->Modify( 0, 0 );
            break;
        }
    SetNewFldLst();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

// ---- Loesche alle nicht referenzierten FeldTypen eines Dokumentes --
void SwDoc::GCFieldTypes()
{
    for( register USHORT n = pFldTypes->Count(); n > INIT_FLDTYPES; )
        if( !(*pFldTypes)[ --n ]->GetDepends() )
            RemoveFldType( n );
}


//----------------------------------------------------------------------

// der StartIndex kann optional mit angegeben werden (z.B. wenn dieser
// zuvor schon mal erfragt wurde - ist sonst eine virtuelle Methode !!)

_SetGetExpFld::_SetGetExpFld( const SwNodeIndex& rNdIdx, const SwTxtFld* pFld,
                            const SwIndex* pIdx )
{
    eSetGetExpFldType = TEXTFIELD;
    CNTNT.pTxtFld = pFld;
    nNode = rNdIdx.GetIndex();
    if( pIdx )
        nCntnt = pIdx->GetIndex();
    else if( pFld )
        nCntnt = *pFld->GetStart();
    else
        nCntnt = 0;
}

    //Erweiterung fuer Sections:
    //  diese haben immer als Content-Position 0xffff !!
    //  Auf dieser steht nie ein Feld, maximal bis STRING_MAXLEN moeglich
_SetGetExpFld::_SetGetExpFld( const SwSectionNode& rSectNd,
                                const SwPosition* pPos )
{
    eSetGetExpFldType = SECTIONNODE;
    CNTNT.pSection = &rSectNd.GetSection();

    if( pPos )
    {
        nNode = pPos->nNode.GetIndex();
        nCntnt = pPos->nContent.GetIndex();
    }
    else
    {
        nNode = rSectNd.GetIndex();
        nCntnt = 0;
    }
}

_SetGetExpFld::_SetGetExpFld( const SwTableBox& rTBox, const SwPosition* pPos )
{
    eSetGetExpFldType = TABLEBOX;
    CNTNT.pTBox = &rTBox;

    if( pPos )
    {
        nNode = pPos->nNode.GetIndex();
        nCntnt = pPos->nContent.GetIndex();
    }
    else
    {
        nNode = 0;
        nCntnt = 0;
        if( rTBox.GetSttNd() )
        {
            SwNodeIndex aIdx( *rTBox.GetSttNd() );
            const SwCntntNode* pNd = aIdx.GetNode().GetNodes().GoNext( &aIdx );
            if( pNd )
                nNode = pNd->GetIndex();
        }
    }
}

_SetGetExpFld::_SetGetExpFld( const SwNodeIndex& rNdIdx,
                                const SwTxtTOXMark& rTOX,
                                const SwIndex* pIdx )
{
    eSetGetExpFldType = TEXTTOXMARK;
    CNTNT.pTxtTOX = &rTOX;
    nNode = rNdIdx.GetIndex();
    if( pIdx )
        nCntnt = pIdx->GetIndex();
    else
        nCntnt = *rTOX.GetStart();
}

_SetGetExpFld::_SetGetExpFld( const SwPosition& rPos )
{
    eSetGetExpFldType = CRSRPOS;
    CNTNT.pPos = &rPos;
    nNode = rPos.nNode.GetIndex();
    nCntnt = rPos.nContent.GetIndex();
}

void _SetGetExpFld::GetPos( SwPosition& rPos ) const
{
    rPos.nNode = nNode;
    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), nCntnt );
}

void _SetGetExpFld::GetPosOfContent( SwPosition& rPos ) const
{
    const SwNode* pNd = GetNodeFromCntnt();
    if( pNd )
        pNd = pNd->GetCntntNode();

    if( pNd )
    {
        rPos.nNode = *pNd;
        rPos.nContent.Assign( (SwCntntNode*)pNd,GetCntPosFromCntnt() );
    }
    else
    {
        rPos.nNode = nNode;
        rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), nCntnt );
    }
}

void _SetGetExpFld::SetBodyPos( const SwCntntFrm& rFrm )
{
    if( !rFrm.IsInDocBody() )
    {
        SwNodeIndex aIdx( *rFrm.GetNode() );
        SwDoc& rDoc = *aIdx.GetNodes().GetDoc();
        SwPosition aPos( aIdx );
#ifndef PRODUCT
        ASSERT( ::GetBodyTxtNode( rDoc, aPos, rFrm ), "wo steht das Feld" );
#else
        ::GetBodyTxtNode( rDoc, aPos, rFrm );
#endif
        nNode = aPos.nNode.GetIndex();
        nCntnt = aPos.nContent.GetIndex();
    }
}

BOOL _SetGetExpFld::operator<( const _SetGetExpFld& rFld ) const
{
    if( nNode < rFld.nNode || ( nNode == rFld.nNode && nCntnt < rFld.nCntnt ))
        return TRUE;
    else if( nNode != rFld.nNode || nCntnt != rFld.nCntnt )
        return FALSE;

    const SwNode *pFirst = GetNodeFromCntnt(),
                 *pNext = rFld.GetNodeFromCntnt();

    // Position gleich: nur weiter wenn beide FeldPointer besetzt sind !!
    if( !pFirst || !pNext )
        return FALSE;

    // gleiche Section ??
    if( pFirst->StartOfSectionNode() != pNext->StartOfSectionNode() )
    {
        // sollte einer in der Tabelle stehen ?
        const SwNode *pFirstStt, *pNextStt;
        const SwTableNode* pTblNd = pFirst->FindTableNode();
        if( pTblNd )
            pFirstStt = pTblNd->StartOfSectionNode();
        else
            pFirstStt = pFirst->StartOfSectionNode();

        if( 0 != ( pTblNd = pNext->FindTableNode() ) )
            pNextStt = pTblNd->StartOfSectionNode();
        else
            pNextStt = pNext->StartOfSectionNode();

        if( pFirstStt != pNextStt )
        {
            if( pFirst->IsTxtNode() && pNext->IsTxtNode() &&
                ( pFirst->FindFlyStartNode() || pNext->FindFlyStartNode() ))
            {
                return ::IsFrameBehind( *(SwTxtNode*)pNext, nCntnt,
                                        *(SwTxtNode*)pFirst, nCntnt );
            }
            return pFirstStt->GetIndex() < pNextStt->GetIndex();
        }
    }

    // ist gleiche Section, dann Feld im gleichen Node ?
    if( pFirst != pNext )
        return pFirst->GetIndex() < pNext->GetIndex();

    // gleicher Node in der Section, dann Position im Node
    return GetCntPosFromCntnt() < rFld.GetCntPosFromCntnt();
}

const SwNode* _SetGetExpFld::GetNodeFromCntnt() const
{
    const SwNode* pRet = 0;
    if( CNTNT.pTxtFld )
        switch( eSetGetExpFldType )
        {
        case TEXTFIELD:
            pRet = &CNTNT.pTxtFld->GetTxtNode();
            break;

        case SECTIONNODE:
            pRet = CNTNT.pSection->GetFmt()->GetSectionNode();
            break;

        case CRSRPOS:
            pRet = &CNTNT.pPos->nNode.GetNode();
            break;

        case TEXTTOXMARK:
            pRet = &CNTNT.pTxtTOX->GetTxtNode();
            break;

        case TABLEBOX:
            if( CNTNT.pTBox->GetSttNd() )
            {
                SwNodeIndex aIdx( *CNTNT.pTBox->GetSttNd() );
                pRet = aIdx.GetNode().GetNodes().GoNext( &aIdx );
            }
            break;
        }
    return pRet;
}

USHORT _SetGetExpFld::GetCntPosFromCntnt() const
{
    USHORT nRet = 0;
    if( CNTNT.pTxtFld )
        switch( eSetGetExpFldType )
        {
        case TEXTFIELD:
            nRet = *CNTNT.pTxtFld->GetStart();
            break;
        case CRSRPOS:
            nRet =  CNTNT.pPos->nContent.GetIndex();
            break;
        case TEXTTOXMARK:
            nRet = *CNTNT.pTxtTOX->GetStart();
            break;
        }
    return nRet;
}

_HashStr::_HashStr( const String& rName, const String& rText,
                    _HashStr* pNxt )
    : SwHash( rName ), aSetStr( rText )
{
    pNext = pNxt;
}

// suche nach dem Namen, ist er vorhanden, returne seinen String, sonst
// einen LeerString
void LookString( SwHash** ppTbl, USHORT nSize, const String& rName,
                    String& rRet, USHORT* pPos )
{
    rRet = rName;
    rRet.EraseLeadingChars().EraseTrailingChars();
    SwHash* pFnd = Find( rRet, ppTbl, nSize, pPos );
    if( pFnd )
        rRet = ((_HashStr*)pFnd)->aSetStr;
    else
        rRet.Erase();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

String lcl_GetDBVarName( SwDoc& rDoc, SwDBNameInfField& rDBFld )
{
    String sDBNumNm( rDBFld.GetDBName( &rDoc ));

    if( sDBNumNm != rDoc.GetDBName() )
        sDBNumNm += DB_DELIM;
    else
        sDBNumNm.Erase();

    sDBNumNm += SwFieldType::GetTypeStr(TYP_DBSETNUMBERFLD);

    return sDBNumNm;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void lcl_CalcFld( SwDoc& rDoc, SwCalc& rCalc, const _SetGetExpFld& rSGEFld,
                        SwNewDBMgr* pMgr )
{
    const SwTxtFld* pTxtFld = rSGEFld.GetFld();
    if( !pTxtFld )
        return ;

    const SwField* pFld = pTxtFld->GetFld().GetFld();
    const USHORT nFldWhich = pFld->GetTyp()->Which();

    if( RES_SETEXPFLD == nFldWhich )
    {
        SwSbxValue aValue;
        if( GSE_EXPR & pFld->GetSubType() )
            aValue.PutDouble( ((SwSetExpField*)pFld)->GetValue() );
        else
            // Erweiterung fuers Rechnen mit Strings
            aValue.PutString( ((SwSetExpField*)pFld)->GetExpStr() );

        // setze im Calculator den neuen Wert
        rCalc.VarChange( pFld->GetTyp()->GetName(), aValue );
    }
    else if( pMgr )
    {
        switch( nFldWhich )
        {
        case RES_DBNUMSETFLD:
            {
                SwDBNumSetField* pDBFld = (SwDBNumSetField*)pFld;

#ifdef REPLACE_OFADBMGR
                String sDBName(pDBFld->GetDBName(&rDoc));
                String sSourceName(sDBName.GetToken(0, DB_DELIM));
                String sTableName(sDBName.GetToken(0).GetToken(1, DB_DELIM));

                if( pDBFld->IsCondValid() &&
                    pMgr->OpenDataSource( sSourceName, sTableName ))
#else
                if( pMgr->OpenDB( DBMGR_STD, pDBFld->GetDBName(&rDoc)) &&
                                                    pDBFld->IsCondValid() )
#endif
                    rCalc.VarChange( lcl_GetDBVarName( rDoc, *pDBFld),
                                    pDBFld->GetFormat() );
            }
            break;
        case RES_DBNEXTSETFLD:
            {
                SwDBNextSetField* pDBFld = (SwDBNextSetField*)pFld;
#ifdef REPLACE_OFADBMGR
                String sDBName(pDBFld->GetDBName(&rDoc));
                String sSourceName(sDBName.GetToken(0, DB_DELIM));
                String sTableName(sDBName.GetToken(0).GetToken(1, DB_DELIM));
                if( !pDBFld->IsCondValid() ||
                    !pMgr->OpenDataSource( sSourceName, sTableName ))
#else
                if( !pMgr->OpenDB( DBMGR_STD, pDBFld->GetDBName(&rDoc)) ||
                                                    !pDBFld->IsCondValid() )
#endif
                    break;

                String sDBNumNm(lcl_GetDBVarName( rDoc, *pDBFld));
                SwCalcExp* pExp = rCalc.VarLook( sDBNumNm );
                if( pExp )
                    rCalc.VarChange( sDBNumNm, pExp->nValue.GetLong() + 1 );
            }
            break;

        }
    }
}

void SwDoc::FldsToCalc( SwCalc& rCalc, const _SetGetExpFld& rToThisFld )
{
    // erzeuge die Sortierteliste aller SetFelder
    pUpdtFlds->MakeFldList( *this, bNewFldLst, GETFLD_CALC );
    bNewFldLst = FALSE;

    SwNewDBMgr* pMgr = GetNewDBMgr();
#ifdef REPLACE_OFADBMGR
    pMgr->CloseAll(FALSE);
#else
    pMgr->SetAllDirty(DBMGR_STD);
#endif

    if( pUpdtFlds->GetSortLst()->Count() )
    {
        USHORT nLast;
        _SetGetExpFld* pFld = (_SetGetExpFld*)&rToThisFld;
        if( pUpdtFlds->GetSortLst()->Seek_Entry( pFld, &nLast ) )
            ++nLast;

        const _SetGetExpFldPtr* ppSortLst = pUpdtFlds->GetSortLst()->GetData();
        for( USHORT n = 0; n < nLast; ++n, ++ppSortLst )
            lcl_CalcFld( *this, rCalc, **ppSortLst, pMgr );
    }

#ifdef REPLACE_OFADBMGR
    pMgr->CloseAll(FALSE);
#else
    pMgr->SetAllDirty(DBMGR_STD, FALSE);
#endif
}

void SwDoc::FldsToCalc( SwCalc& rCalc, ULONG nLastNd, USHORT nLastCnt )
{
    // erzeuge die Sortierteliste aller SetFelder
    pUpdtFlds->MakeFldList( *this, bNewFldLst, GETFLD_CALC );
    bNewFldLst = FALSE;

    SwNewDBMgr* pMgr = GetNewDBMgr();
#ifdef REPLACE_OFADBMGR
    pMgr->CloseAll(FALSE);
#else
    pMgr->SetAllDirty(DBMGR_STD);
#endif

    const _SetGetExpFldPtr* ppSortLst = pUpdtFlds->GetSortLst()->GetData();
    for( USHORT n = pUpdtFlds->GetSortLst()->Count();
        n && ((*ppSortLst)->GetNode() < nLastNd || ((*ppSortLst)->GetNode()
            == nLastNd && (*ppSortLst)->GetCntnt() <= nLastCnt ));
        --n, ++ppSortLst )
        lcl_CalcFld( *this, rCalc, **ppSortLst, pMgr );

#ifdef REPLACE_OFADBMGR
    pMgr->CloseAll(FALSE);
#else
    pMgr->SetAllDirty(DBMGR_STD, FALSE);
#endif
}

void SwDoc::FldsToExpand( SwHash**& ppHashTbl, USHORT& rTblSize,
                            const _SetGetExpFld& rToThisFld )
{
    // erzeuge die Sortierteliste aller SetFelder
    pUpdtFlds->MakeFldList( *this, bNewFldLst, GETFLD_EXPAND );
    bNewFldLst = FALSE;

    // HashTabelle fuer alle String Ersetzungen, wird "one the fly" gefuellt
    // (versuche eine "ungerade"-Zahl zu erzeugen)
    rTblSize = (( pUpdtFlds->GetSortLst()->Count() / 7 ) + 1 ) * 7;
    ppHashTbl = new SwHash*[ rTblSize ];
    memset( ppHashTbl, 0, sizeof( _HashStr* ) * rTblSize );

    USHORT nLast;
    {
        _SetGetExpFld* pTmp = (_SetGetExpFld*)&rToThisFld;
        if( pUpdtFlds->GetSortLst()->Seek_Entry( pTmp, &nLast ) )
            ++nLast;
    }

    USHORT nPos;
    SwHash* pFnd;
    String aNew;
    const _SetGetExpFldPtr* ppSortLst = pUpdtFlds->GetSortLst()->GetData();
    for( ; nLast; --nLast, ++ppSortLst )
    {
        const SwTxtFld* pTxtFld = (*ppSortLst)->GetFld();
        if( !pTxtFld )
            continue;

        const SwField* pFld = pTxtFld->GetFld().GetFld();
        switch( pFld->GetTyp()->Which() )
        {
        case RES_SETEXPFLD:
            if( GSE_STRING & pFld->GetSubType() )
            {
                // setze in der HashTabelle den neuen Wert
                // ist die "Formel" ein Feld ??
                SwSetExpField* pSFld = (SwSetExpField*)pFld;
                LookString( ppHashTbl, rTblSize, pSFld->GetFormula(), aNew );

                if( !aNew.Len() )               // nichts gefunden, dann ist
                    aNew = pSFld->GetFormula(); // die Formel der neue Wert

                // suche den Namen vom Feld
                aNew = ((SwSetExpFieldType*)pSFld->GetTyp())->GetSetRefName();
                // Eintrag vorhanden ?
                pFnd = Find( aNew, ppHashTbl, rTblSize, &nPos );
                if( pFnd )
                    // Eintrag in der HashTabelle aendern
                    ((_HashStr*)pFnd)->aSetStr = pSFld->GetExpStr();
                else
                    // neuen Eintrag einfuegen
                    *(ppHashTbl + nPos ) = new _HashStr( aNew,
                            pSFld->GetExpStr(), (_HashStr*)*(ppHashTbl + nPos) );
            }
            break;
        case RES_DBFLD:
            {
                const String& rName = pFld->GetTyp()->GetName();

                // Eintrag in den HashTable eintragen
                // Eintrag vorhanden ?
                pFnd = Find( rName, ppHashTbl, rTblSize, &nPos );
                if( pFnd )
                    // Eintrag in der HashTabelle aendern
                    ((_HashStr*)pFnd)->aSetStr = pFld->Expand();
                else
                    // neuen Eintrag einfuegen
                    *(ppHashTbl + nPos ) = new _HashStr( rName,
                                pFld->Expand(), (_HashStr*)*(ppHashTbl + nPos));
            }
            break;
        }
    }
}


void SwDoc::UpdateExpFlds( SwTxtFld* pUpdtFld, BOOL bUpdRefFlds )
{
    if( IsExpFldsLocked() )
        return;

    BOOL bOldInUpdateFlds = pUpdtFlds->IsInUpdateFlds();
    pUpdtFlds->SetInUpdateFlds( TRUE );

    pUpdtFlds->MakeFldList( *this, TRUE, GETFLD_ALL );
    bNewFldLst = FALSE;

    if( !pUpdtFlds->GetSortLst()->Count() )
    {
        if( bUpdRefFlds )
            UpdateRefFlds();

        pUpdtFlds->SetInUpdateFlds( bOldInUpdateFlds );
        pUpdtFlds->SetFieldsDirty( FALSE );
        return ;
    }

    USHORT nWhich, n;

    // HashTabelle fuer alle String Ersetzungen, wird "one the fly" gefuellt
    // (versuche eine "ungerade"-Zahl zu erzeugen)
    USHORT nStrFmtCnt = (( pFldTypes->Count() / 7 ) + 1 ) * 7;
    SwHash** pHashStrTbl = new SwHash*[ nStrFmtCnt ];
    memset( pHashStrTbl, 0, sizeof( _HashStr* ) * nStrFmtCnt );

    {
        const SwFieldType* pFldType;
        // gesondert behandeln:
        for( n = pFldTypes->Count(); n; )
            switch( ( pFldType = (*pFldTypes)[ --n ] )->Which() )
            {
            case RES_USERFLD:
                {
                    // Eintrag vorhanden ?
                    USHORT nPos;
                    const String& rNm = pFldType->GetName();
                    String sExpand(((SwUserFieldType*)pFldType)->Expand(GSE_STRING, 0, 0));
                    SwHash* pFnd = Find( rNm, pHashStrTbl, nStrFmtCnt, &nPos );
                    if( pFnd )
                        // Eintrag in der HashTabelle aendern ??
                        ((_HashStr*)pFnd)->aSetStr = sExpand;
                    else
                        // neuen Eintrag einfuegen
                        *(pHashStrTbl + nPos ) = new _HashStr( rNm, sExpand,
                                                (_HashStr*)*(pHashStrTbl + nPos) );
                }
                break;
            case RES_SETEXPFLD:
                ((SwSetExpFieldType*)pFldType)->SetOutlineChgNd( 0 );
                break;
            }
    }

    // Ok, das Array ist soweit mit allen Feldern gefuellt, dann rechne mal
    SwCalc aCalc( *this );

    String sDBNumNm( SwFieldType::GetTypeStr( TYP_DBSETNUMBERFLD ) );

    // aktuelle Datensatznummer schon vorher einstellen
    SwNewDBMgr* pMgr = GetNewDBMgr();
#ifdef REPLACE_OFADBMGR
    pMgr->CloseAll(FALSE);
#else
    pMgr->SetAllDirty(DBMGR_STD);
#endif
/*
    if(pMgr && pMgr->OpenDB(DBMGR_STD, GetDBDesc(), FALSE))
    {
        if(!pMgr->IsInMerge() )
            pMgr->ToFirstSelectedRecord(DBMGR_STD);

        aCalc.VarChange( sDBNumNm, pMgr->GetCurSelectedRecordId(DBMGR_STD));
    }
*/

    String aNew;
    const _SetGetExpFldPtr* ppSortLst = pUpdtFlds->GetSortLst()->GetData();
    for( n = pUpdtFlds->GetSortLst()->Count(); n; --n, ++ppSortLst )
    {
        SwSection* pSect = (SwSection*)(*ppSortLst)->GetSection();
        if( pSect )
        {
            //!SECTION

//          if( pGFld->IsInBodyTxt() )
                pSect->SetCondHidden( aCalc.Calculate(
                                        pSect->GetCondition() ).GetBool() );
            continue;
        }

        SwTxtFld* pTxtFld = (SwTxtFld*)(*ppSortLst)->GetFld();
        if( !pTxtFld )
        {
            ASSERT( !this, "was ist es denn nun" );
            continue;
        }

        SwFmtFld* pFmtFld = (SwFmtFld*)&pTxtFld->GetFld();
        SwField* pFld = pFmtFld->GetFld();

        switch( nWhich = pFld->GetTyp()->Which() )
        {
        case RES_HIDDENTXTFLD:
        {
            SwHiddenTxtField* pHFld = (SwHiddenTxtField*)pFld;
            pHFld->SetValue( !aCalc.Calculate( pHFld->GetPar1() ).GetBool());
            // Feld Evaluieren
            pHFld->Evaluate(this);
        }
        break;
        case RES_HIDDENPARAFLD:
        {
            SwHiddenParaField* pHPFld = (SwHiddenParaField*)pFld;
            pHPFld->SetHidden( aCalc.Calculate( pHPFld->GetPar1() ).GetBool());
        }
        break;
        case RES_DBSETNUMBERFLD:
        {
            ((SwDBSetNumberField*)pFld)->Evaluate(this);
            aCalc.VarChange( sDBNumNm, ((SwDBSetNumberField*)pFld)->GetSetNumber());
        }
        break;
        case RES_DBNEXTSETFLD:
        case RES_DBNUMSETFLD:
            UpdateDBNumFlds( *(SwDBNameInfField*)pFld, aCalc );
        break;
        case RES_DBFLD:
        {
            // Feld Evaluieren
            ((SwDBField*)pFld)->Evaluate();

#ifdef REPLACE_OFADBMGR
                String sDBName(((SwDBField*)pFld)->GetDBName());
                String sSourceName(sDBName.GetToken(0, DB_DELIM));
                String sTableName(sDBName.GetToken(0).GetToken(1, DB_DELIM));

            if( pMgr->OpenDataSource( sSourceName, sTableName ))
                aCalc.VarChange( sDBNumNm, pMgr->GetSelectedRecordId(sSourceName, sTableName));
#else
            if(pMgr->OpenDB(DBMGR_STD, ((SwDBField*)pFld)->GetDBName()))
                aCalc.VarChange( sDBNumNm, pMgr->GetCurSelectedRecordId(DBMGR_STD) );
#endif

            const String& rName = pFld->GetTyp()->GetName();

            // Wert fuer den Calculator setzen
//JP 10.02.96: GetValue macht hier doch keinen Sinn
//          ((SwDBField*)pFld)->GetValue();

//!OK           aCalc.VarChange(aName, ((SwDBField*)pFld)->GetValue(aCalc));

            // Eintrag in den HashTable eintragen
            // Eintrag vorhanden ?
            USHORT nPos;
            SwHash* pFnd = Find( rName, pHashStrTbl, nStrFmtCnt, &nPos );
            if( pFnd )
                // Eintrag in der HashTabelle aendern
                ((_HashStr*)pFnd)->aSetStr = pFld->Expand();
            else
                // neuen Eintrag einfuegen
                *(pHashStrTbl + nPos ) = new _HashStr( rName,
                            pFld->Expand(), (_HashStr*)*(pHashStrTbl + nPos));
        }
        break;
        case RES_GETEXPFLD:
        case RES_SETEXPFLD:
        {
            if( GSE_STRING & pFld->GetSubType() )       // String Ersetzung
            {
                if( RES_GETEXPFLD == nWhich )
                {
                    SwGetExpField* pGFld = (SwGetExpField*)pFld;

                    if( (!pUpdtFld || pUpdtFld == pTxtFld )
                        && pGFld->IsInBodyTxt() )
                    {
                        LookString( pHashStrTbl, nStrFmtCnt,
                                    pGFld->GetFormula(), aNew );
                        pGFld->ChgExpStr( aNew );
                    }
                }
                else
                {
                    SwSetExpField* pSFld = (SwSetExpField*)pFld;
                    // ist die "Formel" ein Feld ??
                    LookString( pHashStrTbl, nStrFmtCnt,
                                pSFld->GetFormula(), aNew );

                    if( !aNew.Len() )               // nichts gefunden, dann ist die
                        aNew = pSFld->GetFormula();     // Formel der neue Wert

                    // nur ein spezielles FeldUpdaten ?
                    if( !pUpdtFld || pUpdtFld == pTxtFld )
                        pSFld->ChgExpStr( aNew );

                    // suche den Namen vom Feld
                    aNew = ((SwSetExpFieldType*)pSFld->GetTyp())->GetSetRefName();
                    // Eintrag vorhanden ?
                    USHORT nPos;
                    SwHash* pFnd = Find( aNew, pHashStrTbl, nStrFmtCnt, &nPos );
                    if( pFnd )
                        // Eintrag in der HashTabelle aendern
                        ((_HashStr*)pFnd)->aSetStr = pSFld->GetExpStr();
                    else
                        // neuen Eintrag einfuegen
                        *(pHashStrTbl + nPos ) = pFnd = new _HashStr( aNew,
                                        pSFld->GetExpStr(),
                                        (_HashStr*)*(pHashStrTbl + nPos) );

                    // Erweiterung fuers Rechnen mit Strings
                    SwSbxValue aValue;
                    aValue.PutString( ((_HashStr*)pFnd)->aSetStr );
                    aCalc.VarChange( aNew, aValue );
                }
            }
            else            // Formel neu berechnen
            {
                if( RES_GETEXPFLD == nWhich )
                {
                    SwGetExpField* pGFld = (SwGetExpField*)pFld;

                    if( (!pUpdtFld || pUpdtFld == pTxtFld )
                        && pGFld->IsInBodyTxt() )
                    {
                        pGFld->SetValue(aCalc.Calculate(
                                        pGFld->GetFormula() ).GetDouble() );
                    }
                }
                else
                {
                    SwSetExpField* pSFld = (SwSetExpField*)pFld;
                    SwSetExpFieldType* pSFldTyp = (SwSetExpFieldType*)pFld->GetTyp();
                    aNew = pSFldTyp->GetName();

                    SwNode* pSeqNd = 0;

                    if( pSFld->IsSequenceFld() )
                    {
                        BYTE nLvl = pSFldTyp->GetOutlineLvl();
                        if( MAXLEVEL > nLvl )
                        {
                            // dann teste, ob die Nummer neu aufsetzen muss
                            pSeqNd = GetNodes()[ (*ppSortLst)->GetNode() ];

                            const SwTxtNode* pOutlNd = pSeqNd->
                                    FindOutlineNodeOfLevel( nLvl );
                            if( pSFldTyp->GetOutlineChgNd() != pOutlNd )
                            {
                                pSFldTyp->SetOutlineChgNd( pOutlNd );
                                aCalc.VarChange( aNew, 0 );
                            }
                        }
                    }

                    aNew += '=';
                    aNew += pSFld->GetFormula();

                    double nErg = aCalc.Calculate( aNew ).GetDouble();
                    // nur ein spezielles Feld updaten ?
                    if( !pUpdtFld || pUpdtFld == pTxtFld )
                    {
                        pSFld->SetValue( nErg );

                        if( pSeqNd )
                            pSFldTyp->SetChapter( *pSFld, *pSeqNd );
                    }
                }
            }
        }
        } // switch

        pFmtFld->Modify( 0, 0 );        // Formatierung anstossen

        if( pUpdtFld == pTxtFld )       // sollte nur dieses geupdatet werden
        {
            if( RES_GETEXPFLD == nWhich ||      // nur GetFeld oder
                RES_HIDDENTXTFLD == nWhich ||   // HiddenTxt?
                RES_HIDDENPARAFLD == nWhich)    // HiddenParaFld?
                break;                          // beenden
            pUpdtFld = 0;                       // ab jetzt alle Updaten
        }
    }

#ifdef REPLACE_OFADBMGR
    pMgr->CloseAll(FALSE);
#else
    pMgr->SetAllDirty(DBMGR_STD, FALSE);
#endif
    // HashTabelle wieder loeschen
    ::DeleteHashTable( pHashStrTbl, nStrFmtCnt );

    // Referenzfelder updaten
    if( bUpdRefFlds )
        UpdateRefFlds();

    pUpdtFlds->SetInUpdateFlds( bOldInUpdateFlds );
    pUpdtFlds->SetFieldsDirty( FALSE );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::UpdateDBNumFlds( SwDBNameInfField& rDBFld, SwCalc& rCalc )
{
    SwNewDBMgr* pMgr = GetNewDBMgr();

    USHORT nFldType = rDBFld.Which();

    BOOL bPar1 = rCalc.Calculate( rDBFld.GetPar1() ).GetBool();

    if( RES_DBNEXTSETFLD == nFldType )
        ((SwDBNextSetField&)rDBFld).SetCondValid( bPar1 );
    else
        ((SwDBNumSetField&)rDBFld).SetCondValid( bPar1 );

    if( rDBFld.GetRealDBName().Len() )
    {
        // Eine bestimmte Datenbank bearbeiten
        if( RES_DBNEXTSETFLD == nFldType )
            ((SwDBNextSetField&)rDBFld).Evaluate(this);
        else
            ((SwDBNumSetField&)rDBFld).Evaluate(this);

        String sDBName( rDBFld.GetDBName(this) );

#ifdef REPLACE_OFADBMGR
        String sSourceName(sDBName.GetToken(0, DB_DELIM));
        String sTableName(sDBName.GetToken(0).GetToken(1, DB_DELIM));

        if( pMgr->OpenDataSource( sSourceName, sTableName ))
            rCalc.VarChange( lcl_GetDBVarName( *this, rDBFld),
                        pMgr->GetSelectedRecordId(sSourceName, sTableName) );
#else
        if( pMgr->OpenDB( DBMGR_STD, sDBName ))
            rCalc.VarChange( lcl_GetDBVarName( *this, rDBFld),
                    pMgr->GetCurSelectedRecordId( DBMGR_STD ) );
#endif
    }
    else
    {
#ifdef REPLACE_OFADBMGR
        DBG_ERROR("TODO: what should happen with unnamed DBFields?")
#else
        // Alle Datenbanken bearbeiten
        OfaDBParam* pParam = pMgr->GetFirstDBData(DBMGR_STD);
//      why that? - the name must be empty
//      String sOldFldName( rDBFld.GetRealDBName() );

        while (pParam)
        {
            if (pParam->HasConnection())
            {
                rDBFld.SetDBName( pParam->GetSymDBName() );

                if( RES_DBNEXTSETFLD == nFldType )
                    ((SwDBNextSetField&)rDBFld).Evaluate(this);
                else
                    ((SwDBNumSetField&)rDBFld).Evaluate(this);

                rCalc.VarChange( lcl_GetDBVarName( *this, rDBFld ),
                        pMgr->GetCurSelectedRecordId( DBMGR_STD ) );
            }
            pParam = pMgr->GetNextDBData();
        }
        rDBFld.SetDBName(aEmptyStr);
#endif
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::_InitFieldTypes()       // wird vom CTOR gerufen!!
{
    // Feldtypen
    USHORT nFldType = 0;
    pFldTypes->Insert( new SwDateTimeFieldType(this), nFldType++ );
    pFldTypes->Insert( new SwChapterFieldType, nFldType++ );
    pFldTypes->Insert( new SwPageNumberFieldType, nFldType++ );
    pFldTypes->Insert( new SwAuthorFieldType(this),nFldType++ );
    pFldTypes->Insert( new SwFileNameFieldType(this), nFldType++ );
    pFldTypes->Insert( new SwDBNameFieldType(this), nFldType++);
    pFldTypes->Insert( new SwGetExpFieldType(this), nFldType++ );
    pFldTypes->Insert( new SwGetRefFieldType( this ), nFldType++ );
    pFldTypes->Insert( new SwHiddenTxtFieldType, nFldType++ );
    pFldTypes->Insert( new SwPostItFieldType, nFldType++ );
    pFldTypes->Insert( new SwDocStatFieldType(this), nFldType++);
    pFldTypes->Insert( new SwDocInfoFieldType(this), nFldType++);
    pFldTypes->Insert( new SwInputFieldType( this ), nFldType++ );
    pFldTypes->Insert( new SwTblFieldType( this ), nFldType++);
    pFldTypes->Insert( new SwMacroFieldType(this), nFldType++ );
    pFldTypes->Insert( new SwHiddenParaFieldType, nFldType++ );
    pFldTypes->Insert( new SwDBNextSetFieldType, nFldType++ );
    pFldTypes->Insert( new SwDBNumSetFieldType, nFldType++ );
    pFldTypes->Insert( new SwDBSetNumberFieldType, nFldType++ );
    pFldTypes->Insert( new SwTemplNameFieldType(this), nFldType++);
    pFldTypes->Insert( new SwTemplNameFieldType(this),nFldType++);
    pFldTypes->Insert( new SwExtUserFieldType, nFldType++ );
    pFldTypes->Insert( new SwRefPageSetFieldType, nFldType++ );
    pFldTypes->Insert( new SwRefPageGetFieldType( this ), nFldType++ );
    pFldTypes->Insert( new SwJumpEditFieldType( this ), nFldType++ );
    pFldTypes->Insert( new SwScriptFieldType( this ), nFldType++ );

    // Types muessen am Ende stehen !!
    // Im InsertFldType wird davon ausgegangen !!!!
    // MIB 14.04.95: Im Sw3StringPool::Setup (sw3imp.cxx) und
    //               lcl_sw3io_InSetExpField (sw3field.cxx) jetzt auch
    pFldTypes->Insert( new SwSetExpFieldType(this,
                SW_RESSTR(STR_POOLCOLL_LABEL_ABB), GSE_SEQ), nFldType++);
    pFldTypes->Insert( new SwSetExpFieldType(this,
                SW_RESSTR(STR_POOLCOLL_LABEL_TABLE), GSE_SEQ),nFldType++);
    pFldTypes->Insert( new SwSetExpFieldType(this,
                SW_RESSTR(STR_POOLCOLL_LABEL_FRAME), GSE_SEQ),nFldType++);
    pFldTypes->Insert( new SwSetExpFieldType(this,
                SW_RESSTR(STR_POOLCOLL_LABEL_DRAWING), GSE_SEQ),nFldType++);

    ASSERT( nFldType == INIT_FLDTYPES, "Bad initsize: SwFldTypes" );
}

void SwDoc::InsDelFldInFldLst( BOOL bIns, const SwTxtFld& rFld )
{
    if( !bNewFldLst || !IsInDtor() )
        pUpdtFlds->InsDelFldInFldLst( bIns, rFld );
}

String SwDoc::GetDBName()
{
#ifdef REPLACE_OFADBMGR
    return GetDBDesc();
#else
    return GetNewDBMgr()->ExtractDBName( GetDBDesc() );
#endif
}

const String& SwDoc::GetDBDesc()
{
    if (!aDBName.Len())
        aDBName = GetNewDBMgr()->GetAddressDBName();
    return aDBName;
}

void SwDoc::SetInitDBFields( BOOL b )
{
    GetNewDBMgr()->SetInitDBFields( b );
}

/*--------------------------------------------------------------------
    Beschreibung: Alle von Feldern verwendete Datenbanken herausfinden
 --------------------------------------------------------------------*/

void SwDoc::GetAllUsedDB( SvStringsDtor& rDBNameList,
                            const SvStringsDtor* pAllDBNames )
{
    USHORT n;
    SvStringsDtor aUsedDBNames;
    SvStringsDtor aAllDBNames;

    if( !pAllDBNames )
    {
        GetAllDBNames( aAllDBNames );
        pAllDBNames = &aAllDBNames;
    }

    SwSectionFmts& rArr = GetSections();
    for( n = rArr.Count(); n; )
    {
        SwSection* pSect = rArr[ --n ]->GetSection();

        if( pSect )
        {
            String aCond( pSect->GetCondition() );
            AddUsedDBToList( rDBNameList, FindUsedDBs( *pAllDBNames,
                                                aCond, aUsedDBNames ) );
            aUsedDBNames.DeleteAndDestroy( 0, aUsedDBNames.Count() );
        }
    }

    const SfxPoolItem* pItem;
    USHORT nMaxItems = GetAttrPool().GetItemCount( RES_TXTATR_FIELD );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 == (pItem = GetAttrPool().GetItem( RES_TXTATR_FIELD, n ) ))
            continue;

        const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
        const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            continue;

        const SwField* pFld = pFmtFld->GetFld();
        switch( pFld->GetTyp()->Which() )
        {
            case RES_DBFLD:
                AddUsedDBToList( rDBNameList,
                                ((SwDBField*)pFld)->GetDBName() );
                break;

            case RES_DBSETNUMBERFLD:
            case RES_DBNAMEFLD:
                AddUsedDBToList( rDBNameList,
                                ((SwDBNameInfField*)pFld)->GetRealDBName() );
                break;

            case RES_DBNUMSETFLD:
            case RES_DBNEXTSETFLD:
                AddUsedDBToList( rDBNameList,
                                ((SwDBNameInfField*)pFld)->GetRealDBName() );
                // kein break  // JP: ist das so richtig ??

            case RES_HIDDENTXTFLD:
            case RES_HIDDENPARAFLD:
                AddUsedDBToList(rDBNameList, FindUsedDBs( *pAllDBNames,
                                            pFld->GetPar1(), aUsedDBNames ));
                aUsedDBNames.DeleteAndDestroy( 0, aUsedDBNames.Count() );
                break;

            case RES_SETEXPFLD:
            case RES_GETEXPFLD:
            case RES_TABLEFLD:
                AddUsedDBToList(rDBNameList, FindUsedDBs( *pAllDBNames,
                                        pFld->GetFormula(), aUsedDBNames ));
                aUsedDBNames.DeleteAndDestroy( 0, aUsedDBNames.Count() );
                break;
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::GetAllDBNames( SvStringsDtor& rAllDBNames )
{
    SwNewDBMgr* pMgr = GetNewDBMgr();

#ifdef REPLACE_OFADBMGR
    const SwDSParamArr& rArr = pMgr->GetDSParamArray();
    for(USHORT i = 0; i < rArr.Count(); i++)
    {
        SwDSParam* pParam = rArr[i];
        String* pStr = new String( pParam->sDataSource );
        (*pStr) += DB_DELIM;
        (*pStr) += pParam->sTableOrQuery;
        rAllDBNames.Insert( pStr, rAllDBNames.Count() );
    }
#else
    OfaDBParam* pParam = pMgr->GetFirstDBData(DBMGR_STD);

    while( pParam )
    {
        if( pParam->GetSymDBName().Len() )
        {
            String* pStr = new String( pParam->GetSymDBName() );
#ifndef UNX
            GetAppCharClass().toUpper( *pStr );
#endif
            rAllDBNames.Insert( pStr, rAllDBNames.Count() );
        }
        pParam = pMgr->GetNextDBData();
    }
#endif
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SvStringsDtor& SwDoc::FindUsedDBs( const SvStringsDtor& rAllDBNames,
                                    const String& rFormel,
                                    SvStringsDtor& rUsedDBNames )
{
    const CharClass& rCC = GetAppCharClass();
    String  sFormel( rFormel);
#ifndef UNX
    rCC.toUpper( sFormel );
#endif

    xub_StrLen nPos;
    for (USHORT i = 0; i < rAllDBNames.Count(); ++i )
    {
        const String* pStr = rAllDBNames.GetObject(i);

        if( STRING_NOTFOUND != (nPos = sFormel.Search( *pStr )) &&
            sFormel.GetChar( nPos + pStr->Len() ) == '.' &&
            (!nPos || !rCC.isLetterNumeric( sFormel, nPos - 1 )))
        {
            // Tabellenname suchen
            xub_StrLen nEndPos;
            nPos += pStr->Len() + 1;
            if( STRING_NOTFOUND != (nEndPos = sFormel.Search('.', nPos)) )
            {
                String* pDBNm = new String( *pStr );
                pDBNm->Append( DB_DELIM );
                pDBNm->Append( sFormel.Copy( nPos, nEndPos - nPos ));
                rUsedDBNames.Insert( pDBNm, rUsedDBNames.Count() );
            }
        }
    }
    return rUsedDBNames;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::AddUsedDBToList( SvStringsDtor& rDBNameList,
                             const SvStringsDtor& rUsedDBNames )
{
    for (USHORT i = 0; i < rUsedDBNames.Count(); i++)
        AddUsedDBToList( rDBNameList, *rUsedDBNames.GetObject(i) );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::AddUsedDBToList( SvStringsDtor& rDBNameList, const String& rDBName)
{
    if( !rDBName.Len() )
        return;

    const CharClass& rCC = GetAppCharClass();
    String sLowerDBName( DBNAME_LOWER( rDBName ));

#ifdef REPLACE_OFADBMGR
    for (USHORT i = 0; i < rDBNameList.Count(); i++)
        if( sLowerDBName.Equals( DBNAME_LOWER(
                rDBNameList.GetObject(i)->GetToken(0) )))
                return;

    const SwDSParam* pParam = GetNewDBMgr()->CreateDSData(rDBName);
    String* pNew = new String( rDBName );
    rDBNameList.Insert( pNew, rDBNameList.Count() );
#else
    for (USHORT i = 0; i < rDBNameList.Count(); i++)
        if( sLowerDBName.Equals( DBNAME_LOWER(
                GetNewDBMgr()->ExtractDBName( *rDBNameList.GetObject(i) ) )))
                return;
    OfaDBParam& rParam = GetNewDBMgr()->GetDBData( DBMGR_STD, &rDBName );
    String* pNew = new String( rParam.GetDBName() );
    if( !pNew->Len() )
        *pNew = rParam.GetSymDBName();
    rDBNameList.Insert( pNew, rDBNameList.Count() );
#endif
}

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/

BOOL SwDoc::RenameUserFields( const String& rOldName, const String& rNewName )
{
    USHORT n;
    BOOL bRet = FALSE;
    String sFormel;

    SwSectionFmts& rArr = GetSections();
    for( n = rArr.Count(); n; )
    {
        SwSection* pSect = rArr[ --n ]->GetSection();

        if( pSect )
        {
            sFormel = pSect->GetCondition();
            RenameUserFld( rOldName, rNewName, sFormel );
            pSect->SetCondition( sFormel );
        }
    }

    const SfxPoolItem* pItem;
    USHORT nMaxItems = GetAttrPool().GetItemCount( RES_TXTATR_FIELD );
    const CharClass& rCC = GetAppCharClass();
    String sLowerOldName( rCC.lower( rOldName ));

    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 == (pItem = GetAttrPool().GetItem( RES_TXTATR_FIELD, n ) ))
            continue;

        SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
        SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            continue;

        SwField* pFld = pFmtFld->GetFld();
        BOOL bExpand = FALSE;

        switch( pFld->GetTyp()->Which() )
        {
        case RES_USERFLD:
            if( sLowerOldName.Equals( rCC.lower( pFld->GetTyp()->GetName() )))
            {
                SwUserFieldType* pOldTyp = (SwUserFieldType*)pFld->GetTyp();

                SwUserFieldType* pTyp = (SwUserFieldType*)InsertFldType(
                        SwUserFieldType(this, rNewName));

                pTyp->SetContent(pOldTyp->GetContent());
                pTyp->SetType(pOldTyp->GetType());
                pTyp->Add(pFmtFld); // Feld auf neuen Typ umhaengen
                pFld->ChgTyp(pTyp);

                bExpand = TRUE;
            }
            break;

        case RES_DBNUMSETFLD:
        case RES_DBNEXTSETFLD:
        case RES_HIDDENTXTFLD:
        case RES_HIDDENPARAFLD:
            sFormel = pFld->GetPar1();
            RenameUserFld( rOldName, rNewName, sFormel );
            pFld->SetPar1( sFormel );
            bExpand = TRUE;
            break;

        case RES_SETEXPFLD:
            if( sLowerOldName.Equals( rCC.lower( pFld->GetTyp()->GetName() )))
            {
                SwSetExpFieldType* pOldTyp = (SwSetExpFieldType*)pFld->GetTyp();

                SwSetExpFieldType* pTyp = (SwSetExpFieldType*)InsertFldType(
                        SwSetExpFieldType(this, rNewName, pOldTyp->GetType()));
                pTyp->Add( pFmtFld );   // Feld auf neuen Typ umhaengen
                pFld->ChgTyp( pTyp );

                bExpand = TRUE;
            }
            // Kein break!!!

        case RES_GETEXPFLD:
        case RES_TABLEFLD:
            sFormel = pFld->GetFormula();
            RenameUserFld( rOldName, rNewName, sFormel );
            pFld->SetPar2( sFormel );
            bExpand = TRUE;
            break;
        }

        if( bExpand )
        {
            pTxtFld->ExpandAlways();
            bRet = TRUE;
        }
    }
    SetModified();

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::RenameUserFld( const String& rOldName, const String& rNewName,
                            String& rFormel )
{
    const CharClass& rCC = GetAppCharClass();

    String  sFormel( rCC.upper( rFormel ));
    String  sOldName( rCC.upper( rOldName ));
    xub_StrLen nPos;

    if( !FindOperator( rOldName ) &&
        !sOldName.Equals( rCC.upper( rNewName ) ) )
    {
        nPos = 0;

        while ((nPos = sFormel.Search(sOldName, nPos)) != STRING_NOTFOUND)
        {
            if(!nPos || !rCC.isLetterNumeric( sFormel, nPos - 1 ))
            {
                rFormel.Erase( nPos, rOldName.Len() );
                rFormel.Insert( rNewName, nPos );
                sFormel = rCC.upper( rFormel );
            }
        }
    }
}

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::ChangeDBFields( const SvStringsDtor& rOldNames,
                            const String& rNewName )
{
    String sFormel;
    USHORT n;

    SwSectionFmts& rArr = GetSections();
    for( n = rArr.Count(); n; )
    {
        SwSection* pSect = rArr[ --n ]->GetSection();

        if( pSect )
        {
            sFormel = pSect->GetCondition();
            ReplaceUsedDBs( rOldNames, rNewName, sFormel);
            pSect->SetCondition(sFormel);
        }
    }

    const SfxPoolItem* pItem;
    USHORT nMaxItems = GetAttrPool().GetItemCount( RES_TXTATR_FIELD );

    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 == (pItem = GetAttrPool().GetItem( RES_TXTATR_FIELD, n ) ))
            continue;

        SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
        SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            continue;

        SwField* pFld = pFmtFld->GetFld();
        BOOL bExpand = FALSE;

        switch( pFld->GetTyp()->Which() )
        {
            case RES_DBFLD:
                if( IsNameInArray( rOldNames, ((SwDBField*)pFld)->GetDBName()))
                {
                    SwDBFieldType* pOldTyp = (SwDBFieldType*)pFld->GetTyp();

                    SwDBFieldType* pTyp = (SwDBFieldType*)InsertFldType(
                            SwDBFieldType(this, pOldTyp->GetColumnName(), rNewName));

                    pTyp->Add(pFmtFld); // Feld auf neuen Typ umhaengen
                    pFld->ChgTyp(pTyp);

                    ((SwDBField*)pFld)->ClearInitialized();
                    ((SwDBField*)pFld)->InitContent();

                    bExpand = TRUE;
                }
                break;

            case RES_DBSETNUMBERFLD:
            case RES_DBNAMEFLD:
                if( IsNameInArray( rOldNames,
                                ((SwDBNameInfField*)pFld)->GetRealDBName()))
                {
                    ((SwDBNameInfField*)pFld)->SetDBName(rNewName);
                    bExpand = TRUE;
                }
                break;

            case RES_DBNUMSETFLD:
            case RES_DBNEXTSETFLD:
                if( IsNameInArray( rOldNames,
                                ((SwDBNameInfField*)pFld)->GetRealDBName()))
                {
                    ((SwDBNameInfField*)pFld)->SetDBName(rNewName);
                    bExpand = TRUE;
                }
                // kein break;
            case RES_HIDDENTXTFLD:
            case RES_HIDDENPARAFLD:
                sFormel = pFld->GetPar1();
                ReplaceUsedDBs( rOldNames, rNewName, sFormel);
                pFld->SetPar1( sFormel );
                bExpand = TRUE;
                break;

            case RES_SETEXPFLD:
            case RES_GETEXPFLD:
            case RES_TABLEFLD:
                sFormel = pFld->GetFormula();
                ReplaceUsedDBs( rOldNames, rNewName, sFormel);
                pFld->SetPar2( sFormel );
                bExpand = TRUE;
                break;
        }

        if (bExpand)
            pTxtFld->ExpandAlways();
    }
    SetModified();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::ReplaceUsedDBs( const SvStringsDtor& rUsedDBNames,
                            const String& rNewName, String& rFormel )
{
    const CharClass& rCC = GetAppCharClass();
    String  sFormel(rFormel);
    String  sNewName( rNewName );
    sNewName.SearchAndReplace( DB_DELIM, '.');
    String sUpperNewNm( sNewName );
#ifdef REPLACE_OFADBMGR
#else
#ifndef UNX
    rCC.toUpper( sFormel );
    rCC.toUpper( sUpperNewNm );
#endif
#endif


    for( USHORT i = 0; i < rUsedDBNames.Count(); ++i )
    {
        String  sDBName( *rUsedDBNames.GetObject( i ) );

#ifdef REPLACE_OFADBMGR
#else
#ifndef UNX
        rCC.toUpper( sDBName );
#endif
#endif

        sDBName.SearchAndReplace( DB_DELIM, '.');
        if( sDBName.Equals( sUpperNewNm ))
        {
            xub_StrLen nPos = 0;

            while ((nPos = sFormel.Search(sDBName, nPos)) != STRING_NOTFOUND)
            {
                if( sFormel.GetChar( nPos + sDBName.Len() ) == '.' &&
                    (!nPos || !rCC.isLetterNumeric( sFormel, nPos - 1 )))
                {
                    rFormel.Erase( nPos, sDBName.Len() );
                    rFormel.Insert( sNewName, nPos );
                    sFormel = rFormel;
#ifdef REPLACE_OFADBMGR
#else
#ifndef UNX
                    rCC.toUpper( sFormel );
#endif
#endif
                }
            }
        }
    }
}

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/

BOOL SwDoc::IsNameInArray( const SvStringsDtor& rArr, const String& rName )
{
    const CharClass& rCC = GetAppCharClass();
    String sLowerName( DBNAME_LOWER( rName ));
    for( USHORT i = 0; i < rArr.Count(); ++i )
        if( sLowerName.Equals( DBNAME_LOWER( *rArr[ i ] )) )
                return TRUE;
    return FALSE;
}

void SwDoc::SetFixFields( BOOL bOnlyTimeDate, const DateTime* pNewDateTime )
{
    BOOL bIsModified = IsModified();

    ULONG nDate, nTime;
    if( pNewDateTime )
    {
        nDate = pNewDateTime->GetDate();
        nTime = pNewDateTime->GetTime();
    }
    else
    {
        nDate = Date().GetDate();
        nTime = Time().GetTime();
    }

    USHORT aTypes[5] = {
        /*0*/   RES_DOCINFOFLD,
        /*1*/   RES_AUTHORFLD,
        /*2*/   RES_EXTUSERFLD,
        /*3*/   RES_FILENAMEFLD,
        /*4*/   RES_DATETIMEFLD };  // MUSS am Ende stehen!!

    USHORT nStt = bOnlyTimeDate ? 4 : 0;

    for( ; nStt < 5; ++nStt )
    {
        SwFieldType* pFldType = GetSysFldType( aTypes[ nStt ] );
        SwClientIter aDocInfIter( *pFldType );

        for( SwFmtFld* pFld = (SwFmtFld*)aDocInfIter.First( TYPE( SwFmtFld ));
                pFld; pFld = (SwFmtFld*)aDocInfIter.Next() )
        {
            if( pFld && pFld->GetTxtFld() )
            {
                BOOL bChgd = FALSE;
                switch( aTypes[ nStt ] )
                {
                case RES_DOCINFOFLD:
                    if( ((SwDocInfoField*)pFld->GetFld())->IsFixed() )
                    {
                        bChgd = TRUE;
                        SwDocInfoField* pDocInfFld = (SwDocInfoField*)pFld->GetFld();
                        pDocInfFld->SetExpansion( ((SwDocInfoFieldType*)
                                    pDocInfFld->GetTyp())->Expand(
                                        pDocInfFld->GetSubType(),
                                        pDocInfFld->GetFormat(),
                                        pDocInfFld->GetLanguage() ) );
                    }
                    break;

                case RES_AUTHORFLD:
                    if( ((SwAuthorField*)pFld->GetFld())->IsFixed() )
                    {
                        bChgd = TRUE;
                        SwAuthorField* pAuthorFld = (SwAuthorField*)pFld->GetFld();
                        pAuthorFld->SetExpansion( ((SwAuthorFieldType*)
                                    pAuthorFld->GetTyp())->Expand(
                                                pAuthorFld->GetFormat() ) );
                    }
                    break;

                case RES_EXTUSERFLD:
                    if( ((SwExtUserField*)pFld->GetFld())->IsFixed() )
                    {
                        bChgd = TRUE;
                        SwExtUserField* pExtUserFld = (SwExtUserField*)pFld->GetFld();
                        pExtUserFld->SetExpansion( ((SwExtUserFieldType*)
                                    pExtUserFld->GetTyp())->Expand(
                                            pExtUserFld->GetSubType(),
                                            pExtUserFld->GetFormat()));
                    }
                    break;

                case RES_DATETIMEFLD:
                    if( ((SwDateTimeField*)pFld->GetFld())->IsFixed() )
                    {
                        bChgd = TRUE;
                        ((SwDateTimeField*)pFld->GetFld())->SetDateTime(
                                                    nDate, nTime );
                    }
                    break;

                case RES_FILENAMEFLD:
                    if( ((SwFileNameField*)pFld->GetFld())->IsFixed() )
                    {
                        bChgd = TRUE;
                        SwFileNameField* pFileNameFld =
                            (SwFileNameField*)pFld->GetFld();
                        pFileNameFld->SetExpansion( ((SwFileNameFieldType*)
                                    pFileNameFld->GetTyp())->Expand(
                                            pFileNameFld->GetFormat() ) );
                    }
                    break;
                }

                // Formatierung anstossen
                if( bChgd )
                    pFld->Modify( 0, 0 );
            }
        }
    }

    if( !bIsModified )
        ResetModified();
}

BOOL SwDoc::SetFieldsDirty( BOOL b, const SwNode* pChk, ULONG nLen )
{
    // teste ggfs. mal, ob die angegbenen Nodes ueberhaupt Felder beinhalten.
    // wenn nicht, braucht das Flag nicht veraendert werden.
    BOOL bFldsFnd = FALSE;
    if( b && pChk && !GetUpdtFlds().IsFieldsDirty() && !IsInDtor()
        // ?? was ist mit Undo, da will man es doch auch haben !!
        /*&& &pChk->GetNodes() == &GetNodes()*/ )
    {
        b = FALSE;
        if( !nLen )
            ++nLen;
        ULONG nStt = pChk->GetIndex();
        const SwNodes& rNds = pChk->GetNodes();
        while( nLen-- )
        {
            const SwTxtNode* pTNd = rNds[ nStt++ ]->GetTxtNode();
            if( pTNd )
            {
                if( pTNd->GetFmtColl() &&
                    MAXLEVEL > pTNd->GetTxtColl()->GetOutlineLevel() )
                    // Kapitelfelder aktualisieren
                    b = TRUE;
                else if( pTNd->GetpSwpHints() && pTNd->GetSwpHints().Count() )
                    for( USHORT n = 0, nEnd = pTNd->GetSwpHints().Count();
                            n < nEnd; ++n )
                    {
                        const SwTxtAttr* pAttr = pTNd->GetSwpHints()[ n ];
                        if( RES_TXTATR_FIELD == pAttr->Which() )
                        {
                            b = TRUE;
                            break;
                        }
                    }

                if( b )
                    break;
            }
        }
        bFldsFnd = b;
    }
    GetUpdtFlds().SetFieldsDirty( b );
    return bFldsFnd;
}
/* -----------------------------21.12.99 12:55--------------------------------

 ---------------------------------------------------------------------------*/
void SwDoc::ChangeAuthorityData( const SwAuthEntry* pNewData )
{
    const USHORT nSize = pFldTypes->Count();

    for( USHORT i = INIT_FLDTYPES; i < nSize; ++i )
    {
        SwFieldType* pFldType = (*pFldTypes)[i];
        if( RES_AUTHORITY  == pFldType->Which() )
        {
            SwAuthorityFieldType* pAuthType = (SwAuthorityFieldType*)pFldType;
            pAuthType->ChangeEntryContent(pNewData);
            break;
        }
    }

}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDocUpdtFld::InsDelFldInFldLst( BOOL bIns, const SwTxtFld& rFld )
{
    USHORT nWhich = rFld.GetFld().GetFld()->GetTyp()->Which();
    switch( nWhich )
    {
    case RES_DBFLD:
    case RES_SETEXPFLD:
    case RES_HIDDENPARAFLD:
    case RES_HIDDENTXTFLD:
    case RES_DBNUMSETFLD:
    case RES_DBNEXTSETFLD:
    case RES_DBSETNUMBERFLD:
    case RES_GETEXPFLD:
        break;          // diese muessen ein-/ausgetragen werden!

    default:
        return;
    }

    SetFieldsDirty( TRUE );
    if( !pFldSortLst )
    {
        if( !bIns )             // keine Liste vorhanden und loeschen
            return;             // dann nichts tun
        pFldSortLst = new _SetGetExpFlds( 64, 16 );
    }

    if( bIns )      // neu einfuegen:
        GetBodyNode( rFld, nWhich );
    else
    {
        // ueber den pTxtFld Pointer suchen. Ist zwar eine Sortierte
        // Liste, aber nach Node-Positionen sortiert. Bis dieser
        // bestimmt ist, ist das Suchen nach dem Pointer schon fertig
        for( USHORT n = 0; n < pFldSortLst->Count(); ++n )
            if( &rFld == (*pFldSortLst)[ n ]->GetPointer() )
                pFldSortLst->DeleteAndDestroy( n--, 1 );
                // ein Feld kann mehrfach vorhanden sein!
    }
}

void SwDocUpdtFld::_MakeFldList( SwDoc& rDoc, int eGetMode )
{
    // neue Version: gehe ueber alle Felder vom Attribut-Pool
    if( pFldSortLst )
        delete pFldSortLst;
    pFldSortLst = new _SetGetExpFlds( 64, 16 );

    // zuerst die Bereiche einsammeln. Alle die ueber Bedingung
    // gehiddet sind, wieder mit Frames versorgen, damit die darin
    // enthaltenen Felder richtig einsortiert werden!!!
    {
        // damit die Frames richtig angelegt werden, muessen sie in der
        // Reihenfolgen von oben nach unten expandiert werden
        SvULongs aTmpArr;
        SwSectionFmts& rArr = rDoc.GetSections();
        SwSectionNode* pSectNd;
        USHORT nArrStt = 0;
        ULONG nSttCntnt = rDoc.GetNodes().GetEndOfExtras().GetIndex();
        for( USHORT n = rArr.Count(); n; )
        {
            SwSection* pSect = rArr[ --n ]->GetSection();
            if( pSect->IsHidden() && pSect->GetCondition().Len() &&
                0 != ( pSectNd = pSect->GetFmt()->GetSectionNode() ))
            {
                ULONG nIdx = pSectNd->GetIndex();
                for( USHORT i = 0;
                    i < aTmpArr.Count() && aTmpArr[ i ] < nIdx;
                    ++i )
                    ;
                aTmpArr.Insert( nIdx, i );
                if( nIdx < nSttCntnt )
                    ++nArrStt;
            }
        }

        // erst alle anzeigen, damit die Frames vorhanden sind. Mit deren
        // Position wird das BodyAnchor ermittelt.
        // Dafuer erst den ContentBereich, dann die Sonderbereiche!!!
        for( n = nArrStt; n < aTmpArr.Count(); ++n )
        {
            pSectNd = rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode();
            ASSERT( pSectNd, "Wo ist mein SectionNode" );
            pSectNd->GetSection().SetCondHidden( FALSE );
        }
        for( n = 0; n < nArrStt; ++n )
        {
            pSectNd = rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode();
            ASSERT( pSectNd, "Wo ist mein SectionNode" );
            pSectNd->GetSection().SetCondHidden( FALSE );
        }

        // so, erst jetzt alle sortiert in die Liste eintragen
        for( n = 0; n < aTmpArr.Count(); ++n )
            GetBodyNode( *rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode() );
    }

    String sTrue( String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "TRUE" ))),
           sFalse( String::CreateFromAscii(
                                       RTL_CONSTASCII_STRINGPARAM( "FALSE" )));

    BOOL bIsDBMgr = 0 != rDoc.GetNewDBMgr();
    USHORT nWhich, n;
    const String* pFormel = 0;
    const SfxPoolItem* pItem;
    USHORT nMaxItems = rDoc.GetAttrPool().GetItemCount( RES_TXTATR_FIELD );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 == (pItem = rDoc.GetAttrPool().GetItem( RES_TXTATR_FIELD, n ) ))
            continue;

        const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
        const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            continue;

        const SwField* pFld = pFmtFld->GetFld();
        switch( nWhich = pFld->GetTyp()->Which() )
        {
            case RES_DBSETNUMBERFLD:
            case RES_GETEXPFLD:
                if( GETFLD_ALL == eGetMode )
                    pFormel = &sTrue;
                break;

            case RES_DBFLD:
                if( GETFLD_EXPAND & eGetMode )
                    pFormel = &sTrue;
                break;

            case RES_SETEXPFLD:
                if( ( GSE_STRING & pFld->GetSubType()
                        ? GETFLD_EXPAND : GETFLD_CALC )
                        & eGetMode )
                    pFormel = &sTrue;
                break;

            case RES_HIDDENPARAFLD:
                if( GETFLD_ALL == eGetMode )
                {
                    pFormel = &pFld->GetPar1();
                    if( !pFormel->Len() || pFormel->Equals( sFalse ))
                        ((SwHiddenParaField*)pFld)->SetHidden( FALSE );
                    else if( pFormel->Equals( sTrue ))
                        ((SwHiddenParaField*)pFld)->SetHidden( TRUE );
                    else
                        break;

                    pFormel = 0;
                    // Formatierung anstossen
                    ((SwFmtFld*)pFmtFld)->Modify( 0, 0 );
                }
                break;

            case RES_HIDDENTXTFLD:
                if( GETFLD_ALL == eGetMode )
                {
                    pFormel = &pFld->GetPar1();
                    if( !pFormel->Len() || pFormel->Equals( sFalse ))
                        ((SwHiddenTxtField*)pFld)->SetValue( TRUE );
                    else if( pFormel->Equals( sTrue ))
                        ((SwHiddenTxtField*)pFld)->SetValue( FALSE );
                    else
                        break;

                    pFormel = 0;

                    // Feld Evaluieren
                    ((SwHiddenTxtField*)pFld)->Evaluate(&rDoc);
                    // Formatierung anstossen
                    ((SwFmtFld*)pFmtFld)->Modify( 0, 0 );
                }
                break;

            case RES_DBNUMSETFLD:
            {
#ifdef REPLACE_OFADBMGR
                String sDBName(((SwDBNumSetField*)pFld)->GetDBName(&rDoc));
                String sSourceName(sDBName.GetToken(0, DB_DELIM));
                String sTableName(sDBName.GetToken(0).GetToken(1, DB_DELIM));

                if( bIsDBMgr &&
                    rDoc.GetNewDBMgr()->OpenDataSource( sSourceName, sTableName )&&
                    GETFLD_ALL == eGetMode ||
                    ( GETFLD_CALC & eGetMode &&
                        ((SwDBNumSetField*)pFld)->IsCondValid()))
#else
                if( bIsDBMgr && rDoc.GetNewDBMgr()->OpenDB(DBMGR_STD,
                                ((SwDBNumSetField*)pFld)->GetDBName(&rDoc)) &&
                    GETFLD_ALL == eGetMode ||
                    ( GETFLD_CALC & eGetMode &&
                        ((SwDBNumSetField*)pFld)->IsCondValid() ))
#endif
                    pFormel = &pFld->GetPar1();
            }
            break;
            case RES_DBNEXTSETFLD:
            {
#ifdef REPLACE_OFADBMGR
                String sDBName(((SwDBNextSetField*)pFld)->GetDBName(&rDoc));
                String sSourceName(sDBName.GetToken(0, DB_DELIM));
                String sTableName(sDBName.GetToken(0).GetToken(1, DB_DELIM));

                if( bIsDBMgr &&
                    rDoc.GetNewDBMgr()->OpenDataSource( sSourceName, sTableName )&&
                    GETFLD_ALL == eGetMode ||
                    ( GETFLD_CALC & eGetMode &&
                        ((SwDBNextSetField*)pFld)->IsCondValid() ))
#else
                if( bIsDBMgr && rDoc.GetNewDBMgr()->OpenDB(DBMGR_STD,
                                ((SwDBNextSetField*)pFld)->GetDBName(&rDoc)) &&
                    GETFLD_ALL == eGetMode ||
                    ( GETFLD_CALC & eGetMode &&
                        ((SwDBNextSetField*)pFld)->IsCondValid() ))
#endif
                    pFormel = &pFld->GetPar1();
            }
            break;
        }

        if( pFormel && pFormel->Len() )
        {
            GetBodyNode( *pTxtFld, nWhich );
            pFormel = 0;
        }
    }
    nFldLstGetMode = eGetMode;
    nNodes = rDoc.GetNodes().Count();

#ifdef JP_DEBUG
    {
    SvFileStream sOut( "f:\\x.x", STREAM_STD_WRITE );
    sOut.Seek( STREAM_SEEK_TO_END );
    sOut << "------------------" << endl;
    const _SetGetExpFldPtr* pSortLst = pFldSortLst->GetData();
    for( USHORT n = pFldSortLst->Count(); n; --n, ++pSortLst )
    {
        String sStr( (*pSortLst)->GetNode() );
        sStr += "\t, ";
        sStr += (*pSortLst)->GetCntnt();
        sStr += "\tNode: ";
        sStr += (*pSortLst)->GetFld()->GetTxtNode().StartOfSectionIndex();
        sStr += "\tPos: ";
        sStr += *(*pSortLst)->GetFld()->GetStart();
        sStr += "\tType: ";
        sStr += (*pSortLst)->GetFld()->GetFld().GetFld()->GetTyp()->Which();

        sOut << sStr.GetStr() << endl;
    }
    }
#endif
    // JP_DEBUG
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDocUpdtFld::GetBodyNode( const SwTxtFld& rTFld, USHORT nFldWhich )
{
    const SwTxtNode& rTxtNd = rTFld.GetTxtNode();
    const SwDoc& rDoc = *rTxtNd.GetDoc();

    // immer den ersten !! (in Tab-Headline, Kopf-/Fuss )
    Point aPt;
    const SwCntntFrm* pFrm = rTxtNd.GetFrm( &aPt, 0, FALSE );

    _SetGetExpFld* pNew;
    BOOL bIsInBody = FALSE;

    if( !pFrm || pFrm->IsInDocBody() )
    {
        // einen Index fuers bestimmen vom TextNode anlegen
        SwNodeIndex aIdx( rTxtNd );
        bIsInBody = rDoc.GetNodes().GetEndOfExtras().GetIndex() < aIdx.GetIndex();
        pNew = new _SetGetExpFld( aIdx, &rTFld );
    }
    else
    {
        // einen Index fuers bestimmen vom TextNode anlegen
        SwPosition aPos( rDoc.GetNodes().GetEndOfPostIts() );
#ifndef PRODUCT
        ASSERT( GetBodyTxtNode( rDoc, aPos, *pFrm ), "wo steht das Feld" );
#else
        GetBodyTxtNode( rDoc, aPos, *pFrm );
#endif
        pNew = new _SetGetExpFld( aPos.nNode, &rTFld, &aPos.nContent );
    }

    // bei GetExp.-/DB.-Felder immer das BodyTxtFlag setzen
    if( RES_GETEXPFLD == nFldWhich )
    {
        SwGetExpField* pGetFld = (SwGetExpField*)rTFld.GetFld().GetFld();
        pGetFld->ChgBodyTxtFlag( bIsInBody );
    }
    else if( RES_DBFLD == nFldWhich )
    {
        SwDBField* pDBFld = (SwDBField*)rTFld.GetFld().GetFld();
        pDBFld->ChgBodyTxtFlag( bIsInBody );
    }

    if( !pFldSortLst->Insert( pNew ))
        delete pNew;
}

void SwDocUpdtFld::GetBodyNode( const SwSectionNode& rSectNd )
{
    const SwDoc& rDoc = *rSectNd.GetDoc();
    _SetGetExpFld* pNew = 0;

    if( rSectNd.GetIndex() < rDoc.GetNodes().GetEndOfExtras().GetIndex() )
    {
        do {            // middle check loop

            // dann muessen wir uns mal den Anker besorgen!
            // einen Index fuers bestimmen vom TextNode anlegen
            SwPosition aPos( rSectNd );
            SwCntntNode* pCNd = rDoc.GetNodes().GoNext( &aPos.nNode ); // zum naechsten ContentNode

            if( !pCNd || !pCNd->IsTxtNode() )
                break;

            // immer den ersten !! (in Tab-Headline, Kopf-/Fuss )
            Point aPt;
            const SwCntntFrm* pFrm = pCNd->GetFrm( &aPt, 0, FALSE );
            if( !pFrm )
                break;

#ifndef PRODUCT
            ASSERT( GetBodyTxtNode( rDoc, aPos, *pFrm ), "wo steht das Feld" );
#else
            GetBodyTxtNode( rDoc, aPos, *pFrm );
#endif
            pNew = new _SetGetExpFld( rSectNd, &aPos );

        } while( FALSE );
    }

    if( !pNew )
        pNew = new _SetGetExpFld( rSectNd );

    if( !pFldSortLst->Insert( pNew ))
        delete pNew;
}

void SwDocUpdtFld::InsertFldType( const SwFieldType& rType )
{
    String sFldName;
    switch( rType.Which() )
    {
    case RES_USERFLD :
        sFldName = ((SwUserFieldType&)rType).GetName();
        break;
    case RES_SETEXPFLD:
        sFldName = ((SwSetExpFieldType&)rType).GetName();
        break;
    default:
        ASSERT( !this, "kein gueltiger FeldTyp" );
    }

    if( sFldName.Len() )
    {
        SetFieldsDirty( TRUE );
        // suchen und aus der HashTabelle entfernen
        GetAppCharClass().toLower( sFldName );
        USHORT n;

        SwHash* pFnd = Find( sFldName, GetFldTypeTable(), TBLSZ, &n );

        if( !pFnd )
        {
            SwCalcFldType* pNew = new SwCalcFldType( sFldName, &rType );
            pNew->pNext = aFldTypeTable[ n ];
            aFldTypeTable[ n ] = pNew;
        }
    }
}

void SwDocUpdtFld::RemoveFldType( const SwFieldType& rType )
{
    String sFldName;
    switch( rType.Which() )
    {
    case RES_USERFLD :
        sFldName = ((SwUserFieldType&)rType).GetName();
        break;
    case RES_SETEXPFLD:
        sFldName = ((SwSetExpFieldType&)rType).GetName();
        break;
    }

    if( sFldName.Len() )
    {
        SetFieldsDirty( TRUE );
        // suchen und aus der HashTabelle entfernen
        GetAppCharClass().toLower( sFldName );
        USHORT n;

        SwHash* pFnd = Find( sFldName, GetFldTypeTable(), TBLSZ, &n );
        if( pFnd )
        {
            if( aFldTypeTable[ n ] == pFnd )
                aFldTypeTable[ n ] = (SwCalcFldType*)pFnd->pNext;
            else
            {
                SwHash* pPrev = aFldTypeTable[ n ];
                while( pPrev->pNext != pFnd )
                    pPrev = pPrev->pNext;
                pPrev->pNext = pFnd->pNext;
            }
            pFnd->pNext = 0;
            delete pFnd;
        }
    }
}

SwDocUpdtFld::SwDocUpdtFld()
    : pFldSortLst( 0 ), nFldLstGetMode( 0 ), nFldUpdtPos( LONG_MAX )
{
    bInUpdateFlds = bFldsDirty = FALSE;
    memset( aFldTypeTable, 0, sizeof( aFldTypeTable ) );
}

SwDocUpdtFld::~SwDocUpdtFld()
{
    delete pFldSortLst;

    for( USHORT n = 0; n < TBLSZ; ++n )
        delete aFldTypeTable[n];
}




