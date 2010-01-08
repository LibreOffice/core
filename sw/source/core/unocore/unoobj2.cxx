/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unoobj2.cxx,v $
 * $Revision: 1.70 $
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

#include <rtl/ustrbuf.hxx>
#include <swtypes.hxx>
#include <hintids.hxx>
#include <cmdid.h>
#include <hints.hxx>
#include <IMark.hxx>
#include <bookmrk.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <unocrsr.hxx>
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
#include <docsh.hxx>
#include <docstyle.hxx>
#include <charfmt.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hrc>
#include <poolfmt.hxx>
#include <edimp.hxx>
#include <fchrfmt.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <doctxm.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <fmtrfmrk.hxx>
#include <txtrfmrk.hxx>
#include <unoclbck.hxx>
#include <unoparaframeenum.hxx>
#include <unofootnote.hxx>
#include <unotextbodyhf.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unoparagraph.hxx>
#include <unomap.hxx>
#include <unoport.hxx>
#include <unocrsrhelper.hxx>
#include <unosett.hxx>
#include <unoprnms.hxx>
#include <unotbl.hxx>
#include <unodraw.hxx>
#include <unocoll.hxx>
#include <unostyle.hxx>
#include <unofield.hxx>
#include <fmtanchr.hxx>
#include <svx/flstitem.hxx>
#include <svx/unolingu.hxx>
#include <svtools/ctrltool.hxx>
#include <flypos.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <fmtcntnt.hxx>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <unotools/collatorwrapper.hxx>
#include <com/sun/star/table/TableSortField.hpp>
#include <unoidx.hxx>
#include <unoframe.hxx>
#include <fmthdft.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <fmtflcnt.hxx>
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_XUB_STRLEN
#include <svtools/svstdarr.hxx>
#include <svx/brshitem.hxx>
#include <fmtclds.hxx>
#include <dcontact.hxx>
#include <dflyobj.hxx>
#include <crsskip.hxx>
// OD 2004-05-07 #i28701#
#include <vector>
// OD 2004-05-24 #i28701#
#include <sortedobjs.hxx>
#include <sortopt.hxx>

#include <algorithm>
#include <iterator>
#include <boost/bind.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;

using ::rtl::OUString;


namespace sw {

sal_Bool SupportsServiceImpl(
        size_t const nServices, char const*const pServices[],
        ::rtl::OUString const & rServiceName)
{
    for (size_t i = 0; i < nServices; ++i)
    {
        if (rServiceName.equalsAscii(pServices[i]))
        {
            return sal_True;
        }
    }
    return sal_False;
}

uno::Sequence< ::rtl::OUString >
GetSupportedServiceNamesImpl(
        size_t const nServices, char const*const pServices[])
{
    uno::Sequence< ::rtl::OUString > ret(nServices);
    for (size_t i = 0; i < nServices; ++i)
    {
        ret[i] = C2U(pServices[i]);
    }
    return ret;
}

} // namespace sw


namespace sw {

void DeepCopyPaM(SwPaM const & rSource, SwPaM & rTarget)
{
    rTarget = rSource;

    if (rSource.GetNext() != &rSource)
    {
        SwPaM *pPam = static_cast<SwPaM *>(rSource.GetNext());
        do
        {
            // create new PaM
            SwPaM *const pNew = new SwPaM(*pPam);
            // insert into ring
            pNew->MoveTo(&rTarget);
            pPam = static_cast<SwPaM *>(pPam->GetNext());
        }
        while (pPam != &rSource);
    }
}

} // namespace sw

struct FrameDependSortListLess
{
    bool operator() (FrameDependSortListEntry const& r1,
                     FrameDependSortListEntry const& r2)
    {
        return  (r1.nIndex <  r2.nIndex)
            || ((r1.nIndex == r2.nIndex) && (r1.nOrder < r2.nOrder));
    }
};

