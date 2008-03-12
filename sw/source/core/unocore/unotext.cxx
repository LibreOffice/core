/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unotext.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:36:03 $
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


#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_CONTROLCHARACTER_HPP_
#include <com/sun/star/text/ControlCharacter.hpp>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOPORT_HXX
#include <unoport.hxx>
#endif
#ifndef _UNOTBL_HXX
#include <unotbl.hxx>
#endif
#ifndef _UNOIDX_HXX
#include <unoidx.hxx>
#endif
#ifndef _UNOFRAME_HXX
#include <unoframe.hxx>
#endif
#ifndef _UNOFIELD_HXX
#include <unofield.hxx>
#endif
#ifndef _UNODRAW_HXX
#include <unodraw.hxx>
#endif
#ifndef _UNOREDLINE_HXX
#include <unoredline.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#include <unoprnms.hxx>
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _UNOCRSRHELPER_HXX
#include <unocrsrhelper.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _SWUNDO_HXX //autogen
#include <swundo.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _BOOKMRK_HXX //autogen
#include <bookmrk.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <memory>
#include <stdlib.h>

using namespace ::com::sun::star;
using ::rtl::OUString;

const sal_Char cInvalidObject[] = "this object is invalid";
/******************************************************************
 * SwXText
 ******************************************************************/
/*-- 09.12.98 12:44:07---------------------------------------------------

  -----------------------------------------------------------------------*/

const SwStartNode *SwXText::GetStartNode() const
{
    return GetDoc()->GetNodes().GetEndOfContent().StartOfSectionNode();
}

uno::Reference< text::XTextCursor >   SwXText::createCursor() throw (uno::RuntimeException)
{
    uno::Reference< text::XTextCursor >  xRet;
    OUString sRet;
    if(IsValid())
    {
        SwNode& rNode = GetDoc()->GetNodes().GetEndOfContent();
        SwPosition aPos(rNode);
        xRet =  (text::XWordCursor*)new SwXTextCursor(this, aPos, GetTextType(), GetDoc());
        xRet->gotoStart(sal_False);
    }
    return xRet;
}
/* -----------------------------15.03.2002 12:39------------------------------

 ---------------------------------------------------------------------------*/
SwXText::SwXText(SwDoc* pDc, CursorType eType) :
    pDoc(pDc),
    bObjectValid(0 != pDc),
    eCrsrType(eType),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT))
{
}
/*-- 09.12.98 12:43:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXText::~SwXText()
{
}
/*-- 09.12.98 12:43:02---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SAL_CALL SwXText::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException)
{
    const uno::Type& rXTextType = ::getCppuType((uno::Reference< text::XText >*)0);
    const uno::Type& rXTextRangeCompareType = ::getCppuType((uno::Reference< text::XTextRangeCompare >*)0);
    const uno::Type& rXSimpleTextType = ::getCppuType((const uno::Reference< XSimpleText >*)0);
    const uno::Type& rXTextRangeType = ::getCppuType((uno::Reference< text::XTextRange >*)0);
    const uno::Type& rXTypeProviderType = ::getCppuType((uno::Reference< lang::XTypeProvider >*)0);
    const uno::Type& rXTextContentInsert = ::getCppuType((uno::Reference< text::XRelativeTextContentInsert >*)0);
    const uno::Type& rXTextContentRemove = ::getCppuType((uno::Reference< text::XRelativeTextContentRemove >*)0);
    const uno::Type& rXPropertySet = ::getCppuType((uno::Reference<beans::XPropertySet>*)0);
    const uno::Type& rXUnoTunnel = ::getCppuType((uno::Reference< lang::XUnoTunnel >*)0);
    const uno::Type& rXTextPortionAppend = ::getCppuType((uno::Reference< text::XTextPortionAppend >*)0);
    const uno::Type& rXParagraphAppend = ::getCppuType((uno::Reference< text::XParagraphAppend >*)0);
    const uno::Type& rXTextContentAppend = ::getCppuType((uno::Reference< text::XTextContentAppend >*)0);
    const uno::Type& rXTextConvert = ::getCppuType((uno::Reference< text::XTextConvert >*)0);
    const uno::Type& rXTextAppend = ::getCppuType((uno::Reference< text::XTextAppend >*)0);
    const uno::Type& rXTextAppendAndConvert = ::getCppuType((uno::Reference< text::XTextAppendAndConvert >*)0);

    uno::Any aRet;
    if(rType == rXTextType)
    {
        uno::Reference< text::XText > xRet = this;
        aRet.setValue(&xRet, rXTextType);
    }
    else if(rType == rXSimpleTextType)
    {
        uno::Reference< XSimpleText > xRet = this;
        aRet.setValue(&xRet, rXSimpleTextType);
    }
    else if(rType == rXTextRangeType)
    {
        uno::Reference< text::XTextRange > xRet = this;
        aRet.setValue(&xRet, rXTextRangeType);
    }
    else if(rType == rXTextRangeCompareType)
    {
        uno::Reference< text::XTextRangeCompare > xRet = this;
        aRet.setValue(&xRet, rXTextRangeCompareType);
    }
    else if(rType == rXTypeProviderType)
    {
        uno::Reference< lang::XTypeProvider > xRet = this;
        aRet.setValue(&xRet, rXTypeProviderType);
    }
    else if(rType == rXTextContentInsert)
    {
        uno::Reference< text::XRelativeTextContentInsert > xRet = this;
        aRet.setValue(&xRet, rXTextContentInsert);
    }
    else if(rType == rXTextContentRemove)
    {
        uno::Reference< text::XRelativeTextContentRemove > xRet = this;
        aRet.setValue(&xRet, rXTextContentRemove);
    }
    else if(rType == rXPropertySet)
    {
        uno::Reference< beans::XPropertySet > xRet = this;
        aRet.setValue(&xRet, rXPropertySet);
    }
    else if(rType == rXUnoTunnel )
    {
        uno::Reference< lang::XUnoTunnel > xRet = this;
        aRet.setValue(&xRet, rXUnoTunnel);
    }
    else if(rType == rXTextAppendAndConvert )
    {
        uno::Reference< XTextAppendAndConvert > xRet = this;
        aRet.setValue(&xRet, rXTextAppendAndConvert);
    }
    else if(rType == rXTextAppend )
    {
        uno::Reference< XTextAppend > xRet = this;
        aRet.setValue(&xRet, rXTextAppend);
    }
    else if(rType == rXTextPortionAppend )
    {
        uno::Reference< XTextPortionAppend > xRet = this;
        aRet.setValue(&xRet, rXTextPortionAppend);
    }
    else if(rType == rXParagraphAppend )
    {
        uno::Reference< XParagraphAppend > xRet = this;
        aRet.setValue(&xRet, rXParagraphAppend );
    }
    else if(rType == rXTextConvert )
    {
        uno::Reference< XTextConvert > xRet = this;
        aRet.setValue(&xRet, rXParagraphAppend );
    }
    else if(rType == rXTextContentAppend )
    {
        uno::Reference< XTextContentAppend > xRet = this;
        aRet.setValue(&xRet, rXTextContentAppend );
    }
    return aRet;
}
/* -----------------------------15.03.00 17:42--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< uno::Type > SAL_CALL SwXText::getTypes() throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aRet(12);
    uno::Type* pTypes = aRet.getArray();
    pTypes[0] = ::getCppuType((uno::Reference< text::XText >*)0);
    pTypes[1] = ::getCppuType((uno::Reference< text::XTextRangeCompare >*)0);
    pTypes[2] = ::getCppuType((uno::Reference< text::XRelativeTextContentInsert >*)0);
    pTypes[3] = ::getCppuType((uno::Reference< text::XRelativeTextContentRemove >*)0);
    pTypes[4] = ::getCppuType((uno::Reference< lang::XUnoTunnel >*)0);
    pTypes[5] = ::getCppuType((uno::Reference< beans::XPropertySet >*)0);
    pTypes[6] = ::getCppuType((uno::Reference< text::XTextPortionAppend >*)0);
    pTypes[7] = ::getCppuType((uno::Reference< text::XParagraphAppend >*)0);
    pTypes[8] = ::getCppuType((uno::Reference< text::XTextContentAppend >*)0);
    pTypes[9] = ::getCppuType((uno::Reference< text::XTextConvert >*)0);
    pTypes[10] = ::getCppuType((uno::Reference< text::XTextAppend >*)0);
    pTypes[11] = ::getCppuType((uno::Reference< text::XTextAppendAndConvert >*)0);

    return aRet;
}

/*-- 09.12.98 12:43:14---------------------------------------------------
    Gehoert der Range in den Text ? - dann einfuegen
  -----------------------------------------------------------------------*/
