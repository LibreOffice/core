/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unoobj.cxx,v $
 * $Revision: 1.111.80.1 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <com/sun/star/table/TableSortField.hpp>

#include <osl/endian.h>
#include <rtl/ustrbuf.hxx>
#include <unotools/collatorwrapper.hxx>
#include <swtypes.hxx>
#include <hintids.hxx>
#include <cmdid.h>
#include <hints.hxx>
#include <IMark.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <istyleaccess.hxx>
#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <unocrsrhelper.hxx>
#include <swundo.hxx>
#include <rootfrm.hxx>
#include <flyfrm.hxx>
#include <ftnidx.hxx>
#include <sfx2/linkmgr.hxx>
#include <docary.hxx>
#include <paratr.hxx>
#include <tools/urlobj.hxx>
#include <pam.hxx>
#include <tools/cachestr.hxx>
#include <shellio.hxx>
#include <swerror.h>
#include <swtblfmt.hxx>
#include <fmtruby.hxx>
#include <docsh.hxx>
#include <docstyle.hxx>
#include <charfmt.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif
#include <poolfmt.hxx>
#include <edimp.hxx>
#include <fchrfmt.hxx>
#include <fmtautofmt.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <doctxm.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <fmtrfmrk.hxx>
#include <txtrfmrk.hxx>
#include <unoclbck.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unomap.hxx>
#include <unosett.hxx>
#include <unoprnms.hxx>
#include <unotbl.hxx>
#include <unodraw.hxx>
#include <unocoll.hxx>
#include <unostyle.hxx>
#include <unofield.hxx>
#include <unometa.hxx>
#include <fmtanchr.hxx>
#include <svx/flstitem.hxx>
#include <svtools/ctrltool.hxx>
#include <flypos.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <unoidx.hxx>
#include <unoframe.hxx>
#include <fmthdft.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <fmtflcnt.hxx>
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#include <svtools/svstdarr.hxx>
#include <svx/brshitem.hxx>
#include <svx/unolingu.hxx>
#include <fmtclds.hxx>
#include <dcontact.hxx>
#include <SwStyleNameMapper.hxx>
#include <crsskip.hxx>
#include <sortopt.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <memory>
#include <unoparaframeenum.hxx>
#include <unoparagraph.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using std::auto_ptr;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

/****************************************************************************
    static methods
****************************************************************************/
uno::Sequence< sal_Int8 >  CreateUnoTunnelId()
{
    static osl::Mutex aCreateMutex;
    osl::Guard<osl::Mutex> aGuard( aCreateMutex );
    uno::Sequence< sal_Int8 > aSeq( 16 );
    rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
    return aSeq;
}
/****************************************************************************
    Hilfsklassen
****************************************************************************/

/* -----------------13.05.98 12:15-------------------
 *
 * --------------------------------------------------*/
SwUnoInternalPaM::SwUnoInternalPaM(SwDoc& rDoc) :
    SwPaM(rDoc.GetNodes())
{
}
SwUnoInternalPaM::~SwUnoInternalPaM()
{
    while( GetNext() != this)
    {
        delete GetNext();
    }
}

SwUnoInternalPaM&   SwUnoInternalPaM::operator=(const SwPaM& rPaM)
{
    const SwPaM* pTmp = &rPaM;
    *GetPoint() = *rPaM.GetPoint();
    if(rPaM.HasMark())
    {
        SetMark();
        *GetMark() = *rPaM.GetMark();
    }
    else
        DeleteMark();
    while(&rPaM != (pTmp = (const SwPaM*)pTmp->GetNext()))
    {
        if(pTmp->HasMark())
            new SwPaM(*pTmp->GetMark(), *pTmp->GetPoint(), this);
        else
            new SwPaM(*pTmp->GetPoint(), this);
    }
    return *this;
}

/*-----------------09.03.98 08:29-------------------

--------------------------------------------------*/
void SwXTextCursor::SelectPam(SwPaM& rCrsr, sal_Bool bExpand)
{
    if(bExpand)
    {
        if(!rCrsr.HasMark())
            rCrsr.SetMark();
    }
    else if(rCrsr.HasMark())
        rCrsr.DeleteMark();

}

/* -----------------20.05.98 14:59-------------------
 *
 * --------------------------------------------------*/
void SwXTextCursor::getTextFromPam(SwPaM& aCrsr, OUString& rBuffer)
{
    if(!aCrsr.HasMark())
        return;
    SvCacheStream aStream( 20480 );
#ifdef OSL_BIGENDIAN
    aStream.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
#else
    aStream.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
#endif
    WriterRef xWrt;
    // TODO/MBA: looks like a BaseURL doesn't make sense here
    SwReaderWriter::GetWriter( C2S(FILTER_TEXT_DLG), String(), xWrt );
    if( xWrt.Is() )
    {
        SwWriter aWriter( aStream, aCrsr );
        xWrt->bASCII_NoLastLineEnd = sal_True;
        xWrt->bExportPargraphNumbering = sal_False;
        SwAsciiOptions aOpt = xWrt->GetAsciiOptions();
        aOpt.SetCharSet( RTL_TEXTENCODING_UNICODE );
        xWrt->SetAsciiOptions( aOpt );
        xWrt->bUCS2_WithStartChar = FALSE;
        // --> FME #i68522#
        const BOOL bOldShowProgress = xWrt->bShowProgress;
        xWrt->bShowProgress = FALSE;
        // <--

        long lLen;
        if( !IsError( aWriter.Write( xWrt ) ) &&
            0x7ffffff > (( lLen  = aStream.GetSize() )
                                    / sizeof( sal_Unicode )) + 1 )
        {
            aStream << (sal_Unicode)'\0';

            long lUniLen = (lLen / sizeof( sal_Unicode ));
            ::rtl::OUStringBuffer aStrBuffer( lUniLen );
            aStream.Seek( 0 );
            aStream.ResetError();
            while(lUniLen)
            {
                String sBuf;
                sal_Int32 nLocalLen = 0;
                if( lUniLen >= STRING_MAXLEN )
                    nLocalLen =  STRING_MAXLEN - 1;
                else
                    nLocalLen = lUniLen;
                sal_Unicode* pStrBuf = sBuf.AllocBuffer( xub_StrLen( nLocalLen + 1));
                aStream.Read( pStrBuf, 2 * nLocalLen );
                pStrBuf[ nLocalLen ] = '\0';
                aStrBuffer.append( pStrBuf, nLocalLen );
                lUniLen -= nLocalLen;
            }
            rBuffer = aStrBuffer.makeStringAndClear();
        }
        xWrt->bShowProgress = bOldShowProgress;
    }
}

/* -----------------06.07.98 07:33-------------------
 *
 * --------------------------------------------------*/
void lcl_setCharStyle(SwDoc* pDoc, const uno::Any aValue, SfxItemSet& rSet)
     throw (lang::IllegalArgumentException)
{
    SwDocShell* pDocSh = pDoc->GetDocShell();
    if(pDocSh)
    {
        OUString uStyle;
        aValue >>= uStyle;
        String sStyle;
        SwStyleNameMapper::FillUIName(uStyle, sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, sal_True );
        SwDocStyleSheet* pStyle =
            (SwDocStyleSheet*)pDocSh->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR);
        if(pStyle)
        {
            SwFmtCharFmt aFmt(pStyle->GetCharFmt());
            rSet.Put(aFmt);
        }
        else
        {
             throw lang::IllegalArgumentException();
        }

    }
};
/* -----------------08.06.06 10:43-------------------
 *
 * --------------------------------------------------*/
void lcl_setAutoStyle(IStyleAccess& rStyleAccess, const uno::Any aValue, SfxItemSet& rSet, bool bPara )
     throw (lang::IllegalArgumentException)
{
    OUString uStyle;
    aValue >>= uStyle;
    String sStyle;
    StylePool::SfxItemSet_Pointer_t pStyle = bPara ?
        rStyleAccess.getByName(uStyle, IStyleAccess::AUTO_STYLE_PARA ):
        rStyleAccess.getByName(uStyle, IStyleAccess::AUTO_STYLE_CHAR );
    if(pStyle.get())
    {
        SwFmtAutoFmt aFmt( bPara ? sal::static_int_cast< USHORT >(RES_AUTO_STYLE)
                                 : sal::static_int_cast< USHORT >(RES_TXTATR_AUTOFMT) );
        aFmt.SetStyleHandle( pStyle );
        rSet.Put(aFmt);
    }
    else
    {
         throw lang::IllegalArgumentException();
    }
};
/* -----------------30.06.98 08:46-------------------
 *
 * --------------------------------------------------*/
void lcl_SetTxtFmtColl(const uno::Any& rAny, SwPaM& rPaM)
    throw (lang::IllegalArgumentException)
{
    SwDoc* pDoc = rPaM.GetDoc();
    SwDocShell* pDocSh = pDoc->GetDocShell();
    if(!pDocSh)
        return;
    OUString uStyle;
    rAny >>= uStyle;
    String sStyle;
    SwStyleNameMapper::FillUIName(uStyle, sStyle, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True );
    SwDocStyleSheet* pStyle =
                    (SwDocStyleSheet*)pDocSh->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_PARA);
    if(pStyle)
    {
        SwTxtFmtColl *pLocal = pStyle->GetCollection();
        UnoActionContext aAction(pDoc);
        pDoc->StartUndo( UNDO_START, NULL );
        SwPaM *pTmpCrsr = &rPaM;
        do {
            pDoc->SetTxtFmtColl(*pTmpCrsr, pLocal);
            pTmpCrsr = static_cast<SwPaM*>(pTmpCrsr->GetNext());
        } while ( pTmpCrsr != &rPaM );
        pDoc->EndUndo( UNDO_END, NULL );
    }
    else
    {
        throw lang::IllegalArgumentException();
    }

}
/* -----------------06.07.98 07:38-------------------
 *
 * --------------------------------------------------*/
 void lcl_setPageDesc(SwDoc* pDoc, const uno::Any& aValue, SfxItemSet& rSet)
 {
    if(aValue.getValueType() != ::getCppuType((const OUString*)0))
        return;
    SwFmtPageDesc *pNewDesc = 0 ;
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState( RES_PAGEDESC, sal_True, &pItem ) )
    {
        pNewDesc = new SwFmtPageDesc(*((SwFmtPageDesc*)pItem));
    }
    if(!pNewDesc)
        pNewDesc = new SwFmtPageDesc();
    OUString uDescName;
    aValue >>= uDescName;
    String sDescName;
    SwStyleNameMapper::FillUIName(uDescName, sDescName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, sal_True );
    if(!pNewDesc->GetPageDesc() || pNewDesc->GetPageDesc()->GetName() != sDescName)
    {
        sal_Bool bPut = sal_False;
        if(sDescName.Len())
        {
            SwPageDesc* pPageDesc = ::GetPageDescByName_Impl(*pDoc, sDescName);
            if(pPageDesc)
            {
                pPageDesc->Add( pNewDesc );
                bPut = sal_True;
            }
            else
            {
                throw lang::IllegalArgumentException();
            }
        }
        if(!bPut)
        {
            rSet.ClearItem(RES_BREAK);
            rSet.Put(SwFmtPageDesc());
        }
        else
            rSet.Put(*pNewDesc);
    }
    delete pNewDesc;
}
/* -----------------30.06.98 10:29-------------------
 *
 * --------------------------------------------------*/
