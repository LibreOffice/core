/*************************************************************************
 *
 *  $RCSfile: txmsrt.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:45:23 $
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

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _TXTFLD_HXX
#include <txtfld.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _TXMSRT_HXX
#include <txmsrt.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _AUTHFLD_HXX
#include <authfld.hxx>
#endif

#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif

extern BOOL IsFrameBehind( const SwTxtNode& rMyNd, xub_StrLen nMySttPos,
                           const SwTxtNode& rBehindNd, xub_StrLen nSttPos );

/*--------------------------------------------------------------------
    Beschreibung: Strings initialisieren
 --------------------------------------------------------------------*/

USHORT SwTOXSortTabBase::nOpt = 0;

SV_IMPL_VARARR( SwTOXSources, SwTOXSource )


SwTOXInternational::SwTOXInternational( LanguageType nLang )
    : eLang( nLang )
{
    pIntl = new International( eLang );
    pCharClass = new CharClass( SvxCreateLocale( eLang ));
}

SwTOXInternational::SwTOXInternational( const SwTOXInternational& rIntl )
    : eLang( rIntl.eLang )
{
    pIntl = new International( *rIntl.pIntl );
    pCharClass = new CharClass( SvxCreateLocale( rIntl.eLang ));
}

SwTOXInternational::~SwTOXInternational()
{
    delete pCharClass;
    delete pIntl;
}

String SwTOXInternational::ToUpper( const String& rStr, xub_StrLen nPos ) const
{
    return pCharClass->toUpper( rStr, nPos, 1 );
}
inline BOOL SwTOXInternational::IsNumeric( const String& rStr ) const
{
    return pCharClass->isNumeric( rStr );
}

/*--------------------------------------------------------------------
     Beschreibung:  SortierElement fuer Verzeichniseintraege
 --------------------------------------------------------------------*/


SwTOXSortTabBase::SwTOXSortTabBase( TOXSortType nTyp, const SwCntntNode* pNd,
                                    const SwTxtTOXMark* pMark,
                                    const SwTOXInternational* pInter )
    : pTxtMark( pMark ), pTOXNd( 0 ), nPos( 0 ), nType( nTyp ),
    pTOXIntl( pInter ), bValidTxt( FALSE ), nCntPos( 0 )
{
    if( pNd )
    {
        xub_StrLen n = 0;
        if( pTxtMark )
            n = *pTxtMark->GetStart();
        SwTOXSource aTmp( pNd, n,
                    pTxtMark ? pTxtMark->GetTOXMark().IsMainEntry() : FALSE );
        aTOXSources.Insert( aTmp, aTOXSources.Count() );

        nPos = pNd->GetIndex();

        switch( nTyp )
        {
        case TOX_SORT_CONTENT:
        case TOX_SORT_PARA:
        case TOX_SORT_TABLE:
            // falls sie in Sonderbereichen stehen, sollte man die
            // Position im Body besorgen
            if( nPos < pNd->GetNodes().GetEndOfExtras().GetIndex() )
            {
                // dann die "Anker" (Body) Position holen.
                Point aPt;
                const SwCntntFrm* pFrm = pNd->GetFrm( &aPt, 0, FALSE );
                if( pFrm )
                {
                    SwPosition aPos( *pNd );
                    const SwDoc& rDoc = *pNd->GetDoc();
#ifndef PRODUCT
                    ASSERT( GetBodyTxtNode( rDoc, aPos, *pFrm ),
                            "wo steht der Absatz" );
#else
                    GetBodyTxtNode( rDoc, aPos, *pFrm );
#endif
                    nPos = aPos.nNode.GetIndex();
                    nCntPos = aPos.nContent.GetIndex();
                }
            }
            else
                nCntPos = n;
            break;
        }
    }
}


String SwTOXSortTabBase::GetURL() const
{
    return aEmptyStr;
}

void SwTOXSortTabBase::FillText( SwTxtNode& rNd, const SwIndex& rInsPos,
                                    USHORT ) const
{
    rNd.Insert( GetTxt(), rInsPos );
}