void SwXText::insertString(const uno::Reference< text::XTextRange > & xTextRange,
                                const OUString& aString, sal_Bool bAbsorb)
                                throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(GetDoc() && xTextRange.is())
    {
        uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
        SwXTextRange* pRange = 0;
        OTextCursorHelper* pCursor = 0;
        if(xRangeTunnel.is())
        {
            pRange  = reinterpret_cast< SwXTextRange * >(
                    sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
            pCursor = reinterpret_cast< OTextCursorHelper * >(
                    sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
        }

        if(pRange && pRange->GetDoc()  == GetDoc() ||
            pCursor && pCursor->GetDoc()  == GetDoc())
        {
            const SwStartNode* pOwnStartNode = GetStartNode();
            if(pCursor)
            {
                const SwStartNode* pTmp = pCursor->GetPaM()->GetNode()->StartOfSectionNode();
                while(pTmp && pTmp->IsSectionNode())
                {
                    pTmp = pTmp->StartOfSectionNode();
                }
                if( !pOwnStartNode || pOwnStartNode != pTmp)
                {
                    throw uno::RuntimeException();
                }
            }
            else //dann pRange
            {
                SwBookmark* pBkm = pRange->GetBookmark();
                const SwStartNode* pTmp = pBkm->GetBookmarkPos().nNode.GetNode().StartOfSectionNode();
                while( pTmp && pTmp->IsSectionNode())
                {
                    pTmp = pTmp->StartOfSectionNode();
                }
                if( !pOwnStartNode || pOwnStartNode != pTmp)
                {
                    throw uno::RuntimeException();
                }
            }
            if(bAbsorb)
            {
                //!! scan for CR characters and inserting the paragraph breaks
                //!! has to be done in the called function.
                //!! Implemented in SwXTextRange::DeleteAndInsert
                xTextRange->setString(aString);
            }
            else
            {
                //hier wird ein PaM angelegt, der vor dem Parameter-PaM liegt, damit der
                //Text davor eingefuegt wird
                UnoActionContext aContext(GetDoc());
                const SwPosition* pPos = pCursor
                                         ? pCursor->GetPaM()->Start()
                                         : pRange->GetBookmark()->BookmarkStart();
                SwPaM aInsertPam(*pPos);
                sal_Bool bGroupUndo = GetDoc()->DoesGroupUndo();
                GetDoc()->DoGroupUndo(sal_False);

                SwUnoCursorHelper::DocInsertStringSplitCR( *GetDoc(), aInsertPam, aString );

                GetDoc()->DoGroupUndo(bGroupUndo);
            }
        }
        else
        {
            throw uno::RuntimeException();
        }
    }
    else
    {
        throw uno::RuntimeException();
    }
}
/*-- 09.12.98 12:43:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXText::insertControlCharacter(const uno::Reference< text::XTextRange > & xTextRange,
                sal_Int16 nControlCharacter, sal_Bool bAbsorb)
                throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(GetDoc() && xTextRange.is())
    {
        SwUnoInternalPaM aPam(*GetDoc());
        if(SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange))
        {
            //Steuerzeichen einfuegen
            SwPaM aTmp(*aPam.Start());
            if(bAbsorb && aPam.HasMark())
                pDoc->DeleteAndJoin(aPam);

            sal_Unicode cIns = 0;
            switch( nControlCharacter )
            {
                case text::ControlCharacter::PARAGRAPH_BREAK :
                    // eine Tabellen Zelle wird jetzt zu einer normalen Textzelle!
                    pDoc->ClearBoxNumAttrs( aTmp.GetPoint()->nNode );
                    pDoc->SplitNode( *aTmp.GetPoint(), sal_False );
                    break;
                case text::ControlCharacter::APPEND_PARAGRAPH:
                {
                    pDoc->ClearBoxNumAttrs( aTmp.GetPoint()->nNode );
                    pDoc->AppendTxtNode( *aTmp.GetPoint() );

                    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
                    SwXTextRange* pRange = 0;
                    OTextCursorHelper* pCursor = 0;
                    if(xRangeTunnel.is())
                    {
                        pRange  = reinterpret_cast< SwXTextRange * >(
                                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
                        pCursor = reinterpret_cast< OTextCursorHelper * >(
                                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
                    }
                    if(pRange)
                    {
                        pRange->_CreateNewBookmark(aTmp);
                    }
                    else if(pCursor)
                    {
                        SwPaM* pCrsr = pCursor->GetPaM();
                        *pCrsr->GetPoint() = *aTmp.GetPoint();
                        pCrsr->DeleteMark();
                    }
                }
                break;
                case text::ControlCharacter::LINE_BREAK:    cIns = 10;      break;
                case text::ControlCharacter::SOFT_HYPHEN:   cIns = CHAR_SOFTHYPHEN; break;
                case text::ControlCharacter::HARD_HYPHEN:   cIns = CHAR_HARDHYPHEN; break;
                case text::ControlCharacter::HARD_SPACE:    cIns = CHAR_HARDBLANK;  break;
            }
            if( cIns )
                pDoc->Insert( aTmp, cIns );

            if(bAbsorb)
            {
                uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
                SwXTextRange* pRange = 0;
                OTextCursorHelper* pCursor = 0;
                if(xRangeTunnel.is())
                {
                    pRange  = reinterpret_cast< SwXTextRange * >(
                            sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
                    pCursor = reinterpret_cast< OTextCursorHelper * >(
                            sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
                }

                SwCursor aCrsr(*aTmp.GetPoint(),0,false);
                SwXTextCursor::SelectPam(aCrsr, sal_True);
                aCrsr.Left(1, CRSR_SKIP_CHARS, FALSE, FALSE);
                //hier muss der uebergebene PaM umgesetzt werden:
                if(pRange)
                    pRange->_CreateNewBookmark(aCrsr);
                else
                {
                    SwPaM* pUnoCrsr = pCursor->GetPaM();
                    *pUnoCrsr->GetPoint() = *aCrsr.GetPoint();
                    if(aCrsr.HasMark())
                    {
                        pUnoCrsr->SetMark();
                        *pUnoCrsr->GetMark() = *aCrsr.GetMark();
                    }
                    else
                        pUnoCrsr->DeleteMark();
                }
            }
        }
    }
    else
        throw uno::RuntimeException();
}
/*-- 09.12.98 12:43:17---------------------------------------------------

  -----------------------------------------------------------------------*/

void SwXText::insertTextContent(const uno::Reference< text::XTextRange > & xRange,
                const uno::Reference< text::XTextContent > & xContent, sal_Bool bAbsorb)
                throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    // erstmal testen, ob der Range an der richtigen Stelle ist und dann
    // am Sw-Content attachToRange aufrufen
    if(!GetDoc())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }
    if(xRange.is() && xContent.is())
    {
        SwUnoInternalPaM aPam(*GetDoc());
        if(SwXTextRange::XTextRangeToSwPaM(aPam, xRange))
        {
            uno::Reference<lang::XUnoTunnel> xRangeTunnel( xRange, uno::UNO_QUERY);
            SwXTextRange* pRange = 0;
            OTextCursorHelper* pCursor = 0;
            SwXTextPortion* pPortion = 0;
            SwXText* pText = 0;
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
            }


            uno::Reference< text::XTextCursor >  xOwnCursor = createCursor();
            uno::Reference<lang::XUnoTunnel> xOwnTunnel( xOwnCursor, uno::UNO_QUERY);
//          OTextCursorHelper* pOwnCursor = reinterpret_cast< OTextCursorHelper * >(
//                  sal::static_int_cast< sal_IntPtr >( xOwnTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));

            const SwStartNode* pOwnStartNode = GetStartNode();
            SwStartNodeType eSearchNodeType = SwNormalStartNode;
            switch(eCrsrType)
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

            const SwNode* pSrcNode = 0;
            if(pCursor && pCursor->GetPaM())
            {
                pSrcNode = pCursor->GetPaM()->GetNode();
            }
            else if (pRange && pRange->GetBookmark())
            {
                SwBookmark* pBkm = pRange->GetBookmark();
                pSrcNode = &pBkm->GetBookmarkPos().nNode.GetNode();
            }
            else if (pPortion && pPortion->GetCrsr())
            {
                pSrcNode = pPortion->GetCrsr()->GetNode();
            }
            else if (pText)
            {
                uno::Reference<text::XTextCursor> xTextCursor = pText->createCursor();
                xTextCursor->gotoEnd(sal_True);
                uno::Reference<lang::XUnoTunnel> xCrsrTunnel( xTextCursor, uno::UNO_QUERY );
                pCursor = reinterpret_cast< OTextCursorHelper * >(
                        sal::static_int_cast< sal_IntPtr >( xCrsrTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
                pSrcNode = pCursor->GetPaM()->GetNode();
            }
            else
                throw lang::IllegalArgumentException();

            const SwStartNode* pTmp = pSrcNode->FindSttNodeByType(eSearchNodeType);

            //SectionNodes ueberspringen
            while(pTmp && pTmp->IsSectionNode())
            {
                pTmp = pTmp->StartOfSectionNode();
            }
            //if the document starts with a section
            while(pOwnStartNode->IsSectionNode())
            {
                pOwnStartNode = pOwnStartNode->StartOfSectionNode();
            }
            //this checks if (this) and xRange are in the same text::XText interface
            if(pOwnStartNode != pTmp)
            {
                uno::RuntimeException aRunException;
                aRunException.Message = C2U("text interface and cursor not related");
                throw aRunException;
            }
            // Sonderbehandlung fuer Contents, die den Range nicht ersetzen, sonder darueber gelegt werden
            // Bookmarks, IndexEntry
            sal_Bool bAttribute = sal_False;
            uno::Reference<lang::XUnoTunnel> xContentTunnel( xContent, uno::UNO_QUERY);
            if(!xContentTunnel.is())
            {
                lang::IllegalArgumentException aArgException;
                aArgException.Message = C2U("text content doesn't support lang::XUnoTunnel");
                throw aArgException;
            }
            SwXDocumentIndexMark* pDocumentIndexMark = reinterpret_cast< SwXDocumentIndexMark * >(
                    sal::static_int_cast< sal_IntPtr >( xContentTunnel->getSomething( SwXDocumentIndexMark::getUnoTunnelId()) ));
            SwXTextSection* pSection = reinterpret_cast< SwXTextSection * >(
                    sal::static_int_cast< sal_IntPtr >( xContentTunnel->getSomething( SwXTextSection::getUnoTunnelId()) ));
            SwXBookmark* pBookmark = reinterpret_cast< SwXBookmark * >(
                    sal::static_int_cast< sal_IntPtr >( xContentTunnel->getSomething( SwXBookmark::getUnoTunnelId()) ));
            SwXReferenceMark* pReferenceMark = reinterpret_cast< SwXReferenceMark * >(
                    sal::static_int_cast< sal_IntPtr >( xContentTunnel->getSomething( SwXReferenceMark::getUnoTunnelId()) ));

            bAttribute = pBookmark || pDocumentIndexMark || pSection || pReferenceMark;

            if(bAbsorb && !bAttribute)
            {
                xRange->setString(aEmptyStr);
            }
            //hier wird tatsaechlich eingefuegt
            uno::Reference< text::XTextRange >  xTempRange;
            if(bAttribute && bAbsorb)
                xTempRange = xRange;
            else
                xTempRange = xRange->getStart();
            SwXTextTable* pTable = reinterpret_cast< SwXTextTable * >(
                    sal::static_int_cast< sal_IntPtr >( xContentTunnel->getSomething( SwXTextTable::getUnoTunnelId()) ));

            if(pTable)
                pTable->attachToRange(xTempRange);
            else
            {
                if(pBookmark)
                    pBookmark ->attachToRange(xTempRange);
                else
                {
                    if(pSection)
                        pSection ->attachToRange(xTempRange);
                    else
                    {
                        SwXFootnote* pFootnote = reinterpret_cast< SwXFootnote * >(
                                sal::static_int_cast< sal_IntPtr >( xContentTunnel->getSomething( SwXFootnote::getUnoTunnelId()) ));

                        if(pFootnote)
                            pFootnote->attachToRange(xTempRange);
                        else
                        {
                            if(pReferenceMark)
                                pReferenceMark->attachToRange(xTempRange);
                            else
                            {
                                SwXFrame* pFrame = reinterpret_cast< SwXFrame * >(
                                        sal::static_int_cast< sal_IntPtr >( xContentTunnel->getSomething( SwXFrame::getUnoTunnelId()) ));

                                if(pFrame)
                                    pFrame->attachToRange(xTempRange);
                                else
                                {
                                    SwXDocumentIndex* pDocumentIndex = reinterpret_cast< SwXDocumentIndex * >(
                                            sal::static_int_cast< sal_IntPtr >( xContentTunnel->getSomething( SwXDocumentIndex::getUnoTunnelId()) ));

                                    if(pDocumentIndex)
                                        pDocumentIndex->attachToRange(xTempRange);
                                    else
                                    {
                                        if(pDocumentIndexMark)
                                            pDocumentIndexMark->attachToRange(xTempRange);
                                        else
                                        {
                                            SwXTextField* pTextField = reinterpret_cast< SwXTextField * >(
                                                    sal::static_int_cast< sal_IntPtr >( xContentTunnel->getSomething( SwXTextField::getUnoTunnelId()) ));

                                            if(pTextField)
                                                pTextField->attachToRange(xTempRange);
                                            else
                                            {
                                                uno::Reference<beans::XPropertySet> xShapeProperties(xContent, uno::UNO_QUERY);
                                                SwXShape* pShape = 0;
                                                if(xShapeProperties.is())
                                                    pShape = reinterpret_cast< SwXShape * >(
                                                            sal::static_int_cast< sal_IntPtr >( xContentTunnel->getSomething( SwXShape::getUnoTunnelId()) ));
                                                if(pShape)
                                                {
                                                    uno::Any aPos(&xRange,
                                                        ::getCppuType((uno::Reference<text::XTextRange>*)0));
                                                    pShape->setPropertyValue(C2U("TextRange"), aPos);

                                                    uno::Reference<frame::XModel> xModel =
                                                                    pDoc->GetDocShell()->GetBaseModel();
                                                    uno::Reference<drawing::XDrawPageSupplier> xPageSupp(
                                                                xModel, uno::UNO_QUERY);

                                                    uno::Reference<drawing::XDrawPage> xPage = xPageSupp->getDrawPage();

                                                    uno::Reference<drawing::XShape> xShape((cppu::OWeakObject*)pShape,
                                                                                                            uno::UNO_QUERY);
                                                    //nuer die XShapes haengen an der Sw-Drawpage
                                                    uno::Reference<drawing::XShapes> xShps(xPage, uno::UNO_QUERY);
                                                    xShps->add(xShape);
                                                }
                                                else
                                                {
                                                    lang::IllegalArgumentException aArgException;
                                                    aArgException.Message = C2U("unknown text content");
                                                    throw aArgException;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            lang::IllegalArgumentException aIllegal;
            aIllegal.Message = C2U("first parameter invalid");
            throw aIllegal;
        }
    }
    else
    {
        lang::IllegalArgumentException aIllegal;
        if(!xRange.is())
            aIllegal.Message = C2U("first parameter invalid;");
        if(!xContent.is())
            aIllegal.Message += C2U("second parameter invalid");
        throw aIllegal;
    }

}
/* -----------------------------10.07.00 15:40--------------------------------

 ---------------------------------------------------------------------------*/
void SwXText::insertTextContentBefore(
    const uno::Reference< text::XTextContent>& xNewContent,
    const uno::Reference< text::XTextContent>& xSuccessor)
        throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }

    SwXParagraph* pPara = SwXParagraph::GetImplementation(xNewContent);
    if(!pPara || !pPara->IsDescriptor() || !xSuccessor.is())
        throw lang::IllegalArgumentException();

    sal_Bool bRet = FALSE;
    SwXTextSection* pXSection = SwXTextSection::GetImplementation( xSuccessor );
    SwXTextTable* pXTable = SwXTextTable::GetImplementation(xSuccessor );
    SwFrmFmt* pTableFmt = pXTable ? pXTable->GetFrmFmt() : 0;
    SwUnoCrsr* pUnoCrsr = 0;
    if(pTableFmt && pTableFmt->GetDoc() == GetDoc())
    {
        SwTable* pTable = SwTable::FindTable( pTableFmt );
        SwTableNode* pTblNode = pTable->GetTableNode();

        SwNodeIndex aTblIdx(  *pTblNode, -1 );
        SwPosition aBefore(aTblIdx);
        bRet = GetDoc()->AppendTxtNode( aBefore );
        pUnoCrsr = GetDoc()->CreateUnoCrsr( aBefore, FALSE);
    }
    else if(pXSection &&
        pXSection->GetFmt() &&
            pXSection->GetFmt()->GetDoc() == GetDoc())
    {
        SwSectionFmt* pSectFmt = pXSection->GetFmt();
        SwSectionNode* pSectNode = pSectFmt->GetSectionNode();

        SwNodeIndex aSectIdx(  *pSectNode, -1 );
        SwPosition aBefore(aSectIdx);
        bRet = GetDoc()->AppendTxtNode( aBefore );
        pUnoCrsr = GetDoc()->CreateUnoCrsr( aBefore, FALSE);
    }
    if(!bRet)
        throw lang::IllegalArgumentException();
    else
    {
        pPara->attachToText(this, pUnoCrsr);
    }

}
/* -----------------------------10.07.00 15:40--------------------------------

 ---------------------------------------------------------------------------*/
void SwXText::insertTextContentAfter(
    const uno::Reference< text::XTextContent>& xNewContent,
    const uno::Reference< text::XTextContent>& xPredecessor)
        throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc())
        throw uno::RuntimeException();
    SwXParagraph* pPara = SwXParagraph::GetImplementation(xNewContent);
    if(!pPara || !pPara->IsDescriptor() || !xPredecessor.is())
        throw lang::IllegalArgumentException();

    SwUnoCrsr* pUnoCrsr = 0;
    SwXTextSection* pXSection = SwXTextSection::GetImplementation( xPredecessor );
    SwXTextTable* pXTable = SwXTextTable::GetImplementation(xPredecessor );
    SwFrmFmt* pTableFmt = pXTable ? pXTable->GetFrmFmt() : 0;
    sal_Bool bRet = FALSE;
    if(pTableFmt && pTableFmt->GetDoc() == GetDoc())
    {
        SwTable* pTable = SwTable::FindTable( pTableFmt );
        SwTableNode* pTblNode = pTable->GetTableNode();

        SwEndNode* pTableEnd = pTblNode->EndOfSectionNode();
        SwPosition aTableEnd(*pTableEnd);
        bRet = GetDoc()->AppendTxtNode( aTableEnd );
        pUnoCrsr = GetDoc()->CreateUnoCrsr( aTableEnd, FALSE);
    }
    else if(pXSection &&
        pXSection->GetFmt() &&
            pXSection->GetFmt()->GetDoc() == GetDoc())
    {
        SwSectionFmt* pSectFmt = pXSection->GetFmt();
        SwSectionNode* pSectNode = pSectFmt->GetSectionNode();
        SwEndNode* pEnd = pSectNode->EndOfSectionNode();
        SwPosition aEnd(*pEnd);
        bRet = GetDoc()->AppendTxtNode( aEnd );
        pUnoCrsr = GetDoc()->CreateUnoCrsr( aEnd, FALSE);
    }
    if(!bRet)
        throw lang::IllegalArgumentException();
    else
    {
        pPara->attachToText(this, pUnoCrsr);
    }
}
/* -----------------------------10.07.00 15:40--------------------------------

 ---------------------------------------------------------------------------*/
void SwXText::removeTextContentBefore(
    const uno::Reference< text::XTextContent>& xSuccessor)
        throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }

    sal_Bool bRet = FALSE;
    SwXTextSection* pXSection = SwXTextSection::GetImplementation( xSuccessor );
    SwXTextTable* pXTable = SwXTextTable::GetImplementation( xSuccessor );
    SwFrmFmt* pTableFmt = pXTable ? pXTable->GetFrmFmt() : 0;
    if(pTableFmt && pTableFmt->GetDoc() == GetDoc())
    {
        SwTable* pTable = SwTable::FindTable( pTableFmt );
        SwTableNode* pTblNode = pTable->GetTableNode();

        SwNodeIndex aTblIdx(  *pTblNode, -1 );
        if(aTblIdx.GetNode().IsTxtNode())
        {
            SwPaM aBefore(aTblIdx);
            bRet = GetDoc()->DelFullPara( aBefore );
        }
    }
    else if(pXSection &&
        pXSection->GetFmt() &&
            pXSection->GetFmt()->GetDoc() == GetDoc())
    {
        SwSectionFmt* pSectFmt = pXSection->GetFmt();
        SwSectionNode* pSectNode = pSectFmt->GetSectionNode();

        SwNodeIndex aSectIdx(  *pSectNode, -1 );
        if(aSectIdx.GetNode().IsTxtNode())
        {
            SwPaM aBefore(aSectIdx);
            bRet = GetDoc()->DelFullPara( aBefore );
        }
    }
    if(!bRet)
        throw lang::IllegalArgumentException();
}
/* -----------------------------10.07.00 15:40--------------------------------

 ---------------------------------------------------------------------------*/
void SwXText::removeTextContentAfter(const uno::Reference< text::XTextContent>& xPredecessor)
        throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetDoc())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }

    sal_Bool bRet = FALSE;
    SwXTextSection* pXSection = SwXTextSection::GetImplementation( xPredecessor );
    SwXTextTable* pXTable = SwXTextTable::GetImplementation(xPredecessor );
    SwFrmFmt* pTableFmt = pXTable ? pXTable->GetFrmFmt() : 0;
    if(pTableFmt && pTableFmt->GetDoc() == GetDoc())
    {
        SwTable* pTable = SwTable::FindTable( pTableFmt );
        SwTableNode* pTblNode = pTable->GetTableNode();
        SwEndNode* pTableEnd = pTblNode->EndOfSectionNode();

        SwNodeIndex aTblIdx(  *pTableEnd, 1 );
        if(aTblIdx.GetNode().IsTxtNode())
        {
            SwPaM aPaM(aTblIdx);
            bRet = GetDoc()->DelFullPara( aPaM );
        }
    }
    else if(pXSection &&
        pXSection->GetFmt() &&
            pXSection->GetFmt()->GetDoc() == GetDoc())
    {
        SwSectionFmt* pSectFmt = pXSection->GetFmt();
        SwSectionNode* pSectNode = pSectFmt->GetSectionNode();
        SwEndNode* pEnd = pSectNode->EndOfSectionNode();
        SwNodeIndex aSectIdx(  *pEnd, 1 );
        if(aSectIdx.GetNode().IsTxtNode())
        {
            SwPaM aAfter(aSectIdx);
            bRet = GetDoc()->DelFullPara( aAfter );
        }
    }
    if(!bRet)
        throw lang::IllegalArgumentException();
}
/*-- 09.12.98 12:43:19---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXText::removeTextContent(const uno::Reference< text::XTextContent > & xContent)
    throw( container::NoSuchElementException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!xContent.is())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("first parameter invalid");
        throw aRuntime;
    }
    else
        xContent->dispose();
}
/*-- 09.12.98 12:43:22---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XText >  SwXText::getText(void)
        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XText >  xRet = (SwXText*)this;
    return xRet;

}
/*-- 09.12.98 12:43:24---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXText::getStart(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextCursor >  xRef = createCursor();
    if(!xRef.is())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }
    xRef->gotoStart(sal_False);
    uno::Reference< text::XTextRange >  xRet(xRef, uno::UNO_QUERY);
    return xRet;
}
/*-- 09.12.98 12:43:27---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXText::getEnd(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextCursor >  xRef = createCursor();
    if(!xRef.is())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }
    else
        xRef->gotoEnd(sal_False);
    uno::Reference< text::XTextRange >  xRet(xRef, uno::UNO_QUERY);;

    return xRet;
}
/*-- 09.12.98 12:43:29---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXText::getString(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextCursor >  xRet = createCursor();
    if(!xRet.is())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }
    else
    {
        xRet->gotoEnd(sal_True);
    }
    return xRet->getString();
}
/*-- 09.12.98 12:43:30---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXText::setString(const OUString& aString) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const SwStartNode* pStartNode = GetStartNode();
    if(!pStartNode)
        throw uno::RuntimeException();

    GetDoc()->StartUndo(UNDO_START, NULL);
    //insert an empty paragraph at the start and at the end to ensure that
    //all tables and sections can be removed by the selecting text::XTextCursor
    {
        SwPosition aStartPos(*pStartNode);
        const SwEndNode* pEnd = pStartNode->EndOfSectionNode();
        SwNodeIndex aEndIdx(*pEnd);
        aEndIdx--;
        //the inserting of nodes should only be done if really necessary
        //to prevent #97924# (removes paragraph attributes when setting the text
        //e.g. of a table cell
        BOOL bInsertNodes = FALSE;
        SwNodeIndex aStartIdx(*pStartNode);
        do
        {
            aStartIdx++;
            SwNode& rCurrentNode = aStartIdx.GetNode();
            if(rCurrentNode.GetNodeType() == ND_SECTIONNODE
                ||rCurrentNode.GetNodeType() == ND_TABLENODE)
            {
                bInsertNodes = TRUE;
                break;
            }
        }
        while(aStartIdx < aEndIdx);
        if(bInsertNodes)
        {
            GetDoc()->AppendTxtNode( aStartPos );
            SwPosition aEndPos(aEndIdx.GetNode());
            SwPaM aPam(aEndPos);
            GetDoc()->AppendTxtNode( *aPam.Start() );
        }
    }

    uno::Reference< text::XTextCursor >  xRet = createCursor();
    if(!xRet.is())
    {
        GetDoc()->EndUndo(UNDO_END, NULL);
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }
    else
    {
        xRet->gotoEnd(sal_True);
    }
    xRet->setString(aString);
    GetDoc()->EndUndo(UNDO_END, NULL);
}
/* -----------------------------28.03.00 11:12--------------------------------
    Description: Checks if pRange/pCursor are member of the same text interface.
                Only one of the pointers has to be set!
 ---------------------------------------------------------------------------*/
sal_Bool    SwXText::CheckForOwnMember(
    const SwXTextRange* pRange,
    const OTextCursorHelper* pCursor)
        throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    DBG_ASSERT((!pRange || !pCursor) && (pRange || pCursor), "only one pointer will be checked" )
    uno::Reference<text::XTextCursor> xOwnCursor = createCursor();

    uno::Reference<lang::XUnoTunnel> xTunnel( xOwnCursor, uno::UNO_QUERY);
    OTextCursorHelper* pOwnCursor = 0;
    if(xTunnel.is())
    {
        pOwnCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething(OTextCursorHelper::getUnoTunnelId()) ));
    }
    DBG_ASSERT(pOwnCursor, "OTextCursorHelper::getUnoTunnelId() ??? ")
    const SwStartNode* pOwnStartNode = pOwnCursor->GetPaM()->GetNode()->StartOfSectionNode();
    SwStartNodeType eSearchNodeType = SwNormalStartNode;
    switch(eCrsrType)
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

    const SwNode* pSrcNode;
    if(pCursor)
    {
        pSrcNode = pCursor->GetPaM()->GetNode();
    }
    else //dann pRange
    {
        SwBookmark* pBkm = pRange->GetBookmark();
        pSrcNode = &pBkm->GetBookmarkPos().nNode.GetNode();
    }
    const SwStartNode* pTmp = pSrcNode->FindSttNodeByType(eSearchNodeType);

    //SectionNodes ueberspringen
    while(pTmp && pTmp->IsSectionNode())
    {
        pTmp = pTmp->StartOfSectionNode();
    }
    //if the document starts with a section
    while(pOwnStartNode->IsSectionNode())
    {
        pOwnStartNode = pOwnStartNode->StartOfSectionNode();
    }
    //this checks if (this) and xRange are in the same text::XText interface
    return(pOwnStartNode == pTmp);
}