// OD 2004-05-07 #i28701# - adjust 4th parameter
void CollectFrameAtNode( SwClient& rClnt, const SwNodeIndex& rIdx,
                         FrameDependSortList_t & rFrames,
                         const bool _bAtCharAnchoredObjs )
{
    // _bAtCharAnchoredObjs:
    // <TRUE>: at-character anchored objects are collected
    // <FALSE>: at-paragraph anchored objects are collected

    // alle Rahmen, Grafiken und OLEs suchen, die an diesem Absatz
    // gebunden sind
    SwDoc* pDoc = rIdx.GetNode().GetDoc();

    USHORT nChkType = static_cast< USHORT >((_bAtCharAnchoredObjs)
            ? FLY_AT_CHAR : FLY_AT_PARA);
    const SwCntntFrm* pCFrm;
    const SwCntntNode* pCNd;
    if( pDoc->GetRootFrm() &&
        0 != (pCNd = rIdx.GetNode().GetCntntNode()) &&
        0 != (pCFrm = pCNd->GetFrm()) )
    {
        const SwSortedObjs *pObjs = pCFrm->GetDrawObjs();
        if( pObjs )
            for( USHORT i = 0; i < pObjs->Count(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
                if ( rFmt.GetAnchor().GetAnchorId() == nChkType )
                {
                    // create SwDepend and insert into array
                    SwDepend* pNewDepend = new SwDepend( &rClnt, &rFmt );
                    xub_StrLen idx =
                        rFmt.GetAnchor().GetCntntAnchor()->nContent.GetIndex();
                    sal_uInt32 nOrder = rFmt.GetAnchor().GetOrder();

                    // OD 2004-05-07 #i28701# - sorting no longer needed,
                    // because list <SwSortedObjs> is already sorted.
                    FrameDependSortListEntry entry(idx, nOrder, pNewDepend);
                    rFrames.push_back(entry);
                }
            }
    }
    else
    {
        const SwSpzFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();
        USHORT nSize = rFmts.Count();
        for ( USHORT i = 0; i < nSize; i++)
        {
            const SwFrmFmt* pFmt = rFmts[ i ];
            const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
            const SwPosition* pAnchorPos;
            if( rAnchor.GetAnchorId() == nChkType &&
                0 != (pAnchorPos = rAnchor.GetCntntAnchor()) &&
                    pAnchorPos->nNode == rIdx )
            {
                //jetzt einen SwDepend anlegen und in das Array einfuegen
                SwDepend* pNewDepend = new SwDepend( &rClnt, (SwFrmFmt*)pFmt);

                // OD 2004-05-07 #i28701# - determine insert position for
                // sorted <rFrameArr>
                xub_StrLen nIndex = pAnchorPos->nContent.GetIndex();
                sal_uInt32 nOrder = rAnchor.GetOrder();

                FrameDependSortListEntry entry(nIndex, nOrder, pNewDepend);
                rFrames.push_back(entry);
            }
        }
        ::std::sort(rFrames.begin(), rFrames.end(), FrameDependSortListLess());
    }
}

/****************************************************************************
    ActionContext
****************************************************************************/
UnoActionContext::UnoActionContext(SwDoc *const pDoc)
    : m_pDoc(pDoc)
{
    SwRootFrm *const pRootFrm = m_pDoc->GetRootFrm();
    if (pRootFrm)
    {
        pRootFrm->StartAllAction();
    }
}

/*-----------------04.03.98 11:56-------------------

--------------------------------------------------*/
UnoActionContext::~UnoActionContext()
{
    // Doc may already have been removed here
    if (m_pDoc)
    {
        SwRootFrm *const pRootFrm = m_pDoc->GetRootFrm();
        if (pRootFrm)
        {
            pRootFrm->EndAllAction();
        }
    }
}

/****************************************************************************
    ActionRemoveContext
****************************************************************************/
UnoActionRemoveContext::UnoActionRemoveContext(SwDoc *const pDoc)
    : m_pDoc(pDoc)
{
    SwRootFrm *const pRootFrm = m_pDoc->GetRootFrm();
    if (pRootFrm)
    {
        pRootFrm->UnoRemoveAllActions();
    }
}

/* -----------------07.07.98 12:05-------------------
 *
 * --------------------------------------------------*/
UnoActionRemoveContext::~UnoActionRemoveContext()
{
    SwRootFrm *const pRootFrm = m_pDoc->GetRootFrm();
    if (pRootFrm)
    {
        pRootFrm->UnoRestoreAllActions();
    }
}


/*-- 09.12.98 14:18:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::insertDocumentFromURL(const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& rOptions)
    throw( lang::IllegalArgumentException, io::IOException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoCursorHelper::InsertFile(pUnoCrsr, rURL, rOptions);
    }
    else
        throw uno::RuntimeException();
}
/* -----------------------------15.12.00 14:01--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue > SwXTextCursor::createSortDescriptor(sal_Bool bFromTable)
{
    uno::Sequence< beans::PropertyValue > aRet(5);
    beans::PropertyValue* pArray = aRet.getArray();

    uno::Any aVal;
    aVal.setValue( &bFromTable, ::getCppuBooleanType());
    pArray[0] = beans::PropertyValue(C2U("IsSortInTable"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    String sSpace(String::CreateFromAscii(" "));
    sal_Unicode uSpace = sSpace.GetChar(0);

    aVal <<= uSpace;
    pArray[1] = beans::PropertyValue(C2U("Delimiter"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal <<= (sal_Bool) sal_False;
    pArray[2] = beans::PropertyValue(C2U("IsSortColumns"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal <<= (sal_Int32) 3;
    pArray[3] = beans::PropertyValue(C2U("MaxSortFieldsCount"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    uno::Sequence< table::TableSortField > aFields(3);
    table::TableSortField* pFields = aFields.getArray();

    Locale aLang( SvxCreateLocale( LANGUAGE_SYSTEM ) );
    // get collator algorithm to be used for the locale
    Sequence < OUString > aSeq( GetAppCollator().listCollatorAlgorithms( aLang ) );
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
    pArray[4] = beans::PropertyValue(C2U("SortFields"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    return aRet;
}

/*-- 09.12.98 14:18:58---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue > SwXTextCursor::createSortDescriptor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXTextCursor::createSortDescriptor(sal_False);
}
/* -----------------------------15.12.00 14:06--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXTextCursor::convertSortProperties(
    const uno::Sequence< beans::PropertyValue >& rDescriptor, SwSortOptions& rSortOpt)
{
    sal_Bool bRet = sal_True;
    const beans::PropertyValue* pProperties = rDescriptor.getConstArray();

    rSortOpt.bTable = sal_False;
    rSortOpt.cDeli = ' ';
    rSortOpt.eDirection = SRT_COLUMNS;  //!! UI text may be contrary though !!

    SwSortKey* pKey1 = new SwSortKey;
    pKey1->nColumnId = USHRT_MAX;
    pKey1->bIsNumeric = TRUE;
    pKey1->eSortOrder   = SRT_ASCENDING;

    SwSortKey* pKey2 = new SwSortKey;
    pKey2->nColumnId = USHRT_MAX;
    pKey2->bIsNumeric = TRUE;
    pKey2->eSortOrder   = SRT_ASCENDING;

    SwSortKey* pKey3 = new SwSortKey;
    pKey3->nColumnId = USHRT_MAX;
    pKey3->bIsNumeric = TRUE;
    pKey3->eSortOrder   = SRT_ASCENDING;
    SwSortKey* aKeys[3] = {pKey1, pKey2, pKey3};

    sal_Bool bOldSortdescriptor(sal_False);
    sal_Bool bNewSortdescriptor(sal_False);

    for( int n = 0; n < rDescriptor.getLength(); ++n )
    {
        uno::Any aValue( pProperties[n].Value );
//      String sPropName = pProperties[n].Name;
        const OUString& rPropName = pProperties[n].Name;

        // old and new sortdescriptor
        if( COMPARE_EQUAL == rPropName.compareToAscii("IsSortInTable"))
        {
            if ( aValue.getValueType() == ::getBooleanCppuType() )
                rSortOpt.bTable = *(sal_Bool*)aValue.getValue();
            else
                bRet = sal_False;
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("Delimiter"))
        {
            sal_Unicode uChar = sal_Unicode();
            if( aValue >>= uChar )
                rSortOpt.cDeli = uChar;
            else
                bRet = sal_False;
        }
        // old sortdescriptor
        else if(COMPARE_EQUAL == rPropName.compareToAscii("SortColumns"))
        {
            bOldSortdescriptor = sal_True;
            if ( aValue.getValueType() == ::getBooleanCppuType() )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                rSortOpt.eDirection = bTemp ? SRT_COLUMNS : SRT_ROWS;
            }
            else
                bRet = sal_False;
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("IsCaseSensitive"))
        {
            bOldSortdescriptor = sal_True;
            if ( aValue.getValueType() == ::getBooleanCppuType() )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                rSortOpt.bIgnoreCase = !bTemp;
            }
            else
                bRet = sal_False;
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("CollatorLocale"))
        {
            bOldSortdescriptor = sal_True;
            Locale aLocale;
            if (aValue >>= aLocale)
                rSortOpt.nLanguage = SvxLocaleToLanguage( aLocale );
            else
                bRet = sal_False;
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("CollatorAlgorithm", 17) &&
            rPropName.getLength() == 18 &&
            (rPropName.getStr()[17] >= '0' && rPropName.getStr()[17] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[17];
            nIndex -= '0';
            OUString aTxt;
            if ((aValue >>= aTxt) && nIndex < 3)
                aKeys[nIndex]->sSortType = aTxt;
            else
                bRet = sal_False;
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("SortRowOrColumnNo", 17) &&
            rPropName.getLength() == 18 &&
            (rPropName.getStr()[17] >= '0' && rPropName.getStr()[17] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[17];
            nIndex -= '0';
            sal_Int16 nCol = -1;
            if( aValue.getValueType() == ::getCppuType((const sal_Int16*)0) && nIndex < 3)
                aValue >>= nCol;
            if( nCol >= 0 )
                aKeys[nIndex]->nColumnId = nCol;
            else
                bRet = sal_False;
        }
        else if(0 == rPropName.indexOf(C2U("IsSortNumeric")) &&
            rPropName.getLength() == 14 &&
            (rPropName.getStr()[13] >= '0' && rPropName.getStr()[13] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[13];
            nIndex = nIndex - '0';
            if ( aValue.getValueType() == ::getBooleanCppuType() && nIndex < 3 )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                aKeys[nIndex]->bIsNumeric = bTemp;
            }
            else
                bRet = sal_False;
        }
        else if(0 == rPropName.indexOf(C2U("IsSortAscending")) &&
            rPropName.getLength() == 16 &&
            (rPropName.getStr()[15] >= '0' && rPropName.getStr()[15] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[15];
            nIndex -= '0';
            if ( aValue.getValueType() == ::getBooleanCppuType() && nIndex < 3 )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                aKeys[nIndex]->eSortOrder = bTemp ? SRT_ASCENDING : SRT_DESCENDING;
            }
            else
                bRet = sal_False;
        }
        // new sortdescriptor
        else if(COMPARE_EQUAL == rPropName.compareToAscii("IsSortColumns"))
        {
            bNewSortdescriptor = sal_True;
            if ( aValue.getValueType() == ::getBooleanCppuType() )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                rSortOpt.eDirection = bTemp ? SRT_COLUMNS : SRT_ROWS;
            }
            else
                bRet = sal_False;
        }
        else if (COMPARE_EQUAL == rPropName.compareToAscii("SortFields"))
        {
            bNewSortdescriptor = sal_True;
            uno::Sequence < table::TableSortField > aFields;
            if ( aValue >>= aFields )
            {
                sal_Int32 nCount(aFields.getLength());
                if (nCount <= 3)
                {
                    table::TableSortField* pFields = aFields.getArray();
                    for (sal_Int32 i = 0; i < nCount; ++i)
                    {
                        rSortOpt.bIgnoreCase = !pFields[i].IsCaseSensitive;
                        rSortOpt.nLanguage = SvxLocaleToLanguage( pFields[i].CollatorLocale );
                        aKeys[i]->sSortType = pFields[i].CollatorAlgorithm;
                        aKeys[i]->nColumnId = static_cast< USHORT >(pFields[i].Field);
                        aKeys[i]->bIsNumeric = (pFields[i].FieldType == table::TableSortFieldType_NUMERIC);
                        aKeys[i]->eSortOrder = pFields[i].IsAscending ? SRT_ASCENDING : SRT_DESCENDING;
                    }
                }
                else
                    bRet = sal_False;
            }
            else
                bRet = sal_False;
        }
    }

    if (bNewSortdescriptor && bOldSortdescriptor)
    {
        DBG_ERROR("someone tried to set the old deprecated and the new sortdescriptor");
        bRet = sal_False;
    }

    if(pKey1->nColumnId != USHRT_MAX)
        rSortOpt.aKeys.C40_INSERT(SwSortKey, pKey1, rSortOpt.aKeys.Count());
    if(pKey2->nColumnId != USHRT_MAX)
        rSortOpt.aKeys.C40_INSERT(SwSortKey, pKey2, rSortOpt.aKeys.Count());
    if(pKey3->nColumnId != USHRT_MAX)
        rSortOpt.aKeys.C40_INSERT(SwSortKey, pKey3, rSortOpt.aKeys.Count());

    return bRet && rSortOpt.aKeys.Count() > 0;
}
/*-- 09.12.98 14:19:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwSortOptions aSortOpt;
    SwUnoCrsr* pUnoCrsr = GetCrsr();

    if(pUnoCrsr)
    {
        if(pUnoCrsr->HasMark())
        {
            if(!SwXTextCursor::convertSortProperties(rDescriptor, aSortOpt))
                    throw uno::RuntimeException();
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
            if( nLen > nCntStt )
                nLen = nCntStt;
            pUnoCrsr->GetPoint()->nContent.Assign(pCNd, nLen );
            pUnoCrsr->SetMark();

            pUnoCrsr->GetPoint()->nNode += nOffset;
            pCNd = pUnoCrsr->GetCntntNode();
            pUnoCrsr->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
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
uno::Reference< XEnumeration >  SAL_CALL SwXTextCursor::createContentEnumeration(const OUString& rServiceName) throw( RuntimeException )
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( !pUnoCrsr || 0 != rServiceName.compareToAscii("com.sun.star.text.TextContent") )
        throw RuntimeException();

    uno::Reference< XEnumeration > xRet = new SwXParaFrameEnumeration(*pUnoCrsr, PARAFRAME_PORTION_TEXTRANGE);
    return xRet;
}
/* -----------------------------07.03.01 14:53--------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference< XEnumeration >  SwXTextCursor::createEnumeration(void) throw( RuntimeException )
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( !pUnoCrsr  )
        throw RuntimeException();
    uno::Reference<XUnoTunnel> xTunnel(xParentText, UNO_QUERY);
    SwXText* pParentText = 0;
    if(xTunnel.is())
    {
        pParentText = reinterpret_cast< SwXText *>(
                sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething(SwXText::getUnoTunnelId()) ));
    }
    DBG_ASSERT(pParentText, "parent is not a SwXText");
    if (!pParentText)
    {
        throw uno::RuntimeException();
    }

    ::std::auto_ptr<SwUnoCrsr> pNewCrsr(
        pUnoCrsr->GetDoc()->CreateUnoCrsr(*pUnoCrsr->GetPoint()) );
    if(pUnoCrsr->HasMark())
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
uno::Type  SwXTextCursor::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}
/* -----------------------------07.03.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXTextCursor::hasElements(void) throw( RuntimeException )
{
    return sal_True;
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
// -----------------------------------------------------------------------------
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


/*-- 09.12.98 14:19:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::SetCrsrAttr(SwPaM& rPam, const SfxItemSet& rSet, USHORT nAttrMode)
{
    sal_uInt16 nFlags = nsSetAttrMode::SETATTR_APICALL | (nAttrMode & nsSetAttrMode::SETATTR_NOFORMATATTR);
    if(nAttrMode & CRSR_ATTR_MODE_DONTREPLACE)
        nFlags |= nsSetAttrMode::SETATTR_DONTREPLACE;
    SwDoc* pDoc = rPam.GetDoc();
    //StartEndAction
    UnoActionContext aAction(pDoc);
    SwPaM* pCrsr = &rPam;
    if( pCrsr->GetNext() != pCrsr )     // Ring von Cursorn
    {
        pDoc->StartUndo(UNDO_INSATTR, NULL);

        SwPaM *_pStartCrsr = &rPam;
        do
        {
            if( _pStartCrsr->HasMark() && ( (CRSR_ATTR_MODE_TABLE & nAttrMode) ||
                *_pStartCrsr->GetPoint() != *_pStartCrsr->GetMark() ))
            {
                pDoc->InsertItemSet(*_pStartCrsr, rSet, nFlags);
            }
        } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != &rPam );

        pDoc->EndUndo(UNDO_INSATTR, NULL);
    }
    else
    {
//          if( !HasSelection() )
//              UpdateAttr();
        pDoc->InsertItemSet( *pCrsr, rSet, nFlags );
    }
    //#outline level,add by zhaojianwei
    if( rSet.GetItemState( RES_PARATR_OUTLINELEVEL, false ) >= SFX_ITEM_AVAILABLE )
    {
        SwTxtNode * pTmpNode = rPam.GetNode()->GetTxtNode();
        if ( pTmpNode )
        {
            rPam.GetDoc()->GetNodes().UpdateOutlineNode( *pTmpNode );
        }

    }
    //<-end,zhaojianwei
}
/*-- 09.12.98 14:19:04---------------------------------------------------

  -----------------------------------------------------------------------*/
// --> OD 2006-07-12 #i63870#
// split third parameter <bCurrentAttrOnly> into new parameters <bOnlyTxtAttr>
// and <bGetFromChrFmt> to get better control about resulting <SfxItemSet>
void SwXTextCursor::GetCrsrAttr( SwPaM& rPam,
                                 SfxItemSet& rSet,
                                 BOOL bOnlyTxtAttr,
                                 BOOL bGetFromChrFmt )
{
    static const sal_uInt16 nMaxLookup = 1000;
    SfxItemSet aSet( *rSet.GetPool(), rSet.GetRanges() );
    SfxItemSet *pSet = &rSet;
    SwPaM *_pStartCrsr = &rPam;
    do
    {
        ULONG nSttNd = _pStartCrsr->GetMark()->nNode.GetIndex(),
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
            for( ULONG n = nSttNd; n <= nEndNd; ++n )
            {
                SwNode* pNd = rPam.GetDoc()->GetNodes()[ n ];
                switch( pNd->GetNodeType() )
                {
                case ND_TEXTNODE:
                    {
                        xub_StrLen nStt = n == nSttNd ? nSttCnt : 0,
                                nEnd = n == nEndNd ? nEndCnt
                                            : ((SwTxtNode*)pNd)->GetTxt().Len();
                        ((SwTxtNode*)pNd)->GetAttr( *pSet, nStt, nEnd, bOnlyTxtAttr, bGetFromChrFmt );
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

static SwStartNode *
lcl_InitStartNode(const CursorType eType, SwUnoCrsr *const pCursor)
{
    switch (eType)
    {
        case CURSOR_TBLTEXT: // table cell: pCursor points at first paragraph
            return pCursor->Start()->nNode.GetNode().StartOfSectionNode();
        case CURSOR_SELECTION_IN_TABLE: // table
            return pCursor->Start()->nNode.GetNode().FindTableNode();
        default:
            return 0;
    }
}

static SwTable const*
lcl_InitTable(const CursorType eType, SwStartNode const*const pStartNode)
{
    switch (eType)
    {
        case CURSOR_TBLTEXT: // table cell
            return & pStartNode->FindTableNode()->GetTable();
        case CURSOR_SELECTION_IN_TABLE: // table
            return & static_cast<SwTableNode const*>(pStartNode)->GetTable();
        default:
            return 0;
    }
}

class SwXParagraphEnumeration::Impl
    : public SwClient
{

public:

    uno::Reference< text::XText > const     m_xParentText;
    const CursorType        m_eCursorType;
    /// Start node of the cell _or_ table the enumeration belongs to.
    /// Used to restrict the movement of the UNO cursor to the cell and its
    /// embedded tables.
    SwStartNode const*const m_pOwnStartNode;
    SwTable const*const     m_pOwnTable;
    const ULONG             m_nEndIndex;
    sal_Int32               m_nFirstParaStart;
    sal_Int32               m_nLastParaEnd;
    bool                    m_bFirstParagraph;
    uno::Reference< text::XTextContent >    m_xNextPara;

    Impl(   uno::Reference< text::XText > const& xParent,
            ::std::auto_ptr<SwUnoCrsr> pCursor,
            const CursorType eType)
        : SwClient( pCursor.release() )
        , m_xParentText( xParent )
        , m_eCursorType( eType )
        // remember table and start node for later travelling
        // (used in export of tables in tables)
        , m_pOwnStartNode( lcl_InitStartNode(eType, GetCursor()) )
        // for import of tables in tables we have to remember the actual
        // table and start node of the current position in the enumeration.
        , m_pOwnTable( lcl_InitTable(eType, m_pOwnStartNode) )
        , m_nEndIndex( GetCursor()->End()->nNode.GetIndex() )
        , m_nFirstParaStart( -1 )
        , m_nLastParaEnd( -1 )
        , m_bFirstParagraph( true )
    {
        OSL_ENSURE(m_xParentText.is(), "SwXParagraphEnumeration: no parent?");
        OSL_ENSURE(GetRegisteredIn(),  "SwXParagraphEnumeration: no cursor?");

        if ((CURSOR_SELECTION == m_eCursorType) ||
            (CURSOR_SELECTION_IN_TABLE == m_eCursorType))
        {
            SwUnoCrsr & rCursor = *GetCursor();
            rCursor.Normalize();
            m_nFirstParaStart = rCursor.GetPoint()->nContent.GetIndex();
            m_nLastParaEnd = rCursor.GetMark()->nContent.GetIndex();
            rCursor.DeleteMark();
        }
    }

    ~Impl() {
        // Impl owns the cursor; delete it here: SolarMutex is locked
        delete GetRegisteredIn();
    }

    SwUnoCrsr * GetCursor() {
        return static_cast<SwUnoCrsr*>(
                const_cast<SwModify*>(GetRegisteredIn()));
    }

    uno::Reference< text::XTextContent > NextElement_Impl()
        throw (container::NoSuchElementException, lang::WrappedTargetException,
                uno::RuntimeException);

    // SwClient
    virtual void    Modify(SfxPoolItem *pOld, SfxPoolItem *pNew);

};

void SwXParagraphEnumeration::Impl::Modify(SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

/*-- 10.12.98 11:52:12---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraphEnumeration::SwXParagraphEnumeration(
        uno::Reference< text::XText > const& xParent,
        ::std::auto_ptr<SwUnoCrsr> pCursor,
        const CursorType eType)
    : m_pImpl( new SwXParagraphEnumeration::Impl(xParent, pCursor, eType) )
{
}
/*-- 10.12.98 11:52:12---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraphEnumeration::~SwXParagraphEnumeration()
{
}

/* -----------------------------06.04.00 16:33--------------------------------

 ---------------------------------------------------------------------------*/
OUString SAL_CALL
SwXParagraphEnumeration::getImplementationName() throw (uno::RuntimeException)
{
    return C2U("SwXParagraphEnumeration");
}
/* -----------------------------06.04.00 16:33--------------------------------

 ---------------------------------------------------------------------------*/
static char const*const g_ServicesParagraphEnum[] =
{
    "com.sun.star.text.ParagraphEnumeration",
};
static const size_t g_nServicesParagraphEnum(
    sizeof(g_ServicesParagraphEnum)/sizeof(g_ServicesParagraphEnum[0]));

sal_Bool SAL_CALL
SwXParagraphEnumeration::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            g_nServicesParagraphEnum, g_ServicesParagraphEnum, rServiceName);
}
/* -----------------------------06.04.00 16:33--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SAL_CALL
SwXParagraphEnumeration::getSupportedServiceNames()
throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesParagraphEnum, g_ServicesParagraphEnum);
}

/*-- 10.12.98 11:52:13---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXParagraphEnumeration::hasMoreElements() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    return (m_pImpl->m_bFirstParagraph) ? sal_True : m_pImpl->m_xNextPara.is();
}
/*-- 14.08.03 13:10:14---------------------------------------------------

  -----------------------------------------------------------------------*/

//!! compare to SwShellTableCrsr::FillRects() in viscrs.cxx
static SwTableNode *
lcl_FindTopLevelTable(
        SwTableNode *const pTblNode, SwTable const*const pOwnTable)
{
    // find top-most table in current context (section) level

    SwTableNode * pLast = pTblNode;
    for (SwTableNode* pTmp = pLast;
         pTmp != NULL  &&  &pTmp->GetTable() != pOwnTable;  /* we must not go up higher than the own table! */
         pTmp = pTmp->StartOfSectionNode()->FindTableNode() )
    {
        pLast = pTmp;
    }
    return pLast;
}


static bool
lcl_CursorIsInSection(
        SwUnoCrsr const*const pUnoCrsr, SwStartNode const*const pOwnStartNode)
{
    // returns true if the cursor is in the section (or in a sub section!)
    // represented by pOwnStartNode

    bool bRes = true;
    if (pUnoCrsr && pOwnStartNode)
    {
        const SwEndNode * pOwnEndNode = pOwnStartNode->EndOfSectionNode();
        bRes = pOwnStartNode->GetIndex() <= pUnoCrsr->Start()->nNode.GetIndex() &&
               pUnoCrsr->End()->nNode.GetIndex() <= pOwnEndNode->GetIndex();
    }
    return bRes;
}


uno::Reference< text::XTextContent >
SwXParagraphEnumeration::Impl::NextElement_Impl()
throw (container::NoSuchElementException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SwUnoCrsr *const pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
    {
        throw uno::RuntimeException();
    }

    // check for exceeding selections
    if (!m_bFirstParagraph &&
        ((CURSOR_SELECTION == m_eCursorType) ||
         (CURSOR_SELECTION_IN_TABLE == m_eCursorType)))
    {
        SwPosition* pStart = pUnoCrsr->Start();
        const ::std::auto_ptr<SwUnoCrsr> aNewCrsr(
            pUnoCrsr->GetDoc()->CreateUnoCrsr(*pStart, sal_False) );
        // one may also go into tables here
        if ((CURSOR_TBLTEXT != m_eCursorType) &&
            (CURSOR_SELECTION_IN_TABLE != m_eCursorType))
        {
            aNewCrsr->SetRemainInSection( sal_False );
        }

        // os 2005-01-14: This part is only necessary to detect movements out
        // of a selection; if there is no selection we don't have to care
        SwTableNode *const pTblNode = aNewCrsr->GetNode()->FindTableNode();
        if (((CURSOR_TBLTEXT != m_eCursorType) &&
            (CURSOR_SELECTION_IN_TABLE != m_eCursorType)) && pTblNode)
        {
            aNewCrsr->GetPoint()->nNode = pTblNode->EndOfSectionIndex();
            aNewCrsr->Move(fnMoveForward, fnGoNode);
        }
        else
        {
            aNewCrsr->MovePara(fnParaNext, fnParaStart);
        }
        if (m_nEndIndex < aNewCrsr->Start()->nNode.GetIndex())
        {
            return 0;
        }
    }

    sal_Bool bInTable = sal_False;
    if (!m_bFirstParagraph)
    {
        pUnoCrsr->SetRemainInSection( sal_False );
        // what to do if already in a table?
        SwTableNode * pTblNode = pUnoCrsr->GetNode()->FindTableNode();
        pTblNode = lcl_FindTopLevelTable( pTblNode, m_pOwnTable );
        if (pTblNode && (&pTblNode->GetTable() != m_pOwnTable))
        {
            // this is a foreign table: go to end
            pUnoCrsr->GetPoint()->nNode = pTblNode->EndOfSectionIndex();
            if (!pUnoCrsr->Move(fnMoveForward, fnGoNode))
            {
                return 0;
            }
            bInTable = sal_True;
        }
    }

    uno::Reference< text::XTextContent >  xRef;
    // the cursor must remain in the current section or a subsection
    // before AND after the movement...
    if (lcl_CursorIsInSection( pUnoCrsr, m_pOwnStartNode ) &&
        (m_bFirstParagraph || bInTable ||
        (pUnoCrsr->MovePara(fnParaNext, fnParaStart) &&
            lcl_CursorIsInSection( pUnoCrsr, m_pOwnStartNode ))))
    {
        SwPosition* pStart = pUnoCrsr->Start();
        const sal_Int32 nFirstContent =
            (m_bFirstParagraph) ? m_nFirstParaStart : -1;
        const sal_Int32 nLastContent =
            (m_nEndIndex == pStart->nNode.GetIndex()) ? m_nLastParaEnd : -1;

        // position in a table, or in a simple paragraph?
        SwTableNode * pTblNode = pUnoCrsr->GetNode()->FindTableNode();
        pTblNode = lcl_FindTopLevelTable( pTblNode, m_pOwnTable );
        if (/*CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType && */
            pTblNode && (&pTblNode->GetTable() != m_pOwnTable))
        {
            // this is a foreign table
            SwFrmFmt* pTableFmt =
                static_cast<SwFrmFmt*>(pTblNode->GetTable().GetFrmFmt());
            text::XTextTable *const pTable =
                SwXTextTables::GetObject( *pTableFmt );
            xRef = static_cast<text::XTextContent*>(
                    static_cast<SwXTextTable*>(pTable));
        }
        else
        {
            text::XText *const pText = m_xParentText.get();
            xRef = SwXParagraph::CreateXParagraph(*pUnoCrsr->GetDoc(),
                *pStart->nNode.GetNode().GetTxtNode(),
                static_cast<SwXText*>(pText), nFirstContent, nLastContent);
        }
    }

    return xRef;
}
/*-- 10.12.98 11:52:14---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SAL_CALL SwXParagraphEnumeration::nextElement()
throw (container::NoSuchElementException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (m_pImpl->m_bFirstParagraph)
    {
        m_pImpl->m_xNextPara = m_pImpl->NextElement_Impl();
        m_pImpl->m_bFirstParagraph = false;
    }
    const uno::Reference< text::XTextContent > xRef = m_pImpl->m_xNextPara;
    if (!xRef.is())
    {
        throw container::NoSuchElementException();
    }
    m_pImpl->m_xNextPara = m_pImpl->NextElement_Impl();

    uno::Any aRet;
    aRet <<= xRef;
    return aRet;
}

/******************************************************************
 * SwXTextRange
 ******************************************************************/

class SwXTextRange::Impl
    : public SwClient
{

public:

    const SfxItemPropertySet &  m_rPropSet;
    const enum RangePosition    m_eRangePosition;
    SwDoc &                     m_rDoc;
    uno::Reference<text::XText> m_xParentText;
    SwDepend            m_ObjectDepend; // register at format of table or frame
    ::sw::mark::IMark * m_pMark;

    Impl(   SwDoc & rDoc, const enum RangePosition eRange,
            SwFrmFmt *const pTblFmt = 0,
            const uno::Reference< text::XText > & xParent = 0)
        : SwClient()
        , m_rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR))
        , m_eRangePosition(eRange)
        , m_rDoc(rDoc)
        , m_xParentText(xParent)
        , m_ObjectDepend(this, pTblFmt)
        , m_pMark(0)
    {
    }

    ~Impl()
    {
        // Impl owns the bookmark; delete it here: SolarMutex is locked
        Invalidate();
    }

    void Invalidate()
    {
        if (m_pMark)
        {
            m_rDoc.getIDocumentMarkAccess()->deleteMark(m_pMark);
            m_pMark = 0;
        }
    }

    const ::sw::mark::IMark * GetBookmark() const { return m_pMark; }

    // SwClient
    virtual void    Modify(SfxPoolItem *pOld, SfxPoolItem *pNew);

};

void SwXTextRange::Impl::Modify(SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    const bool bAlreadyRegistered = 0 != GetRegisteredIn();
    ClientModify(this, pOld, pNew);
    if (m_ObjectDepend.GetRegisteredIn())
    {
        ClientModify(&m_ObjectDepend, pOld, pNew);
        // if the depend was removed then the range must be removed too
        if (!m_ObjectDepend.GetRegisteredIn() && GetRegisteredIn())
        {
            const_cast<SwModify*>(GetRegisteredIn())->Remove(this);
        }
        // or if the range has been removed but the depend ist still
        // connected then the depend must be removed
        else if (bAlreadyRegistered && !GetRegisteredIn() &&
                    m_ObjectDepend.GetRegisteredIn())
        {
            const_cast<SwModify*>(m_ObjectDepend.GetRegisteredIn())
                ->Remove(& m_ObjectDepend);
        }
    }
    if (!GetRegisteredIn())
    {
        m_pMark = 0;
    }
}


SwXTextRange::SwXTextRange(SwPaM& rPam,
        const uno::Reference< text::XText > & xParent,
        const enum RangePosition eRange)
    : m_pImpl( new SwXTextRange::Impl(*rPam.GetDoc(), eRange, 0, xParent) )
{
    SetPositions(rPam);
}

SwXTextRange::SwXTextRange(SwFrmFmt& rTblFmt)
    : m_pImpl(
        new SwXTextRange::Impl(*rTblFmt.GetDoc(), RANGE_IS_TABLE, &rTblFmt) )
{
    SwTable *const pTable = SwTable::FindTable( &rTblFmt );
    SwTableNode *const pTblNode = pTable->GetTableNode();
    SwPosition aPosition( *pTblNode );
    SwPaM aPam( aPosition );

    SetPositions( aPam );
}

SwXTextRange::~SwXTextRange()
{
}

const SwDoc * SwXTextRange::GetDoc() const
{
    return & m_pImpl->m_rDoc;
}

SwDoc * SwXTextRange::GetDoc()
{
    return & m_pImpl->m_rDoc;
}


void SwXTextRange::Invalidate()
{
    m_pImpl->Invalidate();
}

void SwXTextRange::SetPositions(const SwPaM& rPam)
{
    m_pImpl->Invalidate();
    IDocumentMarkAccess* const pMA = m_pImpl->m_rDoc.getIDocumentMarkAccess();
    m_pImpl->m_pMark = pMA->makeMark(rPam, ::rtl::OUString(),
                IDocumentMarkAccess::UNO_BOOKMARK);
    m_pImpl->m_pMark->Add(m_pImpl.get());
}

void SwXTextRange::DeleteAndInsert(
        const ::rtl::OUString& rText, const bool bForceExpandHints)
throw (uno::RuntimeException)
{
    if (RANGE_IS_TABLE == m_pImpl->m_eRangePosition)
    {
        // setString on table not allowed
        throw uno::RuntimeException();
    }

    const SwPosition aPos(GetDoc()->GetNodes().GetEndOfContent());
    SwCursor aCursor(aPos, 0, false);
    if (GetPositions(aCursor))
    {
        UnoActionContext aAction(& m_pImpl->m_rDoc);
        m_pImpl->m_rDoc.StartUndo(UNDO_INSERT, NULL);
        if (aCursor.HasMark())
        {
            m_pImpl->m_rDoc.DeleteAndJoin(aCursor);
        }

        if (rText.getLength())
        {
            SwUnoCursorHelper::DocInsertStringSplitCR(
                    m_pImpl->m_rDoc, aCursor, rText, bForceExpandHints);

            SwXTextCursor::SelectPam(aCursor, sal_True);
            aCursor.Left(rText.getLength(), CRSR_SKIP_CHARS, FALSE, FALSE);
        }
        SetPositions(aCursor);
        m_pImpl->m_rDoc.EndUndo(UNDO_INSERT, NULL);
    }
}

const uno::Sequence< sal_Int8 > & SwXTextRange::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}

// XUnoTunnel
sal_Int64 SAL_CALL
SwXTextRange::getSomething(const uno::Sequence< sal_Int8 >& rId)
throw (uno::RuntimeException)
{
    return ::sw::UnoTunnelImpl<SwXTextRange>(rId, this);
}

OUString SAL_CALL
SwXTextRange::getImplementationName() throw (uno::RuntimeException)
{
    return OUString::createFromAscii("SwXTextRange");
}

static char const*const g_ServicesTextRange[] =
{
    "com.sun.star.text.TextRange",
    "com.sun.star.style.CharacterProperties",
    "com.sun.star.style.CharacterPropertiesAsian",
    "com.sun.star.style.CharacterPropertiesComplex",
    "com.sun.star.style.ParagraphProperties",
    "com.sun.star.style.ParagraphPropertiesAsian",
    "com.sun.star.style.ParagraphPropertiesComplex",
};
static const size_t g_nServicesTextRange(
    sizeof(g_ServicesTextRange)/sizeof(g_ServicesTextRange[0]));

sal_Bool SAL_CALL SwXTextRange::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            g_nServicesTextRange, g_ServicesTextRange, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXTextRange::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesTextRange, g_ServicesTextRange);
}

