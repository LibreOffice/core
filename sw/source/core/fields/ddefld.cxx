/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sfx2/linkmgr.hxx>
#include <doc.hxx>
#include <editsh.hxx>
#include <ndtxt.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <ddefld.hxx>
#include <swtable.hxx>
#include <swbaslnk.hxx>
#include <swddetbl.hxx>
#include <unofldmid.h>
#include <hints.hxx>

using namespace ::com::sun::star;

#define DDE_TXT_ENCODING    osl_getThreadTextEncoding()

class SwIntrnlRefLink : public SwBaseLink
{
    SwDDEFieldType& rFldType;
public:
    SwIntrnlRefLink( SwDDEFieldType& rType, sal_uInt16 nUpdateType, sal_uInt16 nFmt )
        : SwBaseLink( nUpdateType, nFmt ),
        rFldType( rType )
    {}

    virtual void Closed();
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const ::com::sun::star::uno::Any & rValue );

    virtual const SwNode* GetAnchor() const;
    virtual sal_Bool IsInRange( sal_uLong nSttNd, sal_uLong nEndNd, xub_StrLen nStt = 0,
                            xub_StrLen nEnd = STRING_NOTFOUND ) const;
};

::sfx2::SvBaseLink::UpdateResult SwIntrnlRefLink::DataChanged( const OUString& rMimeType,
                                const uno::Any & rValue )
{
    switch( SotExchange::GetFormatIdFromMimeType( rMimeType ) )
    {
    case FORMAT_STRING:
        if( !IsNoDataFlag() )
        {
            uno::Sequence< sal_Int8 > aSeq;
            rValue >>= aSeq;
            String sStr( (sal_Char*)aSeq.getConstArray(), static_cast<xub_StrLen>(aSeq.getLength()),
                               DDE_TXT_ENCODING  );

            // remove not needed CR-LF at the end
            xub_StrLen n = sStr.Len();
            while( n && 0 == sStr.GetChar( n-1 ) )
                --n;
            if( n && 0x0a == sStr.GetChar( n-1 ) )
                --n;
            if( n && 0x0d == sStr.GetChar( n-1 ) )
                --n;

            sal_Bool bDel = n != sStr.Len();
            if( bDel )
                sStr.Erase( n );

            rFldType.SetExpansion( sStr );
            // set Expansion first! (otherwise this flag will be deleted)
            rFldType.SetCRLFDelFlag( bDel );
        }
        break;

    // other formats
    default:
        return SUCCESS;
    }

    OSL_ENSURE( rFldType.GetDoc(), "no pDoc" );

    // no dependencies left?
    if( rFldType.GetDepends() && !rFldType.IsModifyLocked() && !ChkNoDataFlag() )
    {
        ViewShell* pSh;
        SwEditShell* pESh = rFldType.GetDoc()->GetEditShell( &pSh );

        // Search for fields. If no valid found, disconnect.
        SwMsgPoolItem aUpdateDDE( RES_UPDATEDDETBL );
        int bCallModify = sal_False;
        rFldType.LockModify();

        SwClientIter aIter( rFldType );     // TODO
        SwClient * pLast = aIter.GoStart();
        if( pLast )     // Could we jump to beginning?
            do {
                // a DDE table or a DDE field attribute in the text
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
                    pLast->ModifyNotification( 0, &aUpdateDDE );
                    bCallModify = sal_True;
                }
            } while( 0 != ( pLast = ++aIter ));

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

    return SUCCESS;
}