BOOL SwTOXSortTabBase::operator==( const SwTOXSortTabBase& rCmp )
{
    BOOL bRet = nPos == rCmp.nPos && nCntPos == rCmp.nCntPos &&
            (!aTOXSources[0].pNd || !rCmp.aTOXSources[0].pNd ||
            aTOXSources[0].pNd == rCmp.aTOXSources[0].pNd );

    if( TOX_SORT_CONTENT == nType )
    {
        bRet = bRet && pTxtMark && rCmp.pTxtMark &&
                *pTxtMark->GetStart() == *rCmp.pTxtMark->GetStart();

        if( bRet )
        {
            // beide Pointer vorhanden -> vergleiche Text
            // beide Pointer nicht vorhanden -> vergleiche AlternativText
            const xub_StrLen *pEnd  = pTxtMark->GetEnd(),
                                *pEndCmp = rCmp.pTxtMark->GetEnd();

            bRet = ( ( pEnd && pEndCmp ) || ( !pEnd && !pEndCmp ) ) &&
                    pTOXIntl->IsEqual( GetTxt(), rCmp.GetTxt(),
                                        INTN_COMPARE_IGNORECASE );
        }
    }
    return bRet;
}

BOOL SwTOXSortTabBase::operator<( const SwTOXSortTabBase& rCmp )
{
    if( nPos < rCmp.nPos )
        return TRUE;

    if( nPos == rCmp.nPos )
    {
        if( nCntPos < rCmp.nCntPos )
            return TRUE;

        if( nCntPos == rCmp.nCntPos )
        {
            const SwNode* pFirst = aTOXSources[0].pNd;
            const SwNode* pNext = rCmp.aTOXSources[0].pNd;

            if( pFirst && pFirst == pNext )
            {
                if( TOX_SORT_CONTENT == nType && pTxtMark && rCmp.pTxtMark )
                {
                    if( *pTxtMark->GetStart() < *rCmp.pTxtMark->GetStart() )
                        return TRUE;

                    if( *pTxtMark->GetStart() == *rCmp.pTxtMark->GetStart() )
                    {
                        const xub_StrLen *pEnd = pTxtMark->GetEnd(),
                                            *pEndCmp = rCmp.pTxtMark->GetEnd();

                        // beide Pointer vorhanden -> vergleiche Text
                        // beide Pointer nicht vorhanden -> vergleiche AlternativText
                        if( ( pEnd && pEndCmp ) || ( !pEnd && !pEndCmp ) )
                            return pTOXIntl->IsLess( GetTxt(), rCmp.GetTxt(),
                                                    INTN_COMPARE_IGNORECASE );

                        if( pEnd && !pEndCmp )
                            return TRUE;
                    }
                }
            }
            else if( pFirst && pFirst->IsTxtNode() &&
                     pNext && pNext->IsTxtNode() )
                    return ::IsFrameBehind( *(SwTxtNode*)pNext, nCntPos,
                                            *(SwTxtNode*)pFirst, nCntPos );
        }
    }
    return FALSE;
}


/*--------------------------------------------------------------------
     Beschreibung: sortierter Stichworteintrag
 --------------------------------------------------------------------*/


SwTOXIndex::SwTOXIndex( const SwTxtNode& rNd,
                                const SwTxtTOXMark* pMark, USHORT nOptions,
                                BYTE nKyLevel,
                                const SwTOXInternational& rIntl )
    : SwTOXSortTabBase( TOX_SORT_INDEX, &rNd, pMark, &rIntl ),
    nKeyLevel(nKyLevel)
{
    nPos = rNd.GetIndex();
    nOpt = nOptions;
}

//
// Stichworte vergleichen. Bezieht sich nur auf den Text
//


BOOL SwTOXIndex::operator==( const SwTOXSortTabBase& rCmpBase )
{
    SwTOXIndex& rCmp = (SwTOXIndex&)rCmpBase;

    // In Abhaengigkeit von den Optionen Grosskleinschreibung beachten
    BOOL bRet;
    if(GetLevel() != rCmp.GetLevel() || nKeyLevel != rCmp.nKeyLevel)
        return FALSE;

    String sMyTxt( GetTxt() ), sOtherTxt( rCmp.GetTxt() );
    sMyTxt.Insert( pTOXIntl->GetIndexChar( sMyTxt ), 0 );
    sOtherTxt.Insert( pTOXIntl->GetIndexChar( sOtherTxt ), 0 );

    USHORT nCmpFlags;
    if( GetOptions() & TOI_CASE_SENSITIVE )
        nCmpFlags = 0;
    else
        nCmpFlags = INTN_COMPARE_IGNORECASE;
    bRet = pTOXIntl->IsEqual( sMyTxt, sOtherTxt, nCmpFlags );

    // Wenn nicht zusammengefasst wird muss die Pos aus gewertet werden
    if(bRet && !(GetOptions() & TOI_SAME_ENTRY))
        bRet = nPos == rCmp.nPos;

    return bRet;
}

