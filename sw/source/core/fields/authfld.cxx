/*************************************************************************
 *
 *  $RCSfile: authfld.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-20 11:11:17 $
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

#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_USHORTS
#define _SVSTDARR_LONGS

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#include <svtools/svstdarr.hxx>

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _AUTHFLD_HXX
#include <authfld.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _TXMSRT_HXX
#include <txmsrt.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX
#include <txtfld.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#include <doc.hxx>
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif

typedef SwAuthEntry* SwAuthEntryPtr;
SV_DECL_PTRARR_DEL( SwAuthDataArr, SwAuthEntryPtr, 5, 5 )
SV_IMPL_PTRARR( SwAuthDataArr, SwAuthEntryPtr )


typedef SwTOXSortKey* TOXSortKeyPtr;
SV_DECL_PTRARR_DEL( SortKeyArr, TOXSortKeyPtr, 5, 5 )
SV_IMPL_PTRARR( SortKeyArr, TOXSortKeyPtr )


/* -----------------16.09.99 11:53-------------------

 --------------------------------------------------*/
SwAuthEntry::SwAuthEntry(const SwAuthEntry& rCopy)
    : nRefCount(0)
{
    for(USHORT i = 0; i < AUTH_FIELD_END; i++)
        aAuthFields[i] = rCopy.aAuthFields[i];
}
// --------------------------------------------------------
BOOL    SwAuthEntry::operator==(const SwAuthEntry& rComp)
{
    for(USHORT i = 0; i < AUTH_FIELD_END; i++)
        if(aAuthFields[i] != rComp.aAuthFields[i])
            return FALSE;
    return TRUE;
}
// --------------------------------------------------------
BOOL    SwAuthEntry::GetFirstAuthorField(USHORT& nPos, String& rToFill)const
{
    BOOL bRet = FALSE;
        for(USHORT i = 0; i < AUTH_FIELD_END; i++)
            if(aAuthFields[i].Len())
            {
                rToFill = aAuthFields[i];
                nPos = i;
                bRet = TRUE;
                break;
            }
    return bRet;
}
// --------------------------------------------------------
BOOL    SwAuthEntry::GetNextAuthorField(USHORT& nPos, String& rToFill)const
{
    BOOL bRet = FALSE;
    if(AUTH_FIELD_END > ++nPos)
    {
        for(USHORT i = nPos; i < AUTH_FIELD_END; i++)
            if(aAuthFields[i].Len())
            {
                rToFill = aAuthFields[i];
                nPos = i;
                bRet = TRUE;
                break;
            }
    }
    return bRet;
}

// --------------------------------------------------------

/* -----------------14.09.99 16:15-------------------

 --------------------------------------------------*/
SwAuthorityFieldType::SwAuthorityFieldType(SwDoc* pDoc)
    : SwFieldType( RES_AUTHORITY ),
    m_pDoc(pDoc),
    m_pDataArr(new SwAuthDataArr ),
    m_pSequArr(new SvLongs(5, 5)),
    m_pSortKeyArr(new SortKeyArr(3, 3)),
    m_bSortByDocument(TRUE),
    m_bIsSequence(FALSE),
    m_cPrefix('['),
    m_cSuffix(']')
{
}

SwAuthorityFieldType::SwAuthorityFieldType( const SwAuthorityFieldType& rFType)
    : SwFieldType( RES_AUTHORITY ),
    m_pDataArr(new SwAuthDataArr ),
    m_pSequArr(new SvLongs(5, 5)),
    m_pSortKeyArr(new SortKeyArr(3, 3)),
    m_bSortByDocument(rFType.m_bSortByDocument),
    m_bIsSequence(rFType.m_bIsSequence),
    m_cPrefix(rFType.m_cPrefix),
    m_cSuffix(rFType.m_cSuffix)
{
    for(USHORT i = 0; i < rFType.m_pSortKeyArr->Count(); i++)
        m_pSortKeyArr->Insert((*rFType.m_pSortKeyArr)[i], i);
}

/* -----------------17.09.99 13:52-------------------

 --------------------------------------------------*/
