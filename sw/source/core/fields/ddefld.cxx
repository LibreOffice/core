/*************************************************************************
 *
 *  $RCSfile: ddefld.cxx,v $
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

#ifndef _LINKNAME_HXX //autogen
#include <so3/linkname.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _SWBASLNK_HXX
#include <swbaslnk.hxx>
#endif
#ifndef _SWDDETBL_HXX
#include <swddetbl.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif

using namespace rtl;
/*--------------------------------------------------------------------
    Beschreibung: Globale Variablen
 --------------------------------------------------------------------*/

class SwIntrnlRefLink : public SwBaseLink
{
    SwDDEFieldType& rFldType;
public:
    SwIntrnlRefLink( SwDDEFieldType& rType, USHORT nUpdateType, USHORT nFmt )
        : SwBaseLink( nUpdateType, nFmt ),
        rFldType( rType )
    {}

    virtual void Closed();
    virtual void DataChanged( SvData& );

    virtual const SwNode* GetAnchor() const;
    virtual BOOL IsInRange( ULONG nSttNd, ULONG nEndNd, xub_StrLen nStt = 0,
                            xub_StrLen nEnd = STRING_NOTFOUND ) const;
};


void SwIntrnlRefLink::DataChanged( SvData& rData )
{
    switch( rData.GetFormat() )
    {
    case FORMAT_STRING:
        if( !IsNoDataFlag() )
        {
            String sStr;
            rData.GetData( sStr );
            // CR-LF am Ende entfernen, ist ueberfluessig!
            USHORT n = sStr.Len();
            if( n && 0x0a == sStr.GetChar( n-1 ) )
                --n;
            if( n && 0x0d == sStr.GetChar( n-1 ) )
                --n;

            BOOL bDel = n != sStr.Len();
            if( bDel )
                sStr.Erase( n );

            rFldType.SetExpansion( sStr );
            // erst Expansion setzen! (sonst wird das Flag geloescht!)
            rFldType.SetCRLFDelFlag( bDel );
        }
        break;

    // weitere Formate ...
    default:
        return;
    }

    ASSERT( rFldType.GetDoc(), "Kein pDoc" );

    // keine Abhaengigen mehr?
    if( rFldType.GetDepends() && !rFldType.IsModifyLocked() && !ChkNoDataFlag() )
    {
        ViewShell* pSh;
        SwEditShell* pESh = rFldType.GetDoc()->GetEditShell( &pSh );

        // dann suchen wir uns mal alle Felder. Wird kein gueltiges
        // gefunden, dann Disconnecten wir uns!
        SwMsgPoolItem aUpdateDDE( RES_UPDATEDDETBL );
        int bCallModify = FALSE;
        rFldType.LockModify();

        SwClientIter aIter( rFldType );
        SwClient * pLast = aIter.GoStart();
        if( pLast )     // konnte zum Anfang gesprungen werden ??
            do {
                // eine DDE-Tabelle oder ein DDE-FeldAttribut im Text
                if( !pLast->IsA( TYPE( SwFmtFld ) ) ||
                    ((SwFmtFld*)pLast)->GetTxtFld() )
                {
                    if( !bCallModify )
                    {
                        if( pESh )
                            pESh->StartAllAction();
                        else if( pSh )
                            pSh->StartAction();
                    }
                    pLast->Modify( 0, &aUpdateDDE );
                    bCallModify = TRUE;
                }
            } while( 0 != ( pLast = aIter++ ));

        rFldType.UnlockModify();

        if( bCallModify )
        {
            if( pESh )
                pESh->EndAllAction();
            else if( pSh )
                pSh->EndAction();

            if( pSh )
                pSh->GetDoc()->SetModified();
        }
    }
}

void SwIntrnlRefLink::Closed()
{
    if( rFldType.GetDoc() && !rFldType.GetDoc()->IsInDtor() )
    {
        // Advise verabschiedet sich, alle Felder in Text umwandeln ?
        ViewShell* pSh;
        SwEditShell* pESh = rFldType.GetDoc()->GetEditShell( &pSh );
        if( pESh )
        {
            pESh->StartAllAction();
            pESh->FieldToText( &rFldType );
            pESh->EndAllAction();
        }
        else
        {
            pSh->StartAction();
            // am Doc aufrufen ??
            pSh->EndAction();
        }
    }
    SvBaseLink::Closed();
}