//
// kleiner haengt nur vom Text ab


//

BOOL SwTOXIndex::operator<( const SwTOXSortTabBase& rCmpBase )
{
    SwTOXIndex& rCmp = (SwTOXIndex&)rCmpBase;

    USHORT nFlag = GetOptions() & TOI_CASE_SENSITIVE ? 0 : INTN_COMPARE_IGNORECASE;

    String sMyTxt( GetTxt() ), sOtherTxt( rCmp.GetTxt() );
    sMyTxt.Insert( pTOXIntl->GetIndexChar( sMyTxt ), 0 );
    sOtherTxt.Insert( pTOXIntl->GetIndexChar( sOtherTxt ), 0 );

    BOOL bRet = pTOXIntl->IsLess( sMyTxt, sOtherTxt, nFlag ) &&
                GetLevel() == rCmp.GetLevel();

    // Wenn nicht zusammengefasst wird muss die Pos aus gewertet werden
    if( !bRet && !(GetOptions() & TOI_SAME_ENTRY) )
        bRet = pTOXIntl->IsEqual( sMyTxt, sOtherTxt, nFlag ) &&
                nPos < rCmp.nPos;

    return bRet;
}

//
// Das Stichwort selbst


//

void SwTOXIndex::_GetText( String& rTxt )
{
    ASSERT(pTxtMark, "pTxtMark == 0, Kein Stichwort");
    const SwTOXMark& rTOXMark = pTxtMark->GetTOXMark();
    switch(nKeyLevel)
    {
        case FORM_PRIMARY_KEY    :
            rTxt = rTOXMark.GetPrimaryKey();
        break;
        case FORM_SECONDARY_KEY  :
            rTxt = rTOXMark.GetSecondaryKey();
        break;
        case FORM_ENTRY          :
            rTxt = rTOXMark.GetText();
        break;
    }
    // if TOI_INITIAL_CAPS is set, first character is to be capitalized
    if( TOI_INITIAL_CAPS & nOpt && pTOXIntl )
    {
        String sUpper( pTOXIntl->ToUpper( rTxt, 0 ));
        rTxt.Erase( 0, 1 ).Insert( sUpper, 0 );
    }
}

void SwTOXIndex::FillText( SwTxtNode& rNd, const SwIndex& rInsPos, USHORT ) const
{
    const xub_StrLen* pEnd = pTxtMark->GetEnd();
    String sTmp;
    if( pEnd && !pTxtMark->GetTOXMark().IsAlternativeText() &&
            0 == (GetOptions() & TOI_KEY_AS_ENTRY))
    {
        sTmp = ((SwTxtNode*)aTOXSources[0].pNd)->GetExpandTxt(
                            *pTxtMark->GetStart(),
                            *pEnd - *pTxtMark->GetStart());
        if(TOI_INITIAL_CAPS&nOpt && pTOXIntl)
        {
            String sUpper( pTOXIntl->ToUpper( sTmp, 0 ));
            sTmp.Erase( 0, 1 ).Insert( sUpper, 0 );
        }
    }
    else
        sTmp = GetTxt();

    rNd.Insert( sTmp, rInsPos );
}



USHORT SwTOXIndex::GetLevel() const
{
    ASSERT(pTxtMark, "pTxtMark == 0, Kein Stichwort");

    USHORT nForm = FORM_PRIMARY_KEY;

    if( 0 == (GetOptions() & TOI_KEY_AS_ENTRY)&&
        pTxtMark->GetTOXMark().GetPrimaryKey().Len() )
    {
        nForm = FORM_SECONDARY_KEY;
        if( pTxtMark->GetTOXMark().GetSecondaryKey().Len() )
            nForm = FORM_ENTRY;
    }
    return nForm;
}

/*--------------------------------------------------------------------
     Beschreibung: Schluessel und Trennzeichen
 --------------------------------------------------------------------*/


SwTOXCustom::SwTOXCustom(const String& rStr, USHORT nLevel,
                            const SwTOXInternational& rIntl )
    : SwTOXSortTabBase( TOX_SORT_CUSTOM, 0, 0, &rIntl ),
    aKey(rStr), nLev(nLevel)
{
}


BOOL SwTOXCustom::operator==(const SwTOXSortTabBase& rCmpBase)
{
    return pTOXIntl->IsEqual( GetTxt(), rCmpBase.GetTxt(), 0 ) &&
            GetLevel() == rCmpBase.GetLevel();
}