SwAuthorityFieldType::~SwAuthorityFieldType()
{
    DBG_ASSERT(!m_pDataArr->Count(), "Array is not empty")
    m_pSortKeyArr->DeleteAndDestroy(0, m_pSortKeyArr->Count());
    delete m_pSortKeyArr;
    delete m_pSequArr;
    delete m_pDataArr;
}
/*-- 14.09.99 16:22:09---------------------------------------------------

  -----------------------------------------------------------------------*/
SwFieldType*    SwAuthorityFieldType::Copy()  const
{
    return new SwAuthorityFieldType(m_pDoc);
}
/* -----------------17.09.99 13:43-------------------

 --------------------------------------------------*/
void    SwAuthorityFieldType::RemoveField(long nHandle)
{
#ifdef DBG_UTIL
    BOOL bRemoved = FALSE;
#endif
    for(USHORT j = 0; j < m_pDataArr->Count(); j++)
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject(j);
        long nRet = (long)(void*)pTemp;
        if(nRet == nHandle)
        {
#ifdef DBG_UTIL
            bRemoved = TRUE;
#endif
            pTemp->RemoveRef();
            if(!pTemp->GetRefCount())
            {
                m_pDataArr->DeleteAndDestroy(j, 1);
            }
            break;
        }
    }
#ifdef DBG_UTIL
    DBG_ASSERT(bRemoved, "Field unknown" )
#endif
}
/* -----------------17.09.99 13:43-------------------

 --------------------------------------------------*/
long    SwAuthorityFieldType::AddField(const String& rFieldContents)
{
    long nRet = 0;
    SwAuthEntry* pEntry = new SwAuthEntry;
    for( USHORT i = 0; i < AUTH_FIELD_END; ++i )
        pEntry->SetAuthorField( (ToxAuthorityField)i,
                        rFieldContents.GetToken( i, TOX_STYLE_DELIMITER ));

    for(USHORT j = 0; j < m_pDataArr->Count() && pEntry; j++)
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject(j);
        if(*pTemp == *pEntry)
        {
            DELETEZ(pEntry);
            nRet = (long)(void*)pTemp;
            pTemp->AddRef();
        }
    }
    //if it is a new Entry - insert
    if(pEntry)
    {
        nRet = (long)(void*)pEntry;
        pEntry->AddRef();
        m_pDataArr->Insert(pEntry, m_pDataArr->Count());
    }
    return nRet;
}
/* -----------------17.09.99 14:18-------------------

 --------------------------------------------------*/
BOOL SwAuthorityFieldType::AddField(long nHandle)
{
    BOOL bRet = FALSE;
    for( USHORT j = 0; j < m_pDataArr->Count(); j++ )
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject(j);
        long nTmp = (long)(void*)pTemp;
        if( nTmp == nHandle )
        {
            bRet = TRUE;
            pTemp->AddRef();
            break;
        }
    }
    return bRet;
}
/* -----------------17.09.99 14:52-------------------

 --------------------------------------------------*/
const SwAuthEntry*  SwAuthorityFieldType::GetEntryByHandle(long nHandle) const
{
    const SwAuthEntry* pRet = 0;
    for(USHORT j = 0; j < m_pDataArr->Count(); j++)
    {
        const SwAuthEntry* pTemp = m_pDataArr->GetObject(j);
        long nTmp = (long)(void*)pTemp;
        if( nTmp == nHandle )
        {
            pRet = pTemp;
            break;
        }
    }
    ASSERT( pRet, "invalid Handle" );
    return pRet;
}
/* -----------------21.09.99 13:34-------------------

 --------------------------------------------------*/
void SwAuthorityFieldType::GetAllEntryIdentifiers(
                SvStringsDtor& rToFill )const
{
    for(USHORT j = 0; j < m_pDataArr->Count(); j++)
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject(j);
        rToFill.Insert( new String( pTemp->GetAuthorField(
                    AUTH_FIELD_IDENTIFIER )), rToFill.Count() );
    }
}
/* -----------------21.09.99 13:34-------------------

 --------------------------------------------------*/
