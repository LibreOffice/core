/*************************************************************************
 *
 *  $RCSfile: atrfld.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:27 $
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

#include "doc.hxx"          // Update fuer UserFields
#include "fldbas.hxx"          // fuer FieldType

#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#include "reffld.hxx"
#include "ddefld.hxx"
#include "usrfld.hxx"
#include "expfld.hxx"
#include "swfont.hxx"       // fuer GetFldsColor
#include "ndtxt.hxx"        // SwTxtNode
#include "calc.hxx"         // Update fuer UserFields
#include "hints.hxx"

TYPEINIT2( SwFmtFld, SfxPoolItem, SwClient )

/****************************************************************************
 *
 *  class SwFmtFld
 *
 ****************************************************************************/

    // Konstruktor fuers Default vom Attribut-Pool
SwFmtFld::SwFmtFld()
    : SfxPoolItem( RES_TXTATR_FIELD ),
    SwClient( 0 ),
    pField( 0 ),
    pTxtAttr( 0 )
{
}

SwFmtFld::SwFmtFld( const SwField &rFld )
    : SfxPoolItem( RES_TXTATR_FIELD ),
    SwClient( rFld.GetTyp() ),
    pTxtAttr( 0 )
{
    pField = rFld.Copy();
}

SwFmtFld::SwFmtFld( const SwFmtFld& rAttr )
    : SfxPoolItem( RES_TXTATR_FIELD ),
    SwClient( rAttr.GetFld()->GetTyp() ),
    pTxtAttr( 0 )
{
    pField = rAttr.GetFld()->Copy();
}

SwFmtFld::~SwFmtFld()
{
    SwFieldType* pType = pField ? pField->GetTyp() : 0;

    if (pType && pType->Which() == RES_DBFLD)
        pType = 0;  // DB-Feldtypen zerstoeren sich selbst

    delete pField;

    // bei einige FeldTypen muessen wir den FeldTypen noch loeschen
    if( pType && pType->IsLastDepend() )
    {
        BOOL bDel = FALSE;
        switch( pType->Which() )
        {
        case RES_USERFLD:
            bDel = ((SwUserFieldType*)pType)->IsDeleted();
            break;

        case RES_SETEXPFLD:
            bDel = ((SwSetExpFieldType*)pType)->IsDeleted();
            break;

        case RES_DDEFLD:
            bDel = ((SwDDEFieldType*)pType)->IsDeleted();
            break;
        }

        if( bDel )
        {
            // vorm loeschen erstmal austragen
            pType->Remove( this );
            delete pType;
        }
    }
}

int SwFmtFld::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return pField->GetTyp() == ((SwFmtFld&)rAttr).GetFld()->GetTyp() &&
           pField->GetFormat() == ((SwFmtFld&)rAttr).GetFld()->GetFormat();
}

SfxPoolItem* SwFmtFld::Clone( SfxItemPool* ) const
{
    return new SwFmtFld( *this );
}

void SwFmtFld::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    if( !pTxtAttr )
        return;

    SwTxtNode* pTxtNd = (SwTxtNode*)&pTxtAttr->GetTxtNode();
    ASSERT( pTxtNd, "wo ist denn mein Node?" );
    if( pNew )
    {
        switch( pNew->Which() )
        {
        case RES_TXTATR_FLDCHG:
                // "Farbe hat sich geaendert !"
                // this, this fuer "nur Painten"
                pTxtNd->Modify( this, this );
                return;
        case RES_REFMARKFLD_UPDATE:
                // GetReferenz-Felder aktualisieren
                if( RES_GETREFFLD == GetFld()->GetTyp()->Which() )
                    ((SwGetRefField*)GetFld())->UpdateField();
                break;
        case RES_DOCPOS_UPDATE:
                // Je nach DocPos aktualisieren (SwTxtFrm::Modify())
                pTxtNd->Modify( pNew, this );
                return;

        case RES_ATTRSET_CHG:
        case RES_FMT_CHG:
                pTxtNd->Modify( pOld, pNew );
                return;
        }
    }

    switch (GetFld()->GetTyp()->Which())
    {
        case RES_HIDDENPARAFLD:
            if( !pOld || RES_HIDDENPARA_PRINT != pOld->Which() )
                break;
        case RES_DBSETNUMBERFLD:
        case RES_DBNUMSETFLD:
        case RES_DBNEXTSETFLD:
        case RES_DBNAMEFLD:
            pTxtNd->Modify( 0, pNew);
            return;
    }

    if( RES_USERFLD == GetFld()->GetTyp()->Which() )
    {
        SwUserFieldType* pType = (SwUserFieldType*)GetFld()->GetTyp();
        if(!pType->IsValid())
        {
            SwCalc aCalc( *pTxtNd->GetDoc() );
            pType->GetValue( aCalc );
        }
    }
    pTxtAttr->Expand();
}

BOOL SwFmtFld::GetInfo( SfxPoolItem& rInfo ) const
{
    const SwTxtNode* pTxtNd;
    if( RES_AUTOFMT_DOCNODE != rInfo.Which() ||
        !pTxtAttr || 0 == ( pTxtNd = pTxtAttr->GetpTxtNode() ) ||
        &pTxtNd->GetNodes() != ((SwAutoFmtGetDocNode&)rInfo).pNodes )
        return TRUE;

    ((SwAutoFmtGetDocNode&)rInfo).pCntntNode = pTxtNd;
    return FALSE;
}


