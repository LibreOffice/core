/*************************************************************************
 *
 *  $RCSfile: expfld.cxx,v $
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

#include <float.h>

#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _TOOLS_RESID_HXX //autogen
#include <tools/resid.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif


#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _SVX_PAGEITEM_HXX //autogen
#include <svx/pageitem.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen wg. SvxLanguageItem
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen wg. SvxFontItem
#include <svx/fontitem.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _LAYFRM_HXX
#include <layfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>
#endif
#ifndef _CALC_HXX
#include <calc.hxx>         // SwGetExpField
#endif
#ifndef _PAM_HXX
#include <pam.hxx>          // SwGet-/SetExpField
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>       // SwGet-/SetExpField
#endif
#ifndef _SWCACHE_HXX
#include <swcache.hxx>      // SwGet-/SetExpField
#endif
#ifndef _COM_SUN_STAR_TEXT_SETVARIABLETYPE_HPP_
#include <com/sun/star/text/SetVariableType.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::rtl;

SV_IMPL_PTRARR( _SwSeqFldList, _SeqFldLstElem* )

//-----------------------------------------------------------------------------
sal_Int16 lcl_SubTypeToAPI(USHORT nSubType)
{
        sal_Int16 nRet;
        switch(nSubType)
        {
            case GSE_EXPR   :   nRet = SetVariableType::VAR /*0*/; break;
            case GSE_SEQ    :   nRet = SetVariableType::SEQUENCE /*1*/; break;
            case GSE_FORMULA:   nRet = SetVariableType::FORMULA /*2*/; break;
            case GSE_STRING :   nRet = SetVariableType::STRING /*3*/; break;
        }
        return nRet;
}
//-----------------------------------------------------------------------------
sal_Int32 lcl_APIToSubType(const uno::Any& rAny)
{
        sal_Int16 nVal;
        rAny >>= nVal;
        sal_Int32 nSet = 0;
        switch(nVal)
        {
            case SetVariableType::VAR:      nSet = GSE_EXPR;  break;
            case SetVariableType::SEQUENCE: nSet = GSE_SEQ;  break;
            case SetVariableType::FORMULA:  nSet = GSE_FORMULA; break;
            case SetVariableType::STRING:   nSet = GSE_STRING;  break;
            default:
                DBG_ERROR("wrong value")
                nSet = -1;
        }
        return nSet;
}
//-----------------------------------------------------------------------------

void ReplacePoint( String& rTmpName )
{
    // Ersten und letzten Punkt ersetzen, da in Tabellennamen Punkte erlaubt sind
    xub_StrLen nLen = rTmpName.Len();
    sal_Unicode *pStr = rTmpName.GetBufferAccess(), *pBackStr = pStr + nLen;

    for( xub_StrLen i = nLen; i; --i, pBackStr-- )
        if( '.' == *pBackStr )
        {
            *pBackStr = DB_DELIM;
            break;
        }
    for( i = 0; i < nLen; ++i, ++pStr )
        if( '.' == *pStr )
        {
            *pStr = DB_DELIM;
            break;
        }
}

SwTxtNode* GetFirstTxtNode( const SwDoc& rDoc, SwPosition& rPos,
                            const SwCntntFrm *pCFrm, Point &rPt )
{
    SwTxtNode* pTxtNode;
    if ( !pCFrm )
    {
        rPos.nNode = *rDoc.GetNodes().GetEndOfContent().StartOfSectionNode();
        SwCntntNode* pCNd;
        while( 0 != (pCNd = rDoc.GetNodes().GoNext( &rPos.nNode ) ) &&
                0 == ( pTxtNode = pCNd->GetTxtNode() ) )
                        ;
        ASSERT( pTxtNode, "wo ist der 1.TextNode" );
        rPos.nContent.Assign( pTxtNode, 0 );
    }
    else if ( !pCFrm->IsValid() )
    {
        pTxtNode = (SwTxtNode*)pCFrm->GetNode();
        rPos.nNode = *pTxtNode;
        rPos.nContent.Assign( pTxtNode, 0 );
    }
    else
    {
        pCFrm->GetCrsrOfst( &rPos, rPt );
        pTxtNode = rPos.nNode.GetNode().GetTxtNode();
    }
    return pTxtNode;
}