const SwAuthEntry*  SwAuthorityFieldType::GetEntryByIdentifier(
                                const String& rIdentifier)const
{
    const SwAuthEntry* pRet = 0;
    for( USHORT j = 0; j < m_pDataArr->Count(); ++j )
    {
        const SwAuthEntry* pTemp = m_pDataArr->GetObject(j);
        if( rIdentifier == pTemp->GetAuthorField( AUTH_FIELD_IDENTIFIER ))
        {
            pRet = pTemp;
            break;
        }
    }
    return pRet;
}
/* -----------------------------21.12.99 13:20--------------------------------

 ---------------------------------------------------------------------------*/
void SwAuthorityFieldType::ChangeEntryContent(const SwAuthEntry* pNewEntry)
{
    for( USHORT j = 0; j < m_pDataArr->Count(); ++j )
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject(j);
        if(pTemp->GetAuthorField(AUTH_FIELD_IDENTIFIER) ==
                    pNewEntry->GetAuthorField(AUTH_FIELD_IDENTIFIER))
        {
            for(USHORT i = 0; i < AUTH_FIELD_END; i++)
                pTemp->SetAuthorField((ToxAuthorityField) i,
                    pNewEntry->GetAuthorField((ToxAuthorityField)i));
            break;
        }
    }
}
/*-- 11.10.99 08:49:22---------------------------------------------------
    Description:    appends a new entry (if new) and returns the array position

  -----------------------------------------------------------------------*/
USHORT  SwAuthorityFieldType::AppendField( const SwAuthEntry& rInsert )
{
    USHORT nRet = 0;
    for( nRet = 0; nRet < m_pDataArr->Count(); ++nRet )
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject( nRet );
        if( *pTemp == rInsert )
        {
            break;
            //ref count unchanged
        }
    }

    //if it is a new Entry - insert
    if( nRet == m_pDataArr->Count() )
        m_pDataArr->Insert( new SwAuthEntry( rInsert ), nRet );

    return nRet;
}

/*-- 11.10.99 08:49:23---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwAuthorityFieldType::MergeFieldType( const SwAuthorityFieldType& rNew,
                                            SvUShorts& rMap )
{
    DBG_ASSERT( !rMap.Count(), "array not empty!")
    for( USHORT i = 0; i < rNew.m_pDataArr->Count(); ++i )
    {
        USHORT  nNewPos = AppendField(*rNew.m_pDataArr->GetObject(i));
        rMap.Insert(nNewPos, i);
    }
}

/*-- 11.10.99 08:49:23---------------------------------------------------
    Description:    After import is done some of the array members may have a

  -----------------------------------------------------------------------*/
void SwAuthorityFieldType::RemoveUnusedFields()
{
    for( USHORT j = m_pDataArr->Count(); j; )
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject( --j );
        if( !pTemp->GetRefCount() )
        {
            m_pDataArr->Remove( j );
            delete pTemp;
        }
    }
}

/*-- 11.10.99 08:49:24---------------------------------------------------

  -----------------------------------------------------------------------*/
long    SwAuthorityFieldType::GetHandle(USHORT nPos)
{
    long nRet = 0;
    if( nPos < m_pDataArr->Count() )
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject(nPos);
        nRet = (long)(void*)pTemp;
    }
    return nRet;
}
/* -----------------20.10.99 13:38-------------------

 --------------------------------------------------*/
USHORT  SwAuthorityFieldType::GetPosition(long nHandle)
{
    USHORT j = 0;
    for( ; j < m_pDataArr->Count(); ++j )
    {
        const SwAuthEntry* pTemp = m_pDataArr->GetObject(j);
        long nTmp = (long)(void*)pTemp;
        if( nTmp == nHandle )
            break;
    }
    if( j == m_pDataArr->Count() )
        j = USHRT_MAX;

    ASSERT( USHRT_MAX != j, "handle not found" );
    return j;
}

/*-- 11.10.99 08:51:03---------------------------------------------------

  -----------------------------------------------------------------------*/
USHORT  SwAuthorityFieldType::GetEntryCount() const
{
    return m_pDataArr->Count();
}
/*-- 11.10.99 08:51:03---------------------------------------------------

  -----------------------------------------------------------------------*/
const SwAuthEntry*  SwAuthorityFieldType::GetEntryByPosition(USHORT nPos) const
{
    if(nPos < m_pDataArr->Count())
        return m_pDataArr->GetObject(nPos);
    DBG_ERROR("wrong index")
    return 0;
}
/* -----------------19.10.99 13:46-------------------

 --------------------------------------------------*/
