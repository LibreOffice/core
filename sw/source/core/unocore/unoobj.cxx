/*************************************************************************
 *
 *  $RCSfile: unoobj.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: os $ $Date: 2000-11-03 08:52:28 $
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


#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#include <swtypes.hxx>
#include <hintids.hxx>
#include <cmdid.h>
#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif
#ifndef _BOOKMRK_HXX //autogen
#include <bookmrk.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _NDNOTXT_HXX //autogen
#include <ndnotxt.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _SWUNDO_HXX //autogen
#include <swundo.hxx>
#endif
#ifndef _ROOTFRM_HXX //autogen
#include <rootfrm.hxx>
#endif
#ifndef _FLYFRM_HXX //autogen
#include <flyfrm.hxx>
#endif
#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
#endif
#ifndef _LINKMGR_HXX
#include <so3/linkmgr.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#include <pam.hxx>
#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif
#ifndef _SHELLIO_HXX //autogen
#include <shellio.hxx>
#endif
#include <swerror.h>
#ifndef _SWTBLFMT_HXX //autogen
#include <swtblfmt.hxx>
#endif
#ifndef _FMTHBSH_HXX
#include <fmthbsh.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _DOCSTYLE_HXX //autogen
#include <docstyle.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _EDIMP_HXX //autogen
#include <edimp.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _CNTFRM_HXX //autogen
#include <cntfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX //autogen
#include <pagefrm.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen
#include <fmtrfmrk.hxx>
#endif
#ifndef _TXTRFMRK_HXX //autogen
#include <txtrfmrk.hxx>
#endif
#ifndef _UNOCLBCK_HXX
#include <unoclbck.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef SW_UNOMID_HXX
#include <unomid.h>
#endif
#ifndef _UNOSETT_HXX
#include <unosett.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _UNOTBL_HXX
#include <unotbl.hxx>
#endif
#ifndef _UNODRAW_HXX
#include <unodraw.hxx>
#endif
#ifndef _UNOCOLL_HXX
#include <unocoll.hxx>
#endif
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif
#ifndef _UNOFIELD_HXX
#include <unofield.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include <svx/flstitem.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _FLYPOS_HXX
#include <flypos.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_PAGESTYLELAYOUT_HPP_
#include <com/sun/star/style/PageStyleLayout.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _UNOIDX_HXX
#include <unoidx.hxx>
#endif
#ifndef _UNOFRAME_HXX
#include <unoframe.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_ //autogen
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#include <svtools/svstdarr.hxx>
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif

//TODO: new Interface & new uno::Exception for protected content
#define EXCEPT_ON_PROTECTION(rUnoCrsr)  \
    if((rUnoCrsr).HasReadonlySel()) \
        throw uno::RuntimeException();

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;

using namespace ::rtl;

//collectn.cxx
BOOL lcl_IsNumeric(const String&);
/****************************************************************************
    static methods
****************************************************************************/
::com::sun::star::uno::Sequence< sal_Int8 >  CreateUnoTunnelId()
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
class SwParaSelection
{
    SwUnoCrsr* pUnoCrsr;
public:
    SwParaSelection(SwUnoCrsr* pCrsr);
    ~SwParaSelection();
};

SwParaSelection::SwParaSelection(SwUnoCrsr* pCrsr) :
    pUnoCrsr(pCrsr)
{
    if(pUnoCrsr->HasMark())
        pUnoCrsr->DeleteMark();
    // steht er schon am Anfang?
    if(pUnoCrsr->GetPoint()->nContent != 0)
        pUnoCrsr->MovePara(fnParaCurr, fnParaStart);
    // oder gleichzeitig am Ende?
    if(pUnoCrsr->GetPoint()->nContent != pUnoCrsr->GetCntntNode()->Len())
    {
        pUnoCrsr->SetMark();
        pUnoCrsr->MovePara(fnParaCurr, fnParaEnd);
    }
}

SwParaSelection::~SwParaSelection()
{
    if(pUnoCrsr->GetPoint()->nContent != 0)
    {
        pUnoCrsr->DeleteMark();
        pUnoCrsr->MovePara(fnParaCurr, fnParaStart);
    }
}
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
/****************************************************************************
    ActionContext
****************************************************************************/
UnoActionContext::UnoActionContext(SwDoc* pDc) :
    pDoc(pDc)
{
    SwRootFrm* pRootFrm = pDoc->GetRootFrm();
    if(pRootFrm)
        pRootFrm->StartAllAction();
}
/*-----------------04.03.98 11:56-------------------

--------------------------------------------------*/
UnoActionContext::~UnoActionContext()
{
    //das Doc kann hier schon entfernt worden sein
    if(pDoc)
    {
        SwRootFrm* pRootFrm = pDoc->GetRootFrm();
        if(pRootFrm)
            pRootFrm->EndAllAction();
    }
}

/****************************************************************************
    ActionRemoveContext
****************************************************************************/
UnoActionRemoveContext::UnoActionRemoveContext(SwDoc* pDc) :
    pDoc(pDc)
{
    SwRootFrm* pRootFrm = pDoc->GetRootFrm();
    if(pRootFrm)
        pRootFrm->UnoRemoveAllActions();
}
/* -----------------07.07.98 12:05-------------------
 *
 * --------------------------------------------------*/
UnoActionRemoveContext::~UnoActionRemoveContext()
{
    SwRootFrm* pRootFrm = pDoc->GetRootFrm();
    if(pRootFrm)
        pRootFrm->UnoRestoreAllActions();

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
/* -----------------11.06.98 14:19-------------------
 *
 * --------------------------------------------------*/
sal_Bool lcl_IsEndOfPara(SwPaM& rUnoCrsr)
{
    return rUnoCrsr.GetCntntNode() &&
        rUnoCrsr.GetPoint()->nContent == rUnoCrsr.GetCntntNode()->Len();
}

/* -----------------11.06.98 14:19-------------------
 *
 * --------------------------------------------------*/
inline sal_Bool lcl_IsStartOfPara(SwPaM& rUnoCrsr)
{
    return rUnoCrsr.GetPoint()->nContent == 0;
}

/* -----------------20.05.98 14:59-------------------
 *
 * --------------------------------------------------*/
void lcl_Append(sal_Unicode*& pCurrent, const SVBT16* pStr, sal_uInt32 nLen)
{
    for(sal_uInt32 i = 0; i < nLen; i++)
    {
         sal_uInt16 nVal = SVBT16ToShort( *pStr );
        pStr += 1;
        *pCurrent++ = nVal;
    }
}

void SwXTextCursor::getTextFromPam(SwPaM& aCrsr, OUString& rBuffer)
{
    if(!aCrsr.HasMark())
        return;
    SvCacheStream aStream(20480);
    WriterRef xWrt;
    SwIoSystem::GetWriter( C2S(FILTER_TEXT_DLG), xWrt );
    if( xWrt.Is() )
    {
        SwWriter aWriter(aStream, aCrsr);
        xWrt->bASCII_NoLastLineEnd = sal_True;
        SwAsciiOptions aOpt = xWrt->GetAsciiOptions();
        aOpt.SetCharSet( RTL_TEXTENCODING_UNICODE );
        xWrt->SetAsciiOptions( aOpt );
        xWrt->bUCS2_WithStartChar = FALSE;

        if(!IsError( aWriter.Write( xWrt )))
        {
            long lLen  = aStream.GetSize();
            lLen /= 2;
            sal_Unicode* pUnicodeBuffer = new sal_Unicode[lLen],
            *pCurrentBuffer = pUnicodeBuffer;
            const void* pStr = aStream.GetBuffer();
            if(pStr)
            {
                lcl_Append(pCurrentBuffer, (SVBT16*)pStr, lLen);
            }
            else
            {
                aStream.Seek( 0 );
                aStream.ResetError();
                char *pBuffer = new char[16384];
                long lLocalLen  = lLen;
                while(lLocalLen > 0)
                {
                    long nRead = aStream.Read( pBuffer, 16384 );
                    lcl_Append(pCurrentBuffer, (SVBT16*)pBuffer, nRead / 2);
                    lLocalLen -= nRead;
                }
                delete pBuffer;
            }
            rBuffer = OUString(pUnicodeBuffer, lLen);
            delete pUnicodeBuffer;
        }
    }
}

/* -----------------21.07.98 11:36-------------------
 *
 * --------------------------------------------------*/
void lcl_InsertFile(SwUnoCrsr* pUnoCrsr,
    const String& rFileName,
    const String& rFilterName,
    const String& rFilterOption,
    const String& rPassword)
{
    SfxMedium* pMed = 0;
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    SwDocShell* pDocSh = pDoc->GetDocShell();
    if(!pDocSh || !rFileName.Len())
        return;

    const SfxFilter* pFilter = 0;

    SfxObjectFactory& rFact = pDocSh->GetFactory();
    const sal_uInt16 nCount = rFact.GetFilterCount();
    if(rFilterName.Len())
        for( sal_uInt16 i = 0; i < nCount; i++ )
        {
            const SfxFilter* pFlt = rFact.GetFilter( i );
            if( pFlt->GetName() == rFilterName ||
                pFlt->GetFilterName() == rFilterName )
            {
                pFilter = pFlt;
                break;
            }
        }
    if ( !pFilter )
    {
        pMed = new SfxMedium(rFileName, STREAM_READ, sal_True, 0, 0 );
        SfxFilterMatcher aMatcher( rFact.GetFilterContainer() );
        ErrCode nErr = aMatcher.GuessFilter( *pMed, &pFilter, sal_False );
        if ( nErr || !pFilter)
            DELETEZ(pMed);
        else
            pMed->SetFilter( pFilter );
    }
    else
    {
        pMed = new SfxMedium(rFileName, STREAM_READ, sal_True,
#if SUPD<609
            sal_True,
#endif
                    pFilter, 0);
        if(rFilterOption.Len())
        {
            SfxItemSet* pSet =  pMed->GetItemSet();
            SfxStringItem aOptionItem(SID_FILE_FILTEROPTIONS, rFilterOption);
            pSet->Put(aOptionItem);
        }
    }

    if( !pMed )
        return;

    SfxObjectShellRef aRef( pDocSh );

    pDocSh->RegisterTransfer( *pMed );
    pMed->DownLoad();   // ggfs. den DownLoad anstossen
    if( aRef.Is() && 1 < aRef->GetRefCount() )  // noch gueltige Ref?
    {
        SwReader* pRdr;
        SfxItemSet* pSet =  pMed->GetItemSet();
        pSet->Put(SfxBoolItem(FN_API_CALL, sal_True));
        if(rPassword.Len())
            pSet->Put(SfxStringItem(SID_PASSWORD, rPassword));
        Reader *pRead = pDocSh->StartConvertFrom( *pMed, &pRdr, 0, pUnoCrsr);
        if( pRead )
        {
            String sTmpBaseURL( INetURLObject::GetBaseURL() );
            INetURLObject::SetBaseURL( pMed->GetName() );

            UnoActionContext aContext(pDoc);

            if(pUnoCrsr->HasMark())
                pDoc->DeleteAndJoin(*pUnoCrsr);

            SwNodeIndex aSave(  pUnoCrsr->GetPoint()->nNode, -1 );
            xub_StrLen nCntnt = pUnoCrsr->GetPoint()->nContent.GetIndex();

            sal_uInt32 nErrno = pRdr->Read( *pRead );   // und Dokument einfuegen

            if(!nErrno)
            {
                aSave++;
                pUnoCrsr->SetMark();
                pUnoCrsr->GetMark()->nNode = aSave;

                SwCntntNode* pCntNode = aSave.GetNode().GetCntntNode();
                if( !pCntNode )
                    nCntnt = 0;
                pUnoCrsr->GetMark()->nContent.Assign( pCntNode, nCntnt );
            }

            delete pRdr;

            INetURLObject::SetBaseURL( sTmpBaseURL );

            // ggfs. alle Verzeichnisse updaten:
/*          if( pWrtShell->IsUpdateTOX() )
            {
                SfxRequest aReq( *this, FN_UPDATE_TOX );
                Execute( aReq );
                pWrtShell->SetUpdateTOX( sal_False );       // wieder zurueck setzen
            }*/

        }
    }
    delete pMed;
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
        String sStyle(SwXStyleFamilies::GetUIName(uStyle, SFX_STYLE_FAMILY_CHAR));
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
    String sStyle(SwXStyleFamilies::GetUIName(uStyle, SFX_STYLE_FAMILY_PARA));
    SwDocStyleSheet* pStyle =
                    (SwDocStyleSheet*)pDocSh->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_PARA);
    if(pStyle)
    {
        SwTxtFmtColl *pLocal = pStyle->GetCollection();
        UnoActionContext aAction(pDoc);
        pDoc->StartUndo( UNDO_START );
        FOREACHUNOPAM_START(&rPaM)
            pDoc->SetTxtFmtColl(*PUNOPAM, pLocal);
        FOREACHUNOPAM_END()
        pDoc->EndUndo( UNDO_END );
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
    SwFmtPageDesc* pNewDesc = 0;
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState( RES_PAGEDESC, sal_True, &pItem ) )
    {
        pNewDesc = new SwFmtPageDesc(*((SwFmtPageDesc*)pItem));
    }
    if(!pNewDesc)
        pNewDesc = new SwFmtPageDesc();
    OUString uDescName;
    aValue >>= uDescName;
    String sDescName(SwXStyleFamilies::GetUIName(uDescName, SFX_STYLE_FAMILY_PAGE));
    if(!pNewDesc->GetPageDesc() || pNewDesc->GetPageDesc()->GetName() != sDescName)
    {
        sal_uInt16 nCount = pDoc->GetPageDescCnt();
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
    sal_Int16 nTmp;
    aValue >>= nTmp;
    sal_uInt16 nStt = (nTmp < 0 ? USHRT_MAX : (sal_uInt16)nTmp);
    SwDoc* pDoc = rCrsr.GetDoc();
    UnoActionContext aAction(pDoc);

    if( rCrsr.GetNext() != &rCrsr )         // Mehrfachselektion ?
    {
        pDoc->StartUndo( UNDO_START );
        SwPamRanges aRangeArr( rCrsr );
        SwPaM aPam( *rCrsr.GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
            pDoc->SetNodeNumStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), nStt );
        pDoc->EndUndo( UNDO_END );
    }
    else
        pDoc->SetNodeNumStart( *rCrsr.GetPoint(), nStt );
}

/* -----------------30.06.98 10:30-------------------
 *
 * --------------------------------------------------*/
sal_Int16 lcl_IsNodeNumStart(SwPaM& rPam, beans::PropertyState& eState)
{
    const SwTxtNode* pTxtNd = rPam.GetNode()->GetTxtNode();
    if( pTxtNd && pTxtNd->GetNum() && pTxtNd->GetNumRule() )
    {
        eState = beans::PropertyState_DIRECT_VALUE;
        sal_uInt16 nTmp = pTxtNd->GetNum()->GetSetValue();
        return USHRT_MAX == nTmp ? -1 : (sal_Int16)nTmp;
    }
    eState = beans::PropertyState_DEFAULT_VALUE;
    return -1;
}
/* -----------------25.05.98 11:41-------------------
 *
 * --------------------------------------------------*/
void lcl_setNumberingProperty(const uno::Any& rValue, SwPaM& rPam)
{
    if(rValue.getValueType() == ::getCppuType((const uno::Reference<container::XIndexReplace>*)0))
    {
        uno::Reference< XIndexReplace > * pxNum = (uno::Reference< XIndexReplace > *)rValue.getValue();
        if(!(*pxNum).is())
            return;
        SwXNumberingRules* pSwNum = 0;

        uno::Reference<lang::XUnoTunnel> xNumTunnel((*pxNum), uno::UNO_QUERY);
        if(xNumTunnel.is())
        {
            pSwNum = (SwXNumberingRules*)
                xNumTunnel->getSomething( SwXNumberingRules::getUnoTunnelId() );
        }

        if(pSwNum)
        {
            if(pSwNum->GetNumRule())
            {
                SwDoc* pDoc = rPam.GetDoc();
                SwNumRule aRule(*pSwNum->GetNumRule());
                const String* pNewCharStyles =  pSwNum->GetNewCharStyleNames();
                const String* pBulletFontNames = pSwNum->GetBulletFontNames();
                for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
                {
                    SwNumFmt aFmt(aRule.Get( i ));
                    if( pNewCharStyles[i].Len() &&
                        pNewCharStyles[i] != SwXNumberingRules::GetInvalidStyle() &&
                        (!aFmt.GetCharFmt() || pNewCharStyles[i] != aFmt.GetCharFmt()->GetName()))
                    {
                        if(!pNewCharStyles[i].Len())
                            aFmt.SetCharFmt(0);
                        else
                        {

                            // CharStyle besorgen und an der chaos::Rule setzen
                            sal_uInt16 nChCount = pDoc->GetCharFmts()->Count();
                            SwCharFmt* pCharFmt = 0;
                            for(sal_uInt16 i = 0; i< nChCount; i++)
                            {
                                SwCharFmt& rChFmt = *((*(pDoc->GetCharFmts()))[i]);;
                                if(rChFmt.GetName() == pNewCharStyles[i])
                                {
                                    pCharFmt = &rChFmt;
                                    break;
                                }
                            }

                            if(!pCharFmt)
                            {
                                SfxStyleSheetBasePool* pPool = pDoc->GetDocShell()->GetStyleSheetPool();
                                SfxStyleSheetBase* pBase;
                                pBase = pPool->Find(pNewCharStyles[i], SFX_STYLE_FAMILY_CHAR);
                            // soll das wirklich erzeugt werden?
                                if(!pBase)
                                    pBase = &pPool->Make(pNewCharStyles[i], SFX_STYLE_FAMILY_PAGE);
                                pCharFmt = ((SwDocStyleSheet*)pBase)->GetCharFmt();
                            }
                            if(pCharFmt)
                                aFmt.SetCharFmt(pCharFmt);
                        }
                    }
                    //jetzt nochmal fuer Fonts
                    if(pBulletFontNames[i] != SwXNumberingRules::GetInvalidStyle() &&
                        ((pBulletFontNames[i].Len() && !aFmt.GetBulletFont()) ||
                        pBulletFontNames[i].Len() &&
                                aFmt.GetBulletFont()->GetName() != pBulletFontNames[i] ))
                    {
                        const SvxFontListItem* pFontListItem =
                                (const SvxFontListItem* )pDoc->GetDocShell()
                                                    ->GetItem( SID_ATTR_CHAR_FONTLIST );
                        const FontList*  pList = pFontListItem->GetFontList();

                        FontInfo aInfo = pList->Get(
                            pBulletFontNames[i],WEIGHT_NORMAL, ITALIC_NONE);
                        Font aFont(aInfo);
                        aFmt.SetBulletFont(&aFont);
                    }
                    aRule.Set( i, aFmt );
                }
                UnoActionContext aAction(rPam.GetDoc());
                rPam.GetDoc()->SetNumRule( rPam, aRule );
            }
            else if(pSwNum->GetCreatedNumRuleName().Len())
            {
                SwDoc* pDoc = rPam.GetDoc();
                UnoActionContext aAction(pDoc);
                SwNumRule* pRule = pDoc->FindNumRulePtr( pSwNum->GetCreatedNumRuleName() );
                if(!pRule)
                    throw RuntimeException();
                pDoc->SetNumRule( rPam, *pRule );
            }
        }
    }
}

/* -----------------17.09.98 09:44-------------------
 *
 * --------------------------------------------------*/