/* -----------------------------28.03.00 11:07--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int16 SwXText::ComparePositions(
    const uno::Reference<text::XTextRange>& xPos1,
    const uno::Reference<text::XTextRange>& xPos2)
            throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    sal_Int16 nCompare = 0;
    SwUnoInternalPaM aPam1(*GetDoc());
    SwUnoInternalPaM aPam2(*GetDoc());

    BOOL bExcept = FALSE;
    if(SwXTextRange::XTextRangeToSwPaM(aPam1, xPos1) &&
            SwXTextRange::XTextRangeToSwPaM(aPam2, xPos2))
    {
        uno::Reference<lang::XUnoTunnel> xRangeTunnel1( xPos1, uno::UNO_QUERY);
        SwXTextRange* pRange1 = 0;
        OTextCursorHelper* pCursor1 = 0;
        if(xRangeTunnel1.is())
        {
            pRange1     = reinterpret_cast< SwXTextRange * >(
                    sal::static_int_cast< sal_IntPtr >( xRangeTunnel1->getSomething( SwXTextRange::getUnoTunnelId()) ));
            pCursor1    = reinterpret_cast< OTextCursorHelper * >(
                    sal::static_int_cast< sal_IntPtr >( xRangeTunnel1->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
        }
        uno::Reference<lang::XUnoTunnel> xRangeTunnel2( xPos2, uno::UNO_QUERY);
        SwXTextRange* pRange2 = 0;
        OTextCursorHelper* pCursor2 = 0;
        if(xRangeTunnel2.is())
        {
            pRange2 = reinterpret_cast< SwXTextRange * >(
                    sal::static_int_cast< sal_IntPtr >( xRangeTunnel2->getSomething( SwXTextRange::getUnoTunnelId()) ));
            pCursor2 = reinterpret_cast< OTextCursorHelper * >(
                    sal::static_int_cast< sal_IntPtr >( xRangeTunnel2->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
        }

        if((pRange1||pCursor1) && (pRange2||pCursor2))
        {
            if(CheckForOwnMember(pRange1, pCursor1)
                && CheckForOwnMember( pRange2, pCursor2))
            {
                const SwPosition *pStart1 = 0;
                const SwPosition *pStart2 = 0;

                if(pRange1)
                    pStart1 = pRange1->GetBookmark() ? pRange1->GetBookmark()->BookmarkStart() : 0;
                else
                    pStart1 = pCursor1->GetPaM() ? pCursor1->GetPaM()->Start() : 0;

                if(pRange2)
                    pStart2 = pRange2->GetBookmark() ? pRange2->GetBookmark()->BookmarkStart() : 0;
                else
                    pStart2 = pCursor2->GetPaM() ? pCursor2->GetPaM()->Start() : 0;

                if(pStart1 && pStart2)
                {
                    if(*pStart1 < *pStart2)
                        nCompare = 1;
                    else if(*pStart1 > *pStart2)
                        nCompare = -1;
                    else
                    {
                        DBG_ASSERT(*pStart1 == *pStart2, "SwPositions should be equal here")
                        nCompare = 0;
                    }
                }
                else
                    bExcept = TRUE;
            }
            else
                bExcept = TRUE;
        }
        else
            bExcept = TRUE;
    }
    else
        bExcept = TRUE;
    if(bExcept)
        throw lang::IllegalArgumentException();

    return nCompare;
}

/*-- 28.03.00 10:37:22---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int16 SwXText::compareRegionStarts(
    const uno::Reference<text::XTextRange>& xR1,
    const uno::Reference<text::XTextRange>& xR2)
            throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!xR1.is() || !xR2.is())
        throw lang::IllegalArgumentException();
    uno::Reference<text::XTextRange> xStart1 = xR1->getStart();
    uno::Reference<text::XTextRange> xStart2 = xR2->getStart();

    return ComparePositions(xStart1, xStart2);
}
/*-- 28.03.00 10:37:25---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int16 SwXText::compareRegionEnds(
    const uno::Reference<text::XTextRange>& xR1,
    const uno::Reference<text::XTextRange>& xR2)
                    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!xR1.is() || !xR2.is())
        throw lang::IllegalArgumentException();
    uno::Reference<text::XTextRange> xEnd1 = xR1->getEnd();
    uno::Reference<text::XTextRange> xEnd2 = xR2->getEnd();

    return ComparePositions(xEnd1, xEnd2);
}

/*-- 15.03.2002 12:30:40---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo > SwXText::getPropertySetInfo(  )
    throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo =
        new SfxItemPropertySetInfo(_pMap);
    return xInfo;
}
/*-- 15.03.2002 12:30:42---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXText::setPropertyValue( const ::rtl::OUString& /*aPropertyName*/, const uno::Any& /*aValue*/ )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
    lang::IllegalArgumentException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    throw lang::IllegalArgumentException();
}
/*-- 15.03.2002 12:30:42---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXText::getPropertyValue(
    const ::rtl::OUString& rPropertyName )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw  uno::RuntimeException();
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                    _pMap, rPropertyName);
    uno::Any aRet;
    if(pMap)
    {
        switch(pMap->nWID)
        {
//          no code necessary - the redline is always located at the end node
//            case FN_UNO_REDLINE_NODE_START:
//            break;
            case FN_UNO_REDLINE_NODE_END:
            {
                const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
                USHORT nRedTblCount = rRedTbl.Count();
                if ( nRedTblCount > 0 )
                {
                    const SwStartNode* pStartNode = GetStartNode();
                    ULONG nOwnIndex = pStartNode->EndOfSectionIndex();
                    for(USHORT nRed = 0; nRed < nRedTblCount; nRed++)
                    {
                        const SwRedline* pRedline = rRedTbl[nRed];
                        const SwPosition* pRedStart = pRedline->Start();
                        const SwNodeIndex nRedNode = pRedStart->nNode;
                        if(nOwnIndex == nRedNode.GetIndex())
                        {
                            aRet <<= SwXRedlinePortion::CreateRedlineProperties(*pRedline, TRUE);
                            break;
                        }
                    }
                }
            }
            break;
        }
    }
    else
    {
        beans::UnknownPropertyException aExcept;
        aExcept.Message = C2U("Unknown property: ");
        aExcept.Message += rPropertyName;
        throw aExcept;
    }
    return aRet;

}
/*-- 15.03.2002 12:30:42---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXText::addPropertyChangeListener(
    const ::rtl::OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    throw uno::RuntimeException();
}
/*-- 15.03.2002 12:30:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXText::removePropertyChangeListener(
    const ::rtl::OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException,
            uno::RuntimeException)
{
    throw uno::RuntimeException();
}
/*-- 15.03.2002 12:30:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXText::addVetoableChangeListener(
    const ::rtl::OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/ )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    throw uno::RuntimeException();
}
/*-- 15.03.2002 12:30:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXText::removeVetoableChangeListener(
    const ::rtl::OUString& /*rPropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/ )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    throw uno::RuntimeException();
}

/* -----------------------------08.01.01 09:07--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXText::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------08.01.01 09:07--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SwXText::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}
/*-- 23.06.2006 08:56:30---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange > SwXText::appendParagraph(
                const uno::Sequence< beans::PropertyValue > & rProperties )
                throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    return finishOrAppendParagraph(false, rProperties);
}
/*-- 23.06.2006 08:56:22---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange > SwXText::finishParagraph(
                const uno::Sequence< beans::PropertyValue > & rProperties )
                throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    return finishOrAppendParagraph(true, rProperties);
}

/*-- 08.05.2006 13:26:26---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange > SwXText::finishOrAppendParagraph(
        bool bFinish,
        const uno::Sequence< beans::PropertyValue > & rProperties )
            throw (lang::IllegalArgumentException, uno::RuntimeException)

{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw  uno::RuntimeException();
    uno::Reference< text::XTextRange > xRet;

    const SwStartNode* pStartNode = GetStartNode();
    if(!pStartNode)
        throw  uno::RuntimeException();
    {
        bool bIllegalException = false;
        bool bRuntimeException = false;
        ::rtl::OUString sMessage;
        pDoc->StartUndo(UNDO_START , NULL);
        //find end node, go backward - don't skip tables because the new paragraph has to be the last node
        //aPam.Move( fnMoveBackward, fnGoNode );
        SwPosition aInsertPosition( SwNodeIndex( *pStartNode->EndOfSectionNode(), -1 ) );
        SwPaM aPam(aInsertPosition);
        pDoc->AppendTxtNode( *aPam.GetPoint() );
        //remove attributes from the previous paragraph
        pDoc->ResetAttrs(aPam);
        //in case of finishParagraph the PaM needs to be moved to the previous paragraph
        if(bFinish)
            aPam.Move( fnMoveBackward, fnGoNode );
        if(rProperties.getLength())
        {
            // now set the properties
            const SfxItemPropertyMap* pParagraphMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH);
            SfxItemPropertySet aParaPropSet(pParagraphMap);

            const beans::PropertyValue* pValues = rProperties.getConstArray();

            for( sal_Int32 nProp = 0; nProp < rProperties.getLength(); ++nProp)
            {
                // no sorting of property names required - results in performance issues as long as SfxItemPropertyMap::GetByName
                // is not able to hash the maps
                const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName( pParagraphMap, pValues[nProp].Name );
                if(pMap)
                {
                    try
                    {
                        SwXTextCursor::SetPropertyValue(
                        aPam,
                        aParaPropSet,
                        pValues[nProp].Name,
                        pValues[nProp].Value,
                        pMap, 0);
                    }
                    catch( lang::IllegalArgumentException& rIllegal )
                    {
                        sMessage = rIllegal.Message;
                        bIllegalException = true;
                    }
                    catch( uno::RuntimeException& rRuntime )
                    {
                        sMessage = rRuntime.Message;
                        bRuntimeException = true;
                    }
                }
                else
                    bIllegalException = true;
                if( bIllegalException || bRuntimeException )
                {
                    break;
                }
            }
        }
        pDoc->EndUndo(UNDO_END, NULL);
        if( bIllegalException || bRuntimeException )
        {
            SwUndoIter aUndoIter( &aPam, UNDO_EMPTY );
            pDoc->Undo(aUndoIter);
            if(bIllegalException)
            {
                lang::IllegalArgumentException aEx;
                aEx.Message = sMessage;
                throw aEx;
            }
            else //if(bRuntimeException)
            {
                uno::RuntimeException aEx;
                aEx.Message = sMessage;
                throw aEx;
            }
        }
        SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(*aPam.Start(), sal_False);
        xRet = new SwXParagraph(this, pUnoCrsr);
    }

    return xRet;
}
/*-- 08.05.2006 13:28:26---------------------------------------------------
    Append text portions at the end of the last paragraph of the text
    interface. Support of import filters.
  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange > SwXText::appendTextPortion(
        const ::rtl::OUString& rText,
        const uno::Sequence< beans::PropertyValue > & rCharacterAndParagraphProperties )
            throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw  uno::RuntimeException();
   uno::Reference< text::XTextRange > xRet;
    uno::Reference< text::XTextCursor > xTextCursor = createCursor();
    xTextCursor->gotoEnd(sal_False);

    uno::Reference< lang::XUnoTunnel > xRangeTunnel( xTextCursor, uno::UNO_QUERY_THROW );
    SwXTextCursor* pTextCursor = reinterpret_cast< SwXTextCursor * >(
            sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextCursor::getUnoTunnelId())));
    {
        bool bIllegalException = false;
        bool bRuntimeException = false;
        ::rtl::OUString sMessage;
        pDoc->StartUndo(UNDO_INSERT, NULL);

//        SwPaM aPam(*pStartNode->EndOfSectionNode());
        //aPam.Move( fnMoveBackward, fnGoNode );
        SwUnoCrsr* pCursor = pTextCursor->GetCrsr();
        pCursor->MovePara( fnParaCurr, fnParaEnd );
        pDoc->DontExpandFmt( *pCursor->Start() );

        if(rText.getLength())
        {
            xub_StrLen nContentPos = pCursor->GetPoint()->nContent.GetIndex();
            SwUnoCursorHelper::DocInsertStringSplitCR( *pDoc, *pCursor, rText );
            SwXTextCursor::SelectPam(*pCursor, sal_True);
            pCursor->GetPoint()->nContent = nContentPos;
        }

        if(rCharacterAndParagraphProperties.getLength())
        {

            const beans::PropertyValue* pValues = rCharacterAndParagraphProperties.getConstArray();
            for( sal_Int32 nProp = 0; nProp < rCharacterAndParagraphProperties.getLength(); ++nProp)
            {
                // no sorting of property names required - results in performance issues as long as SfxItemPropertyMap::GetByName
                // is not able to hash the maps
                const SfxItemPropertyMap* pCursorMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR);
                SfxItemPropertySet aCursorPropSet(pCursorMap);
                const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName( pCursorMap, pValues[nProp].Name );
                if(pMap)
                {
                    try
                    {
                        SwXTextCursor::SetPropertyValue(
                        *pCursor,
                        aCursorPropSet,
                        pValues[nProp].Name,
                        pValues[nProp].Value,
                        pMap, nsSetAttrMode::SETATTR_NOFORMATATTR);
                    }
                    catch( lang::IllegalArgumentException& rIllegal )
                    {
                        sMessage = rIllegal.Message;
                        bIllegalException = true;
                    }
                    catch( uno::RuntimeException& rRuntime )
                    {
                        sMessage = rRuntime.Message;
                        bRuntimeException = true;
                    }
                }
                else
                    bIllegalException = true;
                if( bIllegalException || bRuntimeException )
                {
                    break;
                }
            }
        }
        pDoc->EndUndo(UNDO_INSERT, NULL);
        if( bIllegalException || bRuntimeException )
        {
            SwUndoIter aUndoIter( pCursor, UNDO_EMPTY );
            pDoc->Undo(aUndoIter);
            delete pCursor;
            pCursor = 0;
            if(bIllegalException)
            {
                lang::IllegalArgumentException aEx;
                aEx.Message = sMessage;
                throw aEx;
            }
            else //if(bRuntimeException)
            {
                uno::RuntimeException aEx;
                aEx.Message = sMessage;
                throw aEx;
            }
        }
        xRet = new SwXTextRange(*pCursor, this);
        delete pCursor;
    }
   return xRet;
}
/*-- 11.05.2006 15:46:26---------------------------------------------------
    enable appending text contents like graphic objects, shapes and so on
    to support import filters
  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange > SwXText::appendTextContent(
    const uno::Reference< text::XTextContent >& xTextContent,
    const uno::Sequence< beans::PropertyValue >& rCharacterAndParagraphProperties )
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw  uno::RuntimeException();
    const SwStartNode* pStartNode = GetStartNode();
    if(!pStartNode)
        throw  uno::RuntimeException();
    uno::Reference< text::XTextRange > xRet;
    {
        pDoc->StartUndo(UNDO_INSERT, NULL);
        //find end node, go backward - don't skip tables because the new paragraph has to be the last node
        SwPaM aPam(*pStartNode->EndOfSectionNode());
        aPam.Move( fnMoveBackward, fnGoNode );
        //set cursor to the end of the last text node
        SwCursor* pCursor = new SwCursor( *aPam.Start(),0,false );
        xRet = new SwXTextRange(*pCursor, this);
        pCursor->MovePara( fnParaCurr, fnParaEnd );
        pDoc->DontExpandFmt( *pCursor->Start() );
        //now attach the text content here
        insertTextContent( xRet, xTextContent, false );
        //now apply the properties to the anchor
        if( rCharacterAndParagraphProperties.getLength())
        {
            try
            {
                uno::Reference< beans::XPropertySet > xAnchor( xTextContent->getAnchor(), uno::UNO_QUERY);
                if( xAnchor.is() )
                {
                    for( sal_Int32 nElement = 0; nElement < rCharacterAndParagraphProperties.getLength(); ++nElement )
                    {
                        xAnchor->setPropertyValue( rCharacterAndParagraphProperties[nElement].Name, rCharacterAndParagraphProperties[nElement].Value );
                    }
                }

            }
            catch(const uno::Exception&)
            {
                throw uno::RuntimeException();
            }
        }
        delete pCursor;
        pDoc->EndUndo(UNDO_INSERT, NULL);
    }
    return xRet;
}
/*-- 11.05.2006 15:46:26---------------------------------------------------
    move previously appended paragraphs into a text frames
    to support import filters
  -----------------------------------------------------------------------*/