void lcl_SetNodeNumStart( SwPaM& rCrsr, uno::Any aValue )
{
    sal_Int16 nTmp = 1;
    aValue >>= nTmp;
    sal_uInt16 nStt = (nTmp < 0 ? USHRT_MAX : (sal_uInt16)nTmp);
    SwDoc* pDoc = rCrsr.GetDoc();
    UnoActionContext aAction(pDoc);

    if( rCrsr.GetNext() != &rCrsr )         // Mehrfachselektion ?
    {
        pDoc->StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( rCrsr );
        SwPaM aPam( *rCrsr.GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
        {
          pDoc->SetNumRuleStart(*aRangeArr.SetPam( n, aPam ).GetPoint());
          pDoc->SetNodeNumStart(*aRangeArr.SetPam( n, aPam ).GetPoint(),
                    nStt );
        }
        pDoc->EndUndo( UNDO_END, NULL);
    }
    else
    {
        pDoc->SetNumRuleStart( *rCrsr.GetPoint());
        pDoc->SetNodeNumStart( *rCrsr.GetPoint(), nStt );
    }
}

/* -----------------17.09.98 09:44-------------------
 *
 * --------------------------------------------------*/
sal_Bool lcl_setCrsrPropertyValue(const SfxItemPropertySimpleEntry* pEntry,
                                SwPaM& rPam,
                                SfxItemSet& rItemSet,
                                const uno::Any& aValue ) throw (lang::IllegalArgumentException)
{
    sal_Bool bRet = sal_True;
    if(0 == ( pEntry->nFlags&PropertyAttribute::MAYBEVOID ) &&
        aValue.getValueType() == ::getCppuVoidType())
        bRet = sal_False;
    else
    {
        switch(pEntry->nWID)
        {
            case RES_TXTATR_CHARFMT:
                lcl_setCharStyle(rPam.GetDoc(), aValue, rItemSet );
            break;
            case RES_TXTATR_AUTOFMT:
                lcl_setAutoStyle(rPam.GetDoc()->GetIStyleAccess(), aValue, rItemSet, false );
            break;
            case FN_UNO_CHARFMT_SEQUENCE:
            {
                Sequence<OUString> aCharStyles;
                if(aValue >>= aCharStyles)
                {
                    for(sal_Int32 nStyle = 0; nStyle < aCharStyles.getLength(); nStyle++)
                    {
                        Any aStyle;
                        rPam.GetDoc()->StartUndo( UNDO_START, NULL);
                        aStyle <<= aCharStyles.getConstArray()[nStyle];
                        //create a local set and apply each format directly
                        SfxItemSet aSet(rPam.GetDoc()->GetAttrPool(), RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT );
                        lcl_setCharStyle(rPam.GetDoc(), aStyle, aSet );
                        //the first style should replace the current attributes, all other have to be added
                        SwXTextCursor::SetCrsrAttr(rPam, aSet, nStyle ? CRSR_ATTR_MODE_DONTREPLACE : 0);
                        rPam.GetDoc()->EndUndo( UNDO_START, NULL );
                    }
                }
                else
                    bRet = sal_False;
            }
            break;
            case FN_UNO_PARA_STYLE :
                lcl_SetTxtFmtColl(aValue, rPam);
            break;
            case RES_AUTO_STYLE:
                lcl_setAutoStyle(rPam.GetDoc()->GetIStyleAccess(), aValue, rItemSet, true );
            break;
            case FN_UNO_PAGE_STYLE :
            break;
            case FN_UNO_NUM_START_VALUE  :
                lcl_SetNodeNumStart( rPam, aValue );
            break;
            case FN_UNO_NUM_LEVEL  :
            // --> OD 2008-07-14 #i91601#
            case FN_UNO_LIST_ID:
            // <--
            case FN_UNO_IS_NUMBER  :
            {
                // multi selection is not considered
                SwTxtNode* pTxtNd = rPam.GetNode()->GetTxtNode();
                // --> OD 2008-05-14 #refactorlists# - check on list style not needed
//                const SwNumRule* pRule = pTxtNd->GetNumRule();
//                if( FN_UNO_NUM_LEVEL == pEntry->nWID  &&  pRule != NULL )
                if ( FN_UNO_NUM_LEVEL == pEntry->nWID )
                // <--
                {
                    sal_Int16 nLevel = 0;
                    aValue >>= nLevel;

                    pTxtNd->SetAttrListLevel(nLevel);

                }
                // --> OD 2008-07-14 #i91601#
                else if( FN_UNO_LIST_ID == pEntry->nWID )
                {
                    ::rtl::OUString sListId;
                    aValue >>= sListId;
                    pTxtNd->SetListId( sListId );
                }
                // <--
                else if( FN_UNO_IS_NUMBER == pEntry->nWID )
                {
                    BOOL bIsNumber = *(sal_Bool*) aValue.getValue();
                    if(!bIsNumber)
                        pTxtNd->SetCountedInList( false );
                }
                //PROPERTY_MAYBEVOID!
            }
            break;
            case FN_NUMBER_NEWSTART :
            {
                sal_Bool bVal = sal_False;
                if (aValue >>= bVal)
                    rPam.GetDoc()->SetNumRuleStart(*rPam.GetPoint(), bVal);
                else
                    throw lang::IllegalArgumentException();
            }
            break;
            case FN_UNO_NUM_RULES:
                SwUnoCursorHelper::setNumberingProperty(aValue, rPam);
            break;
            case RES_PARATR_DROP:
            {
                if( MID_DROPCAP_CHAR_STYLE_NAME == pEntry->nMemberId)
                {
                    OUString uStyle;
                    if(aValue >>= uStyle)
                    {
                        String sStyle;
                        SwStyleNameMapper::FillUIName(uStyle, sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, sal_True );
                        SwDoc* pDoc = rPam.GetDoc();
                        //default character style mustn't be set as default format
                        SwDocStyleSheet* pStyle =
                            (SwDocStyleSheet*)pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR);
                        SwFmtDrop* pDrop = 0;
                        if(pStyle &&
                                ((SwDocStyleSheet*)pStyle)->GetCharFmt() != pDoc->GetDfltCharFmt())
                        {
                            const SfxPoolItem* pItem;
                            if(SFX_ITEM_SET == rItemSet.GetItemState( RES_PARATR_DROP, sal_True, &pItem ) )
                                pDrop = new SwFmtDrop(*((SwFmtDrop*)pItem));
                            if(!pDrop)
                                pDrop = new SwFmtDrop();
                            rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *(SwDocStyleSheet*)pStyle ) );
                            pDrop->SetCharFmt(xStyle->GetCharFmt());
                        }
                        else
                             throw lang::IllegalArgumentException();
                        rItemSet.Put(*pDrop);
                        delete pDrop;
                    }
                    else
                        throw lang::IllegalArgumentException();
                }
                else
                    bRet = sal_False;
            }
            break;
            case RES_TXTATR_CJK_RUBY:
                if(MID_RUBY_CHARSTYLE == pEntry->nMemberId )
                {
                    OUString sTmp;
                    if(aValue >>= sTmp)
                    {
                        SwFmtRuby* pRuby = 0;
                        const SfxPoolItem* pItem;
                        if(SFX_ITEM_SET == rItemSet.GetItemState( RES_TXTATR_CJK_RUBY, sal_True, &pItem ) )
                            pRuby = new SwFmtRuby(*((SwFmtRuby*)pItem));
                        if(!pRuby)
                            pRuby = new SwFmtRuby(aEmptyStr);
                        String sStyle;
                        SwStyleNameMapper::FillUIName(sTmp, sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, sal_True );
                         pRuby->SetCharFmtName( sStyle );
                        pRuby->SetCharFmtId( 0 );
                        if(sStyle.Len() > 0)
                        {
                            sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
                            pRuby->SetCharFmtId(nId);
                        }
                        rItemSet.Put(*pRuby);
                        delete pRuby;
                    }
                    else
                        throw lang::IllegalArgumentException();
                    bRet = sal_True;
                }
                else
                    bRet = sal_False;
            break;
            case RES_PAGEDESC      :
            if(MID_PAGEDESC_PAGEDESCNAME == pEntry->nMemberId )
            {
                lcl_setPageDesc(rPam.GetDoc(), aValue, rItemSet);
                break;
            }
            //hier kein break
            default: bRet = sal_False;
        }
    }
return bRet;
}

/* -----------------30.06.98 08:39-------------------
 *
 * --------------------------------------------------*/
SwFmtColl* SwXTextCursor::GetCurTxtFmtColl(SwPaM& rPaM, BOOL bConditional)
{
    static const sal_uInt16 nMaxLookup = 1000;
    SwFmtColl *pFmt = 0;

//  if ( GetCrsrCnt() > nMaxLookup )
//      return 0;
    sal_Bool bError = sal_False;
    SwPaM *pTmpCrsr = &rPaM;
    do {

        ULONG nSttNd = pTmpCrsr->GetMark()->nNode.GetIndex();
        ULONG nEndNd = pTmpCrsr->GetPoint()->nNode.GetIndex();
        xub_StrLen nSttCnt = pTmpCrsr->GetMark()->nContent.GetIndex();
        xub_StrLen nEndCnt = pTmpCrsr->GetPoint()->nContent.GetIndex();

        if( nSttNd > nEndNd || ( nSttNd == nEndNd && nSttCnt > nEndCnt ))
        {
            sal_uInt32 nTmp = nSttNd; nSttNd = nEndNd; nEndNd = nTmp;
            nTmp = nSttCnt; nSttCnt = nEndCnt; nEndCnt = (sal_uInt16)nTmp;
        }

        if( nEndNd - nSttNd >= nMaxLookup )
        {
            pFmt = 0;
            break;
        }

        const SwNodes& rNds = rPaM.GetDoc()->GetNodes();
        for( ULONG n = nSttNd; n <= nEndNd; ++n )
        {
            const SwTxtNode* pNd = rNds[ n ]->GetTxtNode();
            if( pNd )
            {
                SwFmtColl* pNdFmt = bConditional ? pNd->GetFmtColl()
                                                    : &pNd->GetAnyFmtColl();
                if( !pFmt )
                    pFmt = pNdFmt;
                else if( pFmt != pNdFmt )
                {
                    bError = sal_True;
                    break;
                }
            }
        }
        if(bError)
            break;

        pTmpCrsr = static_cast<SwPaM*>(pTmpCrsr->GetNext());
    } while ( pTmpCrsr != &rPaM );
    return bError ? 0 : pFmt;
}

/* -----------------26.06.98 16:20-------------------
 *  Hilfsfunktion fuer PageDesc
 * --------------------------------------------------*/
 SwPageDesc*    GetPageDescByName_Impl(SwDoc& rDoc, const String& rName)
 {
    SwPageDesc* pRet = 0;
    sal_uInt16 nDCount = rDoc.GetPageDescCnt();
    sal_uInt16 i;

    for( i = 0; i < nDCount; i++ )
    {
        SwPageDesc* pDsc = &rDoc._GetPageDesc( i );
        if(pDsc->GetName() == rName)
        {
            pRet = pDsc;
            break;
        }
    }
    if(!pRet)
    {
        for(i = RC_POOLPAGEDESC_BEGIN; i <= STR_POOLPAGE_LANDSCAPE; ++i)
        {
            const String aFmtName(SW_RES(i));
            if(aFmtName == rName)
            {
                pRet = rDoc.GetPageDescFromPool( static_cast< sal_uInt16 >(RES_POOLPAGE_BEGIN + i - RC_POOLPAGEDESC_BEGIN) );
                break;
            }
        }
    }

    return pRet;
 }