BOOL SwTOXCustom::operator < (const SwTOXSortTabBase& rCmpBase)
{
    return pTOXIntl->IsLess( GetTxt(), rCmpBase.GetTxt(), 0 ) &&
            GetLevel() <= rCmpBase.GetLevel();
}


USHORT SwTOXCustom::GetLevel() const
{
    return nLev;
}


void SwTOXCustom::_GetText( String& rTxt )
{
    rTxt = aKey;
}

/*--------------------------------------------------------------------
     Beschreibung: sortierter Inhaltsverz. Eintrag
 --------------------------------------------------------------------*/


SwTOXContent::SwTOXContent( const SwTxtNode& rNd, const SwTxtTOXMark* pMark,
                        const SwTOXInternational& rIntl)
    : SwTOXSortTabBase( TOX_SORT_CONTENT, &rNd, pMark, &rIntl )
{
}


//  Der Text des Inhalts
//

void SwTOXContent::_GetText( String& rTxt )
{
    const xub_StrLen* pEnd = pTxtMark->GetEnd();
    if( pEnd && !pTxtMark->GetTOXMark().IsAlternativeText() )
        rTxt = ((SwTxtNode*)aTOXSources[0].pNd)->GetExpandTxt(
                                     *pTxtMark->GetStart(),
                                     *pEnd - *pTxtMark->GetStart() );
    else
        rTxt = pTxtMark->GetTOXMark().GetAlternativeText();
}

void SwTOXContent::FillText( SwTxtNode& rNd, const SwIndex& rInsPos, USHORT ) const
{
    const xub_StrLen* pEnd = pTxtMark->GetEnd();
    if( pEnd && !pTxtMark->GetTOXMark().IsAlternativeText() )
        ((SwTxtNode*)aTOXSources[0].pNd)->GetExpandTxt( rNd, &rInsPos,
                                    *pTxtMark->GetStart(),
                                    *pEnd - *pTxtMark->GetStart() );
    else
        rNd.Insert( GetTxt(), rInsPos );
}

//
// Die Ebene fuer Anzeige
//


USHORT SwTOXContent::GetLevel() const
{
    return pTxtMark->GetTOXMark().GetLevel();
}

/*--------------------------------------------------------------------
     Beschreibung: Verzeichnis aus Absaetzen zusammengesammelt
 --------------------------------------------------------------------*/

// bei Sortierung von OLE/Grafiken aufpassen !!!
// Die Position darf nicht die im Dokument,
// sondern muss die vom "Henkel" sein  !!


SwTOXPara::SwTOXPara( const SwCntntNode& rNd, SwTOXElement eT, USHORT nLevel )
    : SwTOXSortTabBase( TOX_SORT_PARA, &rNd, 0, 0 ),
    eType( eT ),
    m_nLevel(nLevel),
    nStartIndex(0),
    nEndIndex(STRING_LEN)
{
}


void SwTOXPara::_GetText( String& rTxt )
{
    const SwCntntNode* pNd = aTOXSources[0].pNd;
    switch( eType )
    {
    case TOX_SEQUENCE:
    case TOX_TEMPLATE:
        {
            xub_StrLen nStt = nStartIndex;
/* JP 22.01.98:
    Tabs ueberspringen - macht aber keinen Sinn, solange in der TOX-Form
    nicht die KapitelNummer eingestellt werden kann
            const String& rTmp = ((SwTxtNode*)pNd)->GetTxt();
            while( '\t' == rTmp.GetChar( nStt ) && nStt < rTmp.Len() )
                ++nStt;
*/
            rTxt = ((SwTxtNode*)pNd)->GetExpandTxt(
                    nStt,
                    STRING_NOTFOUND == nEndIndex ? STRING_LEN : nEndIndex - nStt);
        }
        break;

    case TOX_OLE:
    case TOX_GRAPHIC:
    case TOX_FRAME:
        {
            // suche das FlyFormat, dort steht der Object/Grafik-Name
            SwFrmFmt* pFly = pNd->GetFlyFmt();
            if( pFly )
                rTxt = pFly->GetName();
            else
            {
                ASSERT( !this, "Grafik/Object ohne Namen" )
                USHORT nId = TOX_OLE == eType
                                ? STR_OBJECT_DEFNAME
                                : TOX_GRAPHIC == eType
                                    ? STR_GRAPHIC_DEFNAME
                                    : STR_FRAME_DEFNAME;
                rTxt = SW_RESSTR( nId );
            }
        }
        break;
    }
}