sal_Bool lcl_setCrsrPropertyValue(const SfxItemPropertyMap* pMap,
                                SwPaM& rPam,
                                SfxItemSet& rSet,
                                const uno::Any& aValue,
                                sal_Bool& rPut ) throw (lang::IllegalArgumentException)
{
    sal_Bool bRet = sal_True;
    rPut = sal_True;
    if(aValue.getValueType() == ::getCppuVoidType())
        bRet = sal_False;
    else
    {
        switch(pMap->nWID)
        {
            case RES_TXTATR_CHARFMT:
                lcl_setCharStyle(rPam.GetDoc(), aValue, rSet);
            break;
            case FN_UNO_PARA_STYLE :
                lcl_SetTxtFmtColl(aValue, rPam);
                rPut = sal_False;
            break;
            case FN_UNO_PAGE_STYLE :
            break;
            case FN_UNO_NUM_START_VALUE  :
                lcl_SetNodeNumStart( rPam, aValue );
            break;
            case FN_UNO_NUM_LEVEL  :
            case FN_UNO_IS_NUMBER  :
            {
                SwTxtNode* pTxtNd = rPam.GetNode()->GetTxtNode();
                const SwNumRule* pRule = pTxtNd->GetNumRule();
                // hier wird Multiselektion nicht beruecksichtigt
                if( pRule && pTxtNd->GetNum() )
                {
                    if( FN_UNO_NUM_LEVEL == pMap->nWID)
                    {
                        sal_Int16 nLevel;
                        aValue >>= nLevel;
                        sal_Int16 nOldLevel = pTxtNd->GetNum()->GetLevel() & ~NO_NUMLEVEL;
                        if(nLevel < MAXLEVEL && nOldLevel != nLevel)
                        {
                            UnoActionContext aAction(rPam.GetDoc());
                            sal_Bool bDown = nLevel > nOldLevel;
                            sal_Int8 nMove = (sal_Int8)(bDown ? nLevel - nOldLevel : nOldLevel - nLevel);
                            while( nMove-- )
                            {
                                rPam.GetDoc()->NumUpDown( rPam, bDown );
                            }
                        }
                    }
                    else
                    {
                        BOOL bIsNumber = *(sal_Bool*) aValue.getValue();
                        SwNodeNum aNum = *pTxtNd->GetNum();
                        sal_Int16 nOldLevel = aNum.GetLevel() & ~NO_NUMLEVEL;
                        if(!bIsNumber)
                            nOldLevel |= NO_NUMLEVEL;
                        aNum.SetLevel(nOldLevel);
                        pTxtNd->UpdateNum( aNum );

                    }
                    rPut = sal_False;
                }
                //PROPERTY_MAYBEVOID!
            }
            break;
            case FN_NUMBER_NEWSTART :
            {
                sal_Bool bVal = *(sal_Bool*)aValue.getValue();
                rPam.GetDoc()->SetNumRuleStart(*rPam.GetPoint(), bVal);
            }
            break;
            case FN_UNO_NUM_RULES:
                lcl_setNumberingProperty(aValue, rPam);
                rPut = sal_False;
            break;
            case RES_PARATR_DROP:
            {
                if( MID_DROPCAP_CHAR_STYLE_NAME == pMap->nMemberId)
                {
                    if(aValue.getValueType() == ::getCppuType((const OUString*)0))
                    {
                        SwFmtDrop* pDrop = 0;
                        const SfxPoolItem* pItem;
                        if(SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_DROP, sal_True, &pItem ) )
                            pDrop = new SwFmtDrop(*((SwFmtDrop*)pItem));
                        if(!pDrop)
                            pDrop = new SwFmtDrop();
                        OUString uStyle;
                        aValue >>= uStyle;
                        String sStyle(SwXStyleFamilies::GetUIName(uStyle, SFX_STYLE_FAMILY_CHAR));
                        SwDocStyleSheet* pStyle =
                            (SwDocStyleSheet*)rPam.GetDoc()->GetDocShell()->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR);
                        if(pStyle)
                            pDrop->SetCharFmt(pStyle->GetCharFmt());
                        else
                             throw lang::IllegalArgumentException();
                        rSet.Put(*pDrop);
                        delete pDrop;
                    }
                    else
                        throw lang::IllegalArgumentException();
                }
                else
                    bRet = sal_False;
            }
            break;
            case RES_PAGEDESC      :
            if(MID_PAGEDESC_PAGEDESCNAME == pMap->nMemberId )
            {
                lcl_setPageDesc(rPam.GetDoc(), aValue, rSet);
                break;
            }
            //hier kein break
            default: bRet = sal_False;
        }
    }
return bRet;
}
/* -----------------30.03.99 10:52-------------------
 * spezielle Properties am Cursor zuruecksetzen
 * --------------------------------------------------*/
void    lcl_resetCrsrPropertyValue(const SfxItemPropertyMap* pMap, SwPaM& rPam)
{
    SwDoc* pDoc = rPam.GetDoc();
    switch(pMap->nWID)
    {
        case FN_UNO_PARA_STYLE :
//          lcl_SetTxtFmtColl(aValue, pUnoCrsr);
        break;
        case FN_UNO_PAGE_STYLE :
        break;
        case FN_UNO_NUM_START_VALUE  :
        {
            UnoActionContext aAction(pDoc);

            if( rPam.GetNext() != &rPam )           // Mehrfachselektion ?
            {
                pDoc->StartUndo( UNDO_START );
                SwPamRanges aRangeArr( rPam );
                SwPaM aPam( *rPam.GetPoint() );
                for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
                    pDoc->SetNodeNumStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), 1 );
                pDoc->EndUndo( UNDO_END );
            }
            else
                pDoc->SetNodeNumStart( *rPam.GetPoint(), 0 );
        }

        break;
        case FN_UNO_NUM_LEVEL  :
        break;
        case FN_UNO_NUM_RULES:
//          lcl_setNumberingProperty(aValue, pUnoCrsr);
        break;
    }
}

/* -----------------30.06.98 08:39-------------------
 *
 * --------------------------------------------------*/
SwFmtColl* SwXTextCursor::GetCurTxtFmtColl(SwPaM& rPam, BOOL bConditional)
{
    static const sal_uInt16 nMaxLookup = 255;
    SwFmtColl *pFmt = 0;

//  if ( GetCrsrCnt() > nMaxLookup )
//      return 0;
    sal_Bool bError = sal_False;
    FOREACHUNOPAM_START(&rPam)

        sal_uInt32 nSttNd = PUNOPAM->GetMark()->nNode.GetIndex(),
              nEndNd = PUNOPAM->GetPoint()->nNode.GetIndex();
        xub_StrLen nSttCnt = PUNOPAM->GetMark()->nContent.GetIndex(),
                nEndCnt = PUNOPAM->GetPoint()->nContent.GetIndex();

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

        const SwNodes& rNds = rPam.GetDoc()->GetNodes();
        for( sal_uInt32 n = nSttNd; n <= nEndNd; ++n )
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
    FOREACHUNOPAM_END()
    return bError ? 0 : pFmt;
}
/* -----------------04.07.98 15:15-------------------
 *
 * --------------------------------------------------*/
String lcl_GetCurPageStyle(SwPaM& rPaM)
{
    String sRet;
    const SwPageFrm* pPage = rPaM.GetCntntNode()->GetFrm()->FindPageFrm();
    if(pPage)
    {
        sRet = SwXStyleFamilies::GetProgrammaticName(
                    pPage->GetPageDesc()->GetName(), SFX_STYLE_FAMILY_PAGE);
    }
    return sRet;
}
/* -----------------25.05.98 11:40-------------------
 *
 * --------------------------------------------------*/
uno::Any  lcl_getNumberingProperty(SwPaM& rPam, beans::PropertyState& eState)
{
    uno::Any aAny;
    const SwNumRule* pNumRule = rPam.GetDoc()->GetCurrNumRule( *rPam.GetPoint() );
    if(pNumRule)
    {
        uno::Reference< container::XIndexReplace >  xNum = new SwXNumberingRules(*pNumRule);
        aAny.setValue(&xNum, ::getCppuType((const uno::Reference<container::XIndexReplace>*)0));
        eState = beans::PropertyState_DIRECT_VALUE;
    }
    else
        eState = beans::PropertyState_DEFAULT_VALUE;
    return aAny;
}

/* -----------------16.09.98 12:27-------------------
 *  Lesen spezieller Properties am Cursor
 * --------------------------------------------------*/