/******************************************************************************
 ******************************************************************************
 ******************************************************************************
 * Taskforce ONE51
 ******************************************************************************/

/******************************************************************
 * SwXTextCursor
 ******************************************************************/

/*-- 09.12.98 14:19:00---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_STATIC_LINK( SwXTextCursor, RemoveCursor_Impl,
                  uno::Reference< XInterface >*, EMPTYARG )
{
    ASSERT( pThis != NULL, "no reference?" );
    //ASSERT( pArg != NULL, "no reference?" );

    // --> FME 2006-03-07 #126177# Tell the SwXTextCursor that the user event
    // has been executed. It is not necessary to remove the user event in
    // ~SwXTextCursor
    pThis->DoNotRemoveUserEvent();
    // <--

    SwUnoCrsr* pCursor = pThis->GetCrsr();
    if( pCursor != NULL )
    {
        pCursor->Remove( pThis );
        delete pCursor;
    }

    // --> FME 2006-03-07 #126177#
    //delete pArg;
    // <--

    return 0;
}

void    SwXTextCursor::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);

    // if the cursor leaves its designated section, it becomes invalid
    if( !mbRemoveUserEvent && ( pOld != NULL ) && ( pOld->Which() == RES_UNOCURSOR_LEAVES_SECTION ) )
    {
        // --> FME 2006-03-07 #126177# We don't need to create a reference
        // to the SwXTextCursor to prevent its deletion. If the destructor
        // of the SwXTextCursor is called before the user event is executed,
        // the user event will be removed. This is necessary, because an other
        // thread might be currently waiting in ~SwXTextCursor. In this case
        // the pRef = new ... stuff did not work!

        // create reference to this object to prevent deletion before
        // the STATIC_LINK is executed. The link will delete the
        // reference.
        //uno::Reference<XInterface>* pRef =
            //new uno::Reference<XInterface>( static_cast<XServiceInfo*>( this ) );

        mbRemoveUserEvent = true;
        // <--

        mnUserEventId = Application::PostUserEvent(
                        STATIC_LINK( this, SwXTextCursor, RemoveCursor_Impl ), this );
    }

    if(!GetRegisteredIn())
        aLstnrCntnr.Disposing();

}

/*-- 09.12.98 14:19:01---------------------------------------------------

  -----------------------------------------------------------------------*/
const SwPaM*    SwXTextCursor::GetPaM() const
{
    return GetCrsr() ? GetCrsr() : 0;
}

SwPaM*  SwXTextCursor::GetPaM()
{
    return GetCrsr() ? GetCrsr() : 0;
}

/*-- 09.12.98 14:19:02---------------------------------------------------

  -----------------------------------------------------------------------*/
const SwDoc* SwXTextCursor::GetDoc()const
{
    return   GetCrsr() ? GetCrsr()->GetDoc() : 0;
}
/* -----------------22.07.99 13:52-------------------

 --------------------------------------------------*/
SwDoc* SwXTextCursor::GetDoc()
{
    return   GetCrsr() ? GetCrsr()->GetDoc() : 0;
}


/*-- 09.12.98 14:19:19---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextCursor::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextCursor");
}
/*-- 09.12.98 14:19:19---------------------------------------------------

  -----------------------------------------------------------------------*/
BOOL SwXTextCursor::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    String sServiceName(rServiceName);
    return sServiceName.EqualsAscii("com.sun.star.text.TextCursor") ||
         sServiceName.EqualsAscii("com.sun.star.style.CharacterProperties")||
        sServiceName.EqualsAscii("com.sun.star.style.CharacterPropertiesAsian")||
        sServiceName.EqualsAscii("com.sun.star.style.CharacterPropertiesComplex")||
        sServiceName.EqualsAscii("com.sun.star.style.ParagraphProperties") ||
        sServiceName.EqualsAscii("com.sun.star.style.ParagraphPropertiesAsian") ||
        sServiceName.EqualsAscii("com.sun.star.style.ParagraphPropertiesComplex") ||
        sServiceName.EqualsAscii("com.sun.star.text.TextSortable");
}
/*-- 09.12.98 14:19:19---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXTextCursor::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(8);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextCursor");
     pArray[1] = C2U("com.sun.star.style.CharacterProperties");
    pArray[2] = C2U("com.sun.star.style.CharacterPropertiesAsian");
    pArray[3] = C2U("com.sun.star.style.CharacterPropertiesComplex");
    pArray[4] = C2U("com.sun.star.style.ParagraphProperties");
    pArray[5] = C2U("com.sun.star.style.ParagraphPropertiesAsian");
    pArray[6] = C2U("com.sun.star.style.ParagraphPropertiesComplex");
    pArray[7] = C2U("com.sun.star.text.TextSortable");
    return aRet;
}
/*-- 09.12.98 14:19:19---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextCursor::SwXTextCursor(uno::Reference< XText >  xParent, const SwPosition& rPos,
                    CursorType eSet, SwDoc* pDoc, const SwPosition* pMark) :
    aLstnrCntnr(( util::XSortable*)this),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR)),
    xParentText(xParent),
    pLastSortOptions(0),
    eType(eSet),
    mbRemoveUserEvent( false )
{
    SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(rPos, sal_False);
    if(pMark)
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *pMark;
    }
    pUnoCrsr->Add(this);
}

/* -----------------04.03.99 09:02-------------------
 *
 * --------------------------------------------------*/
SwXTextCursor::SwXTextCursor(uno::Reference< XText >  xParent,
    SwUnoCrsr* pSourceCrsr, CursorType eSet) :
    aLstnrCntnr( (util::XSortable*)this),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR)),
    xParentText(xParent),
    pLastSortOptions(0),
    eType(eSet),
    mbRemoveUserEvent( false )
{
    SwUnoCrsr* pUnoCrsr = pSourceCrsr->GetDoc()->CreateUnoCrsr(*pSourceCrsr->GetPoint(), sal_False);
    if(pSourceCrsr->HasMark())
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *pSourceCrsr->GetMark();
    }
    pUnoCrsr->Add(this);
}