const SwTxtNode* GetBodyTxtNode( const SwDoc& rDoc, SwPosition& rPos,
                                const SwFrm& rFrm )
{
    const SwLayoutFrm* pLayout = (SwLayoutFrm*)rFrm.GetUpper();
    const SwTxtNode* pTxtNode = 0;

    while( pLayout )
    {
        if( pLayout->IsFlyFrm() )
        {
            // hole das FlyFormat
            SwFrmFmt* pFlyFmt = ((SwFlyFrm*)pLayout)->GetFmt();
            ASSERT( pFlyFmt, "kein FlyFormat gefunden, wo steht das Feld" );

            const SwFmtAnchor &rAnchor = pFlyFmt->GetAnchor();

            if( FLY_AT_FLY == rAnchor.GetAnchorId() )
            {
                // und der Fly muss irgendwo angehaengt sein, also
                // den befragen
                pLayout = (SwLayoutFrm*)((SwFlyFrm*)pLayout)->GetAnchor();
                continue;
            }
            else if( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
                     FLY_AUTO_CNTNT == rAnchor.GetAnchorId() ||
                     FLY_IN_CNTNT == rAnchor.GetAnchorId() )
            {
                ASSERT( rAnchor.GetCntntAnchor(), "keine gueltige Position" );
                rPos = *rAnchor.GetCntntAnchor();
                pTxtNode = rPos.nNode.GetNode().GetTxtNode();
                if( FLY_AT_CNTNT == rAnchor.GetAnchorId() )
                    ((SwTxtNode*)pTxtNode)->MakeStartIndex( &rPos.nContent );
// oder doch besser das Ende vom (Anker-)TextNode nehmen ??
//                  ((SwTxtNode*)pTxtNode)->MakeEndIndex( &rPos.nContent );

                // noch nicht abbrechen, kann ja auch noch im
                // Header/Footer/Footnote/Fly stehen !!
                pLayout = ((SwFlyFrm*)pLayout)->GetAnchor()
                            ? ((SwFlyFrm*)pLayout)->GetAnchor()->GetUpper() : 0;
                continue;
            }
            else
            {
                pLayout->FindPageFrm()->GetCntntPosition(
                                                pLayout->Frm().Pos(), rPos );
                pTxtNode = rPos.nNode.GetNode().GetTxtNode();
            }
        }
        else if( pLayout->IsFtnFrm() )
        {
            // hole den Node vom Anker
            const SwTxtFtn* pFtn = ((SwFtnFrm*)pLayout)->GetAttr();
            pTxtNode = &pFtn->GetTxtNode();
            rPos.nNode = *pTxtNode;
            rPos.nContent = *pFtn->GetStart();
        }
        else if( pLayout->IsHeaderFrm() || pLayout->IsFooterFrm() )
        {
            const SwCntntFrm* pCntFrm;
            const SwPageFrm* pPgFrm = pLayout->FindPageFrm();
            if( pLayout->IsHeaderFrm() )
                pCntFrm = pPgFrm->FindFirstBodyCntnt();
            else
                pCntFrm = pPgFrm->FindLastBodyCntnt();

            if( pCntFrm )
            {
                pTxtNode = pCntFrm->GetNode()->GetTxtNode();
                rPos.nNode = *pTxtNode;
                ((SwTxtNode*)pTxtNode)->MakeEndIndex( &rPos.nContent );
            }
            else
            {
                Point aPt( pLayout->Frm().Pos() );
                aPt.Y()++;      // aus dem Header raus
                pCntFrm = pPgFrm->GetCntntPos( aPt, FALSE, TRUE, FALSE );
                pTxtNode = GetFirstTxtNode( rDoc, rPos, pCntFrm, aPt );
            }
        }
        else
        {
            pLayout = pLayout->GetUpper();
            continue;
        }
        break;      // gefunden und beende die Schleife
    }
    return pTxtNode;
}

/*--------------------------------------------------------------------
    Beschreibung: SwSetExpFieldType by JP
 --------------------------------------------------------------------*/

SwGetExpFieldType::SwGetExpFieldType(SwDoc* pDoc)
    : SwValueFieldType( pDoc, RES_GETEXPFLD )
{
}

SwFieldType* SwGetExpFieldType::Copy() const
{
    return new SwGetExpFieldType(GetDoc());
}

void SwGetExpFieldType::Modify( SfxPoolItem*, SfxPoolItem* pNew )
{
    if( pNew && RES_DOCPOS_UPDATE == pNew->Which() )
        SwModify::Modify( 0, pNew );
    // sonst nichts weiter expandieren
}

/*--------------------------------------------------------------------
    Beschreibung: SwGetExpField by JP
 --------------------------------------------------------------------*/

SwGetExpField::SwGetExpField(SwGetExpFieldType* pTyp, const String& rFormel,
                            USHORT nSub, ULONG nFmt)
    : SwFormulaField( pTyp, nFmt, 0.0 ),
    nSubType(nSub),
    bIsInBodyTxt( TRUE )
{
    SetFormula( rFormel );
}

String SwGetExpField::Expand() const
{
    if(nSubType & SUB_CMD)
        return GetFormula();
    else
        return sExpand;
}

String SwGetExpField::GetCntnt(BOOL bName) const
{
    if ( bName )
    {
        String aStr( SwFieldType::GetTypeStr( GSE_FORMULA & nSubType
                                                ? TYP_FORMELFLD
                                                : TYP_GETFLD ) );
        aStr += ' ';
        aStr += GetFormula();
        return aStr;
    }
    return Expand();
}

SwField* SwGetExpField::Copy() const
{
    SwGetExpField *pTmp = new SwGetExpField((SwGetExpFieldType*)GetTyp(),
                                            GetFormula(), nSubType, GetFormat());
    pTmp->SetLanguage(GetLanguage());
    pTmp->SwValueField::SetValue(GetValue());
    pTmp->sExpand       = sExpand;
    pTmp->bIsInBodyTxt  = bIsInBodyTxt;

    return pTmp;
}

