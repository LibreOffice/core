/*************************************************************************
 *
 *  $RCSfile: unocrsrhelper.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:17:35 $
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


#pragma hdrstop

#include <cmdid.h>
#ifndef _UNOCRSRHELPER_HXX
#include <unocrsrhelper.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef SW_UNOMID_HXX
#include <unomid.h>
#endif
#ifndef _UNOIDX_HXX
#include <unoidx.hxx>
#endif
#ifndef _UNOFIELD_HXX
#include <unofield.hxx>
#endif
#ifndef _UNOTBL_HXX
#include <unotbl.hxx>
#endif
#ifndef _UNOSETT_HXX
#include <unosett.hxx>
#endif
#ifndef _UNOCOLL_HXX
#include <unocoll.hxx>
#endif
#ifndef _UNOFRAME_HXX
#include <unoframe.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _DOCSTYLE_HXX //autogen
#include <docstyle.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _TXTRFMRK_HXX //autogen
#include <txtrfmrk.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _SHELLIO_HXX //autogen
#include <shellio.hxx>
#endif
#ifndef _EDIMP_HXX //autogen
#include <edimp.hxx>
#endif
#ifndef _SWUNDO_HXX //autogen
#include <swundo.hxx>
#endif
#ifndef _CNTFRM_HXX //autogen
#include <cntfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX //autogen
#include <pagefrm.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _FCHRFMT_HXX
#include <fchrfmt.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include <svx/flstitem.hxx>
#endif
#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#include <svtools/svstdarr.hxx>
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif

#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::rtl;


namespace SwUnoCursorHelper
{
/* -----------------16.09.98 12:27-------------------
 *  Lesen spezieller Properties am Cursor
 * --------------------------------------------------*/