/*-- 09.12.98 14:19:20---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextCursor::~SwXTextCursor()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // --> FME 2006-03-07 #126177# A user event has been posted in order
    // to delete the SwUnoCursor. Remove the user event if the SwXTextCursor
    // is destroyed before the user event could be handled.
    if ( mbRemoveUserEvent )
    {
        mbRemoveUserEvent = false;
        Application::RemoveUserEvent( mnUserEventId );
    }
    // <--

    SwUnoCrsr* pUnoCrsr = GetCrsr();
    delete pUnoCrsr;
    delete pLastSortOptions;
}
/*-- 09.12.98 14:19:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::DeleteAndInsert(const String& rText,
        const bool bForceExpandHints)
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        // Start/EndAction
        SwDoc* pDoc = pUnoCrsr->GetDoc();
        UnoActionContext aAction(pDoc);
        xub_StrLen nTxtLen = rText.Len();
        pDoc->StartUndo(UNDO_INSERT, NULL);
        SwCursor *_pStartCrsr = pUnoCrsr;
        do
        {
            if(_pStartCrsr->HasMark())
            {
                pDoc->DeleteAndJoin(*_pStartCrsr);
            }
            if(nTxtLen)
            {
                const bool bSuccess(
                    SwUnoCursorHelper::DocInsertStringSplitCR(
                        *pDoc, *_pStartCrsr, rText, bForceExpandHints ) );
                DBG_ASSERT( bSuccess, "Doc->Insert(Str) failed." );
                (void) bSuccess;

                SwXTextCursor::SelectPam(*pUnoCrsr, sal_True);
                _pStartCrsr->Left(rText.Len(), CRSR_SKIP_CHARS, FALSE, FALSE);
            }
        } while( (_pStartCrsr=(SwCursor*)_pStartCrsr->GetNext()) != pUnoCrsr );
        pDoc->EndUndo(UNDO_INSERT, NULL);
    }
}


enum ForceIntoMetaMode { META_CHECK_BOTH, META_INIT_START, META_INIT_END };

sal_Bool lcl_ForceIntoMeta(SwPaM & rCursor,
        uno::Reference<text::XText> const & xParentText,
        const enum ForceIntoMetaMode eMode)
{
    sal_Bool bRet( sal_True ); // means not forced in META_CHECK_BOTH
    SwXMeta const * const pXMeta( dynamic_cast<SwXMeta*>(xParentText.get()) );
    ASSERT(pXMeta, "no parent?");
    if (!pXMeta)
        throw uno::RuntimeException();
    SwTxtNode * pTxtNode;
    xub_StrLen nStart;
    xub_StrLen nEnd;
    const bool bSuccess( pXMeta->SetContentRange(pTxtNode, nStart, nEnd) );
    ASSERT(bSuccess, "no pam?");
    if (!bSuccess)
        throw uno::RuntimeException();
    // force the cursor back into the meta if it has moved outside
    SwPosition start(*pTxtNode, nStart);
    SwPosition end(*pTxtNode, nEnd);
    switch (eMode)
    {
        case META_INIT_START:
            *rCursor.GetPoint() = start;
            break;
        case META_INIT_END:
            *rCursor.GetPoint() = end;
            break;
        case META_CHECK_BOTH:
            if (*rCursor.Start() < start)
            {
                *rCursor.Start() = start;
                bRet = sal_False;
            }
            if (*rCursor.End() > end)
            {
                *rCursor.End() = end;
                bRet = sal_False;
            }
            break;
    }
    return bRet;
}

bool SwXTextCursor::IsAtEndOfMeta() const
{
    if (CURSOR_META == eType)
    {
        SwUnoCrsr const * const pCursor( GetCrsr() );
        SwXMeta const*const pXMeta( dynamic_cast<SwXMeta*>(xParentText.get()) );
        ASSERT(pXMeta, "no meta?");
        if (pCursor && pXMeta)
        {
            SwTxtNode * pTxtNode;
            xub_StrLen nStart;
            xub_StrLen nEnd;
            const bool bSuccess(
                    pXMeta->SetContentRange(pTxtNode, nStart, nEnd) );
            ASSERT(bSuccess, "no pam?");
            if (bSuccess)
            {
                const SwPosition end(*pTxtNode, nEnd);
                if (   (*pCursor->GetPoint() == end)
                    || (*pCursor->GetMark()  == end))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextCursor::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextCursor::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return OTextCursorHelper::getSomething(rId);
}

/*-- 09.12.98 14:18:12---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::collapseToStart(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if(pUnoCrsr->HasMark())
        {
            if(*pUnoCrsr->GetPoint() > *pUnoCrsr->GetMark())
                pUnoCrsr->Exchange();
            pUnoCrsr->DeleteMark();
        }
    }
    else
    {
        throw uno::RuntimeException();
    }
}
/*-- 09.12.98 14:18:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::collapseToEnd(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if(pUnoCrsr->HasMark())
        {
            if(*pUnoCrsr->GetPoint() < *pUnoCrsr->GetMark())
                pUnoCrsr->Exchange();
            pUnoCrsr->DeleteMark();
        }
    }
    else
    {
        throw uno::RuntimeException();
    }
}
/*-- 09.12.98 14:18:41---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::isCollapsed(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_True;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr && pUnoCrsr->GetMark())
    {
        bRet = (*pUnoCrsr->GetPoint() == *pUnoCrsr->GetMark());
    }
    return bRet;
}
/*-- 09.12.98 14:18:42---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::goLeft(sal_Int16 nCount, sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        bRet = pUnoCrsr->Left( nCount, CRSR_SKIP_CHARS, FALSE, FALSE);
        if (CURSOR_META == eType)
        {
            bRet = lcl_ForceIntoMeta(*pUnoCrsr, xParentText, META_CHECK_BOTH)
                && bRet;
        }
    }
    else
    {
        throw uno::RuntimeException();
    }
    return bRet;
}
/*-- 09.12.98 14:18:42---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::goRight(sal_Int16 nCount, sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        bRet = pUnoCrsr->Right(nCount, CRSR_SKIP_CHARS, FALSE, FALSE);
        if (CURSOR_META == eType)
        {
            bRet = lcl_ForceIntoMeta(*pUnoCrsr, xParentText, META_CHECK_BOTH)
                && bRet;
        }
    }
    else
    {
        throw uno::RuntimeException();
    }
    return bRet;
}
/*-- 09.12.98 14:18:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::gotoStart(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        if(eType == CURSOR_BODY)
        {
            pUnoCrsr->Move( fnMoveBackward, fnGoDoc );
            //check, that the cursor is not in a table
            SwTableNode* pTblNode = pUnoCrsr->GetNode()->FindTableNode();
            SwCntntNode* pCont = 0;
            while( pTblNode )
            {
                pUnoCrsr->GetPoint()->nNode = *pTblNode->EndOfSectionNode();
                pCont = GetDoc()->GetNodes().GoNext(&pUnoCrsr->GetPoint()->nNode);
                pTblNode = pCont ? pCont->FindTableNode() : 0;
            }
            if(pCont)
                pUnoCrsr->GetPoint()->nContent.Assign(pCont, 0);
            const SwStartNode* pTmp = pUnoCrsr->GetNode()->StartOfSectionNode();
            if(pTmp->IsSectionNode())
            {
                SwSectionNode* pSectionStartNode = (SwSectionNode*)pTmp;
                if(pSectionStartNode->GetSection().IsHiddenFlag())
                {
                    pCont = GetDoc()->GetNodes().GoNextSection(
                                &pUnoCrsr->GetPoint()->nNode, sal_True, sal_False);
                    if(pCont)
                        pUnoCrsr->GetPoint()->nContent.Assign(pCont, 0);
                }
            }
        }
        else if(eType == CURSOR_FRAME ||
                eType == CURSOR_TBLTEXT ||
                eType == CURSOR_HEADER ||
                eType == CURSOR_FOOTER ||
                eType == CURSOR_FOOTNOTE||
                eType == CURSOR_REDLINE)
        {
            pUnoCrsr->MoveSection( fnSectionCurr, fnSectionStart);
        }
        else if (CURSOR_META == eType)
        {
            lcl_ForceIntoMeta(*pUnoCrsr, xParentText, META_INIT_START);
        }
    }
    else
    {
        throw uno::RuntimeException();
    }
}
/*-- 09.12.98 14:18:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::gotoEnd(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        if(eType == CURSOR_BODY)
        {
            pUnoCrsr->Move( fnMoveForward, fnGoDoc );
        }
        else if(eType == CURSOR_FRAME ||
                eType == CURSOR_TBLTEXT ||
                eType == CURSOR_HEADER ||
                eType == CURSOR_FOOTER ||
                eType == CURSOR_FOOTNOTE||
                eType == CURSOR_REDLINE)
        {
            pUnoCrsr->MoveSection( fnSectionCurr, fnSectionEnd);
        }
        else if (CURSOR_META == eType)
        {
            lcl_ForceIntoMeta(*pUnoCrsr, xParentText, META_INIT_END);
        }
    }
    else
    {
        throw uno::RuntimeException();
    }
}

void SwXTextCursor::gotoRange(const uno::Reference< XTextRange > & xRange, sal_Bool bExpand )
                                throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pOwnCursor = GetCrsr();
    if(!pOwnCursor || !xRange.is())
    {
        throw uno::RuntimeException();
    }

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange  = reinterpret_cast< SwXTextRange * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
        pCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
    }

    if (!pRange && !pCursor)
    {
        throw uno::RuntimeException();
    }

    SwStartNodeType eSearchNodeType = SwNormalStartNode;
    switch(eType)
    {
        case CURSOR_FRAME:      eSearchNodeType = SwFlyStartNode;       break;
        case CURSOR_TBLTEXT:    eSearchNodeType = SwTableBoxStartNode;  break;
        case CURSOR_FOOTNOTE:   eSearchNodeType = SwFootnoteStartNode;  break;
        case CURSOR_HEADER:     eSearchNodeType = SwHeaderStartNode;    break;
        case CURSOR_FOOTER:     eSearchNodeType = SwFooterStartNode;    break;
        //case CURSOR_INVALID:
        //case CURSOR_BODY:
        default:
            ;
    }
    const SwStartNode* pOwnStartNode = pOwnCursor->GetNode()->
                                            FindSttNodeByType(eSearchNodeType);

    SwPaM aPam(GetDoc()->GetNodes());
    const SwPaM * pPam(0);
    if (pCursor)
    {
        pPam = pCursor->GetPaM();
    }
    else if (pRange)
    {
        if (pRange->GetPositions(aPam))
        {
            pPam = & aPam;
        }
    }

    if (!pPam)
    {
        throw uno::RuntimeException();
    }
    const SwStartNode* pTmp =
        pPam->GetNode()->FindSttNodeByType(eSearchNodeType);

    //SectionNodes ueberspringen
    while(pTmp && pTmp->IsSectionNode())
    {
        pTmp = pTmp->StartOfSectionNode();
    }
    while(pOwnStartNode && pOwnStartNode->IsSectionNode())
    {
        pOwnStartNode = pOwnStartNode->StartOfSectionNode();
    }
    if(pOwnStartNode != pTmp)
    {
        throw uno::RuntimeException();
    }

    if (CURSOR_META == eType)
    {
        SwPaM CopyPam(*pPam->GetMark(), *pPam->GetPoint());
        const bool bNotForced(
                lcl_ForceIntoMeta(CopyPam, xParentText, META_CHECK_BOTH) );
        if (!bNotForced)
        {
            throw uno::RuntimeException(
                C2U("gotoRange: parameter range not contained in nesting"
                    " text content for which this cursor was created"),
                static_cast<text::XWordCursor*>(this));
        }
    }

    //jetzt muss die Selektion erweitert werden
    if(bExpand)
    {
        // der Cursor soll alles einschliessen, was bisher von ihm und dem uebergebenen
        // Range eingeschlossen wurde
        SwPosition aOwnLeft(*pOwnCursor->GetPoint());
        SwPosition aOwnRight(pOwnCursor->HasMark() ? *pOwnCursor->GetMark() : aOwnLeft);
        if(aOwnRight < aOwnLeft)
        {
            SwPosition aTmp = aOwnLeft;
            aOwnLeft = aOwnRight;
            aOwnRight = aTmp;
        }
        SwPosition const* pParamLeft  = pPam->Start();
        SwPosition const* pParamRight = pPam->End();

        // jetzt sind vier SwPositions da, zwei davon werden gebraucht, also welche?
        if(aOwnRight > *pParamRight)
            *pOwnCursor->GetPoint() = aOwnRight;
        else
            *pOwnCursor->GetPoint() = *pParamRight;
        pOwnCursor->SetMark();
        if(aOwnLeft < *pParamLeft)
            *pOwnCursor->GetMark() = aOwnLeft;
        else
            *pOwnCursor->GetMark() = *pParamLeft;
    }
    else
    {
        // cursor should be the given range
        *pOwnCursor->GetPoint() = *pPam->GetPoint();
        if (pPam->HasMark())
        {
            pOwnCursor->SetMark();
            *pOwnCursor->GetMark() = *pPam->GetMark();
        }
        else
        {
            pOwnCursor->DeleteMark();
        }
    }
}

/*-- 09.12.98 14:18:44---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::isStartOfWord(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        bRet = pUnoCrsr->IsStartWordWT( i18n::WordType::DICTIONARY_WORD );
    }
    else
    {
        throw uno::RuntimeException();
    }
    return bRet;
}
/*-- 09.12.98 14:18:44---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::isEndOfWord(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        bRet = pUnoCrsr->IsEndWordWT( i18n::WordType::DICTIONARY_WORD );
    }
    else
    {
        throw uno::RuntimeException();
    }
    return bRet;
}
/*-- 09.12.98 14:18:44---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::gotoNextWord(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    //Probleme gibt's noch mit einem Absatzanfang, an dem kein Wort beginnt.

    SwUnoCrsr* pUnoCrsr = GetCrsr();

    sal_Bool bRet = sal_False;
    if(pUnoCrsr)
    {
        // remember old position to check if cursor has moved
        // since the called functions are sometimes a bit unreliable
        // in specific cases...
        SwPosition  *pPoint     = pUnoCrsr->GetPoint();
        SwNode      *pOldNode   = &pPoint->nNode.GetNode();
        xub_StrLen   nOldIndex  = pPoint->nContent.GetIndex();

        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        //Absatzende?
        if(pUnoCrsr->GetCntntNode() &&
                pPoint->nContent == pUnoCrsr->GetCntntNode()->Len())
        {
            pUnoCrsr->Right(1, CRSR_SKIP_CHARS, FALSE, FALSE);
        }
        else
        {
            sal_Bool bTmp = pUnoCrsr->GoNextWordWT( i18n::WordType::DICTIONARY_WORD );
            //if there is no next word within the current paragraph try to go to the start of the next paragraph
            if(!bTmp)
                pUnoCrsr->MovePara(fnParaNext, fnParaStart);
        }

        // return true if cursor has moved
        bRet = &pPoint->nNode.GetNode() != pOldNode  ||
                pPoint->nContent.GetIndex() != nOldIndex;
        if (bRet && (CURSOR_META == eType))
        {
            bRet = lcl_ForceIntoMeta(*pUnoCrsr, xParentText, META_CHECK_BOTH);
        }
    }
    else
    {
        throw uno::RuntimeException();
    }

    return bRet;
}
/*-- 09.12.98 14:18:45---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::gotoPreviousWord(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    // hier machen Leerzeichen am Absatzanfang Probleme
    SwUnoCrsr* pUnoCrsr = GetCrsr();

    sal_Bool bRet = sal_False;
    if(pUnoCrsr)
    {
        SwPosition  *pPoint     = pUnoCrsr->GetPoint();
        SwNode      *pOldNode   = &pPoint->nNode.GetNode();
        xub_StrLen   nOldIndex  = pPoint->nContent.GetIndex();

        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        //Absatzanfang ?
        if(pPoint->nContent == 0)
        {
            pUnoCrsr->Left(1, CRSR_SKIP_CHARS, FALSE, FALSE);
        }
        else
        {
            pUnoCrsr->GoPrevWordWT( i18n::WordType::DICTIONARY_WORD );
            if(pPoint->nContent == 0)
                pUnoCrsr->Left(1, CRSR_SKIP_CHARS, FALSE, FALSE);
        }

        // return true if cursor has moved
        bRet = &pPoint->nNode.GetNode() != pOldNode  ||
                pPoint->nContent.GetIndex() != nOldIndex;
        if (bRet && (CURSOR_META == eType))
        {
            bRet = lcl_ForceIntoMeta(*pUnoCrsr, xParentText, META_CHECK_BOTH);
        }
    }
    else
    {
        throw uno::RuntimeException();
    }

    return bRet;
}
/*-- 09.12.98 14:18:45---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::gotoEndOfWord(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();

    sal_Bool bRet = sal_False;
    if(pUnoCrsr)
    {
        SwPosition  *pPoint     = pUnoCrsr->GetPoint();
        SwNode      &rOldNode   = pPoint->nNode.GetNode();
        xub_StrLen   nOldIndex  = pPoint->nContent.GetIndex();

        sal_Int16 nWordType = i18n::WordType::DICTIONARY_WORD;
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        if(!pUnoCrsr->IsEndWordWT( nWordType ))
            pUnoCrsr->GoEndWordWT( nWordType );

        // restore old cursor if we are not at the end of a word by now
        // otherwise use current one
        bRet = pUnoCrsr->IsEndWordWT( nWordType );
        if (!bRet)
        {
            pPoint->nNode       = rOldNode;
            pPoint->nContent    = nOldIndex;
        }
        else if (CURSOR_META == eType)
        {
            bRet = lcl_ForceIntoMeta(*pUnoCrsr, xParentText, META_CHECK_BOTH);
        }
    }
    else
    {
        throw uno::RuntimeException();
    }

    return bRet;
}
/*-- 09.12.98 14:18:46---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::gotoStartOfWord(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();

    sal_Bool bRet = sal_False;
    if(pUnoCrsr)
    {
        SwPosition  *pPoint     = pUnoCrsr->GetPoint();
        SwNode      &rOldNode   = pPoint->nNode.GetNode();
        xub_StrLen   nOldIndex  = pPoint->nContent.GetIndex();

        sal_Int16 nWordType = i18n::WordType::DICTIONARY_WORD;
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        if(!pUnoCrsr->IsStartWordWT( nWordType ))
            pUnoCrsr->GoStartWordWT( nWordType );

        // restore old cursor if we are not at the start of a word by now
        // otherwise use current one
        bRet = pUnoCrsr->IsStartWordWT( nWordType );
        if (!bRet)
        {
            pPoint->nNode       = rOldNode;
            pPoint->nContent    = nOldIndex;
        }
        else if (CURSOR_META == eType)
        {
            bRet = lcl_ForceIntoMeta(*pUnoCrsr, xParentText, META_CHECK_BOTH);
        }
    }
    else
    {
        throw uno::RuntimeException();
    }

    return bRet;
}
/*-- 09.12.98 14:18:46---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::isStartOfSentence(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        // start of paragraph?
        bRet = pUnoCrsr->GetPoint()->nContent == 0;
        // with mark ->no sentence start
        // (check if cursor is no selection, i.e. it does not have
        // a mark or else point and mark are identical)
        if(!bRet && (!pUnoCrsr->HasMark() ||
                        *pUnoCrsr->GetPoint() == *pUnoCrsr->GetMark()))
        {
            SwCursor aCrsr(*pUnoCrsr->GetPoint(),0,false);
            SwPosition aOrigPos = *aCrsr.GetPoint();
            aCrsr.GoSentence(SwCursor::START_SENT );
            bRet = aOrigPos == *aCrsr.GetPoint();
        }
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 09.12.98 14:18:47---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::isEndOfSentence(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        //end of paragraph?
        bRet = pUnoCrsr->GetCntntNode() &&
                pUnoCrsr->GetPoint()->nContent == pUnoCrsr->GetCntntNode()->Len();
        // with mark->no sentence end
        // (check if cursor is no selection, i.e. it does not have
        // a mark or else point and mark are identical)
        if(!bRet && (!pUnoCrsr->HasMark() ||
                        *pUnoCrsr->GetPoint() == *pUnoCrsr->GetMark()))
        {
            SwCursor aCrsr(*pUnoCrsr->GetPoint(),0,false);
            SwPosition aOrigPos = *aCrsr.GetPoint();
            aCrsr.GoSentence(SwCursor::END_SENT );
            bRet = aOrigPos == *aCrsr.GetPoint();
        }
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 09.12.98 14:18:47---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::gotoNextSentence(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        BOOL bWasEOS = isEndOfSentence();
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        bRet = pUnoCrsr->GoSentence(SwCursor::NEXT_SENT);
        if(!bRet)
            bRet = pUnoCrsr->MovePara(fnParaNext, fnParaStart);

        // if at the end of the sentence (i.e. at the space after the '.')
        // advance to next word in order for GoSentence to work properly
        // next time and have isStartOfSentence return true after this call
        if (!pUnoCrsr->IsStartWord())
        {
            BOOL bNextWord = pUnoCrsr->GoNextWord();
            if (bWasEOS && !bNextWord)
                bRet = sal_False;
        }
        if (CURSOR_META == eType)
        {
            bRet = lcl_ForceIntoMeta(*pUnoCrsr, xParentText, META_CHECK_BOTH)
                && bRet;
        }
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 09.12.98 14:18:47---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::gotoPreviousSentence(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        bRet = pUnoCrsr->GoSentence(SwCursor::PREV_SENT);
        if(!bRet)
        {
            if(0 != (bRet = pUnoCrsr->MovePara(fnParaPrev, fnParaStart)))
            {
                pUnoCrsr->MovePara(fnParaCurr, fnParaEnd);
                //at the end of a paragraph move to the sentence end again
                //
                pUnoCrsr->GoSentence(SwCursor::PREV_SENT);
            }
        }
        if (CURSOR_META == eType)
        {
            bRet = lcl_ForceIntoMeta(*pUnoCrsr, xParentText, META_CHECK_BOTH)
                && bRet;
        }
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/* -----------------15.10.99 08:24-------------------

 --------------------------------------------------*/