void SwGetExpField::ChangeExpansion( const SwFrm& rFrm, const SwTxtFld& rFld )
{
    if( bIsInBodyTxt )      // nur Felder in Footer, Header, FootNote, Flys
        return;

    ASSERT( !rFrm.IsInDocBody(), "Flag ist nicht richtig, Frame steht im DocBody" );

    // bestimme mal das Dokument (oder geht es noch einfacher?)
    const SwTxtNode* pTxtNode = &rFld.GetTxtNode();
    SwDoc& rDoc = *(SwDoc*)pTxtNode->GetDoc();

    // einen Index fuers bestimmen vom TextNode anlegen
    SwPosition aPos( SwNodeIndex( rDoc.GetNodes() ) );
    pTxtNode = GetBodyTxtNode( rDoc, aPos, rFrm );

    // Wenn kein Layout vorhanden, kommt es in Kopf und Fusszeilen dazu
    // das ChnageExpansion uebers Layout-Formatieren aufgerufen wird
    // aber kein TxtNode vorhanden ist
    //
    if(!pTxtNode)
        return;

    _SetGetExpFld aEndFld( aPos.nNode, &rFld, &aPos.nContent );
    if(GetSubType() & GSE_STRING)
    {
        SwHash** ppHashTbl;
        USHORT nSize;
        rDoc.FldsToExpand( ppHashTbl, nSize, aEndFld );
        LookString( ppHashTbl, nSize, GetFormula(), sExpand );
        ::DeleteHashTable( ppHashTbl, nSize );      // HashTabelle loeschen
    }
    else
    {
        // fuelle den Calculator mit den Werten
        SwCalc aCalc( rDoc );
        rDoc.FldsToCalc( aCalc, aEndFld );

        // Wert berechnen
        SetValue(aCalc.Calculate(GetFormula()).GetDouble());

        // Auswertung nach Format
        sExpand = ((SwValueFieldType*)GetTyp())->ExpandValue(
                                GetValue(), GetFormat(), GetLanguage());
    }
}

String SwGetExpField::GetPar2() const
{
    return GetFormula();
}

void SwGetExpField::SetPar2(const String& rStr)
{
    SetFormula(rStr);
}

USHORT SwGetExpField::GetSubType() const
{
    return nSubType;
}

void SwGetExpField::SetSubType(USHORT nType)
{
    nSubType = nType;
}

void SwGetExpField::SetLanguage(USHORT nLng)
{
    if (nSubType & SUB_CMD)
        SwField::SetLanguage(nLng);
    else
        SwValueField::SetLanguage(nLng);
}

/*-----------------07.03.98 16:08-------------------

--------------------------------------------------*/
BOOL SwGetExpField::QueryValue( uno::Any& rAny, const String& rProperty ) const
{
    if( rProperty.EqualsAscii( UNO_NAME_VALUE ))
        rAny <<= GetValue();
    else if( rProperty.EqualsAscii( UNO_NAME_NUMBER_FORMAT ))
        rAny <<= (sal_Int32)GetFormat();
    else if( rProperty.EqualsAscii( UNO_NAME_VARIABLE_SUBTYPE ))
         rAny <<= (sal_Int16)nSubType;
    else if( rProperty.EqualsAscii( UNO_NAME_CONTENT ))
         rAny <<= OUString( GetFormula() );
    else if( rProperty.EqualsAscii( UNO_NAME_SUB_TYPE ))
    {
        sal_Int16 nRet = lcl_SubTypeToAPI(GetSubType() & 0xff);
        rAny <<= nRet;
    }
    else if(rProperty.EqualsAscii(UNO_NAME_IS_SHOW_FORMULA))
    {
        BOOL bTmp = 0 != (nSubType & SUB_CMD);
        rAny.setValue(&bTmp, ::getBooleanCppuType());
    }
    else if(rProperty.EqualsAscii(UNO_NAME_CURRENT_PRESENTATION))
        rAny <<= rtl::OUString(GetExpStr());
#ifdef DBG_UTIL
    else
        DBG_ERROR("Welches Property?")
#endif
    return TRUE;
}
/*-----------------07.03.98 16:08-------------------

--------------------------------------------------*/
BOOL SwGetExpField::PutValue( const uno::Any& rAny, const String& rProperty )
{
    if( rProperty.EqualsAscii( UNO_NAME_VALUE ))
    {
        SetValue(*(double*) rAny.getValue());
    }
    else if( rProperty.EqualsAscii( UNO_NAME_NUMBER_FORMAT ))
    {
        sal_Int32 nTmp;
        rAny >>= nTmp;
        SetFormat(nTmp);
    }
    else if( rProperty.EqualsAscii( UNO_NAME_VARIABLE_SUBTYPE ))
    {
         sal_Int32 nTmp;
         rAny >>= nTmp;
         nSubType = nTmp;
    }
    else if( rProperty.EqualsAscii( UNO_NAME_CONTENT ))
    {
        OUString uTmp;
        rAny >>= uTmp;
        SetFormula( uTmp );
    }
    else if( rProperty.EqualsAscii( UNO_NAME_SUB_TYPE ))
    {
        sal_Int32 nSet = lcl_APIToSubType(rAny);
        if(nSet >=0 )
            SetSubType((GetSubType() & 0xff00) | nSet);
    }
    else if(rProperty.EqualsAscii(UNO_NAME_IS_SHOW_FORMULA))
    {
        sal_Bool bTmp = *(sal_Bool*) rAny.getValue();
        if(bTmp)
            nSubType |= SUB_CMD;
        else
            nSubType &= (~SUB_CMD);
    }
    else if(rProperty.EqualsAscii(UNO_NAME_CURRENT_PRESENTATION))
    {
        OUString sVal;
        rAny >>= sVal;
        ChgExpStr(sVal);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Welches Property?")
#endif
    return TRUE;
}

/*-----------------JP: 17.06.93 -------------------
 Set-Expression-Type
 --------------------------------------------------*/

SwSetExpFieldType::SwSetExpFieldType( SwDoc* pDoc, const String& rName, USHORT nTyp )
    : SwValueFieldType( pDoc, RES_SETEXPFLD ),
    sName( rName ),
    nType(nTyp),
    cDelim( '.' ), nLevel( UCHAR_MAX ),
    bDeleted( FALSE ),
    pOutlChgNd( 0 )
{
    if( ( GSE_SEQ | GSE_STRING ) & nType )
        EnableFormat(FALSE);    // Numberformatter nicht einsetzen
}

SwFieldType* SwSetExpFieldType::Copy() const
{
    SwSetExpFieldType* pNew = new SwSetExpFieldType(GetDoc(), sName, nType);
    pNew->bDeleted = bDeleted;
    pNew->cDelim = cDelim;
    pNew->nLevel = nLevel;

    return pNew;
}

const String& SwSetExpFieldType::GetName() const
{
    return sName;
}

void SwSetExpFieldType::Modify( SfxPoolItem*, SfxPoolItem* )
{
    return;     // nicht weiter expandieren
}

void SwSetExpFieldType::SetSeqFormat(ULONG nFmt)
{
    SwClientIter aIter(*this);
    for( SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE(SwFmtFld) );
            pFld; pFld = (SwFmtFld*)aIter.Next() )
        pFld->GetFld()->ChangeFormat( nFmt );
}