void SwTOXPara::FillText( SwTxtNode& rNd, const SwIndex& rInsPos, USHORT ) const
{
    if( TOX_TEMPLATE == eType || TOX_SEQUENCE == eType )
    {
        SwTxtNode* pSrc = (SwTxtNode*)aTOXSources[0].pNd;
        xub_StrLen nStt = nStartIndex;
/* JP 22.01.98:
    Tabs ueberspringen - macht aber keinen Sinn, solange in der TOX-Form
    nicht die KapitelNummer eingestellt werden kann
        const String& rTxt = pSrc->GetTxt();
        while( '\t' == rTxt.GetChar( nStt ) && nStt < rTxt.Len() )
            ++nStt;
*/
        pSrc->GetExpandTxt( rNd, &rInsPos, nStt,
                nEndIndex == STRING_LEN ? STRING_LEN : nEndIndex - nStt );
    }
    else
        rNd.Insert( GetTxt(), rInsPos );
}


USHORT SwTOXPara::GetLevel() const
{
    USHORT nRet = m_nLevel;
    const SwCntntNode*  pNd = aTOXSources[0].pNd;

    if( TOX_TEMPLATE == eType && pNd->GetTxtNode() )
    {
        USHORT nTmp = ((SwTxtNode*)pNd)->GetTxtColl()->GetOutlineLevel();
        if(nTmp < NO_NUMBERING)
            nRet = nTmp + 1;
    }
    return nRet;
}


String SwTOXPara::GetURL() const
{
    String aTxt;
    const SwCntntNode* pNd = aTOXSources[0].pNd;
    switch( eType )
    {
    case TOX_TEMPLATE:
        {
            if( MAXLEVEL >= ((SwTxtNode*)pNd)->GetTxtColl()->GetOutlineLevel())
            {
                aTxt = '#';
                const SwNodeNum* pNum = ((SwTxtNode*)pNd)->GetOutlineNum();
                if( pNum && pNd->GetDoc()->GetOutlineNumRule() )
                {
                    // dann noch die rel. Nummer davor setzen
                    const SwNumRule& rRule = *pNd->GetDoc()->GetOutlineNumRule();
                    for( int n = 0; n <= pNum->GetLevel(); ++n )
                    {
                        int nNum = pNum->GetLevelVal()[ n ];
                        nNum -= ( rRule.Get( n ).GetStart() - 1 );
                        ( aTxt += String::CreateFromInt32( nNum )) += '.';
                    }
                }
                aTxt += INetURLObject::createFragment(
                                    ((SwTxtNode*)pNd)->GetExpandTxt() );
                ( aTxt += cMarkSeperator ).AppendAscii( pMarkToOutline );
            }
        }
        break;

    case TOX_OLE:
    case TOX_GRAPHIC:
    case TOX_FRAME:
        {
            // suche das FlyFormat, dort steht der Object/Grafik-Name
            SwFrmFmt* pFly = pNd->GetFlyFmt();
            if( pFly )
            {
                (( aTxt = '#' ) += pFly->GetName() ) += cMarkSeperator;
                const sal_Char* pStr;
                switch( eType )
                {
                case TOX_OLE:       pStr = pMarkToOLE; break;
                case TOX_GRAPHIC:   pStr = pMarkToGraphic; break;
                case TOX_FRAME:     pStr = pMarkToFrame; break;
                default:            pStr = 0;
                }
                if( pStr )
                    aTxt.AppendAscii( pStr );
            }
        }
        break;
    }
    return aTxt;
}


/*--------------------------------------------------------------------
    Beschreibung: Tabelle
 --------------------------------------------------------------------*/


SwTOXTable::SwTOXTable( const SwCntntNode& rNd )
    : SwTOXSortTabBase( TOX_SORT_TABLE, &rNd, 0, 0 ),
    nLevel(FORM_ALPHA_DELIMITTER)
{
}


void SwTOXTable::_GetText( String& rTxt )
{
    const SwNode* pNd = aTOXSources[0].pNd;
    if( pNd && 0 != ( pNd = pNd->FindTableNode() ) )
    {
        rTxt = ((SwTableNode*)pNd)->GetTable().GetFrmFmt()->GetName();
    }
    else
    {
        ASSERT( !this, "Wo ist meine Tabelle geblieben?" )
        rTxt = SW_RESSTR( STR_TABLE_DEFNAME );
    }
}