sal_Bool SwXTextCursor::gotoStartOfSentence(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        // if we're at the para start then we wont move
        // but bRet is also true if GoSentence failed but
        // the start of the sentence is reached
        bRet = SwUnoCursorHelper::IsStartOfPara(*pUnoCrsr)
            || pUnoCrsr->GoSentence(SwCursor::START_SENT) ||
            SwUnoCursorHelper::IsStartOfPara(*pUnoCrsr);
        if (CURSOR_META == eType)
        {
            bRet = lcl_ForceIntoMeta(*pUnoCrsr, xParentText, META_CHECK_BOTH)
                && bRet;
        }
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/* -----------------15.10.99 08:24-------------------

 --------------------------------------------------*/
sal_Bool SwXTextCursor::gotoEndOfSentence(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        // bRet is true if GoSentence() succeeded or if the
        // MovePara() succeeded while the end of the para is
        // not reached already
        sal_Bool bAlreadyParaEnd = SwUnoCursorHelper::IsEndOfPara(*pUnoCrsr);
        bRet = !bAlreadyParaEnd &&
                    (pUnoCrsr->GoSentence(SwCursor::END_SENT) ||
                        pUnoCrsr->MovePara(fnParaCurr, fnParaEnd));
        if (CURSOR_META == eType)
        {
            bRet = lcl_ForceIntoMeta(*pUnoCrsr, xParentText, META_CHECK_BOTH)
                && bRet;
        }
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

/*-- 09.12.98 14:18:48---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::isStartOfParagraph(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        bRet = SwUnoCursorHelper::IsStartOfPara(*pUnoCrsr);
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 09.12.98 14:18:48---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::isEndOfParagraph(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        bRet = SwUnoCursorHelper::IsEndOfPara(*pUnoCrsr);
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 09.12.98 14:18:49---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::gotoStartOfParagraph(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if (CURSOR_META == eType)
        return bRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr )
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        bRet = SwUnoCursorHelper::IsStartOfPara(*pUnoCrsr);
        if(!bRet)
            bRet = pUnoCrsr->MovePara(fnParaCurr, fnParaStart);
    }
    else
        throw uno::RuntimeException();

    // since MovePara(fnParaCurr, fnParaStart) only returns false
    // if we were already at the start of the paragraph this function
    // should always complete successfully.
    DBG_ASSERT( bRet, "gotoStartOfParagraph failed" );
    return bRet;
}
/*-- 09.12.98 14:18:49---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::gotoEndOfParagraph(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if (CURSOR_META == eType)
        return bRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        bRet = SwUnoCursorHelper::IsEndOfPara(*pUnoCrsr);
        if(!bRet)
            bRet = pUnoCrsr->MovePara(fnParaCurr, fnParaEnd);
    }
    else
        throw uno::RuntimeException();

    // since MovePara(fnParaCurr, fnParaEnd) only returns false
    // if we were already at the end of the paragraph this function
    // should always complete successfully.
    DBG_ASSERT( bRet, "gotoEndOfParagraph failed" );
    return bRet;
}
/*-- 09.12.98 14:18:50---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::gotoNextParagraph(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if (CURSOR_META == eType)
        return bRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        bRet = pUnoCrsr->MovePara(fnParaNext, fnParaStart);
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 09.12.98 14:18:50---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::gotoPreviousParagraph(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if (CURSOR_META == eType)
        return bRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        bRet = pUnoCrsr->MovePara(fnParaPrev, fnParaStart);
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 09.12.98 14:18:50---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XText >  SwXTextCursor::getText(void) throw( uno::RuntimeException )
{
    return xParentText;
}
/*-- 09.12.98 14:18:50---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextCursor::getStart(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = ((SwXTextCursor*)this)->GetCrsr();
    if( pUnoCrsr)
    {
        SwPaM aPam(*pUnoCrsr->Start());
        uno::Reference< XText >  xParent = getText();
        if (CURSOR_META == eType)
        {
            // return cursor to prevent modifying SwXTextRange for META
            SwXTextCursor * const pCursor(
                new SwXTextCursor(xParent, *pUnoCrsr->GetPoint(),
                    CURSOR_META, pUnoCrsr->GetDoc()) );
            pCursor->gotoStart(sal_False);
            xRet = static_cast<text::XWordCursor*>(pCursor);
        }
        else
        {
            xRet = new SwXTextRange(aPam, xParent);
        }
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 09.12.98 14:18:51---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextCursor::getEnd(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = ((SwXTextCursor*)this)->GetCrsr();
    if( pUnoCrsr)
    {
        SwPaM aPam(*pUnoCrsr->End());
        uno::Reference< XText >  xParent = getText();
        if (CURSOR_META == eType)
        {
            // return cursor to prevent modifying SwXTextRange for META
            SwXTextCursor * const pCursor(
                new SwXTextCursor(xParent, *pUnoCrsr->GetPoint(),
                    CURSOR_META, pUnoCrsr->GetDoc()) );
            pCursor->gotoEnd(sal_False);
            xRet = static_cast<text::XWordCursor*>(pCursor);
        }
        else
        {
            xRet = new SwXTextRange(aPam, xParent);
        }
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 09.12.98 14:18:51---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextCursor::getString(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    OUString aTxt;
    SwUnoCrsr* pUnoCrsr = ((SwXTextCursor*)this)->GetCrsr();
    if( pUnoCrsr)
    {
/*      if( pUnoCrsr->GetPoint()->nNode.GetIndex() ==
            pUnoCrsr->GetMark()->nNode.GetIndex() )
        {
            SwTxtNode* pTxtNd = pUnoCrsr->GetNode()->GetTxtNode();
            if( pTxtNd )
            {
                sal_uInt16 nStt = pUnoCrsr->Start()->nContent.GetIndex();
                aTxt = pTxtNd->GetExpandTxt( nStt,
                        pUnoCrsr->End()->nContent.GetIndex() - nStt );
            }
        }
        else
*/      {
            //Text ueber mehrere Absaetze
            SwXTextCursor::getTextFromPam(*pUnoCrsr, aTxt);
        }
    }
    else
        throw uno::RuntimeException();
    return aTxt;
}
/*-- 09.12.98 14:18:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::setString(const OUString& aString) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();

    const bool bForceExpandHints( (CURSOR_META != eType)
        ? false
        : dynamic_cast<SwXMeta*>(xParentText.get())->CheckForOwnMemberMeta(
            *GetPaM(), true) );
    DeleteAndInsert(aString, bForceExpandHints);
}
/* -----------------------------03.05.00 12:56--------------------------------

 ---------------------------------------------------------------------------*/
