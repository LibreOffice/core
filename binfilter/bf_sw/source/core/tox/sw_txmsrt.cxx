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

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_COLLATOROPTIONS_HPP_
#include <com/sun/star/i18n/CollatorOptions.hpp>
#endif
#ifndef _UNO_LINGU_HXX
#include <bf_svx/unolingu.hxx>
#endif

#ifndef _TXTFLD_HXX
#include <txtfld.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
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
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _AUTHFLD_HXX
#include <authfld.hxx>
#endif
#ifndef _TOXWRAP_HXX
#include <toxwrap.hxx>
#endif

#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
namespace binfilter {

extern BOOL IsFrameBehind( const SwTxtNode& rMyNd, xub_StrLen nMySttPos,
                           const SwTxtNode& rBehindNd, xub_StrLen nSttPos );

using namespace ::com::sun::star::uno;
using namespace ::rtl;
/*--------------------------------------------------------------------
    Beschreibung: Strings initialisieren
 --------------------------------------------------------------------*/


SV_IMPL_VARARR( SwTOXSources, SwTOXSource )


SwTOXInternational::SwTOXInternational( LanguageType nLang, USHORT nOpt,
                                        const String& rSortAlgorithm ) :
    eLang( nLang ),
    sSortAlgorithm(rSortAlgorithm),
    nOptions( nOpt )
{
    Init();
}

 SwTOXInternational::SwTOXInternational( const SwTOXInternational& rIntl ) :
     eLang( rIntl.eLang ),
     sSortAlgorithm(rIntl.sSortAlgorithm),
     nOptions( rIntl.nOptions )
 {
   Init();
 }

 void SwTOXInternational::Init()
 {
    pIndexWrapper = new IndexEntrySupplierWrapper();
 
     const ::com::sun::star::lang::Locale aLcl( SvxCreateLocale( eLang ) );
     pIndexWrapper->SetLocale( aLcl );
 
     if(!sSortAlgorithm.Len())
     {
         Sequence < OUString > aSeq( pIndexWrapper->GetAlgorithmList( aLcl ));
         if(aSeq.getLength())
             sSortAlgorithm = aSeq.getConstArray()[0];
     }
 
     if ( nOptions & TOI_CASE_SENSITIVE )
         pIndexWrapper->LoadAlgorithm( aLcl, sSortAlgorithm, 0 );
     else
         pIndexWrapper->LoadAlgorithm( aLcl, sSortAlgorithm, SW_COLLATOR_IGNORES );
 
    pCharClass = new CharClass( aLcl );
 
 }

 SwTOXInternational::~SwTOXInternational()
 {
    delete pCharClass;
    delete pIndexWrapper;
 }

inline BOOL SwTOXInternational::IsNumeric( const String& rStr ) const
{
    return pCharClass->isNumeric( rStr );
}

sal_Int32 SwTOXInternational::Compare( const String& rTxt1, const String& rTxtReading1,
                                       const ::com::sun::star::lang::Locale& rLocale1,
                                       const String& rTxt2, const String& rTxtReading2,
                                       const ::com::sun::star::lang::Locale& rLocale2 ) const
{
    return pIndexWrapper->CompareIndexEntry( rTxt1, rTxtReading1, rLocale1,
                                             rTxt2, rTxtReading2, rLocale2 );
}



/*--------------------------------------------------------------------
     Beschreibung:	SortierElement fuer Verzeichniseintraege
 --------------------------------------------------------------------*/


SwTOXSortTabBase::SwTOXSortTabBase( TOXSortType nTyp, const SwCntntNode* pNd,
                                    const SwTxtTOXMark* pMark,
                                    const SwTOXInternational* pInter,
                                    const ::com::sun::star::lang::Locale* pLocale )
    : pTxtMark( pMark ), pTOXNd( 0 ), nPos( 0 ), nType( nTyp ),
    pTOXIntl( pInter ), bValidTxt( FALSE ), nCntPos( 0 )
{
    if ( pLocale )
        aLocale = *pLocale;

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
#ifdef DBG_UTIL
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
    String sMyTxt;
    String sMyTxtReading;

    GetTxt( sMyTxt, sMyTxtReading );

    rNd.Insert( sMyTxt, rInsPos );
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

            String sMyTxt;
            String sMyTxtReading;
            GetTxt( sMyTxt, sMyTxtReading );

            String sOtherTxt;
            String sOtherTxtReading;
            rCmp.GetTxt( sOtherTxt, sOtherTxtReading );

            bRet = ( ( pEnd && pEndCmp ) || ( !pEnd && !pEndCmp ) ) &&
                    pTOXIntl->IsEqual( sMyTxt, sMyTxtReading, GetLocale(),
                                       sOtherTxt, sOtherTxtReading, rCmp.GetLocale() );
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

                        String sMyTxt;
                        String sMyTxtReading;
                        GetTxt( sMyTxt, sMyTxtReading );

                        String sOtherTxt;
                        String sOtherTxtReading;
                        rCmp.GetTxt( sOtherTxt, sOtherTxtReading );

                        // beide Pointer vorhanden -> vergleiche Text
                        // beide Pointer nicht vorhanden -> vergleiche AlternativText
                        if( ( pEnd && pEndCmp ) || ( !pEnd && !pEndCmp ) )
                            pTOXIntl->IsEqual( sMyTxt, sMyTxtReading, GetLocale(),
                                               sOtherTxt, sOtherTxtReading, rCmp.GetLocale() );

                        if( pEnd && !pEndCmp )
                            return TRUE;
                    }
                }
            }
            else if( pFirst && pFirst->IsTxtNode() &&
                     pNext && pNext->IsTxtNode() )
                    return ::binfilter::IsFrameBehind( *(SwTxtNode*)pNext, nCntPos,
                                            *(SwTxtNode*)pFirst, nCntPos );
        }
    }
    return FALSE;
}