void SwIntrnlRefLink::Closed()
{
    if( rFldType.GetDoc() && !rFldType.GetDoc()->IsInDtor() )
    {
        // advise goes, convert all fields into text?
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
    // here, any anchor of the normal NodesArray should be sufficient
    const SwNode* pNd = 0;
    SwClientIter aIter( rFldType );     // TODO
    SwClient * pLast = aIter.GoStart();
    if( pLast )     // Could we jump to beginning?
        do {
            // a DDE table or a DDE field attribute in the text
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
        } while( 0 != ( pLast = ++aIter ));

    return pNd;
}

sal_Bool SwIntrnlRefLink::IsInRange( sal_uLong nSttNd, sal_uLong nEndNd,
                                xub_StrLen nStt, xub_StrLen nEnd ) const
{
    // here, any anchor of the normal NodesArray should be sufficient
    SwNodes* pNds = &rFldType.GetDoc()->GetNodes();
    SwClientIter aIter( rFldType );         // TODO
    SwClient * pLast = aIter.GoStart();
    if( pLast )     // Could we jump to beginning?
        do {
            // a DDE table or a DDE field attribute in the text
            if( !pLast->IsA( TYPE( SwFmtFld ) ))
            {
                SwDepend* pDep = (SwDepend*)pLast;
                SwDDETable* pDDETbl = (SwDDETable*)pDep->GetToTell();
                const SwTableNode* pTblNd = pDDETbl->GetTabSortBoxes()[0]->
                                GetSttNd()->FindTableNode();
                if( pTblNd->GetNodes().IsDocNodes() &&
                    nSttNd < pTblNd->EndOfSectionIndex() &&
                    nEndNd > pTblNd->GetIndex() )
                    return sal_True;
            }
            else if( ((SwFmtFld*)pLast)->GetTxtFld() )
            {
                const SwTxtFld* pTFld = ((SwFmtFld*)pLast)->GetTxtFld();
                const SwTxtNode* pNd = pTFld->GetpTxtNode();
                if( pNd && pNds == &pNd->GetNodes() )
                {
                    sal_uLong nNdPos = pNd->GetIndex();
                    if( nSttNd <= nNdPos && nNdPos <= nEndNd &&
                        ( nNdPos != nSttNd || *pTFld->GetStart() >= nStt ) &&
                        ( nNdPos != nEndNd || *pTFld->GetStart() < nEnd ))
                        return sal_True;
                }
            }
        } while( 0 != ( pLast = ++aIter ));

    return sal_False;
}

SwDDEFieldType::SwDDEFieldType(const OUString& rName,
                               const OUString& rCmd, sal_uInt16 nUpdateType )
    : SwFieldType( RES_DDEFLD ),
    aName( rName ), pDoc( 0 ), nRefCnt( 0 )
{
    bCRLFFlag = bDeleted = sal_False;
    refLink = new SwIntrnlRefLink( *this, nUpdateType, FORMAT_STRING );
    SetCmd( rCmd );
}

SwDDEFieldType::~SwDDEFieldType()
{
    if( pDoc && !pDoc->IsInDtor() )
        pDoc->GetLinkManager().Remove( refLink );
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

OUString SwDDEFieldType::GetName() const
{
    return aName;
}

void SwDDEFieldType::SetCmd( OUString aStr )
{
    sal_Int32 nIndex = 0;
    do
    {
        aStr = aStr.replaceFirst("  ", " ", &nIndex);
    } while (nIndex>=0);
    refLink->SetLinkSourceName( aStr );
}

OUString SwDDEFieldType::GetCmd() const
{
    return refLink->GetLinkSourceName();
}

void SwDDEFieldType::SetDoc( SwDoc* pNewDoc )
{
    if( pNewDoc == pDoc )
        return;

    if( pDoc && refLink.Is() )
    {
        OSL_ENSURE( !nRefCnt, "How do we get the references?" );
        pDoc->GetLinkManager().Remove( refLink );
    }

    pDoc = pNewDoc;
    if( pDoc && nRefCnt )
    {
        refLink->SetVisible( pDoc->IsVisibleLinks() );
        pDoc->GetLinkManager().InsertDDELink( refLink );
    }
}


void SwDDEFieldType::_RefCntChgd()
{
    if( nRefCnt )
    {
        refLink->SetVisible( pDoc->IsVisibleLinks() );
        pDoc->GetLinkManager().InsertDDELink( refLink );
        if( pDoc->GetCurrentViewShell() )   //swmod 071108//swmod 071225
            UpdateNow();
    }
    else
    {
        Disconnect();
        pDoc->GetLinkManager().Remove( refLink );
    }
}

bool SwDDEFieldType::QueryValue( uno::Any& rVal, sal_uInt16 nWhichId ) const
{
    sal_Int32 nPart = -1;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR2:      nPart = 2; break;
    case FIELD_PROP_PAR4:      nPart = 1; break;
    case FIELD_PROP_SUBTYPE:   nPart = 0; break;
    case FIELD_PROP_BOOL1:
        {
            sal_Bool bSet = GetType() == sfx2::LINKUPDATE_ALWAYS ? sal_True : sal_False;
            rVal.setValue(&bSet, ::getBooleanCppuType());
        }
        break;
    case FIELD_PROP_PAR5:
        rVal <<= aExpansion;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    if ( nPart>=0 )
        rVal <<= GetCmd().getToken(nPart, sfx2::cTokenSeparator);
    return true;
}

bool SwDDEFieldType::PutValue( const uno::Any& rVal, sal_uInt16 nWhichId )
{
    sal_Int32 nPart = -1;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR2:      nPart = 2; break;
    case FIELD_PROP_PAR4:      nPart = 1; break;
    case FIELD_PROP_SUBTYPE:   nPart = 0; break;
    case FIELD_PROP_BOOL1:
        SetType( static_cast<sal_uInt16>(*(sal_Bool*)rVal.getValue() ?
                                     sfx2::LINKUPDATE_ALWAYS :
                                     sfx2::LINKUPDATE_ONCALL ) );
        break;
    case FIELD_PROP_PAR5:
        rVal >>= aExpansion;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    if( nPart>=0 )
    {
        const OUString sOldCmd( GetCmd() );
        OUString sNewCmd;
        sal_Int32 nIndex = 0;
        for (sal_Int32 i=0; i<3; ++i)
        {
            OUString sToken = sOldCmd.getToken(0, sfx2::cTokenSeparator, nIndex);
            if (i==nPart)
            {
                rVal >>= sToken;
            }
            sNewCmd += sToken + OUString(sfx2::cTokenSeparator);
        }
        SetCmd( sNewCmd );
    }
    return true;
}

SwDDEField::SwDDEField( SwDDEFieldType* pInitType )
    : SwField(pInitType)
{
}

SwDDEField::~SwDDEField()
{
    if( GetTyp()->IsLastDepend() )
        ((SwDDEFieldType*)GetTyp())->Disconnect();
}

OUString SwDDEField::Expand() const
{
    OUString aStr = ((SwDDEFieldType*)GetTyp())->GetExpansion();
    aStr = aStr.replaceAll("\r", OUString());
    aStr = aStr.replaceAll("\t", " ");
    aStr = aStr.replaceAll("\n", "|");
    if (aStr.endsWith("|"))
    {
        return aStr.copy(0, aStr.getLength()-1);
    }
    return aStr;
}

SwField* SwDDEField::Copy() const
{
    return new SwDDEField((SwDDEFieldType*)GetTyp());
}

/// get field type name
OUString SwDDEField::GetPar1() const
{
    return ((const SwDDEFieldType*)GetTyp())->GetName();
}

/// get field type command
OUString SwDDEField::GetPar2() const
{
    return ((const SwDDEFieldType*)GetTyp())->GetCmd();
}

/// set field type command
void SwDDEField::SetPar2(const OUString& rStr)
{
    ((SwDDEFieldType*)GetTyp())->SetCmd(rStr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