BOOL SwFmtFld::IsFldInDoc() const
{
    const SwTxtNode* pTxtNd;
    return pTxtAttr && 0 != ( pTxtNd = pTxtAttr->GetpTxtNode() ) &&
            pTxtNd->GetNodes().IsDocNodes();
}

BOOL SwFmtFld::IsProtect() const
{
    const SwTxtNode* pTxtNd;
    return pTxtAttr && 0 != ( pTxtNd = pTxtAttr->GetpTxtNode() ) &&
            pTxtNd->IsProtect();
}

/*************************************************************************
|*
|*                SwTxtFld::SwTxtFld()
|*
|*    Beschreibung      Attribut fuer automatischen Text, Ctor
|*    Ersterstellung    BP 30.04.92
|*    Letzte Aenderung  JP 15.08.94
|*
*************************************************************************/

SwTxtFld::SwTxtFld( const SwFmtFld& rAttr, xub_StrLen nStart )
    : SwTxtAttr( rAttr, nStart ),
    aExpand( rAttr.GetFld()->Expand() ),
    pMyTxtNd( 0 )
{
    ((SwFmtFld&)rAttr).pTxtAttr = this;
}

SwTxtFld::~SwTxtFld( )
{
}

/*************************************************************************
|*
|*                SwTxtFld::Expand()
|*
|*    Beschreibung      exandiert das Feld und tauscht den Text im Node
|*    Ersterstellung    BP 30.04.92
|*    Letzte Aenderung  JP 15.08.94
|*
*************************************************************************/

void SwTxtFld::Expand()
{
    // Wenn das expandierte Feld sich nicht veraendert hat, wird returnt
    ASSERT( pMyTxtNd, "wo ist denn mein Node?" );

    const SwField* pFld = GetFld().GetFld();
    XubString aNewExpand( pFld->Expand() );

    if( aNewExpand == aExpand )
    {
        // Bei Seitennummernfeldern
        const USHORT nWhich = pFld->GetTyp()->Which();
        if( RES_CHAPTERFLD != nWhich && RES_PAGENUMBERFLD != nWhich &&
            RES_REFPAGEGETFLD != nWhich &&
            ( RES_GETEXPFLD != nWhich ||
                ((SwGetExpField*)pFld)->IsInBodyTxt() ) )
        {
            // BP: das muesste man noch optimieren!
            //JP 12.06.97: stimmt, man sollte auf jedenfall eine Status-
            //              aenderung an die Frames posten
            if( pMyTxtNd->CalcVisibleFlag() )
                pMyTxtNd->Modify( 0, 0 );
            return;
        }
    }

    aExpand = aNewExpand;

    // 0, this fuer Formatieren
    pMyTxtNd->Modify( 0, (SfxPoolItem*)&GetFld() );
}

/*************************************************************************
 *                      SwTxtFld::CopyFld()
 *************************************************************************/

void SwTxtFld::CopyFld( SwTxtFld *pDest ) const
{
    ASSERT( pMyTxtNd, "wo ist denn mein Node?" );
    ASSERT( pDest->pMyTxtNd, "wo ist denn mein Node?" );

    SwDoc *pDoc = pMyTxtNd->GetDoc();
    SwDoc *pDestDoc = pDest->pMyTxtNd->GetDoc();

    SwFmtFld& rFmtFld = (SwFmtFld&)pDest->GetFld();
    const USHORT nFldWhich = rFmtFld.GetFld()->GetTyp()->Which();

    if( pDoc != pDestDoc )
    {
        // Die Hints stehen in unterschiedlichen Dokumenten,
        // der Feldtyp muss im neuen Dokument angemeldet werden.
        // Z.B: Kopieren ins ClipBoard.
        SwFieldType* pFldType;
        if( nFldWhich != RES_DBFLD && nFldWhich != RES_USERFLD &&
            nFldWhich != RES_SETEXPFLD && nFldWhich != RES_DDEFLD &&
            RES_AUTHORITY != nFldWhich )
            pFldType = pDestDoc->GetSysFldType( (const RES_FIELDS)nFldWhich );
        else
            pFldType = pDestDoc->InsertFldType( *rFmtFld.GetFld()->GetTyp() );

        // Sonderbehandlung fuer DDE-Felder
        if( RES_DDEFLD == nFldWhich )
        {
            if( rFmtFld.GetTxtFld() )
                ((SwDDEFieldType*)rFmtFld.GetFld()->GetTyp())->DecRefCnt();
            ((SwDDEFieldType*)pFldType)->IncRefCnt();
        }

        ASSERT( pFldType, "unbekannter FieldType" );
        pFldType->Add( &rFmtFld );          // ummelden
        rFmtFld.GetFld()->ChgTyp( pFldType );
    }

    // Expressionfelder Updaten
    if( nFldWhich == RES_SETEXPFLD || nFldWhich == RES_GETEXPFLD ||
        nFldWhich == RES_HIDDENTXTFLD )
    {
        SwTxtFld* pFld = (SwTxtFld*)this;
        pDestDoc->UpdateExpFlds( pFld );
    }
    // Tabellenfelder auf externe Darstellung
    else if( RES_TABLEFLD == nFldWhich &&
        ((SwTblField*)rFmtFld.GetFld())->IsIntrnlName() )
    {
        // erzeuge aus der internen (fuer CORE) die externe (fuer UI) Formel
        const SwTableNode* pTblNd = pMyTxtNd->FindTableNode();
        if( pTblNd )        // steht in einer Tabelle
            ((SwTblField*)rFmtFld.GetFld())->PtrToBoxNm( &pTblNd->GetTable() );
    }
}