ULONG SwSetExpFieldType::GetSeqFormat()
{
    if( !GetDepends() )
        return SVX_NUM_ARABIC;

    SwField *pFld = ((SwFmtFld*)GetDepends())->GetFld();
    return pFld->GetFormat();
}

USHORT SwSetExpFieldType::SetSeqRefNo( SwSetExpField& rFld )
{
    if( !GetDepends() || !(GSE_SEQ & nType) )
        return USHRT_MAX;

extern void InsertSort( SvUShorts& rArr, USHORT nIdx, USHORT* pInsPos = 0 );
    SvUShorts aArr( 64 );

    USHORT n;

    // dann testmal, ob die Nummer schon vergeben ist oder ob eine neue
    // bestimmt werden muss.
    SwClientIter aIter( *this );
    const SwTxtNode* pNd;
    for( SwFmtFld* pF = (SwFmtFld*)aIter.First( TYPE( SwFmtFld )); pF;
            pF = (SwFmtFld*)aIter.Next() )
        if( pF->GetFld() != &rFld && pF->GetTxtFld() &&
            0 != ( pNd = pF->GetTxtFld()->GetpTxtNode() ) &&
            pNd->GetNodes().IsDocNodes() )
            InsertSort( aArr, ((SwSetExpField*)pF->GetFld())->GetSeqNumber() );


    // teste erstmal ob die Nummer schon vorhanden ist:
    USHORT nNum = rFld.GetSeqNumber();
    if( USHRT_MAX != nNum )
    {
        for( n = 0; n < aArr.Count(); ++n )
            if( aArr[ n ] > nNum )
                return nNum;            // nicht vorhanden -> also benutzen
            else if( aArr[ n ] == nNum )
                break;                  // schon vorhanden -> neue erzeugen

        if( n == aArr.Count() )
            return nNum;            // nicht vorhanden -> also benutzen
    }

    // alle Nummern entsprechend geflag, also bestimme die richtige Nummer
    for( n = 0; n < aArr.Count(); ++n )
        if( n != aArr[ n ] )
            break;

    rFld.SetSeqNumber( n );
    return n;
}

USHORT SwSetExpFieldType::GetSeqFldList( SwSeqFldList& rList )
{
    if( rList.Count() )
        rList.Remove( 0, rList.Count() );

    SwClientIter aIter( *this );
    const SwTxtNode* pNd;
    for( SwFmtFld* pF = (SwFmtFld*)aIter.First( TYPE( SwFmtFld )); pF;
            pF = (SwFmtFld*)aIter.Next() )
        if( pF->GetTxtFld() &&
            0 != ( pNd = pF->GetTxtFld()->GetpTxtNode() ) &&
            pNd->GetNodes().IsDocNodes() )
        {
            _SeqFldLstElem* pNew = new _SeqFldLstElem( pNd->GetExpandTxt( 0,
                                (*pF->GetTxtFld()->GetStart()) + 1, FALSE ),
                            ((SwSetExpField*)pF->GetFld())->GetSeqNumber() );
            rList.InsertSort( pNew );
        }

    return rList.Count();
}