USHORT  SwAuthorityFieldType::GetSequencePos(long nHandle)
{
    //find the field in a sorted array of handles,
    //aHandleArr....
    if(!m_pSequArr->Count())
    {
        SwTOXSortTabBases aSortArr;
        SwClientIter aIter( *this );

        SwTOXInternational aIntl(
                        ((const SvxLanguageItem&)m_pDoc->GetAttrPool().
                        GetDefaultItem(RES_CHRATR_LANGUAGE )).GetLanguage());

        for( SwFmtFld* pFmtFld = (SwFmtFld*)aIter.First( TYPE(SwFmtFld) );
                                pFmtFld; pFmtFld = (SwFmtFld*)aIter.Next() )
        {
            SwAuthorityField* pAFld = (SwAuthorityField*)pFmtFld->GetFld();
            const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
            if(!pTxtFld)
                continue;
            const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
            ULONG nPos = rTxtNode.GetIndex();
            BOOL bInsert = TRUE;
            if( rTxtNode.GetTxt().Len() && rTxtNode.GetFrm() &&
                rTxtNode.GetNodes().IsDocNodes() )
            {
                SwTOXAuthority* pNew = new SwTOXAuthority( rTxtNode,
                                                            *pFmtFld, aIntl );
    //          InsertSorted(pNew);
    //          aSortArr
                Range aRange(0, aSortArr.Count());
                for(short i = (short)aRange.Min(); i < (short)aRange.Max(); ++i)
                {
                    SwTOXSortTabBase* pOld = aSortArr[i];
                    if(*pOld == *pNew)
                    {
                        bInsert = FALSE;
                        //only the first occurence in the document
                        //has to be in the array
                        if(*pOld < *pNew)
                        {
                            delete pNew;
                        }
                        else
                        {
                            // remove the old content
                            aSortArr.DeleteAndDestroy( i, 1 );
                            aSortArr.Insert(pNew, i );
                        }
                    }
                    if(*pNew < *pOld)
                        break;
                }
                if(bInsert)
                    aSortArr.Insert(pNew, i );
                bInsert = TRUE;
            }
        }

        for(USHORT i = 0; i < aSortArr.Count(); i++)
        {
            const SwTOXSortTabBase& rBase = *aSortArr[i];
            SwFmtFld& rFmtFld = ((SwTOXAuthority&)rBase).GetFldFmt();
            SwAuthorityField* pAFld = (SwAuthorityField*)rFmtFld.GetFld();
            m_pSequArr->Insert(pAFld->GetHandle(), i);
        }
        aSortArr.DeleteAndDestroy(0, aSortArr.Count());
    }
    //find nHandle
    USHORT nRet;
    for(USHORT i = 0; i < m_pSequArr->Count(); i++)
    {
        if((*m_pSequArr)[i] == nHandle)
        {
            nRet = i + 1;
            break;
        }
    }
    ASSERT(nRet, "Handle not found")
    return nRet;
}

/* -----------------19.10.99 13:25-------------------

 --------------------------------------------------*/
void SwAuthorityFieldType::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    //re-generate positions of the fields
    m_pSequArr->Remove(0, m_pSequArr->Count());
    SwModify::Modify( pOld, pNew );
}
/* -----------------20.10.99 13:38-------------------

 --------------------------------------------------*/
USHORT SwAuthorityFieldType::GetSortKeyCount() const
{
    return m_pSortKeyArr->Count();
}
/* -----------------20.10.99 13:38-------------------

 --------------------------------------------------*/
const SwTOXSortKey*  SwAuthorityFieldType::GetSortKey(USHORT nIdx) const
{
    SwTOXSortKey* pRet = 0;
    if(m_pSortKeyArr->Count() > nIdx)
        pRet = (*m_pSortKeyArr)[nIdx];
    DBG_ASSERT(pRet, "Sort key not found")
    return pRet;
}
/* -----------------20.10.99 13:38-------------------

 --------------------------------------------------*/