uno::Reference< text::XTextContent > SwXText::convertToTextFrame(
    const uno::Reference< text::XTextRange >& xStart,
    const uno::Reference< text::XTextRange >& xEnd,
    const uno::Sequence< beans::PropertyValue >& rFrameProperties )
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw  uno::RuntimeException();
    uno::Reference< text::XTextContent > xRet;
    SwUnoInternalPaM aStartPam(*GetDoc());
    std::auto_ptr < SwUnoInternalPaM > pEndPam( new SwUnoInternalPaM(*GetDoc()));
    if(SwXTextRange::XTextRangeToSwPaM(aStartPam, xStart) &&
        SwXTextRange::XTextRangeToSwPaM(*pEndPam, xEnd) )
    {
        uno::Reference<lang::XUnoTunnel> xStartRangeTunnel( xStart, uno::UNO_QUERY);
        SwXTextRange* pStartRange  = reinterpret_cast< SwXTextRange * >(
                   sal::static_int_cast< sal_IntPtr >( xStartRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
        uno::Reference<lang::XUnoTunnel> xEndRangeTunnel( xEnd, uno::UNO_QUERY);
        SwXTextRange* pEndRange  = reinterpret_cast< SwXTextRange * >(
                   sal::static_int_cast< sal_IntPtr >( xEndRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
        //bokmarks have to be removed before the referenced text node is deleted in DelFullPara
        if( pStartRange )
        {
            SwBookmark* pStartBookmark = pStartRange->GetBookmark();
            if( pStartBookmark )
                pDoc->deleteBookmark( pStartBookmark->GetName() );
        }
        if( pEndRange )
        {
            SwBookmark* pEndBookmark = pEndRange->GetBookmark();
            if( pEndBookmark )
                pDoc->deleteBookmark( pEndBookmark->GetName() );
        }

        pDoc->StartUndo( UNDO_START, NULL );
        bool bIllegalException = false;
        bool bRuntimeException = false;
        ::rtl::OUString sMessage;
        SwNode* pStartStartNode = aStartPam.GetNode()->StartOfSectionNode();
        while(pStartStartNode && pStartStartNode->IsSectionNode())
        {
        }
        SwNode* pEndStartNode = pEndPam->GetNode()->StartOfSectionNode();
        while(pEndStartNode && pEndStartNode->IsSectionNode())
        {
            pEndStartNode = pEndStartNode->StartOfSectionNode();
        }
        if(pStartStartNode != pEndStartNode || pStartStartNode != GetStartNode())
            throw lang::IllegalArgumentException();
        //make a selection from aStartPam to a EndPam
        SwSelBoxes aBoxes;
        SfxItemSet aFrameItemSet(pDoc->GetAttrPool(),
                                    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
                                   0 );

        aStartPam.SetMark();
        *aStartPam.End() = *pEndPam->End();
        pEndPam.reset(0);

        SwXTextFrame* pNewFrame;
        uno::Reference< text::XTextFrame > xNewFrame = pNewFrame = new SwXTextFrame( pDoc );
        pNewFrame->SetSelection( aStartPam );
        try
        {
            const beans::PropertyValue* pValues = rFrameProperties.getConstArray();
            for(sal_Int32 nProp = 0; nProp < rFrameProperties.getLength(); ++nProp)
                pNewFrame->SwXFrame::setPropertyValue(pValues[nProp].Name, pValues[nProp].Value);

            {//has to be in a block to remove the SwIndexes before DelFullPara is called
                uno::Reference< text::XTextRange> xInsertTextRange = new SwXTextRange(aStartPam, this);
                pNewFrame->attach( xInsertTextRange );
                pNewFrame->setName(pDoc->GetUniqueFrameName());
            }

            if( !aStartPam.GetTxt().Len() )
            {

                bool bMoved = false;
                {//has to be in a block to remove the SwIndexes before DelFullPara is called
                    SwPaM aMovePam( *aStartPam.GetNode() );
                    if( aMovePam.Move( fnMoveForward, fnGoCntnt ) )
                    {
                        //move the anchor to the next paragraph
                        SwFmtAnchor aNewAnchor( pNewFrame->GetFrmFmt()->GetAnchor() );
                        aNewAnchor.SetAnchor( aMovePam.Start() );
                        pDoc->SetAttr( aNewAnchor, *pNewFrame->GetFrmFmt() );
                    }
                    bMoved = true;
                }
                if(bMoved)
                {
                    aStartPam.DeleteMark();
//                    SwPaM aDelPam( *aStartPam.GetNode() );
                    pDoc->DelFullPara(aStartPam/*aDelPam*/);
                }
            }
        }
        catch( lang::IllegalArgumentException& rIllegal )
        {
            sMessage = rIllegal.Message;
            bIllegalException = true;
        }
        catch( uno::RuntimeException& rRuntime )
        {
            sMessage = rRuntime.Message;
            bRuntimeException = true;
        }
        xRet = pNewFrame;
        pDoc->EndUndo(UNDO_END, NULL);
        if( bIllegalException || bRuntimeException )
        {
            SwUndoIter aUndoIter( &aStartPam, UNDO_EMPTY );
            pDoc->Undo(aUndoIter);
            if(bIllegalException)
            {
                lang::IllegalArgumentException aEx;
                aEx.Message = sMessage;
                throw aEx;
            }
            else //if(bRuntimeException)
            {
                uno::RuntimeException aEx;
                aEx.Message = sMessage;
                throw aEx;
            }
        }
    }
    else
        throw lang::IllegalArgumentException();
    return xRet;
}
/*-- 11.05.2006 15:46:26---------------------------------------------------
    Move previously imported paragraphs into a new text table.

  -----------------------------------------------------------------------*/
struct VerticallyMergedCell
{
    std::vector<uno::Reference< beans::XPropertySet > > aCells;
    sal_Int32                                           nLeftPosition;
    bool                                                bOpen;

    VerticallyMergedCell( uno::Reference< beans::XPropertySet >&   rxCell, sal_Int32 nLeft ) :
        nLeftPosition( nLeft ),
        bOpen( true )
        {
            aCells.push_back( rxCell );
        }
};
#define COL_POS_FUZZY 2
bool lcl_SimilarPosition( sal_Int32 nPos1, sal_Int32 nPos2 )
{
    return abs( nPos1 - nPos2 ) < COL_POS_FUZZY;
}

uno::Reference< text::XTextTable > SwXText::convertToTable(
    const uno::Sequence< uno::Sequence< uno::Sequence< uno::Reference< text::XTextRange > > > >& rTableRanges,
   const uno::Sequence< uno::Sequence< uno::Sequence< beans::PropertyValue > > >& rCellProperties,
   const uno::Sequence< uno::Sequence< beans::PropertyValue > >& rRowProperties,
   const uno::Sequence< beans::PropertyValue >& rTableProperties )
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw  uno::RuntimeException();

    //at first collect the text ranges as SwPaMs
    const uno::Sequence< uno::Sequence< uno::Reference< text::XTextRange > > >* pTableRanges = rTableRanges.getConstArray();
    std::auto_ptr < SwPaM > pFirstPaM;
    std::vector< std::vector<SwNodeRange> > aTableNodes;
    bool bExcept = false;
    SwPaM aLastPaM(pDoc->GetNodes());
    for( sal_Int32 nRow = 0; !bExcept && (nRow < rTableRanges.getLength()); ++nRow)
    {
        std::vector<SwNodeRange> aRowNodes;
        const uno::Sequence< uno::Sequence< uno::Reference< text::XTextRange > > >& rRow = pTableRanges[nRow];
       const uno::Sequence< uno::Reference< text::XTextRange > >* pRow = pTableRanges[nRow].getConstArray();

        for( sal_Int32 nCell = 0; nCell < rRow.getLength(); ++nCell)
        {
            if( pRow[nCell].getLength() != 2 )
                throw lang::IllegalArgumentException();
            const uno::Reference< text::XTextRange > xStartRange = pRow[nCell][0];
            const uno::Reference< text::XTextRange > xEndRange = pRow[nCell][1];
            SwUnoInternalPaM aStartCellPam(*pDoc);
            SwUnoInternalPaM aEndCellPam(*pDoc);

            // !!! TODO - PaMs in tables and sections do not work here - the same applies to PaMs in frames !!!

            if(!SwXTextRange::XTextRangeToSwPaM(aStartCellPam, xStartRange) ||
                !SwXTextRange::XTextRangeToSwPaM(aEndCellPam, xEndRange) )
                throw lang::IllegalArgumentException();
            /** check the nodes between start and end
                it is allowed to have pairs of StartNode/EndNodes
             */
            if(aStartCellPam.Start()->nNode < aEndCellPam.End()->nNode)
            {
                const SwNode& rStartNode = aStartCellPam.Start()->nNode.GetNode();
                if(!rStartNode.IsTxtNode() ||
                        !aEndCellPam.End()->nNode.GetNode().IsTxtNode())
                {
                    //start and end of the cell must be on a SwTxtNode
                    bExcept = true;
                    break;
                }
                // increment on each StartNode and decrement on each EndNode
                // we must reach zero at the end and must not go below zero
                long nOpenNodeBlock = 0;
                SwNodeIndex aCellIndex = aStartCellPam.Start()->nNode;
                while( ++aCellIndex < aEndCellPam.End()->nNode.GetIndex())
                {
                    if( aCellIndex.GetNode().IsStartNode() )
                        ++nOpenNodeBlock;
                    else if(aCellIndex.GetNode().IsEndNode() )
                        --nOpenNodeBlock;
                    if( nOpenNodeBlock < 0 )
                    {
                        bExcept = true;
                        break;
                    }
                }
                if( nOpenNodeBlock != 0)
                {
                    bExcept = true;
                    break;
                }
            }

            /** The vector<vector> NodeRanges has to contain consecutive nodes.
                In rTableRanges the ranges don't need to be full paragraphs but they have to follow
                each other. To process the ranges they have to be aligned on paragraph borders
                by inserting paragraph breaks. Non-consecutive ranges must initiate an
                exception.

             */
            if(!nRow && !nCell)
            {
                //align the beginning - if necessary
                if(aStartCellPam.Start()->nContent.GetIndex())
                    pDoc->SplitNode(*aStartCellPam.Start(), sal_False);
            }
            else
            {
                //check the predecessor
                ULONG nLastNodeIndex = aLastPaM.End()->nNode.GetIndex();
                ULONG nStartCellNodeIndex = aStartCellPam.Start()->nNode.GetIndex();
                ULONG nLastNodeEndIndex = aLastPaM.End()->nNode.GetIndex();
                if( nLastNodeIndex == nStartCellNodeIndex)
                {
                    //- same node as predecessor then equal nContent?
                    if(aLastPaM.End()->nContent != aStartCellPam.Start()->nContent)
                        bExcept = true;
                    else
                    {
                        pDoc->SplitNode(*aStartCellPam.Start(), sal_False);
                    }
                }
                else if(nStartCellNodeIndex == ( nLastNodeEndIndex + 1))
                {
                    //next paragraph - now the content index of the new should be 0
                    //and of the old one should be equal to the text length
                    //but if it isn't we don't care - the cell is being inserted on the
                    //node border anyway
                }
                else
                    bExcept = true;
            }
           //now check if there's a need to insert another paragraph break
            if( aEndCellPam.End()->nContent.GetIndex() < aEndCellPam.End()->nNode.GetNode().GetTxtNode()->Len())
           {
               pDoc->SplitNode(*aEndCellPam.End(), sal_False);
                //take care that the new start/endcell is moved to the right position
               //aStartCellPam has to point to the start of the new (previous) node
               //aEndCellPam has to point the the end of the new (previous) node
                aStartCellPam.DeleteMark();
                aStartCellPam.Move(fnMoveBackward, fnGoNode);
               aStartCellPam.GetPoint()->nContent = 0;
                aEndCellPam.DeleteMark();
                aEndCellPam.Move(fnMoveBackward, fnGoNode);
                aEndCellPam.GetPoint()->nContent = aEndCellPam.GetNode()->GetTxtNode()->Len();

           }

            *aLastPaM.GetPoint() = *aEndCellPam.Start();
            if( aStartCellPam.HasMark() )
            {
                aLastPaM.SetMark();
                *aLastPaM.GetMark() = *aEndCellPam.End();
            }
            else
                aLastPaM.DeleteMark();

            SwNodeRange aCellRange( aStartCellPam.Start()->nNode, aEndCellPam.End()->nNode);
            aRowNodes.push_back(aCellRange);
            if( !nRow && !nCell )
                pFirstPaM.reset( new SwPaM(*aStartCellPam.Start()));
        }
        aTableNodes.push_back(aRowNodes);
    }

    if(bExcept)
    {
        SwUndoIter aUndoIter( &aLastPaM, UNDO_EMPTY );
        pDoc->Undo(aUndoIter);
        throw lang::IllegalArgumentException();
    }

    typedef uno::Sequence< text::TableColumnSeparator > TableColumnSeparators;
    std::vector< TableColumnSeparators > aRowSeparators(rRowProperties.getLength());
    std::vector<VerticallyMergedCell>       aMergedCells;

    const SwTable* pTable = pDoc->TextToTable( aTableNodes );
    SwXTextTable* pTextTable = 0;
    uno::Reference< text::XTextTable > xRet = pTextTable = new SwXTextTable( *pTable->GetFrmFmt() );
    uno::Reference< beans::XPropertySet > xPrSet = pTextTable;
    // set properties to the table - catch lang::WrappedTargetException and lang::IndexOutOfBoundsException
    try
    {
        //apply table properties
        const beans::PropertyValue* pTableProperties = rTableProperties.getConstArray();
        sal_Int32 nProperty = 0;
        for( ; nProperty < rTableProperties.getLength(); ++nProperty)
            xPrSet->setPropertyValue( pTableProperties[nProperty].Name, pTableProperties[nProperty].Value );

        //apply row properties
        uno::Reference< table::XTableRows >  xRows = xRet->getRows();
        const beans::PropertyValues* pRowProperties = rRowProperties.getConstArray();
        sal_Int32 nRow = 0;
        for( ; nRow < xRows->getCount(); ++nRow)
        {
            if( nRow >= rRowProperties.getLength())
            {
                break;
            }
            uno::Reference< beans::XPropertySet > xRow;
            xRows->getByIndex( nRow ) >>= xRow;
            const beans::PropertyValue* pProperties = pRowProperties[nRow].getConstArray();
            for( nProperty = 0; nProperty < pRowProperties[nRow].getLength(); ++nProperty)
            {
                if( pProperties[ nProperty ].Name.equalsAsciiL(
                                RTL_CONSTASCII_STRINGPARAM ( "TableColumnSeparators" )))
                {
                    //add the separators to access the cell's positions for vertical merging later
                    TableColumnSeparators aSeparators;
                    pProperties[ nProperty ].Value >>= aSeparators;
                    aRowSeparators[nRow] = aSeparators;
                }
                xRow->setPropertyValue( pProperties[ nProperty ].Name, pProperties[ nProperty ].Value );
            }
        }

#ifdef DEBUG
//-->debug cell properties of all rows
    {
        ::rtl::OUString sNames;
        for( sal_Int32  nDebugRow = 0; nDebugRow < rCellProperties.getLength(); ++nDebugRow)
        {
            const uno::Sequence< beans::PropertyValues > aDebugCurrentRow = rCellProperties[nDebugRow];
            sal_Int32 nDebugCells = aDebugCurrentRow.getLength();
            (void) nDebugCells;
            for( sal_Int32  nDebugCell = 0; nDebugCell < nDebugCells; ++nDebugCell)
            {
                const uno::Sequence< beans::PropertyValue >& aDebugCellProperties = aDebugCurrentRow[nDebugCell];
                sal_Int32 nDebugCellProperties = aDebugCellProperties.getLength();
                for( sal_Int32  nDebugProperty = 0; nDebugProperty < nDebugCellProperties; ++nDebugProperty)
                {
                    const ::rtl::OUString sName = aDebugCellProperties[nDebugProperty].Name;
                    sNames += sName;
                    sNames += ::rtl::OUString('-');
                }
                sNames += ::rtl::OUString('+');
            }
            sNames += ::rtl::OUString('|');
        }
        (void)sNames;
    }
//--<
#endif

        //apply cell properties
        for( nRow = 0; nRow < rCellProperties.getLength(); ++nRow)
        {
            const uno::Sequence< beans::PropertyValues > aCurrentRow = rCellProperties[nRow];
            sal_Int32 nCells = aCurrentRow.getLength();
            for( sal_Int32  nCell = 0; nCell < nCells; ++nCell)
            {
                const uno::Sequence< beans::PropertyValue >& aCellProperties = aCurrentRow[nCell];
                sal_Int32 nCellProperties = aCellProperties.getLength();
                uno::Reference< beans::XPropertySet > xCell( pTextTable->getCellByPosition(nCell, nRow), uno::UNO_QUERY );
                for( nProperty = 0; nProperty < nCellProperties; ++nProperty)
                {
                    if(aCellProperties[nProperty].Name.equalsAsciiL(
                                RTL_CONSTASCII_STRINGPARAM ( "VerticalMerge")))
                    {
                        //determine left border position
                        //add the cell to a queue of merged cells
                        //
                        sal_Bool bMerge = sal_False;
                        aCellProperties[nProperty].Value >>= bMerge;
                        sal_Int32 nLeftPos = -1;
                        if( !nCell )
                            nLeftPos = 0;
                        else if( aRowSeparators[nRow].getLength() >= nCell )
                        {
                            const text::TableColumnSeparator* pSeparators = aRowSeparators[nRow].getConstArray();
                            nLeftPos = pSeparators[nCell - 1].Position;
                        }
                        if( bMerge )
                        {
                            // 'close' all the cell with the same left position
                            // if separate vertical merges in the same column exist
                            if( aMergedCells.size() )
                            {
                                std::vector<VerticallyMergedCell>::iterator aMergedIter = aMergedCells.begin();
                                while( aMergedIter != aMergedCells.end())
                                {
                                    if( lcl_SimilarPosition( aMergedIter->nLeftPosition, nLeftPos) )
                                    {
                                        aMergedIter->bOpen = false;
                                    }
                                    ++aMergedIter;
                                }
                            }
                            //add the new group of merged cells
                            aMergedCells.push_back(VerticallyMergedCell(xCell, nLeftPos ));
                        }
                        else
                        {
                            //find the cell that
                            DBG_ASSERT(aMergedCells.size(), "the first merged cell is missing")
                            if( aMergedCells.size() )
                            {
                                std::vector<VerticallyMergedCell>::iterator aMergedIter = aMergedCells.begin();
#if OSL_DEBUG_LEVEL > 1
                                bool bDbgFound = false;
#endif
                                while( aMergedIter != aMergedCells.end())
                                {
                                    if( aMergedIter->bOpen &&
                                        lcl_SimilarPosition( aMergedIter->nLeftPosition, nLeftPos) )
                                    {
                                        aMergedIter->aCells.push_back( xCell );
#if OSL_DEBUG_LEVEL > 1
                                        bDbgFound = true;
#endif
                                    }
                                    ++aMergedIter;
                                }
#if OSL_DEBUG_LEVEL > 1
                                DBG_ASSERT( bDbgFound, "couldn't find first vertically merged cell" )
#endif
                            }
                        }
                    }
                    else
                        xCell->setPropertyValue(aCellProperties[nProperty].Name, aCellProperties[nProperty].Value);
                }
            }
        }
        //now that the cell properties are set the vertical merge values have to be applied
        if( aMergedCells.size() )
        {
            std::vector<VerticallyMergedCell>::iterator aMergedIter = aMergedCells.begin();
            while( aMergedIter != aMergedCells.end())
            {
                sal_Int32 nCellCount = (sal_Int32)aMergedIter->aCells.size();
                std::vector<uno::Reference< beans::XPropertySet > >::iterator aCellIter = aMergedIter->aCells.begin();
                bool bFirstCell = true;
                //the first of the cells get's the number of cells set as RowSpan
                //the others get the inverted number of remaining merged cells (3,-2,-1)
                while( aCellIter != aMergedIter->aCells.end() )
                {
                    (*aCellIter)->setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_ROW_SPAN)), uno::makeAny( nCellCount ));
                    if( bFirstCell )
                    {
                        nCellCount *= -1;
                        bFirstCell = false;
                    }
                    ++nCellCount;
                    ++aCellIter;
                }
                ++aMergedIter;
            }
        }
    }
    catch( const lang::WrappedTargetException& rWrapped )
    {
        (void)rWrapped;
    }
    catch ( const lang::IndexOutOfBoundsException& rBounds )
    {
        (void)rBounds;
    }


        bool bIllegalException = false;
        bool bRuntimeException = false;
        ::rtl::OUString sMessage;
        pDoc->StartUndo(UNDO_START, NULL);
        pDoc->EndUndo(UNDO_START, NULL);
        if( bIllegalException || bRuntimeException )
        {
            SwUndoIter aUndoIter( pFirstPaM.get(), UNDO_EMPTY );
            pDoc->Undo(aUndoIter);
            if(bIllegalException)
            {
                lang::IllegalArgumentException aEx;
                aEx.Message = sMessage;
                throw aEx;
            }
            else //if(bRuntimeException)
            {
                uno::RuntimeException aEx;
                aEx.Message = sMessage;
                throw aEx;
            }
        }
    return xRet;
}