uno::Reference< text::XText > SAL_CALL
SwXTextRange::getText() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!m_pImpl->m_xParentText.is())
    {
        if (m_pImpl->m_eRangePosition == RANGE_IS_TABLE &&
            m_pImpl->m_ObjectDepend.GetRegisteredIn())
        {
            SwFrmFmt const*const pTblFmt = static_cast<SwFrmFmt const*>(
                    m_pImpl->m_ObjectDepend.GetRegisteredIn());
            SwTable const*const pTable = SwTable::FindTable( pTblFmt );
            SwTableNode const*const pTblNode = pTable->GetTableNode();
            const SwPosition aPosition( *pTblNode );
            m_pImpl->m_xParentText =
                ::sw::CreateParentXText(m_pImpl->m_rDoc, aPosition);
        }
    }
    OSL_ENSURE(m_pImpl->m_xParentText.is(), "SwXTextRange::getText: no text");
    return m_pImpl->m_xParentText;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextRange::getStart() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Reference< text::XTextRange >  xRet;
    ::sw::mark::IMark const * const pBkmk = m_pImpl->GetBookmark();
    if (!m_pImpl->m_xParentText.is())
    {
        getText();
    }
    if(pBkmk)
    {
        SwPaM aPam(pBkmk->GetMarkStart());
        xRet = new SwXTextRange(aPam, m_pImpl->m_xParentText);
    }
    else if (RANGE_IS_TABLE == m_pImpl->m_eRangePosition)
    {
        // start and end are this, if its a table
        xRet = this;
    }
    else
    {
        throw uno::RuntimeException();
    }
    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextRange::getEnd() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Reference< text::XTextRange >  xRet;
    ::sw::mark::IMark const * const pBkmk = m_pImpl->GetBookmark();
    if (!m_pImpl->m_xParentText.is())
    {
        getText();
    }
    if(pBkmk)
    {
        SwPaM aPam(pBkmk->GetMarkEnd());
        xRet = new SwXTextRange(aPam, m_pImpl->m_xParentText);
    }
    else if (RANGE_IS_TABLE == m_pImpl->m_eRangePosition)
    {
        // start and end are this, if its a table
        xRet = this;
    }
    else
    {
        throw uno::RuntimeException();
    }
    return xRet;
}