void SwAuthorityFieldType::SetSortKeys(USHORT nKeyCount, SwTOXSortKey aKeys[])
{
    m_pSortKeyArr->DeleteAndDestroy(0, m_pSortKeyArr->Count());
    USHORT nArrIdx = 0;
    for(USHORT i = 0; i < nKeyCount; i++)
        if(aKeys[i].eField < AUTH_FIELD_END)
            m_pSortKeyArr->Insert(new SwTOXSortKey(aKeys[i]), nArrIdx++);
}

/* -----------------14.09.99 16:15-------------------

 --------------------------------------------------*/
SwAuthorityField::SwAuthorityField( SwAuthorityFieldType* pType,
                                    const String& rFieldContents )
    : SwField(pType)
{
    nHandle = pType->AddField( rFieldContents );
}
/* -----------------17.09.99 14:24-------------------

 --------------------------------------------------*/
SwAuthorityField::SwAuthorityField( SwAuthorityFieldType* pType,
                                                long nSetHandle )
    : SwField( pType ),
    nHandle( nSetHandle )
{
    pType->AddField( nHandle );
}
/* -----------------15.09.99 15:00-------------------

 --------------------------------------------------*/
SwAuthorityField::~SwAuthorityField()
{
    ((SwAuthorityFieldType* )GetTyp())->RemoveField(nHandle);
}
/*-- 14.09.99 16:20:59---------------------------------------------------

  -----------------------------------------------------------------------*/
String  SwAuthorityField::Expand() const
{
    SwAuthorityFieldType* pAuthType = (SwAuthorityFieldType*)GetTyp();
    String sRet(pAuthType->GetPrefix());

    if( pAuthType->IsSequence() )
    {
        sRet += String::CreateFromInt32( pAuthType->GetSequencePos( nHandle ));
    }
    else
    {
        const SwAuthEntry* pEntry = pAuthType->GetEntryByHandle(nHandle);
        //TODO: Expand to: identifier, number sequence, ...
        if(pEntry)
            sRet += pEntry->GetAuthorField(AUTH_FIELD_IDENTIFIER);
    }
    sRet += pAuthType->GetSuffix();
    return sRet;
}
/*-- 14.09.99 16:21:00---------------------------------------------------

  -----------------------------------------------------------------------*/
SwField* SwAuthorityField::Copy() const
{
    SwAuthorityFieldType* pAuthType = (SwAuthorityFieldType*)GetTyp();
    return new SwAuthorityField(pAuthType, nHandle);
}
/* -----------------21.09.99 12:55-------------------

 --------------------------------------------------*/
const String&   SwAuthorityField::GetFieldText(ToxAuthorityField eField) const
{
    SwAuthorityFieldType* pAuthType = (SwAuthorityFieldType*)GetTyp();
    const SwAuthEntry* pEntry = pAuthType->GetEntryByHandle( nHandle );
    return pEntry->GetAuthorField( eField );
}
/* -----------------21.09.99 14:57-------------------

 --------------------------------------------------*/
void    SwAuthorityField::SetPar1(const String& rStr)
{
    SwAuthorityFieldType* pType = (SwAuthorityFieldType* )GetTyp();
    pType->RemoveField(nHandle);
    nHandle = pType->AddField(rStr);
}
/* -----------------11.10.99 09:43-------------------

 --------------------------------------------------*/
USHORT  SwAuthorityField::GetHandlePosition() const
{
    SwAuthorityFieldType* pAuthType = (SwAuthorityFieldType*)GetTyp();
    DBG_ASSERT(pAuthType, "no field type")
    return pAuthType->GetPosition(nHandle);
}


SwFieldType* SwAuthorityField::ChgTyp( SwFieldType* pFldTyp )
{
    SwAuthorityFieldType* pSrcTyp = (SwAuthorityFieldType*)GetTyp(),
                        * pDstTyp = (SwAuthorityFieldType*)pFldTyp;
    if( pSrcTyp != pDstTyp )
    {

        const SwAuthEntry* pEntry = pSrcTyp->GetEntryByHandle( nHandle );
        USHORT nHdlPos = pDstTyp->AppendField( *pEntry );
        pSrcTyp->RemoveField( nHandle );
        nHandle = pDstTyp->GetHandle( nHdlPos );
        pDstTyp->AddField( nHandle );
        SwField::ChgTyp( pFldTyp );
    }
    return pSrcTyp;
}