Any SwXTextCursor::GetPropertyValue(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const OUString& rPropertyName)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aAny;
    const SfxItemPropertySimpleEntry* pEntry = rPropSet.getPropertyMap()->getByName(
                                    rPropertyName);
    if(pEntry)
    {
        PropertyState eTemp;
        BOOL bDone = SwUnoCursorHelper::getCrsrPropertyValue( *pEntry, rPaM, &aAny, eTemp );
        if(!bDone)
        {
            SfxItemSet aSet(rPaM.GetDoc()->GetAttrPool(),
                RES_CHRATR_BEGIN, RES_FRMATR_END -1,
                RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                0L);
            SwXTextCursor::GetCrsrAttr(rPaM, aSet);

            rPropSet.getPropertyValue(*pEntry, aSet, aAny);
        }
    }
    else
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( 0 ) );

    return aAny;
}
/* -----------------------------03.05.00 12:57--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextCursor::SetPropertyValue(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet, const OUString& rPropertyName,
    const Any& aValue, USHORT nAttrMode)
        throw (UnknownPropertyException, PropertyVetoException,
            IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    SwDoc* pDoc = rPaM.GetDoc();
    const SfxItemPropertySimpleEntry* pEntry = rPropSet.getPropertyMap()->getByName(rPropertyName);
    if(pEntry)
    {
        if( pEntry->nFlags & PropertyAttribute::READONLY)
            throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( 0 ) );

        SfxItemSet aItemSet( pDoc->GetAttrPool(), pEntry->nWID, pEntry->nWID );
        SwXTextCursor::GetCrsrAttr( rPaM, aItemSet );

        if(!lcl_setCrsrPropertyValue( pEntry, rPaM, aItemSet, aValue ))
            rPropSet.setPropertyValue(*pEntry, aValue, aItemSet );
        SwXTextCursor::SetCrsrAttr(rPaM, aItemSet, nAttrMode );
    }
    else
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( 0 ) );
}
/* -----------------------------03.05.00 13:16--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< PropertyState > SwXTextCursor::GetPropertyStates(
            SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
            const Sequence< OUString >& PropertyNames,
            SwGetPropertyStatesCaller eCaller )
            throw(UnknownPropertyException, RuntimeException)
{
    const OUString* pNames = PropertyNames.getConstArray();
    Sequence< PropertyState > aRet ( PropertyNames.getLength() );
    PropertyState* pStates = aRet.getArray();

    SfxItemSet *pSet = 0, *pSetParent = 0;
    const SfxItemPropertyMap *pMap = rPropSet.getPropertyMap();
    for( INT32 i = 0, nEnd = PropertyNames.getLength(); i < nEnd; i++ )
    {
        const SfxItemPropertySimpleEntry* pEntry = pMap->getByName( pNames[i] );
        if(!pEntry)
        {
            if(pNames[i].equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT)) ||
               pNames[i].equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT)))
            {
                pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                continue;
            }
            else if( SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT == eCaller )
            {
                //this values marks the element as unknown property
                pStates[i] = beans::PropertyState_MAKE_FIXED_SIZE;
                continue;
            }
            else
                throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pNames[i], static_cast < cppu::OWeakObject * > ( 0 ) );
        }
        if ((eCaller == SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION ||  eCaller == SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT) &&
            pEntry->nWID < FN_UNO_RANGE_BEGIN &&
            pEntry->nWID > FN_UNO_RANGE_END  &&
            pEntry->nWID < RES_CHRATR_BEGIN &&
            pEntry->nWID > RES_TXTATR_END )
            pStates[i] = beans::PropertyState_DEFAULT_VALUE;
        else
        {
            if ( pEntry->nWID >= FN_UNO_RANGE_BEGIN &&
                 pEntry->nWID <= FN_UNO_RANGE_END )
                SwUnoCursorHelper::getCrsrPropertyValue(*pEntry, rPaM, 0, pStates[i] );
            else
            {
                if( !pSet )
                {
                    switch ( eCaller )
                    {
                        case SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT:
                        case SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION:
                            pSet = new SfxItemSet( rPaM.GetDoc()->GetAttrPool(),
                                    RES_CHRATR_BEGIN,   RES_TXTATR_END );
                        break;
                        case SW_PROPERTY_STATE_CALLER_SINGLE_VALUE_ONLY:
                            pSet = new SfxItemSet( rPaM.GetDoc()->GetAttrPool(),
                                    pEntry->nWID, pEntry->nWID );
                        break;
                        default:
                            pSet = new SfxItemSet( rPaM.GetDoc()->GetAttrPool(),
                                RES_CHRATR_BEGIN, RES_FRMATR_END - 1,
                                RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                                RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                                0L );
                    }
                    // --> OD 2006-07-12 #i63870#
                    SwXTextCursor::GetCrsrAttr( rPaM, *pSet );
                    // <--
                }

                if( pSet->Count() )
                    pStates[i] = rPropSet.getPropertyState( *pEntry, *pSet );
                else
                    pStates[i] = PropertyState_DEFAULT_VALUE;

                //try again to find out if a value has been inherited
                if( beans::PropertyState_DIRECT_VALUE == pStates[i] )
                {
                    if( !pSetParent )
                    {
                        pSetParent = pSet->Clone( FALSE );
                        // --> OD 2006-07-12 #i63870#
                        SwXTextCursor::GetCrsrAttr( rPaM, *pSetParent, TRUE, FALSE );
                        // <--
                    }

                    if( (pSetParent)->Count() )
                        pStates[i] = rPropSet.getPropertyState( *pEntry, *pSetParent );
                    else
                        pStates[i] = PropertyState_DEFAULT_VALUE;
                }
            }
        }
    }
    delete pSet;
    delete pSetParent;
    return aRet;
}
/* -----------------------------03.05.00 13:17--------------------------------

 ---------------------------------------------------------------------------*/
PropertyState SwXTextCursor::GetPropertyState(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet, const OUString& rPropertyName)
                        throw(UnknownPropertyException, RuntimeException)
{
    Sequence < OUString > aStrings ( 1 );
    aStrings[0] = rPropertyName;
    Sequence < PropertyState > aSeq = GetPropertyStates( rPaM, rPropSet, aStrings, SW_PROPERTY_STATE_CALLER_SINGLE_VALUE_ONLY );
    return aSeq[0];
}
/* -----------------------------03.05.00 13:20--------------------------------

 ---------------------------------------------------------------------------*/
void lcl_SelectParaAndReset ( SwPaM &rPaM, SwDoc* pDoc, const SvUShortsSort* pWhichIds = 0 )
{
    // if we are reseting paragraph attributes, we need to select the full paragraph first
    SwPosition aStart = *rPaM.Start();
    SwPosition aEnd = *rPaM.End();
    auto_ptr < SwUnoCrsr > pTemp ( pDoc->CreateUnoCrsr(aStart, FALSE) );
    if(!SwUnoCursorHelper::IsStartOfPara(*pTemp))
        pTemp->MovePara(fnParaCurr, fnParaStart);
    pTemp->SetMark();
    *pTemp->GetPoint() = aEnd;
    SwXTextCursor::SelectPam(*pTemp, sal_True);
    if(!SwUnoCursorHelper::IsEndOfPara(*pTemp))
        pTemp->MovePara(fnParaCurr, fnParaEnd);
    pDoc->ResetAttrs(*pTemp, sal_True, pWhichIds);
}


void SwXTextCursor::SetPropertyToDefault(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const OUString& rPropertyName)
    throw(UnknownPropertyException, RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());
    SwDoc* pDoc = rPaM.GetDoc();
    const SfxItemPropertySimpleEntry*   pEntry = rPropSet.getPropertyMap()->getByName( rPropertyName);
    if(pEntry)
    {
        if ( pEntry->nFlags & PropertyAttribute::READONLY)
            throw RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "setPropertyToDefault: property is read-only: " ) ) + rPropertyName, 0 );
        if(pEntry->nWID < RES_FRMATR_END)
        {
            SvUShortsSort aWhichIds;
            aWhichIds.Insert(pEntry->nWID);
            if(pEntry->nWID < RES_PARATR_BEGIN)
                pDoc->ResetAttrs(rPaM, sal_True, &aWhichIds);
            else
                lcl_SelectParaAndReset ( rPaM, pDoc, &aWhichIds );
        }
        else
            SwUnoCursorHelper::resetCrsrPropertyValue(*pEntry, rPaM);
    }
    else
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( 0 ) );
}
/* -----------------------------03.05.00 13:19--------------------------------

 ---------------------------------------------------------------------------*/
Any SwXTextCursor::GetPropertyDefault(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aRet;
    SwDoc* pDoc = rPaM.GetDoc();
    const SfxItemPropertySimpleEntry*   pEntry = rPropSet.getPropertyMap()->getByName( rPropertyName);
    if(pEntry)
    {
        if(pEntry->nWID < RES_FRMATR_END)
        {
            const SfxPoolItem& rDefItem = pDoc->GetAttrPool().GetDefaultItem(pEntry->nWID);
            rDefItem.QueryValue(aRet, pEntry->nMemberId);
        }
    }
    else
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( 0 ) );
    return aRet;
}
/*-- 09.12.98 14:18:54---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXTextCursor::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef;
    if(!xRef.is())
    {
        static SfxItemPropertyMapEntry aCrsrExtMap_Impl[] =
        {
            { SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT), FN_SKIP_HIDDEN_TEXT, &::getBooleanCppuType(), PROPERTY_NONE,     0},
            { SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT), FN_SKIP_PROTECTED_TEXT, &::getBooleanCppuType(), PROPERTY_NONE,     0},
            {0,0,0,0,0,0}
        };
        uno::Reference< beans::XPropertySetInfo >  xInfo = m_pPropSet->getPropertySetInfo();
        // PropertySetInfo verlaengern!
        const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
        xRef = new SfxExtItemPropertySetInfo(
            aCrsrExtMap_Impl,
            aPropSeq );
    }
    return xRef;
}
/*-- 09.12.98 14:18:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
        throw( beans::UnknownPropertyException, beans::PropertyVetoException,
             lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT)))
        {
            sal_Bool bSet = *(sal_Bool*)aValue.getValue();
            pUnoCrsr->SetSkipOverHiddenSections(bSet);
        }
        else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT)))
        {
            sal_Bool bSet = *(sal_Bool*)aValue.getValue();
            pUnoCrsr->SetSkipOverProtectSections(bSet);
        }
        else
            SetPropertyValue(*pUnoCrsr, *m_pPropSet, rPropertyName, aValue);
    }
    else
        throw uno::RuntimeException();

}
/*-- 09.12.98 14:18:55---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXTextCursor::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aAny;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT)))
        {
            BOOL bSet = pUnoCrsr->IsSkipOverHiddenSections();
            aAny.setValue(&bSet, ::getBooleanCppuType());
        }
        else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT)))
        {
            BOOL bSet = pUnoCrsr->IsSkipOverProtectSections();
            aAny.setValue(&bSet, ::getBooleanCppuType());
        }
        else
            aAny = GetPropertyValue(*pUnoCrsr, *m_pPropSet, rPropertyName);
    }
    else
        throw uno::RuntimeException();
    return aAny;

}
/*-- 09.12.98 14:18:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::addPropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 09.12.98 14:18:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 09.12.98 14:18:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 09.12.98 14:18:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 05.03.99 11:36:11---------------------------------------------------

  -----------------------------------------------------------------------*/