/******************************************************************
 * SwXBodyText
 ******************************************************************/
SwXBodyText::SwXBodyText(SwDoc* _pDoc) :
    SwXText(_pDoc, CURSOR_BODY)
{
}

/*-- 10.12.98 11:17:27---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXBodyText::~SwXBodyText()
{

}
/* -----------------------------06.04.00 16:33--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXBodyText::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXBodyText");
}
/* -----------------------------06.04.00 16:33--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXBodyText::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return C2U("com.sun.star.text.Text") == rServiceName;
}
/* -----------------------------06.04.00 16:33--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXBodyText::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.Text");
    return aRet;
}
/*-- 10.12.98 11:17:27---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXBodyText::queryAggregation(
    const uno::Type& rType )
        throw(uno::RuntimeException)
{
    uno::Any aRet;
    const uno::Type& rXEnumerationAccessType = ::getCppuType((uno::Reference< container::XEnumerationAccess >*)0);
    const uno::Type& rXElementAccessType = ::getCppuType((uno::Reference< container::XElementAccess >*)0);
    const uno::Type& rXServiceInfoType = ::getCppuType((uno::Reference< lang::XServiceInfo >*)0);

    if(rType == rXEnumerationAccessType)
    {
        uno::Reference<container::XEnumerationAccess> xRet = this;
        aRet.setValue(&xRet, rXEnumerationAccessType);
    }
    else if(rType == rXElementAccessType)
    {
        uno::Reference<container::XElementAccess> xRet = this;
        aRet.setValue(&xRet, rXElementAccessType);
    }
    else if(rType == rXServiceInfoType)
    {
        uno::Reference<lang::XServiceInfo> xRet = this;
        aRet.setValue(&xRet, rXServiceInfoType);
    }
    else
    {
        aRet = SwXText::queryInterface( rType );
    }
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = OWeakAggObject::queryAggregation( rType );
    return aRet;
}

/*-- 10.12.98 11:17:28---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Type > SwXBodyText::getTypes(  ) throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes = SwXBodyTextBaseClass::getTypes();
    uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();
    long nIndex = aTypes.getLength();
    aTypes.realloc(aTypes.getLength() + aTextTypes.getLength());
    uno::Type* pTypes = aTypes.getArray();
    const uno::Type* pTextTypes = aTextTypes.getConstArray();
    for(int i = 0; i < aTextTypes.getLength(); i++)
        pTypes[nIndex++] = pTextTypes[i];
    return aTypes;
}
/* -----------------------------21.03.00 15:39--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SwXBodyText::getImplementationId(  ) throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}
/*-- 10.12.98 11:17:28---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SAL_CALL
    SwXBodyText::queryInterface( const uno::Type& rType )
        throw(uno::RuntimeException)
{
      uno::Any aRet = SwXText::queryInterface( rType );
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXBodyTextBaseClass::queryInterface( rType );
    return aRet;
}
/* -----------------------------05.01.00 11:07--------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference< text::XTextCursor >  SwXBodyText::CreateTextCursor(sal_Bool bIgnoreTables)
{
    uno::Reference< text::XTextCursor >  xRet;
    if(IsValid())
    {
        SwNode& rNode = GetDoc()->GetNodes().GetEndOfContent();
       //the cursor has to skip tables contained in this text
        SwPaM aPam(rNode);
        aPam.Move( fnMoveBackward, fnGoDoc );
        if(!bIgnoreTables)
        {
            SwTableNode* pTblNode = aPam.GetNode()->FindTableNode();
            SwCntntNode* pCont = 0;
            while( pTblNode )
            {
                aPam.GetPoint()->nNode = *pTblNode->EndOfSectionNode();
                pCont = GetDoc()->GetNodes().GoNext(&aPam.GetPoint()->nNode);
                pTblNode = pCont->FindTableNode();
            }
            if(pCont)
                aPam.GetPoint()->nContent.Assign(pCont, 0);
        }
        xRet =  (text::XWordCursor*)new SwXTextCursor(this, *aPam.GetPoint(), CURSOR_BODY, GetDoc());
    }
    return xRet;
}
/*-- 10.12.98 11:17:29---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextCursor >  SwXBodyText::createTextCursor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextCursor >  aRef = CreateTextCursor(sal_False);
    if(!aRef.is())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }
    return aRef;
}
/*-- 10.12.98 11:17:29---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextCursor >  SwXBodyText::createTextCursorByRange(
            const uno::Reference< text::XTextRange > & aTextPosition) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextCursor >  aRef;
    if(!IsValid())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }
    SwUnoInternalPaM aPam(*GetDoc());
    if(SwXTextRange::XTextRangeToSwPaM(aPam, aTextPosition))
    {
        SwNode& rNode = GetDoc()->GetNodes().GetEndOfContent();

        SwStartNode* p1 = aPam.GetNode()->StartOfSectionNode();
        //document starts with a section?
        while(p1->IsSectionNode())
        {
            p1 = p1->StartOfSectionNode();
        }
        SwStartNode* p2 = rNode.StartOfSectionNode();

        if(p1 == p2)
            aRef =  (text::XWordCursor*)new SwXTextCursor(this , *aPam.GetPoint(), CURSOR_BODY, GetDoc(), aPam.GetMark());
    }
    if(!aRef.is())
        throw uno::RuntimeException();
    return aRef;
}
/*-- 10.12.98 11:17:30---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXBodyText::createEnumeration(void)
                            throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< container::XEnumeration >  aRef;
    if(IsValid())
    {
        SwNode& rNode = GetDoc()->GetNodes().GetEndOfContent();
        SwPosition aPos(rNode);
        SwUnoCrsr* pUnoCrsr = GetDoc()->CreateUnoCrsr(aPos, sal_False);
        pUnoCrsr->Move( fnMoveBackward, fnGoDoc );
        aRef = new SwXParagraphEnumeration(this, pUnoCrsr, CURSOR_BODY);
    }
    else
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }
    return aRef;

}
/* -----------------18.12.98 13:36-------------------
 *
 * --------------------------------------------------*/
