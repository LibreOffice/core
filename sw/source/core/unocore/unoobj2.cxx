/*************************************************************************
 *
 *  $RCSfile: unoobj2.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: os $ $Date: 2001-04-05 11:40:01 $
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
#ifndef _UNOCRSRHELPER_HXX
#include <unocrsrhelper.hxx>
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
#define _SVSTDARR_XUB_STRLEN
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
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif

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

void CollectFrameAtNode( SwClient& rClnt, const SwNodeIndex& rIdx,
                            SwDependArr& rFrameArr, BOOL bSort )
{
    // bSortFlag says: TRUE - search AutoContent Flys,
    //                 else search AtContent Flys

    // alle Rahmen, Grafiken und OLEs suchen, die an diesem Absatz
    // gebunden sind
    SvXub_StrLens aSortArr( 8, 8 );
    SwDoc* pDoc = rIdx.GetNode().GetDoc();

    USHORT nChkType = bSort ? FLY_AUTO_CNTNT : FLY_AT_CNTNT;
    const SwCntntFrm* pCFrm;
    const SwCntntNode* pCNd;
    if( pDoc->GetRootFrm() &&
        0 != (pCNd = rIdx.GetNode().GetCntntNode()) &&
        0 != (pCFrm = pCNd->GetFrm()) )
    {
        const SwDrawObjs *pObjs = pCFrm->GetDrawObjs();
        if( pObjs )
            for( USHORT i = 0; i < pObjs->Count(); ++i )
            {
                const SdrObject *pO = (*pObjs)[ i ];
                const SwFlyFrm *pFly;
                SwFrmFmt* pFmt;

                if( pO->IsWriterFlyFrame()
                    ? ( (pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm())
                                ->IsFlyAtCntFrm() &&
                        (bSort ? pFly->IsAutoPos() : !pFly->IsAutoPos() ) &&
                        0 != ( pFmt = (SwFrmFmt*)pFly->GetFmt()) )
                    : ( 0 != (pFmt=((SwDrawContact*)GetUserCall(pO))->GetFmt())
                        && pFmt->GetAnchor().GetAnchorId() == nChkType )
                    )
                {
                    //jetzt einen SwDepend anlegen und in das Array einfuegen
                    SwDepend* pNewDepend = new SwDepend( &rClnt, pFmt );

                    USHORT nInsPos = rFrameArr.Count();
                    if( bSort )
                    {
                        xub_StrLen nInsertIndex = pFmt->GetAnchor().
                                    GetCntntAnchor()->nContent.GetIndex();

                        USHORT nEnd = nInsPos;
                        for( nInsPos = 0; nInsPos < nEnd; ++nInsPos )
                            if( aSortArr[ nInsPos ] > nInsertIndex )
                                break;
                        aSortArr.Insert( nInsertIndex, nInsPos );
                    }
                    rFrameArr.C40_INSERT( SwDepend, pNewDepend, nInsPos );
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
                USHORT nInsPos = rFrameArr.Count();
                if( bSort )
                {
                    xub_StrLen nInsertIndex = pAnchorPos->nContent.GetIndex();

                    USHORT nEnd = nInsPos;
                    for( nInsPos = 0; nInsPos < nEnd; ++nInsPos )
                        if( aSortArr[ nInsPos ] > nInsertIndex )
                            break;
                    aSortArr.Insert( nInsertIndex, nInsPos );
                }
                rFrameArr.C40_INSERT( SwDepend, pNewDepend, nInsPos );
            }
        }
    }
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
        SwUnoCursorHelper::InsertFile(pUnoCrsr, rURL, sFilterName, sFilterOption, sPassword);
    }
    else
        throw uno::RuntimeException();
}
/* -----------------------------15.12.00 14:01--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue > SwXTextCursor::createSortDescriptor(sal_Bool bFromTable)
{
    uno::Sequence< beans::PropertyValue > aRet(12);
    beans::PropertyValue* pArray = aRet.getArray();

    uno::Any aVal;
    sal_Bool bFalse = sal_False;
    sal_Bool bTrue = sal_True;
    aVal.setValue( &bFromTable, ::getCppuBooleanType());
    pArray[0] = beans::PropertyValue(C2U("IsSortInTable"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    String sSpace(String::CreateFromAscii(" "));
    sal_Unicode uSpace = sSpace.GetChar(0);

    aVal <<= uSpace;
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
    rSortOpt.eDirection = SRT_COLUMNS;

    rSortOpt.aKeys;
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
                rSortOpt.bTable = *(sal_Bool*)aValue.getValue();
            else
                bRet = sal_False;
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("Delimiter"))
        {
            sal_Unicode uChar;
            if( aValue >>= uChar )
                rSortOpt.cDeli = uChar;
            else
                bRet = sal_False;
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("SortColumns"))
        {
            if ( aValue.getValueType() == ::getBooleanCppuType() )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                rSortOpt.eDirection = bTemp ? SRT_COLUMNS : SRT_ROWS;
            }
            else
                bRet = sal_False;
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
                aKeys[nIndex]->nColumnId = nCol;
            else
                bRet = sal_False;
        }
        else if(0 == rPropName.search(C2U("IsSortNumeric")) &&
            rPropName.getLength() == 14 &&
            (rPropName.getStr()[13] >= '0' && rPropName.getStr()[13] <= '9'))
        {
            sal_uInt16 nIndex = rPropName.getStr()[13];
            nIndex = nIndex - '0';
            if ( aValue.getValueType() == ::getBooleanCppuType() && nIndex < 3 )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                aKeys[nIndex]->eSortKeyType = bTemp ? SRT_NUMERIC : SRT_APLHANUM;
            }
            else
                bRet = sal_False;
        }
        else if(0 == rPropName.search(C2U("IsSortAscending")) && rPropName.getLength() == 16 &&
            lcl_IsNumeric(String(rPropName[(sal_uInt16)15])))
        {
            sal_uInt16 nIndex = rPropName.getStr()[16];
            if ( aValue.getValueType() == ::getBooleanCppuType() && nIndex < 3 )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                aKeys[nIndex]->eSortOrder = bTemp ? SRT_ASCENDING : SRT_DESCENDING;
            }
            else
                bRet = sal_False;
        }
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
/* -----------------------------07.03.01 14:53--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XEnumeration >  SwXTextCursor::createEnumeration(void) throw( RuntimeException )
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( !pUnoCrsr  )
        throw RuntimeException();
    Reference<XUnoTunnel> xTunnel(xParentText, UNO_QUERY);
    SwXText* pParentText = 0;
    if(xTunnel.is())
    {
        pParentText = (SwXText*)xTunnel->getSomething(SwXText::getUnoTunnelId());
    }
    DBG_ASSERT(pParentText, "parent is not a SwXText")

    SwUnoCrsr* pNewCrsr = pUnoCrsr->GetDoc()->CreateUnoCrsr(*pUnoCrsr->GetPoint());
    if(pUnoCrsr->HasMark())
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pUnoCrsr->GetMark();
    }
    CursorType eSetType = eType == CURSOR_TBLTEXT ? CURSOR_SELECTION_IN_TABLE : CURSOR_SELECTION;
    Reference< XEnumeration > xRet = new SwXParagraphEnumeration(pParentText, *pNewCrsr, eSetType);

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
        eCursorType(eType),
        nEndIndex(rPos.nNode.GetIndex())
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
        eCursorType(eType),
        nEndIndex(pCrsr->End()->nNode.GetIndex())
{
    if(CURSOR_SELECTION == eCursorType || CURSOR_SELECTION_IN_TABLE == eCursorType)
    {
        if(*pCrsr->GetPoint() > *pCrsr->GetMark())
            pCrsr->Exchange();
        if(pCrsr->HasMark())
            pCrsr->DeleteMark();
    }
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
            SwPosition* pStart = pUnoCrsr->Start();
            SwUnoCrsr* pNewCrsr = pUnoCrsr->GetDoc()->CreateUnoCrsr(*pStart, sal_False);
            //man soll hier auch in Tabellen landen duerfen
            if(CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType)
                pNewCrsr->SetRemainInSection( sal_False );

            //was mache ich, wenn ich schon in einer Tabelle stehe?
            SwTableNode* pTblNode = pNewCrsr->GetNode()->FindTableNode();
            if((CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType) && pTblNode)
            {
                pNewCrsr->GetPoint()->nNode = pTblNode->EndOfSectionIndex();
                bRet = pNewCrsr->Move(fnMoveForward, fnGoNode);
            }
            else
                bRet = pNewCrsr->MovePara(fnParaNext, fnParaStart);
            if((CURSOR_SELECTION == eCursorType || CURSOR_SELECTION_IN_TABLE == eCursorType)
                        && nEndIndex < pNewCrsr->Start()->nNode.GetIndex())
                bRet = FALSE;
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
            if(CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType)
            {
                pUnoCrsr->SetRemainInSection( sal_False );
                //was mache ich, wenn ich schon in einer Tabelle stehe?
                SwTableNode* pTblNode = pUnoCrsr->GetNode()->FindTableNode();
                if(pTblNode)
                {
                    // wir haben es mit einer Tabelle zu tun - also ans Ende
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
            SwPosition* pStart = pUnoCrsr->Start();
            //steht man nun in einer Tabelle, oder in einem einfachen Absatz?

            SwTableNode* pTblNode = pUnoCrsr->GetNode()->FindTableNode();
            if(CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType && pTblNode)
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

        UnoActionContext aAction(aNewCrsr.GetDoc());
        pDoc->StartUndo(UNDO_INSERT);
        if(aNewCrsr.HasMark())
            pDoc->DeleteAndJoin(aNewCrsr);

        if(rText.Len())
        {
            if( !pDoc->Insert(aNewCrsr, rText) )
            {
                ASSERT( sal_False, "Doc->Insert(Str) failed." )
            }
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
        else if(eRangePosition == RANGE_IS_TABLE &&
            aObjectDepend.GetRegisteredIn() )
        {
            SwFrmFmt* pTblFmt = (SwFrmFmt*)aObjectDepend.GetRegisteredIn();
            SwDoc* pDoc = pTblFmt->GetDoc();
            SwTable* pTable = SwTable::FindTable( pTblFmt );
            SwTableNode* pTblNode = pTable->GetTableNode();
            SwPosition aPosition( *pTblNode );
            Reference< XTextRange >  xRange = SwXTextRange::CreateTextRangeFromPosition(pDoc,
                        aPosition, 0);
            xParentText = xRange->getText();
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
    if(!xParentText.is())
        getText();
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
    if(!xParentText.is())
        getText();
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
/* -----------------24.02.99 14:18-------------------
 * Der StartNode muss in einem existierenden Header/Footen liegen
 * --------------------------------------------------*/