/*--------------------------------------------------------------------
     Beschreibung: sortierter Stichworteintrag
 --------------------------------------------------------------------*/



//
// Stichworte vergleichen. Bezieht sich nur auf den Text
//



//
// kleiner haengt nur vom Text ab


//


//
// Das Stichwort selbst


//






/*--------------------------------------------------------------------
     Beschreibung: Schluessel und Trennzeichen
 --------------------------------------------------------------------*/












/*--------------------------------------------------------------------
     Beschreibung: sortierter Inhaltsverz. Eintrag
 --------------------------------------------------------------------*/




//	Der Text des Inhalts
//



//
// Die Ebene fuer Anzeige
//



/*--------------------------------------------------------------------
     Beschreibung: Verzeichnis aus Absaetzen zusammengesammelt
 --------------------------------------------------------------------*/

// bei Sortierung von OLE/Grafiken aufpassen !!!
// Die Position darf nicht die im Dokument,
// sondern muss die vom "Henkel" sein  !!











/*--------------------------------------------------------------------
    Beschreibung: Tabelle
 --------------------------------------------------------------------*/







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

USHORT SwTOXAuthority::GetLevel() const
{
    String sText(((SwAuthorityField*)m_rField.GetFld())->
                        GetFieldText(AUTH_FIELD_AUTHORITY_TYPE));
    USHORT nRet = 0;
    if( pTOXIntl->IsNumeric( sText ) )
    {
        nRet = (USHORT)sText.ToInt32();
        nRet++;
    }
    if(nRet >= AUTH_TYPE_END)
        nRet = 0;
    return nRet;
 }
/*-- 15.09.99 14:28:08---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwTOXAuthority::_GetText( String& rTxt, String& rTxtReading )
{
    //
    rTxt = m_rField.GetFld()->Expand();
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
        const SwAuthorityFieldType* pType = (const SwAuthorityFieldType*)pField->GetTyp();
        sal_Unicode cChar = pType->GetPrefix();
        if(cChar && cChar != ' ')
        sText.Erase(0, 1);
        cChar = pType->GetSuffix();
        if(cChar && cChar != ' ')
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
            String sMyTxt = pField->GetFieldText(pKey->eField);
            String sMyTxtReading;
            String sOtherTxt = pCmpField->GetFieldText(pKey->eField);
            String sOtherTxtReading;

            sal_Int32 nComp = pTOXIntl->Compare( sMyTxt, sMyTxtReading, GetLocale(),
                                                 sOtherTxt, sOtherTxtReading, rBase.GetLocale() );

            if( nComp )
            {
                bRet = (-1 == nComp) == pKey->bSortAscending;
                break;
            }
        }
    }
    return bRet;
}

}