uno::Type SwXBodyText::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Reference<text::XTextRange>*)0);
}
/* -----------------18.12.98 13:36-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXBodyText::hasElements(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(IsValid())
        return sal_True;
    else
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }
}
/******************************************************************
 *  SwXHeadFootText
 ******************************************************************/
TYPEINIT1(SwXHeadFootText, SwClient);
/* -----------------------------06.04.00 16:40--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXHeadFootText::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXHeadFootText");
}
/* -----------------------------06.04.00 16:40--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXHeadFootText::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return C2U("com.sun.star.text.Text") == rServiceName;
}
/* -----------------------------06.04.00 16:40--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXHeadFootText::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.Text");
    return aRet;
}
/*-- 11.12.98 10:14:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXHeadFootText::SwXHeadFootText(SwFrmFmt& rHeadFootFmt, BOOL bHeader) :
    SwXText(rHeadFootFmt.GetDoc(), bHeader ? CURSOR_HEADER : CURSOR_FOOTER),
    SwClient(&rHeadFootFmt),
    bIsHeader(bHeader)
{

}
/*-- 11.12.98 10:14:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXHeadFootText::~SwXHeadFootText()
{

}
/*-- 11.12.98 10:14:49---------------------------------------------------

  -----------------------------------------------------------------------*/