void SwSetExpFieldType::SetChapter( SwSetExpField& rFld, const SwNode& rNd )
{
    const SwTxtNode* pTxtNd = rNd.FindOutlineNodeOfLevel( nLevel );
    if( pTxtNd )
    {
        ASSERT( pTxtNd && pTxtNd->GetOutlineNum(),
                "kein Outline TextNode" );

        SwNodeNum aNum( *pTxtNd->GetOutlineNum() );
        if( nLevel < aNum.GetLevel() )
            aNum.SetLevel( nLevel );

        // nur die Nummer besorgen, ohne Pre-/Post-fixstrings
        String sNumber( GetDoc()->GetOutlineNumRule()->MakeNumString(
                                                aNum, FALSE ));
        if( sNumber.Len() )
            rFld.ChgExpStr(  ( sNumber += cDelim ) += rFld.GetExpStr() );
    }
}

/* -----------------24.03.99 09:44-------------------
 *
 * --------------------------------------------------*/
BOOL SwSetExpFieldType::QueryValue( uno::Any& rAny, const String& rProperty ) const
{
    if( rProperty.EqualsAscii( UNO_NAME_SUB_TYPE ))
    {
        sal_Int16 nRet = lcl_SubTypeToAPI(GetType());
        rAny <<= nRet;
    }
    else if(rProperty.EqualsAscii( UNO_NAME_NUMBERING_SEPARATOR ))
    {
        OUString sRet(GetDelimiter());
        rAny <<= sRet;
    }
    else if(rProperty.EqualsAscii( UNO_NAME_CHAPTER_NUMBERING_LEVEL ))
    {
        sal_Int8 nRet = nLevel < MAXLEVEL? nLevel : -1;
        rAny <<= nRet;
    }
    return TRUE;
}

BOOL SwSetExpFieldType::PutValue( const uno::Any& rAny, const String& rProperty )
{
    BOOL bRet = FALSE;
    if( rProperty.EqualsAscii(UNO_NAME_SUB_TYPE ) )
    {
        sal_Int32 nSet = lcl_APIToSubType(rAny);
        if(nSet >=0)
            SetType(nSet);
    }
    else if(rProperty.EqualsAscii( UNO_NAME_NUMBERING_SEPARATOR ))
    {
        OUString sDelim;
        rAny >>= sDelim;
        if(sDelim.getLength())
            SetDelimiter(sDelim.getStr()[0]);
        else
            SetDelimiter(' ');
    }
    else if(rProperty.EqualsAscii( UNO_NAME_CHAPTER_NUMBERING_LEVEL ))
    {
        sal_Int8 nLvl;
        rAny >>= nLvl;
        if(nLvl < 0 || nLvl >= MAXLEVEL)
            SetOutlineLvl(UCHAR_MAX);
        else
            SetOutlineLvl(nLvl);
    }
    return bRet;
}

BOOL SwSeqFldList::InsertSort( _SeqFldLstElem* pNew )
{
    sal_Unicode* p = pNew->sDlgEntry.GetBufferAccess();
    while( *p )
    {
        if( *p < 0x20 )
            *p = 0x20;
        ++p;
    }

    USHORT nPos;
    BOOL bRet = SeekEntry( *pNew, &nPos );
    if( !bRet )
        C40_INSERT( _SeqFldLstElem, pNew, nPos );
    return bRet;
}

BOOL SwSeqFldList::SeekEntry( const _SeqFldLstElem& rNew, USHORT* pP )
{
    register USHORT nO = Count(), nM, nU = 0;
    const International& rInt = Application::GetAppInternational();
    const CharClass& rCC = GetAppCharClass();
    if( nO > 0 )
    {
        //#59900# Die Sortierung soll die Nummer korrekt einordnen
        //also "10" nach "9" und nicht "10" nach "1"
        const String& rTmp2 = rNew.sDlgEntry;
        xub_StrLen nFndPos2 = 0;
        String sNum2( rTmp2.GetToken( 0, ' ', nFndPos2 ));
        BOOL bIsNum2IsNumeric = rCC.isNumeric( sNum2 );
        sal_Int32 nNum2 = bIsNum2IsNumeric ? sNum2.ToInt32() : 0;

        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;

            //#59900# Die Sortierung soll die Nummer korrekt einordnen
            //also "10" nach "9" und nicht "10" nach "1"
            const String& rTmp1 = (*((_SeqFldLstElem**)pData + nM))->sDlgEntry;
            xub_StrLen nFndPos1 = 0;
            String sNum1( rTmp1.GetToken( 0, ' ', nFndPos1 ));
            StringCompare eCmp;

            if( bIsNum2IsNumeric && rCC.isNumeric( sNum1 ) )
            {
                sal_Int32 nNum1 = sNum1.ToInt32();
                eCmp = nNum2 == nNum1 ? COMPARE_EQUAL
                                      : nNum2 >= nNum1
                                            ? COMPARE_GREATER
                                            : COMPARE_LESS;
                if( COMPARE_EQUAL == eCmp )
                    eCmp = rInt.Compare( rTmp2.Copy( nFndPos2 ),
                                         rTmp1.Copy( nFndPos1 ));
            }
            else
                eCmp = rInt.Compare( rTmp2, rTmp1, INTN_COMPARE_IGNORECASE );

            if( COMPARE_EQUAL == eCmp )
            {
                if( pP ) *pP = nM;
                return TRUE;
            }
            else if( COMPARE_GREATER == eCmp )
                nU = nM + 1;
            else if( nM == 0 )
                break;
            else
                nO = nM - 1;
        }
    }
    if( pP ) *pP = nU;
    return FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung: SwSetExpField by JP
 --------------------------------------------------------------------*/