const SwNode* SwIntrnlRefLink::GetAnchor() const
{
    // hier sollte irgend ein Anchor aus dem normalen Nodes-Array reichen
    const SwNode* pNd = 0;
    SwClientIter aIter( rFldType );
    SwClient * pLast = aIter.GoStart();
    if( pLast )     // konnte zum Anfang gesprungen werden ??
        do {
            // eine DDE-Tabelle oder ein DDE-FeldAttribut im Text
            if( !pLast->IsA( TYPE( SwFmtFld ) ))
            {
                SwDepend* pDep = (SwDepend*)pLast;
                SwDDETable* pDDETbl = (SwDDETable*)pDep->GetToTell();
                pNd = pDDETbl->GetTabSortBoxes()[0]->GetSttNd();
            }
            else if( ((SwFmtFld*)pLast)->GetTxtFld() )
                pNd = ((SwFmtFld*)pLast)->GetTxtFld()->GetpTxtNode();

            if( pNd && &rFldType.GetDoc()->GetNodes() == &pNd->GetNodes() )
                break;
            pNd = 0;
        } while( 0 != ( pLast = aIter++ ));

    return pNd;
}

BOOL SwIntrnlRefLink::IsInRange( ULONG nSttNd, ULONG nEndNd,
                                xub_StrLen nStt, xub_StrLen nEnd ) const
{
    // hier sollte irgend ein Anchor aus dem normalen Nodes-Array reichen
    SwNodes* pNds = &rFldType.GetDoc()->GetNodes();
    SwClientIter aIter( rFldType );
    SwClient * pLast = aIter.GoStart();
    if( pLast )     // konnte zum Anfang gesprungen werden ??
        do {
            // eine DDE-Tabelle oder ein DDE-FeldAttribut im Text
            if( !pLast->IsA( TYPE( SwFmtFld ) ))
            {
                SwDepend* pDep = (SwDepend*)pLast;
                SwDDETable* pDDETbl = (SwDDETable*)pDep->GetToTell();
                const SwTableNode* pTblNd = pDDETbl->GetTabSortBoxes()[0]->
                                GetSttNd()->FindTableNode();
                if( pTblNd->GetNodes().IsDocNodes() &&
                    nSttNd < pTblNd->EndOfSectionIndex() &&
                    nEndNd > pTblNd->GetIndex() )
                    return TRUE;
            }
            else if( ((SwFmtFld*)pLast)->GetTxtFld() )
            {
                const SwTxtFld* pTFld = ((SwFmtFld*)pLast)->GetTxtFld();
                const SwTxtNode* pNd = pTFld->GetpTxtNode();
                if( pNd && pNds == &pNd->GetNodes() )
                {
                    ULONG nNdPos = pNd->GetIndex();
                    if( nSttNd <= nNdPos && nNdPos <= nEndNd &&
                        ( nNdPos != nSttNd || *pTFld->GetStart() >= nStt ) &&
                        ( nNdPos != nEndNd || *pTFld->GetStart() < nEnd ))
                        return TRUE;
                }
            }
        } while( 0 != ( pLast = aIter++ ));

    return FALSE;
}

SwDDEFieldType::SwDDEFieldType(const String& rName,
                                const String& rCmd, USHORT nUpdateType )
    : SwFieldType( RES_DDEFLD ),
    aName( rName ), pDoc( 0 ), nRefCnt( 0 )
{
    bCRLFFlag = bDeleted = FALSE;
    refLink = new SwIntrnlRefLink( *this, nUpdateType, FORMAT_STRING );
    SetCmd( rCmd );
}

SwDDEFieldType::~SwDDEFieldType()
{
    if( pDoc && !pDoc->IsInDtor() )
        pDoc->GetLinkManager().Remove( *refLink );
    refLink->Disconnect();
}


SwFieldType* SwDDEFieldType::Copy() const
{
    SwDDEFieldType* pType = new SwDDEFieldType( aName, GetCmd(), GetType() );
    pType->aExpansion = aExpansion;
    pType->bCRLFFlag = bCRLFFlag;
    pType->bDeleted = bDeleted;
    pType->SetDoc( pDoc );
    return pType;
}

const String& SwDDEFieldType::GetName() const
{
    return aName;
}

void SwDDEFieldType::SetCmd( const String& rStr )
{
    String sCmd( rStr );
    xub_StrLen nPos;
    while( STRING_NOTFOUND != (nPos = sCmd.SearchAscii( "  " )) )
        sCmd.Erase( nPos, 1 );
    refLink->SetLinkSourceName( new SvLinkName( sCmd ) );
}

String SwDDEFieldType::GetCmd() const
{
    SvLinkName* pLNm = refLink->GetLinkSourceName();
    if( pLNm )
        return pLNm->GetName();
    return aEmptyStr;
}

void SwDDEFieldType::SetDoc( SwDoc* pNewDoc )
{
    if( pNewDoc == pDoc )
        return;

    if( pDoc && refLink.Is() )
    {
        ASSERT( !nRefCnt, "wie kommen die Referenzen rueber?" );
        pDoc->GetLinkManager().Remove( *refLink );
    }

    pDoc = pNewDoc;
    if( pDoc && nRefCnt )
    {
        refLink->SetVisible( pDoc->IsVisibleLinks() );
        pDoc->GetLinkManager().InsertDDELink( *refLink );
    }
}