const SwStartNode *SwXHeadFootText::GetStartNode() const
{
    const SwStartNode *pSttNd = 0;
    SwFrmFmt* pHeadFootFmt = GetFmt();
    if(pHeadFootFmt)
    {
        const SwFmtCntnt& rFlyCntnt = pHeadFootFmt->GetCntnt();
        if( rFlyCntnt.GetCntntIdx() )
            pSttNd = rFlyCntnt.GetCntntIdx()->GetNode().GetStartNode();
    }
    return pSttNd;
}

uno::Reference< text::XTextCursor >   SwXHeadFootText::createCursor() throw(uno::RuntimeException)
{
    return createTextCursor();
}
/* -----------------------------21.03.00 15:39--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< uno::Type > SwXHeadFootText::getTypes(  ) throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aHFTypes = SwXHeadFootTextBaseClass::getTypes();
    uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();

    long nIndex = aHFTypes.getLength();
    aHFTypes.realloc(
        aHFTypes.getLength() +
        aTextTypes.getLength());

    uno::Type* pHFTypes = aHFTypes.getArray();
    const uno::Type* pTextTypes = aTextTypes.getConstArray();
    for(long nPos = 0; nPos < aTextTypes.getLength(); nPos++)
        pHFTypes[nIndex++] = pTextTypes[nPos];

    return aHFTypes;
}

/* -----------------------------21.03.00 15:39--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SwXHeadFootText::getImplementationId(  ) throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}
/* -----------------------------21.03.00 15:46--------------------------------

 ---------------------------------------------------------------------------*/