SwSetExpField::SwSetExpField(SwSetExpFieldType* pTyp, const String& rFormel,
                                        ULONG nFmt)
    : SwFormulaField( pTyp, nFmt, 0.0 ), nSeqNo( USHRT_MAX ),
    nSubType(0)
{
    SetFormula(rFormel);
    // SubType ignorieren !!!
    bInput = FALSE;
    if( IsSequenceFld() )
    {
        SwValueField::SetValue(1.0);
        if( !rFormel.Len() )
        {
            String sFormel(rFormel);
            sFormel += pTyp->GetName();
            sFormel += '+';
            sFormel += '1';
            SetFormula(sFormel);
        }
    }
}

String SwSetExpField::Expand() const
{
    String aStr;
    if (nSubType & SUB_CMD)
    {   // Der CommandString ist gefragt
        aStr = GetTyp()->GetName();
        aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " = " ));
        aStr += GetFormula();
    }
    else if(!(nSubType & SUB_INVISIBLE))
    {   // Der Wert ist sichtbar
        aStr = sExpand;
    }
    return aStr;
}

/*--------------------------------------------------------------------
    Beschreibung: liefert den Namen oder den Inhalt
 --------------------------------------------------------------------*/

String SwSetExpField::GetCntnt(BOOL bName) const
{
    if( bName )
    {
        USHORT nStrType;

        if( IsSequenceFld() )
            nStrType = TYP_SEQFLD;
        else if( bInput )
            nStrType = TYP_SETINPFLD;
        else
            nStrType = TYP_SETFLD;

        String aStr( SwFieldType::GetTypeStr( nStrType ) );
        aStr += ' ';
        aStr += GetTyp()->GetName();

        if( TYP_SEQFLD != nStrType )
        {
            // Sequence nicht die Formel ausgeben
            aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " = " ));
            aStr += GetFormula();
        }
        return aStr;
    }
    return Expand();
}

SwField* SwSetExpField::Copy() const
{
    SwSetExpField *pTmp = new SwSetExpField((SwSetExpFieldType*)GetTyp(),
                                            GetFormula(), GetFormat());
    pTmp->SwValueField::SetValue(GetValue());
    pTmp->sExpand       = sExpand;
    pTmp->SetLanguage(GetLanguage());
    pTmp->aPText        = aPText;
    pTmp->bInput        = bInput;
    pTmp->nSeqNo        = nSeqNo;
    pTmp->SetSubType(GetSubType());

    return pTmp;
}

void SwSetExpField::SetSubType(USHORT nSub)
{
    ((SwSetExpFieldType*)GetTyp())->SetType(nSub & 0xff);
    nSubType = nSub & 0xff00;

    DBG_ASSERT( (nSub & 0xff) != 3, "SubType ist illegal!" );
}

USHORT SwSetExpField::GetSubType() const
{
    return ((SwSetExpFieldType*)GetTyp())->GetType() | nSubType;
}

void SwSetExpField::SetValue( const double& rAny )
{
    SwValueField::SetValue(rAny);

    if( IsSequenceFld() )
        sExpand = FormatNumber( (USHORT)GetValue(), GetFormat() );
    else
        sExpand = ((SwValueFieldType*)GetTyp())->ExpandValue( rAny,
                                                GetFormat(), GetLanguage());
}

void SwGetExpField::SetValue( const double& rAny )
{
    SwValueField::SetValue(rAny);
    sExpand = ((SwValueFieldType*)GetTyp())->ExpandValue( rAny, GetFormat(),
                                                            GetLanguage());
}
/* -----------------14.07.99 12:21-------------------
    Description: Find the index of the reference text
    following the current field
 --------------------------------------------------*/
xub_StrLen SwGetExpField::GetReferenceTextPos( const SwFmtFld& rFmt, SwDoc& rDoc)
{
    //
    const SwTxtFld* pTxtFld = rFmt.GetTxtFld();
    const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
    //
    xub_StrLen nRet = *pTxtFld->GetStart() + 1;
    String sNodeText = rTxtNode.GetTxt();
    sNodeText.Erase(0, nRet);
    if(sNodeText.Len())
    {
        //now check if sNodeText starts with a non-alphanumeric character plus a blank
        static USHORT nIds[] =
        {
            RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
            RES_CHRATR_FONT, RES_CHRATR_FONT,
            0, 0
        };
        SwAttrSet aSet(rDoc.GetAttrPool(), nIds);
        rTxtNode.GetAttr(aSet, nRet, nRet+1);

        LanguageType eLang = aSet.GetLanguage().GetLanguage();
        if( RTL_TEXTENCODING_SYMBOL != aSet.GetFont().GetCharSet())
        {
            International aInt( eLang );
            sal_Unicode c0 = sNodeText.GetChar(0);
            BOOL bIsAlphaNum = aInt.IsAlphaNumeric( c0 );
            if(!bIsAlphaNum ||
                (c0 == ' ' || c0 == '\t'))
            {
                nRet++;
                if( sNodeText.Len() > 1 &&
                    (sNodeText.GetChar(1) == ' ' ||
                     sNodeText.GetChar(1) == '\t'))
                    nRet++;
            }
        }
    }
    return nRet;
}