OUString SAL_CALL SwXTextRange::getString() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    OUString sRet;
    // for tables there is no bookmark, thus also no text
    // one could export the table as ASCII here maybe?
    SwPaM aPaM(GetDoc()->GetNodes());
    if (GetPositions(aPaM) && aPaM.HasMark())
    {
        SwXTextCursor::getTextFromPam(aPaM, sRet);
    }
    return sRet;
}

void SAL_CALL SwXTextRange::setString(const OUString& rString)
throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    DeleteAndInsert(rString, false);
}

bool SwXTextRange::GetPositions(SwPaM& rToFill) const
{
    ::sw::mark::IMark const * const pBkmk = m_pImpl->GetBookmark();
    if(pBkmk)
    {
        *rToFill.GetPoint() = pBkmk->GetMarkPos();
        if(pBkmk->IsExpanded())
        {
            rToFill.SetMark();
            *rToFill.GetMark() = pBkmk->GetOtherMarkPos();
        }
        else
        {
            rToFill.DeleteMark();
        }
        return true;
    }
    return false;
}

namespace sw {

bool XTextRangeToSwPaM( SwUnoInternalPaM & rToFill,
        const uno::Reference< text::XTextRange > & xTextRange)
{
    bool bRet = false;

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    SwXTextPortion* pPortion = 0;
    SwXText* pText = 0;
    SwXParagraph* pPara = 0;
    if(xRangeTunnel.is())
    {
        pRange  = ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
        pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);
        pPortion=
            ::sw::UnoTunnelGetImplementation<SwXTextPortion>(xRangeTunnel);
        pText   = ::sw::UnoTunnelGetImplementation<SwXText>(xRangeTunnel);
        pPara   = ::sw::UnoTunnelGetImplementation<SwXParagraph>(xRangeTunnel);
    }