USHORT SwTOXTable::GetLevel() const
{
    return nLevel;
}


String SwTOXTable::GetURL() const
{
    String aTxt;
    const SwNode* pNd = aTOXSources[0].pNd;
    if( pNd && 0 != ( pNd = pNd->FindTableNode() ) )
    {
        aTxt = ((SwTableNode*)pNd)->GetTable().GetFrmFmt()->GetName();
        if( aTxt.Len() )
        {
            ( aTxt.Insert( '#', 0 ) += cMarkSeperator ).
                                            AppendAscii( pMarkToTable );
        }
    }
    return aTxt;
}
/*-- 15.09.99 14:28:08---------------------------------------------------

  -----------------------------------------------------------------------*/

SwTOXAuthority::SwTOXAuthority( const SwCntntNode& rNd,
                SwFmtFld& rField, const SwTOXInternational& rIntl ) :
    SwTOXSortTabBase( TOX_SORT_AUTHORITY, &rNd, 0, &rIntl ),
    m_rField(rField)
{
    if(rField.GetTxtFld())
        nCntPos = *rField.GetTxtFld()->GetStart();
}

USHORT  SwTOXAuthority::GetLevel() const
{
    String sText(((SwAuthorityField*)m_rField.GetFld())->
                        GetFieldText(AUTH_FIELD_AUTHORITY_TYPE));
    USHORT nRet = 0;
    if( pTOXIntl->IsNumeric( sText ) )
    {
        nRet = sText.ToInt32();
        nRet++;
    }
    if(nRet >= AUTH_TYPE_END)
        nRet = 0;
    return nRet;
}
/*-- 15.09.99 14:28:08---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwTOXAuthority::_GetText( String& rToFill)
{
    //
    rToFill = m_rField.GetFld()->Expand();
}
/* -----------------21.09.99 12:50-------------------

 --------------------------------------------------*/
void    SwTOXAuthority::FillText( SwTxtNode& rNd,
                        const SwIndex& rInsPos, USHORT nAuthField ) const
{
    SwAuthorityField* pField = (SwAuthorityField*)m_rField.GetFld();
    String sText;
    if(AUTH_FIELD_IDENTIFIER == nAuthField)
    {
        sText = pField->Expand();
        sText.Erase(0, 1);
        sText.Erase(sText.Len() - 1, 1);
    }
    else if(AUTH_FIELD_AUTHORITY_TYPE == nAuthField)
    {
        USHORT nLevel = GetLevel();
        if(nLevel)
            sText = SwAuthorityFieldType::GetAuthTypeName((ToxAuthorityType) --nLevel);
    }
    else
        sText = (pField->GetFieldText((ToxAuthorityField) nAuthField));
    rNd.Insert( sText, rInsPos );
}
/* -----------------14.10.99 09:35-------------------

 --------------------------------------------------*/
BOOL    SwTOXAuthority::operator==( const SwTOXSortTabBase& rCmp)
{
    return nType == rCmp.nType &&
            ((SwAuthorityField*)m_rField.GetFld())->GetHandle() ==
                ((SwAuthorityField*)((SwTOXAuthority&)rCmp).m_rField.GetFld())->GetHandle();
}
/* -----------------21.10.99 09:52-------------------

 --------------------------------------------------*/
BOOL    SwTOXAuthority::operator<( const SwTOXSortTabBase& rBase)
{
    BOOL bRet = FALSE;
    SwAuthorityField* pField = (SwAuthorityField*)m_rField.GetFld();
    SwAuthorityFieldType* pType = (SwAuthorityFieldType*)
                                                pField->GetTyp();
    if(pType->IsSortByDocument())
        bRet = SwTOXSortTabBase::operator<(rBase);
    else
    {
        SwAuthorityField* pCmpField = (SwAuthorityField*)
                        ((SwTOXAuthority&)rBase).m_rField.GetFld();


        for(USHORT i = 0; i < pType->GetSortKeyCount(); i++)
        {
            const SwTOXSortKey* pKey = pType->GetSortKey(i);
            String sText1 = pField->GetFieldText(pKey->eField);
            String sText2 = pCmpField->GetFieldText(pKey->eField);

            StringCompare eComp = pTOXIntl->Compare( sText1, sText2,
                                             INTN_COMPARE_IGNORECASE);
            if( COMPARE_EQUAL != eComp )
            {
                bRet = (COMPARE_LESS == eComp) == pKey->bSortAscending;
                break;
            }
        }
    }
    return bRet;
}