/*--------------------------------------------------------------------
    Beschreibung: Parameter setzen
 --------------------------------------------------------------------*/

const String& SwSetExpField::GetPar1() const
{
    return ((SwSetExpFieldType*)GetTyp())->GetName();
}

String SwSetExpField::GetPar2() const
{
    USHORT nType = ((SwSetExpFieldType*)GetTyp())->GetType();

    if (nType & GSE_STRING)
        return GetFormula();
    return GetExpandedFormula();
}

void SwSetExpField::SetPar2(const String& rStr)
{
    USHORT nType = ((SwSetExpFieldType*)GetTyp())->GetType();

    if( !(nType & GSE_SEQ) || rStr.Len() )
    {
        if (nType & GSE_STRING)
            SetFormula(rStr);
        else
            SetExpandedFormula(rStr);
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Eingabefeld Type
 ---------------------------------------------------------------------*/

SwInputFieldType::SwInputFieldType( SwDoc* pD )
    : SwFieldType( RES_INPUTFLD ), pDoc( pD )
{
}

SwFieldType* SwInputFieldType::Copy() const
{
    SwInputFieldType* pType = new SwInputFieldType( pDoc );
    return pType;
}

/*--------------------------------------------------------------------
    Beschreibung: Eingabefeld
 --------------------------------------------------------------------*/

SwInputField::SwInputField(SwInputFieldType* pType, const String& rContent,
                           const String& rPrompt, USHORT nSub, ULONG nFmt) :
    SwField(pType, nFmt), nSubType(nSub), aContent(rContent), aPText(rPrompt)
{
}

String SwInputField::GetCntnt(BOOL bName) const
{
    if ( bName )
    {
        String aStr(SwField::GetCntnt(bName));
        if ((nSubType & 0x00ff) == INP_USR)
        {
            aStr += GetTyp()->GetName();
            aStr += ' ';
            aStr += aContent;
        }
        return aStr;
    }
    return Expand();
}

SwField* SwInputField::Copy() const
{
    SwInputField* pFld = new SwInputField((SwInputFieldType*)GetTyp(), aContent,
                                          aPText, GetSubType(), GetFormat());
    return pFld;
}

String SwInputField::Expand() const
{
    String sRet;
    if((nSubType & 0x00ff) == INP_TXT)
        sRet = aContent;

    else if( (nSubType & 0x00ff) == INP_USR )
    {
        SwUserFieldType* pUserTyp = (SwUserFieldType*)
                            ((SwInputFieldType*)GetTyp())->GetDoc()->
                            GetFldType( RES_USERFLD, aContent );
        if( pUserTyp )
            sRet = pUserTyp->GetContent();
    }
    return sRet;
}

/*-----------------06.03.98 11:12-------------------

--------------------------------------------------*/
BOOL SwInputField::QueryValue( uno::Any& rAny, const String& rProperty ) const
{
    if( rProperty.EqualsAscii( UNO_NAME_HINT ))
        rAny <<= OUString( aPText );
    else if( rProperty.EqualsAscii( UNO_NAME_CONTENT ))
         rAny <<= OUString( aContent );
#ifdef DBG_UTIL
    else
        DBG_ERROR("Welches Property?")
#endif
    return TRUE;
}
/*-----------------06.03.98 11:12-------------------

--------------------------------------------------*/
BOOL SwInputField::PutValue( const uno::Any& rAny, const String& rProperty )
{
    if( rProperty.EqualsAscii( UNO_NAME_HINT ))
    {
        OUString uTmp;
        rAny >>= uTmp;
        aPText = String( uTmp );
    }
    else if( rProperty.EqualsAscii( UNO_NAME_CONTENT ))
    {
        OUString uTmp;
        rAny >>= uTmp;
        aContent = String( uTmp );
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Welches Property?")
#endif
    return TRUE;
}
/*--------------------------------------------------------------------
    Beschreibung: Bedingung setzen
 --------------------------------------------------------------------*/

void SwInputField::SetPar1(const String& rStr)
{
    aContent = rStr;
}

const String& SwInputField::GetPar1() const
{
    return aContent;
}

/*--------------------------------------------------------------------
    Beschreibung: True/False Text
 --------------------------------------------------------------------*/

void SwInputField::SetPar2(const String& rStr)
{
    aPText = rStr;
}

String SwInputField::GetPar2() const
{
    return aPText;
}

USHORT SwInputField::GetSubType() const
{
    return nSubType;
}

void SwInputField::SetSubType(USHORT nSub)
{
    nSubType = nSub;
}
/*-----------------05.03.98 17:22-------------------

--------------------------------------------------*/
BOOL SwSetExpField::QueryValue( uno::Any& rAny, const String& rProperty ) const
{
    if( rProperty.EqualsAscii( UNO_NAME_IS_VISIBLE ))
    {
        sal_Bool bVal = 0 == (nSubType & SUB_INVISIBLE);
        rAny.setValue(&bVal, ::getBooleanCppuType());
    }
    else if( rProperty.EqualsAscii( UNO_NAME_NUMBER_FORMAT ))
        rAny <<= (sal_Int32)GetFormat();
    else if( rProperty.EqualsAscii( UNO_NAME_NUMBERING_TYPE ))
        rAny <<= (sal_Int16)GetFormat();
    else if( rProperty.EqualsAscii( UNO_NAME_SEQUENCE_VALUE ))
        rAny <<= (sal_Int16)nSeqNo;
    else if( rProperty.EqualsAscii( UNO_NAME_VARIABLE_NAME ))
        rAny <<= OUString( GetPar1() );
    else if( rProperty.EqualsAscii( UNO_NAME_CONTENT ))
         rAny <<= OUString( GetFormula() );
    else if( rProperty.EqualsAscii( UNO_NAME_VALUE ))
        rAny <<= (Double)GetValue();
    else if( rProperty.EqualsAscii( UNO_NAME_SUB_TYPE ))
    {
        sal_Int16 nRet = 0;
            nRet = lcl_SubTypeToAPI(GetSubType() & 0xff);
        rAny <<= nRet;
    }
    else if( rProperty.EqualsAscii( UNO_NAME_HINT ))
        rAny <<= OUString( aPText );
    else if(rProperty.EqualsAscii(UNO_NAME_IS_SHOW_FORMULA))
    {
        BOOL bTmp = 0 != (nSubType & SUB_CMD);
        rAny.setValue(&bTmp, ::getBooleanCppuType());
    }
    else if(rProperty.EqualsAscii(UNO_NAME_IS_INPUT))
    {
        BOOL bTmp = GetInputFlag();
        rAny.setValue(&bTmp, ::getBooleanCppuType());
    }
    else if(rProperty.EqualsAscii(UNO_NAME_CURRENT_PRESENTATION))
        rAny <<= rtl::OUString(GetExpStr());
#ifdef DBG_UTIL
    else
        DBG_ERROR("Welches Property?")
#endif
    return TRUE;
}
/*-----------------05.03.98 17:22-------------------

--------------------------------------------------*/
BOOL SwSetExpField::PutValue( const uno::Any& rAny, const String& rProperty )
{
    if(rProperty.EqualsAscii( UNO_NAME_IS_VISIBLE ))
    {
        sal_Bool bVal = *(sal_Bool*)rAny.getValue();
        if(bVal)
            nSubType &= ~SUB_INVISIBLE;
        else
            nSubType |= SUB_INVISIBLE;
    }
    else if( rProperty.EqualsAscii( UNO_NAME_NUMBER_FORMAT ))
    {
        sal_Int32 nTmp;
        rAny >>= nTmp;
        SetFormat(nTmp);
    }
    else if( rProperty.EqualsAscii( UNO_NAME_NUMBERING_TYPE ))
    {
        sal_Int16 nSet;
        rAny >>=nSet;
        if(nSet <= SVX_NUMBER_NONE )
            SetFormat(nSet);
        else
            //exception(wrong_value)
            ;
    }
    else if( rProperty.EqualsAscii( UNO_NAME_SEQUENCE_VALUE ))
    {
        sal_Int16 nSet;
        rAny >>=nSet;
        nSeqNo = nSet;
    }
    else if( rProperty.EqualsAscii( UNO_NAME_VARIABLE_NAME ))
    {
        OUString uTmp;
        rAny >>= uTmp;
        SetPar1( uTmp );
    }
    else if( rProperty.EqualsAscii( UNO_NAME_CONTENT ))
    {
        OUString uTmp;
        rAny >>= uTmp;
        SetFormula( uTmp );
    }
    else if( rProperty.EqualsAscii( UNO_NAME_VALUE ))
    {
         double fVal;
         rAny >>= fVal;
         SetValue(fVal);
    }
    else if( rProperty.EqualsAscii( UNO_NAME_SUB_TYPE ))
    {
        sal_Int32 nSet = lcl_APIToSubType(rAny);
        if(nSet >= 0)
            SetSubType((GetSubType() & 0xff00) | nSet);
    }
    else if( rProperty.EqualsAscii( UNO_NAME_HINT ))
    {
        OUString uTmp;
        rAny >>= uTmp;
        aPText = String( uTmp );
    }
    else if(rProperty.EqualsAscii(UNO_NAME_IS_SHOW_FORMULA))
    {
        sal_Bool bTmp = *(sal_Bool*) rAny.getValue();
        if(bTmp)
            nSubType |= SUB_CMD;
        else
            nSubType &= (~SUB_CMD);
    }
    else if(rProperty.EqualsAscii(UNO_NAME_IS_INPUT))
    {
        sal_Bool bTmp = *(sal_Bool*) rAny.getValue();
        SetInputFlag(bTmp);
    }
    else if(rProperty.EqualsAscii(UNO_NAME_CURRENT_PRESENTATION))
    {
        OUString sVal;
        rAny >>= sVal;
        ChgExpStr(sVal);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Welches Property?")
#endif
    return TRUE;
}