    // if it's a text then create a temporary cursor there and re-use
    // the pCursor variable
    if(pText)
    {
        const uno::Reference< text::XTextCursor > xTextCursor =
            pText->createCursor();
        xTextCursor->gotoEnd(sal_True);
        const uno::Reference<lang::XUnoTunnel> xCrsrTunnel(
                xTextCursor, uno::UNO_QUERY);
        pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xCrsrTunnel);
    }
    if(pRange && pRange->GetDoc() == rToFill.GetDoc())
    {
        bRet = pRange->GetPositions(rToFill);
    }
    else
    {
        if (pPara)
        {
            bRet = pPara->SelectPaM(rToFill);
        }
        else
        {
            SwDoc* const pDoc = (pCursor) ? pCursor->GetDoc()
                : ((pPortion) ? pPortion->GetCursor()->GetDoc() : 0);
            const SwPaM* const pUnoCrsr = (pCursor) ? pCursor->GetPaM()
                : ((pPortion) ? pPortion->GetCursor() : 0);
            if (pUnoCrsr && pDoc == rToFill.GetDoc())
            {
                DBG_ASSERT((SwPaM*)pUnoCrsr->GetNext() == pUnoCrsr,
                        "what to do about rings?");
                bRet = true;
                *rToFill.GetPoint() = *pUnoCrsr->GetPoint();
                if (pUnoCrsr->HasMark())
                {
                    rToFill.SetMark();
                    *rToFill.GetMark() = *pUnoCrsr->GetMark();
                }
                else
                    rToFill.DeleteMark();
            }
        }
    }
    return bRet;
}