sal_Bool lcl_getCrsrPropertyValue(const SfxItemPropertyMap* pMap
                                        , SwPaM& rPam
                                        , const SfxItemSet& rSet
                                        , uno::Any& rAny
                                        , beans::PropertyState& eState )
{

    eState = beans::PropertyState_DIRECT_VALUE;
//    PropertyState_DEFAULT_VALUE
//    PropertyState_AMBIGUOUS_VALUE
    sal_Bool bDone = sal_True;
    switch(pMap->nWID)
    {
        case FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL:
        {
            SwFmtColl* pFmt = SwXTextCursor::GetCurTxtFmtColl(rPam, FALSE);
            sal_Int8 nRet = -1;
            if(pFmt && ((SwTxtFmtColl*)pFmt)->GetOutlineLevel() != NO_NUMBERING)
                nRet = ((SwTxtFmtColl*)pFmt)->GetOutlineLevel();
            rAny <<= nRet;
        }
        break;
        case FN_UNO_PARA_CONDITIONAL_STYLE_NAME:
        case FN_UNO_PARA_STYLE :
        {
            String sVal;
            SwFmtColl* pFmt = SwXTextCursor::GetCurTxtFmtColl(rPam, FN_UNO_PARA_CONDITIONAL_STYLE_NAME == pMap->nWID);
            if(pFmt)
                sVal = SwXStyleFamilies::GetProgrammaticName(pFmt->GetName(), SFX_STYLE_FAMILY_PARA);
            rAny <<= OUString(sVal);
            if(!sVal.Len())
                eState = beans::PropertyState_AMBIGUOUS_VALUE;
        }
        break;
        case FN_UNO_PAGE_STYLE :
        {
            String sVal = lcl_GetCurPageStyle(rPam);
            rAny <<= OUString(sVal);
            if(!sVal.Len())
                eState = beans::PropertyState_AMBIGUOUS_VALUE;
        }
        break;
        case FN_UNO_NUM_START_VALUE  :
        {
            sal_Int16 nValue = lcl_IsNodeNumStart(rPam, eState);
            rAny <<= nValue;
        }
        break;
        case FN_UNO_NUM_LEVEL  :
        case FN_UNO_IS_NUMBER  :
        case FN_NUMBER_NEWSTART:
        {
            const SwTxtNode* pTxtNd = rPam.GetNode()->GetTxtNode();
            const SwNumRule* pRule = pTxtNd->GetNumRule();
            // hier wird Multiselektion nicht beruecksichtigt
            if( pRule && pTxtNd->GetNum() )
            {
                if(pMap->nWID == FN_UNO_NUM_LEVEL)
                    rAny <<= (sal_Int16)(pTxtNd->GetNum()->GetLevel()&~NO_NUMLEVEL);
                else if(pMap->nWID == FN_UNO_IS_NUMBER)
                {
                    BOOL bIsNumber = 0 == (pTxtNd->GetNum()->GetLevel() & NO_NUMLEVEL);
                    rAny.setValue(&bIsNumber, ::getBooleanCppuType());
                }
                else /*if(pMap->nWID == UNO_NAME_PARA_IS_NUMBERING_RESTART)*/
                {
                    BOOL bIsRestart = pTxtNd->GetNum()->IsStart();
                    rAny.setValue(&bIsRestart, ::getBooleanCppuType());
                }
            }
            else
                eState = beans::PropertyState_DEFAULT_VALUE;
            //PROPERTY_MAYBEVOID!
        }
        break;
        case FN_UNO_NUM_RULES  :
            rAny = lcl_getNumberingProperty(rPam, eState);
        break;
        case RES_PAGEDESC      :
        if(MID_PAGEDESC_PAGEDESCNAME == pMap->nMemberId )
        {
            const SfxPoolItem* pItem;
            if(SFX_ITEM_SET == rSet.GetItemState( RES_PAGEDESC, sal_True, &pItem ) )
            {
                const SwPageDesc* pDesc = ((const SwFmtPageDesc*)pItem)->GetPageDesc();
                if(pDesc)
                    rAny <<= OUString(
                        SwXStyleFamilies::GetProgrammaticName(pDesc->GetName(), SFX_STYLE_FAMILY_PAGE));
            }
            else
                eState = beans::PropertyState_DEFAULT_VALUE;
        }
        else
            bDone = sal_False;
        break;
        case FN_UNO_DOCUMENT_INDEX_MARK:
        {
            SwTxtAttr* pTxtAttr = rPam.GetNode()->GetTxtNode()->GetTxtAttr(
                                rPam.GetPoint()->nContent, RES_TXTATR_TOXMARK);
            if(pTxtAttr)
            {
                const SwTOXMark& rMark = pTxtAttr->GetTOXMark();
                uno::Reference< XDocumentIndexMark >  xRef = SwXDocumentIndexMark::GetObject(
                        (SwTOXType*)rMark.GetTOXType(), &rMark, rPam.GetDoc());
                rAny.setValue(&xRef, ::getCppuType((uno::Reference<XDocumentIndex>*)0));
            }
            else
                //auch hier - nicht zu unterscheiden
                eState = beans::PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_DOCUMENT_INDEX:
        {
            const SwTOXBase* pBase = rPam.GetDoc()->GetCurTOX(
                                                    *rPam.Start() );
            if( pBase )
            {
                SwSectionFmt* pFmt = ((SwTOXBaseSection*)pBase)->GetFmt();
                SwClientIter aIter(*pFmt);
                SwXDocumentIndex* pxIdx = (SwXDocumentIndex*)aIter.First(TYPE(SwXDocumentIndex));
                uno::Reference< XDocumentIndex >  aRef;
                if(pxIdx)
                    aRef = pxIdx;
                else
                    aRef = new SwXDocumentIndex((const SwTOXBaseSection*)pBase, rPam.GetDoc() );
                rAny.setValue(&aRef, ::getCppuType((uno::Reference<XDocumentIndex>*)0));
            }
            else
                eState = beans::PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_FIELD:
        {
            const SwPosition *pPos = rPam.Start();
            const SwTxtNode *pTxtNd =
                rPam.GetDoc()->GetNodes()[pPos->nNode.GetIndex()]->GetTxtNode();
            SwTxtAttr* pTxtAttr =
                pTxtNd ? pTxtNd->GetTxtAttr(pPos->nContent, RES_TXTATR_FIELD)
                       : 0;
            if(pTxtAttr)
            {

                const SwFmtFld& rFld = pTxtAttr->GetFld();
                SwClientIter aIter(*rFld.GetFld()->GetTyp());
                SwXTextField* pFld = 0;
                SwXTextField* pTemp = (SwXTextField*)aIter.First(TYPE(SwXTextField));
                while(pTemp && !pFld)
                {
                    if(pTemp->GetFldFmt() == &rFld)
                        pFld = pTemp;
                    pTemp = (SwXTextField*)aIter.Next();
                }
                if(!pFld)
                    pFld = new SwXTextField( rFld, rPam.GetDoc());
                uno::Reference< XTextField >  xRet = pFld;
                rAny.setValue(&xRet, ::getCppuType((uno::Reference<XTextField>*)0));
            }
            else
                eState = beans::PropertyState_DEFAULT_VALUE;
        }
        break;
/*              laesst sich nicht feststellen
*               case FN_UNO_BOOKMARK:
        {
            if()
            {
                uno::Reference< XBookmark >  xBkm = SwXBookmarks::GetObject(rBkm);
                rAny.set(&xBkm, ::getCppuType((const XBookmark*)0)());
            }
        }
        break;*/
        case FN_UNO_TEXT_TABLE:
        case FN_UNO_CELL:
        {
            SwStartNode* pSttNode = rPam.GetNode()->FindStartNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();
            if(SwTableBoxStartNode == eType)
            {
                const SwTableNode* pTblNode = pSttNode->FindTableNode();
                SwFrmFmt* pTableFmt = (SwFrmFmt*)pTblNode->GetTable().GetFrmFmt();
                SwTable& rTable = ((SwTableNode*)pSttNode)->GetTable();
                if(FN_UNO_TEXT_TABLE == pMap->nWID)
                {
                    uno::Reference< XTextTable >  xTable = SwXTextTables::GetObject(*pTableFmt);
                    rAny.setValue(&xTable, ::getCppuType((uno::Reference<XTextTable>*)0));
                }
                else
                {
                    SwTableBox* pBox = pSttNode->GetTblBox();
                    uno::Reference< table::XCell >  xCell = SwXCell::CreateXCell(pTableFmt, pBox);
                    rAny.setValue(&xCell, ::getCppuType((uno::Reference<table::XCell>*)0));
                }
            }
            else
                eState = beans::PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_FRAME:
        {
            SwStartNode* pSttNode = rPam.GetNode()->FindStartNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();

            SwFrmFmt* pFmt;
            if(eType == SwFlyStartNode && 0 != (pFmt = pSttNode->GetFlyFmt()))
            {
                uno::Reference< XTextFrame >  xFrm = (SwXTextFrame*) SwXFrames::GetObject(*pFmt, FLYCNTTYPE_FRM);
                rAny.setValue(&xFrm, ::getCppuType((uno::Reference<XTextFrame>*)0));
            }
            else
                eState = beans::PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_SECTION:
        {
            SwSection* pSect = rPam.GetDoc()->GetCurrSection(*rPam.GetPoint());
            if(pSect)
            {
                uno::Reference< XTextSection >  xSect = SwXTextSections::GetObject( *pSect->GetFmt() );
                rAny.setValue(&xSect, ::getCppuType((uno::Reference<XTextSection>*)0) );
            }
            else
                eState = beans::PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_ENDNOTE:
        case FN_UNO_FOOTNOTE:
        {
            SwTxtAttr* pTxtAttr = rPam.GetNode()->GetTxtNode()->
                                        GetTxtAttr(rPam.GetPoint()->nContent, RES_TXTATR_FTN);
            if(pTxtAttr)
            {
                const SwFmtFtn& rFtn = pTxtAttr->GetFtn();
                if(rFtn.IsEndNote() == (FN_UNO_ENDNOTE == pMap->nWID))
                {
                    uno::Reference< XFootnote >  xFoot = new SwXFootnote(rPam.GetDoc(), rFtn);
                    rAny.setValue(&xFoot, ::getCppuType((uno::Reference<XFootnote>*)0));
                }
                else
                    eState = beans::PropertyState_DEFAULT_VALUE;
            }
            else
                eState = beans::PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_REFERENCE_MARK:
        {
            SwTxtAttr* pTxtAttr = rPam.GetNode()->GetTxtNode()->
                                        GetTxtAttr(rPam.GetPoint()->nContent, RES_TXTATR_REFMARK);
            if(pTxtAttr)
            {
                const SwFmtRefMark& rRef = pTxtAttr->GetRefMark();
                uno::Reference< XTextContent >  xRef = SwXReferenceMarks::GetObject( rPam.GetDoc(), &rRef );
                rAny.setValue(&xRef, ::getCppuType((uno::Reference<XTextContent>*)0));
            }
            else
                eState = beans::PropertyState_DEFAULT_VALUE;
        }
        break;
        case RES_TXTATR_CHARFMT:
        // kein break hier!
        default: bDone = sal_False;
    }
    return bDone;
}

/* -----------------26.06.98 16:20-------------------
 *  Hilfsfunktion fuer PageDesc
 * --------------------------------------------------*/
 SwPageDesc*    GetPageDescByName_Impl(SwDoc& rDoc, const String& rName)
 {
    SwPageDesc* pRet = 0;
    sal_uInt16 nDCount = rDoc.GetPageDescCnt();
    for( sal_uInt16 i = 0; i < nDCount; i++ )
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
        for(i = RC_POOLPAGEDESC_BEGIN; i <= STR_POOLPAGE_ENDNOTE; ++i)
        {
            const String aFmtName(SW_RES(i));
            if(aFmtName == rName)
            {
                pRet = rDoc.GetPageDescFromPool( RES_POOLPAGE_BEGIN + i - RC_POOLPAGEDESC_BEGIN );
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
/* -----------------24.02.99 14:18-------------------
 * Der StartNode muss in einem existierenden Header/Footen liegen
 * --------------------------------------------------*/
sal_Bool lcl_IsStarNodeInFormat(sal_Bool bHeader, SwStartNode* pSttNode,
    const SwFrmFmt* pFrmFmt, SwFrmFmt*& rpFormat)
{
    sal_Bool bRet = sal_False;
    const SfxItemSet& rSet = pFrmFmt->GetAttrSet();
    const SfxPoolItem* pItem;
    SwFrmFmt* pHeadFootFmt;
    if(SFX_ITEM_SET == rSet.GetItemState(bHeader ? RES_HEADER : RES_FOOTER, sal_True, &pItem) &&
            0 != (pHeadFootFmt = bHeader ?
                    ((SwFmtHeader*)pItem)->GetHeaderFmt() :
                                ((SwFmtFooter*)pItem)->GetFooterFmt()))
    {
        const SwFmtCntnt& rFlyCntnt = pHeadFootFmt->GetCntnt();
        const SwNode& rNode = rFlyCntnt.GetCntntIdx()->GetNode();
        const SwStartNode* pCurSttNode = rNode.FindSttNodeByType(
            bHeader ? SwHeaderStartNode : SwFooterStartNode);
        if(pCurSttNode && pCurSttNode == pSttNode)
        {
            bRet = sal_True;
            rpFormat = pHeadFootFmt;
        }
    }
    return bRet;
}

/* -----------------03.11.98 15:58-------------------
 *
 * --------------------------------------------------*/
uno::Reference< XTextRange >  CreateTextRangeFromPosition(SwDoc* pDoc,
                        const SwPosition& rPos, const SwPosition* pMark)
{
    uno::Reference< XTextRange >  aRet;
    SwUnoCrsr* pNewCrsr = pDoc->CreateUnoCrsr(rPos, sal_False);
    if(pMark)
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pMark;
    }
    uno::Reference< XText >  xParentText;
    //jetzt besorgen wir uns mal den Parent:
    SwStartNode* pSttNode = rPos.nNode.GetNode().FindStartNode();
    while(pSttNode && pSttNode->IsSectionNode())
    {
        pSttNode = pSttNode->FindStartNode();
    }
    SwStartNodeType eType = pSttNode->GetStartNodeType();
    switch(eType)
    {
        case SwTableBoxStartNode:
        {
            const SwTableNode* pTblNode = pSttNode->FindTableNode();
            SwFrmFmt* pTableFmt = (SwFrmFmt*)pTblNode->GetTable().GetFrmFmt();
            SwTableBox* pBox = pSttNode->GetTblBox();

            if( pBox )
                aRet = new SwXTextRange(*pTableFmt, *pBox, *pNewCrsr);
            else
                aRet = new SwXTextRange(*pTableFmt, *pSttNode, *pNewCrsr);
        }
        break;
        case SwFlyStartNode:
        {
            SwFrmFmt* pFmt;
            if(0 != (pFmt = pSttNode->GetFlyFmt()))
            {
                aRet = new SwXTextRange(*pFmt, *pNewCrsr);

            }
        }
        break;
        case SwHeaderStartNode:
        case SwFooterStartNode:
        {
            sal_Bool bHeader = SwHeaderStartNode == eType;
            sal_uInt16 nPDescCount = pDoc->GetPageDescCnt();
            for(sal_uInt16 i = 0; i < nPDescCount; i++)
            {
                const SwPageDesc& rDesc = pDoc->GetPageDesc( i );
                const SwFrmFmt* pFrmFmtMaster = &rDesc.GetMaster();
                const SwFrmFmt* pFrmFmtLeft = &rDesc.GetLeft();

                SwFrmFmt* pHeadFootFmt = 0;
                if(!lcl_IsStarNodeInFormat(bHeader, pSttNode, pFrmFmtMaster, pHeadFootFmt))
                    lcl_IsStarNodeInFormat(bHeader, pSttNode, pFrmFmtLeft, pHeadFootFmt);

                if(pHeadFootFmt)
                {
                    SwXHeadFootText* pxHdFt = (SwXHeadFootText*)SwClientIter( *pHeadFootFmt ).
                                    First( TYPE( SwXHeadFootText ));
                    xParentText = pxHdFt;
                    if(!pxHdFt)
                        xParentText = new SwXHeadFootText(*pHeadFootFmt, bHeader);
                    break;
                }
            }
        }
        break;
        case SwFootnoteStartNode:
        {
            sal_uInt16 n, nFtnCnt = pDoc->GetFtnIdxs().Count();
            SwTxtFtn* pTxtFtn;
            uno::Reference< XFootnote >  xRef;
            for( n = 0; n < nFtnCnt; ++n )
            {
                pTxtFtn = pDoc->GetFtnIdxs()[ n ];
                const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
                const SwTxtFtn* pTxtFtn = rFtn.GetTxtFtn();
#ifdef DEBUG
                const SwStartNode* pTmpSttNode = pTxtFtn->GetStartNode()->GetNode().
                                FindSttNodeByType(SwFootnoteStartNode);
#endif

                if(pSttNode == pTxtFtn->GetStartNode()->GetNode().
                                    FindSttNodeByType(SwFootnoteStartNode))
                {
                    aRet = new SwXFootnote(pDoc, rFtn);
                    break;
                }
            }
        }
        break;
        default:
        {
            // dann ist es der Body-Text
            uno::Reference<frame::XModel> xModel = pDoc->GetDocShell()->GetBaseModel();
            uno::Reference< XTextDocument > xDoc(
                xModel, uno::UNO_QUERY);
            xParentText = xDoc->getText();
        }
    }
    if(!aRet.is())
        aRet = new SwXTextRange(*pNewCrsr, xParentText);
    return aRet;
}
/******************************************************************
 * SwXTextCursor
 ******************************************************************/
/*-----------------24.03.98 14:49-------------------

--------------------------------------------------*/
uno::Reference< uno::XInterface >  SwXTextCursor_NewInstance_Impl()
{
    return (cppu::OWeakObject*)new SwXTextCursor();
};
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
         sServiceName.EqualsAscii("com.sun.star.style.ParagraphProperties");
}
/*-- 09.12.98 14:19:19---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXTextCursor::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextCursor");
     pArray[1] = C2U("com.sun.star.style.CharacterProperties");
     pArray[2] = C2U("com.sun.star.style.ParagraphProperties");
    return aRet;
}
/*-- 09.12.98 14:19:19---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextCursor::SwXTextCursor(uno::Reference< XText >  xParent, const SwPosition& rPos,
                    CursorType eSet, SwDoc* pDoc, const SwPosition* pMark) :
    aLstnrCntnr(( util::XSortable*)this),
    xParentText(xParent),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    pLastSortOptions(0),
    eType(eSet)
{
    SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(rPos, sal_False);
    if(pMark)
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *pMark;
    }
    pUnoCrsr->Add(this);
}
/*-- 09.12.98 14:19:19---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextCursor::SwXTextCursor(SwXText* pParent) :
    xParentText(pParent),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    pLastSortOptions(0),
    eType(CURSOR_INVALID),
    aLstnrCntnr( (util::XSortable*)this)
{

}
/* -----------------04.03.99 09:02-------------------
 *
 * --------------------------------------------------*/
SwXTextCursor::SwXTextCursor(uno::Reference< XText >  xParent,
    SwUnoCrsr* pSourceCrsr, CursorType eSet) :
    aLstnrCntnr( (util::XSortable*)this),
    xParentText(xParent),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    pLastSortOptions(0),
    eType(eSet)
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
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    delete pUnoCrsr;
    delete pLastSortOptions;
}
/*-- 09.12.98 14:19:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::DeleteAndInsert(const String& rText)
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        // Start/EndAction
        SwDoc* pDoc = pUnoCrsr->GetDoc();
        UnoActionContext aAction(pDoc);
        xub_StrLen nTxtLen = rText.Len();
        pDoc->StartUndo(UNDO_INSERT);
        SwCursor *_pStartCrsr = pUnoCrsr;
        do
        {
            if(_pStartCrsr->HasMark())
            {
                pDoc->DeleteAndJoin(*_pStartCrsr);
            }
            if(nTxtLen)
            {
                //OPT: GetSystemCharSet
                if( !pDoc->Insert(*_pStartCrsr, rText) )
                    ASSERT( sal_False, "Doc->Insert(Str) failed." );
                    SwXTextCursor::SelectPam(*pUnoCrsr, sal_True);
                    _pStartCrsr->Left(rText.Len());
            }
        } while( (_pStartCrsr=(SwCursor*)_pStartCrsr->GetNext()) != pUnoCrsr );
        pDoc->EndUndo(UNDO_INSERT);
    }
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
            return (sal_Int64)this;
    }
    return 0;
}

/*-- 09.12.98 14:18:12---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::collapseToStart(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if(*pUnoCrsr->GetPoint() < *pUnoCrsr->GetMark())
            pUnoCrsr->Exchange();
        if(pUnoCrsr->HasMark())
            pUnoCrsr->DeleteMark();
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
        bRet = pUnoCrsr->Left( nCount);
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
        bRet = pUnoCrsr->Right(nCount);
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
                pTblNode = pCont->FindTableNode();
            }
            if(pCont)
                pUnoCrsr->GetPoint()->nContent.Assign(pCont, 0);
            const SwStartNode* pTmp = pUnoCrsr->GetNode()->FindStartNode();
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
                eType == CURSOR_FOOTNOTE)
        {
            pUnoCrsr->MoveSection( fnSectionCurr, fnSectionStart);
        }
        else
            DBG_WARNING("not implemented")
            ;
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
            pUnoCrsr->Move( fnMoveForward, fnGoDoc );
        else if(eType == CURSOR_FRAME ||
                eType == CURSOR_TBLTEXT ||
                eType == CURSOR_HEADER ||
                eType == CURSOR_FOOTER ||
                eType == CURSOR_FOOTNOTE)
        {
            pUnoCrsr->MoveSection( fnSectionCurr, fnSectionEnd);
        }
        else
            DBG_WARNING("not implemented");
            ;
    }
    else
    {
        throw uno::RuntimeException();
    }
}
/* -----------------05.03.99 07:27-------------------
 *
 * --------------------------------------------------*/
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
    SwXTextCursor* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (SwXTextCursor*)xRangeTunnel->getSomething(
                                SwXTextCursor::getUnoTunnelId());
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
    }
    const SwStartNode* pOwnStartNode = pOwnCursor->GetNode()->
                                            FindSttNodeByType(eSearchNodeType);

    const SwNode* pSrcNode = 0;
    if(pCursor && pCursor->GetCrsr())
    {
        pSrcNode = pCursor->GetCrsr()->GetNode();
    }
    else if(pRange && pRange->GetBookmark())
    {
        SwBookmark* pBkm = pRange->GetBookmark();
        pSrcNode = &pBkm->GetPos().nNode.GetNode();
    }
    const SwStartNode* pTmp = pSrcNode ? pSrcNode->FindSttNodeByType(eSearchNodeType) : 0;

    //SectionNodes ueberspringen
    while(pTmp && pTmp->IsSectionNode())
    {
        pTmp = pTmp->FindStartNode();
    }
    while(pOwnStartNode && pOwnStartNode->IsSectionNode())
    {
        pOwnStartNode = pOwnStartNode->FindStartNode();
    }
    if(pOwnStartNode != pTmp)
    {
        throw uno::RuntimeException();
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
        SwPosition* pParamLeft;
        SwPosition* pParamRight;
        if(pCursor)
        {
            const SwUnoCrsr* pTmp = pCursor->GetCrsr();
            pParamLeft = new SwPosition(*pTmp->GetPoint());
            pParamRight = new SwPosition(pTmp->HasMark() ? *pTmp->GetMark() : *pParamLeft);
        }
        else
        {
            SwBookmark* pBkm = pRange->GetBookmark();
            pParamLeft = new SwPosition(pBkm->GetPos());
            pParamRight = new SwPosition(pBkm->GetOtherPos() ? *pBkm->GetOtherPos() : *pParamLeft);
        }
        if(*pParamRight < *pParamLeft)
        {
            SwPosition* pTmp = pParamLeft;
            pParamLeft = pParamRight;
            pParamRight = pTmp;
        }
        // jetzt sind vier SwPositions da, zwei davon werden gebraucht, also welche?
        if(aOwnRight < *pParamRight)
            *pOwnCursor->GetPoint() = aOwnRight;
        else
            *pOwnCursor->GetPoint() = *pParamRight;
        pOwnCursor->SetMark();
        if(aOwnLeft < *pParamLeft)
            *pOwnCursor->GetMark() = *pParamLeft;
        else
            *pOwnCursor->GetMark() = aOwnLeft;
        delete pParamLeft;
        delete pParamRight;
    }
    else
    {
        //der Cursor soll dem uebergebenen Range entsprechen
        if(pCursor)
        {
            const SwUnoCrsr* pTmp = pCursor->GetCrsr();
            *pOwnCursor->GetPoint() = *pTmp->GetPoint();
            if(pTmp->HasMark())
            {
                pOwnCursor->SetMark();
                *pOwnCursor->GetMark() = *pTmp->GetMark();
            }
            else
                pOwnCursor->DeleteMark();
        }
        else
        {
            SwBookmark* pBkm = pRange->GetBookmark();
            *pOwnCursor->GetPoint() = pBkm->GetPos();
            if(pBkm->GetOtherPos())
            {
                pOwnCursor->SetMark();
                *pOwnCursor->GetMark() = *pBkm->GetOtherPos();
            }
            else
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
        bRet = pUnoCrsr->IsStartWord();
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
        bRet = pUnoCrsr->IsEndWord();
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
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        //Absatzende?
        if(pUnoCrsr->GetCntntNode() &&
                pUnoCrsr->GetPoint()->nContent == pUnoCrsr->GetCntntNode()->Len())
                bRet = pUnoCrsr->Right();
        else
        {
            bRet = pUnoCrsr->GoNextWord();
            //if there is no next word within the current paragraph try to go to the start of the next paragraph
            if(!bRet)
                bRet = pUnoCrsr->MovePara(fnParaNext, fnParaStart);
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
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        //Absatzanfang ?
        if(pUnoCrsr->GetPoint()->nContent == 0)
            bRet = pUnoCrsr->Left();
        else
        {
            bRet = pUnoCrsr->GoPrevWord();
            if(pUnoCrsr->GetPoint()->nContent == 0)
                pUnoCrsr->Left();
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
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        if(!pUnoCrsr->IsEndWord())
        {
            bRet = pUnoCrsr->GoEndWord();
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
    sal_Bool bRet = FALSE;
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        if(!pUnoCrsr->IsStartWord())
        {
            bRet = pUnoCrsr->GoStartWord();
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
        // Absatzanfang?
        bRet = pUnoCrsr->GetPoint()->nContent == 0;
        // mitMarkierung ->kein Satzanfang
        if(!bRet && !pUnoCrsr->HasMark())
        {
            SwCursor aCrsr(*pUnoCrsr->GetPoint());
            aCrsr.LeftRight(sal_True, 1);
            aCrsr.SetMark();
            if(aCrsr.LeftRight(sal_True, 1))
            {
                SwTxtNode* pTxtNd = aCrsr.GetNode()->GetTxtNode();
                if( pTxtNd )
                {
                    xub_StrLen nStt = aCrsr.Start()->nContent.GetIndex();
                    String aTxt = pTxtNd->GetExpandTxt( nStt,
                        aCrsr.End()->nContent.GetIndex() - nStt );
                    char cChar = aTxt.GetChar(0);
                    switch(cChar)
                    {
                        case ';':
                        case '.':
                        case ':':
                        case '!':
                        case '?':
                            bRet = sal_True;
                        break;
                    }
                }
            }
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
        //Absatzende?
        bRet = pUnoCrsr->GetCntntNode() &&
                pUnoCrsr->GetPoint()->nContent == pUnoCrsr->GetCntntNode()->Len();
        // mitMarkierung ->kein Satzende
        if(!bRet && !pUnoCrsr->HasMark())
        {
            SwCursor aCrsr(*pUnoCrsr->GetPoint());
            aCrsr.SetMark();
            if(aCrsr.LeftRight(sal_True, 1))
            {
                SwTxtNode* pTxtNd = aCrsr.GetNode()->GetTxtNode();
                if( pTxtNd )
                {
                    xub_StrLen nStt = aCrsr.Start()->nContent.GetIndex();
                    String aTxt = pTxtNd->GetExpandTxt( nStt,
                        aCrsr.End()->nContent.GetIndex() - nStt );
                    char cChar = aTxt.GetChar(0);
                    switch(cChar)
                    {
                        case ';':
                        case '.':
                        case ':':
                        case '!':
                        case '?':
                            bRet = sal_True;
                        break;
                    }
                }
            }
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
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        bRet = pUnoCrsr->GoSentence(SwCursor::NEXT_SENT);
        if(!bRet)
            bRet = pUnoCrsr->MovePara(fnParaNext, fnParaStart);
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
        bRet = lcl_IsStartOfPara(*pUnoCrsr)
            || pUnoCrsr->GoSentence(SwCursor::START_SENT) ||
            lcl_IsStartOfPara(*pUnoCrsr);
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
        sal_Bool bAlreadyParaEnd = lcl_IsEndOfPara(*pUnoCrsr);
        bRet = !bAlreadyParaEnd &&
                    (pUnoCrsr->GoSentence(SwCursor::END_SENT) ||
                        pUnoCrsr->MovePara(fnParaCurr, fnParaEnd));

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
        bRet = lcl_IsStartOfPara(*pUnoCrsr);
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
        bRet = lcl_IsEndOfPara(*pUnoCrsr);
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
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr )
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        bRet = lcl_IsStartOfPara(*pUnoCrsr);
        if(!bRet)
            bRet = pUnoCrsr->MovePara(fnParaCurr, fnParaStart);
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 09.12.98 14:18:49---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::gotoEndOfParagraph(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SelectPam(*pUnoCrsr, Expand);
        bRet = lcl_IsEndOfPara(*pUnoCrsr);
        if(!bRet)
            pUnoCrsr->MovePara(fnParaCurr, fnParaEnd);
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 09.12.98 14:18:50---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextCursor::gotoNextParagraph(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
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
        xRet = new SwXTextRange(aPam, xParent);
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
        xRet = new SwXTextRange(aPam, xParent);
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

    EXCEPT_ON_PROTECTION(*pUnoCrsr)

    DeleteAndInsert(aString);
}
/* -----------------------------03.05.00 12:56--------------------------------

 ---------------------------------------------------------------------------*/
Any SwXTextCursor::GetPropertyValue(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet, const OUString& rPropertyName)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aAny;
    SfxItemSet aSet(rPaM.GetDoc()->GetAttrPool(),
        RES_CHRATR_BEGIN,   RES_PARATR_NUMRULE,
        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
        RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
        RES_FILL_ORDER,     RES_FRMATR_END -1,
        0L);
    SwXTextCursor::GetCrsrAttr(rPaM, aSet);

    String aPropertyName(rPropertyName);
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                rPropSet.getPropertyMap(), rPropertyName);
    BOOL bDone = FALSE;
    if(pMap)
    {
        PropertyState eTemp;
        bDone = lcl_getCrsrPropertyValue(pMap, rPaM, aSet, aAny, eTemp );
        if(!bDone)
            aAny = rPropSet.getPropertyValue(aPropertyName, aSet);
    }
    else
        throw UnknownPropertyException();
    return aAny;
}
/* -----------------------------03.05.00 12:57--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextCursor::SetPropertyValue(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet, const OUString& rPropertyName, const Any& aValue)
        throw (UnknownPropertyException, PropertyVetoException,
            IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    Any aAny;
    EXCEPT_ON_PROTECTION(rPaM)

    SwDoc* pDoc = rPaM.GetDoc();
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                            rPropSet.getPropertyMap(), rPropertyName);
    if(pMap)
    {
        SfxItemSet rSet(pDoc->GetAttrPool(),
            pMap->nWID, pMap->nWID);
        SwXTextCursor::GetCrsrAttr(rPaM, rSet);
        BOOL bDef = FALSE;
        BOOL bPut;
        if(!lcl_setCrsrPropertyValue(pMap, rPaM, rSet, aValue, bPut ))
            rPropSet.setPropertyValue(rPropertyName, aValue, rSet);
        if(bPut)
            SwXTextCursor::SetCrsrAttr(rPaM, rSet);
    }
    else
        throw UnknownPropertyException();
}

PropertyState lcl_SwXTextCursor_GetPropertyState( SfxItemSet** ppSet,
                                    SfxItemSet** ppSetParent,
                                    SwPaM& rPaM,
                                    SfxItemPropertySet& rPropSet,
                                    const OUString& rPropertyName )
{
    PropertyState eRet = PropertyState_DEFAULT_VALUE;
    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(
                            rPropSet.getPropertyMap(), rPropertyName );
    if( pMap )
    {
        String aPropertyName( rPropertyName );

        if( !*ppSet )
        {
            *ppSet = new SfxItemSet( rPaM.GetDoc()->GetAttrPool(),
                    RES_CHRATR_BEGIN,   RES_PARATR_NUMRULE,
                    RES_FILL_ORDER,     RES_FRMATR_END -1,
                    RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                    RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                    0L );
            SwXTextCursor::GetCrsrAttr( rPaM, **ppSet, FALSE );
        }

        Any aAny;
        BOOL bDone = lcl_getCrsrPropertyValue(pMap, rPaM, **ppSet, aAny, eRet );
        if(!bDone)
            eRet = rPropSet.getPropertyState( aPropertyName,**ppSet );

        //try again to find out if a value has been inherited
        if( beans::PropertyState_DIRECT_VALUE == eRet )
        {
            if( !*ppSetParent )
            {
                *ppSetParent = (*ppSet)->Clone( FALSE );
                SwXTextCursor::GetCrsrAttr( rPaM, **ppSetParent, TRUE );
            }
            bDone = lcl_getCrsrPropertyValue( pMap, rPaM, **ppSetParent,
                                                aAny, eRet );
            if( !bDone )
                eRet = rPropSet.getPropertyState( aPropertyName,
                                                  **ppSetParent );
        }
    }
    else
        throw UnknownPropertyException();
    return eRet;
}

/* -----------------------------03.05.00 13:16--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< PropertyState > SwXTextCursor::GetPropertyStates(
            SwPaM& rPaM, SfxItemPropertySet& rPropSet,
            const Sequence< OUString >& PropertyNames)
            throw(UnknownPropertyException, RuntimeException)
{
    const OUString* pNames = PropertyNames.getConstArray();
    Sequence< PropertyState > aRet(PropertyNames.getLength());
    PropertyState* pStates = aRet.getArray();

    SfxItemSet *pSet = 0, *pSetParent = 0;
    for( INT32 i = 0, nEnd = PropertyNames.getLength(); i < nEnd; i++ )
        pStates[i] = ::lcl_SwXTextCursor_GetPropertyState( &pSet, &pSetParent,
                                            rPaM, rPropSet, pNames[i] );
    delete pSet;
    delete pSetParent;

    return aRet;
}
/* -----------------------------03.05.00 13:17--------------------------------

 ---------------------------------------------------------------------------*/
PropertyState SwXTextCursor::GetPropertyState(
    SwPaM& rPaM, SfxItemPropertySet& rPropSet, const OUString& rPropertyName)
                        throw(UnknownPropertyException, RuntimeException)
{
    PropertyState eRet;
    SfxItemSet *pSet = 0, *pSetParent = 0;
    eRet = ::lcl_SwXTextCursor_GetPropertyState( &pSet, &pSetParent,
                                       rPaM, rPropSet, rPropertyName );
    delete pSet;
    delete pSetParent;

    return eRet;
}
/* -----------------------------03.05.00 13:20--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextCursor::SetPropertyToDefault(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const OUString& rPropertyName)
    throw(UnknownPropertyException, RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());
    SwDoc* pDoc = rPaM.GetDoc();
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                            rPropSet.getPropertyMap(), rPropertyName);
    if(pMap)
    {
        if(pMap->nWID < RES_FRMATR_END)
        {
            SvUShortsSort aWhichIds;
            aWhichIds.Insert(pMap->nWID);
            if(pMap->nWID < RES_PARATR_BEGIN)
                pDoc->ResetAttr(rPaM, TRUE, &aWhichIds);
            else
            {
                //fuer Absatzattribute muss die Selektion jeweils auf
                //Absatzgrenzen erweitert werden
                SwPosition aStart = *rPaM.Start();
                SwPosition aEnd = *rPaM.End();
                SwUnoCrsr* pTemp = pDoc->CreateUnoCrsr(aStart, FALSE);
                if(!lcl_IsStartOfPara(*pTemp))
                {
                    pTemp->MovePara(fnParaCurr, fnParaStart);
                }
                pTemp->SetMark();
                *pTemp->GetPoint() = aEnd;
                //pTemp->Exchange();
                SwXTextCursor::SelectPam(*pTemp, TRUE);
                if(!lcl_IsEndOfPara(*pTemp))
                {
                    pTemp->MovePara(fnParaCurr, fnParaEnd);
                }
                pDoc->ResetAttr(*pTemp, TRUE, &aWhichIds);
                delete pTemp;
            }
        }
        else
            lcl_resetCrsrPropertyValue(pMap, rPaM);
    }
    else
        throw UnknownPropertyException();
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
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                            rPropSet.getPropertyMap(), rPropertyName);
    if(pMap)
    {
        if(pMap->nWID < RES_FRMATR_END)
        {
            const SfxPoolItem& rDefItem =
                pDoc->GetAttrPool().GetDefaultItem(pMap->nWID);
            rDefItem.QueryValue(aRet, pMap->nMemberId);
        }
    }
    else
        throw UnknownPropertyException();
    return aRet;
}
/*-- 09.12.98 14:18:54---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXTextCursor::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef;
    if(!xRef.is())
    {
        static SfxItemPropertyMap aCrsrExtMap_Impl[] =
        {
            { SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT), FN_SKIP_HIDDEN_TEXT, &::getBooleanCppuType(), PROPERTY_NONE,     0},
            { SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT), FN_SKIP_PROTECTED_TEXT, &::getBooleanCppuType(), PROPERTY_NONE,     0},
            {0,0,0,0}
        };
        uno::Reference< beans::XPropertySetInfo >  xInfo = aPropSet.getPropertySetInfo();
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
    uno::Any aAny;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if(!rPropertyName.compareToAscii(UNO_NAME_IS_SKIP_HIDDEN_TEXT.pName))
        {
            sal_Bool bSet = *(sal_Bool*)aValue.getValue();
            pUnoCrsr->SetSkipOverHiddenSections(bSet);
        }
        else if(!rPropertyName.compareToAscii(UNO_NAME_IS_SKIP_PROTECTED_TEXT.pName))
        {
            sal_Bool bSet = *(sal_Bool*)aValue.getValue();
            pUnoCrsr->SetSkipOverProtectSections(bSet);
        }
        else
            SetPropertyValue(*pUnoCrsr, aPropSet, rPropertyName, aValue);
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
        if(!rPropertyName.compareToAscii(UNO_NAME_IS_SKIP_HIDDEN_TEXT.pName))
        {
            BOOL bSet = pUnoCrsr->IsSkipOverHiddenSections();
            aAny.setValue(&bSet, ::getBooleanCppuType());
        }
        else if(!rPropertyName.compareToAscii(UNO_NAME_IS_SKIP_PROTECTED_TEXT.pName))
        {
            BOOL bSet = pUnoCrsr->IsSkipOverProtectSections();
            aAny.setValue(&bSet, ::getBooleanCppuType());
        }
        else
            aAny = GetPropertyValue(*pUnoCrsr, aPropSet, rPropertyName);
    }
    else
        throw uno::RuntimeException();
    return aAny;

}
/*-- 09.12.98 14:18:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 09.12.98 14:18:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 09.12.98 14:18:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 09.12.98 14:18:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
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
        eRet = GetPropertyState(*pUnoCrsr, aPropSet, rPropertyName);
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
    return GetPropertyStates(*pUnoCrsr, aPropSet, PropertyNames);
}
/*-- 05.03.99 11:36:12---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::setPropertyToDefault(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SetPropertyToDefault(*pUnoCrsr, aPropSet, rPropertyName);
    }
    else
        throw uno::RuntimeException();
}
/*-- 05.03.99 11:36:12---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextCursor::getPropertyDefault(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        aRet = GetPropertyDefault(*pUnoCrsr, aPropSet, rPropertyName);
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 09.12.98 14:18:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::insertDocumentFromURL(const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& aOptions)
    throw( lang::IllegalArgumentException, io::IOException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        String sFilterName;
        String sFilterOption;
        String sPassword;
        OUString uFilterName(C2U(   UNO_NAME_FILTER_NAME    ));
        OUString uFilterOption(C2U( UNO_NAME_FILTER_OPTION  ));
        OUString uPassword(C2U( UNO_NAME_PASSWORD ));
        sal_Bool bIllegalArgument = sal_False;

        for ( int n = 0; n < aOptions.getLength(); ++n )
        {
            // get Property-Value from options
            const beans::PropertyValue &rProp = aOptions.getConstArray()[n];
            uno::Any aValue( rProp.Value );

            // FilterName-Property?
            if ( rProp.Name == uFilterName )
            {
                if ( rProp.Value.getValueType() == ::getCppuType((const OUString*)0))
                {
                    OUString uFilterName;
                    rProp.Value >>= uFilterName;
                    sFilterName = String(uFilterName);
                }
                else if ( rProp.Value.getValueType() != ::getVoidCppuType() )
                    bIllegalArgument = sal_True;
            }
            else if( rProp.Name == uFilterOption )
            {
                if ( rProp.Value.getValueType() == ::getCppuType((const OUString*)0))
                {
                    OUString uFilterOption;
                    rProp.Value >>= uFilterOption;
                    sFilterOption = String(uFilterOption) ;
                }
                else if ( rProp.Value.getValueType() != ::getVoidCppuType() )

                    bIllegalArgument = sal_True;
            }
            else if( rProp.Name == uPassword )
            {
                if ( rProp.Value.getValueType() == ::getCppuType((const OUString*)0))
                {
                    OUString uPassword;
                    rProp.Value >>= uPassword;
                    sPassword = String(uPassword );
                }
                else if ( rProp.Value.getValueType() != ::getVoidCppuType() )
                    bIllegalArgument = sal_True;
            }
            else if(rProp.Value.getValueType() != ::getVoidCppuType())
                bIllegalArgument = sal_True;
        }
        if(bIllegalArgument)
            throw lang::IllegalArgumentException();
        lcl_InsertFile(pUnoCrsr, rURL, sFilterName, sFilterOption, sPassword);
    }
    else
        throw uno::RuntimeException();
}
/*-- 09.12.98 14:18:58---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue > SwXTextCursor::createSortDescriptor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence< beans::PropertyValue > aRet(12);
    beans::PropertyValue* pArray = aRet.getArray();

    uno::Any aVal;
    sal_Bool bFalse = sal_False;
    sal_Bool bTrue = sal_True;
    aVal.setValue( &bFalse, ::getCppuBooleanType());
    pArray[0] = beans::PropertyValue(C2U("IsSortInTable"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    String sSpace(String::CreateFromAscii(" "));
    sal_Unicode uSpace = sSpace.GetChar(0);
    aVal.setValue( &uSpace, ::getCppuCharType());
    pArray[1] = beans::PropertyValue(C2U("Delimiter"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal.setValue( &bTrue, ::getCppuBooleanType());
    pArray[2] = beans::PropertyValue(C2U("SortColumns"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal <<= (INT16)1;
    pArray[3] = beans::PropertyValue(C2U("SortRowOrColumnNo0"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal.setValue( &bTrue, ::getCppuBooleanType());
    pArray[4] = beans::PropertyValue(C2U("IsSortNumeric0"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal.setValue( &bTrue, ::getCppuBooleanType());
    pArray[5] = beans::PropertyValue(C2U("IsSortAscending0"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal <<= (INT16)1;
    pArray[6] = beans::PropertyValue(C2U("SortRowOrColumnNo1"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal.setValue( &bTrue, ::getCppuBooleanType());
    pArray[7] = beans::PropertyValue(C2U("IsSortNumeric1"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal.setValue( &bTrue, ::getCppuBooleanType());
    pArray[8] = beans::PropertyValue(C2U("IsSortAscending1"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal <<= (INT16)1;
    pArray[9] = beans::PropertyValue(C2U("SortRowOrColumnNo2"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal.setValue( &bTrue, ::getCppuBooleanType());
    pArray[10] = beans::PropertyValue(C2U("IsSortNumeric2"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal.setValue( &bTrue, ::getCppuBooleanType());
    pArray[11] = beans::PropertyValue(C2U("IsSortAscending2"), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    return aRet;
}
/*-- 09.12.98 14:19:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    const beans::PropertyValue* pProperties = rDescriptor.getConstArray();
    SwSortOptions aSortOpt;

    aSortOpt.bTable = sal_False;
    aSortOpt.nDeli = ' ';
    aSortOpt.eDirection = SRT_COLUMNS;

    aSortOpt.aKeys;
    SwSortKey* pKey1 = new SwSortKey;
    pKey1->nColumnId = USHRT_MAX;
    pKey1->eSortKeyType = SRT_NUMERIC;
    pKey1->eSortOrder   = SRT_ASCENDING;

    SwSortKey* pKey2 = new SwSortKey;
    pKey2->nColumnId = USHRT_MAX;
    pKey2->eSortKeyType = SRT_NUMERIC;
    pKey2->eSortOrder   = SRT_ASCENDING;

    SwSortKey* pKey3 = new SwSortKey;
    pKey3->nColumnId = USHRT_MAX;
    pKey3->eSortKeyType = SRT_NUMERIC;
    pKey3->eSortOrder   = SRT_ASCENDING;
    SwSortKey* aKeys[3] = {pKey1, pKey2, pKey3};

    for( int n = 0; n < rDescriptor.getLength(); ++n )
    {
        uno::Any aValue( pProperties[n].Value );
//      String sPropName = pProperties[n].Name;
        const OUString& rPropName = pProperties[n].Name;
        if( COMPARE_EQUAL == rPropName.compareToAscii("IsSortInTable"))
        {
            if ( aValue.getValueType() == ::getBooleanCppuType() )
            {
                aValue >>= aSortOpt.bTable;
            }
            else
                throw uno::RuntimeException();
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("Delimiter"))
        {
            if ( aValue.getValueType() == ::getCppuType((sal_Unicode*)0))
            {
                sal_Unicode uChar;
                aValue >>= uChar;
                aSortOpt.nDeli = uChar;
            }
            else
                throw uno::RuntimeException();
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("SortColumns"))
        {
            if ( aValue.getValueType() == ::getBooleanCppuType() )
            {
                sal_Bool bTemp;
                aValue >>= bTemp;
                aSortOpt.eDirection = bTemp ? SRT_COLUMNS : SRT_ROWS;
            }
            else
                throw uno::RuntimeException();
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("SortRowOrColumnNo", 17) &&
            rPropName.getLength() == 18 &&
            (rPropName.getStr()[17] >= '0' && rPropName.getStr()[17] <= '9'))
        {
            sal_uInt16 nIndex = rPropName.getStr()[17];
            nIndex -= '0';
            sal_Int16 nCol = -1;
            if( aValue.getValueType() == ::getCppuType((const sal_Int16*)0) && nIndex < 3)
                aValue >>= nCol;
            if( nCol >= 0 )
            {
                aKeys[nIndex]->nColumnId = nCol;
            }
            else
                throw uno::RuntimeException();
        }
        else if(0 == rPropName.search(C2U("IsSortNumeric")) &&
            rPropName.getLength() == 14 &&
            (rPropName.getStr()[13] >= '0' && rPropName.getStr()[13] <= '9'))
        {
            sal_uInt16 nIndex = rPropName.getStr()[13];
            nIndex = nIndex - '0';
            if ( aValue.getValueType() == ::getBooleanCppuType() && nIndex < 3 )
            {
                sal_Bool bTemp;
                aValue >>= bTemp;
                aKeys[nIndex]->eSortKeyType = bTemp ? SRT_NUMERIC : SRT_APLHANUM;
            }
            else
                throw uno::RuntimeException();
        }
        else if(0 == rPropName.search(C2U("IsSortAscending")) && rPropName.getLength() == 16 &&
            lcl_IsNumeric(String(rPropName[(sal_uInt16)15])))
        {
            sal_uInt16 nIndex = rPropName.getStr()[13];
            if ( aValue.getValueType() == ::getBooleanCppuType() && nIndex < 3 )
            {
                sal_Bool bTemp;
                aValue >>= bTemp;
                aKeys[nIndex]->eSortOrder = bTemp ? SRT_ASCENDING : SRT_DESCENDING;
            }
            else
                throw uno::RuntimeException();
        }
    }
    if(pKey1->nColumnId != USHRT_MAX)
        aSortOpt.aKeys.C40_INSERT(SwSortKey, pKey1, aSortOpt.aKeys.Count());
    if(pKey2->nColumnId != USHRT_MAX)
        aSortOpt.aKeys.C40_INSERT(SwSortKey, pKey2, aSortOpt.aKeys.Count());
    if(pKey3->nColumnId != USHRT_MAX)
        aSortOpt.aKeys.C40_INSERT(SwSortKey, pKey3, aSortOpt.aKeys.Count());

    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr && aSortOpt.aKeys.Count())
    {
        if(pUnoCrsr->HasMark())
        {
            UnoActionContext aContext( pUnoCrsr->GetDoc() );
            pUnoCrsr->GetDoc()->SortText(*pUnoCrsr, aSortOpt);
        }
    }
    else
        throw uno::RuntimeException();
}
/*-- 10.12.98 11:52:15---------------------------------------------------

  -----------------------------------------------------------------------*/
void ClientModify(SwClient* pClient, SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        if( (void*)pClient->GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            ((SwModify*)pClient->GetRegisteredIn())->Remove(pClient);
        break;

    case RES_FMT_CHG:
        // wurden wir an das neue umgehaengt und wird das alte geloscht?
        if( ((SwFmtChg*)pNew)->pChangedFmt == pClient->GetRegisteredIn() &&
            ((SwFmtChg*)pOld)->pChangedFmt->IsFmtInDTOR() )
            ((SwModify*)pClient->GetRegisteredIn())->Remove(pClient);
        break;
    }
}

/* -----------------------------03.04.00 09:11--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XEnumeration >  SAL_CALL SwXTextCursor::createContentEnumeration(const OUString& rServiceName) throw( RuntimeException )
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( !pUnoCrsr || 0 != rServiceName.compareToAscii("com.sun.star.text.TextContent") )
        throw RuntimeException();

    Reference< XEnumeration > xRet = new SwXParaFrameEnumeration(*pUnoCrsr, PARAFRAME_PORTION_TEXTRANGE);
    return xRet;
}
/* -----------------------------03.04.00 09:11--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SAL_CALL SwXTextCursor::getAvailableServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString::createFromAscii("com.sun.star.text.TextContent");
    return aRet;
}
/*-- 09.12.98 14:19:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwXTextCursor::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        aLstnrCntnr.Disposing();

}
/*-- 09.12.98 14:19:01---------------------------------------------------

  -----------------------------------------------------------------------*/
const SwPaM*    SwXTextCursor::GetPaM() const
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

/*-- 09.12.98 14:19:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::SetCrsrAttr(SwPaM& rPam, const SfxItemSet& rSet, sal_Bool bTableMode)
{
    sal_uInt16 nFlags = 0;//???
    SwDoc* pDoc = rPam.GetDoc();
    //StartEndAction
    UnoActionContext aAction(pDoc);
    SwPaM* pCrsr = &rPam;
    if( pCrsr->GetNext() != pCrsr )     // Ring von Cursorn
    {
        pDoc->StartUndo(UNDO_INSATTR);

        SwPaM *_pStartCrsr = &rPam;
        do
        {
            if( _pStartCrsr->HasMark() && ( bTableMode ||
                *_pStartCrsr->GetPoint() != *_pStartCrsr->GetMark() ))
                pDoc->Insert(*_pStartCrsr, rSet, nFlags );
        } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != &rPam );

        pDoc->EndUndo(UNDO_INSATTR);
    }
    else
    {
//          if( !HasSelection() )
//              UpdateAttr();
        pDoc->Insert( *pCrsr, rSet, nFlags );
    }
}
/*-- 09.12.98 14:19:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::GetCrsrAttr(SwPaM& rPam, SfxItemSet& rSet, BOOL bCurrentAttrOnly)
{
static const sal_uInt16 nMaxLookup = 255;
    SfxItemSet aSet( *rSet.GetPool(), rSet.GetRanges() );
    SfxItemSet *pSet = &rSet;
    SwPaM *_pStartCrsr = &rPam;
    do
    {
        sal_uInt32 nSttNd = _pStartCrsr->GetMark()->nNode.GetIndex(),
                    nEndNd = _pStartCrsr->GetPoint()->nNode.GetIndex();
            xub_StrLen nSttCnt = _pStartCrsr->GetMark()->nContent.GetIndex(),
                    nEndCnt = _pStartCrsr->GetPoint()->nContent.GetIndex();

            if( nSttNd > nEndNd || ( nSttNd == nEndNd && nSttCnt > nEndCnt ))
            {
                sal_uInt32 nTmp = nSttNd; nSttNd = nEndNd; nEndNd = nTmp;
                nTmp = nSttCnt; nSttCnt = nEndCnt; nEndCnt = (sal_uInt16)nTmp;
            }

            if( nEndNd - nSttNd >= nMaxLookup )
            {
                rSet.ClearItem();
                rSet.InvalidateAllItems();
                return;// uno::Any();
            }

            // beim 1.Node traegt der Node die Werte in den GetSet ein (Initial)
            // alle weiteren Nodes werden zum GetSet zu gemergt
            for( sal_uInt32 n = nSttNd; n <= nEndNd; ++n )
            {
                SwNode* pNd = rPam.GetDoc()->GetNodes()[ n ];
                switch( pNd->GetNodeType() )
                {
                case ND_TEXTNODE:
                    {
                        xub_StrLen nStt = n == nSttNd ? nSttCnt : 0,
                                nEnd = n == nEndNd ? nEndCnt
                                            : ((SwTxtNode*)pNd)->GetTxt().Len();
                        ((SwTxtNode*)pNd)->GetAttr( *pSet, nStt, nEnd, bCurrentAttrOnly, !bCurrentAttrOnly );
                    }
                    break;
                case ND_GRFNODE:
                case ND_OLENODE:
                    ((SwCntntNode*)pNd)->GetAttr( *pSet );
                    break;

                default:
                    pNd = 0;
                }

                if( pNd )
                {
                    if( pSet != &rSet )
                        rSet.MergeValues( aSet );

                    if( aSet.Count() )
                        aSet.ClearItem();

                }
                pSet = &aSet;
            }
    } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != &rPam );
}
/******************************************************************
 * SwXParagraphEnumeration
 ******************************************************************/
/* -----------------------------06.04.00 16:33--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXParagraphEnumeration::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXParagraphEnumeration");
}
/* -----------------------------06.04.00 16:33--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXParagraphEnumeration::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.ParagraphEnumeration") == rServiceName;
}
/* -----------------------------06.04.00 16:33--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXParagraphEnumeration::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.ParagraphEnumeration");
    return aRet;
}
/*-- 10.12.98 11:52:12---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraphEnumeration::SwXParagraphEnumeration(SwXText* pParent,
                                                    SwPosition& rPos,
                                                    CursorType eType) :
        xParentText(pParent),
        bFirstParagraph(sal_True),
        eCursorType(eType)
{
    SwUnoCrsr* pUnoCrsr = pParent->GetDoc()->CreateUnoCrsr(rPos, sal_False);
    pUnoCrsr->Add(this);
}

/*-- 10.12.98 11:52:12---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraphEnumeration::SwXParagraphEnumeration(SwXText* pParent,
                                                SwUnoCrsr*  pCrsr,
                                                CursorType eType) :
        SwClient(pCrsr),
        xParentText(pParent),
        bFirstParagraph(sal_True),
        eCursorType(eType)
{
}
/*-- 10.12.98 11:52:12---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraphEnumeration::~SwXParagraphEnumeration()
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        delete pUnoCrsr;

}
/*-- 10.12.98 11:52:13---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXParagraphEnumeration::hasMoreElements(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if(bFirstParagraph)
            bRet = sal_True;
        else
        {
            SwPosition* pStart = pUnoCrsr->GetPoint();
            SwUnoCrsr* pNewCrsr = pUnoCrsr->GetDoc()->CreateUnoCrsr(*pStart, sal_False);
            //man soll hier auch in Tabellen landen duerfen
            if(CURSOR_TBLTEXT != eCursorType)
                pNewCrsr->SetRemainInSection( sal_False );

            //was mache ich, wenn ich schon in einer Tabelle stehe?
            SwStartNode* pSttNode = pUnoCrsr->GetNode()->FindStartNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();
            if(CURSOR_TBLTEXT != eCursorType &&
                                SwTableBoxStartNode == eType)
            {
                // wir haben es mit einer Tabelle zu tun
                FASTBOOL bMoved = pNewCrsr->MoveTable( fnTableCurr, fnTableEnd );
            }
            bRet = pNewCrsr->MovePara(fnParaNext, fnParaStart);
            delete pNewCrsr;
        }
    }
    return bRet;
}
/*-- 10.12.98 11:52:14---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXParagraphEnumeration::nextElement(void)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextContent >  aRef;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
         XText* pText = xParentText.get();
        sal_Bool bInTable = sal_False;
        if(!bFirstParagraph)
        {
            //man soll hier auch in Tabellen landen duerfen
            if(CURSOR_TBLTEXT != eCursorType)
            {
                pUnoCrsr->SetRemainInSection( sal_False );
                //was mache ich, wenn ich schon in einer Tabelle stehe?
                SwTableNode* pTblNode = pUnoCrsr->GetNode()->FindTableNode();
                if(pTblNode)
                {
                    // wir haben es mit einer Tabelle zu tun - also ans Ende
                    FASTBOOL bMoved = pUnoCrsr->MoveTable( fnTableCurr, fnTableEnd );
                    pUnoCrsr->GetPoint()->nNode = pTblNode->EndOfSectionIndex();
                    if(!pUnoCrsr->Move(fnMoveForward, fnGoNode))
                    {
                        throw container::NoSuchElementException();
                    }
                    else
                        bInTable = sal_True;

                }
            }
        }

        if( bFirstParagraph || bInTable || pUnoCrsr->MovePara(fnParaNext, fnParaStart))
        {
            bFirstParagraph = sal_False;
            SwPosition* pStart = pUnoCrsr->GetPoint();
            //steht man nun in einer Tabelle, oder in einem einfachen Absatz?

            SwTableNode* pTblNode = pUnoCrsr->GetNode()->FindTableNode();
            if(CURSOR_TBLTEXT != eCursorType && pTblNode)
            {
                // wir haben es mit einer Tabelle zu tun
                SwFrmFmt* pTableFmt = (SwFrmFmt*)pTblNode->GetTable().GetFrmFmt();
             XTextTable* pTable = SwXTextTables::GetObject( *pTableFmt );
                aRef =  (XTextContent*)(SwXTextTable*)pTable;
            }
            else
            {
                SwUnoCrsr* pNewCrsr = pUnoCrsr->GetDoc()->CreateUnoCrsr(*pStart, sal_False);
                aRef =  (XTextContent*)new SwXParagraph((SwXText*)pText, pNewCrsr);
            }
        }
        else
            throw container::NoSuchElementException();

    }
    else
        throw uno::RuntimeException();

    uno::Any aRet(&aRef, ::getCppuType((uno::Reference<XTextContent>*)0));
    return aRet;
}
void SwXParagraphEnumeration::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

/******************************************************************
 * SwXTextRange
 ******************************************************************/
TYPEINIT1(SwXTextRange, SwClient);

/* -----------------10.12.98 13:19-------------------
 *
 * --------------------------------------------------*/
/*uno::Reference< uno::XInterface >  SwXTextRange_NewInstance_Impl()
{
    return *new SwXTextRange();
};
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextRange::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
    //XUnoTunnel
sal_Int64 SAL_CALL SwXTextRange::getSomething(
    const uno::Sequence< sal_Int8 >& rId )
        throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
        {
                return (sal_Int64)this;
        }
    return 0;
}
/* -----------------------------06.04.00 16:34--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextRange::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextRange");
}
/* -----------------------------06.04.00 16:34--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextRange::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    String sServiceName(rServiceName);
    return sServiceName.EqualsAscii("com.sun.star.text.TextRange") ||
         sServiceName.EqualsAscii("com.sun.star.style.CharacterProperties")||
         sServiceName.EqualsAscii("com.sun.star.style.ParagraphProperties");
}
/* -----------------------------06.04.00 16:34--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextRange::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextRange");
     pArray[1] = C2U("com.sun.star.style.CharacterProperties");
     pArray[2] = C2U("com.sun.star.style.ParagraphProperties");
    return aRet;
}
/*-- 10.12.98 12:54:42---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextRange::SwXTextRange() :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    pDoc(0),
    aObjectDepend(this, 0),
    pBox(0),
    pBoxStartNode(0),
    eRangePosition(RANGE_INVALID)
{
}
/*-- 10.12.98 12:54:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextRange::SwXTextRange(SwPaM& rPam, const uno::Reference< XText > & rxParent) :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    xParentText(rxParent),
    aObjectDepend(this, 0),
    pDoc(rPam.GetDoc()),
    pBox(0),
    pBoxStartNode(0),
    eRangePosition(RANGE_IN_TEXT)
{
    //Bookmark an der anlegen
    _CreateNewBookmark(rPam);
}
/*-- 10.12.98 12:54:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextRange::SwXTextRange(SwFrmFmt& rFmt, SwPaM& rPam) :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    aObjectDepend(this, &rFmt),
    pDoc(rPam.GetDoc()),
    pBox(0),
    pBoxStartNode(0),
    eRangePosition(RANGE_IN_FRAME)
{
    //Bookmark an der anlegen
    _CreateNewBookmark(rPam);
}
/*-- 10.12.98 12:54:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextRange::SwXTextRange(SwFrmFmt& rTblFmt, SwTableBox& rTblBox, SwPaM& rPam) :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    pDoc(rPam.GetDoc()),
    aObjectDepend(this, &rTblFmt),
    pBox(&rTblBox),
    pBoxStartNode(0),
    eRangePosition(RANGE_IN_CELL)
{
    //Bookmark an der anlegen
    _CreateNewBookmark(rPam);
}
/* -----------------------------09.08.00 16:07--------------------------------

 ---------------------------------------------------------------------------*/
SwXTextRange::SwXTextRange(SwFrmFmt& rTblFmt, const SwStartNode& rStartNode, SwPaM& rPam) :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    pDoc(rPam.GetDoc()),
    aObjectDepend(this, &rTblFmt),
    pBox(0),
    pBoxStartNode(&rStartNode),
    eRangePosition(RANGE_IN_CELL)
{
    //Bookmark an der anlegen
    _CreateNewBookmark(rPam);
}
/* -----------------19.02.99 11:39-------------------
 *
 * --------------------------------------------------*/
SwXTextRange::SwXTextRange(SwFrmFmt& rTblFmt) :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    aObjectDepend(this, &rTblFmt),
    pDoc(rTblFmt.GetDoc()),
    pBox(0),
    pBoxStartNode(0),
    eRangePosition(RANGE_IS_TABLE)
{
}

/*-- 10.12.98 12:54:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextRange::~SwXTextRange()
{
    if(GetBookmark())
        pDoc->DelBookmark( GetBookmark()->GetName() );
}
/*-- 10.12.98 12:54:44---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwXTextRange::_CreateNewBookmark(SwPaM& rPam)
{
    SwBookmark* pBkm = GetBookmark();
    if(pBkm)
        pDoc->DelBookmark( pBkm->GetName() );
    KeyCode aCode;
    String sBookmarkName(C2S("SwXTextPosition"));
    String sShortName;
    rPam.GetDoc()->MakeUniqueBookmarkName(sBookmarkName);

    SwBookmark* pMark = pDoc->MakeBookmark(rPam, aCode,
                sBookmarkName, sShortName, UNO_BOOKMARK);
    pMark->Add(this);
}
/*-- 10.12.98 12:54:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwXTextRange::DeleteAndInsert(const String& rText) throw( uno::RuntimeException )
{
    SwBookmark* pBkm = GetBookmark();
    if(pBkm )
    {
        const SwPosition& rPoint = pBkm->GetPos();
        const SwPosition* pMark = pBkm->GetOtherPos();
        SwCursor aNewCrsr( rPoint);
        SwDoc* pDoc = aNewCrsr.GetDoc();
        if(pMark)
        {
            aNewCrsr.SetMark();
            *aNewCrsr.GetMark() = *pMark;
        }
        EXCEPT_ON_PROTECTION(aNewCrsr)

        UnoActionContext aAction(aNewCrsr.GetDoc());
        pDoc->StartUndo(UNDO_INSERT);
        if(aNewCrsr.HasMark())
            pDoc->DeleteAndJoin(aNewCrsr);

        if(rText.Len())
        {
            if( !pDoc->Insert(aNewCrsr, rText) )
                ASSERT( sal_False, "Doc->Insert(Str) failed." );
            SwXTextCursor::SelectPam(aNewCrsr, sal_True);
            aNewCrsr.Left(rText.Len());
        }
        _CreateNewBookmark(aNewCrsr);
        pDoc->EndUndo(UNDO_INSERT);
    }

}

/*-- 10.12.98 12:54:46---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XText >  SwXTextRange::getText(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!xParentText.is())
    {
        if(eRangePosition == RANGE_IN_FRAME &&
            aObjectDepend.GetRegisteredIn())
        {
            SwFrmFmt* pFrmFmt = (SwFrmFmt*)aObjectDepend.GetRegisteredIn();
            SwXTextFrame* pxFrm = (SwXTextFrame*)SwClientIter( *pFrmFmt ).
                                                First( TYPE( SwXTextFrame ));
            if(pxFrm)
                ((SwXTextRange*)this)->xParentText = pxFrm;
            else
                ((SwXTextRange*)this)->xParentText = new SwXTextFrame(*pFrmFmt);
            ((SwModify*)aObjectDepend.GetRegisteredIn())->
                    Remove(&((SwXTextRange*)this)->aObjectDepend);

        }
        else if(eRangePosition == RANGE_IN_CELL &&
            aObjectDepend.GetRegisteredIn())
        {
            const SwStartNode* pSttNd = pBoxStartNode ? pBoxStartNode : pBox->GetSttNd();
            const SwTableNode* pTblNode = pSttNd->FindTableNode();
            const SwFrmFmt* pTableFmt = pTblNode->GetTable().GetFrmFmt();
            ((SwXTextRange*)this)->xParentText =
                    pBox ? SwXCell::CreateXCell((SwFrmFmt*)pTableFmt, pBox)
                         : new SwXCell( (SwFrmFmt*)pTableFmt, *pBoxStartNode );
            ((SwModify*)aObjectDepend.GetRegisteredIn())->
                    Remove(&((SwXTextRange*)this)->aObjectDepend);
        }
        else if(eRangePosition == RANGE_IN_CELL &&
            aObjectDepend.GetRegisteredIn())
        {
            //die Tabelle muss schon einen Parent kennen!
            DBG_WARNING("not implemented")
        }
        else
        {
            // jetzt noch alle Faelle finden, die nicht abgedeckt sind
            // (Body, Kopf-/Fusszeilen, Fussnotentext )
            DBG_WARNING("not implemented")
        }
    }
    return xParentText;
}
/*-- 10.12.98 12:54:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextRange::getStart(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  xRet;
    SwBookmark* pBkm = GetBookmark();
    if(pBkm)
    {
        SwPaM aPam(pBkm->GetPos());
        xRet = new SwXTextRange(aPam, xParentText);
    }
    else if(eRangePosition == RANGE_IS_TABLE)
    {
        //start und ende sind mit this identisch, wenn es eine Tabelle ist
        xRet = this;
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 10.12.98 12:54:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextRange::getEnd(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  xRet;
    SwBookmark* pBkm = GetBookmark();
    if(pBkm)
    {
        SwPaM aPam(pBkm->GetOtherPos()? *pBkm->GetOtherPos() : pBkm->GetPos());
        xRet = new SwXTextRange(aPam, xParentText);
    }
    else if(eRangePosition == RANGE_IS_TABLE)
    {
        //start und ende sind mit this identisch, wenn es eine Tabelle ist
        xRet = this;
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 10.12.98 12:54:47---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextRange::getString(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwBookmark* pBkm = GetBookmark();
    OUString sRet;
    //fuer Tabellen gibt es keine Bookmark, also auch keinen Text
    //evtl. koennte man hier die Tabelle als ASCII exportieren?
    if(pBkm && pBkm->GetOtherPos())
    {
        const SwPosition& rPoint = pBkm->GetPos();
        const SwPosition* pMark = pBkm->GetOtherPos();
        SwPaM aCrsr(*pMark, rPoint);
/*      if( rPoint.nNode.GetIndex() ==
            pMark->nNode.GetIndex() )
        {
            SwTxtNode* pTxtNd = aCrsr.GetNode()->GetTxtNode();
            if( pTxtNd )
            {
                sal_uInt16 nStt = aCrsr.Start()->nContent.GetIndex();
                sRet = pTxtNd->GetExpandTxt( nStt,
                        aCrsr.End()->nContent.GetIndex() - nStt );
            }
        }
        else
*/      {
            SwXTextCursor::getTextFromPam(aCrsr, sRet);
        }
    }
    return sRet;
}
/*-- 10.12.98 12:54:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextRange::setString(const OUString& aString)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(RANGE_IN_CELL == eRangePosition)
    {
        //setString in Tabellen kann nicht erlaubt werden
        throw uno::RuntimeException();
    }
    else
        DeleteAndInsert(aString);
}
/*-- 10.12.98 12:54:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextRange::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    sal_Bool bAlreadyRegisterred = 0 != GetRegisteredIn();
    ClientModify(this, pOld, pNew);
    if(aObjectDepend.GetRegisteredIn())
    {
        ClientModify(&aObjectDepend, pOld, pNew);
        // if the depend was removed then the range must be removed too
        if(!aObjectDepend.GetRegisteredIn() && GetRegisteredIn())
            ((SwModify*)GetRegisteredIn())->Remove(this);
        // or if the range has been removed but the depend ist still
        // connected then the depend must be removed
        else if(bAlreadyRegisterred && !GetRegisteredIn() &&
                aObjectDepend.GetRegisteredIn())
            ((SwModify*)aObjectDepend.GetRegisteredIn())->Remove(&aObjectDepend);
    }
}
/*-- 10.12.98 12:54:49---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool    SwXTextRange::GetPositions(SwPaM& rToFill) const
{
    sal_Bool bRet = sal_False;
    SwBookmark* pBkm = GetBookmark();
    if(pBkm)
    {
        *rToFill.GetPoint() = pBkm->GetPos();
        if(pBkm->GetOtherPos())
        {
            rToFill.SetMark();
            *rToFill.GetMark() = *pBkm->GetOtherPos();
        }
        else
            rToFill.DeleteMark();
        bRet = sal_True;
    }
    return bRet;
}
/*-- 10.12.98 12:54:49---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool        SwXTextRange::XTextRangeToSwPaM( SwUnoInternalPaM& rToFill,
                            const uno::Reference< XTextRange > & xTextRange)
{
    sal_Bool bRet = sal_False;

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    SwXTextCursor* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (SwXTextCursor*)xRangeTunnel->getSomething(
                                SwXTextCursor::getUnoTunnelId());
    }

    if(pRange && pRange->GetDoc() == rToFill.GetDoc())
    {
        bRet = pRange->GetPositions(rToFill);
    }
    else
    {
        const SwPaM* pPam = 0;
        if(pCursor && pCursor->GetDoc() == rToFill.GetDoc() &&
            0 != (pPam = pCursor->GetPaM()))
            {
                DBG_ASSERT((SwPaM*)pPam->GetNext() == pPam, "was machen wir mit Ringen?" )
                bRet = sal_True;
                *rToFill.GetPoint() = *pPam->GetPoint();
                if(pPam->HasMark())
                {
                    rToFill.SetMark();
                    *rToFill.GetMark() = *pPam->GetMark();
                }
                else
                    rToFill.DeleteMark();
            }
    }
    return bRet;
}
/* -----------------------------03.04.00 09:11--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XEnumeration >  SAL_CALL SwXTextRange::createContentEnumeration(
        const OUString& rServiceName)
                throw( RuntimeException )
{
    SwBookmark* pBkm = GetBookmark();
    if( !pBkm || COMPARE_EQUAL != rServiceName.compareToAscii("com.sun.star.text.TextContent") )
        throw RuntimeException();

    const SwPosition& rPoint = pBkm->GetPos();
    const SwPosition* pMark = pBkm->GetOtherPos();
    SwUnoCrsr* pNewCrsr = pDoc->CreateUnoCrsr(rPoint, FALSE);
    if(pMark && *pMark != rPoint)
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pMark;
    }
    Reference< XEnumeration > xRet = new SwXParaFrameEnumeration(*pNewCrsr, PARAFRAME_PORTION_TEXTRANGE);
    delete pNewCrsr;
    return xRet;
}
/* -----------------------------03.04.00 09:11--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SAL_CALL SwXTextRange::getAvailableServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString::createFromAscii("com.sun.star.text.TextContent");
    return aRet;
}
/*-- 03.05.00 12:41:46---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo > SAL_CALL SwXTextRange::getPropertySetInfo(  ) throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static Reference< XPropertySetInfo > xRef =
        aPropSet.getPropertySetInfo();
    return xRef;
}
/*-- 03.05.00 12:41:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXTextRange::setPropertyValue(
    const OUString& rPropertyName, const Any& rValue )
    throw(UnknownPropertyException, PropertyVetoException,
        IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(GetDoc()->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    SwXTextCursor::SetPropertyValue(aPaM, aPropSet, rPropertyName, rValue);
}
/*-- 03.05.00 12:41:47---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SAL_CALL SwXTextRange::getPropertyValue( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    return SwXTextCursor::GetPropertyValue(aPaM, aPropSet, rPropertyName);
}
/*-- 03.05.00 12:41:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXTextRange::addPropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING("not implemented")
}
/*-- 03.05.00 12:41:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXTextRange::removePropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING("not implemented")
}
/*-- 03.05.00 12:41:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXTextRange::addVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING("not implemented")
}
/*-- 03.05.00 12:41:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXTextRange::removeVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING("not implemented")
}
/*-- 03.05.00 12:41:48---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyState SAL_CALL SwXTextRange::getPropertyState( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    return SwXTextCursor::GetPropertyState(aPaM, aPropSet, rPropertyName);
}
/*-- 03.05.00 12:41:49---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< PropertyState > SAL_CALL SwXTextRange::getPropertyStates(
    const Sequence< OUString >& rPropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    return SwXTextCursor::GetPropertyStates(aPaM, aPropSet, rPropertyName);
}
/*-- 03.05.00 12:41:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXTextRange::setPropertyToDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    SwXTextCursor::SetPropertyToDefault(aPaM, aPropSet, rPropertyName);
}
/*-- 03.05.00 12:41:50---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SAL_CALL SwXTextRange::getPropertyDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    return SwXTextCursor::GetPropertyDefault(aPaM, aPropSet, rPropertyName);
}

/******************************************************************
 * SwXTextRanges
 ******************************************************************/
/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextRanges::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextRanges::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/****************************************************************************
 *  Text positions
 *  Bis zum ersten Zugriff auf eine TextPosition wird ein SwCursor gehalten,
 * danach wird ein Array mit uno::Reference< XTextPosition >  angelegt
 *
****************************************************************************/
SV_IMPL_PTRARR(XTextRangeArr, XTextRangeRefPtr);
/* -----------------------------06.04.00 16:36--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextRanges::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextRanges");
}
/* -----------------------------06.04.00 16:36--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextRanges::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextRanges") == rServiceName;
}
/* -----------------------------06.04.00 16:36--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextRanges::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextRanges");
    return aRet;
}
/*-- 10.12.98 13:57:20---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextRanges::SwXTextRanges() :
    pRangeArr(0)
{

}
/*-- 10.12.98 13:57:21---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextRanges::SwXTextRanges( SwUnoCrsr* pCrsr, uno::Reference< XText >  xParent) :
    SwClient(pCrsr),
    xParentText(xParent),
    pRangeArr(0)
{

}
/*-- 10.12.98 13:57:22---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextRanges::SwXTextRanges(SwPaM* pCrsr) :
    pRangeArr(0)
{
    SwUnoCrsr* pUnoCrsr = pCrsr->GetDoc()->CreateUnoCrsr(*pCrsr->GetPoint());
    if(pCrsr->HasMark())
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *pCrsr->GetMark();
    }
    if(pCrsr->GetNext() != pCrsr)
    {
        SwPaM *_pStartCrsr = (SwPaM *)pCrsr->GetNext();
        do
        {
            //neuen PaM erzeugen
            SwPaM* pPaM = _pStartCrsr->HasMark() ?
                        new SwPaM(*_pStartCrsr->GetMark(), *_pStartCrsr->GetPoint()) :
                            new SwPaM(*_pStartCrsr->GetPoint());
            //und in den Ring einfuegen
            pPaM->MoveTo(pUnoCrsr);

        } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != pCrsr );
    }

    pUnoCrsr->Add(this);
}
/*-- 10.12.98 13:57:22---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextRanges::~SwXTextRanges()
{
    SwUnoCrsr* pCrsr = GetCrsr();
    delete pCrsr;
    if(pRangeArr)
    {
        pRangeArr->DeleteAndDestroy(0, pRangeArr->Count());
        delete pRangeArr;
    }
}
/*-- 10.12.98 13:57:24---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXTextRanges::getCount(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int32 nRet = 0;
    SwUnoCrsr* pCrsr = GetCrsr();
    if(pCrsr)
    {
        FOREACHUNOPAM_START(pCrsr)
            nRet++;
        FOREACHUNOPAM_END()
    }
    else if(pRangeArr)
        nRet = pRangeArr->Count();
    return nRet;
}
/*-- 10.12.98 13:57:25---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextRanges::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  aRef;
    XTextRangeArr* pArr = ((SwXTextRanges*)this)->GetRangesArray();
    if(pArr && pArr->Count() > nIndex)
    {
        XTextRangeRefPtr pRef = pArr->GetObject(nIndex);
        aRef = *pRef;
    }
    else
        throw lang::IndexOutOfBoundsException();
    uno::Any aRet(&aRef, ::getCppuType((uno::Reference<XTextRange>*)0));
    return aRet;
}
/*-- 10.12.98 13:57:25---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type  SwXTextRanges::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}
/*-- 10.12.98 13:57:26---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextRanges::hasElements(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return getCount() > 0;
}
/* -----------------10.12.98 14:25-------------------
 *
 * --------------------------------------------------*/
XTextRangeArr*  SwXTextRanges::GetRangesArray()
{
    SwUnoCrsr* pCrsr = GetCrsr();
    if(!pRangeArr && pCrsr)
    {
        pRangeArr = new XTextRangeArr();
        FOREACHUNOPAM_START(pCrsr)
            uno::Reference< XTextRange > * pPtr = new uno::Reference< XTextRange > ;
            // in welcher Umgebung steht denn der PaM?
            SwStartNode* pSttNode = PUNOPAM->GetNode()->FindStartNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();
            uno::Reference< XText >  xCurParentRef;
            switch(eType)
            {
                case SwTableBoxStartNode:
                {
                    SwTableBox* pBox = pSttNode->GetTblBox();
                    *pPtr = new SwXTextRange(*pBox->GetFrmFmt(), *pBox, *PUNOPAM);
                }
                break;
                case SwFlyStartNode:
                {
                    SwFrmFmt* pFlyFmt = pSttNode->GetFlyFmt();
                    SwXTextFrame* pxFrm = (SwXTextFrame*)SwClientIter( *pFlyFmt ).
                                                First( TYPE( SwXTextFrame ));
                    if(pxFrm)
                        xCurParentRef = pxFrm;
                    else
                        *pPtr = new SwXTextRange(*pFlyFmt, *PUNOPAM);
                }
                break;
                case SwFootnoteStartNode:
                {
                    const SwFtnIdxs& rIdxs = pCrsr->GetDoc()->GetFtnIdxs();
                    sal_uInt16 n, nFtnCnt = rIdxs.Count();
                    for( n = 0; n < nFtnCnt; ++n )
                    {
                        const SwTxtFtn* pFtn = rIdxs[ n ];

                        const SwStartNode* pTemp = 0;
                        const SwNode& rNode = pFtn->GetStartNode()->GetNode();
                        if(rNode.GetNodeType() == ND_STARTNODE)
                            pTemp = (const SwStartNode*)&rNode;
                        if(pSttNode == pTemp)
                        {
                            const SwFmtFtn& rFtn = pFtn->GetFtn();
                            //TODO: schon existierendes Fussnotenobjekt wiederfinden!
                            xCurParentRef = new SwXFootnote(pCrsr->GetDoc(), rFtn);
                            break;
                        }
                    }
                }
                break;
                case SwHeaderStartNode:
                case SwFooterStartNode:
                {
                    //PageStyle besorgen, HeaderText anlegen/erfragen,
                    //und dann SwXTextPosition damit anlegen
                    String sPageStyleName = lcl_GetCurPageStyle(*pCrsr);
                    uno::Reference< style::XStyleFamiliesSupplier >  xStyleSupp(
                        pCrsr->GetDoc()->GetDocShell()->GetBaseModel(), uno::UNO_QUERY);

                    uno::Reference< container::XNameAccess >  xStyles = xStyleSupp->getStyleFamilies();
                    uno::Any aStyleFamily = xStyles->getByName(C2U("PageStyles"));

                    uno::Reference< container::XNameContainer > xFamily =
                        *(uno::Reference< container::XNameContainer > *)aStyleFamily.getValue();
                    uno::Any aStyle = xFamily->getByName(sPageStyleName);
                    uno::Reference< style::XStyle >  xStyle = *(uno::Reference< style::XStyle > *)aStyle.getValue();

                    uno::Reference< beans::XPropertySet >  xPropSet(xStyle, uno::UNO_QUERY);
                    uno::Any aLayout = xPropSet->getPropertyValue(C2U(UNO_NAME_PAGE_STYLE_LAYOUT));

                     style::PageStyleLayout eLayout = *(style::PageStyleLayout*)aLayout.getValue();
                    uno::Any aShare = xPropSet->getPropertyValue(C2U(UNO_NAME_HEADER_IS_SHARED));
                    sal_Bool bShare;
                    aShare >>= bShare;
                    sal_Bool bLeft = sal_False;
                    sal_Bool bRight = sal_False;
                    //jetzt evtl. noch zw. linker/rechter Kopf-/Fusszeile unterscheiden
                    if(!bShare && eLayout == style::PageStyleLayout_MIRRORED)
                    {
                        uno::Reference<lang::XUnoTunnel> xTunnel(xStyle, uno::UNO_QUERY);
                        SwXPageStyle* pStyle = (SwXPageStyle*)
                            xTunnel->getSomething( SwXPageStyle::getUnoTunnelId());

                        DBG_ASSERT(pStyle, "Was ist das fuer ein style::XStyle?")
                        bLeft = pSttNode == pStyle->GetStartNode(eType == SwHeaderStartNode, sal_True);
                        bRight = !bLeft;
                    }
                    uno::Any aParent;
                    sal_Bool bFooter = eType == SwFooterStartNode;
                    if(eLayout == style::PageStyleLayout_LEFT || bLeft)
                        aParent = xPropSet->getPropertyValue(C2U(bFooter ? UNO_NAME_FOOTER_TEXT_LEFT : UNO_NAME_HEADER_TEXT_LEFT));
                    else if(eLayout == style::PageStyleLayout_RIGHT)
                        aParent = xPropSet->getPropertyValue(C2U(bFooter ? UNO_NAME_FOOTER_TEXT_RIGHT : UNO_NAME_HEADER_TEXT_RIGHT));
                    else
                        aParent = xPropSet->getPropertyValue(C2U(bFooter ? UNO_NAME_FOOTER_TEXT : UNO_NAME_HEADER_TEXT));

                    if(aParent.getValueType() != ::getVoidCppuType())
                    {
                        uno::Reference< XText >  xText = *(uno::Reference< XText > *)aParent.getValue();
                        xCurParentRef = xText;
                    }
                }


                break;
//              case SwNormalStartNode:
                default:
                {
                    if(!xParentText.is())
                    {
                        uno::Reference< XTextDocument >  xDoc(
                            pCrsr->GetDoc()->GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
                        xParentText = xDoc->getText();
                    }
                    xCurParentRef = xParentText;
                }
            }

            if(xCurParentRef.is())
                *pPtr = new SwXTextRange(*PUNOPAM, xCurParentRef);
            if(pPtr->is())
                pRangeArr->Insert(pPtr, pRangeArr->Count());
        FOREACHUNOPAM_END()
        pCrsr->Remove( this );
    }
    return pRangeArr;
}
/*-- 10.12.98 13:57:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwXTextRanges::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}


/******************************************************************
 * SwXParagraph
 ******************************************************************/
/* -----------------------------11.07.00 12:10--------------------------------

 ---------------------------------------------------------------------------*/
SwXParagraph* SwXParagraph::GetImplementation(Reference< XInterface> xRef )
{
    uno::Reference<lang::XUnoTunnel> xParaTunnel( xRef, uno::UNO_QUERY);
    if(xParaTunnel.is())
        return (SwXParagraph*)xParaTunnel->getSomething(SwXParagraph::getUnoTunnelId());
    return 0;
}
/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXParagraph::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXParagraph::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/* -----------------------------06.04.00 16:37--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXParagraph::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXParagraph");
}
/* -----------------------------06.04.00 16:37--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXParagraph::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    String sServiceName(rServiceName);
    return sServiceName.EqualsAscii("com.sun.star.text.TextContent") ||
        sServiceName.EqualsAscii("com.sun.star.text.Paragraph") ||
         sServiceName.EqualsAscii("com.sun.star.style.CharacterProperties")||
         sServiceName.EqualsAscii("com.sun.star.style.ParagraphProperties");
}
/* -----------------------------06.04.00 16:37--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXParagraph::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(4);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.Paragraph");
     pArray[1] = C2U("com.sun.star.style.CharacterProperties");
     pArray[2] = C2U("com.sun.star.style.ParagraphProperties");
     pArray[3] = C2U("com.sun.star.text.TextContent");
    return aRet;
}
/*-- 11.12.98 08:12:47---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraph::SwXParagraph() :
    aLstnrCntnr( (XTextRange*)this),
    xParentText(0),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH)),
    m_bIsDescriptor(TRUE)
{
}

/*-- 11.12.98 08:12:47---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraph::SwXParagraph(SwXText* pParent, SwUnoCrsr* pCrsr) :
    SwClient(pCrsr),
    xParentText(pParent),
    aLstnrCntnr( (XTextRange*)this),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH)),
    m_bIsDescriptor(FALSE)
{
}
/*-- 11.12.98 08:12:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraph::~SwXParagraph()
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        delete pUnoCrsr;
}
/* -----------------------------11.07.00 14:48--------------------------------

 ---------------------------------------------------------------------------*/
void SwXParagraph::attachToText(SwXText* pParent, SwUnoCrsr* pCrsr)
{
    DBG_ASSERT(m_bIsDescriptor, "Paragraph is not a descriptor")
    if(m_bIsDescriptor)
    {
        m_bIsDescriptor = FALSE;
        pCrsr->Add(this);
        xParentText = pParent;
        if(m_sText.getLength())
        {
            try { setString(m_sText); }
            catch(...){}
            m_sText = OUString();
        }
    }
}
/*-- 11.12.98 08:12:49---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo >  SwXParagraph::getPropertySetInfo(void)
                                            throw( RuntimeException )
{
    static Reference< XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}
/*-- 11.12.98 08:12:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
        lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aAny;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if(rPropertyName.equals(C2U(UNO_NAME_TEXT_WRAP)) ||
            rPropertyName.equals(C2U(UNO_NAME_ANCHOR_TYPE)) ||
                rPropertyName.equals(C2U(UNO_NAME_ANCHOR_TYPES)))
        {
            throw IllegalArgumentException();
        }
        SwParaSelection aParaSel(pUnoCrsr);
        SwXTextCursor::SetPropertyValue(*pUnoCrsr, aPropSet, rPropertyName, aValue);
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 08:12:49---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXParagraph::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aAny;
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        if(!SwXParagraph::getDefaultTextContentValue(aAny, rPropertyName))
        {
            SwParaSelection aParaSel(pUnoCrsr);
            SwNode& rTxtNode = pUnoCrsr->GetPoint()->nNode.GetNode();
            SwAttrSet* pAttrSet = ((SwTxtNode&)rTxtNode).GetpSwAttrSet();

            String aPropertyName(rPropertyName);
            const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                        aPropSet.getPropertyMap(), rPropertyName);
            BOOL bDone = FALSE;
            if(pMap)
            {
                PropertyState eTemp;
                bDone = lcl_getCrsrPropertyValue(pMap, *pUnoCrsr, *pAttrSet, aAny, eTemp );
                if(!bDone)
                    aAny = aPropSet.getPropertyValue(aPropertyName, *pAttrSet);
            }
            else
                throw UnknownPropertyException();
//          aAny = SwXTextCursor::GetPropertyValue(*pUnoCrsr, aPropSet, rPropertyName);
        }
    }
    else
        throw uno::RuntimeException();
    return aAny;
}
/* -----------------------------12.09.00 11:09--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXParagraph::getDefaultTextContentValue(Any& rAny, const OUString& rPropertyName, USHORT nWID)
{
    if(!nWID)
    {
        if(0 == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPE))
            nWID = FN_UNO_ANCHOR_TYPE;
        else if(0 == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPES))
            nWID = FN_UNO_ANCHOR_TYPES;
        else if(0 == rPropertyName.compareToAscii(UNO_NAME_TEXT_WRAP))
            nWID = FN_UNO_TEXT_WRAP;
        else
            return FALSE;
    }

    switch(nWID)
    {
        case FN_UNO_TEXT_WRAP:  rAny <<= WrapTextMode_NONE; break;
        case FN_UNO_ANCHOR_TYPE: rAny <<= TextContentAnchorType_AT_PARAGRAPH; break;
        case FN_UNO_ANCHOR_TYPES:
        {   Sequence<TextContentAnchorType> aTypes(1);
            TextContentAnchorType* pArray = aTypes.getArray();
            pArray[0] = TextContentAnchorType_AT_PARAGRAPH;
            rAny.setValue(&aTypes, ::getCppuType((uno::Sequence<TextContentAnchorType>*)0));
        }
        break;
        default:
            return FALSE;
    }
    return TRUE;
}
/*-- 11.12.98 08:12:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::addPropertyChangeListener(
    const OUString& PropertyName,
    const uno::Reference< beans::XPropertyChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 08:12:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 08:12:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 08:12:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
//-----------------------------------------------------------------------------
beans::PropertyState lcl_SwXParagraph_getPropertyState(
                            SwUnoCrsr& rUnoCrsr,
                            const SwAttrSet** ppSet,
                            SfxItemPropertySet& rPropSet,
                            const OUString& rPropertyName )
                                throw( beans::UnknownPropertyException)
{
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    if(0 == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPE) ||
         0 == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPES) ||
            0 == rPropertyName.compareToAscii(UNO_NAME_TEXT_WRAP))
        return eRet;

    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                            rPropSet.getPropertyMap(), rPropertyName);
    if(!pMap)
    {
        UnknownPropertyException aExcept;
        aExcept.Message = rPropertyName;
        throw aExcept;
    }

    if(!(*ppSet))
    {
        SwNode& rTxtNode = rUnoCrsr.GetPoint()->nNode.GetNode();
        (*ppSet) = ((SwTxtNode&)rTxtNode).GetpSwAttrSet();
    }
    if(!rPropertyName.compareToAscii(UNO_NAME_NUMBERING_RULES))
    {
        //wenn eine Numerierung gesetzt ist, dann hier herausreichen, sonst nichts tun
        lcl_getNumberingProperty( rUnoCrsr, eRet );
    }
    else if(!rPropertyName.compareToAscii(UNO_NAME_PARA_STYLE_NAME) ||
            !rPropertyName.compareToAscii(UNO_NAME_PARA_CONDITIONAL_STYLE_NAME))
    {
        SwFmtColl* pFmt = SwXTextCursor::GetCurTxtFmtColl( rUnoCrsr,
            !rPropertyName.compareToAscii(UNO_NAME_PARA_CONDITIONAL_STYLE_NAME));
        if( !pFmt )
            eRet = beans::PropertyState_AMBIGUOUS_VALUE;
    }
    else if(!rPropertyName.compareToAscii(UNO_NAME_PAGE_STYLE_NAME))
    {
        String sVal = lcl_GetCurPageStyle( rUnoCrsr );
        if( !sVal.Len() )
            eRet = beans::PropertyState_AMBIGUOUS_VALUE;
    }
    else if((*ppSet) && SFX_ITEM_SET == (*ppSet)->GetItemState(pMap->nWID, FALSE))
        eRet = beans::PropertyState_DIRECT_VALUE;
    else
        eRet = beans::PropertyState_DEFAULT_VALUE;

/*  if( !*ppSet )
    {
        // Absatz selektieren
        SwParaSelection aParaSel( &rUnoCrsr );
        *ppSet = new SfxItemSet( rUnoCrsr.GetDoc()->GetAttrPool(),
                        RES_CHRATR_BEGIN,   RES_PARATR_NUMRULE,
                        RES_FILL_ORDER,     RES_FRMATR_END -1,
                        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                        RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                        0L);
        SwXTextCursor::GetCrsrAttr( rUnoCrsr, **ppSet, TRUE );
    }
    if(rPropertyName.equals(C2U(UNO_NAME_PAGE_DESC_NAME)))
    {
        // Sonderbehandlung RES_PAGEDESC
        const SfxPoolItem* pItem;
        if(SFX_ITEM_SET == (*ppSet)->GetItemState( RES_PAGEDESC, sal_True, &pItem ) )
        {
            eRet = beans::PropertyState_DIRECT_VALUE;
        }
    }
    else if(rPropertyName.equals(C2U(UNO_NAME_NUMBERING_RULES)))
    {
        //wenn eine Numerierung gesetzt ist, dann hier herausreichen, sonst nichts tun
        lcl_getNumberingProperty( rUnoCrsr, eRet );
    }
    else if(rPropertyName.equals(C2U(UNO_NAME_PARA_STYLE_NAME)) ||
            rPropertyName.equals(C2U(UNO_NAME_PARA_CONDITIONAL_STYLE_NAME)))
    {
        SwFmtColl* pFmt = SwXTextCursor::GetCurTxtFmtColl( rUnoCrsr,
            rPropertyName.equals(C2U(UNO_NAME_PARA_CONDITIONAL_STYLE_NAME)));
        if( !pFmt )
            eRet = beans::PropertyState_AMBIGUOUS_VALUE;
    }
    else if(rPropertyName.equals(C2U(UNO_NAME_PAGE_STYLE_NAME)))
    {
        String sVal = lcl_GetCurPageStyle( rUnoCrsr );
        if( !sVal.Len() )
            eRet = beans::PropertyState_AMBIGUOUS_VALUE;
    }
    else
    {
        eRet = rPropSet.getPropertyState( rPropertyName, **ppSet );
        //special handling for character attributes that are set in the ItemSet of the text node
        if(beans::PropertyState_DEFAULT_VALUE == eRet)
        {
            const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    rPropSet.getPropertyMap(), rPropertyName);
            if(pMap)
            {
                SwNode& rTxtNode = rUnoCrsr.GetPoint()->nNode.GetNode();
                SwAttrSet* pNodeAttrSet = ((SwTxtNode&)rTxtNode).GetpSwAttrSet();
                if(pNodeAttrSet && SFX_ITEM_SET == pNodeAttrSet->GetItemState(pMap->nWID, FALSE))
                    eRet = beans::PropertyState_DIRECT_VALUE;
            }
        }
    }
*/
    return eRet;
}

/*-- 05.03.99 11:37:30---------------------------------------------------

  -----------------------------------------------------------------------*/
beans::PropertyState SwXParagraph::getPropertyState(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if( pUnoCrsr )
    {
        const SwAttrSet* pSet = 0;
        eRet = lcl_SwXParagraph_getPropertyState( *pUnoCrsr, &pSet,
                                                    aPropSet, rPropertyName );
    }
    else
        throw uno::RuntimeException();
    return eRet;
}
/*-- 05.03.99 11:37:32---------------------------------------------------

  -----------------------------------------------------------------------*/

uno::Sequence< beans::PropertyState > SwXParagraph::getPropertyStates(
        const uno::Sequence< OUString >& PropertyNames)
        throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const OUString* pNames = PropertyNames.getConstArray();
    uno::Sequence< beans::PropertyState > aRet(PropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();

    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if( pUnoCrsr )
    {
        const SwAttrSet* pSet = 0;
        for(sal_Int32 i = 0, nEnd = PropertyNames.getLength(); i < nEnd; i++ )
            pStates[i] = lcl_SwXParagraph_getPropertyState( *pUnoCrsr, &pSet,
                                                         aPropSet, pNames[i]);
    }
    else
        throw uno::RuntimeException();

    return aRet;
}
/*-- 05.03.99 11:37:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::setPropertyToDefault(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if(0 == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPE) ||
            0 == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPES)||
                0 == rPropertyName.compareToAscii(UNO_NAME_TEXT_WRAP))
            return;

        // Absatz selektieren
        SwParaSelection aParaSel(pUnoCrsr);
        SwDoc* pDoc = pUnoCrsr->GetDoc();
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
            if(pMap->nWID < RES_FRMATR_END)
            {
                SvUShortsSort aWhichIds;
                aWhichIds.Insert(pMap->nWID);
                if(pMap->nWID < RES_PARATR_BEGIN)
                    pUnoCrsr->GetDoc()->ResetAttr(*pUnoCrsr, sal_True, &aWhichIds);
                else
                {
                    //fuer Absatzattribute muss die Selektion jeweils auf
                    //Absatzgrenzen erweitert werden
                    SwPosition aStart = *pUnoCrsr->Start();
                    SwPosition aEnd = *pUnoCrsr->End();
                    SwUnoCrsr* pTemp = pUnoCrsr->GetDoc()->CreateUnoCrsr(aStart, sal_False);
                    if(!lcl_IsStartOfPara(*pTemp))
                    {
                        pTemp->MovePara(fnParaCurr, fnParaStart);
                    }
                    pTemp->SetMark();
                    *pTemp->GetPoint() = aEnd;
                    //pTemp->Exchange();
                    SwXTextCursor::SelectPam(*pTemp, sal_True);
                    if(!lcl_IsEndOfPara(*pTemp))
                    {
                        pTemp->MovePara(fnParaCurr, fnParaEnd);
                    }
                    pTemp->GetDoc()->ResetAttr(*pTemp, sal_True, &aWhichIds);
                    delete pTemp;
                }
            }
            else
                lcl_resetCrsrPropertyValue(pMap, *pUnoCrsr);
        }
        else
            throw beans::UnknownPropertyException();
    }
    else
        throw uno::RuntimeException();
}
/*-- 05.03.99 11:37:33---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXParagraph::getPropertyDefault(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        if(SwXParagraph::getDefaultTextContentValue(aRet, rPropertyName))
            return aRet;

        SwDoc* pDoc = pUnoCrsr->GetDoc();
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
            if(pMap->nWID < RES_FRMATR_END)
            {
                const SfxPoolItem& rDefItem =
                    pUnoCrsr->GetDoc()->GetAttrPool().GetDefaultItem(pMap->nWID);
                rDefItem.QueryValue(aRet, pMap->nMemberId);
            }
        }
        else
            throw beans::UnknownPropertyException();
    }
    else
        throw uno::RuntimeException();

    return aRet;
}
/*-- 11.12.98 08:12:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::attach(const uno::Reference< XTextRange > & xTextRange)
                    throw( lang::IllegalArgumentException, uno::RuntimeException )
{

}
/*-- 11.12.98 08:12:51---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXParagraph::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  aRet;
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        // Absatz selektieren
        SwParaSelection aSelection(pUnoCrsr);
        aRet = new SwXTextRange(*pUnoCrsr, xParentText);
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 11.12.98 08:12:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = ((SwXParagraph*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        // Absatz selektieren
        SwParaSelection aSelection(pUnoCrsr);
        pUnoCrsr->GetDoc()->DeleteAndJoin(*pUnoCrsr);
        pUnoCrsr->Remove(this);
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 08:12:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 11.12.98 08:12:53---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/*-- 11.12.98 08:12:53---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXParagraph::createEnumeration(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< container::XEnumeration >  aRef;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        aRef = new SwXTextPortionEnumeration(*pUnoCrsr, xParentText);
    else
        throw uno::RuntimeException();
    return aRef;

}
/*-- 11.12.98 08:12:54---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SwXParagraph::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}
/*-- 11.12.98 08:12:54---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXParagraph::hasElements(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(((SwXParagraph*)this)->GetCrsr())
        return sal_True;
    else
        return sal_False;
}
/*-- 11.12.98 08:12:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XText >  SwXParagraph::getText(void) throw( uno::RuntimeException )
{
    return xParentText;
}
/*-- 11.12.98 08:12:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXParagraph::getStart(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( pUnoCrsr)
    {
        SwPaM aPam(*pUnoCrsr->Start());
        uno::Reference< XText >  xParent = getText();
        xRet = new SwXTextRange(aPam, xParent);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 11.12.98 08:12:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXParagraph::getEnd(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( pUnoCrsr)
    {
        SwPaM aPam(*pUnoCrsr->End());
        uno::Reference< XText >  xParent = getText();
        xRet = new SwXTextRange(aPam, xParent);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 11.12.98 08:12:56---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXParagraph::getString(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    OUString aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( pUnoCrsr)
        aRet = pUnoCrsr->GetNode()->GetTxtNode()->GetTxt();
    else if(IsDescriptor())
        aRet = m_sText;
    else
        throw uno::RuntimeException();
    return aRet;
}
/* -----------------11.12.98 10:07-------------------
 *
 * --------------------------------------------------*/
void SwXTextCursor::SetString(SwUnoCrsr& rUnoCrsr, const OUString& rString)
{
    // Start/EndAction
    SwDoc* pDoc = rUnoCrsr.GetDoc();
    UnoActionContext aAction(pDoc);
    String aText(rString);
    xub_StrLen nTxtLen = aText.Len();
    pDoc->StartUndo(UNDO_INSERT);
    if(rUnoCrsr.HasMark())
        pDoc->DeleteAndJoin(rUnoCrsr);
    if(nTxtLen)
    {
        //OPT: GetSystemCharSet
        if( !pDoc->Insert(rUnoCrsr, aText) )
            ASSERT( sal_False, "Doc->Insert(Str) failed." );
        SwXTextCursor::SelectPam(rUnoCrsr, sal_True);
        rUnoCrsr.Left(nTxtLen);
    }
    pDoc->EndUndo(UNDO_INSERT);
}
/*-- 11.12.98 08:12:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::setString(const OUString& aString) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();

    if(pUnoCrsr)
    {
        EXCEPT_ON_PROTECTION(*pUnoCrsr)
        if(!lcl_IsStartOfPara(*pUnoCrsr))
            pUnoCrsr->MovePara(fnParaCurr, fnParaStart);
        SwXTextCursor::SelectPam(*pUnoCrsr, sal_True);
        if(pUnoCrsr->GetNode()->GetTxtNode()->GetTxt().Len())
            pUnoCrsr->MovePara(fnParaCurr, fnParaEnd);
        SwXTextCursor::SetString(*pUnoCrsr, aString);
        SwXTextCursor::SelectPam(*pUnoCrsr, sal_False);
    }
    else if(IsDescriptor())
        m_sText = aString;
    else
        throw uno::RuntimeException();

}
/* -----------------23.03.99 12:49-------------------
 *
 * --------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXParagraph::createContentEnumeration(const OUString& rServiceName)
    throw( uno::RuntimeException )
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( !pUnoCrsr || COMPARE_EQUAL != rServiceName.compareToAscii("com.sun.star.text.TextContent") )
        throw uno::RuntimeException();

    uno::Reference< container::XEnumeration >  xRet = new SwXParaFrameEnumeration(*pUnoCrsr, PARAFRAME_PORTION_PARAGRAPH);
    return xRet;
}
/* -----------------23.03.99 12:49-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXParagraph::getAvailableServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextContent");
    return aRet;
}
/*-- 11.12.98 08:12:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXParagraph::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        aLstnrCntnr.Disposing();
}
/******************************************************************
 * SwXParaFrameEnumeration
 ******************************************************************/
SV_IMPL_PTRARR(SwDependArr, SwDepend*);
/* -----------------------------06.04.00 16:39--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXParaFrameEnumeration::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXParaFrameEnumeration");
}
/* -----------------------------06.04.00 16:39--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXParaFrameEnumeration::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.util.ContentEnumeration") == rServiceName;
}
/* -----------------------------06.04.00 16:39--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXParaFrameEnumeration::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.util.ContentEnumeration");
    return aRet;
}
/*-- 23.03.99 13:22:29---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParaFrameEnumeration::SwXParaFrameEnumeration(const SwUnoCrsr& rUnoCrsr,
                                                sal_uInt8 nParaFrameMode,
                                                SwFrmFmt* pFmt)
{
    SwDoc* pDoc = rUnoCrsr.GetDoc();
    SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(*rUnoCrsr.GetPoint(), sal_False);
    if(rUnoCrsr.HasMark())
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *rUnoCrsr.GetMark();
    }
    pUnoCrsr->Add(this);

    if(PARAFRAME_PORTION_PARAGRAPH == nParaFrameMode)
    {
        const SwNodeIndex& rOwnNode = rUnoCrsr.GetPoint()->nNode;
        //alle Rahmen, Grafiken und OLEs suchen, die an diesem Absatz gebunden sind
        const SwSpzFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();
        USHORT nSize = rFmts.Count();
        for ( USHORT i = 0; i < nSize; i++)
        {
            const SwFrmFmt* pFmt = rFmts[ i ];
            const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
            const SwPosition* pAnchorPos;
            if( rAnchor.GetAnchorId() == FLY_AT_CNTNT &&
                0 != (pAnchorPos = rAnchor.GetCntntAnchor()) &&
                    pAnchorPos->nNode == rOwnNode)
            {
                //jetzt einen SwDepend anlegen und in das Array einfuegen
                SwDepend* pNewDepend = new SwDepend(this, (SwFrmFmt*)pFmt);
                aFrameArr.C40_INSERT(SwDepend, pNewDepend, aFrameArr.Count());
            }
        }
    }
    else if(pFmt)
    {
        //jetzt einen SwDepend anlegen und in das Array einfuegen
        SwDepend* pNewDepend = new SwDepend(this, pFmt);
        aFrameArr.C40_INSERT(SwDepend, pNewDepend, aFrameArr.Count());
    }
    else if((PARAFRAME_PORTION_CHAR == nParaFrameMode) || (PARAFRAME_PORTION_TEXTRANGE == nParaFrameMode))
    {
        SwPosFlyFrms aFlyFrms;
        //get all frames that are bound at paragraph or at character
        pDoc->GetAllFlyFmts(aFlyFrms, pUnoCrsr);//, bDraw);
        for(USHORT i = 0; i < aFlyFrms.Count(); i++)
        {
            SwPosFlyFrm* pPosFly = aFlyFrms[i];
            SwFrmFmt* pFrmFmt = (SwFrmFmt*)&pPosFly->GetFmt();
            //jetzt einen SwDepend anlegen und in das Array einfuegen
            SwDepend* pNewDepend = new SwDepend(this, pFrmFmt);
            aFrameArr.C40_INSERT(SwDepend, pNewDepend, aFrameArr.Count());
        }
        //created from any text range
        if(pUnoCrsr->HasMark())
        {
            if(pUnoCrsr->Start() != pUnoCrsr->GetPoint())
                pUnoCrsr->Exchange();
            do
            {
                FillFrame(*pUnoCrsr);
                pUnoCrsr->Right();
            }
            while(*pUnoCrsr->Start() < *pUnoCrsr->End());
        }
        FillFrame(*pUnoCrsr);
    }
}
/*-- 23.03.99 13:22:30---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParaFrameEnumeration::~SwXParaFrameEnumeration()
{
    aFrameArr.DeleteAndDestroy(0, aFrameArr.Count());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    delete pUnoCrsr;
}
/* -----------------------------03.04.00 10:15--------------------------------
    Description: Search for a FLYCNT text attribute at the cursor point
                and fill the frame into the array
 ---------------------------------------------------------------------------*/
void SwXParaFrameEnumeration::FillFrame(SwUnoCrsr& rUnoCrsr)
{
    // search for objects at the cursor - anchored at/as char
    SwTxtAttr* pTxtAttr = rUnoCrsr.GetNode()->GetTxtNode()->GetTxtAttr(
                    rUnoCrsr.GetPoint()->nContent, RES_TXTATR_FLYCNT);
    if(pTxtAttr)
    {
        const SwFmtFlyCnt& rFlyCnt = pTxtAttr->GetFlyCnt();
        SwFrmFmt* pFrmFmt = rFlyCnt.GetFrmFmt();
        //jetzt einen SwDepend anlegen und in das Array einfuegen
        SwDepend* pNewDepend = new SwDepend(this, pFrmFmt);
        aFrameArr.C40_INSERT(SwDepend, pNewDepend, aFrameArr.Count());
    }
}
/*-- 23.03.99 13:22:32---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXParaFrameEnumeration::hasMoreElements(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetCrsr())
        throw uno::RuntimeException();
    return xNextObject.is() ? sal_True : CreateNextObject();
}
/*-- 23.03.99 13:22:33---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXParaFrameEnumeration::nextElement(void)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetCrsr())
        throw uno::RuntimeException();
    if(!xNextObject.is() && aFrameArr.Count())
    {
        CreateNextObject();
    }
    if(!xNextObject.is())
        throw container::NoSuchElementException();
    uno::Any aRet(&xNextObject, ::getCppuType((uno::Reference<XTextContent>*)0));
    xNextObject = 0;
    return aRet;
}
/* -----------------23.03.99 13:38-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXParaFrameEnumeration::CreateNextObject()
{
    if(!aFrameArr.Count())
        return sal_False;
    SwDepend* pDepend = aFrameArr.GetObject(0);
    aFrameArr.Remove(0);
    SwFrmFmt* pFormat = (SwFrmFmt*)pDepend->GetRegisteredIn();
    delete pDepend;
    // the format should be valid her otherwise the client
    // would have been removed in ::Modify
    // check for a shape first
    SwClientIter aIter(*pFormat);
    SwDrawContact* pContact = (SwDrawContact*)
                                            aIter.First(TYPE(SwDrawContact));
    if(pContact)
       {
          SdrObject* pSdr = pContact->GetMaster();
        if(pSdr)
        {
            Reference<frame::XModel> xModel =
                            pFormat->GetDoc()->GetDocShell()->GetBaseModel();
            Reference<drawing::XDrawPageSupplier> xPageSupp(
                        xModel, UNO_QUERY);

            Reference<drawing::XDrawPage> xPage = xPageSupp->getDrawPage();
            XDrawPage* pImpPage = xPage.get();
            Reference <XShape> xShape = ((SwXDrawPage*)pImpPage)->GetSvxPage()->_CreateShape( pSdr );
            xNextObject = uno::Reference< XTextContent >(xShape, uno::UNO_QUERY);
        }
       }
    else
    {
        const SwNodeIndex* pIdx = pFormat->GetCntnt().GetCntntIdx();
        DBG_ASSERT(pIdx, "where is the index?");
        const SwNode* pNd = GetCrsr()->GetDoc()->GetNodes()[ pIdx->GetIndex() + 1 ];

        FlyCntType eType;
        if(!pNd->IsNoTxtNode())
            eType = FLYCNTTYPE_FRM;
        else if(pNd->IsGrfNode())
            eType = FLYCNTTYPE_GRF;
        else
            eType = FLYCNTTYPE_OLE;

        uno::Reference< container::XNamed >  xFrame = SwXFrames::GetObject(*pFormat, eType);
        xNextObject = uno::Reference< XTextContent >(xFrame, uno::UNO_QUERY);
    }

    return xNextObject.is();
}

/*-- 23.03.99 13:22:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwXParaFrameEnumeration::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        if( (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            ((SwModify*)GetRegisteredIn())->Remove(this);
        break;

    case RES_FMT_CHG:
        // wurden wir an das neue umgehaengt und wird das alte geloscht?
        if( ((SwFmtChg*)pNew)->pChangedFmt == GetRegisteredIn() &&
            ((SwFmtChg*)pOld)->pChangedFmt->IsFmtInDTOR() )
            ((SwModify*)GetRegisteredIn())->Remove(this);
        break;
    }
    if(!GetRegisteredIn())
    {
        aFrameArr.DeleteAndDestroy(0, aFrameArr.Count());
        xNextObject = 0;
    }
    else
    {
        //dann war es vielleicht ein Frame am Absatz?
        for(sal_uInt16 i = aFrameArr.Count(); i; i--)
        {
            SwDepend* pDepend = aFrameArr.GetObject(i-1);
            if(!pDepend->GetRegisteredIn())
            {
                delete pDepend;
                aFrameArr.Remove(i-1);
            }
        }
    }
}
/******************************************************************
 *  SwXTextPortionEnumeration
 ******************************************************************/
/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextPortionEnumeration::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextPortionEnumeration::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/* -----------------------------06.04.00 16:39--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextPortionEnumeration::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextPortionEnumeration");
}
/* -----------------------------06.04.00 16:39--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextPortionEnumeration::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextPortionEnumeration") == rServiceName;
}
/* -----------------------------06.04.00 16:39--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextPortionEnumeration::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextPortionEnumeration");
    return aRet;
}
/*-- 27.01.99 10:44:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextPortionEnumeration::SwXTextPortionEnumeration(SwPaM& rParaCrsr, uno::Reference< XText >  xParentText) :
    xParent(xParentText),
    bAtEnd(sal_False),
    bFirstPortion(sal_True)
{
    SwUnoCrsr* pUnoCrsr = rParaCrsr.GetDoc()->CreateUnoCrsr(*rParaCrsr.GetPoint(), sal_False);
    pUnoCrsr->Add(this);

    //alle Rahmen, Grafiken und OLEs suchen, die an diesem Absatz AM ZEICHEN gebunden sind
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    sal_uInt16 nCount = pDoc->GetFlyCount();
    const SwNodeIndex nOwnNode = pUnoCrsr->GetPoint()->nNode;
    for( sal_uInt16 i = 0; i < nCount; i++)
    {
        SwFrmFmt* pFmt = pDoc->GetFlyNum(i);
        //steht der Anker in diesem Node und ist er absatzgebunden?
        if( pFmt->GetAnchor().GetAnchorId() == FLY_AUTO_CNTNT )
        {
            const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
            const SwPosition* pAnchorPos = rAnchor.GetCntntAnchor();
            if(pAnchorPos->nNode == nOwnNode)
            {
                //jetzt einen SwDepend anlegen und sortiert in das Array einfuegen
                SwDepend* pNewDepend = new SwDepend(this, pFmt);
                const SwFmtAnchor& rNewAnchor = pFmt->GetAnchor();
                const SwPosition* pNewAnchorPos = rNewAnchor.GetCntntAnchor();
                xub_StrLen nInsertIndex = pNewAnchorPos->nContent.GetIndex();

                for(sal_uInt16 nFrame = 0; nFrame < aFrameArr.Count(); nFrame++)
                {
                    SwDepend* pCurDepend = aFrameArr.GetObject(nFrame);
                    SwFrmFmt* pFormat = (SwFrmFmt*)pCurDepend->GetRegisteredIn();
                    const SwFmtAnchor& rAnchor = pFormat->GetAnchor();
                    const SwPosition* pAnchorPos = rAnchor.GetCntntAnchor();

                    if(pAnchorPos->nContent.GetIndex() > nInsertIndex)
                    {
                        aFrameArr.C40_INSERT(SwDepend, pNewDepend, nFrame);
                        pNewDepend = 0;
                        break;
                    }
                }
                if(pNewDepend)
                    aFrameArr.C40_INSERT(SwDepend, pNewDepend, aFrameArr.Count());
            }
        }
    }
    CreatePortions();
}
/*-- 27.01.99 10:44:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextPortionEnumeration::~SwXTextPortionEnumeration()
{
    for(sal_uInt16 nFrame = aFrameArr.Count(); nFrame; nFrame--)
    {
        SwDepend* pCurDepend = aFrameArr.GetObject(nFrame - 1);
        delete pCurDepend;
        aFrameArr.Remove(nFrame - 1);
    }

    if( aPortionArr.Count() )
        aPortionArr.DeleteAndDestroy(0, aPortionArr.Count() );

    SwUnoCrsr* pUnoCrsr = GetCrsr();
    delete pUnoCrsr;
}
/*-- 27.01.99 10:44:44---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextPortionEnumeration::hasMoreElements(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return aPortionArr.Count() > 0;
}
/*-- 27.01.99 10:44:45---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextPortionEnumeration::nextElement(void)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!aPortionArr.Count())
        throw container::NoSuchElementException();
    XTextRangeRefPtr pPortion = aPortionArr.GetObject(0);
    Any aRet(pPortion, ::getCppuType((uno::Reference<XTextRange>*)0));
    aPortionArr.Remove(0);
    delete pPortion;
    return aRet;
}
/* -----------------------------31.08.00 14:28--------------------------------

 ---------------------------------------------------------------------------*/
void lcl_InsertRefMarkPortion(
    XTextRangeArr& rArr, SwUnoCrsr* pUnoCrsr, Reference<XText> xParent, SwTxtAttr* pAttr, BOOL bEnd)
{
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    SwFmtRefMark& rRefMark = ((SwFmtRefMark&)pAttr->GetAttr());
    Reference<XTextContent> xContent = ((SwUnoCallBack*)pDoc->GetUnoCallBack())->GetRefMark(rRefMark);
    if(!xContent.is())
        xContent = new SwXReferenceMark(pDoc, &rRefMark);

    SwXTextPortion* pPortion;
    if(!bEnd)
    {
        rArr.Insert(
            new Reference< XTextRange >(pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_REFMARK_START)),
            rArr.Count());
        pPortion->SetRefMark(xContent);
        pPortion->SetIsCollapsed(pAttr->GetEnd() ? FALSE : TRUE);
    }
    else
    {
        rArr.Insert(
            new Reference< XTextRange >(pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_REFMARK_END)),
            rArr.Count());
        pPortion->SetRefMark(xContent);
    }
}
//-----------------------------------------------------------------------------
void lcl_InsertTOXMarkPortion(
    XTextRangeArr& rArr, SwUnoCrsr* pUnoCrsr, Reference<XText> xParent, SwTxtAttr* pAttr, BOOL bEnd)
{
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    SwTOXMark& rTOXMark = ((SwTOXMark&)pAttr->GetAttr());

    SwClientIter aIter(*(SwTOXType*)rTOXMark.GetTOXType());
    SwXDocumentIndexMark* pxMark = (SwXDocumentIndexMark*)
                                            aIter.First(TYPE(SwXDocumentIndexMark));
    while( pxMark )
    {
        if(pxMark->GetTOXMark() == &rTOXMark)
            break;
        pxMark = (SwXDocumentIndexMark*)aIter.Next();
    }

    Reference<XTextContent> xContent = pxMark;
    if(!xContent.is())
        xContent = new SwXDocumentIndexMark(rTOXMark.GetTOXType(), &rTOXMark, pDoc);

    SwXTextPortion* pPortion;
    if(!bEnd)
    {
        rArr.Insert(
            new Reference< XTextRange >(pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_TOXMARK_START)),
            rArr.Count());
        pPortion->SetTOXMark(xContent);
        pPortion->SetIsCollapsed(pAttr->GetEnd() ? FALSE : TRUE);
    }
    if(bEnd)
    {
        rArr.Insert(
            new Reference< XTextRange >(pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_TOXMARK_END)),
            rArr.Count());
        pPortion->SetTOXMark(xContent);
    }
}
//-----------------------------------------------------------------------------
#define BKM_TYPE_START          0
#define BKM_TYPE_END            1
#define BKM_TYPE_START_END      2
struct SwXBookmarkPortion_Impl
{
    SwXBookmark*                pBookmark;
    Reference<XTextContent>     xBookmark;
    BYTE                        nBkmType;
    ULONG                       nIndex;

    SwXBookmarkPortion_Impl( SwXBookmark* pXMark, BYTE nType, ULONG nIdx) :
        pBookmark(pXMark),
        xBookmark(pXMark),
        nBkmType(nType),
        nIndex(nIdx){}

    // compare by nIndex
    BOOL operator < (const SwXBookmarkPortion_Impl &rCmp) const;
    BOOL operator ==(const SwXBookmarkPortion_Impl &rCmp) const;

};
BOOL SwXBookmarkPortion_Impl::operator ==(const SwXBookmarkPortion_Impl &rCmp) const
{
    return nIndex == rCmp.nIndex &&
        pBookmark == rCmp.pBookmark &&
            nBkmType == rCmp.nBkmType;
}

BOOL SwXBookmarkPortion_Impl::operator < (const SwXBookmarkPortion_Impl &rCmp) const
{
    return nIndex < rCmp.nIndex;
}

typedef SwXBookmarkPortion_Impl* SwXBookmarkPortion_ImplPtr;
SV_DECL_PTRARR_SORT(SwXBookmarkPortionArr, SwXBookmarkPortion_ImplPtr, 0, 2)
SV_IMPL_OP_PTRARR_SORT(SwXBookmarkPortionArr, SwXBookmarkPortion_ImplPtr)
//-----------------------------------------------------------------------------
void lcl_ExportBookmark(
    SwXBookmarkPortionArr& rBkmArr, ULONG nIndex,
    SwUnoCrsr* pUnoCrsr, Reference<XText> xParent, XTextRangeArr& rPortionArr)
{
    SwXBookmarkPortion_ImplPtr pPtr;
    while(rBkmArr.Count() && nIndex == (pPtr = rBkmArr.GetObject(0))->nIndex)
    {
        SwXTextPortion* pPortion;
        if(BKM_TYPE_START == pPtr->nBkmType || BKM_TYPE_START_END == pPtr->nBkmType)
        {
            rPortionArr.Insert(
                new Reference< XTextRange >(pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_BOOKMARK_START)),
                rPortionArr.Count());
            pPortion->SetBookmark(pPtr->xBookmark);
            pPortion->SetIsCollapsed(BKM_TYPE_START_END == pPtr->nBkmType ? TRUE : FALSE);

        }
        if(BKM_TYPE_END == pPtr->nBkmType)
        {
            rPortionArr.Insert(
                new Reference< XTextRange >(pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_BOOKMARK_END)),
                rPortionArr.Count());
            pPortion->SetBookmark(pPtr->xBookmark);
        }
        rBkmArr.Remove((USHORT)0);
        delete pPtr;
    }
}
//-----------------------------------------------------------------------------
Reference<XTextRange> lcl_ExportHints(SwpHints* pHints,
                                XTextRangeArr& rPortionArr,
                                SwUnoCrsr* pUnoCrsr,
                                Reference<XText> xParent,
                                const xub_StrLen nCurrentIndex,
                                SwTextPortionType& ePortionType,
                                const xub_StrLen& nFirstFrameIndex,
                                SwXBookmarkPortionArr& aBkmArr)
{
    Reference<XTextRange> xRef;
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    sal_Bool bAttrFound = sal_False;
    //search for special text attributes - first some ends
    sal_uInt16 nEndIndex = 0;
    sal_uInt16 nNextEnd = 0;
    while(nEndIndex < pHints->GetEndCount() &&
        (!pHints->GetEnd(nEndIndex)->GetEnd() ||
        nCurrentIndex >= (nNextEnd = (*pHints->GetEnd(nEndIndex)->GetEnd()))))
    {
        if(pHints->GetEnd(nEndIndex)->GetEnd())
        {
            SwTxtAttr* pAttr = pHints->GetEnd(nEndIndex);
            USHORT nAttrWhich = pAttr->Which();
            if(nNextEnd == nCurrentIndex &&
                ( RES_TXTATR_TOXMARK == nAttrWhich ||
                        RES_TXTATR_REFMARK == nAttrWhich))
            {
                switch( nAttrWhich )
                {
                    case RES_TXTATR_TOXMARK:
                        lcl_InsertTOXMarkPortion(
                            rPortionArr, pUnoCrsr, xParent, pAttr, TRUE);
                        ePortionType = PORTION_TEXT;
                        DBG_ERROR("ToxMark");
                    break;
                    case RES_TXTATR_REFMARK:
                        lcl_InsertRefMarkPortion(
                            rPortionArr, pUnoCrsr, xParent, pAttr, TRUE);
                        ePortionType = PORTION_TEXT;
                    break;

                }
            }
        }
        nEndIndex++;
    }
    //then som starts
    sal_uInt16 nStartIndex = 0;
    sal_uInt16 nNextStart = 0;
    while(nStartIndex < pHints->GetStartCount() &&
        nCurrentIndex >= (nNextStart = (*pHints->GetStart(nStartIndex)->GetStart())))
    {
        SwTxtAttr* pAttr = pHints->GetStart(nStartIndex);
        USHORT nAttrWhich = pAttr->Which();
        if(nNextStart == nCurrentIndex &&
            (!pAttr->GetEnd() ||
                RES_TXTATR_TOXMARK == nAttrWhich ||
                    RES_TXTATR_REFMARK == nAttrWhich))
        {
            switch( nAttrWhich )
            {
                case RES_TXTATR_FIELD:
                    pUnoCrsr->Right(1);
                    bAttrFound = sal_True;
                    ePortionType = PORTION_FIELD;
                break;
                case RES_TXTATR_FLYCNT   :
                    pUnoCrsr->Right(1);
                    pUnoCrsr->Exchange();
                    bAttrFound = sal_True;
                    ePortionType = PORTION_FRAME;
                break;
                case RES_TXTATR_FTN      :
                {
                    pUnoCrsr->Right(1);
                    SwXTextPortion* pPortion;
                    xRef =  pPortion = new SwXTextPortion(*pUnoCrsr, xParent, PORTION_FOOTNOTE);
                    Reference<XTextContent> xContent =
                        Reference<XTextContent>(
                        SwXFootnotes::GetObject(*pDoc, pAttr->SwTxtAttr::GetFtn()),
                        UNO_QUERY);
                    pPortion->SetFootnote(xContent);
                    bAttrFound = sal_True;
                    ePortionType = PORTION_TEXT;
                }
                break;
                case RES_TXTATR_SOFTHYPH :
                {
                    SwXTextPortion* pPortion;
                    rPortionArr.Insert(
                        new Reference< XTextRange >(
                            pPortion = new SwXTextPortion(
                                *pUnoCrsr, xParent, PORTION_CONTROL_CHAR)),
                        rPortionArr.Count());
                    pPortion->SetControlChar(3);
                    ePortionType = PORTION_TEXT;
                }
                break;
                case RES_TXTATR_HARDBLANK:
                {
                    ePortionType = PORTION_CONTROL_CHAR;
                    SwXTextPortion* pPortion;
                    rPortionArr.Insert(
                        new Reference< XTextRange >(
                            pPortion = new SwXTextPortion(
                                *pUnoCrsr, xParent, PORTION_CONTROL_CHAR)),
                        rPortionArr.Count());
                    const SwFmtHardBlank& rFmt = pAttr->GetHardBlank();
                    if(rFmt.GetChar() == '-')
                        pPortion->SetControlChar(2);//HARD_HYPHEN
                    else
                        pPortion->SetControlChar(4);//HARD_SPACE
                    ePortionType = PORTION_TEXT;
                }
                break;
                case RES_TXTATR_TOXMARK:
                    lcl_InsertTOXMarkPortion(
                        rPortionArr, pUnoCrsr, xParent, pAttr, FALSE);
                    ePortionType = PORTION_TEXT;
                    DBG_ERROR("ToxMark");
                break;
                case RES_TXTATR_REFMARK:
                    pUnoCrsr->Right(1);
                    bAttrFound = sal_True;
                    lcl_InsertRefMarkPortion(
                        rPortionArr, pUnoCrsr, xParent, pAttr, FALSE);
                    ePortionType = PORTION_TEXT;
                    if(*pUnoCrsr->GetPoint() < *pUnoCrsr->GetMark())
                            pUnoCrsr->Exchange();
                    pUnoCrsr->DeleteMark();
                break;
                default:
                    DBG_ERROR("was fuer ein Attribut?");
            }

        }
        nStartIndex++;
    }

    if(!bAttrFound)
    {
        // hier wird nach Uebergaengen zwischen Attributen gesucht, die nach der
        // aktuellen Cursorposition liegen
        // wenn dabei ein Rahmen 'ueberholt' wird, dann muss auch in der TextPortion unterbrochen werden

        nStartIndex = 0;
        nNextStart = 0;
        while(nStartIndex < pHints->GetStartCount() &&
            nCurrentIndex >= (nNextStart = (*pHints->GetStart(nStartIndex)->GetStart())))
            nStartIndex++;

        sal_uInt16 nEndIndex = 0;
        sal_uInt16 nNextEnd = 0;
        while(nEndIndex < pHints->GetEndCount() &&
            nCurrentIndex >= (nNextEnd = (*pHints->GetEnd(nEndIndex)->GetAnyEnd())))
            nEndIndex++;
        //nMovePos legt die neue EndPosition fest
        sal_uInt16 nMovePos = nNextStart > nCurrentIndex && nNextStart < nNextEnd ? nNextStart : nNextEnd;

        if(aBkmArr.Count() && aBkmArr.GetObject(0)->nIndex < nMovePos)
        {
            DBG_ASSERT(aBkmArr.GetObject(0)->nIndex > nCurrentIndex,
                "forgotten bookmark(s)")
            nMovePos = aBkmArr.GetObject(0)->nIndex;
        }
        // liegt die Endposition nach dem naechsten Rahmen, dann aufbrechen
        if(nFirstFrameIndex != STRING_MAXLEN && nMovePos > nFirstFrameIndex)
            nMovePos = nFirstFrameIndex;

        if(nMovePos > nCurrentIndex)
            pUnoCrsr->Right(nMovePos - nCurrentIndex);
        else
            pUnoCrsr->MovePara(fnParaCurr, fnParaEnd);
    }
    return xRef;
}
//-----------------------------------------------------------------------------
void SwXTextPortionEnumeration::CreatePortions()
{
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr /*&& !bAtEnd*/)
    {
        SwXBookmarkPortionArr aBkmArr;
        SwDoc* pDoc = pUnoCrsr->GetDoc();
        const SwNodeIndex nOwnNode = pUnoCrsr->GetPoint()->nNode;
        //search for all bookmarks that start or end in this paragraph
        const SwBookmarks& rMarks = pDoc->GetBookmarks();
        sal_uInt16 nArrLen = rMarks.Count();
        for( sal_uInt16 n = 0; n < nArrLen; ++n )
        {
            SwBookmark* pMark = rMarks.GetObject( n );
            if( !pMark->IsBookMark() )
                continue;

            const SwPosition& rPos1 = pMark->GetPos();
            const SwPosition* pPos2 = pMark->GetOtherPos();
            BOOL bBackward = pPos2 ? rPos1 > *pPos2: FALSE;
            if(rPos1.nNode == nOwnNode)
            {
                BYTE nType = bBackward ? BKM_TYPE_END : BKM_TYPE_START;
                if(!pPos2)
                {
                    nType = BKM_TYPE_START_END;
                }
                SwXBookmarkPortion_ImplPtr pBkmPtr = new SwXBookmarkPortion_Impl(
                    SwXBookmarks::GetObject( *pMark, pDoc ), nType, rPos1.nContent.GetIndex() );

                aBkmArr.Insert(pBkmPtr);

            }
            if(pPos2 && pPos2->nNode == nOwnNode)
            {
                BYTE nType = bBackward ? BKM_TYPE_START : BKM_TYPE_END;
                SwXBookmarkPortion_ImplPtr pBkmPtr = new SwXBookmarkPortion_Impl(
                        SwXBookmarks::GetObject( *pMark, pDoc ), nType, pPos2->nContent.GetIndex() );
                aBkmArr.Insert(pBkmPtr);
            }
        }
#ifdef DEBUG
        for(long i_debug = 0; i_debug <aBkmArr.Count(); i_debug++)
        {
            SwXBookmarkPortion_ImplPtr pPtr = aBkmArr.GetObject(i_debug);
        }

#endif
        while(!bAtEnd)
        {
            if(pUnoCrsr->HasMark())
            {
                if(*pUnoCrsr->GetPoint() < *pUnoCrsr->GetMark())
                    pUnoCrsr->Exchange();
                pUnoCrsr->DeleteMark();
            }
            if(!bFirstPortion   && pUnoCrsr->GetCntntNode() &&
                    pUnoCrsr->GetPoint()->nContent == pUnoCrsr->GetCntntNode()->Len())
            {
                //hier sollte man nie ankommen!
                bAtEnd = sal_True;
            }
            else
            {
                SwNode* pNode = pUnoCrsr->GetNode();
                if(ND_TEXTNODE == pNode->GetNodeType())
                {
                    SwTxtNode* pTxtNode = (SwTxtNode*)pNode;
                    SwpHints* pHints = pTxtNode->GetpSwpHints();
                    SwTextPortionType ePortionType = PORTION_TEXT;
                    xub_StrLen nCurrentIndex = pUnoCrsr->GetPoint()->nContent.GetIndex();
                    xub_StrLen nFirstFrameIndex = STRING_MAXLEN;
                    uno::Reference< XTextRange >  xRef;
                    if(!pUnoCrsr->GetCntntNode()->Len())
                    {
                        if(pHints)
                        {
                            DBG_ERROR("hints not exported")
                        }
                        if(aBkmArr.Count())
                        {
                            lcl_ExportBookmark(aBkmArr, 0, pUnoCrsr, xParent, aPortionArr);
                        }
                        // der Absatz ist leer, also nur Portion erzeugen und raus
                        xRef = new SwXTextPortion(*pUnoCrsr, xParent, ePortionType);
                    }
                    else
                    {
                        //falls schon Rahmen entsorgt wurden, dann raus hier
                        for(sal_uInt16 nFrame = aFrameArr.Count(); nFrame; nFrame--)
                        {
                            SwDepend* pCurDepend = aFrameArr.GetObject(nFrame - 1);
                            if(!pCurDepend->GetRegisteredIn())
                            {
                                delete pCurDepend;
                                aFrameArr.Remove(nFrame - 1);
                            }
                        }

                        //zunaechst den ersten Frame im aFrameArr finden (bezogen auf die Position im Absatz)
                        SwDepend* pFirstFrameDepend = 0;
                        //Eintraege im aFrameArr sind sortiert!
                        if(aFrameArr.Count())
                        {
                            SwDepend* pCurDepend = aFrameArr.GetObject(0);
                            SwFrmFmt* pFormat = (SwFrmFmt*)pCurDepend->GetRegisteredIn();
                            const SwFmtAnchor& rAnchor = pFormat->GetAnchor();
                            const SwPosition* pAnchorPos = rAnchor.GetCntntAnchor();
                            pFirstFrameDepend = pCurDepend;
                            nFirstFrameIndex = pAnchorPos->nContent.GetIndex();
                        }

                        SwXTextCursor::SelectPam(*pUnoCrsr, sal_True);

                        //ist hier schon ein Rahmen faellig?
                        if(nCurrentIndex == nFirstFrameIndex)
                        {
                            xRef = new SwXTextPortion(*pUnoCrsr, xParent,
                                *(SwFrmFmt*)pFirstFrameDepend->GetRegisteredIn());
                            SwDepend* pCurDepend = aFrameArr.GetObject(0);
                            delete pCurDepend;
                            aFrameArr.Remove(0);
                        }
                    }
                    if(!xRef.is())
                    {
                        lcl_ExportBookmark(aBkmArr, nCurrentIndex, pUnoCrsr, xParent, aPortionArr);
                        if(pHints)
                        {
                            xRef = lcl_ExportHints(pHints,
                                aPortionArr,
                                pUnoCrsr,
                                xParent,
                                nCurrentIndex,
                                ePortionType,
                                nFirstFrameIndex,
                                aBkmArr);

                        }
                        else if(USHRT_MAX != nFirstFrameIndex)
                        {
                            pUnoCrsr->Right(nFirstFrameIndex - nCurrentIndex);
                        }
                        else
                        {
                            lcl_ExportBookmark(aBkmArr, nCurrentIndex, pUnoCrsr, xParent, aPortionArr);
                            if(!aBkmArr.Count())
                                sal_Bool bMove = pUnoCrsr->MovePara(fnParaCurr, fnParaEnd);
                            else
                            {
                                SwXBookmarkPortion_ImplPtr pPtr = aBkmArr.GetObject(0);
                                sal_uInt16 nMovePos = pPtr->nIndex;
                                DBG_ASSERT(nMovePos > nCurrentIndex, "wrong move index")
                                pUnoCrsr->Right(nMovePos - nCurrentIndex);
                            }
                        }
                    }
                    if(!xRef.is() && pUnoCrsr->HasMark() )
                        xRef = new SwXTextPortion(*pUnoCrsr, xParent, ePortionType);
                    if(xRef.is())
                        aPortionArr.Insert(new Reference<XTextRange>(xRef), aPortionArr.Count());
                }
                else
                {
                    DBG_ERROR("kein TextNode - was nun?")
                }
            }
            if(*pUnoCrsr->GetPoint() < *pUnoCrsr->GetMark())
                    pUnoCrsr->Exchange();

            // Absatzende ?
            if(pUnoCrsr->GetCntntNode() &&
                    pUnoCrsr->GetPoint()->nContent == pUnoCrsr->GetCntntNode()->Len())
            {
                bAtEnd = sal_True;
                lcl_ExportBookmark(aBkmArr, pUnoCrsr->GetCntntNode()->Len(),
                                            pUnoCrsr, xParent, aPortionArr);
                SwNode* pNode = pUnoCrsr->GetNode();
                if(ND_TEXTNODE == pNode->GetNodeType())
                {
                    SwTxtNode* pTxtNode = (SwTxtNode*)pNode;
                    SwpHints* pHints = pTxtNode->GetpSwpHints();
                    if(pHints)
                    {
                        SwTextPortionType ePortionType = PORTION_TEXT;
                        Reference<XTextRange> xRef = lcl_ExportHints(pHints,
                            aPortionArr,
                            pUnoCrsr,
                            xParent,
                            pUnoCrsr->GetCntntNode()->Len(),
                            ePortionType,
                            STRING_MAXLEN,
                            aBkmArr);
                        if(xRef.is())
                            aPortionArr.Insert(new Reference<XTextRange>(xRef), aPortionArr.Count());
                    }
                }
            }
        }
    }
}
/*-- 27.01.99 10:44:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwXTextPortionEnumeration::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

/* -----------------22.04.99 11:24-------------------
 *
 * --------------------------------------------------*/
SV_IMPL_PTRARR(SwEvtLstnrArray, XEventListenerPtr);

/*-- 22.04.99 11:24:59---------------------------------------------------

  -----------------------------------------------------------------------*/
SwEventListenerContainer::SwEventListenerContainer( uno::XInterface* pxParent) :
    pListenerArr(0),
    pxParent(pxParent)
{
}
/*-- 22.04.99 11:24:59---------------------------------------------------

  -----------------------------------------------------------------------*/
SwEventListenerContainer::~SwEventListenerContainer()
{
    if(pListenerArr && pListenerArr->Count())
    {
        pListenerArr->DeleteAndDestroy(0, pListenerArr->Count());
    }
    delete pListenerArr;
}
/*-- 22.04.99 11:24:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwEventListenerContainer::AddListener(const uno::Reference< lang::XEventListener > & rxListener)
{
    if(!pListenerArr)
        pListenerArr = new SwEvtLstnrArray;
    uno::Reference< lang::XEventListener > * pInsert = new uno::Reference< lang::XEventListener > ;
    *pInsert = rxListener;
    pListenerArr->Insert(pInsert, pListenerArr->Count());
}
/*-- 22.04.99 11:25:00---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool    SwEventListenerContainer::RemoveListener(const uno::Reference< lang::XEventListener > & rxListener)
{
    if(!pListenerArr)
        return sal_False;
    else
    {
         lang::XEventListener* pLeft = rxListener.get();
        for(sal_uInt16 i = 0; i < pListenerArr->Count(); i++)
        {
            XEventListenerPtr pElem = pListenerArr->GetObject(i);
             lang::XEventListener* pRight = pElem->get();
            if(pLeft == pRight)
            {
                pListenerArr->Remove(i);
                delete pElem;
                return sal_True;
            }
        }
    }
    return sal_False;
}
/*-- 22.04.99 11:25:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwEventListenerContainer::Disposing()
{
    if(!pListenerArr)
        return;

    lang::EventObject aObj(pxParent);
    for(sal_uInt16 i = 0; i < pListenerArr->Count(); i++)
    {
        XEventListenerPtr pElem = pListenerArr->GetObject(i);
        (*pElem)->disposing(aObj);
    }
    pListenerArr->DeleteAndDestroy(0, pListenerArr->Count());
}