sal_Bool getCrsrPropertyValue(const SfxItemPropertyMap* pMap
                                        , SwPaM& rPam
                                        , Any *pAny
                                        , PropertyState& eState
                                        , const SwTxtNode* pNode  )
{
    PropertyState eNewState = PropertyState_DIRECT_VALUE;
//    PropertyState_DEFAULT_VALUE
//    PropertyState_AMBIGUOUS_VALUE
    sal_Bool bDone = sal_True;
    switch(pMap->nWID)
    {
        case FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL:
            if( pAny )
            {
                SwFmtColl* pFmt = 0;
                if(pNode)
                    pFmt = pNode->GetFmtColl();
                else
                    pFmt = SwXTextCursor::GetCurTxtFmtColl(rPam, FALSE);
                sal_Int8 nRet = -1;
                if(pFmt && ((SwTxtFmtColl*)pFmt)->GetOutlineLevel() != NO_NUMBERING)
                    nRet = ((SwTxtFmtColl*)pFmt)->GetOutlineLevel();
                *pAny <<= nRet;
            }
        break;
        case FN_UNO_PARA_CONDITIONAL_STYLE_NAME:
        case FN_UNO_PARA_STYLE :
        {
            SwFmtColl* pFmt = 0;
            if(pNode)
                pFmt = FN_UNO_PARA_CONDITIONAL_STYLE_NAME == pMap->nWID
                            ? pNode->GetFmtColl() : &pNode->GetAnyFmtColl();
            else
                pFmt = SwXTextCursor::GetCurTxtFmtColl(rPam, FN_UNO_PARA_CONDITIONAL_STYLE_NAME == pMap->nWID);
            if(pFmt)
            {
                if( pAny )
                {
                    String sVal;
                    SwStyleNameMapper::FillProgName(pFmt->GetName(), sVal, GET_POOLID_TXTCOLL, sal_True );
                    *pAny <<= OUString(sVal);
                }
            }
            else
                eNewState = PropertyState_AMBIGUOUS_VALUE;
        }
        break;
        case FN_UNO_PAGE_STYLE :
        {
            String sVal;
            GetCurPageStyle(rPam, sVal);
            if( pAny )
                *pAny <<= OUString(sVal);
            if(!sVal.Len())
                eNewState = PropertyState_AMBIGUOUS_VALUE;
        }
        break;
        case FN_UNO_NUM_START_VALUE  :
            if( pAny )
            {
                sal_Int16 nValue = IsNodeNumStart(rPam, eNewState);
                *pAny <<= nValue;
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
                if( pAny )
                {
                    if(pMap->nWID == FN_UNO_NUM_LEVEL)
                        *pAny <<= (sal_Int16)(pTxtNd->GetNum()->GetRealLevel());
                    else if(pMap->nWID == FN_UNO_IS_NUMBER)
                    {
                        BOOL bIsNumber = pTxtNd->GetNum()->IsNum();
                        pAny->setValue(&bIsNumber, ::getBooleanCppuType());
                    }
                    else /*if(pMap->nWID == UNO_NAME_PARA_IS_NUMBERING_RESTART)*/
                    {
                        BOOL bIsRestart = pTxtNd->GetNum()->IsStart();
                        pAny->setValue(&bIsRestart, ::getBooleanCppuType());
                    }
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
            //PROPERTY_MAYBEVOID!
        }
        break;
        case FN_UNO_NUM_RULES  :
            if( pAny )
                getNumberingProperty(rPam, eNewState, pAny);
            else
            {
                if( !rPam.GetDoc()->GetCurrNumRule( *rPam.GetPoint() ) )
                    eNewState = PropertyState_DEFAULT_VALUE;
            }
            break;
        case FN_UNO_DOCUMENT_INDEX_MARK:
        {
            SwTxtAttr* pTxtAttr = rPam.GetNode()->GetTxtNode()->GetTxtAttr(
                                rPam.GetPoint()->nContent, RES_TXTATR_TOXMARK);
            if(pTxtAttr)
            {
                if( pAny )
                {
                    const SwTOXMark& rMark = pTxtAttr->GetTOXMark();
                    Reference< XDocumentIndexMark >  xRef = SwXDocumentIndexMark::GetObject(
                            (SwTOXType*)rMark.GetTOXType(), &rMark, rPam.GetDoc());
                    pAny->setValue(&xRef, ::getCppuType((Reference<XDocumentIndex>*)0));
                }
            }
            else
                //auch hier - nicht zu unterscheiden
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_DOCUMENT_INDEX:
        {
            const SwTOXBase* pBase = rPam.GetDoc()->GetCurTOX(
                                                    *rPam.Start() );
            if( pBase )
            {
                if( pAny )
                {
                    Reference< XDocumentIndex > aRef =
                        SwXDocumentIndexes::GetObject((SwTOXBaseSection*)pBase);
                    pAny->setValue(&aRef, ::getCppuType((Reference<XDocumentIndex>*)0));
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
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
                if( pAny )
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
                    Reference< XTextField >  xRet = pFld;
                    pAny->setValue(&xRet, ::getCppuType((Reference<XTextField>*)0));
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
/*              laesst sich nicht feststellen
*               case FN_UNO_BOOKMARK:
        {
            if()
            {
                Reference< XBookmark >  xBkm = SwXBookmarks::GetObject(rBkm);
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
                if( pAny )
                {
                    const SwTableNode* pTblNode = pSttNode->FindTableNode();
                    SwFrmFmt* pTableFmt = (SwFrmFmt*)pTblNode->GetTable().GetFrmFmt();
                    SwTable& rTable = ((SwTableNode*)pSttNode)->GetTable();
                    if(FN_UNO_TEXT_TABLE == pMap->nWID)
                    {
                        Reference< XTextTable >  xTable = SwXTextTables::GetObject(*pTableFmt);
                        pAny->setValue(&xTable, ::getCppuType((Reference<XTextTable>*)0));
                    }
                    else
                    {
                        SwTableBox* pBox = pSttNode->GetTblBox();
                        Reference< XCell >  xCell = SwXCell::CreateXCell(pTableFmt, pBox);
                        pAny->setValue(&xCell, ::getCppuType((Reference<XCell>*)0));
                    }
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_FRAME:
        {
            SwStartNode* pSttNode = rPam.GetNode()->FindStartNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();

            SwFrmFmt* pFmt;
            if(eType == SwFlyStartNode && 0 != (pFmt = pSttNode->GetFlyFmt()))
            {
                if( pAny )
                {
                    Reference< XTextFrame >  xFrm = (SwXTextFrame*) SwXFrames::GetObject(*pFmt, FLYCNTTYPE_FRM);
                    pAny->setValue(&xFrm, ::getCppuType((Reference<XTextFrame>*)0));
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_SECTION:
        {
            SwSection* pSect = rPam.GetDoc()->GetCurrSection(*rPam.GetPoint());
            if(pSect)
            {
                if( pAny )
                {
                    Reference< XTextSection >  xSect = SwXTextSections::GetObject( *pSect->GetFmt() );
                    pAny->setValue(&xSect, ::getCppuType((Reference<XTextSection>*)0) );
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
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
                    if( pAny )
                    {
                        Reference< XFootnote >  xFoot = new SwXFootnote(rPam.GetDoc(), rFtn);
                        pAny->setValue(&xFoot, ::getCppuType((Reference<XFootnote>*)0));
                    }
                }
                else
                    eNewState = PropertyState_DEFAULT_VALUE;
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_REFERENCE_MARK:
        {
            SwTxtAttr* pTxtAttr = rPam.GetNode()->GetTxtNode()->
                                        GetTxtAttr(rPam.GetPoint()->nContent, RES_TXTATR_REFMARK);
            if(pTxtAttr)
            {
                if( pAny )
                {
                    const SwFmtRefMark& rRef = pTxtAttr->GetRefMark();
                    Reference< XTextContent >  xRef = SwXReferenceMarks::GetObject( rPam.GetDoc(), &rRef );
                    pAny->setValue(&xRef, ::getCppuType((Reference<XTextContent>*)0));
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_CHARFMT_SEQUENCE:
        {

            SwTxtNode* pTxtNode;
            if((pTxtNode = (SwTxtNode*)rPam.GetNode( TRUE )) == rPam.GetNode(FALSE) &&
                    pTxtNode->GetpSwpHints())
            {
                USHORT nPaMStart = rPam.GetPoint()->nContent.GetIndex();
                USHORT nPaMEnd = rPam.GetMark() ? rPam.GetMark()->nContent.GetIndex() : nPaMStart;
                if(nPaMStart > nPaMEnd)
                {
                    USHORT nTmp = nPaMStart;
                    nPaMStart = nPaMEnd;
                    nPaMEnd = nTmp;
                }
                Sequence< ::rtl::OUString> aCharStyles;
                USHORT nCharStylesFound = 0;
                SwpHints* pHints = pTxtNode->GetpSwpHints();
                for(USHORT nAttr = 0; nAttr < pHints->GetStartCount(); nAttr++ )
                {
                    SwTxtAttr* pAttr = pHints->GetStart( nAttr );
                    if(pAttr->Which() != RES_TXTATR_CHARFMT)
                        continue;
                    USHORT nAttrStart = *pAttr->GetStart();
                    USHORT nAttrEnd = *pAttr->GetEnd();
                    //check if the attribute touches the selection
                    if(nAttrEnd > nPaMStart && nAttrStart < nPaMEnd && nAttrEnd > nPaMStart)
                    {
                        //check for overlapping
                        if(nAttrStart > nPaMStart ||
                                    nAttrEnd < nPaMEnd)
                        {
                            aCharStyles.realloc(0);
                            eNewState = PropertyState_AMBIGUOUS_VALUE;
                            break;
                        }
                        else
                        {
                            //now the attribute should start before or at the selection
                            //and it should end at the end of the selection or behind
                            DBG_ASSERT(nAttrStart <= nPaMStart && nAttrEnd >=nPaMEnd,
                                    "attribute overlaps or is outside");
                            //now the name of the style has to be added to the sequence
                            aCharStyles.realloc(aCharStyles.getLength() + 1);
                            DBG_ASSERT(pAttr->GetCharFmt().GetCharFmt(), "no character format set");
                            aCharStyles.getArray()[aCharStyles.getLength() - 1] =
                                        SwStyleNameMapper::GetProgName(
                                            pAttr->GetCharFmt().GetCharFmt()->GetName(), GET_POOLID_CHRFMT);
                        }
                    }

                }
                if(aCharStyles.getLength())
                    eNewState = PropertyState_DIRECT_VALUE;
                if(pAny)
                    (*pAny) <<= aCharStyles;
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case RES_TXTATR_CHARFMT:
        // kein break hier!
        default: bDone = sal_False;
    }
    if( bDone )
        eState = eNewState;
    return bDone;
};
/* -----------------30.06.98 10:30-------------------
 *
 * --------------------------------------------------*/
sal_Int16 IsNodeNumStart(SwPaM& rPam, PropertyState& eState)
{
    const SwTxtNode* pTxtNd = rPam.GetNode()->GetTxtNode();
    if( pTxtNd && pTxtNd->GetNum() && pTxtNd->GetNumRule() )
    {
        eState = PropertyState_DIRECT_VALUE;
        sal_uInt16 nTmp = pTxtNd->GetNum()->GetSetValue();
        return USHRT_MAX == nTmp ? -1 : (sal_Int16)nTmp;
    }
    eState = PropertyState_DEFAULT_VALUE;
    return -1;
}

/* -----------------25.05.98 11:41-------------------
 *
 * --------------------------------------------------*/
void setNumberingProperty(const Any& rValue, SwPaM& rPam)
{
    Reference<XIndexReplace> xIndexReplace;
    if(rValue >>= xIndexReplace)
    {
        SwXNumberingRules* pSwNum = 0;

        Reference<XUnoTunnel> xNumTunnel(xIndexReplace, UNO_QUERY);
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

                            // CharStyle besorgen und an der Rule setzen
                            sal_uInt16 nChCount = pDoc->GetCharFmts()->Count();
                            SwCharFmt* pCharFmt = 0;
                            for(sal_uInt16 nCharFmt = 0; nCharFmt < nChCount; nCharFmt++)
                            {
                                SwCharFmt& rChFmt = *((*(pDoc->GetCharFmts()))[nCharFmt]);;
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
                UnoActionContext aAction(pDoc);

                if( rPam.GetNext() != &rPam )           // Mehrfachselektion ?
                {
                    pDoc->StartUndo( UNDO_START );
                    SwPamRanges aRangeArr( rPam );
                    SwPaM aPam( *rPam.GetPoint() );
                    for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
                        pDoc->SetNumRule( aRangeArr.SetPam( n, aPam ), aRule );
                    pDoc->EndUndo( UNDO_END );
                }
                else
                    pDoc->SetNumRule( rPam, aRule );


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
    else if(rValue.getValueType() == ::getVoidCppuType())
    {
        rPam.GetDoc()->DelNumRules(rPam);
    }
}


/* -----------------25.05.98 11:40-------------------
 *
 * --------------------------------------------------*/
void  getNumberingProperty(SwPaM& rPam, PropertyState& eState, Any * pAny )
{
    const SwNumRule* pNumRule = rPam.GetDoc()->GetCurrNumRule( *rPam.GetPoint() );
    if(pNumRule)
    {
        Reference< XIndexReplace >  xNum = new SwXNumberingRules(*pNumRule);
        if ( pAny )
            pAny->setValue(&xNum, ::getCppuType((const Reference<XIndexReplace>*)0));
        eState = PropertyState_DIRECT_VALUE;
    }
    else
        eState = PropertyState_DEFAULT_VALUE;
}
/* -----------------04.07.98 15:15-------------------
 *
 * --------------------------------------------------*/
void GetCurPageStyle(SwPaM& rPaM, String &rString)
{
    const SwPageFrm* pPage = rPaM.GetCntntNode()->GetFrm()->FindPageFrm();
    if(pPage)
        SwStyleNameMapper::FillProgName( pPage->GetPageDesc()->GetName(), rString, GET_POOLID_PAGEDESC, sal_True );
}
/* -----------------30.03.99 10:52-------------------
 * spezielle Properties am Cursor zuruecksetzen
 * --------------------------------------------------*/
void resetCrsrPropertyValue(const SfxItemPropertyMap* pMap, SwPaM& rPam)
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
        case FN_UNO_CHARFMT_SEQUENCE:
        {
            SvUShortsSort aWhichIds;
            aWhichIds.Insert(RES_TXTATR_CHARFMT);
            pDoc->ResetAttr(rPam, sal_True, &aWhichIds);
        }
        break;
    }
}
/* -----------------21.07.98 11:36-------------------
 *
 * --------------------------------------------------*/
void InsertFile(SwUnoCrsr* pUnoCrsr,
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

    SfxObjectFactory& rFact = pDocSh->GetFactory();
    const SfxFilter* pFilter = rFact.GetFilterContainer()->GetFilter4FilterName( rFilterName );
    if ( !pFilter )
    {
        pMed = new SfxMedium(rFileName, STREAM_READ, sal_True, 0, 0 );
        SfxFilterMatcher aMatcher( rFact.GetFilterContainer()->GetName() );
        ErrCode nErr = aMatcher.GuessFilter( *pMed, &pFilter, sal_False );
        if ( nErr || !pFilter)
            DELETEZ(pMed);
        else
            pMed->SetFilter( pFilter );
    }
    else
    {
        pMed = new SfxMedium(rFileName, STREAM_READ, sal_True,
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

}//namespace SwUnoCursorHelper

