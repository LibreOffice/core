/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unocrsrhelper.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:27:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


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
#ifndef _UNOFRAME_HXX
#include <unoframe.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
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
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#include <comphelper/storagehelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using ::rtl::OUString;


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
            if (pAny)
            {
                const SwTxtNode * pTmpNode = pNode;

                if (!pTmpNode)
                    pTmpNode = rPam.GetNode()->GetTxtNode();

                sal_Int8 nRet = -1;
                if (pTmpNode && pTmpNode->GetOutlineLevel() != NO_NUMBERING)
                    nRet = sal::static_int_cast< sal_Int8 >(pTmpNode->GetOutlineLevel());

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
                    SwStyleNameMapper::FillProgName(pFmt->GetName(), sVal, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True );
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
            // --> OD 2006-10-19 #134160# - make code robust:
            // consider case that PaM doesn't denote a text node
            const SwNumRule* pRule = pTxtNd ? pTxtNd->GetNumRule() : 0;
            // <--
            // hier wird Multiselektion nicht beruecksichtigt
            if( pRule )
            {
                if( pAny )
                {
                    if(pMap->nWID == FN_UNO_NUM_LEVEL)
                        *pAny <<= (sal_Int16)(pTxtNd->GetLevel());
                    else if(pMap->nWID == FN_UNO_IS_NUMBER)
                    {
                        BOOL bIsNumber = pTxtNd->IsCounted();
                        pAny->setValue(&bIsNumber, ::getBooleanCppuType());
                    }
                    else /*if(pMap->nWID == UNO_NAME_PARA_IS_NUMBERING_RESTART)*/
                    {
                        BOOL bIsRestart = pTxtNd->IsRestart();
                        pAny->setValue(&bIsRestart, ::getBooleanCppuType());
                    }
                }
            }
            else
            {
                eNewState = PropertyState_DEFAULT_VALUE;

                if( pAny )
                {
                    // #i30838# set default values for default properties
                    if(pMap->nWID == FN_UNO_NUM_LEVEL)
                        *pAny <<= static_cast<sal_Int16>( 0 );
                    else if(pMap->nWID == FN_UNO_IS_NUMBER)
                        *pAny <<= false;
                    else /*if(pMap->nWID == UNO_NAME_PARA_IS_NUMBERING_RESTART)*/
                        *pAny <<= false;
                }
            }
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
                    uno::Reference< XDocumentIndexMark >  xRef = SwXDocumentIndexMark::GetObject(
                            (SwTOXType*)rMark.GetTOXType(), &rMark, rPam.GetDoc());
                    pAny->setValue(&xRef, ::getCppuType((uno::Reference<XDocumentIndex>*)0));
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
                    uno::Reference< XDocumentIndex > aRef =
                        SwXDocumentIndexes::GetObject((SwTOXBaseSection*)pBase);
                    pAny->setValue(&aRef, ::getCppuType((uno::Reference<XDocumentIndex>*)0));
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
                    uno::Reference< XTextField >  xRet = pFld;
                    pAny->setValue(&xRet, ::getCppuType((uno::Reference<XTextField>*)0));
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
                uno::Reference< XBookmark >  xBkm = SwXBookmarks::GetObject(rBkm);
                rAny.set(&xBkm, ::getCppuType((const XBookmark*)0)());
            }
        }
        break;*/
        case FN_UNO_TEXT_TABLE:
        case FN_UNO_CELL:
        {
            SwStartNode* pSttNode = rPam.GetNode()->StartOfSectionNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();
            if(SwTableBoxStartNode == eType)
            {
                if( pAny )
                {
                    const SwTableNode* pTblNode = pSttNode->FindTableNode();
                    SwFrmFmt* pTableFmt = (SwFrmFmt*)pTblNode->GetTable().GetFrmFmt();
                    //SwTable& rTable = ((SwTableNode*)pSttNode)->GetTable();
                    if(FN_UNO_TEXT_TABLE == pMap->nWID)
                    {
                        uno::Reference< XTextTable >  xTable = SwXTextTables::GetObject(*pTableFmt);
                        pAny->setValue(&xTable, ::getCppuType((uno::Reference<XTextTable>*)0));
                    }
                    else
                    {
                        SwTableBox* pBox = pSttNode->GetTblBox();
                        uno::Reference< XCell >  xCell = SwXCell::CreateXCell(pTableFmt, pBox);
                        pAny->setValue(&xCell, ::getCppuType((uno::Reference<XCell>*)0));
                    }
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_FRAME:
        {
            SwStartNode* pSttNode = rPam.GetNode()->StartOfSectionNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();

            SwFrmFmt* pFmt;
            if(eType == SwFlyStartNode && 0 != (pFmt = pSttNode->GetFlyFmt()))
            {
                if( pAny )
                {
                    uno::Reference< XTextFrame >  xFrm = (SwXTextFrame*) SwXFrames::GetObject(*pFmt, FLYCNTTYPE_FRM);
                    pAny->setValue(&xFrm, ::getCppuType((uno::Reference<XTextFrame>*)0));
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
                    uno::Reference< XTextSection >  xSect = SwXTextSections::GetObject( *pSect->GetFmt() );
                    pAny->setValue(&xSect, ::getCppuType((uno::Reference<XTextSection>*)0) );
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
                        uno::Reference< XFootnote >  xFoot = new SwXFootnote(rPam.GetDoc(), rFtn);
                        pAny->setValue(&xFoot, ::getCppuType((uno::Reference<XFootnote>*)0));
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
                    uno::Reference< XTextContent >  xRef = SwXReferenceMarks::GetObject( rPam.GetDoc(), &rRef );
                    pAny->setValue(&xRef, ::getCppuType((uno::Reference<XTextContent>*)0));
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
                SwpHints* pHints = pTxtNode->GetpSwpHints();
                for(USHORT nAttr = 0; nAttr < pHints->GetStartCount(); nAttr++ )
                {
                    SwTxtAttr* pAttr = pHints->GetStart( nAttr );
                    if(pAttr->Which() != RES_TXTATR_CHARFMT)
                        continue;
                    USHORT nAttrStart = *pAttr->GetStart();
                    USHORT nAttrEnd = *pAttr->GetEnd();
                    //check if the attribute touches the selection
                    if( ( nAttrEnd > nPaMStart && nAttrStart < nPaMEnd ) ||
                        ( !nAttrStart && !nAttrEnd && !nPaMStart && !nPaMEnd ) )
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
                                            pAttr->GetCharFmt().GetCharFmt()->GetName(), nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
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
    if( pTxtNd && pTxtNd->GetNumRule() && pTxtNd->IsRestart())
    {
        eState = PropertyState_DIRECT_VALUE;
        sal_Int16 nTmp = sal::static_int_cast< sal_Int16 >(pTxtNd->GetStart());
        return nTmp;
    }
    eState = PropertyState_DEFAULT_VALUE;
    return -1;
}

/* -----------------25.05.98 11:41-------------------
 *
 * --------------------------------------------------*/
void setNumberingProperty(const Any& rValue, SwPaM& rPam)
{
    uno::Reference<XIndexReplace> xIndexReplace;
    if(rValue >>= xIndexReplace)
    {
        SwXNumberingRules* pSwNum = 0;

        uno::Reference<XUnoTunnel> xNumTunnel(xIndexReplace, UNO_QUERY);
        if(xNumTunnel.is())
        {
            pSwNum = reinterpret_cast< SwXNumberingRules * >(
                sal::static_int_cast< sal_IntPtr >( xNumTunnel->getSomething( SwXNumberingRules::getUnoTunnelId() )));
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
                    pDoc->StartUndo( UNDO_START, NULL );
                    SwPamRanges aRangeArr( rPam );
                    SwPaM aPam( *rPam.GetPoint() );
                    for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
                        pDoc->SetNumRule( aRangeArr.SetPam( n, aPam ), aRule );
                    pDoc->EndUndo( UNDO_END, NULL );
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
        uno::Reference< XIndexReplace >  xNum = new SwXNumberingRules(*pNumRule);
        if ( pAny )
            pAny->setValue(&xNum, ::getCppuType((const uno::Reference<XIndexReplace>*)0));
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
        SwStyleNameMapper::FillProgName( pPage->GetPageDesc()->GetName(), rString, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, sal_True );
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
                pDoc->StartUndo( UNDO_START, NULL );
                SwPamRanges aRangeArr( rPam );
                SwPaM aPam( *rPam.GetPoint() );
                for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
                    pDoc->SetNodeNumStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), 1 );
                pDoc->EndUndo( UNDO_END, NULL );
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
            pDoc->ResetAttrs(rPam, sal_True, &aWhichIds);
        }
        break;
    }
}
/* -----------------21.07.98 11:36-------------------
 *
 * --------------------------------------------------*/
void InsertFile(SwUnoCrsr* pUnoCrsr,
    const String& rURL,
    const uno::Sequence< beans::PropertyValue >& rOptions
    ) throw( lang::IllegalArgumentException, io::IOException, uno::RuntimeException )
{
    SfxMedium* pMed = 0;
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    SwDocShell* pDocSh = pDoc->GetDocShell();
    comphelper::MediaDescriptor aMediaDescriptor( rOptions );
    ::rtl::OUString sFileName = rURL;
    ::rtl::OUString sFilterName, sFilterOptions, sPassword, sBaseURL;
    uno::Reference < io::XStream > xStream;
    uno::Reference < io::XInputStream > xInputStream;

    if( !sFileName.getLength() )
        aMediaDescriptor[comphelper::MediaDescriptor::PROP_URL()] >>= sFileName;
    if( !sFileName.getLength() )
        aMediaDescriptor[comphelper::MediaDescriptor::PROP_FILENAME()] >>= sFileName;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_INPUTSTREAM()] >>= xInputStream;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_STREAM()] >>= xStream;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_INPUTSTREAM()] >>= xInputStream;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_FILTERNAME()] >>= sFilterName;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_FILTEROPTIONS()] >>= sFilterOptions;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_PASSWORD()] >>= sPassword;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_DOCUMENTBASEURL() ] >>= sBaseURL;
    if ( !xInputStream.is() && xStream.is() )
        xInputStream = xStream->getInputStream();

    if(!pDocSh || (!sFileName.getLength() && !xInputStream.is()))
        return;

    SfxObjectFactory& rFact = pDocSh->GetFactory();
    const SfxFilter* pFilter = rFact.GetFilterContainer()->GetFilter4FilterName( sFilterName );
    uno::Reference < embed::XStorage > xReadStorage;
    if( xInputStream.is() )
    {
        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[0] <<= xInputStream;
        aArgs[1] <<= embed::ElementModes::READ;
        try
        {
            xReadStorage = uno::Reference< embed::XStorage >(
                            ::comphelper::OStorageHelper::GetStorageFactory()->createInstanceWithArguments( aArgs ),
                            uno::UNO_QUERY );
        }
        catch( const io::IOException& rEx)
        {
            (void)rEx;
        }
    }
    if ( !pFilter )
    {
        if( xInputStream.is() && !xReadStorage.is())
        {
            pMed = new SfxMedium;
            pMed->setStreamToLoadFrom(xInputStream, sal_True );
        }
        else
            pMed = xReadStorage.is() ?
                new SfxMedium(xReadStorage, sBaseURL, 0 ) :
                new SfxMedium(sFileName, STREAM_READ, sal_True, 0, 0 );
        if( sBaseURL.getLength() )
            pMed->GetItemSet()->Put( SfxStringItem( SID_DOC_BASEURL, sBaseURL ) );

        SfxFilterMatcher aMatcher( rFact.GetFilterContainer()->GetName() );
        ErrCode nErr = aMatcher.GuessFilter( *pMed, &pFilter, sal_False );
        if ( nErr || !pFilter)
            DELETEZ(pMed);
        else
            pMed->SetFilter( pFilter );
    }
    else
    {
        if(!pMed)
        {
            if( xInputStream.is() && !xReadStorage.is())
            {
                pMed = new SfxMedium;
                pMed->setStreamToLoadFrom(xInputStream, sal_True );
                pMed->SetFilter( pFilter );
            }
            else
            {
                if( xReadStorage.is() )
                {
                    pMed = new SfxMedium(xReadStorage, sBaseURL, 0 );
                    pMed->SetFilter( pFilter );
                }
                else
                    pMed = new SfxMedium(sFileName, STREAM_READ, sal_True, pFilter, 0);
            }
        }
        if(sFilterOptions.getLength())
            pMed->GetItemSet()->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, sFilterOptions ) );
        if( sBaseURL.getLength())
            pMed->GetItemSet()->Put( SfxStringItem( SID_DOC_BASEURL, sBaseURL ) );
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
        if(sPassword.getLength())
            pSet->Put(SfxStringItem(SID_PASSWORD, sPassword));
        Reader *pRead = pDocSh->StartConvertFrom( *pMed, &pRdr, 0, pUnoCrsr);
        if( pRead )
        {

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

/* -----------------14.07.04 ------------------------
 *
 * --------------------------------------------------*/

// insert text and scan for CR characters in order to insert
// paragraph breaks at those positions by calling SplitNode
sal_Bool DocInsertStringSplitCR(
        SwDoc &rDoc,
        const SwPaM &rNewCursor, const String &rText )
{
    sal_Bool bOK = sal_True;

    OUString aTxt;
    xub_StrLen nStartIdx = 0;
    xub_StrLen nMaxLength = STRING_LEN;
    SwTxtNode* pTxtNd = rNewCursor.GetPoint()->nNode.GetNode().GetTxtNode();
    if( pTxtNd )
        nMaxLength = STRING_LEN - pTxtNd->GetTxt().Len();
    xub_StrLen nIdx = rText.Search( '\r', nStartIdx );
    if( ( nIdx == STRING_NOTFOUND && nMaxLength < rText.Len() ) ||
        ( nIdx != STRING_NOTFOUND && nMaxLength < nIdx ) )
        nIdx = nMaxLength;
    while (nIdx != STRING_NOTFOUND )
    {
        DBG_ASSERT( nIdx - nStartIdx >= 0, "index negative!" );
        aTxt = rText.Copy( nStartIdx, nIdx - nStartIdx );
        if (aTxt.getLength() && !rDoc.Insert( rNewCursor, aTxt, true ))
        {
            DBG_ERROR( "Doc->Insert(Str) failed." );
            bOK = sal_False;
        }
        if (!rDoc.SplitNode( *rNewCursor.GetPoint(), false ) )
        {
            DBG_ERROR( "SplitNode failed" );
            bOK = sal_False;
        }
        nStartIdx = nIdx + 1;
        nIdx = rText.Search( '\r', nStartIdx );
    }
    aTxt = rText.Copy( nStartIdx );
    if (aTxt.getLength() && !rDoc.Insert( rNewCursor, aTxt, true ))
    {
        DBG_ERROR( "Doc->Insert(Str) failed." );
        bOK = sal_False;
    }

    return bOK;
}

}//namespace SwUnoCursorHelper