void SwDDEFieldType::_RefCntChgd()
{
    if( nRefCnt )
    {
        refLink->SetVisible( pDoc->IsVisibleLinks() );
        pDoc->GetLinkManager().InsertDDELink( *refLink );
        if( pDoc->GetRootFrm() )
            UpdateNow();
    }
    else
    {
        Disconnect();
        pDoc->GetLinkManager().Remove( *refLink );
    }
}
/* -----------------------------28.08.00 16:23--------------------------------

 ---------------------------------------------------------------------------*/
BOOL    SwDDEFieldType::QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const
{
    USHORT nPart = rProperty.EqualsAscii(UNO_NAME_DDE_COMMAND_TYPE)  ? 0 :
        rProperty.EqualsAscii(UNO_NAME_DDE_COMMAND_FILE)  ? 1 :
            rProperty.EqualsAscii(UNO_NAME_DDE_COMMAND_ELEMENT)  ? 2 :
            rProperty.EqualsAscii(UNO_NAME_IS_AUTOMATIC_UPDATE) ? 3 : USHRT_MAX;
    if(nPart < 3 )
    {
        rVal <<= OUString(GetCmd().GetToken(nPart, cTokenSeperator));
    }
    else if(3 == nPart)
    {
        sal_Bool bSet = GetType() == LINKUPDATE_ALWAYS ? TRUE : FALSE;
        rVal.setValue(&bSet, ::getBooleanCppuType());
    }
    else
        return FALSE;
    return TRUE;
}
/* -----------------------------28.08.00 16:23--------------------------------

 ---------------------------------------------------------------------------*/
BOOL    SwDDEFieldType::PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty )
{
    USHORT nPart = rProperty.EqualsAscii(UNO_NAME_DDE_COMMAND_TYPE)  ? 0 :
        rProperty.EqualsAscii(UNO_NAME_DDE_COMMAND_FILE)  ? 1 :
            rProperty.EqualsAscii(UNO_NAME_DDE_COMMAND_ELEMENT)  ? 2 :
            rProperty.EqualsAscii(UNO_NAME_IS_AUTOMATIC_UPDATE) ? 3 : USHRT_MAX;
    if(nPart < 3 )
    {
        OUString sVal;
        rVal >>= sVal;
        String sCmd = GetCmd();
        while(3 > sCmd.GetTokenCount(cTokenSeperator))
            sCmd += cTokenSeperator;
        sCmd.SetToken(nPart, cTokenSeperator, sVal);
        SetCmd( sCmd );
    }
    else if(3 == nPart)
    {
        sal_Bool bSet = *(sal_Bool*)rVal.getValue();
        SetType( bSet ? LINKUPDATE_ALWAYS : LINKUPDATE_ONCALL );
    }
    else
        return FALSE;
    return TRUE;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwDDEField::SwDDEField( SwDDEFieldType* pType )
    : SwField(pType)
{
}

SwDDEField::~SwDDEField()
{
    if( GetTyp()->IsLastDepend() )                      // der Letzte mach das
        ((SwDDEFieldType*)GetTyp())->Disconnect();      // Licht aus
}

String SwDDEField::Expand() const
{
    xub_StrLen nPos;
    String aStr( ((SwDDEFieldType*)GetTyp())->GetExpansion() );

    aStr.EraseAllChars( '\r' );
    while( (nPos = aStr.Search( '\t' )) != STRING_NOTFOUND )
        aStr.SetChar( ' ', nPos );
    while( (nPos = aStr.Search( '\n' )) != STRING_NOTFOUND )
        aStr.SetChar( '|', nPos );
    if( aStr.Len() && ( aStr.GetChar( aStr.Len()-1 ) == '|') )
        aStr.Erase( aStr.Len()-1, 1 );
    return aStr;
}

SwField* SwDDEField::Copy() const
{
    return new SwDDEField((SwDDEFieldType*)GetTyp());
}

/*--------------------------------------------------------------------
    Beschreibung: Parameter des Typen erfragen
                  Name
 --------------------------------------------------------------------*/
const String& SwDDEField::GetPar1() const
{
    return ((SwDDEFieldType*)GetTyp())->GetName();
}

/*--------------------------------------------------------------------
    Beschreibung: Parameter des Typen erfragen
                  Commando
 --------------------------------------------------------------------*/
String SwDDEField::GetPar2() const
{
    return ((SwDDEFieldType*)GetTyp())->GetCmd();
}

void SwDDEField::SetPar2(const String& rStr)
{
    ((SwDDEFieldType*)GetTyp())->SetCmd(rStr);
}