static bool
lcl_IsStartNodeInFormat(const bool bHeader, SwStartNode *const pSttNode,
    SwFrmFmt const*const pFrmFmt, SwFrmFmt*& rpFormat)
{
    bool bRet = false;
    const SfxItemSet& rSet = pFrmFmt->GetAttrSet();
    const SfxPoolItem* pItem;
    if (SFX_ITEM_SET == rSet.GetItemState(
            static_cast<USHORT>(bHeader ? RES_HEADER : RES_FOOTER),
            sal_True, &pItem))
    {
        SfxPoolItem *const pItemNonConst(const_cast<SfxPoolItem *>(pItem));
        SwFrmFmt *const pHeadFootFmt = (bHeader) ?
            static_cast<SwFmtHeader*>(pItemNonConst)->GetHeaderFmt() :
            static_cast<SwFmtFooter*>(pItemNonConst)->GetFooterFmt();
        if (pHeadFootFmt)
        {
            const SwFmtCntnt& rFlyCntnt = pHeadFootFmt->GetCntnt();
            const SwNode& rNode = rFlyCntnt.GetCntntIdx()->GetNode();
            SwStartNode const*const pCurSttNode = rNode.FindSttNodeByType(
                (bHeader) ? SwHeaderStartNode : SwFooterStartNode);
            if (pCurSttNode && (pCurSttNode == pSttNode))
            {
                rpFormat = pHeadFootFmt;
                bRet = true;
            }
        }
    }
    return bRet;
}

} // namespace sw

uno::Reference< text::XTextRange >
SwXTextRange::CreateXTextRange(
    SwDoc & rDoc, const SwPosition& rPos, const SwPosition *const pMark)
{
    const uno::Reference<text::XText> xParentText(
            ::sw::CreateParentXText(rDoc, rPos));
    const ::std::auto_ptr<SwUnoCrsr> pNewCrsr(
            rDoc.CreateUnoCrsr(rPos, sal_False));
    if(pMark)
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pMark;
    }
    const bool isCell( dynamic_cast<SwXCell*>(xParentText.get()) );
    const uno::Reference< text::XTextRange > xRet(
        new SwXTextRange(*pNewCrsr, xParentText,
            isCell ? RANGE_IN_CELL : RANGE_IN_TEXT) );
    return xRet;
}

namespace sw {

uno::Reference< text::XText >
CreateParentXText(SwDoc & rDoc, const SwPosition& rPos)
{
    uno::Reference< text::XText > xParentText;
    SwStartNode* pSttNode = rPos.nNode.GetNode().StartOfSectionNode();
    while(pSttNode && pSttNode->IsSectionNode())
    {
        pSttNode = pSttNode->StartOfSectionNode();
    }
    SwStartNodeType eType = pSttNode->GetStartNodeType();
    switch(eType)
    {
        case SwTableBoxStartNode:
        {
            SwTableNode const*const pTblNode = pSttNode->FindTableNode();
            SwFrmFmt *const pTableFmt =
                static_cast<SwFrmFmt*>(pTblNode->GetTable().GetFrmFmt());
            SwTableBox *const  pBox = pSttNode->GetTblBox();

            xParentText = (pBox)
                ? SwXCell::CreateXCell( pTableFmt, pBox )
                : new SwXCell( pTableFmt, *pSttNode );
        }
        break;
        case SwFlyStartNode:
        {
            SwFrmFmt *const pFmt = pSttNode->GetFlyFmt();
            if (0 != pFmt)
            {
                SwXTextFrame* pFrame( static_cast<SwXTextFrame*>(
                    SwClientIter( *pFmt ).First( TYPE( SwXTextFrame ) ) ) );
                xParentText = pFrame ? pFrame : new SwXTextFrame( *pFmt );
            }
        }
        break;
        case SwHeaderStartNode:
        case SwFooterStartNode:
        {
            const bool bHeader = (SwHeaderStartNode == eType);
            const sal_uInt16 nPDescCount = rDoc.GetPageDescCnt();
            for(sal_uInt16 i = 0; i < nPDescCount; i++)
            {
                const SwPageDesc& rDesc =
                    // C++ is retarded
                    const_cast<SwDoc const&>(rDoc).GetPageDesc( i );
                const SwFrmFmt* pFrmFmtMaster = &rDesc.GetMaster();
                const SwFrmFmt* pFrmFmtLeft = &rDesc.GetLeft();

                SwFrmFmt* pHeadFootFmt = 0;
                if (!lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrmFmtMaster,
                            pHeadFootFmt))
                {
                    lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrmFmtLeft,
                            pHeadFootFmt);
                }

                if(pHeadFootFmt)
                {
                    SwXHeadFootText* pxHdFt = (SwXHeadFootText*)SwClientIter( *pHeadFootFmt ).
                                    First( TYPE( SwXHeadFootText ));
                    xParentText = pxHdFt;
                    if(!pxHdFt)
                    {
                        xParentText =
                            new SwXHeadFootText(*pHeadFootFmt, bHeader);
                    }
                    break;
                }
            }
        }
        break;
        case SwFootnoteStartNode:
        {
            const sal_uInt16 nFtnCnt = rDoc.GetFtnIdxs().Count();
            uno::Reference< text::XFootnote >  xRef;
            for (sal_uInt16 n = 0; n < nFtnCnt; ++n )
            {
                const SwTxtFtn* pTxtFtn = rDoc.GetFtnIdxs()[ n ];
                const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
                pTxtFtn = rFtn.GetTxtFtn();
#if OSL_DEBUG_LEVEL > 1
                const SwStartNode* pTmpSttNode =
                        pTxtFtn->GetStartNode()->GetNode().
                                FindSttNodeByType(SwFootnoteStartNode);
                (void)pTmpSttNode;
#endif

                if (pSttNode == pTxtFtn->GetStartNode()->GetNode().
                                    FindSttNodeByType(SwFootnoteStartNode))
                {
                    xParentText =
                        static_cast<SwUnoCallBack*>(rDoc.GetUnoCallBack())->
                                                            GetFootnote(rFtn);
                    if (!xParentText.is())
                    {
                        xParentText = new SwXFootnote(&rDoc, rFtn);
                    }
                    break;
                }
            }
        }
        break;
        default:
        {
            // then it is the body text
            const uno::Reference<frame::XModel> xModel =
                rDoc.GetDocShell()->GetBaseModel();
            const uno::Reference< text::XTextDocument > xDoc(
                xModel, uno::UNO_QUERY);
            xParentText = xDoc->getText();
        }
    }
    OSL_ENSURE(xParentText.is(), "no parent text?");
    return xParentText;
}

} // namespace sw

uno::Reference< container::XEnumeration > SAL_CALL
SwXTextRange::createContentEnumeration(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (!rServiceName.equalsAscii("com.sun.star.text.TextContent"))
    {
        throw uno::RuntimeException();
    }

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    const SwPosition aPos(GetDoc()->GetNodes().GetEndOfContent());
    const ::std::auto_ptr<SwUnoCrsr> pNewCrsr(
            m_pImpl->m_rDoc.CreateUnoCrsr(aPos, FALSE));
    if (!GetPositions(*pNewCrsr))
    {
        throw uno::RuntimeException();
    }

    const uno::Reference< container::XEnumeration > xRet =
        new SwXParaFrameEnumeration(*pNewCrsr, PARAFRAME_PORTION_TEXTRANGE);
    return xRet;
}

uno::Reference< container::XEnumeration > SAL_CALL
SwXTextRange::createEnumeration() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    const SwPosition aPos(GetDoc()->GetNodes().GetEndOfContent());
    ::std::auto_ptr<SwUnoCrsr> pNewCrsr(
            m_pImpl->m_rDoc.CreateUnoCrsr(aPos, FALSE));
    if (!GetPositions(*pNewCrsr))
    {
        throw uno::RuntimeException();
    }
    if (!m_pImpl->m_xParentText.is())
    {
        getText();
    }

    const CursorType eSetType = (RANGE_IN_CELL == m_pImpl->m_eRangePosition)
            ? CURSOR_SELECTION_IN_TABLE : CURSOR_SELECTION;
    const uno::Reference< container::XEnumeration > xRet =
        new SwXParagraphEnumeration(m_pImpl->m_xParentText, pNewCrsr, eSetType);
    return xRet;
}