beans::PropertyState SwXTextCursor::getPropertyState(const OUString& rPropertyName)
                        throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        eRet = GetPropertyState(*pUnoCrsr, *m_pPropSet, rPropertyName);
    }
    else
        throw RuntimeException();
    return eRet;
}
/*-- 05.03.99 11:36:11---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyState > SwXTextCursor::getPropertyStates(
            const uno::Sequence< OUString >& PropertyNames)
            throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(!pUnoCrsr)
        throw RuntimeException();
    return GetPropertyStates(*pUnoCrsr, *m_pPropSet, PropertyNames);
}
/*-- 05.03.99 11:36:12---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::setPropertyToDefault(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    Sequence < OUString > aSequence ( &rPropertyName, 1 );
    setPropertiesToDefault ( aSequence );
}
/*-- 05.03.99 11:36:12---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextCursor::getPropertyDefault(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    const Sequence < OUString > aSequence ( &rPropertyName, 1 );
    return getPropertyDefaults ( aSequence ).getConstArray()[0];
}

void SAL_CALL SwXTextCursor::setAllPropertiesToDefault()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if (pUnoCrsr)
    {
        // para specific attribut ranges
        USHORT aParaResetableSetRange[] = {
            RES_FRMATR_BEGIN, RES_FRMATR_END-1,
            RES_PARATR_BEGIN, RES_PARATR_END-1,
            // --> OD 2008-02-25 #refactorlists#
            RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
            // <--
            RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
            0
        };
        // selection specific attribut ranges
        USHORT aResetableSetRange[] = {
            RES_CHRATR_BEGIN, RES_CHRATR_END-1,
            RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
            RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
            RES_TXTATR_CJK_RUBY, RES_TXTATR_CJK_RUBY,
            RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
            0
        };
        SvUShortsSort   aParaWhichIds;
        SvUShortsSort   aWhichIds;
        for (USHORT k = 0;  k < 2;  ++k)
        {
            SvUShortsSort &rWhichIds    = k == 0? aParaWhichIds : aWhichIds;
            USHORT *pResetableSetRange  = k == 0? aParaResetableSetRange : aResetableSetRange;
            while (*pResetableSetRange)
            {
                USHORT nStart   = sal::static_int_cast< USHORT >(*pResetableSetRange++);
                USHORT nEnd     = sal::static_int_cast< USHORT >(*pResetableSetRange++);
                for (USHORT nId = nStart + 1;  nId <= nEnd;  ++nId)
                    rWhichIds.Insert( nId );
            }
        }
        if (aParaWhichIds.Count())
            lcl_SelectParaAndReset( *pUnoCrsr, pUnoCrsr->GetDoc(), &aParaWhichIds );
        if (aWhichIds.Count() )
            pUnoCrsr->GetDoc()->ResetAttrs( *pUnoCrsr, sal_True, &aWhichIds );
    }
    else
        throw uno::RuntimeException();
}

void SAL_CALL SwXTextCursor::setPropertiesToDefault( const Sequence< OUString >& aPropertyNames )
    throw (UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int32 nCount = aPropertyNames.getLength();
    if ( nCount )
    {
        SwUnoCrsr* pUnoCrsr = GetCrsr();
        if(pUnoCrsr)
        {
            SwDoc* pDoc = pUnoCrsr->GetDoc();
            const OUString * pNames = aPropertyNames.getConstArray();
            SvUShortsSort aWhichIds, aParaWhichIds;
            for ( sal_Int32 i = 0; i < nCount; i++ )
            {
                const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap()->getByName( pNames[i] );
                if(!pEntry)
                {
                    if(pNames[i].equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT)) ||
                       pNames[i].equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT)))
                    {
                        continue;
                    }
                    else
                        throw UnknownPropertyException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pNames[i], static_cast < cppu::OWeakObject * > ( 0 ) );
                }
                if( pEntry->nFlags & PropertyAttribute::READONLY)
                    throw RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "setPropertiesToDefault: property is read-only: " ) ) + pNames[i], static_cast < cppu::OWeakObject * > ( this ) );

                if( pEntry->nWID < RES_FRMATR_END)
                {
                    if(pEntry->nWID < RES_PARATR_BEGIN)
                        aWhichIds.Insert(pEntry->nWID);
                    else
                        aParaWhichIds.Insert (pEntry->nWID);
                }
                else if ( pEntry->nWID == FN_UNO_NUM_START_VALUE )
                    SwUnoCursorHelper::resetCrsrPropertyValue(*pEntry, *pUnoCrsr);
            }

            if ( aParaWhichIds.Count() )
                lcl_SelectParaAndReset ( *pUnoCrsr, pDoc, &aParaWhichIds );
            if (aWhichIds.Count() )
                pDoc->ResetAttrs(*pUnoCrsr, sal_True, &aWhichIds);
        }
        else
            throw uno::RuntimeException();
    }
}
Sequence< Any > SAL_CALL SwXTextCursor::getPropertyDefaults( const Sequence< OUString >& aPropertyNames )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int32 nCount = aPropertyNames.getLength();
    Sequence < Any > aRet ( nCount );
    if ( nCount )
    {
        SwUnoCrsr* pUnoCrsr = GetCrsr();
        if (pUnoCrsr)
        {
            SwDoc* pDoc = pUnoCrsr->GetDoc();
            const OUString *pNames = aPropertyNames.getConstArray();
            Any *pAny = aRet.getArray();
            for ( sal_Int32 i = 0; i < nCount; i++)
            {
                const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap()->getByName( pNames[i] );
                if(!pEntry)
                {
                    if(pNames[i].equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT)) ||
                       pNames[i].equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT)))
                    {
                        continue;
                    }
                    else
                        throw UnknownPropertyException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pNames[i], static_cast < cppu::OWeakObject * > ( 0 ) );
                }
                if(pEntry->nWID < RES_FRMATR_END)
                {
                    const SfxPoolItem& rDefItem = pDoc->GetAttrPool().GetDefaultItem(pEntry->nWID);
                    rDefItem.QueryValue(pAny[i], pEntry->nMemberId);
                }
            }
        }
        else
            throw UnknownPropertyException();
    }
    return aRet;
}

/*-- 10.03.2008 09:58:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::makeRedline(
    const ::rtl::OUString& rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties )
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        SwUnoCursorHelper::makeRedline( *pUnoCrsr, rRedlineType, rRedlineProperties );
    else
        throw uno::RuntimeException();
}

/*-- 09.12.98 14:18:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXTextCursor::insertDocumentFromURL(const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& rOptions)
throw (lang::IllegalArgumentException, io::IOException,
        uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if (!pUnoCrsr)
    {
        throw uno::RuntimeException();
    }
    SwUnoCursorHelper::InsertFile(pUnoCrsr, rURL, rOptions);
}

/* -----------------------------15.12.00 14:01--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue >
SwXTextCursor::createSortDescriptor(sal_Bool bFromTable)
{
    uno::Sequence< beans::PropertyValue > aRet(5);
    beans::PropertyValue* pArray = aRet.getArray();

    uno::Any aVal;
    aVal.setValue( &bFromTable, ::getCppuBooleanType());
    pArray[0] = beans::PropertyValue(C2U("IsSortInTable"), -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    String sSpace(String::CreateFromAscii(" "));
    sal_Unicode uSpace = sSpace.GetChar(0);

    aVal <<= uSpace;
    pArray[1] = beans::PropertyValue(C2U("Delimiter"), -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    aVal <<= (sal_Bool) sal_False;
    pArray[2] = beans::PropertyValue(C2U("IsSortColumns"), -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    aVal <<= (sal_Int32) 3;
    pArray[3] = beans::PropertyValue(C2U("MaxSortFieldsCount"), -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    uno::Sequence< table::TableSortField > aFields(3);
    table::TableSortField* pFields = aFields.getArray();

    Locale aLang( SvxCreateLocale( LANGUAGE_SYSTEM ) );
    // get collator algorithm to be used for the locale
    uno::Sequence< OUString > aSeq(
            GetAppCollator().listCollatorAlgorithms( aLang ) );
    INT32 nLen = aSeq.getLength();
    DBG_ASSERT( nLen > 0, "list of collator algorithms is empty!");
    OUString aCollAlg;
    if (nLen > 0)
        aCollAlg = aSeq.getConstArray()[0];

#if OSL_DEBUG_LEVEL > 1
    const OUString *pTxt = aSeq.getConstArray();
    (void)pTxt;
#endif

    pFields[0].Field = 1;
    pFields[0].IsAscending = sal_True;
    pFields[0].IsCaseSensitive = sal_False;
    pFields[0].FieldType = table::TableSortFieldType_ALPHANUMERIC;
    pFields[0].CollatorLocale = aLang;
    pFields[0].CollatorAlgorithm = aCollAlg;

    pFields[1].Field = 1;
    pFields[1].IsAscending = sal_True;
    pFields[1].IsCaseSensitive = sal_False;
    pFields[1].FieldType = table::TableSortFieldType_ALPHANUMERIC;
    pFields[1].CollatorLocale = aLang;
    pFields[1].CollatorAlgorithm = aCollAlg;

    pFields[2].Field = 1;
    pFields[2].IsAscending = sal_True;
    pFields[2].IsCaseSensitive = sal_False;
    pFields[2].FieldType = table::TableSortFieldType_ALPHANUMERIC;
    pFields[2].CollatorLocale = aLang;
    pFields[2].CollatorAlgorithm = aCollAlg;

    aVal <<= aFields;
    pArray[4] = beans::PropertyValue(C2U("SortFields"), -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    return aRet;
}

/*-- 09.12.98 14:18:58---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue > SAL_CALL
SwXTextCursor::createSortDescriptor() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXTextCursor::createSortDescriptor(sal_False);
}

/* -----------------------------15.12.00 14:06--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXTextCursor::convertSortProperties(
    const uno::Sequence< beans::PropertyValue >& rDescriptor,
    SwSortOptions& rSortOpt)
{
    sal_Bool bRet = sal_True;
    const beans::PropertyValue* pProperties = rDescriptor.getConstArray();

    rSortOpt.bTable = sal_False;
    rSortOpt.cDeli = ' ';
    rSortOpt.eDirection = SRT_COLUMNS;  //!! UI text may be contrary though !!

    SwSortKey* pKey1 = new SwSortKey;
    pKey1->nColumnId = USHRT_MAX;
    pKey1->bIsNumeric = TRUE;
    pKey1->eSortOrder = SRT_ASCENDING;

    SwSortKey* pKey2 = new SwSortKey;
    pKey2->nColumnId = USHRT_MAX;
    pKey2->bIsNumeric = TRUE;
    pKey2->eSortOrder = SRT_ASCENDING;

    SwSortKey* pKey3 = new SwSortKey;
    pKey3->nColumnId = USHRT_MAX;
    pKey3->bIsNumeric = TRUE;
    pKey3->eSortOrder = SRT_ASCENDING;
    SwSortKey* aKeys[3] = {pKey1, pKey2, pKey3};

    sal_Bool bOldSortdescriptor(sal_False);
    sal_Bool bNewSortdescriptor(sal_False);

    for (int n = 0; n < rDescriptor.getLength(); ++n)
    {
        uno::Any aValue( pProperties[n].Value );
//      String sPropName = pProperties[n].Name;
        const OUString& rPropName = pProperties[n].Name;

        // old and new sortdescriptor
        if (COMPARE_EQUAL == rPropName.compareToAscii("IsSortInTable"))
        {
            if (aValue.getValueType() == ::getBooleanCppuType())
            {
                rSortOpt.bTable = *(sal_Bool*)aValue.getValue();
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (COMPARE_EQUAL == rPropName.compareToAscii("Delimiter"))
        {
            sal_Unicode uChar = sal_Unicode();
            if (aValue >>= uChar)
            {
                rSortOpt.cDeli = uChar;
            }
            else
            {
                bRet = sal_False;
            }
        }
        // old sortdescriptor
        else if (COMPARE_EQUAL == rPropName.compareToAscii("SortColumns"))
        {
            bOldSortdescriptor = sal_True;
            if (aValue.getValueType() == ::getBooleanCppuType())
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                rSortOpt.eDirection = bTemp ? SRT_COLUMNS : SRT_ROWS;
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (COMPARE_EQUAL == rPropName.compareToAscii("IsCaseSensitive"))
        {
            bOldSortdescriptor = sal_True;
            if (aValue.getValueType() == ::getBooleanCppuType())
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                rSortOpt.bIgnoreCase = !bTemp;
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (COMPARE_EQUAL == rPropName.compareToAscii("CollatorLocale"))
        {
            bOldSortdescriptor = sal_True;
            Locale aLocale;
            if (aValue >>= aLocale)
            {
                rSortOpt.nLanguage = SvxLocaleToLanguage( aLocale );
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (COMPARE_EQUAL == rPropName.compareToAscii("CollatorAlgorithm",
                    17) &&
            rPropName.getLength() == 18 &&
            (rPropName.getStr()[17] >= '0' && rPropName.getStr()[17] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[17];
            nIndex -= '0';
            OUString aTxt;
            if ((aValue >>= aTxt) && nIndex < 3)
            {
                aKeys[nIndex]->sSortType = aTxt;
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (COMPARE_EQUAL == rPropName.compareToAscii("SortRowOrColumnNo",
                    17) &&
            rPropName.getLength() == 18 &&
            (rPropName.getStr()[17] >= '0' && rPropName.getStr()[17] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[17];
            nIndex -= '0';
            sal_Int16 nCol = -1;
            if (aValue.getValueType() == ::getCppuType((const sal_Int16*)0)
                && nIndex < 3)
            {
                aValue >>= nCol;
            }
            if (nCol >= 0)
            {
                aKeys[nIndex]->nColumnId = nCol;
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (0 == rPropName.indexOf(C2U("IsSortNumeric")) &&
            rPropName.getLength() == 14 &&
            (rPropName.getStr()[13] >= '0' && rPropName.getStr()[13] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[13];
            nIndex = nIndex - '0';
            if (aValue.getValueType() == ::getBooleanCppuType() && nIndex < 3)
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                aKeys[nIndex]->bIsNumeric = bTemp;
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (0 == rPropName.indexOf(C2U("IsSortAscending")) &&
            rPropName.getLength() == 16 &&
            (rPropName.getStr()[15] >= '0' && rPropName.getStr()[15] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[15];
            nIndex -= '0';
            if (aValue.getValueType() == ::getBooleanCppuType() && nIndex < 3)
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                aKeys[nIndex]->eSortOrder = (bTemp)
                    ? SRT_ASCENDING : SRT_DESCENDING;
            }
            else
            {
                bRet = sal_False;
            }
        }
        // new sortdescriptor
        else if (COMPARE_EQUAL == rPropName.compareToAscii("IsSortColumns"))
        {
            bNewSortdescriptor = sal_True;
            if (aValue.getValueType() == ::getBooleanCppuType())
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                rSortOpt.eDirection = bTemp ? SRT_COLUMNS : SRT_ROWS;
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (COMPARE_EQUAL == rPropName.compareToAscii("SortFields"))
        {
            bNewSortdescriptor = sal_True;
            uno::Sequence < table::TableSortField > aFields;
            if (aValue >>= aFields)
            {
                sal_Int32 nCount(aFields.getLength());
                if (nCount <= 3)
                {
                    table::TableSortField* pFields = aFields.getArray();
                    for (sal_Int32 i = 0; i < nCount; ++i)
                    {
                        rSortOpt.bIgnoreCase = !pFields[i].IsCaseSensitive;
                        rSortOpt.nLanguage =
                            SvxLocaleToLanguage( pFields[i].CollatorLocale );
                        aKeys[i]->sSortType = pFields[i].CollatorAlgorithm;
                        aKeys[i]->nColumnId =
                            static_cast<USHORT>(pFields[i].Field);
                        aKeys[i]->bIsNumeric = (pFields[i].FieldType ==
                                table::TableSortFieldType_NUMERIC);
                        aKeys[i]->eSortOrder = (pFields[i].IsAscending)
                            ? SRT_ASCENDING : SRT_DESCENDING;
                    }
                }
                else
                {
                    bRet = sal_False;
                }
            }
            else
            {
                bRet = sal_False;
            }
        }
    }

    if (bNewSortdescriptor && bOldSortdescriptor)
    {
        DBG_ERROR("someone tried to set the old deprecated and "
            "the new sortdescriptor");
        bRet = sal_False;
    }

    if (pKey1->nColumnId != USHRT_MAX)
    {
        rSortOpt.aKeys.C40_INSERT(SwSortKey, pKey1, rSortOpt.aKeys.Count());
    }
    if (pKey2->nColumnId != USHRT_MAX)
    {
        rSortOpt.aKeys.C40_INSERT(SwSortKey, pKey2, rSortOpt.aKeys.Count());
    }
    if (pKey3->nColumnId != USHRT_MAX)
    {
        rSortOpt.aKeys.C40_INSERT(SwSortKey, pKey3, rSortOpt.aKeys.Count());
    }

    return bRet && rSortOpt.aKeys.Count() > 0;
}

/*-- 09.12.98 14:19:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXTextCursor::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwSortOptions aSortOpt;
    SwUnoCrsr* pUnoCrsr = GetCrsr();

    if (!pUnoCrsr)
    {
        throw uno::RuntimeException();
    }

    if (pUnoCrsr->HasMark())
    {
        if (!SwXTextCursor::convertSortProperties(rDescriptor, aSortOpt))
        {
                throw uno::RuntimeException();
        }
        UnoActionContext aContext( pUnoCrsr->GetDoc() );

        SwPosition* pStart = pUnoCrsr->Start();
        SwPosition* pEnd   = pUnoCrsr->End();

        SwNodeIndex aPrevIdx( pStart->nNode, -1 );
        ULONG nOffset = pEnd->nNode.GetIndex() - pStart->nNode.GetIndex();
        xub_StrLen nCntStt  = pStart->nContent.GetIndex();

        pUnoCrsr->GetDoc()->SortText(*pUnoCrsr, aSortOpt);

        // Selektion wieder setzen
        pUnoCrsr->DeleteMark();
        pUnoCrsr->GetPoint()->nNode.Assign( aPrevIdx.GetNode(), +1 );
        SwCntntNode* pCNd = pUnoCrsr->GetCntntNode();
        xub_StrLen nLen = pCNd->Len();
        if (nLen > nCntStt)
        {
            nLen = nCntStt;
        }
        pUnoCrsr->GetPoint()->nContent.Assign(pCNd, nLen );
        pUnoCrsr->SetMark();

        pUnoCrsr->GetPoint()->nNode += nOffset;
        pCNd = pUnoCrsr->GetCntntNode();
        pUnoCrsr->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
    }
}

/* -----------------------------03.04.00 09:11--------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference< container::XEnumeration > SAL_CALL
SwXTextCursor::createContentEnumeration(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if (!pUnoCrsr ||
        0 != rServiceName.compareToAscii("com.sun.star.text.TextContent"))
    {
        throw uno::RuntimeException();
    }

    uno::Reference< container::XEnumeration > xRet =
        new SwXParaFrameEnumeration(*pUnoCrsr, PARAFRAME_PORTION_TEXTRANGE);
    return xRet;
}

/* -----------------------------07.03.01 14:53--------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference< container::XEnumeration > SAL_CALL
SwXTextCursor::createEnumeration() throw (uno::RuntimeException)
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if (!pUnoCrsr)
    {
        throw uno::RuntimeException();
    }
    uno::Reference<lang::XUnoTunnel> xTunnel(xParentText, UNO_QUERY);
    SwXText* pParentText = 0;
    if (xTunnel.is())
    {
        pParentText = reinterpret_cast< SwXText *>(
                sal::static_int_cast< sal_IntPtr >(
                    xTunnel->getSomething(SwXText::getUnoTunnelId()) ));
    }
    DBG_ASSERT(pParentText, "parent is not a SwXText");
    if (!pParentText)
    {
        throw uno::RuntimeException();
    }

    ::std::auto_ptr<SwUnoCrsr> pNewCrsr(
        pUnoCrsr->GetDoc()->CreateUnoCrsr(*pUnoCrsr->GetPoint()) );
    if (pUnoCrsr->HasMark())
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pUnoCrsr->GetMark();
    }
    const CursorType eSetType = (CURSOR_TBLTEXT == eType)
            ? CURSOR_SELECTION_IN_TABLE : CURSOR_SELECTION;
    const uno::Reference< XEnumeration > xRet =
        new SwXParagraphEnumeration(pParentText, pNewCrsr, eSetType);

    return xRet;
}

/* -----------------------------07.03.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
uno::Type SAL_CALL
SwXTextCursor::getElementType() throw (uno::RuntimeException)
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}

/* -----------------------------07.03.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SAL_CALL SwXTextCursor::hasElements() throw (uno::RuntimeException)
{
    return sal_True;
}

/* -----------------------------03.04.00 09:11--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SAL_CALL
SwXTextCursor::getAvailableServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString::createFromAscii("com.sun.star.text.TextContent");
    return aRet;
}

// ---------------------------------------------------------------------------
IMPLEMENT_FORWARD_REFCOUNT( SwXTextCursor,SwXTextCursor_Base )

uno::Any SAL_CALL
SwXTextCursor::queryInterface(const uno::Type& rType)
throw (uno::RuntimeException)
{
    return (rType == lang::XUnoTunnel::static_type())
        ? OTextCursorHelper::queryInterface(rType)
        : SwXTextCursor_Base::queryInterface(rType);
}