sal_Bool lcl_IsStartNodeInFormat(sal_Bool bHeader, SwStartNode* pSttNode,
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
uno::Reference< XTextRange >  SwXTextRange::CreateTextRangeFromPosition(SwDoc* pDoc,
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
                    xParentText = ((SwUnoCallBack*)pDoc->GetUnoCallBack())->
                                                            GetFootnote(rFtn);
                    if(!xParentText.is())
                        xParentText = new SwXFootnote(pDoc, rFtn);
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
    delete pNewCrsr;
    return aRet;
}
/*Reference< XTextRange > SwXTextRange::createTextRangeFromPaM(
    SwPaM& rPaM, Reference<XText> xParentText)
{
    Reference< XTextRange > xRet;
    // in welcher Umgebung steht denn der PaM?
    SwStartNode* pSttNode = rPaM.GetNode()->FindStartNode();
    SwStartNodeType eType = pSttNode->GetStartNodeType();
    uno::Reference< XText >  xCurParentRef;
    switch(eType)
    {
        case SwTableBoxStartNode:
        {
            SwTableBox* pBox = pSttNode->GetTblBox();
            xRet = new SwXTextRange(*pBox->GetFrmFmt(), *pBox, rPaM);
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
                xRet = new SwXTextRange(*pFlyFmt, rPaM);
        }
        break;
        case SwFootnoteStartNode:
        {
            const SwFtnIdxs& rIdxs = rPaM.GetDoc()->GetFtnIdxs();
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
                    xCurParentRef = new SwXFootnote(rPaM.GetDoc(), rFtn);
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
            String sPageStyleName = SwUnoCursorHelper::GetCurPageStyle(rPaM);
            uno::Reference< style::XStyleFamiliesSupplier >  xStyleSupp(
                rPaM.GetDoc()->GetDocShell()->GetBaseModel(), uno::UNO_QUERY);

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
                    rPaM.GetDoc()->GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
                xParentText = xDoc->getText();
            }
            xCurParentRef = xParentText;
        }
    }
    if(xCurParentRef.is() && !xRet.is())
        xRet = new SwXTextRange(rPaM, xCurParentRef);
    DBG_ASSERT(xRet.is(), "SwXTextRange not created")
    return xRet;
}*/

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
/* -----------------------------07.03.01 14:55--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XEnumeration >  SwXTextRange::createEnumeration(void) throw( RuntimeException )
{
    SwBookmark* pBkm = GetBookmark();
    if( !pBkm  )
        throw RuntimeException();
    const SwPosition& rPoint = pBkm->GetPos();
    const SwPosition* pMark = pBkm->GetOtherPos();
    SwUnoCrsr* pNewCrsr = pDoc->CreateUnoCrsr(rPoint, FALSE);
    if(pMark && *pMark != rPoint)
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pMark;
    }
    Reference<XUnoTunnel> xTunnel(xParentText, UNO_QUERY);
    SwXText* pParentText = 0;
    if(xTunnel.is())
    {
        pParentText = (SwXText*)xTunnel->getSomething(SwXText::getUnoTunnelId());
    }
    DBG_ASSERT(pParentText, "parent is not a SwXText")
    CursorType eSetType = RANGE_IN_CELL == eRangePosition ? CURSOR_SELECTION_IN_TABLE : CURSOR_SELECTION;
    Reference< XEnumeration > xRet = new SwXParagraphEnumeration(pParentText, *pNewCrsr, eSetType);
    return xRet;
}
/* -----------------------------07.03.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
uno::Type  SwXTextRange::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}
/* -----------------------------07.03.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXTextRange::hasElements(void) throw( RuntimeException )
{
    return sal_True;
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

            Reference< XTextRange >* pPtr =
                new Reference<XTextRange>( SwXTextRange::CreateTextRangeFromPosition(PUNOPAM->GetDoc(),
                        *PUNOPAM->GetPoint(), PUNOPAM->GetMark()));
//              new Reference<XTextRange>( SwXTextRange::createTextRangeFromPaM(*PUNOPAM, xParentText));
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
        {
            ASSERT( sal_False, "Doc->Insert(Str) failed." )
        }
        SwXTextCursor::SelectPam(rUnoCrsr, sal_True);
        rUnoCrsr.Left(nTxtLen);
    }
    pDoc->EndUndo(UNDO_INSERT);
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
        ::CollectFrameAtNode( *this, rUnoCrsr.GetPoint()->nNode,
                                aFrameArr, FALSE );
    else if(pFmt)
    {
        //jetzt einen SwDepend anlegen und in das Array einfuegen
        SwDepend* pNewDepend = new SwDepend(this, pFmt);
        aFrameArr.C40_INSERT(SwDepend, pNewDepend, aFrameArr.Count());
    }
    else if((PARAFRAME_PORTION_CHAR == nParaFrameMode) ||
            (PARAFRAME_PORTION_TEXTRANGE == nParaFrameMode))
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
        if(PARAFRAME_PORTION_CHAR != nParaFrameMode && pUnoCrsr->HasMark())
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