uno::Type SAL_CALL SwXTextRange::getElementType() throw (uno::RuntimeException)
{
    return text::XTextRange::static_type();
}

sal_Bool SAL_CALL SwXTextRange::hasElements() throw (uno::RuntimeException)
{
    return sal_True;
}

uno::Sequence< OUString > SAL_CALL
SwXTextRange::getAvailableServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString::createFromAscii("com.sun.star.text.TextContent");
    return aRet;
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXTextRange::getPropertySetInfo() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    static uno::Reference< beans::XPropertySetInfo > xRef =
        m_pImpl->m_rPropSet.getPropertySetInfo();
    return xRef;
}

void SAL_CALL
SwXTextRange::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    GetPositions(aPaM);
    SwXTextCursor::SetPropertyValue(aPaM, m_pImpl->m_rPropSet,
            rPropertyName, rValue);
}

uno::Any SAL_CALL
SwXTextRange::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    GetPositions(aPaM);
    return SwXTextCursor::GetPropertyValue(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

void SAL_CALL
SwXTextRange::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXTextRange::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextRange::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXTextRange::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextRange::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXTextRange::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXTextRange::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXTextRange::removeVetoableChangeListener(): not implemented");
}

beans::PropertyState SAL_CALL
SwXTextRange::getPropertyState(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    GetPositions(aPaM);
    return SwXTextCursor::GetPropertyState(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

uno::Sequence< beans::PropertyState > SAL_CALL
SwXTextRange::getPropertyStates(const uno::Sequence< OUString >& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    GetPositions(aPaM);
    return SwXTextCursor::GetPropertyStates(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

void SAL_CALL SwXTextRange::setPropertyToDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    GetPositions(aPaM);
    SwXTextCursor::SetPropertyToDefault(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

uno::Any SAL_CALL
SwXTextRange::getPropertyDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    GetPositions(aPaM);
    return SwXTextCursor::GetPropertyDefault(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

void SAL_CALL
SwXTextRange::makeRedline(
    const ::rtl::OUString& rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties )
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    SwUnoCursorHelper::makeRedline( aPaM, rRedlineType, rRedlineProperties );
}

/******************************************************************
 * SwXTextRanges
 ******************************************************************/

class SwXTextRanges::Impl
    : public SwClient
{

public:

    ::std::vector< uno::Reference< text::XTextRange > > m_Ranges;

    Impl(SwPaM *const pPaM)
        : SwClient( (pPaM)
            ? pPaM->GetDoc()->CreateUnoCrsr(*pPaM->GetPoint())
            : 0 )
    {
        if (pPaM)
        {
            ::sw::DeepCopyPaM(*pPaM, *GetCursor());
        }
        MakeRanges();
    }

    ~Impl() {
        // Impl owns the cursor; delete it here: SolarMutex is locked
        delete GetRegisteredIn();
    }

    SwUnoCrsr * GetCursor() {
        return static_cast<SwUnoCrsr*>(
                const_cast<SwModify*>(GetRegisteredIn()));
    }

    void MakeRanges();

    // SwClient
    virtual void    Modify(SfxPoolItem *pOld, SfxPoolItem *pNew);

};

/*-- 10.12.98 13:57:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextRanges::Impl::Modify(SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

/* -----------------10.12.98 14:25-------------------
 *
 * --------------------------------------------------*/
void SwXTextRanges::Impl::MakeRanges()
{
    SwUnoCrsr *const pCursor = GetCursor();
    if (pCursor)
    {
        SwPaM *pTmpCursor = pCursor;
        do {
            const uno::Reference< text::XTextRange > xRange(
                    SwXTextRange::CreateXTextRange(
                        *pTmpCursor->GetDoc(),
                        *pTmpCursor->GetPoint(), pTmpCursor->GetMark()));
            if (xRange.is())
            {
                m_Ranges.push_back(xRange);
            }
            pTmpCursor = static_cast<SwPaM*>(pTmpCursor->GetNext());
        }
        while (pTmpCursor != pCursor);
    }
}

const SwUnoCrsr* SwXTextRanges::GetCursor() const
{
    return m_pImpl->GetCursor();
}

/*-- 10.12.98 13:57:22---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextRanges::SwXTextRanges(SwPaM *const pPaM)
    : m_pImpl( new SwXTextRanges::Impl(pPaM) )
{
}

/*-- 10.12.98 13:57:22---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextRanges::~SwXTextRanges()
{
}

/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextRanges::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL
SwXTextRanges::getSomething(const uno::Sequence< sal_Int8 >& rId)
throw (uno::RuntimeException)
{
    return ::sw::UnoTunnelImpl<SwXTextRanges>(rId, this);
}

/****************************************************************************
 *  Text positions
 *  Bis zum ersten Zugriff auf eine TextPosition wird ein SwCursor gehalten,
 * danach wird ein Array mit uno::Reference< XTextPosition >  angelegt
 *
****************************************************************************/
/* -----------------------------06.04.00 16:36--------------------------------

 ---------------------------------------------------------------------------*/
OUString SAL_CALL
SwXTextRanges::getImplementationName() throw (uno::RuntimeException)
{
    return C2U("SwXTextRanges");
}
/* -----------------------------06.04.00 16:36--------------------------------

 ---------------------------------------------------------------------------*/
static char const*const g_ServicesTextRanges[] =
{
    "com.sun.star.text.TextRanges",
};
static const size_t g_nServicesTextRanges(
    sizeof(g_ServicesTextRanges)/sizeof(g_ServicesTextRanges[0]));

sal_Bool SAL_CALL SwXTextRanges::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            g_nServicesTextRanges, g_ServicesTextRanges, rServiceName);
}

/* -----------------------------06.04.00 16:36--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SAL_CALL
SwXTextRanges::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesTextRanges, g_ServicesTextRanges);
}

/*-- 10.12.98 13:57:24---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SAL_CALL SwXTextRanges::getCount() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    return static_cast<sal_Int32>(m_pImpl->m_Ranges.size());
}
/*-- 10.12.98 13:57:25---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SAL_CALL SwXTextRanges::getByIndex(sal_Int32 nIndex)
throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if ((nIndex < 0) ||
        (static_cast<size_t>(nIndex) >= m_pImpl->m_Ranges.size()))
    {
        throw lang::IndexOutOfBoundsException();
    }
    uno::Any ret;
    ret <<= (m_pImpl->m_Ranges.at(nIndex));
    return ret;
}

/*-- 10.12.98 13:57:25---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SAL_CALL
SwXTextRanges::getElementType() throw (uno::RuntimeException)
{
    return text::XTextRange::static_type();
}
/*-- 10.12.98 13:57:26---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL SwXTextRanges::hasElements() throw (uno::RuntimeException)
{
    // no mutex necessary: getCount() does locking
    return getCount() > 0;
}

/* -----------------11.12.98 10:07-------------------
 *
 * --------------------------------------------------*/
void SwXTextCursor::SetString(SwCursor& rCrsr, const OUString& rString)
{
    // Start/EndAction
    SwDoc* pDoc = rCrsr.GetDoc();
    UnoActionContext aAction(pDoc);
    String aText(rString);
    xub_StrLen nTxtLen = aText.Len();
    pDoc->StartUndo(UNDO_INSERT, NULL);
    if(rCrsr.HasMark())
        pDoc->DeleteAndJoin(rCrsr);
    if(nTxtLen)
    {
        const bool bSuccess( SwUnoCursorHelper::DocInsertStringSplitCR(
                    *pDoc, rCrsr, aText, false ) );
        DBG_ASSERT( bSuccess, "DocInsertStringSplitCR" );
        (void) bSuccess;
        SwXTextCursor::SelectPam(rCrsr, sal_True);
        rCrsr.Left(nTxtLen, CRSR_SKIP_CHARS, FALSE, FALSE);
    }
    pDoc->EndUndo(UNDO_INSERT, NULL);
}

/******************************************************************
 * SwXParaFrameEnumeration
 ******************************************************************/

class SwXParaFrameEnumeration::Impl
    : public SwClient
{

public:

    // created by hasMoreElements
    uno::Reference< text::XTextContent > m_xNextObject;
    FrameDependList_t m_Frames;

    Impl(SwPaM const & rPaM)
        : SwClient(rPaM.GetDoc()->CreateUnoCrsr(*rPaM.GetPoint(), sal_False))
    {
        if (rPaM.HasMark())
        {
            GetCursor()->SetMark();
            *GetCursor()->GetMark() = *rPaM.GetMark();
        }
    }

    ~Impl() {
        // Impl owns the cursor; delete it here: SolarMutex is locked
        delete GetRegisteredIn();
    }

    SwUnoCrsr * GetCursor() {
        return static_cast<SwUnoCrsr*>(
                const_cast<SwModify*>(GetRegisteredIn()));
    }

    // SwClient
    virtual void    Modify(SfxPoolItem *pOld, SfxPoolItem *pNew);

};

/*-- 23.03.99 13:22:37---------------------------------------------------

  -----------------------------------------------------------------------*/

struct InvalidFrameDepend {
    bool operator() (::boost::shared_ptr<SwDepend> const & rEntry)
    { return !rEntry->GetRegisteredIn(); }
};

void SwXParaFrameEnumeration::Impl::Modify(SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        m_Frames.clear();
        m_xNextObject = 0;
    }
    else
    {
        // check if any frame went away...
        FrameDependList_t::iterator const iter =
            ::std::remove_if(m_Frames.begin(), m_Frames.end(),
                    InvalidFrameDepend());
        m_Frames.erase(iter, m_Frames.end());
    }
}

/* -----------------23.03.99 13:38-------------------
 *
 * --------------------------------------------------*/
static sal_Bool
lcl_CreateNextObject(SwUnoCrsr& i_rUnoCrsr,
        uno::Reference<text::XTextContent> & o_rNextObject,
        FrameDependList_t & i_rFrames)
{
    if (!i_rFrames.size())
        return sal_False;

    SwFrmFmt *const pFormat = static_cast<SwFrmFmt*>(const_cast<SwModify*>(
                i_rFrames.front()->GetRegisteredIn()));
    i_rFrames.pop_front();
    // the format should be valid here, otherwise the client
    // would have been removed in ::Modify
    // check for a shape first
    SwClientIter aIter(*pFormat);
    SwDrawContact * const pContact =
        static_cast<SwDrawContact*>( aIter.First(TYPE(SwDrawContact)) );
    if (pContact)
    {
        SdrObject * const pSdr = pContact->GetMaster();
        if (pSdr)
        {
            o_rNextObject.set(pSdr->getUnoShape(), uno::UNO_QUERY);
        }
    }
    else
    {
        const SwNodeIndex* pIdx = pFormat->GetCntnt().GetCntntIdx();
        DBG_ASSERT(pIdx, "where is the index?");
        SwNode const*const pNd =
            i_rUnoCrsr.GetDoc()->GetNodes()[ pIdx->GetIndex() + 1 ];

        const FlyCntType eType = (!pNd->IsNoTxtNode()) ? FLYCNTTYPE_FRM
            : ( (pNd->IsGrfNode()) ? FLYCNTTYPE_GRF : FLYCNTTYPE_OLE );

        const uno::Reference< container::XNamed >  xFrame =
            SwXFrames::GetObject(*pFormat, eType);
        o_rNextObject.set(xFrame, uno::UNO_QUERY);
    }

    return o_rNextObject.is();
}

/* -----------------------------03.04.00 10:15--------------------------------
    Description: Search for a FLYCNT text attribute at the cursor point
                and fill the frame into the array
 ---------------------------------------------------------------------------*/
static void
lcl_FillFrame(SwClient & rEnum, SwUnoCrsr& rUnoCrsr,
        FrameDependList_t & rFrames)
{
    // search for objects at the cursor - anchored at/as char
    SwTxtAttr const*const pTxtAttr =
        rUnoCrsr.GetNode()->GetTxtNode()->GetTxtAttr(
                    rUnoCrsr.GetPoint()->nContent, RES_TXTATR_FLYCNT);
    if (pTxtAttr)
    {
        const SwFmtFlyCnt& rFlyCnt = pTxtAttr->GetFlyCnt();
        SwFrmFmt * const  pFrmFmt = rFlyCnt.GetFrmFmt();
        SwDepend * const pNewDepend = new SwDepend(&rEnum, pFrmFmt);
        rFrames.push_back( ::boost::shared_ptr<SwDepend>(pNewDepend) );
    }
}

/*-- 23.03.99 13:22:29---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParaFrameEnumeration::SwXParaFrameEnumeration(
        const SwPaM& rPaM, const enum ParaFrameMode eParaFrameMode,
        SwFrmFmt *const pFmt)
    : m_pImpl( new SwXParaFrameEnumeration::Impl(rPaM) )
{
    if (PARAFRAME_PORTION_PARAGRAPH == eParaFrameMode)
    {
        FrameDependSortList_t frames;
        ::CollectFrameAtNode(*m_pImpl.get(), rPaM.GetPoint()->nNode,
                frames, false);
        ::std::transform(frames.begin(), frames.end(),
            ::std::back_inserter(m_pImpl->m_Frames),
            ::boost::bind(&FrameDependSortListEntry::pFrameDepend, _1));
    }
    else if (pFmt)
    {
        // create SwDepend for frame and insert into array
        SwDepend *const pNewDepend = new SwDepend(m_pImpl.get(), pFmt);
        m_pImpl->m_Frames.push_back(::boost::shared_ptr<SwDepend>(pNewDepend));
    }
    else if ((PARAFRAME_PORTION_CHAR == eParaFrameMode) ||
             (PARAFRAME_PORTION_TEXTRANGE == eParaFrameMode))
    {
        if (PARAFRAME_PORTION_TEXTRANGE == eParaFrameMode)
        {
            SwPosFlyFrms aFlyFrms;
            //get all frames that are bound at paragraph or at character
            rPaM.GetDoc()->GetAllFlyFmts(aFlyFrms, m_pImpl->GetCursor());
            for(USHORT i = 0; i < aFlyFrms.Count(); i++)
            {
                SwPosFlyFrm* pPosFly = aFlyFrms[i];
                SwFrmFmt *const pFrmFmt =
                    const_cast<SwFrmFmt*>(&pPosFly->GetFmt());
                // create SwDepend for frame and insert into array
                SwDepend *const pNewDepend =
                    new SwDepend(m_pImpl.get(), pFrmFmt);
                m_pImpl->m_Frames.push_back(
                        ::boost::shared_ptr<SwDepend>(pNewDepend) );
            }
            //created from any text range
            if (m_pImpl->GetCursor()->HasMark())
            {
                m_pImpl->GetCursor()->Normalize();
                do
                {
                    lcl_FillFrame(*m_pImpl.get(), *m_pImpl->GetCursor(),
                            m_pImpl->m_Frames);
                    m_pImpl->GetCursor()->Right(
                            1, CRSR_SKIP_CHARS, FALSE, FALSE);
                }
                while (*m_pImpl->GetCursor()->GetPoint() <
                        *m_pImpl->GetCursor()->GetMark());
            }
        }
        lcl_FillFrame(*m_pImpl.get(), *m_pImpl->GetCursor(), m_pImpl->m_Frames);
    }
}
/*-- 23.03.99 13:22:30---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParaFrameEnumeration::~SwXParaFrameEnumeration()
{
}

/*-- 23.03.99 13:22:32---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXParaFrameEnumeration::hasMoreElements() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!m_pImpl->GetCursor())
        throw uno::RuntimeException();

    return (m_pImpl->m_xNextObject.is())
        ? sal_True
        : lcl_CreateNextObject(*m_pImpl->GetCursor(),
            m_pImpl->m_xNextObject, m_pImpl->m_Frames);
}
/*-- 23.03.99 13:22:33---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SAL_CALL SwXParaFrameEnumeration::nextElement()
throw (container::NoSuchElementException,
        lang::WrappedTargetException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!m_pImpl->GetCursor())
    {
        throw uno::RuntimeException();
    }

    if (!m_pImpl->m_xNextObject.is() && m_pImpl->m_Frames.size())
    {
        lcl_CreateNextObject(*m_pImpl->GetCursor(),
                m_pImpl->m_xNextObject, m_pImpl->m_Frames);
    }
    if (!m_pImpl->m_xNextObject.is())
    {
        throw container::NoSuchElementException();
    }
    uno::Any aRet;
    aRet <<= m_pImpl->m_xNextObject;
    m_pImpl->m_xNextObject = 0;
    return aRet;
}

/* -----------------------------06.04.00 16:39--------------------------------

 ---------------------------------------------------------------------------*/
OUString SAL_CALL
SwXParaFrameEnumeration::getImplementationName() throw (uno::RuntimeException)
{
    return C2U("SwXParaFrameEnumeration");
}

/* -----------------------------06.04.00 16:39--------------------------------

 ---------------------------------------------------------------------------*/
static char const*const g_ServicesParaFrameEnum[] =
{
    "com.sun.star.util.ContentEnumeration",
};
static const size_t g_nServicesParaFrameEnum(
    sizeof(g_ServicesParaFrameEnum)/sizeof(g_ServicesParaFrameEnum[0]));

sal_Bool SAL_CALL
SwXParaFrameEnumeration::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            g_nServicesParaFrameEnum, g_ServicesParaFrameEnum, rServiceName);
}

/* -----------------------------06.04.00 16:39--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SAL_CALL
SwXParaFrameEnumeration::getSupportedServiceNames()
throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesParaFrameEnum, g_ServicesParaFrameEnum);
}


// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_REFCOUNT( SwXTextCursor,SwXTextCursor_Base )

uno::Any SAL_CALL SwXTextCursor::queryInterface( const uno::Type& _rType ) throw (uno::RuntimeException)
{
    if( _rType == ::getCppuType( (uno::Reference<XUnoTunnel>*)0)  )
        return OTextCursorHelper::queryInterface( _rType );
    else
        return SwXTextCursor_Base::queryInterface( _rType );
}