uno::Any SwXHeadFootText::queryInterface( const uno::Type& aType ) throw(uno::RuntimeException)
{
    uno::Any aRet = SwXHeadFootTextBaseClass::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType() )
        aRet = SwXText::queryInterface(aType);
    return aRet;
}

/*-- 11.12.98 10:14:50---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextCursor >  SwXHeadFootText::createTextCursor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextCursor >     xRet;
    SwFrmFmt* pHeadFootFmt = GetFmt();
    if(pHeadFootFmt)
    {
        const SwFmtCntnt& rFlyCntnt = pHeadFootFmt->GetCntnt();
        const SwNode& rNode = rFlyCntnt.GetCntntIdx()->GetNode();
        SwPosition aPos(rNode);
        SwXTextCursor* pCrsr = new SwXTextCursor(this, aPos, bIsHeader ? CURSOR_HEADER : CURSOR_FOOTER, GetDoc());
        SwUnoCrsr* pUnoCrsr = pCrsr->GetCrsr();
        pUnoCrsr->Move(fnMoveForward, fnGoNode);

        //save current start node to be able to check if there is content after the table -
        //otherwise the cursor would be in the body text!

        const SwStartNode* pOwnStartNode = rNode.FindSttNodeByType(
                        bIsHeader ? SwHeaderStartNode : SwFooterStartNode);
        //steht hier eine Tabelle?
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
        const SwStartNode* pNewStartNode = pUnoCrsr->GetNode()->FindSttNodeByType(
                        bIsHeader ? SwHeaderStartNode : SwFooterStartNode);
        if(!pNewStartNode || pNewStartNode != pOwnStartNode)
        {
            pCrsr = NULL;
            uno::RuntimeException aExcept;
            aExcept.Message = S2U("no text available");
            throw aExcept;
        }
        xRet =  (text::XWordCursor*)pCrsr;
    }
    else
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }
    return xRet;
}
/*-- 11.12.98 10:14:50---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextCursor >  SwXHeadFootText::createTextCursorByRange(
                const uno::Reference< text::XTextRange > & aTextPosition) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextCursor >  xRet;
    SwFrmFmt* pHeadFootFmt = GetFmt();
    SwUnoInternalPaM aPam(*GetDoc());
    if(pHeadFootFmt && SwXTextRange::XTextRangeToSwPaM(aPam, aTextPosition))
    {
        SwNode& rNode = pHeadFootFmt->GetCntnt().GetCntntIdx()->GetNode();
        SwPosition aPos(rNode);
        SwPaM aHFPam(aPos);
        aHFPam.Move(fnMoveForward, fnGoNode);
        SwStartNode* pOwnStartNode = aHFPam.GetNode()->FindSttNodeByType(
                        bIsHeader ? SwHeaderStartNode : SwFooterStartNode);
        SwStartNode* p1 = aPam.GetNode()->FindSttNodeByType(
            bIsHeader ? SwHeaderStartNode : SwFooterStartNode);
        if(p1 == pOwnStartNode)
            xRet =  (text::XWordCursor*)new SwXTextCursor(this, *aPam.GetPoint(),
                            bIsHeader ? CURSOR_HEADER : CURSOR_FOOTER, GetDoc(), aPam.GetMark());
    }
    return xRet;
}
/* -----------------19.03.99 15:44-------------------
 *
 * --------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXHeadFootText::createEnumeration(void)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< container::XEnumeration >  aRef;
    //wenn this ungueltig ist, dann kommt die uno::Exception aus createTextCursor()
    SwFrmFmt* pHeadFootFmt = GetFmt();
    if(pHeadFootFmt)
    {
        const SwFmtCntnt& rFlyCntnt = pHeadFootFmt->GetCntnt();
        const SwNode& rNode = rFlyCntnt.GetCntntIdx()->GetNode();
        SwPosition aPos(rNode);
        SwUnoCrsr* pUnoCrsr = GetDoc()->CreateUnoCrsr(aPos, sal_False);
        pUnoCrsr->Move(fnMoveForward, fnGoNode);
        aRef = new SwXParagraphEnumeration(this, pUnoCrsr, bIsHeader ? CURSOR_HEADER : CURSOR_FOOTER);
    }
    else
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U(cInvalidObject);
        throw aRuntime;
    }

    return aRef;
}
/* -----------------19.03.99 15:50-------------------
 *
 * --------------------------------------------------*/
uno::Type SwXHeadFootText::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Reference<text::XTextRange>*)0);
}
/* -----------------19.03.99 15:50-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXHeadFootText::hasElements(void) throw( uno::RuntimeException )
{
    return sal_True;
}

/*-- 11.12.98 10:14:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwXHeadFootText::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify( this, pOld, pNew);
}

