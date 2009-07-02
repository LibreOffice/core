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
#include <fmthbsh.hxx>
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
#include <unoobj.hxx>
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;

using ::rtl::OUString;

// OD 2004-05-07 #i28701# - adjust 4th parameter
void CollectFrameAtNode( SwClient& rClnt, const SwNodeIndex& rIdx,
                         SwDependArr& rFrameArr,
                         const bool _bAtCharAnchoredObjs )
{
    // _bAtCharAnchoredObjs:
    // <TRUE>: at-character anchored objects are collected
    // <FALSE>: at-paragraph anchored objects are collected

    // alle Rahmen, Grafiken und OLEs suchen, die an diesem Absatz
    // gebunden sind
    SwDoc* pDoc = rIdx.GetNode().GetDoc();

    USHORT nChkType = static_cast< USHORT >(_bAtCharAnchoredObjs ? FLY_AUTO_CNTNT : FLY_AT_CNTNT);
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
                    //jetzt einen SwDepend anlegen und in das Array einfuegen
                    SwDepend* pNewDepend = new SwDepend( &rClnt, &rFmt );

                    // OD 2004-05-07 #i28701# - sorting no longer needed,
                    // because list <SwSortedObjs> is already sorted.
                    rFrameArr.C40_INSERT( SwDepend, pNewDepend, rFrameArr.Count() );
                }
            }
    }
    else
    {
        // OD 2004-05-07 #i28701# - helper list to get <rFrameArr> sorted
        std::vector< std::pair< xub_StrLen, sal_uInt32 > > aSortLst;
        typedef std::vector< std::pair< xub_StrLen, sal_uInt32 > >::iterator tSortLstIter;

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
                USHORT nInsPos = rFrameArr.Count();
                {
                    xub_StrLen nCntIndex = pAnchorPos->nContent.GetIndex();
                    sal_uInt32 nAnchorOrder = rAnchor.GetOrder();

                    tSortLstIter aInsIter = aSortLst.end();
                    for ( tSortLstIter aIter = aSortLst.begin();
                          aIter != aSortLst.end();
                          ++aIter )
                    {
                        if ( (*aIter).first > nCntIndex ||
                             ( (*aIter).first == nCntIndex &&
                               (*aIter).second > nAnchorOrder ) )
                        {
                            nInsPos = sal::static_int_cast< USHORT >(aIter - aSortLst.begin());
                            aInsIter = aIter;
                            break;
                        }
                    }
                    std::pair< xub_StrLen, sal_uInt32 > aEntry( nCntIndex,
                                                                nAnchorOrder );
                    aSortLst.insert( aInsIter, aEntry );
                }
                rFrameArr.C40_INSERT( SwDepend, pNewDepend, nInsPos );
            }
        }
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

    SwUnoCrsr* pNewCrsr = pUnoCrsr->GetDoc()->CreateUnoCrsr(*pUnoCrsr->GetPoint());
    if(pUnoCrsr->HasMark())
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pUnoCrsr->GetMark();
    }
    CursorType eSetType = eType == CURSOR_TBLTEXT ? CURSOR_SELECTION_IN_TABLE : CURSOR_SELECTION;
    SwXParagraphEnumeration *pEnum =
        new SwXParagraphEnumeration(pParentText, pNewCrsr, eSetType);
    uno::Reference< XEnumeration > xRet = pEnum;
    if (eType == CURSOR_TBLTEXT)
    {
        // for import of tables in tables we have to remember the actual
        // table and start node of the current position in the enumeration.
        SwTableNode *pStartN = pUnoCrsr->GetPoint()->nNode.GetNode().FindTableNode();
        if (pStartN)
        {
            pEnum->SetOwnTable( &pStartN->GetTable() );
            pEnum->SetOwnStartNode( pStartN );
        }
    }

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
                pDoc->Insert(*_pStartCrsr, rSet, nFlags );
        } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != &rPam );

        pDoc->EndUndo(UNDO_INSATTR, NULL);
    }
    else
    {
//          if( !HasSelection() )
//              UpdateAttr();
        pDoc->Insert( *pCrsr, rSet, nFlags );
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
        nFirstParaStart(-1),
        nLastParaEnd(-1),
        nEndIndex(rPos.nNode.GetIndex()),
        eCursorType(eType),
        bFirstParagraph(sal_True)
{
    pOwnTable = 0;
    pOwnStartNode = 0;
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
        nFirstParaStart(-1),
        nLastParaEnd(-1),
        nEndIndex(pCrsr->End()->nNode.GetIndex()),
        eCursorType(eType),
        bFirstParagraph(sal_True)
{
    pOwnTable = 0;
    pOwnStartNode = 0;
    if(CURSOR_SELECTION == eCursorType || CURSOR_SELECTION_IN_TABLE == eCursorType)
    {
        if(*pCrsr->GetPoint() > *pCrsr->GetMark())
            pCrsr->Exchange();
        nFirstParaStart = pCrsr->GetPoint()->nContent.GetIndex();
        nLastParaEnd = pCrsr->GetMark()->nContent.GetIndex();
        if(pCrsr->HasMark())
            pCrsr->DeleteMark();
    }
}
/*-- 10.12.98 11:52:12---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParagraphEnumeration::~SwXParagraphEnumeration()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        delete pUnoCrsr;

}
/*-- 10.12.98 11:52:13---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXParagraphEnumeration::hasMoreElements(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return bFirstParagraph ? sal_True : xNextPara.is();
}
/*-- 14.08.03 13:10:14---------------------------------------------------

  -----------------------------------------------------------------------*/

//!! compare to SwShellTableCrsr::FillRects() in viscrs.cxx
SwTableNode * lcl_FindTopLevelTable(
        /*SwUnoCrsr* pUnoCrsr ,*/
        SwTableNode *pTblNode,
        const SwTable *pOwnTable )
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


BOOL lcl_CursorIsInSection(
        const SwUnoCrsr *pUnoCrsr,
        const SwStartNode *pOwnStartNode )
{
    // returns true if the cursor is in the section (or in a sub section!)
    // represented by pOwnStartNode

    BOOL bRes = TRUE;
    if (pUnoCrsr && pOwnStartNode)
    {
        const SwEndNode * pOwnEndNode = pOwnStartNode->EndOfSectionNode();
        bRes = pOwnStartNode->GetIndex() <= pUnoCrsr->Start()->nNode.GetIndex() &&
               pUnoCrsr->End()->nNode.GetIndex() <= pOwnEndNode->GetIndex();
    }
    return bRes;
}


uno::Reference< XTextContent > SAL_CALL SwXParagraphEnumeration::NextElement_Impl(void)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Reference< XTextContent >  aRef;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        // check for exceeding selections
        if(!bFirstParagraph &&
            (CURSOR_SELECTION == eCursorType || CURSOR_SELECTION_IN_TABLE == eCursorType))
        {
            SwPosition* pStart = pUnoCrsr->Start();
            ::std::auto_ptr<SwUnoCrsr> aNewCrsr( pUnoCrsr->GetDoc()->CreateUnoCrsr(*pStart, sal_False) );
            //man soll hier auch in Tabellen landen duerfen
            if(CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType)
                aNewCrsr->SetRemainInSection( sal_False );

            // os 2005-01-14: This part is only necessary to detect movements out of a selection
            // if there is no selection we don't have to care
            SwTableNode* pTblNode = aNewCrsr->GetNode()->FindTableNode();
            if((CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType) && pTblNode)
            {
                aNewCrsr->GetPoint()->nNode = pTblNode->EndOfSectionIndex();
                aNewCrsr->Move(fnMoveForward, fnGoNode);
            }
            else
                aNewCrsr->MovePara(fnParaNext, fnParaStart);
            if(nEndIndex < aNewCrsr->Start()->nNode.GetIndex())
                return aRef;    // empty reference
        }

        XText* pText = xParentText.get();
        sal_Bool bInTable = sal_False;
        if(!bFirstParagraph)
        {
            //man soll hier auch in Tabellen landen duerfen
            //if(CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType)
            {
                //BOOL bRemain = sal_False;
                //pUnoCrsr->SetRemainInSection( bRemain );
                pUnoCrsr->SetRemainInSection( sal_False );
                //was mache ich, wenn ich schon in einer Tabelle stehe?
                SwTableNode* pTblNode = pUnoCrsr->GetNode()->FindTableNode();
                pTblNode = lcl_FindTopLevelTable( /*pUnoCrsr,*/ pTblNode, pOwnTable );
                if(pTblNode  &&  &pTblNode->GetTable() != pOwnTable)
                {
                    // wir haben es mit einer fremden Tabelle zu tun - also ans Ende
                    pUnoCrsr->GetPoint()->nNode = pTblNode->EndOfSectionIndex();
                    if(!pUnoCrsr->Move(fnMoveForward, fnGoNode))
                        return aRef;
                    else
                        bInTable = sal_True;

                }
            }
        }

        // the cursor must remain in the current section or a subsection
        // before AND after the movement...
        if( lcl_CursorIsInSection( pUnoCrsr, pOwnStartNode ) &&
            (bFirstParagraph || bInTable ||
            (pUnoCrsr->MovePara(fnParaNext, fnParaStart) &&
                lcl_CursorIsInSection( pUnoCrsr, pOwnStartNode ) ) ) )
        {
            SwPosition* pStart = pUnoCrsr->Start();
            sal_Int32 nFirstContent = bFirstParagraph ? nFirstParaStart : -1;
            sal_Int32 nLastContent = nEndIndex ==  pStart->nNode.GetIndex() ? nLastParaEnd : -1;
            //steht man nun in einer Tabelle, oder in einem einfachen Absatz?

            SwTableNode* pTblNode = pUnoCrsr->GetNode()->FindTableNode();
            pTblNode = lcl_FindTopLevelTable( /*pUnoCrsr,*/ pTblNode, pOwnTable );
            if(/*CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType && */
                pTblNode  &&  &pTblNode->GetTable() != pOwnTable)
            {
                // wir haben es mit einer fremden Tabelle zu tun
                SwFrmFmt* pTableFmt = (SwFrmFmt*)pTblNode->GetTable().GetFrmFmt();
                XTextTable* pTable = SwXTextTables::GetObject( *pTableFmt );
                aRef =  (XTextContent*)(SwXTextTable*)pTable;
            }
            else
            {
                aRef = new SwXParagraph(static_cast<SwXText*>(pText),
                    pStart->nNode.GetNode().GetTxtNode(),
                    nFirstContent, nLastContent);
            }
        }
    }
    else
        throw uno::RuntimeException();

    return aRef;
}
/*-- 10.12.98 11:52:14---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXParagraphEnumeration::nextElement(void)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextContent >  aRef;

    if (bFirstParagraph)
    {
        xNextPara = NextElement_Impl();
        bFirstParagraph = sal_False;
    }
    aRef = xNextPara;
    if (!aRef.is())
        throw container::NoSuchElementException();
    xNextPara = NextElement_Impl();

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

const uno::Sequence< sal_Int8 > & SwXTextRange::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}

//XUnoTunnel
sal_Int64 SAL_CALL SwXTextRange::getSomething(
    const uno::Sequence< sal_Int8 >& rId )
        throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
        {
            return sal::static_int_cast< sal_Int64 >( reinterpret_cast <sal_IntPtr >(this) );
        }
    return 0;
}

OUString SwXTextRange::getImplementationName(void) throw( RuntimeException )
{
    return OUString::createFromAscii("SwXTextRange");
}

BOOL SwXTextRange::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    String sServiceName(rServiceName);
    return sServiceName.EqualsAscii("com.sun.star.text.TextRange") ||
        sServiceName.EqualsAscii("com.sun.star.style.CharacterProperties")||
        sServiceName.EqualsAscii("com.sun.star.style.CharacterPropertiesAsian")||
        sServiceName.EqualsAscii("com.sun.star.style.CharacterPropertiesComplex")||
        sServiceName.EqualsAscii("com.sun.star.style.ParagraphProperties") ||
        sServiceName.EqualsAscii("com.sun.star.style.ParagraphPropertiesAsian") ||
        sServiceName.EqualsAscii("com.sun.star.style.ParagraphPropertiesComplex");
}

Sequence< OUString > SwXTextRange::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(7);
    aRet[0] = OUString::createFromAscii("com.sun.star.text.TextRange");
    aRet[1] = OUString::createFromAscii("com.sun.star.style.CharacterProperties");
    aRet[2] = OUString::createFromAscii("com.sun.star.style.CharacterPropertiesAsian");
    aRet[3] = OUString::createFromAscii("com.sun.star.style.CharacterPropertiesComplex");
    aRet[4] = OUString::createFromAscii("com.sun.star.style.ParagraphProperties");
    aRet[5] = OUString::createFromAscii("com.sun.star.style.ParagraphPropertiesAsian");
    aRet[6] = OUString::createFromAscii("com.sun.star.style.ParagraphPropertiesComplex");
    return aRet;
}

SwXTextRange::SwXTextRange(SwPaM& rPam,
        const uno::Reference< XText > & rxParent, enum RangePosition eRange) :
    eRangePosition(eRange),
    pDoc(rPam.GetDoc()),
    pBox(0),
    pBoxStartNode(0),
    aObjectDepend(this, 0),
    m_pPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    xParentText(rxParent),
    pMark(NULL)
{
    _CreateNewBookmark(rPam);
}

SwXTextRange::SwXTextRange(SwFrmFmt& rTblFmt) :
    eRangePosition(RANGE_IS_TABLE),
    pDoc(rTblFmt.GetDoc()),
    pBox(0),
    pBoxStartNode(0),
    aObjectDepend(this, &rTblFmt),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR)),
    pMark(NULL)
{
}

SwXTextRange::~SwXTextRange()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    ::sw::mark::IMark const * const pBkmk = GetBookmark();
    if(pBkmk)
        pDoc->getIDocumentMarkAccess()->deleteMark(pBkmk);
}

void SwXTextRange::_CreateNewBookmark(SwPaM& rPam)
{
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();

    ::sw::mark::IMark const * const pBkmk = GetBookmark();
    if(pBkmk)
        pMarkAccess->deleteMark(pBkmk);
    pMark = pMarkAccess->makeMark(rPam, ::rtl::OUString(), IDocumentMarkAccess::UNO_BOOKMARK);
    pMark->Add(this);
}

void SwXTextRange::DeleteAndInsert(const String& rText)
    throw(uno::RuntimeException)
{
    ::sw::mark::IMark const * const pBkmk = GetBookmark();
    if(pBkmk)
    {
        const SwPosition& rPoint = pBkmk->GetMarkStart();
        SwCursor aNewCrsr(rPoint, 0, false);
        if(pBkmk->IsExpanded())
        {
            aNewCrsr.SetMark();
            const SwPosition& rMark = pBkmk->GetMarkEnd();
            *aNewCrsr.GetMark() = rMark;
        }
        UnoActionContext aAction(pDoc);
        pDoc->StartUndo(UNDO_INSERT, NULL);
        if(aNewCrsr.HasMark())
            pDoc->DeleteAndJoin(aNewCrsr);

        if(rText.Len())
        {
            SwUnoCursorHelper::DocInsertStringSplitCR(*pDoc, aNewCrsr, rText);

            SwXTextCursor::SelectPam(aNewCrsr, sal_True);
            aNewCrsr.Left(rText.Len(), CRSR_SKIP_CHARS, FALSE, FALSE);
        }
        _CreateNewBookmark(aNewCrsr);
        pDoc->EndUndo(UNDO_INSERT, NULL);
    }
}

uno::Reference< XText >  SwXTextRange::getText(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!xParentText.is())
    {
        if (eRangePosition == RANGE_IS_TABLE &&
            aObjectDepend.GetRegisteredIn() )
        {
            SwFrmFmt* pTblFmt = (SwFrmFmt*)aObjectDepend.GetRegisteredIn();
            SwTable* pTable = SwTable::FindTable( pTblFmt );
            SwTableNode* pTblNode = pTable->GetTableNode();
            SwPosition aPosition( *pTblNode );
            uno::Reference< XTextRange >  xRange =
                SwXTextRange::CreateTextRangeFromPosition(pDoc, aPosition, 0);
            xParentText = xRange->getText();
        }
        else
        {
            OSL_ENSURE(false, "SwXTextRange::getText: no text");
        }
    }
    return xParentText;
}

uno::Reference< XTextRange >  SwXTextRange::getStart(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  xRet;

    ::sw::mark::IMark const * const pBkmk = GetBookmark();
    if(!xParentText.is())
        getText();
    if(pBkmk)
    {
        SwPaM aPam(pBkmk->GetMarkStart());
        xRet = new SwXTextRange(aPam, xParentText);
    }
    else if(eRangePosition == RANGE_IS_TABLE)
    {
        //start and end are this, if its a table
        xRet = this;
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

uno::Reference< XTextRange >  SwXTextRange::getEnd(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  xRet;
    ::sw::mark::IMark const * const pBkmk = GetBookmark();
    if(!xParentText.is())
        getText();
    if(pBkmk)
    {
        SwPaM aPam(pBkmk->GetMarkEnd());
        xRet = new SwXTextRange(aPam, xParentText);
    }
    else if(eRangePosition == RANGE_IS_TABLE)
    {
        //start and end are this, if its a table
        xRet = this;
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

OUString SwXTextRange::getString(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    OUString sRet;
    ::sw::mark::IMark const * const pBkmk = GetBookmark();
    // for tables there is no bookmark, thus also no text
    // one could export the table as ASCII here maybe?
    if(pBkmk && pBkmk->IsExpanded())
    {
        const SwPosition& rPoint = pBkmk->GetMarkPos();
        const SwPosition& rMark = pBkmk->GetOtherMarkPos();
        SwPaM aCrsr(rMark, rPoint);
        SwXTextCursor::getTextFromPam(aCrsr, sRet);
    }
    return sRet;
}

void SwXTextRange::setString(const OUString& aString)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(RANGE_IS_TABLE == eRangePosition)
    {
        //setString in Tabellen kann nicht erlaubt werden
        throw uno::RuntimeException();
    }
    else
        DeleteAndInsert(aString);
}

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
    if(!GetRegisteredIn())
        pMark = NULL;
}

sal_Bool SwXTextRange::GetPositions(SwPaM& rToFill) const
{
    sal_Bool bRet = sal_False;
    ::sw::mark::IMark const * const pBkmk = GetBookmark();
    if(pBkmk)
    {
        *rToFill.GetPoint() = pBkmk->GetMarkPos();
        if(pBkmk->IsExpanded())
        {
            rToFill.SetMark();
            *rToFill.GetMark() = pBkmk->GetOtherMarkPos();
        }
        else
            rToFill.DeleteMark();
        bRet = sal_True;
    }
    return bRet;
}

sal_Bool SwXTextRange::XTextRangeToSwPaM( SwUnoInternalPaM& rToFill,
                            const uno::Reference< XTextRange > & xTextRange)
{
    sal_Bool bRet = sal_False;

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    SwXTextPortion* pPortion = 0;
    SwXText* pText = 0;
    SwXParagraph* pPara = 0;
    if(xRangeTunnel.is())
    {
        pRange  = reinterpret_cast< SwXTextRange * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
        pCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
        pPortion = reinterpret_cast< SwXTextPortion * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextPortion::getUnoTunnelId()) ));
        pText   = reinterpret_cast< SwXText * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXText::getUnoTunnelId()) ));
        pPara   = reinterpret_cast< SwXParagraph * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXParagraph::getUnoTunnelId()) ));
    }

    //if it's a text cursor then create a temporary cursor there and re-use the pCursor variable
    uno::Reference< XTextCursor > xTextCursor;
    if(pText)
    {
        xTextCursor = pText->createCursor();
        xTextCursor->gotoEnd(sal_True);
        uno::Reference<XUnoTunnel> xCrsrTunnel( xTextCursor, UNO_QUERY);
        pCursor = reinterpret_cast<  OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xCrsrTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
    }
    if(pRange && pRange->GetDoc() == rToFill.GetDoc())
    {
        bRet = pRange->GetPositions(rToFill);
    }
    else
    {
        if (pPara)
        {
            const SwTxtNode * pTxtNode( pPara->GetTxtNode() );

            if (pTxtNode) {
                bRet = sal_True;
                *rToFill.GetPoint() = SwPosition( *pTxtNode );
                // set selection to the whole paragraph
                rToFill.SetMark();
                rToFill.GetMark()->nContent = pTxtNode->GetTxt().Len();
            }
        }
        else
        {
            const SwPaM* pUnoCrsr = 0;
            SwDoc* pDoc = pCursor ? pCursor->GetDoc() : NULL;

            if ( !pDoc )
                pDoc = pPortion ? pPortion->GetCrsr()->GetDoc() : NULL;
            pUnoCrsr = pCursor ? pCursor->GetPaM() : pPortion ? pPortion->GetCrsr() : 0;
            if (pUnoCrsr && pDoc == rToFill.GetDoc())
            {
                DBG_ASSERT((SwPaM*)pUnoCrsr->GetNext() == pUnoCrsr, "was machen wir mit Ringen?" );
                bRet = sal_True;
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

sal_Bool lcl_IsStartNodeInFormat(sal_Bool bHeader, SwStartNode* pSttNode,
    const SwFrmFmt* pFrmFmt, SwFrmFmt*& rpFormat)
{
    sal_Bool bRet = sal_False;
    const SfxItemSet& rSet = pFrmFmt->GetAttrSet();
    const SfxPoolItem* pItem;
    SwFrmFmt* pHeadFootFmt;
    if(SFX_ITEM_SET == rSet.GetItemState( static_cast< USHORT >(bHeader ? RES_HEADER : RES_FOOTER), sal_True, &pItem) &&
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

uno::Reference< XTextRange >  SwXTextRange::CreateTextRangeFromPosition(
    SwDoc* pDoc, const SwPosition& rPos, const SwPosition* pMark)
{
    uno::Reference< XText > xParentText( CreateParentXText(pDoc, rPos) );
    std::auto_ptr<SwUnoCrsr> pNewCrsr( pDoc->CreateUnoCrsr(rPos, sal_False) );
    if(pMark)
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pMark;
    }
    bool isCell( dynamic_cast<SwXCell*>(xParentText.get()) );
    uno::Reference< XTextRange > xRet(
        new SwXTextRange(*pNewCrsr, xParentText,
            isCell ? RANGE_IN_CELL : RANGE_IN_TEXT) );
    return xRet;

}

uno::Reference< XText >  SwXTextRange::CreateParentXText(SwDoc* pDoc,
        const SwPosition& rPos)
{
    uno::Reference< XText > xParentText;
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
            const SwTableNode* pTblNode = pSttNode->FindTableNode();
            SwFrmFmt* pTableFmt = (SwFrmFmt*)pTblNode->GetTable().GetFrmFmt();
            SwTableBox* pBox = pSttNode->GetTblBox();

            xParentText = pBox
                ? SwXCell::CreateXCell( pTableFmt, pBox )
                : new SwXCell( pTableFmt, *pSttNode );
        }
        break;
        case SwFlyStartNode:
        {
            SwFrmFmt* pFmt = pSttNode->GetFlyFmt();
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
            sal_Bool bHeader = SwHeaderStartNode == eType;
            sal_uInt16 nPDescCount = pDoc->GetPageDescCnt();
            for(sal_uInt16 i = 0; i < nPDescCount; i++)
            {
                const SwPageDesc& rDesc = const_cast<const SwDoc*>(pDoc)
                    ->GetPageDesc( i );
                const SwFrmFmt* pFrmFmtMaster = &rDesc.GetMaster();
                const SwFrmFmt* pFrmFmtLeft = &rDesc.GetLeft();

                SwFrmFmt* pHeadFootFmt = 0;
                if(!lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrmFmtMaster, pHeadFootFmt))
                    lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrmFmtLeft, pHeadFootFmt);

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
            uno::Reference< XFootnote >  xRef;
            for( n = 0; n < nFtnCnt; ++n )
            {
                const SwTxtFtn* pTxtFtn = pDoc->GetFtnIdxs()[ n ];
                const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
                pTxtFtn = rFtn.GetTxtFtn();
#if OSL_DEBUG_LEVEL > 1
                const SwStartNode* pTmpSttNode = pTxtFtn->GetStartNode()->GetNode().
                                FindSttNodeByType(SwFootnoteStartNode);
                (void)pTmpSttNode;
#endif

                if (pSttNode == pTxtFtn->GetStartNode()->GetNode().
                                    FindSttNodeByType(SwFootnoteStartNode))
                {
                    xParentText = ((SwUnoCallBack*)pDoc->GetUnoCallBack())->
                                                            GetFootnote(rFtn);
                    if (!xParentText.is())
                    {
                        xParentText = new SwXFootnote(pDoc, rFtn);
                    }
                    break;
                }
            }
        }
        break;
        default:
        {
            // then it is the body text
            uno::Reference<frame::XModel> xModel =
                pDoc->GetDocShell()->GetBaseModel();
            uno::Reference< XTextDocument > xDoc(
                xModel, uno::UNO_QUERY);
            xParentText = xDoc->getText();
        }
    }
    OSL_ENSURE(xParentText.is(), "no parent text?");
    return xParentText;
}

uno::Reference< XEnumeration >  SAL_CALL SwXTextRange::createContentEnumeration(
    const OUString& rServiceName)
        throw(RuntimeException)
{
    ::sw::mark::IMark const * const pBkmk = GetBookmark();
    if( !pBkmk || COMPARE_EQUAL != rServiceName.compareToAscii("com.sun.star.text.TextContent") )
        throw RuntimeException();

    const SwPosition& rPoint = pBkmk->GetMarkPos();
    SwUnoCrsr* pNewCrsr = pDoc->CreateUnoCrsr(rPoint, FALSE);
    if(pBkmk->IsExpanded() && pBkmk->GetOtherMarkPos() != rPoint)
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = pBkmk->GetOtherMarkPos();
    }
    uno::Reference< XEnumeration > xRet = new SwXParaFrameEnumeration(*pNewCrsr, PARAFRAME_PORTION_TEXTRANGE);
    delete pNewCrsr;
    return xRet;
}

uno::Reference< XEnumeration > SwXTextRange::createEnumeration(void) throw( RuntimeException )
{
    ::sw::mark::IMark const * const pBkmk = GetBookmark();
    if(!pBkmk) throw RuntimeException();
    const SwPosition& rPoint = pBkmk->GetMarkPos();
    SwUnoCrsr* pNewCrsr = pDoc->CreateUnoCrsr(rPoint, FALSE);
    if(pBkmk->IsExpanded() && pBkmk->GetOtherMarkPos() != rPoint)
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = pBkmk->GetOtherMarkPos();
    }
    uno::Reference<XUnoTunnel> xTunnel(xParentText, UNO_QUERY);
    SwXText* pParentText = 0;
    if(xTunnel.is())
    {
        pParentText = reinterpret_cast< SwXText * >(
                sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething(SwXText::getUnoTunnelId()) ));
    }
    DBG_ASSERT(pParentText, "parent is not a SwXText");
    CursorType eSetType = RANGE_IN_CELL == eRangePosition ? CURSOR_SELECTION_IN_TABLE : CURSOR_SELECTION;
    uno::Reference< XEnumeration > xRet =
        new SwXParagraphEnumeration(pParentText, pNewCrsr, eSetType);
    return xRet;
}

uno::Type  SwXTextRange::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}

sal_Bool SwXTextRange::hasElements(void) throw( RuntimeException )
{
    return sal_True;
}

Sequence< OUString > SAL_CALL SwXTextRange::getAvailableServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString::createFromAscii("com.sun.star.text.TextContent");
    return aRet;
}

uno::Reference< XPropertySetInfo > SAL_CALL SwXTextRange::getPropertySetInfo(  ) throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Reference< XPropertySetInfo > xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

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
    SwXTextCursor::SetPropertyValue(aPaM, *m_pPropSet, rPropertyName, rValue);
}

Any SAL_CALL SwXTextRange::getPropertyValue( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    return SwXTextCursor::GetPropertyValue(aPaM, *m_pPropSet, rPropertyName);
}

void SAL_CALL SwXTextRange::addPropertyChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_WARNING("not implemented");
}

void SAL_CALL SwXTextRange::removePropertyChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_WARNING("not implemented");
}

void SAL_CALL SwXTextRange::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_WARNING("not implemented");
}

void SAL_CALL SwXTextRange::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_WARNING("not implemented");
}

PropertyState SAL_CALL SwXTextRange::getPropertyState( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    return SwXTextCursor::GetPropertyState(aPaM, *m_pPropSet, rPropertyName);
}

Sequence< PropertyState > SAL_CALL SwXTextRange::getPropertyStates(
    const Sequence< OUString >& rPropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    return SwXTextCursor::GetPropertyStates(aPaM, *m_pPropSet, rPropertyName);
}

void SAL_CALL SwXTextRange::setPropertyToDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    SwXTextCursor::SetPropertyToDefault(aPaM, *m_pPropSet, rPropertyName);
}

Any SAL_CALL SwXTextRange::getPropertyDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    return SwXTextCursor::GetPropertyDefault(aPaM, *m_pPropSet, rPropertyName);
}

void SwXTextRange::makeRedline(
    const ::rtl::OUString& rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties )
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    SwUnoCursorHelper::makeRedline( aPaM, rRedlineType, rRedlineProperties );
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
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast <sal_IntPtr >(this) );
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
    vos::OGuard aGuard(Application::GetSolarMutex());
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
    if(pArr && 0 <= nIndex && nIndex < pArr->Count())
    {
        XTextRangeRefPtr pRef = pArr->GetObject( USHORT( nIndex ));
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

            uno::Reference< XTextRange >* pPtr =
                new uno::Reference<XTextRange>( SwXTextRange::CreateTextRangeFromPosition(PUNOPAM->GetDoc(),
                        *PUNOPAM->GetPoint(), PUNOPAM->GetMark()));
//              new uno::Reference<XTextRange>( SwXTextRange::createTextRangeFromPaM(*PUNOPAM, xParentText));
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
        if( !SwUnoCursorHelper::DocInsertStringSplitCR( *pDoc, rCrsr, aText ) )
        {
            DBG_ASSERT( sal_False, "DocInsertStringSplitCR" );
        }
        SwXTextCursor::SelectPam(rCrsr, sal_True);
        rCrsr.Left(nTxtLen, CRSR_SKIP_CHARS, FALSE, FALSE);
    }
    pDoc->EndUndo(UNDO_INSERT, NULL);
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
SwXParaFrameEnumeration::SwXParaFrameEnumeration(const SwPaM& rPaM,
                                                sal_uInt8 nParaFrameMode,
                                                SwFrmFmt* pFmt)
{
    SwDoc* pDoc = rPaM.GetDoc();
    SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(*rPaM.GetPoint(), sal_False);
    if (rPaM.HasMark())
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *rPaM.GetMark();
    }
    pUnoCrsr->Add(this);

    if (PARAFRAME_PORTION_PARAGRAPH == nParaFrameMode)
    {
        ::CollectFrameAtNode( *this, rPaM.GetPoint()->nNode,
                                aFrameArr, FALSE );
    }
    else if (pFmt)
    {
        //jetzt einen SwDepend anlegen und in das Array einfuegen
        SwDepend* pNewDepend = new SwDepend(this, pFmt);
        aFrameArr.C40_INSERT(SwDepend, pNewDepend, aFrameArr.Count());
    }
    else if((PARAFRAME_PORTION_CHAR == nParaFrameMode) ||
            (PARAFRAME_PORTION_TEXTRANGE == nParaFrameMode))
    {
        if(PARAFRAME_PORTION_TEXTRANGE == nParaFrameMode)
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
                    pUnoCrsr->Right(1, CRSR_SKIP_CHARS, FALSE, FALSE);
                }
                while(*pUnoCrsr->GetPoint() < *pUnoCrsr->GetMark());
            }
        }
        FillFrame(*pUnoCrsr);
    }
}
/*-- 23.03.99 13:22:30---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXParaFrameEnumeration::~SwXParaFrameEnumeration()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
            xNextObject = uno::Reference< XTextContent >(pSdr->getUnoShape(), uno::UNO_QUERY);
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
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_REFCOUNT( SwXTextCursor,SwXTextCursor_Base )

uno::Any SAL_CALL SwXTextCursor::queryInterface( const uno::Type& _rType ) throw (uno::RuntimeException)
{
    if( _rType == ::getCppuType( (uno::Reference<XUnoTunnel>*)0)  )
        return OTextCursorHelper::queryInterface( _rType );
    else
        return SwXTextCursor_Base::queryInterface( _rType );
}


