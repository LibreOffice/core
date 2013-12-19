/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <txtfrm.hxx>
#include <flyfrm.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <unotextrange.hxx>
#include <unocrsrhelper.hxx>
#include <crstate.hxx>
#include <accmap.hxx>
#include <fesh.hxx>
#include <viewopt.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <breakit.hxx>
#include <accpara.hxx>
#include <access.hrc>
#include <accportions.hxx>
#include <sfx2/viewsh.hxx>      // for ExecuteAtViewShell(...)
#include <sfx2/viewfrm.hxx>      // for ExecuteAtViewShell(...)
#include <sfx2/dispatch.hxx>    // for ExecuteAtViewShell(...)
#include <unotools/charclass.hxx>   // for GetWordBoundary
// for get/setCharacterAttribute(...)

#include <reffld.hxx>
#include <docufld.hxx>
#include <expfld.hxx>
#include <flddat.hxx>
#include <fldui.hrc>
#include "../../ui/inc/fldmgr.hxx"
#include "fldbas.hxx"      // SwField
#include <svl/svstdarr.hxx>
#include <unocrsr.hxx>
//#include <unoobj.hxx>
#include <unoport.hxx>
#include <doc.hxx>
#include <crsskip.hxx>
#include <txtatr.hxx>
#include <acchyperlink.hxx>
#include <acchypertextdata.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <section.hxx>
#include <doctxm.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <algorithm>
#include <docufld.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <modcfg.hxx>
//#include "accnote.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include "swmodule.hxx"
#include "redline.hxx"
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <wrong.hxx>
#include <editeng/brshitem.hxx>
#include <swatrset.hxx>
#include <frmatr.hxx>
#include <unosett.hxx>
#include <paratr.hxx>
#include <com/sun/star/container/XIndexReplace.hpp>
// --> OD 2006-07-12 #i63870#
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <com/sun/star/text/WritingMode2.hpp>
#include <editeng/brshitem.hxx>
#include <viewimp.hxx>
#include <boost/scoped_ptr.hpp>
#include <textmarkuphelper.hxx>
// --> OD 2010-02-22 #i10825#
#include <parachangetrackinginfo.hxx>
#include <com/sun/star/text/TextMarkupType.hpp>
// <--
// --> OD 2010-03-08 #i92233#
#include <comphelper/stlunosequence.hxx>
// <--

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::container;
using ::rtl::OUString;

using beans::PropertyValue;
using beans::XMultiPropertySet;
using beans::UnknownPropertyException;
using beans::PropertyState_DIRECT_VALUE;

using std::max;
using std::min;
using std::sort;

namespace com { namespace sun { namespace star {
    namespace text {
        class XText;
    }
} } }


const sal_Char sServiceName[] = "com.sun.star.text.AccessibleParagraphView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleParagraphView";
const xub_StrLen MAX_DESC_TEXT_LEN = 40;
const SwTxtNode* SwAccessibleParagraph::GetTxtNode() const
{
    const SwFrm* pFrm = GetFrm();
    DBG_ASSERT( pFrm->IsTxtFrm(), "The text frame has mutated!" );

    const SwTxtNode* pNode = static_cast<const SwTxtFrm*>(pFrm)->GetTxtNode();
    DBG_ASSERT( pNode != NULL, "A text frame without a text node." );

    return pNode;
}

::rtl::OUString SwAccessibleParagraph::GetString()
{
    return GetPortionData().GetAccessibleString();
}

::rtl::OUString SwAccessibleParagraph::GetDescription()
{
    // --> OD 2004-09-29 #117933# - provide empty description for paragraphs
    return ::rtl::OUString();
    // <--
}

sal_Int32 SwAccessibleParagraph::GetCaretPos()
{
    sal_Int32 nRet = -1;

    // get the selection's point, and test whether it's in our node
    // --> OD 2005-12-20 #i27301# - consider adjusted method signature
    SwPaM* pCaret = GetCursor( false );  // caret is first PaM in PaM-ring
    // <--
    if( pCaret != NULL )
    {
        const SwTxtNode* pNode = GetTxtNode();

        // check whether the point points into 'our' node
        SwPosition* pPoint = pCaret->GetPoint();
        if( pNode->GetIndex() == pPoint->nNode.GetIndex() )
        {
            // same node? Then check whether it's also within 'our' part
            // of the paragraph
            sal_uInt16 nIndex = pPoint->nContent.GetIndex();
            if(!GetPortionData().IsValidCorePosition( nIndex ) ||
                ( GetPortionData().IsZeroCorePositionData() && nIndex== 0) )
            {
                SwTxtFrm *pTxtFrm = PTR_CAST( SwTxtFrm, GetFrm() );
                bool bFormat = (pTxtFrm && pTxtFrm->HasPara());
                if(bFormat)
                {
                    ClearPortionData();
                    UpdatePortionData();
                }
            }
            if( GetPortionData().IsValidCorePosition( nIndex ) )
            {
                // Yes, it's us!
                // --> OD 2006-10-19 #70538#
                // consider that cursor/caret is in front of the list label
                if ( pCaret->IsInFrontOfLabel() )
                {
                    nRet = 0;
                }
                else
                {
                    nRet = GetPortionData().GetAccessiblePosition( nIndex );
                }
                // <--

                DBG_ASSERT( nRet >= 0, "invalid cursor?" );
                DBG_ASSERT( nRet <= GetPortionData().GetAccessibleString().
                                              getLength(), "invalid cursor?" );
            }
            // else: in this paragraph, but in different frame
        }
        // else: not in this paragraph
    }
    // else: no cursor -> no caret

    return nRet;
}

sal_Bool SwAccessibleParagraph::GetSelection(
    sal_Int32& nStart, sal_Int32& nEnd)
{
    sal_Bool bRet = sal_False;
    nStart = -1;
    nEnd = -1;

    // get the selection, and test whether it affects our text node
    // --> OD 2005-12-20 #i27301# - consider adjusted method signature
    SwPaM* pCrsr = GetCursor( true );
    // <--
    if( pCrsr != NULL )
    {
        // get SwPosition for my node
        const SwTxtNode* pNode = GetTxtNode();
        sal_uLong nHere = pNode->GetIndex();

        // iterate over ring
        SwPaM* pRingStart = pCrsr;
        do
        {
            // ignore, if no mark
            if( pCrsr->HasMark() )
            {
                // check whether nHere is 'inside' pCrsr
                SwPosition* pStart = pCrsr->Start();
                sal_uLong nStartIndex = pStart->nNode.GetIndex();
                SwPosition* pEnd = pCrsr->End();
                sal_uLong nEndIndex = pEnd->nNode.GetIndex();
                if( ( nHere >= nStartIndex ) &&
                    ( nHere <= nEndIndex )      )
                {
                    // translate start and end positions

                    // start position
                    sal_Int32 nLocalStart = -1;
                    if( nHere > nStartIndex )
                    {
                        // selection starts in previous node:
                        // then our local selection starts with the paragraph
                        nLocalStart = 0;
                    }
                    else
                    {
                        DBG_ASSERT( nHere == nStartIndex,
                                    "miscalculated index" );

                        // selection starts in this node:
                        // then check whether it's before or inside our part of
                        // the paragraph, and if so, get the proper position
                        sal_uInt16 nCoreStart = pStart->nContent.GetIndex();
                        if( nCoreStart <
                            GetPortionData().GetFirstValidCorePosition() )
                        {
                            nLocalStart = 0;
                        }
                        else if( nCoreStart <=
                                 GetPortionData().GetLastValidCorePosition() )
                        {
                            DBG_ASSERT(
                                GetPortionData().IsValidCorePosition(
                                                                  nCoreStart ),
                                 "problem determining valid core position" );

                            nLocalStart =
                                GetPortionData().GetAccessiblePosition(
                                                                  nCoreStart );
                        }
                    }

                    // end position
                    sal_Int32 nLocalEnd = -1;
                    if( nHere < nEndIndex )
                    {
                        // selection ends in following node:
                        // then our local selection extends to the end
                        nLocalEnd = GetPortionData().GetAccessibleString().
                                                                   getLength();
                    }
                    else
                    {
                        DBG_ASSERT( nHere == nEndIndex,
                                    "miscalculated index" );

                        // selection ends in this node: then select everything
                        // before our part of the node
                        sal_uInt16 nCoreEnd = pEnd->nContent.GetIndex();
                        if( nCoreEnd >
                                GetPortionData().GetLastValidCorePosition() )
                        {
                            // selection extends beyond out part of this para
                            nLocalEnd = GetPortionData().GetAccessibleString().
                                                                   getLength();
                        }
                        else if( nCoreEnd >=
                                 GetPortionData().GetFirstValidCorePosition() )
                        {
                            // selection is inside our part of this para
                            DBG_ASSERT(
                                GetPortionData().IsValidCorePosition(
                                                                  nCoreEnd ),
                                 "problem determining valid core position" );

                            nLocalEnd = GetPortionData().GetAccessiblePosition(
                                                                   nCoreEnd );
                        }
                    }

                    if( ( nLocalStart != -1 ) && ( nLocalEnd != -1 ) )
                    {
                        nStart = nLocalStart;
                        nEnd = nLocalEnd;
                        bRet = sal_True;
                    }
                }
                // else: this PaM doesn't point to this paragraph
            }
            // else: this PaM is collapsed and doesn't select anything

            // next PaM in ring
            pCrsr = static_cast<SwPaM*>( pCrsr->GetNext() );
        }
        while( !bRet && (pCrsr != pRingStart) );
    }
    // else: nocursor -> no selection

    return bRet;
}

// --> OD 2005-12-20 #i27301# - new parameter <_bForSelection>
SwPaM* SwAccessibleParagraph::GetCursor( const bool _bForSelection )
{
    // get the cursor shell; if we don't have any, we don't have a
    // cursor/selection either
    SwPaM* pCrsr = NULL;
    SwCrsrShell* pCrsrShell = SwAccessibleParagraph::GetCrsrShell();
    // --> OD 2005-12-20 #i27301#
    // - if cursor is retrieved for selection, the cursors for a table selection
    //   has to be returned.
    if ( pCrsrShell != NULL &&
         ( _bForSelection || !pCrsrShell->IsTableMode() ) )
    // <--
    {
        SwFEShell *pFESh = pCrsrShell->ISA( SwFEShell )
                            ? static_cast< SwFEShell * >( pCrsrShell ) : 0;
        if( !pFESh ||
            !(pFESh->IsFrmSelected() || pFESh->IsObjSelected() > 0) )
        {
            // get the selection, and test whether it affects our text node
            pCrsr = pCrsrShell->GetCrsr( sal_False /* ??? */ );
        }
    }

    return pCrsr;
}

sal_Bool SwAccessibleParagraph::IsHeading() const
{
    const SwTxtNode *pTxtNd = GetTxtNode();
    return pTxtNd->IsOutline();
}

void SwAccessibleParagraph::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // MULTILINE
    rStateSet.AddState( AccessibleStateType::MULTI_LINE );

    // MULTISELECTABLE
    SwCrsrShell *pCrsrSh = GetCrsrShell();
    if( pCrsrSh )
        rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );

    // FOCUSABLE
    if( pCrsrSh )
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );

    // FOCUSED (simulates node index of cursor)
    // --> OD 2005-12-20 #i27301# - consider adjusted method signature
    SwPaM* pCaret = GetCursor( false );
    // <--
    const SwTxtNode* pTxtNd = GetTxtNode();
    if( pCaret != 0 && pTxtNd != 0 &&
        pTxtNd->GetIndex() == pCaret->GetPoint()->nNode.GetIndex() &&
        nOldCaretPos != -1)
    {
        Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() )
            rStateSet.AddState( AccessibleStateType::FOCUSED );
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }
}

void SwAccessibleParagraph::_InvalidateContent( sal_Bool bVisibleDataFired )
{
    ::rtl::OUString sOldText( GetString() );

    ClearPortionData();

    const ::rtl::OUString& rText = GetString();

    if( rText != sOldText )
    {
        // The text is changed
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::TEXT_CHANGED;

        // determine exact changes between sOldText and rText
        comphelper::OCommonAccessibleText::implInitTextChangedEvent(
            sOldText, rText,
            aEvent.OldValue, aEvent.NewValue );

        FireAccessibleEvent( aEvent );
        uno::Reference< XAccessible > xparent = getAccessibleParent();
        uno::Reference< XAccessibleContext > xAccContext(xparent,uno::UNO_QUERY);
        if (xAccContext.is() && xAccContext->getAccessibleRole() == AccessibleRole::TABLE_CELL)
        {
            SwAccessibleContext* pPara = static_cast< SwAccessibleContext* >(xparent.get());
            if(pPara)
            {
                AccessibleEventObject aParaEvent;
                aParaEvent.EventId = AccessibleEventId::VALUE_CHANGED;
                pPara->FireAccessibleEvent(aParaEvent);
            }
        }
    }
    else if( !bVisibleDataFired )
    {
        FireVisibleDataEvent();
    }

    sal_Bool bNewIsHeading = IsHeading();
    //Get the real heading level, Heading1 ~ Heading10
    nHeadingLevel = GetRealHeadingLevel();
    sal_Bool bOldIsHeading;
    {
        vos::OGuard aGuard( aMutex );
        bOldIsHeading = bIsHeading;
        if( bIsHeading != bNewIsHeading )
            bIsHeading = bNewIsHeading;
    }


    if( bNewIsHeading != bOldIsHeading || rText != sOldText )
    {
        ::rtl::OUString sNewDesc( GetDescription() );
        ::rtl::OUString sOldDesc;
        {
            vos::OGuard aGuard( aMutex );
            sOldDesc = sDesc;
            if( sDesc != sNewDesc )
                sDesc = sNewDesc;
        }

        if( sNewDesc != sOldDesc )
        {
            // The text is changed
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::DESCRIPTION_CHANGED;
            aEvent.OldValue <<= sOldDesc;
            aEvent.NewValue <<= sNewDesc;

            FireAccessibleEvent( aEvent );
        }
    }
}

void SwAccessibleParagraph::_InvalidateCursorPos()
{
    // The text is changed
    sal_Int32 nNew = GetCaretPos();
    sal_Int32 nOld;
    {
        vos::OGuard aGuard( aMutex );
        nOld = nOldCaretPos;
        nOldCaretPos = nNew;
    }
    if( -1 != nNew )
    {
        // remember that object as the one that has the caret. This is
        // neccessary to notify that object if the cursor leaves it.
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    Window *pWin = GetWindow();
    if( nOld != nNew )
    {
        // The cursor's node position is sumilated by the focus!
        if( pWin && pWin->HasFocus() && -1 == nOld )
            FireStateChangedEvent( AccessibleStateType::FOCUSED, sal_True );


        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CARET_CHANGED;
        aEvent.OldValue <<= nOld;
        aEvent.NewValue <<= nNew;

        FireAccessibleEvent( aEvent );

        if( pWin && pWin->HasFocus() && -1 == nNew )
            FireStateChangedEvent( AccessibleStateType::FOCUSED, sal_False );
        //To send TEXT_SELECTION_CHANGED event
        sal_Int32 nStart=0;
        sal_Int32 nEnd  =0;
        sal_Bool bCurSelection=GetSelection(nStart,nEnd);
        if(m_bLastHasSelection || bCurSelection )
        {
            aEvent.EventId = AccessibleEventId::TEXT_SELECTION_CHANGED;
            aEvent.OldValue <<= uno::Any();
            aEvent.NewValue <<= uno::Any();
            FireAccessibleEvent(aEvent);
        }
        m_bLastHasSelection =bCurSelection;
    }
}

void SwAccessibleParagraph::_InvalidateFocus()
{
    Window *pWin = GetWindow();
    if( pWin )
    {
        sal_Int32 nPos;
        {
            vos::OGuard aGuard( aMutex );
            nPos = nOldCaretPos;
        }
        ASSERT( nPos != -1, "focus object should be selected" );

        FireStateChangedEvent( AccessibleStateType::FOCUSED,
                               pWin->HasFocus() && nPos != -1 );
    }
}

SwAccessibleParagraph::SwAccessibleParagraph(
        SwAccessibleMap& rInitMap,
        const SwTxtFrm& rTxtFrm )
    // --> OD 2010-02-24 #i108125#
    : SwClient( const_cast<SwTxtNode*>(rTxtFrm.GetTxtNode()) )
    // <--
    , SwAccessibleContext( &rInitMap, AccessibleRole::PARAGRAPH, &rTxtFrm )
    , sDesc()
    , pPortionData( NULL )
    , pHyperTextData( NULL )
    , nOldCaretPos( -1 )
    , bIsHeading( sal_False )
    //Get the real heading level, Heading1 ~ Heading10
    , nHeadingLevel (-1)
    , aSelectionHelper( *this )
    // --> OD 2010-02-19 #i108125#
    , mpParaChangeTrackInfo( new SwParaChangeTrackingInfo( rTxtFrm ) )
    // <--
    , m_bLastHasSelection(false)  //To add TEXT_SELECTION_CHANGED event
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    bIsHeading = IsHeading();
    //Get the real heading level, Heading1 ~ Heading10
    nHeadingLevel = GetRealHeadingLevel();
    // --> OD 2004-09-27 #117970# - set an empty accessibility name for paragraphs
    SetName( ::rtl::OUString() );
    // <--

    // If this object has the focus, then it is remembered by the map itself.
    // not necessary to remember this pos here. Generally, the pos will be updated in invalidateXXX method, which may fire the
    //Focus event based on the difference of new & old caret pos.
    //nOldCaretPos = GetCaretPos();
}

SwAccessibleParagraph::~SwAccessibleParagraph()
{
    if(Application::GetUnoWrapper())
        vos::OGuard aGuard(Application::GetSolarMutex());

    delete pPortionData;
    delete pHyperTextData;
    // --> OD 2010-02-22 #i108125#
    delete mpParaChangeTrackInfo;
    // <--
}

sal_Bool SwAccessibleParagraph::HasCursor()
{
    vos::OGuard aGuard( aMutex );
    return nOldCaretPos != -1;
}

void SwAccessibleParagraph::UpdatePortionData()
    throw( uno::RuntimeException )
{
    // obtain the text frame
    DBG_ASSERT( GetFrm() != NULL, "The text frame has vanished!" );
    DBG_ASSERT( GetFrm()->IsTxtFrm(), "The text frame has mutated!" );
    const SwTxtFrm* pFrm = static_cast<const SwTxtFrm*>( GetFrm() );

    // build new portion data
    delete pPortionData;
    pPortionData = new SwAccessiblePortionData(
        pFrm->GetTxtNode(), GetMap()->GetShell()->GetViewOptions() );
    pFrm->VisitPortions( *pPortionData );

    DBG_ASSERT( pPortionData != NULL, "UpdatePortionData() failed" );
}

void SwAccessibleParagraph::ClearPortionData()
{
    delete pPortionData;
    pPortionData = NULL;

    delete pHyperTextData;
    pHyperTextData = 0;
}


void SwAccessibleParagraph::ExecuteAtViewShell( sal_uInt16 nSlot )
{
    DBG_ASSERT( GetMap() != NULL, "no map?" );
    ViewShell* pViewShell = GetMap()->GetShell();

    DBG_ASSERT( pViewShell != NULL, "View shell exptected!" );
    SfxViewShell* pSfxShell = pViewShell->GetSfxViewShell();

    DBG_ASSERT( pSfxShell != NULL, "SfxViewShell shell exptected!" );
    if( !pSfxShell )
        return;

    SfxViewFrame *pFrame = pSfxShell->GetViewFrame();
    DBG_ASSERT( pFrame != NULL, "View frame exptected!" );
    if( !pFrame )
        return;

    SfxDispatcher *pDispatcher = pFrame->GetDispatcher();
    DBG_ASSERT( pDispatcher != NULL, "Dispatcher exptected!" );
    if( !pDispatcher )
        return;

    pDispatcher->Execute( nSlot );
}

SwXTextPortion* SwAccessibleParagraph::CreateUnoPortion(
    sal_Int32 nStartIndex,
    sal_Int32 nEndIndex )
{
    DBG_ASSERT( (IsValidChar(nStartIndex, GetString().getLength()) &&
                 (nEndIndex == -1)) ||
                IsValidRange(nStartIndex, nEndIndex, GetString().getLength()),
                "please check parameters before calling this method" );

    sal_uInt16 nStart = GetPortionData().GetModelPosition( nStartIndex );
    sal_uInt16 nEnd = (nEndIndex == -1) ? (nStart + 1) :
                        GetPortionData().GetModelPosition( nEndIndex );

    // create UNO cursor
    SwTxtNode* pTxtNode = const_cast<SwTxtNode*>( GetTxtNode() );
    SwIndex aIndex( pTxtNode, nStart );
    SwPosition aStartPos( *pTxtNode, aIndex );
    SwUnoCrsr* pUnoCursor = pTxtNode->GetDoc()->CreateUnoCrsr( aStartPos );
    pUnoCursor->SetMark();
    pUnoCursor->GetMark()->nContent = nEnd;

    // create a (dummy) text portion to be returned
    uno::Reference<text::XText> aEmpty;
    SwXTextPortion* pPortion =
        new SwXTextPortion ( pUnoCursor, aEmpty, PORTION_TEXT);
    delete pUnoCursor;

    return pPortion;
}


//
// range checking for parameter
//

sal_Bool SwAccessibleParagraph::IsValidChar(
    sal_Int32 nPos, sal_Int32 nLength)
{
    return (nPos >= 0) && (nPos < nLength);
}

sal_Bool SwAccessibleParagraph::IsValidPosition(
    sal_Int32 nPos, sal_Int32 nLength)
{
    return (nPos >= 0) && (nPos <= nLength);
}

sal_Bool SwAccessibleParagraph::IsValidRange(
    sal_Int32 nBegin, sal_Int32 nEnd, sal_Int32 nLength)
{
    return IsValidPosition(nBegin, nLength) && IsValidPosition(nEnd, nLength);
}
SwTOXSortTabBase* SwAccessibleParagraph::GetTOXSortTabBase()
{
    const SwTxtNode* pTxtNd = GetTxtNode();
    if( pTxtNd )
    {
        const SwSectionNode * pSectNd = pTxtNd->FindSectionNode();
        if( pSectNd )
        {
            const SwSection * pSect = &pSectNd->GetSection();
            SwTOXBaseSection *pTOXBaseSect = (SwTOXBaseSection *)pSect;
            if( pSect->GetType() == TOX_CONTENT_SECTION )
            {
                SwTOXSortTabBase* pSortBase = 0;
                int nSize = pTOXBaseSect->GetTOXSortTabBases()->Count();

                for(int nIndex = 0; nIndex<nSize; nIndex++ )
                {
                    pSortBase = (*(pTOXBaseSect->GetTOXSortTabBases()))[nIndex];
                    if( pSortBase->pTOXNd == pTxtNd )
                        break;
                }

                if (pSortBase)
                {
                    return pSortBase;
                }
            }
        }
    }
    return NULL;
}

short SwAccessibleParagraph::GetTOCLevel()
{
    SwTOXSortTabBase* pToxBase = GetTOXSortTabBase();
    if( pToxBase )
    {
        const SwCntntNode*  pNd = pToxBase->aTOXSources[0].pNd;
        if( pNd )
            return pToxBase->GetLevel();
        else
            return -1;
    }
    else
        return -1;
}

//the function is to check whether the position is in a redline range.
const SwRedline* SwAccessibleParagraph::GetRedlineAtIndex( sal_Int32 )
{
    const SwRedline* pRedline = NULL;
    SwPaM* pCrSr = GetCursor( true );
    if ( pCrSr )
    {
        SwPosition* pStart = pCrSr->Start();
        const SwTxtNode* pNode = GetTxtNode();
        if ( pNode )
        {
            const SwDoc* pDoc = pNode->GetDoc();
            if ( pDoc )
            {
                pRedline = pDoc->GetRedline( *pStart, NULL );
            }
        }
    }

    return pRedline;
}

//
// text boundaries
//


sal_Bool SwAccessibleParagraph::GetCharBoundary(
    i18n::Boundary& rBound,
    const ::rtl::OUString&,
    sal_Int32 nPos )
{
    if( GetPortionData().FillBoundaryIFDateField( rBound,  nPos) )
        return sal_True;

    rBound.startPos = nPos;
    rBound.endPos = nPos+1;
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetWordBoundary(
    i18n::Boundary& rBound,
    const ::rtl::OUString& rText,
    sal_Int32 nPos )
{
    sal_Bool bRet = sal_False;

    // now ask the Break-Iterator for the word
    DBG_ASSERT( pBreakIt != NULL, "We always need a break." );
    DBG_ASSERT( pBreakIt->GetBreakIter().is(), "No break-iterator." );
    if( pBreakIt->GetBreakIter().is() )
    {
        // get locale for this position
        sal_uInt16 nModelPos = GetPortionData().GetModelPosition( nPos );
        lang::Locale aLocale = pBreakIt->GetLocale(
                              GetTxtNode()->GetLang( nModelPos ) );

        // which type of word are we interested in?
        // (DICTIONARY_WORD includes punctuation, ANY_WORD doesn't.)
        const sal_uInt16 nWordType = i18n::WordType::ANY_WORD;

/*
        // get word boundary, as the Break-Iterator sees fit.
        sal_Unicode SpaceChar(' ');
        if (rText.getCodePointAt(nPos) == SpaceChar)
        {
            int nStartPos = nPos;
            int nEndPos = nPos+1;
            while (nStartPos >= 0 && rText.getCodePointAt(nStartPos) == SpaceChar)
                --nStartPos;
            while (nEndPos < rText.getLength() && rText.getCodePointAt(nEndPos) == SpaceChar)
                ++nEndPos;
            //Get the previous word boundary + the followed space characters
            if (nStartPos >= 0)
            {
                rBound = pBreakIt->xBreak->getWordBoundary( rText, nStartPos, aLocale, nWordType, sal_True );
                rBound.endPos += (nEndPos-nStartPos - 1);
            }
            //When the frontal characters are whitespace, return the all space characters directly.
            else
            {
                rBound.startPos = 0;
                rBound.endPos = nEndPos;
            }
        }
        // add the " " into the word boundry
        else
        {
            rBound = pBreakIt->xBreak->getWordBoundary(rText, nPos, aLocale, nWordType, sal_True );
            sal_Int32 nEndPos = rBound.endPos, nLength = rText.getLength();
            while ( nEndPos < nLength && rText.getCodePointAt(nEndPos) == SpaceChar )
                nEndPos++;
            rBound.endPos = nEndPos;
        }
        tabCharInWord( nPos, rBound);
        if( GetPortionData().FillBoundaryIFDateField( rBound,  rBound.startPos) )
            return sal_True;
        return sal_True; // MT: So why do we need the return TRUE above???
*/
        // get word boundary, as the Break-Iterator sees fit.
        rBound = pBreakIt->GetBreakIter()->getWordBoundary(
            rText, nPos, aLocale, nWordType, sal_True );

        // It's a word if the first character is an alpha-numeric character.
        bRet = GetAppCharClass().isLetterNumeric(
            rText.getStr()[ rBound.startPos ] );
    }
    else
    {
        // no break Iterator -> no word
        rBound.startPos = nPos;
        rBound.endPos = nPos;
    }

    return bRet;
}

sal_Bool SwAccessibleParagraph::GetSentenceBoundary(
    i18n::Boundary& rBound,
    const ::rtl::OUString& rText,
    sal_Int32 nPos )
{
    const sal_Unicode* pStr = rText.getStr();
    if (pStr)
    {
        while( pStr[nPos] == sal_Unicode(' ') && nPos < rText.getLength())
            nPos++;
    }
    GetPortionData().GetSentenceBoundary( rBound, nPos );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetLineBoundary(
    i18n::Boundary& rBound,
    const ::rtl::OUString& rText,
    sal_Int32 nPos )
{
    if( rText.getLength() == nPos )
        GetPortionData().GetLastLineBoundary( rBound );
    else
        GetPortionData().GetLineBoundary( rBound, nPos );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetParagraphBoundary(
    i18n::Boundary& rBound,
    const ::rtl::OUString& rText,
    sal_Int32 )
{
    rBound.startPos = 0;
    rBound.endPos = rText.getLength();
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetAttributeBoundary(
    i18n::Boundary& rBound,
    const ::rtl::OUString&,
    sal_Int32 nPos )
{
    GetPortionData().GetAttributeBoundary( rBound, nPos );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::GetGlyphBoundary(
    i18n::Boundary& rBound,
    const ::rtl::OUString& rText,
    sal_Int32 nPos )
{
    sal_Bool bRet = sal_False;

    // ask the Break-Iterator for the glyph by moving one cell
    // forward, and then one cell back
    DBG_ASSERT( pBreakIt != NULL, "We always need a break." );
    DBG_ASSERT( pBreakIt->GetBreakIter().is(), "No break-iterator." );
    if( pBreakIt->GetBreakIter().is() )
    {
        // get locale for this position
        sal_uInt16 nModelPos = GetPortionData().GetModelPosition( nPos );
        lang::Locale aLocale = pBreakIt->GetLocale(
                              GetTxtNode()->GetLang( nModelPos ) );

        // get word boundary, as the Break-Iterator sees fit.
        const sal_uInt16 nIterMode = i18n::CharacterIteratorMode::SKIPCELL;
        sal_Int32 nDone = 0;
        rBound.endPos = pBreakIt->GetBreakIter()->nextCharacters(
             rText, nPos, aLocale, nIterMode, 1, nDone );
        rBound.startPos = pBreakIt->GetBreakIter()->previousCharacters(
             rText, rBound.endPos, aLocale, nIterMode, 1, nDone );

        bRet = ((rBound.startPos <= nPos) && (nPos <= rBound.endPos));
        DBG_ASSERT( rBound.startPos <= nPos, "start pos too high" );
        DBG_ASSERT( rBound.endPos >= nPos, "end pos too low" );
    }
    else
    {
        // no break Iterator -> no glyph
        rBound.startPos = nPos;
        rBound.endPos = nPos;
    }

    return bRet;
}


sal_Bool SwAccessibleParagraph::GetTextBoundary(
    i18n::Boundary& rBound,
    const ::rtl::OUString& rText,
    sal_Int32 nPos,
    sal_Int16 nTextType )
    throw (
        lang::IndexOutOfBoundsException,
        lang::IllegalArgumentException,
        uno::RuntimeException)
{
    // error checking
    if( !( AccessibleTextType::LINE == nTextType
                ? IsValidPosition( nPos, rText.getLength() )
                : IsValidChar( nPos, rText.getLength() ) ) )
        throw lang::IndexOutOfBoundsException();

    sal_Bool bRet;

    switch( nTextType )
    {
        case AccessibleTextType::WORD:
            bRet = GetWordBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::SENTENCE:
            bRet = GetSentenceBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::PARAGRAPH:
            bRet = GetParagraphBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::CHARACTER:
            bRet = GetCharBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::LINE:
            //Solve the problem of returning wrong LINE and PARAGRAPH
            if((nPos == rText.getLength()) && nPos > 0)
                bRet = GetLineBoundary( rBound, rText, nPos - 1);
            else
                bRet = GetLineBoundary( rBound, rText, nPos );
            break;

        case AccessibleTextType::ATTRIBUTE_RUN:
            bRet = GetAttributeBoundary( rBound, rText, nPos );
            if(bRet)
            {
                SwCrsrShell* pCrsrShell = GetCrsrShell();
                if( pCrsrShell != NULL && pCrsrShell->GetViewOptions() && pCrsrShell->GetViewOptions()->IsOnlineSpell())
                {
                    SwTxtNode* pTxtNode = const_cast<SwTxtNode*>( GetTxtNode() );
                    if(pTxtNode)
                    {
                        const SwWrongList* pWrongList = pTxtNode->GetWrong();
                        if( NULL != pWrongList )
                        {
                            xub_StrLen nBegin = nPos;
                            xub_StrLen nLen = 1;
                            const xub_StrLen nNext = pWrongList->NextWrong(nBegin);
                            xub_StrLen nLast;
                            xub_StrLen nWrongPos = pWrongList->GetWrongPos( nBegin );
                            if ( nWrongPos >= pWrongList->Count() ||
                                 ( nLast = pWrongList->Pos( nWrongPos ) ) >= nBegin )
                            {
                                nLast = nWrongPos
                                        ? pWrongList->Pos( --nWrongPos )
                                        : STRING_LEN;
                            }
                            if ( nBegin > pWrongList->GetBeginInv() &&
                                 ( nLast == STRING_LEN || nLast < pWrongList->GetEndInv() ) )
                            {
                                nLast = nBegin > pWrongList->GetEndInv()
                                        ? pWrongList->GetEndInv()
                                        : nBegin;
                            }
                            else if ( nLast < STRING_LEN )
                            {
                                nLast += pWrongList->Len( nWrongPos );
                            }
                            //
                            sal_Bool bIn = pWrongList->InWrongWord(nBegin,nLen); // && !pTxtNode->IsSymbol(nBegin) )
                            if(bIn)
                            {
                                rBound.startPos = max(nNext,(xub_StrLen)rBound.startPos);
                                rBound.endPos = min(xub_StrLen(nNext + nLen),(xub_StrLen)rBound.endPos);
                            }
                            else
                            {
                                if (STRING_LEN == nLast)//first
                                {
                                    rBound.endPos = min(nNext,(xub_StrLen)rBound.endPos);
                                }
                                else if(STRING_LEN == nNext)
                                {
                                    rBound.startPos = max(nLast,(xub_StrLen)rBound.startPos);
                                }
                                else
                                {
                                    rBound.startPos = max(nLast,(xub_StrLen)rBound.startPos);
                                    rBound.endPos = min(nNext,(xub_StrLen)rBound.endPos);
                                }
                            }
                        }
                    }
                }
            }
            break;

        case AccessibleTextType::GLYPH:
            bRet = GetGlyphBoundary( rBound, rText, nPos );
            break;

        default:
            throw lang::IllegalArgumentException( );
    }

    return bRet;
}

::rtl::OUString SAL_CALL SwAccessibleParagraph::getAccessibleDescription (void)
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    vos::OGuard aGuard2( aMutex );
    if( !sDesc.getLength() )
        sDesc = GetDescription();

    return sDesc;
}

lang::Locale SAL_CALL SwAccessibleParagraph::getLocale (void)
        throw (IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwTxtFrm *pTxtFrm = PTR_CAST( SwTxtFrm, GetFrm() );
    if( !pTxtFrm )
    {
        THROW_RUNTIME_EXCEPTION( XAccessibleContext, "internal error (no text frame)" );
    }

    const SwTxtNode *pTxtNd = pTxtFrm->GetTxtNode();
    lang::Locale aLoc( pBreakIt->GetLocale( pTxtNd->GetLang( 0 ) ) );

    return aLoc;
}

/** paragraphs are in relation CONTENT_FLOWS_FROM and/or CONTENT_FLOWS_TO

    OD 2005-12-02 #i27138#

    @author OD
*/
uno::Reference<XAccessibleRelationSet> SAL_CALL SwAccessibleParagraph::getAccessibleRelationSet()
    throw ( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleContext );

    utl::AccessibleRelationSetHelper* pHelper = new utl::AccessibleRelationSetHelper();

    const SwTxtFrm* pTxtFrm = dynamic_cast<const SwTxtFrm*>(GetFrm());
    ASSERT( pTxtFrm,
            "<SwAccessibleParagraph::getAccessibleRelationSet()> - missing text frame");
    if ( pTxtFrm )
    {
        const SwCntntFrm* pPrevCntFrm( pTxtFrm->FindPrevCnt( true ) );
        if ( pPrevCntFrm )
        {
            uno::Sequence< uno::Reference<XInterface> > aSequence(1);
            aSequence[0] = GetMap()->GetContext( pPrevCntFrm );
            AccessibleRelation aAccRel( AccessibleRelationType::CONTENT_FLOWS_FROM,
                                        aSequence );
            pHelper->AddRelation( aAccRel );
        }

        const SwCntntFrm* pNextCntFrm( pTxtFrm->FindNextCnt( true ) );
        if ( pNextCntFrm )
        {
            uno::Sequence< uno::Reference<XInterface> > aSequence(1);
            aSequence[0] = GetMap()->GetContext( pNextCntFrm );
            AccessibleRelation aAccRel( AccessibleRelationType::CONTENT_FLOWS_TO,
                                        aSequence );
            pHelper->AddRelation( aAccRel );
        }
    }

    return pHelper;
}

void SAL_CALL SwAccessibleParagraph::grabFocus()
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext );

    // get cursor shell
    SwCrsrShell *pCrsrSh = GetCrsrShell();
    // --> OD 2005-12-20 #i27301# - consider new method signature
    SwPaM *pCrsr = GetCursor( false );
    // <--
    const SwTxtFrm *pTxtFrm = static_cast<const SwTxtFrm*>( GetFrm() );
    const SwTxtNode* pTxtNd = pTxtFrm->GetTxtNode();

    if( pCrsrSh != 0 && pTxtNd != 0 &&
        ( pCrsr == 0 ||
           pCrsr->GetPoint()->nNode.GetIndex() != pTxtNd->GetIndex() ||
          !pTxtFrm->IsInside( pCrsr->GetPoint()->nContent.GetIndex()) ) )
    {
        // create pam for selection
        SwIndex aIndex( const_cast< SwTxtNode * >( pTxtNd ),
                        pTxtFrm->GetOfst() );
        SwPosition aStartPos( *pTxtNd, aIndex );
        SwPaM aPaM( aStartPos );

        // set PaM at cursor shell
        Select( aPaM );


    }

    /* ->#i13955# */
    Window * pWindow = GetWindow();

    if (pWindow != NULL)
        pWindow->GrabFocus();
    /* <-#i13955# */
}

// --> OD 2007-01-17 #i71385#
bool lcl_GetBackgroundColor( Color & rColor,
                             const SwFrm* pFrm,
                             SwCrsrShell* pCrsrSh )
{
    const SvxBrushItem* pBackgrdBrush = 0;
    const Color* pSectionTOXColor = 0;
    SwRect aDummyRect;
    if ( pFrm &&
         pFrm->GetBackgroundBrush( pBackgrdBrush, pSectionTOXColor, aDummyRect, false ) )
    {
        if ( pSectionTOXColor )
        {
            rColor = *pSectionTOXColor;
            return true;
        }
        else
        {
            rColor =  pBackgrdBrush->GetColor();
            return true;
        }
    }
    else if ( pCrsrSh )
    {
        rColor = pCrsrSh->Imp()->GetRetoucheColor();
        return true;
    }

    return false;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getForeground()
                                throw (uno::RuntimeException)
{
    Color aBackgroundCol;

    if ( lcl_GetBackgroundColor( aBackgroundCol, GetFrm(), GetCrsrShell() ) )
    {
        if ( aBackgroundCol.IsDark() )
        {
            return COL_WHITE;
        }
        else
        {
            return COL_BLACK;
        }
    }

    return SwAccessibleContext::getForeground();
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getBackground()
                                throw (uno::RuntimeException)
{
    Color aBackgroundCol;

    if ( lcl_GetBackgroundColor( aBackgroundCol, GetFrm(), GetCrsrShell() ) )
    {
        return aBackgroundCol.GetColor();
    }

    return SwAccessibleContext::getBackground();
}
// <--

::rtl::OUString SAL_CALL SwAccessibleParagraph::getImplementationName()
        throw( uno::RuntimeException )
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SAL_CALL SwAccessibleParagraph::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

uno::Sequence< ::rtl::OUString > SAL_CALL SwAccessibleParagraph::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    pArray[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

uno::Sequence< ::rtl::OUString > getAttributeNames()
{
    static uno::Sequence< ::rtl::OUString >* pNames = NULL;

    if( pNames == NULL )
    {
        // Add the font name to attribute list
        uno::Sequence< ::rtl::OUString >* pSeq = new uno::Sequence< ::rtl::OUString >( 13 );

        ::rtl::OUString* pStrings = pSeq->getArray();

        // sorted list of strings
        sal_Int32 i = 0;

#define STR(x) pStrings[i++] = OUString::createFromAscii(x)
        STR( GetPropName( UNO_NAME_CHAR_BACK_COLOR ).pName );
        STR( GetPropName( UNO_NAME_CHAR_COLOR ).pName );
           STR( GetPropName( UNO_NAME_CHAR_CONTOURED ).pName );
        STR( GetPropName( UNO_NAME_CHAR_EMPHASIS ).pName );
        STR( GetPropName( UNO_NAME_CHAR_ESCAPEMENT ).pName );
        STR( GetPropName( UNO_NAME_CHAR_FONT_NAME ).pName );
        STR( GetPropName( UNO_NAME_CHAR_HEIGHT ).pName );
        STR( GetPropName( UNO_NAME_CHAR_POSTURE ).pName );
        STR( GetPropName( UNO_NAME_CHAR_SHADOWED ).pName );
        STR( GetPropName( UNO_NAME_CHAR_STRIKEOUT ).pName );
        STR( GetPropName( UNO_NAME_CHAR_UNDERLINE ).pName );
        STR( GetPropName( UNO_NAME_CHAR_UNDERLINE_COLOR ).pName );
        STR( GetPropName( UNO_NAME_CHAR_WEIGHT ).pName );
#undef STR
        DBG_ASSERT( i == pSeq->getLength(), "Please adjust length" );
        if( i != pSeq->getLength() )
            pSeq->realloc( i );
        pNames = pSeq;
    }
    return *pNames;
}

uno::Sequence< ::rtl::OUString > getSupplementalAttributeNames()
{
    static uno::Sequence< ::rtl::OUString >* pNames = NULL;

    if( pNames == NULL )
    {
        uno::Sequence< ::rtl::OUString >* pSeq = new uno::Sequence< ::rtl::OUString >( 9 );

        ::rtl::OUString* pStrings = pSeq->getArray();

        // sorted list of strings
        sal_Int32 i = 0;

#define STR(x) pStrings[i++] = OUString::createFromAscii(x)
        STR( GetPropName( UNO_NAME_NUMBERING_LEVEL ).pName );
        STR( GetPropName( UNO_NAME_NUMBERING_RULES ).pName );
        STR( GetPropName( UNO_NAME_PARA_ADJUST ).pName );
        STR( GetPropName( UNO_NAME_PARA_BOTTOM_MARGIN ).pName );
        STR( GetPropName( UNO_NAME_PARA_FIRST_LINE_INDENT ).pName );
        STR( GetPropName( UNO_NAME_PARA_LEFT_MARGIN ).pName );
        STR( GetPropName( UNO_NAME_PARA_LINE_SPACING ).pName );
        STR( GetPropName( UNO_NAME_PARA_RIGHT_MARGIN ).pName );
        STR( GetPropName( UNO_NAME_TABSTOPS ).pName );
#undef STR
        DBG_ASSERT( i == pSeq->getLength(), "Please adjust length" );
        if( i != pSeq->getLength() )
            pSeq->realloc( i );
        pNames = pSeq;
    }
    return *pNames;
}
//
//=====  XInterface  =======================================================
//

uno::Any SwAccessibleParagraph::queryInterface( const uno::Type& rType )
    throw (uno::RuntimeException)
{
    uno::Any aRet;
    if ( rType == ::getCppuType((uno::Reference<XAccessibleText> *)0) )
    {
        uno::Reference<XAccessibleText> aAccText = (XAccessibleText *) *this; // resolve ambiguity
        aRet <<= aAccText;
    }
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleEditableText> *)0) )
    {
        uno::Reference<XAccessibleEditableText> aAccEditText = this;
        aRet <<= aAccEditText;
    }
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleSelection> *)0) )
    {
        uno::Reference<XAccessibleSelection> aAccSel = this;
        aRet <<= aAccSel;
    }
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleHypertext> *)0) )
    {
        uno::Reference<XAccessibleHypertext> aAccHyp = this;
        aRet <<= aAccHyp;
    }
    // --> OD 2006-07-13 #i63870#
    // add interface com::sun:star:accessibility::XAccessibleTextAttributes
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleTextAttributes> *)0) )
    {
        uno::Reference<XAccessibleTextAttributes> aAccTextAttr = this;
        aRet <<= aAccTextAttr;
    }
    // <--
    // --> OD 2008-06-10 #i89175#
    // add interface com::sun:star:accessibility::XAccessibleTextMarkup
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleTextMarkup> *)0) )
    {
        uno::Reference<XAccessibleTextMarkup> aAccTextMarkup = this;
        aRet <<= aAccTextMarkup;
    }
    // add interface com::sun:star:accessibility::XAccessibleMultiLineText
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleMultiLineText> *)0) )
    {
        uno::Reference<XAccessibleMultiLineText> aAccMultiLineText = this;
        aRet <<= aAccMultiLineText;
    }
    // <--
    //MSAA Extension Implementation in app  module
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleTextSelection> *)NULL) )
    {
        uno::Reference< com::sun::star::accessibility::XAccessibleTextSelection > aTextExtension = this;
        aRet <<= aTextExtension;
    }
    else if ( rType == ::getCppuType((uno::Reference<XAccessibleExtendedAttributes> *)NULL) )
    {
        uno::Reference<XAccessibleExtendedAttributes> xAttr = this;
        aRet <<= xAttr;
    }
    else
    {
        aRet = SwAccessibleContext::queryInterface(rType);
    }

    return aRet;
}

//====== XTypeProvider ====================================================
uno::Sequence< uno::Type > SAL_CALL SwAccessibleParagraph::getTypes() throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleContext::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    // --> OD 2006-07-13 #i63870#
    // add type accessibility::XAccessibleTextAttributes
    // --> OD 2008-06-10 #i89175#
    // add type accessibility::XAccessibleTextMarkup and accessibility::XAccessibleMultiLineText
    aTypes.realloc( nIndex + 6 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex++] = ::getCppuType( static_cast< uno::Reference< XAccessibleEditableText > * >( 0 ) );
    pTypes[nIndex++] = ::getCppuType( static_cast< uno::Reference< XAccessibleTextAttributes > * >( 0 ) );
    pTypes[nIndex++] = ::getCppuType( static_cast< uno::Reference< XAccessibleSelection > * >( 0 ) );
    pTypes[nIndex++] = ::getCppuType( static_cast< uno::Reference< XAccessibleTextMarkup > * >( 0 ) );
    pTypes[nIndex++] = ::getCppuType( static_cast< uno::Reference< XAccessibleMultiLineText > * >( 0 ) );
    pTypes[nIndex] = ::getCppuType( static_cast< uno::Reference< XAccessibleHypertext > * >( 0 ) );
    // <--

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleParagraph::getImplementationId()
        throw(uno::RuntimeException)
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


//
//=====  XAccesibleText  ===================================================
//

sal_Int32 SwAccessibleParagraph::getCaretPosition()
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nRet = GetCaretPos();
    {
        vos::OGuard aOldCaretPosGuard( aMutex );
        ASSERT( nRet == nOldCaretPos, "caret pos out of sync" );
        nOldCaretPos = nRet;
    }
    if( -1 != nRet )
    {
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    return nRet;
}

sal_Bool SAL_CALL SwAccessibleParagraph::setCaretPosition( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidPosition( nIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    sal_Bool bRet = sal_False;

    // get cursor shell
    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( pCrsrShell != NULL )
    {
        // create pam for selection
        SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );
        SwIndex aIndex( pNode, GetPortionData().GetModelPosition(nIndex));
        SwPosition aStartPos( *pNode, aIndex );
        SwPaM aPaM( aStartPos );

        // set PaM at cursor shell
        bRet = Select( aPaM );
    }

    return bRet;
}

sal_Unicode SwAccessibleParagraph::getCharacter( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    ::rtl::OUString sText( GetString() );

    // return character (if valid)
    if( IsValidChar(nIndex, sText.getLength() ) )
    {
        return sText.getStr()[nIndex];
    }
    else
        throw lang::IndexOutOfBoundsException();
}

com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop > SwAccessibleParagraph::GetCurrentTabStop( sal_Int32 nIndex  )
{
vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );



    /*  #i12332# The position after the string needs special treatment.
        IsValidChar -> IsValidPosition
    */
    if( ! (IsValidPosition( nIndex, GetString().getLength() ) ) )
        throw lang::IndexOutOfBoundsException();

    /*  #i12332#  */
    sal_Bool bBehindText = sal_False;
    if ( nIndex == GetString().getLength() )
        bBehindText = sal_True;

    // get model position & prepare GetCharRect() arguments
    SwCrsrMoveState aMoveState;
    aMoveState.bRealHeight = sal_True;
    aMoveState.bRealWidth = sal_True;
    SwSpecialPos aSpecialPos;
    SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );

    sal_uInt16 nPos = 0;

    /*  #i12332# FillSpecialPos does not accept nIndex ==
         GetString().getLength(). In that case nPos is set to the
         length of the string in the core. This way GetCharRect
         returns the rectangle for a cursor at the end of the
         paragraph. */
    if (bBehindText)
    {
        nPos = pNode->GetTxt().Len();
    }
    else
        nPos = GetPortionData().FillSpecialPos
            (nIndex, aSpecialPos, aMoveState.pSpecialPos );

    // call GetCharRect
    SwRect aCoreRect;
    SwIndex aIndex( pNode, nPos );
    SwPosition aPosition( *pNode, aIndex );
    GetFrm()->GetCharRect( aCoreRect, aPosition, &aMoveState );

    // already get the caret postion

    /*SwFrm* pTFrm = const_cast<SwFrm*>(GetFrm());
    com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop > tabs =
        pTFrm->GetTabStopInfo(aCoreRect.Left());*/

    com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop > tabs;
    const xub_StrLen nStrLen = GetTxtNode()->GetTxt().Len();
    if( nStrLen > 0 )
    {
        SwFrm* pTFrm = const_cast<SwFrm*>(GetFrm());
        tabs = pTFrm->GetTabStopInfo(aCoreRect.Left());
    }

    if( tabs.hasElements() )
    {
        // translate core coordinates into accessibility coordinates
        Window *pWin = GetWindow();
        CHECK_FOR_WINDOW( XAccessibleComponent, pWin );

        SwRect aTmpRect(0, 0, tabs[0].Position, 0);

        Rectangle aScreenRect( GetMap()->CoreToPixel( aTmpRect.SVRect() ));
        SwRect aFrmLogBounds( GetBounds( *(GetMap()) ) ); // twip rel to doc root

        Point aFrmPixPos( GetMap()->CoreToPixel( aFrmLogBounds.SVRect() ).TopLeft() );
        aScreenRect.Move( -aFrmPixPos.X(), -aFrmPixPos.Y() );

        tabs[0].Position = aScreenRect.GetWidth();
    }

    return tabs;
}

struct IndexCompare
{
    const PropertyValue* pValues;
    IndexCompare( const PropertyValue* pVals ) : pValues(pVals) {}
    bool operator() ( const sal_Int32& a, const sal_Int32& b ) const
    {
        return (pValues[a].Name < pValues[b].Name) ? true : false;
    }
};

String SwAccessibleParagraph::GetFieldTypeNameAtIndex(sal_Int32 nIndex)
{
    String strTypeName;
    SwFldMgr aMgr;
    SwTxtFld* pTxtFld = NULL;
    SwTxtNode* pTxtNd = const_cast<SwTxtNode*>( GetTxtNode() );
    SwIndex fldIndex( pTxtNd, nIndex );
    sal_Int32 nFldIndex = GetPortionData().GetFieldIndex(nIndex);
    if (nFldIndex >= 0)
    {
        const SwpHints* pSwpHints = GetTxtNode()->GetpSwpHints();
        if (pSwpHints)
        {
            const sal_uInt16  nSize = pSwpHints ? pSwpHints->Count() : 0;
            for( sal_uInt16 i = 0; i < nSize; ++i )
            {
                const SwTxtAttr* pHt = (*pSwpHints)[i];
                if ( ( pHt->Which() == RES_TXTATR_FIELD
                       || pHt->Which() == RES_TXTATR_ANNOTATION
                       || pHt->Which() == RES_TXTATR_INPUTFIELD )
                     && (nFldIndex-- == 0))
                {
                    pTxtFld = (SwTxtFld *)pHt;
                    break;
                }
                else if ( pHt->Which() == RES_TXTATR_REFMARK
                          && (nFldIndex-- == 0) )
                    strTypeName = String(OUString(RTL_CONSTASCII_USTRINGPARAM("set reference")));
            }
        }
    }
    if (pTxtFld)
    {
        const SwField* pField = (pTxtFld->GetFmtFld()).GetField();
        if (pField)
        {
            strTypeName = pField->GetTyp()->GetTypeStr(pField->GetTypeId());
            sal_uInt16 nWhich = pField->GetTyp()->Which();
            rtl::OUString sEntry;
            sal_Int32 subType = 0;
            switch (nWhich)
            {
            case RES_DOCSTATFLD:
                subType = ((SwDocStatField*)pField)->GetSubType();
                break;
            case RES_GETREFFLD:
                {
                    sal_uInt16 nSub = pField->GetSubType();
                    switch( nSub )
                    {
                    case REF_BOOKMARK:
                        {
                            const SwGetRefField* pRefFld = dynamic_cast<const SwGetRefField*>(pField);
                            if ( pRefFld && pRefFld->IsRefToHeadingCrossRefBookmark() )
                                sEntry = OUString(RTL_CONSTASCII_USTRINGPARAM("Headings"));
                            else if ( pRefFld && pRefFld->IsRefToNumItemCrossRefBookmark() )
                                sEntry = OUString(RTL_CONSTASCII_USTRINGPARAM("Numbered Paragraphs"));
                            else
                                sEntry = OUString(RTL_CONSTASCII_USTRINGPARAM("Bookmarks"));
                        }
                        break;
                    case REF_FOOTNOTE:
                        sEntry = OUString(RTL_CONSTASCII_USTRINGPARAM("Footnotes"));
                        break;
                    case REF_ENDNOTE:
                        sEntry = OUString(RTL_CONSTASCII_USTRINGPARAM("Endnotes"));
                        break;
                    case REF_SETREFATTR:
                        sEntry = OUString(RTL_CONSTASCII_USTRINGPARAM("Insert Reference"));
                        break;
                    case REF_SEQUENCEFLD:
                        sEntry = ((SwGetRefField*)pField)->GetSetRefName();
                        break;
                    }
                    //Get format string
                    strTypeName = sEntry;
                    // <pField->GetFormat() >= 0> is always true as <pField->GetFormat()> is unsigned
//                    if (pField->GetFormat() >= 0)
                    {
                        sEntry = aMgr.GetFormatStr( pField->GetTypeId(), pField->GetFormat() );
                        if (sEntry.getLength() > 0)
                        {
                            strTypeName.AppendAscii("-");
                            strTypeName += String(sEntry);
                        }
                    }
                }
                break;
            case RES_DATETIMEFLD:
                subType = ((SwDateTimeField*)pField)->GetSubType();
                break;
            case RES_JUMPEDITFLD:
                {
                    sal_uInt16 nFormat= pField->GetFormat();
                    sal_uInt16 nSize = aMgr.GetFormatCount(pField->GetTypeId(), sal_False);
                    if (nFormat < nSize)
                    {
                        sEntry = aMgr.GetFormatStr(pField->GetTypeId(), nFormat);
                        if (sEntry.getLength() > 0)
                        {
                            strTypeName.AppendAscii("-");
                            strTypeName += String(sEntry);
                        }
                    }
                }
                break;
            case RES_EXTUSERFLD:
                subType = ((SwExtUserField*)pField)->GetSubType();
                break;
            case RES_HIDDENTXTFLD:
            case RES_SETEXPFLD:
                {
                    sEntry = pField->GetTyp()->GetName();
                    if (sEntry.getLength() > 0)
                    {
                        strTypeName.AppendAscii("-");
                        strTypeName += String(sEntry);
                    }
                }
                break;
            case RES_DOCINFOFLD:
                subType = pField->GetSubType();
                subType &= 0x00ff;
                break;
            case RES_REFPAGESETFLD:
                {
                    SwRefPageSetField* pRPld = (SwRefPageSetField*)pField;
                    sal_Bool bOn = pRPld->IsOn();
                    strTypeName.AppendAscii("-");
                    if (bOn)
                        strTypeName += String(OUString(RTL_CONSTASCII_USTRINGPARAM("on")));
                    else
                        strTypeName += String(OUString(RTL_CONSTASCII_USTRINGPARAM("off")));
                }
                break;
            case RES_AUTHORFLD:
                {
                    strTypeName.AppendAscii("-");
                    strTypeName += aMgr.GetFormatStr(pField->GetTypeId(), pField->GetFormat() & 0xff);
                }
                break;
            }
            if (subType > 0 || (subType == 0 && (nWhich == RES_DOCINFOFLD || nWhich == RES_EXTUSERFLD || nWhich == RES_DOCSTATFLD)))
            {
                SvStringsDtor aLst;
                aMgr.GetSubTypes(pField->GetTypeId(), aLst);
                if (subType < aLst.Count())
                    sEntry = *aLst[subType];
                if (sEntry.getLength() > 0)
                {
                    if (nWhich == RES_DOCINFOFLD)
                    {
                        strTypeName = String(sEntry);
                        sal_uInt32 nSize = aMgr.GetFormatCount(pField->GetTypeId(), sal_False);
                        sal_uInt16 nExSub = pField->GetSubType() & 0xff00;
                        if (nSize > 0 && nExSub > 0)
                        {
                            //Get extra subtype string
                            strTypeName.AppendAscii("-");
                            sEntry = aMgr.GetFormatStr(pField->GetTypeId(), nExSub/0x0100-1);
                            strTypeName += String(sEntry);
                        }
                    }
                    else
                    {
                        strTypeName.AppendAscii("-");
                        strTypeName += String(sEntry);
                    }
                }
            }
        }
    }
    return strTypeName;
}
// --> OD 2006-07-20 #i63870#
// re-implement method on behalf of methods <_getDefaultAttributesImpl(..)> and
// <_getRunAttributesImpl(..)>
uno::Sequence<PropertyValue> SwAccessibleParagraph::getCharacterAttributes(
    sal_Int32 nIndex,
    const uno::Sequence< ::rtl::OUString >& aRequestedAttributes )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{

    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    const ::rtl::OUString& rText = GetString();

    if( ! IsValidChar( nIndex, rText.getLength()+1 ) )
        throw lang::IndexOutOfBoundsException();

    bool bSupplementalMode = false;
    uno::Sequence< ::rtl::OUString > aNames = aRequestedAttributes;
    if (aNames.getLength() == 0)
    {
        bSupplementalMode = true;
        aNames = getAttributeNames();
    }
    // retrieve default character attributes
    tAccParaPropValMap aDefAttrSeq;
    _getDefaultAttributesImpl( aNames, aDefAttrSeq, true );

    // retrieved run character attributes
    tAccParaPropValMap aRunAttrSeq;
    _getRunAttributesImpl( nIndex, aNames, aRunAttrSeq );

    // merge default and run attributes
    uno::Sequence< PropertyValue > aValues( aDefAttrSeq.size() );
    PropertyValue* pValues = aValues.getArray();
    sal_Int32 i = 0;
    for ( tAccParaPropValMap::const_iterator aDefIter = aDefAttrSeq.begin();
          aDefIter != aDefAttrSeq.end();
          ++aDefIter )
    {
        tAccParaPropValMap::const_iterator aRunIter =
                                        aRunAttrSeq.find( aDefIter->first );
        if ( aRunIter != aRunAttrSeq.end() )
        {
            pValues[i] = aRunIter->second;
        }
        else
        {
            pValues[i] = aDefIter->second;
        }
        ++i;
    }
    if( bSupplementalMode )
    {
        uno::Sequence< ::rtl::OUString > aSupplementalNames = aRequestedAttributes;
        if (aSupplementalNames.getLength() == 0)
            aSupplementalNames = getSupplementalAttributeNames();

        tAccParaPropValMap aSupplementalAttrSeq;
        _getSupplementalAttributesImpl( nIndex, aSupplementalNames, aSupplementalAttrSeq );

        aValues.realloc( aValues.getLength() + aSupplementalAttrSeq.size() );
        pValues = aValues.getArray();

        for ( tAccParaPropValMap::const_iterator aSupplementalIter = aSupplementalAttrSeq.begin();
            aSupplementalIter != aSupplementalAttrSeq.end();
            ++aSupplementalIter )
        {
            pValues[i] = aSupplementalIter->second;
            ++i;
        }

        _correctValues( nIndex, aValues );

        aValues.realloc( aValues.getLength() + 1 );

        pValues = aValues.getArray();

        const SwTxtNode* pTxtNode( GetTxtNode() );
        PropertyValue& rValue = pValues[aValues.getLength() - 1 ];
        rValue.Name = OUString::createFromAscii("NumberingPrefix");
        OUString sNumBullet = pTxtNode->GetNumString();
        rValue.Value <<= sNumBullet;
        rValue.Handle = -1;
        rValue.State = PropertyState_DIRECT_VALUE;

        String strTypeName = GetFieldTypeNameAtIndex(nIndex);
        if (strTypeName.Len() > 0)
        {
            aValues.realloc( aValues.getLength() + 1 );
            pValues = aValues.getArray();
            rValue = pValues[aValues.getLength() - 1];
            rValue.Name = OUString::createFromAscii("FieldType");
            rValue.Value <<= rtl::OUString(strTypeName.ToLowerAscii());
            rValue.Handle = -1;
            rValue.State = PropertyState_DIRECT_VALUE;
        }

        //sort property values
        // build sorted index array
        sal_Int32 nLength = aValues.getLength();
        const PropertyValue* pPairs = aValues.getConstArray();
        sal_Int32* pIndices = new sal_Int32[nLength];
        for( i = 0; i < nLength; i++ )
            pIndices[i] = i;
        sort( &pIndices[0], &pIndices[nLength], IndexCompare(pPairs) );
        // create sorted sequences accoring to index array
        uno::Sequence<PropertyValue> aNewValues( nLength );
        PropertyValue* pNewValues = aNewValues.getArray();
        for( i = 0; i < nLength; i++ )
        {
            pNewValues[i] = pPairs[pIndices[i]];
        }
        delete[] pIndices;
        return aNewValues;
    }

//    // create a (dummy) text portion for the sole purpose of calling
//    // getPropertyValues on it
//    Reference<XMultiPropertySet> xPortion = CreateUnoPortion( nIndex, nIndex + 1 );

//    // get values
//    Sequence<OUString> aNames = getAttributeNames();
//    sal_Int32 nLength = aNames.getLength();
//    Sequence<Any> aAnys( nLength );
//    aAnys = xPortion->getPropertyValues( aNames );

//    // copy names + anys into return sequence
//    Sequence<PropertyValue> aValues( aNames.getLength() );
//    const OUString* pNames = aNames.getConstArray();
//    const Any* pAnys = aAnys.getConstArray();
//    PropertyValue* pValues = aValues.getArray();
//    for( sal_Int32 i = 0; i < nLength; i++ )
//    {
//        PropertyValue& rValue = pValues[i];
//        rValue.Name = pNames[i];
//        rValue.Value = pAnys[i];
//        rValue.Handle = -1;                         // handle not supported
//        rValue.State = PropertyState_DIRECT_VALUE;  // states not supported
//    }

//    // adjust background color if we're in a gray portion
//    DBG_ASSERT( pValues[CHAR_BACK_COLOR_POS].Name.
//                equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("CharBackColor")),
//                "Please adjust CHAR_BACK_COLOR_POS constant." );
//    if( GetPortionData().IsInGrayPortion( nIndex ) )
//        pValues[CHAR_BACK_COLOR_POS].Value <<= SwViewOption::GetFieldShadingsColor().GetColor();

    return aValues;
}

// --> OD 2006-07-11 #i63870#
void SwAccessibleParagraph::_getDefaultAttributesImpl(
        const uno::Sequence< ::rtl::OUString >& aRequestedAttributes,
        tAccParaPropValMap& rDefAttrSeq,
        const bool bOnlyCharAttrs )
{
    // retrieve default attributes
    const SwTxtNode* pTxtNode( GetTxtNode() );
    ::boost::scoped_ptr<SfxItemSet> pSet;
    if ( !bOnlyCharAttrs )
    {
        pSet.reset( new SfxItemSet( const_cast<SwAttrPool&>(pTxtNode->GetDoc()->GetAttrPool()),
                               RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                               RES_PARATR_BEGIN, RES_PARATR_END - 1,
                               RES_FRMATR_BEGIN, RES_FRMATR_END - 1,
                               0 ) );
    }
    else
    {
        pSet.reset( new SfxItemSet( const_cast<SwAttrPool&>(pTxtNode->GetDoc()->GetAttrPool()),
                               RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                               0 ) );
    }
    // --> OD 2007-11-12 #i82637#
    // From the perspective of the a11y API the default character attributes
    // are the character attributes, which are set at the paragraph style
    // of the paragraph. The character attributes set at the automatic paragraph
    // style of the paragraph are treated as run attributes.
//    pTxtNode->SwCntntNode::GetAttr( *pSet );
    // get default paragraph attributes, if needed, and merge these into <pSet>
    if ( !bOnlyCharAttrs )
    {
        SfxItemSet aParaSet( const_cast<SwAttrPool&>(pTxtNode->GetDoc()->GetAttrPool()),
                             RES_PARATR_BEGIN, RES_PARATR_END - 1,
                             RES_FRMATR_BEGIN, RES_FRMATR_END - 1,
                             0 );
        pTxtNode->SwCntntNode::GetAttr( aParaSet );
        pSet->Put( aParaSet );
    }
    // get default character attributes and merge these into <pSet>
    ASSERT( pTxtNode->GetTxtColl(),
            "<SwAccessibleParagraph::_getDefaultAttributesImpl(..)> - missing paragraph style. Serious defect, please inform OD!" );
    if ( pTxtNode->GetTxtColl() )
    {
        SfxItemSet aCharSet( const_cast<SwAttrPool&>(pTxtNode->GetDoc()->GetAttrPool()),
                             RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                             0 );
        aCharSet.Put( pTxtNode->GetTxtColl()->GetAttrSet() );
        pSet->Put( aCharSet );
    }
    // <--

    // build-up sequence containing the run attributes <rDefAttrSeq>
    tAccParaPropValMap aDefAttrSeq;
    {
        const SfxItemPropertyMap* pPropMap =
                    aSwMapProvider.GetPropertySet( PROPERTY_MAP_TEXT_CURSOR )->getPropertyMap();
        PropertyEntryVector_t aPropertyEntries = pPropMap->getPropertyEntries();
        PropertyEntryVector_t::const_iterator aPropIt = aPropertyEntries.begin();
        while ( aPropIt != aPropertyEntries.end() )
        {
            const SfxPoolItem* pItem = pSet->GetItem( aPropIt->nWID );
            if ( pItem )
            {
                uno::Any aVal;
                pItem->QueryValue( aVal, aPropIt->nMemberId );

                PropertyValue rPropVal;
                rPropVal.Name = aPropIt->sName;
                rPropVal.Value = aVal;
                rPropVal.Handle = -1;
                rPropVal.State = beans::PropertyState_DEFAULT_VALUE;

                aDefAttrSeq[rPropVal.Name] = rPropVal;
            }
            ++aPropIt;
        }

        // --> OD 2007-01-15 #i72800#
        // add property value entry for the paragraph style
        if ( !bOnlyCharAttrs && pTxtNode->GetTxtColl() )
        {
            const ::rtl::OUString sParaStyleName =
                    ::rtl::OUString::createFromAscii(
                            GetPropName( UNO_NAME_PARA_STYLE_NAME ).pName );
            if ( aDefAttrSeq.find( sParaStyleName ) == aDefAttrSeq.end() )
            {
                PropertyValue rPropVal;
                rPropVal.Name = sParaStyleName;
                uno::Any aVal( uno::makeAny( ::rtl::OUString( pTxtNode->GetTxtColl()->GetName() ) ) );
                rPropVal.Value = aVal;
                rPropVal.Handle = -1;
                rPropVal.State = beans::PropertyState_DEFAULT_VALUE;

                aDefAttrSeq[rPropVal.Name] = rPropVal;
            }
        }
        // <--

        // --> OD 2007-01-15 #i73371#
        // resolve value text::WritingMode2::PAGE of property value entry WritingMode
        if ( !bOnlyCharAttrs && GetFrm() )
        {
            const ::rtl::OUString sWritingMode =
                    ::rtl::OUString::createFromAscii(
                            GetPropName( UNO_NAME_WRITING_MODE ).pName );
            tAccParaPropValMap::iterator aIter = aDefAttrSeq.find( sWritingMode );
            if ( aIter != aDefAttrSeq.end() )
            {
                PropertyValue rPropVal( aIter->second );
                sal_Int16 nVal = rPropVal.Value.get<sal_Int16>();
                if ( nVal == text::WritingMode2::PAGE )
                {
                    const SwFrm* pUpperFrm( GetFrm()->GetUpper() );
                    while ( pUpperFrm )
                    {
                        if ( pUpperFrm->GetType() &
                               ( FRM_PAGE | FRM_FLY | FRM_SECTION | FRM_TAB | FRM_CELL ) )
                        {
                            if ( pUpperFrm->IsVertical() )
                            {
                                nVal = text::WritingMode2::TB_RL;
                            }
                            else if ( pUpperFrm->IsRightToLeft() )
                            {
                                nVal = text::WritingMode2::RL_TB;
                            }
                            else
                            {
                                nVal = text::WritingMode2::LR_TB;
                            }
                            rPropVal.Value <<= nVal;
                            aDefAttrSeq[rPropVal.Name] = rPropVal;
                            break;
                        }

                        if ( dynamic_cast<const SwFlyFrm*>(pUpperFrm) )
                        {
                            pUpperFrm = dynamic_cast<const SwFlyFrm*>(pUpperFrm)->GetAnchorFrm();
                        }
                        else
                        {
                            pUpperFrm = pUpperFrm->GetUpper();
                        }
                    }
                }
            }
        }
        // <--
    }

    if ( aRequestedAttributes.getLength() == 0 )
    {
        rDefAttrSeq = aDefAttrSeq;
    }
    else
    {
        const ::rtl::OUString* pReqAttrs = aRequestedAttributes.getConstArray();
        const sal_Int32 nLength = aRequestedAttributes.getLength();
        for( sal_Int32 i = 0; i < nLength; ++i )
        {
            tAccParaPropValMap::const_iterator const aIter = aDefAttrSeq.find( pReqAttrs[i] );
            if ( aIter != aDefAttrSeq.end() )
            {
                rDefAttrSeq[ aIter->first ] = aIter->second;
            }
        }
    }
}

uno::Sequence< PropertyValue > SwAccessibleParagraph::getDefaultAttributes(
        const uno::Sequence< ::rtl::OUString >& aRequestedAttributes )
        throw ( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    tAccParaPropValMap aDefAttrSeq;
    _getDefaultAttributesImpl( aRequestedAttributes, aDefAttrSeq );

    // --> OD 2010-03-08 #i92233#
    static rtl::OUString sMMToPixelRatio( rtl::OUString::createFromAscii( "MMToPixelRatio" ) );
    bool bProvideMMToPixelRatio( false );
    {
        if ( aRequestedAttributes.getLength() == 0 )
        {
            bProvideMMToPixelRatio = true;
        }
        else
        {
            const rtl::OUString* aRequestedAttrIter =
                  ::std::find( ::comphelper::stl_begin( aRequestedAttributes ),
                               ::comphelper::stl_end( aRequestedAttributes ),
                               sMMToPixelRatio );
            if ( aRequestedAttrIter != ::comphelper::stl_end( aRequestedAttributes ) )
            {
                bProvideMMToPixelRatio = true;
            }
        }
    }
    // <--

    uno::Sequence< PropertyValue > aValues( aDefAttrSeq.size() +
                                            ( bProvideMMToPixelRatio ? 1 : 0 ) );
    PropertyValue* pValues = aValues.getArray();
    sal_Int32 i = 0;
    for ( tAccParaPropValMap::const_iterator aIter  = aDefAttrSeq.begin();
          aIter != aDefAttrSeq.end();
          ++aIter )
    {
        pValues[i] = aIter->second;
        ++i;
    }

    // --> OD 2010-03-08 #i92233#
    if ( bProvideMMToPixelRatio )
    {
        PropertyValue rPropVal;
        rPropVal.Name = sMMToPixelRatio;
        const Size a100thMMSize( 1000, 1000 );
        const Size aPixelSize = GetMap()->LogicToPixel( a100thMMSize );
        const float fRatio = ((float)a100thMMSize.Width()/100)/aPixelSize.Width();
        rPropVal.Value = uno::makeAny( fRatio );
        rPropVal.Handle = -1;
        rPropVal.State = beans::PropertyState_DEFAULT_VALUE;
        pValues[ aValues.getLength() - 1 ] = rPropVal;
    }
    // <--

    return aValues;
}

void SwAccessibleParagraph::_getRunAttributesImpl(
        const sal_Int32 nIndex,
        const uno::Sequence< ::rtl::OUString >& aRequestedAttributes,
        tAccParaPropValMap& rRunAttrSeq )
{
    // create PaM for character at position <nIndex>
    SwPaM* pPaM( 0 );
    {
        const SwTxtNode* pTxtNode( GetTxtNode() );
        SwPosition* pStartPos = new SwPosition( *pTxtNode );
        pStartPos->nContent.Assign( const_cast<SwTxtNode*>(pTxtNode), static_cast<sal_uInt16>(nIndex) );
        SwPosition* pEndPos = new SwPosition( *pTxtNode );
        pEndPos->nContent.Assign( const_cast<SwTxtNode*>(pTxtNode), static_cast<sal_uInt16>(nIndex+1) );

        pPaM = new SwPaM( *pStartPos, *pEndPos );

        delete pStartPos;
        delete pEndPos;
    }

    // retrieve character attributes for the created PaM <pPaM>
    SfxItemSet aSet( pPaM->GetDoc()->GetAttrPool(),
                     RES_CHRATR_BEGIN, RES_CHRATR_END -1,
                     0 );
    // --> OD 2007-11-12 #i82637#
    // From the perspective of the a11y API the character attributes, which
    // are set at the automatic paragraph style of the paragraph are treated
    // as run attributes.
//    SwXTextCursor::GetCrsrAttr( *pPaM, aSet, sal_True, sal_True );
    // get character attributes from automatic paragraph style and merge these into <aSet>
    {
        const SwTxtNode* pTxtNode( GetTxtNode() );
        if ( pTxtNode->HasSwAttrSet() )
        {
            SfxItemSet aAutomaticParaStyleCharAttrs( pPaM->GetDoc()->GetAttrPool(),
                                                     RES_CHRATR_BEGIN, RES_CHRATR_END -1,
                                                     0 );
            aAutomaticParaStyleCharAttrs.Put( *(pTxtNode->GetpSwAttrSet()), sal_False );
            aSet.Put( aAutomaticParaStyleCharAttrs );
        }
    }
    // get character attributes at <pPaM> and merge these into <aSet>
    {
        SfxItemSet aCharAttrsAtPaM( pPaM->GetDoc()->GetAttrPool(),
                                    RES_CHRATR_BEGIN, RES_CHRATR_END -1,
                                    0 );
        SwUnoCursorHelper::GetCrsrAttr(*pPaM, aCharAttrsAtPaM, sal_True, sal_True);
        aSet.Put( aCharAttrsAtPaM );
    }
    // <--

    // build-up sequence containing the run attributes <rRunAttrSeq>
    {
        tAccParaPropValMap aRunAttrSeq;
        {
            // --> OD 2007-11-12 #i82637#
            tAccParaPropValMap aDefAttrSeq;
            uno::Sequence< ::rtl::OUString > aDummy;
            _getDefaultAttributesImpl( aDummy, aDefAttrSeq, true );
            // <--

            const SfxItemPropertyMap* pPropMap =
                    aSwMapProvider.GetPropertySet( PROPERTY_MAP_TEXT_CURSOR )->getPropertyMap();
            PropertyEntryVector_t aPropertyEntries = pPropMap->getPropertyEntries();
            PropertyEntryVector_t::const_iterator aPropIt = aPropertyEntries.begin();
            while ( aPropIt != aPropertyEntries.end() )
            {
                const SfxPoolItem* pItem( 0 );
                // --> OD 2007-11-12 #i82637#
                // Found character attributes, whose value equals the value of
                // the corresponding default character attributes, are excluded.
                if ( aSet.GetItemState( aPropIt->nWID, sal_True, &pItem ) == SFX_ITEM_SET )
                {
                    uno::Any aVal;
                    pItem->QueryValue( aVal, aPropIt->nMemberId );

                    PropertyValue rPropVal;
                    rPropVal.Name = aPropIt->sName;
                    rPropVal.Value = aVal;
                    rPropVal.Handle = -1;
                    rPropVal.State = PropertyState_DIRECT_VALUE;

                    tAccParaPropValMap::const_iterator aDefIter =
                                            aDefAttrSeq.find( rPropVal.Name );
                    if ( aDefIter == aDefAttrSeq.end() ||
                         rPropVal.Value != aDefIter->second.Value )
                    {
                        aRunAttrSeq[rPropVal.Name] = rPropVal;
                    }
                }

                ++aPropIt;
            }
        }

        if ( aRequestedAttributes.getLength() == 0 )
        {
            rRunAttrSeq = aRunAttrSeq;
        }
        else
        {
            const ::rtl::OUString* pReqAttrs = aRequestedAttributes.getConstArray();
            const sal_Int32 nLength = aRequestedAttributes.getLength();
            for( sal_Int32 i = 0; i < nLength; ++i )
            {
                tAccParaPropValMap::iterator aIter = aRunAttrSeq.find( pReqAttrs[i] );
                if ( aIter != aRunAttrSeq.end() )
                {
                    rRunAttrSeq[ (*aIter).first ] = (*aIter).second;
                }
            }
        }
    }

    delete pPaM;
}

uno::Sequence< PropertyValue > SwAccessibleParagraph::getRunAttributes(
        sal_Int32 nIndex,
        const uno::Sequence< ::rtl::OUString >& aRequestedAttributes )
        throw ( lang::IndexOutOfBoundsException,
                uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    {
        const ::rtl::OUString& rText = GetString();
        if ( !IsValidChar( nIndex, rText.getLength() ) )
        {
            throw lang::IndexOutOfBoundsException();
        }
    }

    tAccParaPropValMap aRunAttrSeq;
    _getRunAttributesImpl( nIndex, aRequestedAttributes, aRunAttrSeq );

    uno::Sequence< PropertyValue > aValues( aRunAttrSeq.size() );
    PropertyValue* pValues = aValues.getArray();
    sal_Int32 i = 0;
    for ( tAccParaPropValMap::const_iterator aIter  = aRunAttrSeq.begin();
          aIter != aRunAttrSeq.end();
          ++aIter )
    {
        pValues[i] = aIter->second;
        ++i;
    }

    return aValues;
}
// <--
void SwAccessibleParagraph::_getSupplementalAttributesImpl(
        const sal_Int32,
        const uno::Sequence< ::rtl::OUString >& aRequestedAttributes,
        tAccParaPropValMap& rSupplementalAttrSeq )
{
    const SwTxtNode* pTxtNode( GetTxtNode() );
    ::boost::scoped_ptr<SfxItemSet> pSet;
    pSet.reset( new SfxItemSet( const_cast<SwAttrPool&>(pTxtNode->GetDoc()->GetAttrPool()),
        RES_PARATR_ADJUST, RES_PARATR_ADJUST,
        RES_PARATR_TABSTOP, RES_PARATR_TABSTOP,
        RES_PARATR_LINESPACING, RES_PARATR_LINESPACING,
        RES_UL_SPACE, RES_UL_SPACE,
        RES_LR_SPACE, RES_LR_SPACE,
        RES_PARATR_NUMRULE, RES_PARATR_NUMRULE,
        RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
        0 ) );

    if ( pTxtNode->HasBullet() || pTxtNode->HasNumber() )
    {
        pSet->Put( pTxtNode->GetAttr(RES_PARATR_LIST_LEVEL, RES_PARATR_LIST_LEVEL) );
    }
    pSet->Put( pTxtNode->SwCntntNode::GetAttr(RES_UL_SPACE) );
    pSet->Put( pTxtNode->SwCntntNode::GetAttr(RES_LR_SPACE) );
    pSet->Put( pTxtNode->SwCntntNode::GetAttr(RES_PARATR_ADJUST) );

    tAccParaPropValMap aSupplementalAttrSeq;
    {
//        const SfxItemPropertySet& rPropSet =
//                    aSwMapProvider.GetPropertyMap( PROPERTY_MAP_ACCESSIBILITY_TEXT_ATTRIBUTE );
//        const SfxItemPropertyMap* pPropMap( rPropSet.getPropertyMap() );
        const SfxItemPropertyMapEntry* pPropMap(
                aSwMapProvider.GetPropertyMapEntries( PROPERTY_MAP_ACCESSIBILITY_TEXT_ATTRIBUTE ) );
        while ( pPropMap->pName )
        {
            const SfxPoolItem* pItem = pSet->GetItem( pPropMap->nWID );
            if ( pItem )
            {
                uno::Any aVal;
                pItem->QueryValue( aVal, pPropMap->nMemberId );

                PropertyValue rPropVal;
                rPropVal.Name = OUString::createFromAscii( pPropMap->pName );
                rPropVal.Value = aVal;
                rPropVal.Handle = -1;
                rPropVal.State = beans::PropertyState_DEFAULT_VALUE;

                aSupplementalAttrSeq[rPropVal.Name] = rPropVal;
            }

            ++pPropMap;
        }
    }

    const OUString* pSupplementalAttrs = aRequestedAttributes.getConstArray();
    const sal_Int32 nSupplementalLength = aRequestedAttributes.getLength();

    for( sal_Int32 index = 0; index < nSupplementalLength; ++index )
    {
        tAccParaPropValMap::const_iterator const aIter = aSupplementalAttrSeq.find( pSupplementalAttrs[index] );
        if ( aIter != aSupplementalAttrSeq.end() )
        {
            rSupplementalAttrSeq[ aIter->first ] = aIter->second;
        }
    }
}

void SwAccessibleParagraph::_correctValues( const sal_Int32 nIndex,
                                           uno::Sequence< PropertyValue >& rValues)
{
    PropertyValue ChangeAttr, ChangeAttrColor;

    const SwRedline* pRedline = GetRedlineAtIndex( nIndex );
    if ( pRedline )
    {

        const SwModuleOptions *pOpt = SW_MOD()->GetModuleConfig();
        AuthorCharAttr aChangeAttr;
        if ( pOpt )
        {
            switch( pRedline->GetType())
            {
            case nsRedlineType_t::REDLINE_INSERT:
                aChangeAttr = pOpt->GetInsertAuthorAttr();
                break;
            case nsRedlineType_t::REDLINE_DELETE:
                aChangeAttr = pOpt->GetDeletedAuthorAttr();
                break;
            case nsRedlineType_t::REDLINE_FORMAT:
                aChangeAttr = pOpt->GetFormatAuthorAttr();
                break;
            }
        }
        switch( aChangeAttr.nItemId )
        {
        case SID_ATTR_CHAR_WEIGHT:
            ChangeAttr.Name = OUString::createFromAscii( GetPropName( UNO_NAME_CHAR_WEIGHT).pName );
            ChangeAttr.Value <<= awt::FontWeight::BOLD;
            break;
        case SID_ATTR_CHAR_POSTURE:
            ChangeAttr.Name = OUString::createFromAscii( GetPropName( UNO_NAME_CHAR_POSTURE).pName );
            ChangeAttr.Value <<= awt::FontSlant_ITALIC; //char posture
            break;
        case SID_ATTR_CHAR_STRIKEOUT:
            ChangeAttr.Name = OUString::createFromAscii( GetPropName( UNO_NAME_CHAR_STRIKEOUT).pName );
            ChangeAttr.Value <<= awt::FontStrikeout::SINGLE; //char strikeout
            break;
        case SID_ATTR_CHAR_UNDERLINE:
            ChangeAttr.Name = OUString::createFromAscii( GetPropName( UNO_NAME_CHAR_UNDERLINE).pName );
            ChangeAttr.Value <<= aChangeAttr.nAttr; //underline line
            break;
        }
        if( aChangeAttr.nColor != COL_NONE )
        {
            if( aChangeAttr.nItemId == SID_ATTR_BRUSH )
            {
                ChangeAttrColor.Name = OUString::createFromAscii( GetPropName( UNO_NAME_CHAR_BACK_COLOR).pName );
                if( aChangeAttr.nColor == COL_TRANSPARENT )//char backcolor
                    ChangeAttrColor.Value <<= COL_BLUE;
                else
                    ChangeAttrColor.Value <<= aChangeAttr.nColor;
            }
            else
            {
                ChangeAttrColor.Name = OUString::createFromAscii( GetPropName( UNO_NAME_CHAR_COLOR ).pName );
                if( aChangeAttr.nColor == COL_TRANSPARENT )//char color
                    ChangeAttrColor.Value <<= COL_BLUE;
                else
                    ChangeAttrColor.Value <<= aChangeAttr.nColor;
            }
        }
    }

    PropertyValue* pValues = rValues.getArray();

    const SwTxtNode* pTxtNode( GetTxtNode() );

    sal_Int32 nValues = rValues.getLength();
    for (sal_Int32 i = 0;  i < nValues;  ++i)
    {
        PropertyValue& rValue = pValues[i];

        if (rValue.Name.compareTo( ChangeAttr.Name )==0)
        {
            rValue.Value = ChangeAttr.Value;
            continue;
        }

        if (rValue.Name.compareTo( ChangeAttrColor.Name )==0)
        {
            rValue.Value = ChangeAttr.Value;
            continue;
        }

        //back color
        if (rValue.Name.compareTo(::rtl::OUString::createFromAscii( GetPropName( UNO_NAME_CHAR_BACK_COLOR ).pName ) )==0)
        {
            uno::Any &anyChar = rValue.Value;
            sal_uInt32 crBack = static_cast<sal_uInt32>( reinterpret_cast<sal_uIntPtr>(anyChar.pReserved));
            if (COL_AUTO == crBack)
            {
                uno::Reference<XAccessibleComponent> xComponent(this);
                if (xComponent.is())
                {
                    crBack = (sal_uInt32)xComponent->getBackground();
                }
                rValue.Value <<= crBack;
            }
            continue;
        }

        //char color
        if (rValue.Name.compareTo(::rtl::OUString::createFromAscii( GetPropName( UNO_NAME_CHAR_COLOR ).pName ) )==0)
        {
            if( GetPortionData().IsInGrayPortion( nIndex ) )
                 rValue.Value <<= SwViewOption::GetFieldShadingsColor().GetColor();
            uno::Any &anyChar = rValue.Value;
            sal_uInt32 crChar = static_cast<sal_uInt32>( reinterpret_cast<sal_uIntPtr>(anyChar.pReserved));

            if( COL_AUTO == crChar )
            {
                uno::Reference<XAccessibleComponent> xComponent(this);
                if (xComponent.is())
                {
                    Color cr(xComponent->getBackground());
                    crChar = cr.IsDark() ? COL_WHITE : COL_BLACK;
                    rValue.Value <<= crChar;
                }
            }
            continue;
        }

        // UnderLine
        if (rValue.Name.compareTo(::rtl::OUString::createFromAscii( GetPropName( UNO_NAME_CHAR_UNDERLINE ).pName ) )==0)
        {
            //misspelled word
            SwCrsrShell* pCrsrShell = GetCrsrShell();
            if( pCrsrShell != NULL && pCrsrShell->GetViewOptions() && pCrsrShell->GetViewOptions()->IsOnlineSpell())
            {
                const SwWrongList* pWrongList = pTxtNode->GetWrong();
                if( NULL != pWrongList )
                {
                    xub_StrLen nBegin = nIndex;
                    xub_StrLen nLen = 1;
                    if( pWrongList->InWrongWord(nBegin,nLen) && !pTxtNode->IsSymbol(nBegin) )
                    {
                        rValue.Value <<= (sal_uInt16)UNDERLINE_WAVE;
                    }
                }
            }
            continue;
        }

        // UnderLineColor
        if (rValue.Name.compareTo(::rtl::OUString::createFromAscii( GetPropName( UNO_NAME_CHAR_UNDERLINE_COLOR ).pName ) )==0)
        {
            //misspelled word
            SwCrsrShell* pCrsrShell = GetCrsrShell();
            if( pCrsrShell != NULL && pCrsrShell->GetViewOptions() && pCrsrShell->GetViewOptions()->IsOnlineSpell())
            {
                const SwWrongList* pWrongList = pTxtNode->GetWrong();
                if( NULL != pWrongList )
                {
                    xub_StrLen nBegin = nIndex;
                    xub_StrLen nLen = 1;
                    if( pWrongList->InWrongWord(nBegin,nLen) && !pTxtNode->IsSymbol(nBegin) )
                    {
                        rValue.Value <<= (sal_Int32)0x00ff0000;
                        continue;
                    }
                }
            }

            uno::Any &anyChar = rValue.Value;
            sal_uInt32 crUnderline = static_cast<sal_uInt32>( reinterpret_cast<sal_uIntPtr>(anyChar.pReserved));
            if ( COL_AUTO == crUnderline )
            {
                uno::Reference<XAccessibleComponent> xComponent(this);
                if (xComponent.is())
                {
                    Color cr(xComponent->getBackground());
                    crUnderline = cr.IsDark() ? COL_WHITE : COL_BLACK;
                    rValue.Value <<= crUnderline;
                }
            }

            continue;
        }

        //tab stop
        if (rValue.Name.compareTo(::rtl::OUString::createFromAscii( GetPropName( UNO_NAME_TABSTOPS ).pName ) )==0)
        {
            com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop > tabs = GetCurrentTabStop( nIndex );
            if( !tabs.hasElements() )
            {
                tabs.realloc(1);
                ::com::sun::star::style::TabStop ts;
                com::sun::star::awt::Rectangle rc0 = getCharacterBounds(0);
                com::sun::star::awt::Rectangle rc1 = getCharacterBounds(nIndex);
                if( rc1.X - rc0.X >= 48 )
                    ts.Position = (rc1.X - rc0.X) - (rc1.X - rc0.X - 48)% 47 + 47;
                else
                    ts.Position = 48;
                ts.DecimalChar = ' ';
                ts.FillChar = ' ';
                ts.Alignment = ::com::sun::star::style::TabAlign_LEFT;
                tabs[0] = ts;
            }
            rValue.Value <<= tabs;
            continue;
        }

        //number bullet
        if (rValue.Name.compareTo(::rtl::OUString::createFromAscii( GetPropName( UNO_NAME_NUMBERING_RULES ).pName ) )==0)
        {
            if ( pTxtNode->HasBullet() || pTxtNode->HasNumber() )
            {
                uno::Any aVal;
                SwNumRule* pNumRule = pTxtNode->GetNumRule();
                if (pNumRule)
                {
                    uno::Reference< container::XIndexReplace >  xNum = new SwXNumberingRules(*pNumRule);
                    aVal.setValue(&xNum, ::getCppuType((const uno::Reference< container::XIndexReplace >*)0));
                }
                rValue.Value <<= aVal;
            }
            continue;
        }

        //footnote & endnote
        if (rValue.Name.compareTo(::rtl::OUString::createFromAscii( GetPropName( UNO_NAME_CHAR_ESCAPEMENT ).pName ) )==0)
        {
            if ( GetPortionData().IsIndexInFootnode(nIndex) )
            {
                const OUString sEscapmentName = OUString::createFromAscii( GetPropName( UNO_NAME_CHAR_ESCAPEMENT ).pName );
                rValue.Value <<= (sal_Int32)101;
            }
            continue;
        }
    }
}

awt::Rectangle SwAccessibleParagraph::getCharacterBounds(
    sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );


    /*  #i12332# The position after the string needs special treatment.
        IsValidChar -> IsValidPosition
    */
    if( ! (IsValidPosition( nIndex, GetString().getLength() ) ) )
        throw lang::IndexOutOfBoundsException();

    /*  #i12332#  */
    sal_Bool bBehindText = sal_False;
    if ( nIndex == GetString().getLength() )
        bBehindText = sal_True;

    // get model position & prepare GetCharRect() arguments
    SwCrsrMoveState aMoveState;
    aMoveState.bRealHeight = sal_True;
    aMoveState.bRealWidth = sal_True;
    SwSpecialPos aSpecialPos;
    SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );

    sal_uInt16 nPos = 0;

    /*  #i12332# FillSpecialPos does not accept nIndex ==
         GetString().getLength(). In that case nPos is set to the
         length of the string in the core. This way GetCharRect
         returns the rectangle for a cursor at the end of the
         paragraph. */
    if (bBehindText)
    {
        nPos = pNode->GetTxt().Len();
    }
    else
        nPos = GetPortionData().FillSpecialPos
            (nIndex, aSpecialPos, aMoveState.pSpecialPos );

    // call GetCharRect
    SwRect aCoreRect;
    SwIndex aIndex( pNode, nPos );
    SwPosition aPosition( *pNode, aIndex );
    GetFrm()->GetCharRect( aCoreRect, aPosition, &aMoveState );

    // translate core coordinates into accessibility coordinates
    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin );

    Rectangle aScreenRect( GetMap()->CoreToPixel( aCoreRect.SVRect() ));
    SwRect aFrmLogBounds( GetBounds( *(GetMap()) ) ); // twip rel to doc root

    Point aFrmPixPos( GetMap()->CoreToPixel( aFrmLogBounds.SVRect() ).TopLeft() );
    aScreenRect.Move( -aFrmPixPos.X(), -aFrmPixPos.Y() );

    // convert into AWT Rectangle
    return awt::Rectangle(
        aScreenRect.Left(), aScreenRect.Top(),
        aScreenRect.GetWidth(), aScreenRect.GetHeight() );
}

sal_Int32 SwAccessibleParagraph::getCharacterCount()
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    return GetString().getLength();
}

sal_Int32 SwAccessibleParagraph::getIndexAtPoint( const awt::Point& rPoint )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());


    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // construct SwPosition (where GetCrsrOfst() will put the result into)
    SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );
    SwIndex aIndex( pNode, 0);
    SwPosition aPos( *pNode, aIndex );

    // construct Point (translate into layout coordinates)
    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin );
    Point aPoint( rPoint.X, rPoint.Y );
    SwRect aLogBounds( GetBounds( *(GetMap()), GetFrm() ) ); // twip rel to doc root
    Point aPixPos( GetMap()->CoreToPixel( aLogBounds.SVRect() ).TopLeft() );
    aPoint.X() += aPixPos.X();
    aPoint.Y() += aPixPos.Y();
    MapMode aMapMode = pWin->GetMapMode();
    Point aCorePoint( GetMap()->PixelToCore( aPoint ) );
    if( !aLogBounds.IsInside( aCorePoint ) )
    {
        /* #i12332# rPoint is may also be in rectangle returned by
            getCharacterBounds(getCharacterCount() */

        awt::Rectangle aRectEndPos =
            getCharacterBounds(getCharacterCount());

        if (rPoint.X - aRectEndPos.X >= 0 &&
            rPoint.X - aRectEndPos.X < aRectEndPos.Width &&
            rPoint.Y - aRectEndPos.Y >= 0 &&
            rPoint.Y - aRectEndPos.Y < aRectEndPos.Height)
            return getCharacterCount();

        return -1;
    }

    // ask core for position
    DBG_ASSERT( GetFrm() != NULL, "The text frame has vanished!" );
    DBG_ASSERT( GetFrm()->IsTxtFrm(), "The text frame has mutated!" );
    const SwTxtFrm* pFrm = static_cast<const SwTxtFrm*>( GetFrm() );
    SwCrsrMoveState aMoveState;
    aMoveState.bPosMatchesBounds = sal_True;
    sal_Bool bSuccess = pFrm->GetCrsrOfst( &aPos, aCorePoint, &aMoveState );

    SwIndex aCntntIdx = aPos.nContent;
    const xub_StrLen nIndex = aCntntIdx.GetIndex();
    if ( nIndex > 0 )
    {
        SwRect aResultRect;
        pFrm->GetCharRect( aResultRect, aPos );
        bool bVert = pFrm->IsVertical();
        bool bR2L = pFrm->IsRightToLeft();

        if ( (!bVert && aResultRect.Pos().X() > aCorePoint.X()) ||
             ( bVert && aResultRect.Pos().Y() > aCorePoint.Y()) ||
             ( bR2L  && aResultRect.Right()   < aCorePoint.X()) )
        {
            SwIndex aIdxPrev( pNode, nIndex - 1);
            SwPosition aPosPrev( *pNode, aIdxPrev );
            SwRect aResultRectPrev;
            pFrm->GetCharRect( aResultRectPrev, aPosPrev );
            if ( (!bVert && aResultRectPrev.Pos().X() < aCorePoint.X() && aResultRect.Pos().Y() == aResultRectPrev.Pos().Y()) ||
                 ( bVert && aResultRectPrev.Pos().Y() < aCorePoint.Y() && aResultRect.Pos().X() == aResultRectPrev.Pos().X()) ||
                 (  bR2L && aResultRectPrev.Right()   > aCorePoint.X() && aResultRect.Pos().Y() == aResultRectPrev.Pos().Y()) )
                aPos = aPosPrev;
        }
    }

    return bSuccess ?
        GetPortionData().GetAccessiblePosition( aPos.nContent.GetIndex() )
        : -1L;
}

::rtl::OUString SwAccessibleParagraph::getSelectedText()
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    sal_Bool bSelected = GetSelection( nStart, nEnd );
    return bSelected
           ? GetString().copy( nStart, nEnd - nStart )
           : ::rtl::OUString();
}

sal_Int32 SwAccessibleParagraph::getSelectionStart()
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    GetSelection( nStart, nEnd );
    return nStart;
}

sal_Int32 SwAccessibleParagraph::getSelectionEnd()
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    GetSelection( nStart, nEnd );
    return nEnd;
}

sal_Bool SwAccessibleParagraph::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidRange( nStartIndex, nEndIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    sal_Bool bRet = sal_False;

    // get cursor shell
    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( pCrsrShell != NULL )
    {
        // create pam for selection
        SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );
        SwIndex aIndex( pNode, GetPortionData().GetModelPosition(nStartIndex));
        SwPosition aStartPos( *pNode, aIndex );
        SwPaM aPaM( aStartPos );
        aPaM.SetMark();
        aPaM.GetPoint()->nContent =
            GetPortionData().GetModelPosition(nEndIndex);

        // set PaM at cursor shell
        bRet = Select( aPaM );
    }

    return bRet;
}

::rtl::OUString SwAccessibleParagraph::getText()
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    return GetString();
}

::rtl::OUString SwAccessibleParagraph::getTextRange(
    sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    ::rtl::OUString sText( GetString() );

    if ( IsValidRange( nStartIndex, nEndIndex, sText.getLength() ) )
    {
        OrderRange( nStartIndex, nEndIndex );
        return sText.copy(nStartIndex, nEndIndex-nStartIndex );
    }
    else
        throw lang::IndexOutOfBoundsException();
}

/*accessibility::*/TextSegment SwAccessibleParagraph::getTextAtIndex( sal_Int32 nIndex, sal_Int16 nTextType ) throw (lang::IndexOutOfBoundsException, lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    /*accessibility::*/TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;

    const ::rtl::OUString rText = GetString();
    // implement the silly specification that first position after
    // text must return an empty string, rather than throwing an
    // IndexOutOfBoundsException, except for LINE, where the last
    // line is returned
    if( nIndex == rText.getLength() && AccessibleTextType::LINE != nTextType )
        return aResult;

    // with error checking
    i18n::Boundary aBound;
    sal_Bool bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );

    DBG_ASSERT( aBound.startPos >= 0,               "illegal boundary" );
    DBG_ASSERT( aBound.startPos <= aBound.endPos,   "illegal boundary" );

    // return word (if present)
    if ( bWord )
    {
        aResult.SegmentText = rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
        aResult.SegmentStart = aBound.startPos;
        aResult.SegmentEnd = aBound.endPos;
    }

    return aResult;
}

/*accessibility::*/TextSegment SwAccessibleParagraph::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 nTextType ) throw (lang::IndexOutOfBoundsException, lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    const ::rtl::OUString rText = GetString();

    /*accessibility::*/TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;
    //If nIndex = 0, then nobefore text so return -1 directly.
    if( nIndex == 0 )
            return aResult;
    //Tab will be return when call WORDTYPE

    // get starting pos
    i18n::Boundary aBound;
    if (nIndex ==  rText.getLength())
        aBound.startPos = aBound.endPos = nIndex;
    else
    {
        sal_Bool bTmp = GetTextBoundary( aBound, rText, nIndex, nTextType );

        if ( ! bTmp )
            aBound.startPos = aBound.endPos = nIndex;
    }

    // now skip to previous word
    if (nTextType==2 || nTextType == 3)
    {
        i18n::Boundary preBound = aBound;
        while(preBound.startPos==aBound.startPos && nIndex > 0)
        {
            nIndex = min( nIndex, preBound.startPos ) - 1;
            if( nIndex < 0 ) break;
            GetTextBoundary( preBound, rText, nIndex, nTextType );
        }
        //if (nIndex>0)
        if (nIndex>=0)
        //Tab will be return when call WORDTYPE
        {
            aResult.SegmentText = rText.copy( preBound.startPos, preBound.endPos - preBound.startPos );
            aResult.SegmentStart = preBound.startPos;
            aResult.SegmentEnd = preBound.endPos;
        }
    }
    else
    {
        sal_Bool bWord = sal_False;
        while( !bWord )
        {
            nIndex = min( nIndex, aBound.startPos ) - 1;
            if( nIndex >= 0 )
            {
                bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );
            }
            else
                break;  // exit if beginning of string is reached
        }

        if (bWord && nIndex<rText.getLength())
        {
            aResult.SegmentText = rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
            aResult.SegmentStart = aBound.startPos;
            aResult.SegmentEnd = aBound.endPos;
        }
    }
    return aResult;
}

/*accessibility::*/TextSegment SwAccessibleParagraph::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 nTextType ) throw (lang::IndexOutOfBoundsException, lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    /*accessibility::*/TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;
    const ::rtl::OUString rText = GetString();

    // implement the silly specification that first position after
    // text must return an empty string, rather than throwing an
    // IndexOutOfBoundsException
    if( nIndex == rText.getLength() )
        return aResult;


    // get first word, then skip to next word
    i18n::Boundary aBound;
    GetTextBoundary( aBound, rText, nIndex, nTextType );
    sal_Bool bWord = sal_False;
    while( !bWord )
    {
        nIndex = max( sal_Int32(nIndex+1), aBound.endPos );
        if( nIndex < rText.getLength() )
            bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );
        else
            break;  // exit if end of string is reached
    }

    if ( bWord )
    {
        aResult.SegmentText = rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
        aResult.SegmentStart = aBound.startPos;
        aResult.SegmentEnd = aBound.endPos;
    }

/*
        sal_Bool bWord = sal_False;
    bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );

        if (nTextType==2)
        {
                Boundary nexBound=aBound;

        // real current word
        if( nIndex <= aBound.endPos && nIndex >= aBound.startPos )
        {
            while(nexBound.endPos==aBound.endPos&&nIndex<rText.getLength())
            {
                // nIndex = max( (sal_Int32)(nIndex), nexBound.endPos) + 1;
                nIndex = max( (sal_Int32)(nIndex), nexBound.endPos) ;
                const sal_Unicode* pStr = rText.getStr();
                if (pStr)
                {
                    if( pStr[nIndex] == sal_Unicode(' ') )
                        nIndex++;
                }
                if( nIndex < rText.getLength() )
                {
                    bWord = GetTextBoundary( nexBound, rText, nIndex, nTextType );
                }
            }
        }

        if (bWord && nIndex<rText.getLength())
        {
            aResult.SegmentText = rText.copy( nexBound.startPos, nexBound.endPos - nexBound.startPos );
            aResult.SegmentStart = nexBound.startPos;
            aResult.SegmentEnd = nexBound.endPos;
        }

    }
    else
    {
        bWord = sal_False;
        while( !bWord )
        {
            nIndex = max( (sal_Int32)(nIndex+1), aBound.endPos );
            if( nIndex < rText.getLength() )
            {
                bWord = GetTextBoundary( aBound, rText, nIndex, nTextType );
            }
            else
                break;  // exit if end of string is reached
        }
        if (bWord && nIndex<rText.getLength())
        {
            aResult.SegmentText = rText.copy( aBound.startPos, aBound.endPos - aBound.startPos );
            aResult.SegmentStart = aBound.startPos;
            aResult.SegmentEnd = aBound.endPos;
        }
    }
*/
    return aResult;
}

sal_Bool SwAccessibleParagraph::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );
    vos::OGuard aGuard(Application::GetSolarMutex());

    // select and copy (through dispatch mechanism)
    setSelection( nStartIndex, nEndIndex );
    ExecuteAtViewShell( SID_COPY );
    return sal_True;
}


//
//=====  XAccesibleEditableText  ==========================================
//

sal_Bool SwAccessibleParagraph::cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleEditableText );
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( !IsEditableState() )
        return sal_False;

    // select and cut (through dispatch mechanism)
    setSelection( nStartIndex, nEndIndex );
    ExecuteAtViewShell( SID_CUT );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::pasteText( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleEditableText );
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( !IsEditableState() )
        return sal_False;

    // select and paste (through dispatch mechanism)
    setSelection( nIndex, nIndex );
    ExecuteAtViewShell( SID_PASTE );
    return sal_True;
}

sal_Bool SwAccessibleParagraph::deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    return replaceText( nStartIndex, nEndIndex, ::rtl::OUString() );
}

sal_Bool SwAccessibleParagraph::insertText( const ::rtl::OUString& sText, sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    return replaceText( nIndex, nIndex, sText );
}

sal_Bool SwAccessibleParagraph::replaceText(
    sal_Int32 nStartIndex, sal_Int32 nEndIndex,
    const ::rtl::OUString& sReplacement )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleEditableText );

    const ::rtl::OUString& rText = GetString();

    if( IsValidRange( nStartIndex, nEndIndex, rText.getLength() ) )
    {
        if( !IsEditableState() )
            return sal_False;

        SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );

        // translate positions
        sal_uInt16 nStart, nEnd;
        sal_Bool bSuccess = GetPortionData().GetEditableRange(
                                        nStartIndex, nEndIndex, nStart, nEnd );

        // edit only if the range is editable
        if( bSuccess )
        {
            // create SwPosition for nStartIndex
            SwIndex aIndex( pNode, nStart );
            SwPosition aStartPos( *pNode, aIndex );

            // create SwPosition for nEndIndex
            SwPosition aEndPos( aStartPos );
            aEndPos.nContent = nEnd;

            // now create XTextRange as helper and set string
            const uno::Reference<text::XTextRange> xRange(
                SwXTextRange::CreateXTextRange(
                    *pNode->GetDoc(), aStartPos, &aEndPos));
            xRange->setString(sReplacement);

            // delete portion data
            ClearPortionData();
        }

        return bSuccess;
    }
    else
        throw lang::IndexOutOfBoundsException();
}


sal_Bool SwAccessibleParagraph::setAttributes(
    sal_Int32 nStartIndex,
    sal_Int32 nEndIndex,
    const uno::Sequence<PropertyValue>& rAttributeSet )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleEditableText );

    const ::rtl::OUString& rText = GetString();

    if( ! IsValidRange( nStartIndex, nEndIndex, rText.getLength() ) )
        throw lang::IndexOutOfBoundsException();

    if( !IsEditableState() )
        return sal_False;


    // create a (dummy) text portion for the sole purpose of calling
    // setPropertyValue on it
    uno::Reference<XMultiPropertySet> xPortion = CreateUnoPortion( nStartIndex,
                                                              nEndIndex );

    // build sorted index array
    sal_Int32 nLength = rAttributeSet.getLength();
    const PropertyValue* pPairs = rAttributeSet.getConstArray();
    sal_Int32* pIndices = new sal_Int32[nLength];
    sal_Int32 i;
    for( i = 0; i < nLength; i++ )
        pIndices[i] = i;
    sort( &pIndices[0], &pIndices[nLength], IndexCompare(pPairs) );

    // create sorted sequences accoring to index array
    uno::Sequence< ::rtl::OUString > aNames( nLength );
    ::rtl::OUString* pNames = aNames.getArray();
    uno::Sequence< uno::Any > aValues( nLength );
    uno::Any* pValues = aValues.getArray();
    for( i = 0; i < nLength; i++ )
    {
        const PropertyValue& rVal = pPairs[pIndices[i]];
        pNames[i]  = rVal.Name;
        pValues[i] = rVal.Value;
    }
    delete[] pIndices;

    // now set the values
    sal_Bool bRet = sal_True;
    try
    {
        xPortion->setPropertyValues( aNames, aValues );
    }
    catch( UnknownPropertyException e )
    {
        // error handling through return code!
        bRet = sal_False;
    }

    return bRet;
}

sal_Bool SwAccessibleParagraph::setText( const ::rtl::OUString& sText )
    throw (uno::RuntimeException)
{
    return replaceText(0, GetString().getLength(), sText);
}

//=====  XAccessibleSelection  ============================================

void SwAccessibleParagraph::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleParagraph::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    return aSelectionHelper.isAccessibleChildSelected(nChildIndex);
}

void SwAccessibleParagraph::clearAccessibleSelection(  )
    throw ( uno::RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.clearAccessibleSelection();
}

void SwAccessibleParagraph::selectAllAccessibleChildren(  )
    throw ( uno::RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.selectAllAccessibleChildren();
}

sal_Int32 SwAccessibleParagraph::getSelectedAccessibleChildCount(  )
    throw ( uno::RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    return aSelectionHelper.getSelectedAccessibleChildCount();
}

uno::Reference<XAccessible> SwAccessibleParagraph::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException)
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    return aSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

// --> OD 2004-11-16 #111714# - index has to be treated as global child index.
void SwAccessibleParagraph::deselectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    CHECK_FOR_DEFUNC( XAccessibleSelection );

    aSelectionHelper.deselectAccessibleChild( nChildIndex );
}

//=====  XAccessibleHypertext  ============================================

class SwHyperlinkIter_Impl
{
    const SwpHints *pHints;
    xub_StrLen nStt;
    xub_StrLen nEnd;
    sal_uInt16 nPos;

public:
    SwHyperlinkIter_Impl( const SwTxtFrm *pTxtFrm );
    const SwTxtAttr *next();
    sal_uInt16 getCurrHintPos() const { return nPos-1; }

    xub_StrLen startIdx() const { return nStt; }
    xub_StrLen endIdx() const { return nEnd; }
};

SwHyperlinkIter_Impl::SwHyperlinkIter_Impl( const SwTxtFrm *pTxtFrm ) :
    pHints( pTxtFrm->GetTxtNode()->GetpSwpHints() ),
    nStt( pTxtFrm->GetOfst() ),
    nPos( 0 )
{
    const SwTxtFrm *pFollFrm = pTxtFrm->GetFollow();
    nEnd = pFollFrm ? pFollFrm->GetOfst() : pTxtFrm->GetTxtNode()->Len();
}

const SwTxtAttr *SwHyperlinkIter_Impl::next()
{
    const SwTxtAttr *pAttr = 0;
    if( pHints )
    {
        while( !pAttr && nPos < pHints->Count() )
        {
            const SwTxtAttr *pHt = (*pHints)[nPos];
            if( RES_TXTATR_INETFMT == pHt->Which() )
            {
                xub_StrLen nHtStt = *pHt->GetStart();
                xub_StrLen nHtEnd = *pHt->GetAnyEnd();
                if( nHtEnd > nHtStt &&
                    ( (nHtStt >= nStt && nHtStt < nEnd) ||
                      (nHtEnd > nStt && nHtEnd <= nEnd) ) )
                {
                    pAttr = pHt;
                }
            }
            ++nPos;
        }
    }

    return pAttr;
};

sal_Int32 SAL_CALL SwAccessibleParagraph::getHyperLinkCount()
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    sal_Int32 nCount = 0;
    // --> OD 2007-06-27 #i77108# - provide hyperlinks also in editable documents.
//    if( !IsEditableState() )
    // <--
    {
        const SwTxtFrm *pTxtFrm = static_cast<const SwTxtFrm*>( GetFrm() );
        SwHyperlinkIter_Impl aIter( pTxtFrm );
        while( aIter.next() )
            nCount++;
    }

    /* Can't fin the function "GetTOCFirstWordEndIndex" declaration in sym2.0 (Added by yanjun)
    if( GetTOXSortTabBase()  )
    {
        SwTxtNode* pNode = const_cast<SwTxtNode*>(GetTxtNode());
        if(pNode && pNode->GetTOCFirstWordEndIndex() > 0)
            nCount++;
    }
    */
    return nCount;
}

uno::Reference< XAccessibleHyperlink > SAL_CALL
    SwAccessibleParagraph::getHyperLink( sal_Int32 nLinkIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    uno::Reference< XAccessibleHyperlink > xRet;

    // --> OD 2007-06-27 #i77108# - provide hyperlinks also in editable documents.
//    if( !IsEditableState() )
    const SwTxtFrm *pTxtFrm = static_cast<const SwTxtFrm*>( GetFrm() );
    SwHyperlinkIter_Impl aHIter( pTxtFrm );
    //SwAccessibleAutoRecognizerHelper_Impl aARHelper( pTxtFrm );
    sal_Int32 nARCount = 0;
    sal_Int32 nARIndex = 0;
    sal_Int32 nTIndex = -1;
    sal_Int32 nTOCEndIndex = -1;
    SwTxtNode* pNode = NULL;
    SwTOXSortTabBase* pTBase = GetTOXSortTabBase();
    if( pTBase )
    {
        pNode = const_cast<SwTxtNode*>(GetTxtNode());
    }
    nTOCEndIndex = -1;
    //if(pNode)
    //  nTOCEndIndex = pNode->GetTOCFirstWordEndIndex();
    SwTxtAttr* pHt = (SwTxtAttr*)(aHIter.next());
    while( (nLinkIndex < getHyperLinkCount()) && nTIndex < nLinkIndex)
    {
        // no candidates, exit
        //if( (!pHt) && (nARIndex >= nARCount) && nTOCEndIndex <= 0)
        //  break;

        sal_Int32 nHStt = -1;
        sal_Int32 nAStt = -1;
        sal_Bool bH = sal_False;
        sal_Bool bA = sal_False;


        if( pHt )
            nHStt = *pHt->GetStart();
        if( nARIndex < nARCount )
        {
            /*
            sal_Int32 nAEnd;
            aARHelper.getPosition( nARIndex, nAStt, nAEnd );
            */
        }
        sal_Bool bTOC = sal_False;
        // Inside TOC & get the first link
        if( pTBase && nTIndex == -1 )
        {
            nTIndex++;
            bTOC = sal_True;
        }
        else
        {
            if( nHStt >=0 && nAStt >=0 )
            {   // both hyperlink and smart tag available
                nTIndex++;
                if( nHStt <= nAStt )
                    bH = sal_True;
                else
                    bA = sal_True;
            }
            else if( nHStt >= 0 )
            {   // only hyperlink available
                nTIndex++;
                bH = sal_True;
            }
            else if( nAStt >= 0 )
            {   // only smart tag available
                nTIndex++;
                bA = sal_True;
            }
        }

        if( nTIndex == nLinkIndex )
        {   // found
            if( bH )
            {   // it's a hyperlink
                if( pHt )
                {
//                    const SwField* pFFld = pHt->GetFld().GetFld();
                    {
                        if( !pHyperTextData )
                            pHyperTextData = new SwAccessibleHyperTextData;
                        SwAccessibleHyperTextData::iterator aIter =
                            pHyperTextData ->find( pHt );
                        if( aIter != pHyperTextData->end() )
                        {
                            xRet = (*aIter).second;
                        }
                        if( !xRet.is() )
                        {
                            {
                                const sal_Int32 nTmpHStt= GetPortionData().GetAccessiblePosition(
                                    max( aHIter.startIdx(), *pHt->GetStart() ) );
                                const sal_Int32 nTmpHEnd= GetPortionData().GetAccessiblePosition(
                                    min( aHIter.endIdx(), *pHt->GetAnyEnd() ) );
                                xRet = new SwAccessibleHyperlink( aHIter.getCurrHintPos(),
                                    this, nTmpHStt, nTmpHEnd );
                            }
                            if( aIter != pHyperTextData->end() )
                            {
                                (*aIter).second = xRet;
                            }
                            else
                            {
                                SwAccessibleHyperTextData::value_type aEntry( pHt, xRet );
                                pHyperTextData->insert( aEntry );
                            }
                        }
                    }
                }
            }
            else if( bTOC )
            {
                //xRet = new SwAccessibleTOCLink( this );
            }
            else if( bA )
            {
                /*
                // it's a smart tag
                if( !pAutoRecognizerData )
                    pAutoRecognizerData = new SwAccessibleAutoRecognizerData;
                SwAccessibleAutoRecognizerData::iterator aIter =
                    pAutoRecognizerData ->find( nARIndex );
                if( aIter != pAutoRecognizerData->end() )
                {
                    xRet = (*aIter).second;
                }
                if( !xRet.is() )
                {
                    sal_Int32 nAStt = 0;
                    sal_Int32 nAEnd = 0;
                    //aARHelper.getPosition( nARIndex, nAStt, nAEnd );
                    xRet = new SwAccessibleAutoRecognizer( this, nAStt, nAEnd );
                    if( aIter != pAutoRecognizerData->end() )
                    {
                        (*aIter).second = xRet;
                    }
                    else
                    {
                        SwAccessibleAutoRecognizerData::value_type aEntry( nARIndex, xRet );
                        pAutoRecognizerData->insert( aEntry );
                    }
                }
                */
            }
            break;
        }

        // iterate next
        if( bH )
            // iterate next hyperlink
            pHt = (SwTxtAttr*)(aHIter.next());
        else if( bA )
            // iterate next smart tag
            nARIndex++;
        else if(bTOC)
            continue;
        else
            // no candidate, exit
            break;
    }
    /*
        const SwTxtFrm *pTxtFrm = static_cast<const SwTxtFrm*>( GetFrm() );
        SwHyperlinkIter_Impl aHIter( pTxtFrm );
        while( nLinkIndex-- )
            aHIter.next();

        const SwTxtAttr *pHt = aHIter.next();
        if( pHt )
        {
            if( !pHyperTextData )
                pHyperTextData = new SwAccessibleHyperTextData;
            SwAccessibleHyperTextData::iterator aIter =
                pHyperTextData ->find( pHt );
            if( aIter != pHyperTextData->end() )
            {
                xRet = (*aIter).second;
            }
            if( !xRet.is() )
            {
                sal_Int32 nHStt= GetPortionData().GetAccessiblePosition(
                                max( aHIter.startIdx(), *pHt->GetStart() ) );
                sal_Int32 nHEnd= GetPortionData().GetAccessiblePosition(
                                min( aHIter.endIdx(), *pHt->GetAnyEnd() ) );
                xRet = new SwAccessibleHyperlink( aHIter.getCurrHintPos(),
                                                  this, nHStt, nHEnd );
                if( aIter != pHyperTextData->end() )
                {
                    (*aIter).second = xRet;
                }
                else
                {
                    SwAccessibleHyperTextData::value_type aEntry( pHt, xRet );
                    pHyperTextData->insert( aEntry );
                }
            }
        }
    }
    */
    if( !xRet.is() )
        throw lang::IndexOutOfBoundsException();

    return xRet;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getHyperLinkIndex( sal_Int32 nCharIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidPosition( nCharIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    sal_Int32 nRet = -1;
    // --> OD 2007-06-27 #i77108# - provide hyperlinks also in editable documents.
//    if( !IsEditableState() )
    // <--
    {
        const SwTxtFrm *pTxtFrm = static_cast<const SwTxtFrm*>( GetFrm() );
        SwHyperlinkIter_Impl aHIter( pTxtFrm );

        xub_StrLen nIdx = GetPortionData().GetModelPosition( nCharIndex );
        sal_Int32 nPos = 0;
        const SwTxtAttr *pHt = aHIter.next();
        while( pHt && !(nIdx >= *pHt->GetStart() && nIdx < *pHt->GetAnyEnd()) )
        {
            pHt = aHIter.next();
            nPos++;
        }

        if( pHt )
            nRet = nPos;
    }
    /* Added by yanjun for acc miagration
    if( nRet == -1 && GetTOXSortTabBase() )
    {
        SwTxtNode* pNode = const_cast<SwTxtNode*>(GetTxtNode());
        if( nCharIndex >= 0 && nCharIndex < pNode->GetTOCFirstWordEndIndex())
            nRet = 0;
    }
    */

    if (nRet == -1)
        throw lang::IndexOutOfBoundsException();
    else
        return nRet;
    //return nRet;
}

// --> OD 2008-05-26 #i71360#
// --> OD 2010-02-22 #i108125# - adjustments for change tracking text markup
sal_Int32 SAL_CALL SwAccessibleParagraph::getTextMarkupCount( sal_Int32 nTextMarkupType )
                                        throw (lang::IllegalArgumentException,
                                               uno::RuntimeException)
{
    std::auto_ptr<SwTextMarkupHelper> pTextMarkupHelper;
    switch ( nTextMarkupType )
    {
        case text::TextMarkupType::TRACK_CHANGE_INSERTION:
        case text::TextMarkupType::TRACK_CHANGE_DELETION:
        case text::TextMarkupType::TRACK_CHANGE_FORMATCHANGE:
        {
            pTextMarkupHelper.reset( new SwTextMarkupHelper(
                GetPortionData(),
                *(mpParaChangeTrackInfo->getChangeTrackingTextMarkupList( nTextMarkupType ) )) );
        }
        break;
        default:
        {
            pTextMarkupHelper.reset( new SwTextMarkupHelper( GetPortionData(), *GetTxtNode() ) );
        }
    }

    return pTextMarkupHelper->getTextMarkupCount( nTextMarkupType );
}
//MSAA Extension Implementation in app  module
sal_Bool SAL_CALL SwAccessibleParagraph::scrollToPosition( const ::com::sun::star::awt::Point&, sal_Bool )
    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getSelectedPortionCount(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nSeleted = 0;
    SwPaM* pCrsr = GetCursor( true );
    if( pCrsr != NULL )
    {
        // get SwPosition for my node
        const SwTxtNode* pNode = GetTxtNode();
        sal_uLong nHere = pNode->GetIndex();

        // iterate over ring
        SwPaM* pRingStart = pCrsr;
        do
        {
            // ignore, if no mark
            if( pCrsr->HasMark() )
            {
                // check whether nHere is 'inside' pCrsr
                SwPosition* pStart = pCrsr->Start();
                sal_uLong nStartIndex = pStart->nNode.GetIndex();
                SwPosition* pEnd = pCrsr->End();
                sal_uLong nEndIndex = pEnd->nNode.GetIndex();
                if( ( nHere >= nStartIndex ) &&
                    ( nHere <= nEndIndex )      )
                {
                    nSeleted++;
                }
                // else: this PaM doesn't point to this paragraph
            }
            // else: this PaM is collapsed and doesn't select anything

            // next PaM in ring
            pCrsr = static_cast<SwPaM*>( pCrsr->GetNext() );
        }
        while( pCrsr != pRingStart );
    }
    return nSeleted;

}

sal_Int32 SAL_CALL SwAccessibleParagraph::getSeletedPositionStart( sal_Int32 nSelectedPortionIndex )
    throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    /*sal_Bool bSelected = */GetSelectionAtIndex(nSelectedPortionIndex, nStart, nEnd );
    return nStart;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getSeletedPositionEnd( sal_Int32 nSelectedPortionIndex )
    throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    sal_Int32 nStart, nEnd;
    /*sal_Bool bSelected = */GetSelectionAtIndex(nSelectedPortionIndex, nStart, nEnd );
    return nEnd;
}

sal_Bool SAL_CALL SwAccessibleParagraph::removeSelection( sal_Int32 selectionIndex )
    throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    if(selectionIndex < 0) return sal_False;

    sal_Bool bRet = sal_False;
    sal_Int32 nSelected = selectionIndex;

    // get the selection, and test whether it affects our text node
    SwPaM* pCrsr = GetCursor( true );
//  SwPaM* pFirst = pCrsr;
    SwPaM* pPrev = pCrsr;

    if( pCrsr != NULL )
    {
        // get SwPosition for my node
        const SwTxtNode* pNode = GetTxtNode();
        sal_uLong nHere = pNode->GetIndex();

        // iterate over ring
        SwPaM* pRingStart = pCrsr;
        do
        {
            // ignore, if no mark
            if( pCrsr->HasMark() )
            {
                // check whether nHere is 'inside' pCrsr
                SwPosition* pStart = pCrsr->Start();
                sal_uLong nStartIndex = pStart->nNode.GetIndex();
                SwPosition* pEnd = pCrsr->End();
                sal_uLong nEndIndex = pEnd->nNode.GetIndex();
                if( ( nHere >= nStartIndex ) &&
                    ( nHere <= nEndIndex )      )
                {
                    if( nSelected == 0 )
                    {
                        pCrsr->MoveTo((Ring*)0);
                        delete pCrsr;
                        bRet = sal_True;
                    }
                    else
                    {
                        nSelected--;
                    }
                }
            }
            // else: this PaM is collapsed and doesn't select anything
           pPrev = pCrsr;
           pCrsr = static_cast<SwPaM*>( pCrsr->GetNext() );
        }
        while( !bRet && (pCrsr != pRingStart) );
    }
    return sal_True;
}

sal_Int32 SAL_CALL SwAccessibleParagraph::addSelection( sal_Int32, sal_Int32 startOffset, sal_Int32 endOffset)
    throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC_THIS( XAccessibleText, *this );

    // parameter checking
    sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidRange( startOffset, endOffset, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    sal_Int32 nSelectedCount = getSelectedPortionCount();
    for ( sal_Int32 i = nSelectedCount ; i >= 0 ; i--)
    {
        sal_Int32 nStart, nEnd;
        sal_Bool bSelected = GetSelectionAtIndex(i, nStart, nEnd );
        if(bSelected)
        {
            if(nStart <= nEnd )
            {
                if (( startOffset>=nStart && startOffset <=nEnd ) ||     //startOffset in a selection
                       ( endOffset>=nStart && endOffset <=nEnd )     ||  //endOffset in a selection
                    ( startOffset <= nStart && endOffset >=nEnd)  ||       //start and  end include the old selection
                    ( startOffset >= nStart && endOffset <=nEnd) )
                {
                    removeSelection(i);
                }

            }
            else
            {
                if (( startOffset>=nEnd && startOffset <=nStart ) ||     //startOffset in a selection
                       ( endOffset>=nEnd && endOffset <=nStart )     || //endOffset in a selection
                    ( startOffset <= nStart && endOffset >=nEnd)  ||       //start and  end include the old selection
                    ( startOffset >= nStart && endOffset <=nEnd) )

                {
                    removeSelection(i);
                }
            }
        }

    }

    sal_Bool bRet = sal_False;

    // get cursor shell
    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( pCrsrShell != NULL )
    {
        // create pam for selection
        pCrsrShell->StartAction();
//        SwTxtNode* pNode = const_cast<SwTxtNode*>( GetTxtNode() );
        SwPaM* aPaM = pCrsrShell->CreateCrsr();
        aPaM->SetMark();
        aPaM->GetPoint()->nContent = GetPortionData().GetModelPosition(startOffset);
        aPaM->GetMark()->nContent =  GetPortionData().GetModelPosition(endOffset);
        //pCrsrShell->ShowCrsr();
        pCrsrShell->EndAction();
        // set PaM at cursor shell
        //bRet = Select( aPaM );
    }

    return bRet;
}

/*accessibility::*/TextSegment SAL_CALL
        SwAccessibleParagraph::getTextMarkup( sal_Int32 nTextMarkupIndex,
                                              sal_Int32 nTextMarkupType )
                                        throw (lang::IndexOutOfBoundsException,
                                               lang::IllegalArgumentException,
                                               uno::RuntimeException)
{
    std::auto_ptr<SwTextMarkupHelper> pTextMarkupHelper;
    switch ( nTextMarkupType )
    {
        case text::TextMarkupType::TRACK_CHANGE_INSERTION:
        case text::TextMarkupType::TRACK_CHANGE_DELETION:
        case text::TextMarkupType::TRACK_CHANGE_FORMATCHANGE:
        {
            pTextMarkupHelper.reset( new SwTextMarkupHelper(
                GetPortionData(),
                *(mpParaChangeTrackInfo->getChangeTrackingTextMarkupList( nTextMarkupType ) )) );
        }
        break;
        default:
        {
            pTextMarkupHelper.reset( new SwTextMarkupHelper( GetPortionData(), *GetTxtNode() ) );
        }
    }

    return pTextMarkupHelper->getTextMarkup( nTextMarkupIndex, nTextMarkupType );
}

uno::Sequence< /*accessibility::*/TextSegment > SAL_CALL
        SwAccessibleParagraph::getTextMarkupAtIndex( sal_Int32 nCharIndex,
                                                     sal_Int32 nTextMarkupType )
                                        throw (lang::IndexOutOfBoundsException,
                                               lang::IllegalArgumentException,
                                               uno::RuntimeException)
{
    // parameter checking
    const sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidPosition( nCharIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    std::auto_ptr<SwTextMarkupHelper> pTextMarkupHelper;
    switch ( nTextMarkupType )
    {
        case text::TextMarkupType::TRACK_CHANGE_INSERTION:
        case text::TextMarkupType::TRACK_CHANGE_DELETION:
        case text::TextMarkupType::TRACK_CHANGE_FORMATCHANGE:
        {
            pTextMarkupHelper.reset( new SwTextMarkupHelper(
                GetPortionData(),
                *(mpParaChangeTrackInfo->getChangeTrackingTextMarkupList( nTextMarkupType ) )) );
        }
        break;
        default:
        {
            pTextMarkupHelper.reset( new SwTextMarkupHelper( GetPortionData(), *GetTxtNode() ) );
        }
    }

    return pTextMarkupHelper->getTextMarkupAtIndex( nCharIndex, nTextMarkupType );
}
// <--

// --> OD 2008-05-29 #i89175#
sal_Int32 SAL_CALL SwAccessibleParagraph::getLineNumberAtIndex( sal_Int32 nIndex )
                                        throw (lang::IndexOutOfBoundsException,
                                               uno::RuntimeException)
{
    // parameter checking
    const sal_Int32 nLength = GetString().getLength();
    if ( ! IsValidPosition( nIndex, nLength ) )
    {
        throw lang::IndexOutOfBoundsException();
    }

    const sal_Int32 nLineNo = GetPortionData().GetLineNo( nIndex );
    return nLineNo;
}

/*accessibility::*/TextSegment SAL_CALL
        SwAccessibleParagraph::getTextAtLineNumber( sal_Int32 nLineNo )
                                        throw (lang::IndexOutOfBoundsException,
                                               uno::RuntimeException)
{
    // parameter checking
    if ( nLineNo < 0 ||
         nLineNo >= GetPortionData().GetLineCount() )
    {
        throw lang::IndexOutOfBoundsException();
    }

    i18n::Boundary aLineBound;
    GetPortionData().GetBoundaryOfLine( nLineNo, aLineBound );

    /*accessibility::*/TextSegment aTextAtLine;
    const ::rtl::OUString rText = GetString();
    aTextAtLine.SegmentText = rText.copy( aLineBound.startPos,
                                          aLineBound.endPos - aLineBound.startPos );
    aTextAtLine.SegmentStart = aLineBound.startPos;
    aTextAtLine.SegmentEnd = aLineBound.endPos;

    return aTextAtLine;
}

/*accessibility::*/TextSegment SAL_CALL SwAccessibleParagraph::getTextAtLineWithCaret()
                                        throw (uno::RuntimeException)
{
    const sal_Int32 nLineNoOfCaret = getNumberOfLineWithCaret();

    if ( nLineNoOfCaret >= 0 &&
         nLineNoOfCaret < GetPortionData().GetLineCount() )
    {
        return getTextAtLineNumber( nLineNoOfCaret );
    }

    return /*accessibility::*/TextSegment();
}

sal_Int32 SAL_CALL SwAccessibleParagraph::getNumberOfLineWithCaret()
                                        throw (uno::RuntimeException)
{
    const sal_Int32 nCaretPos = getCaretPosition();
    const sal_Int32 nLength = GetString().getLength();
    if ( !IsValidPosition( nCaretPos, nLength ) )
    {
        return -1;
    }

    sal_Int32 nLineNo = GetPortionData().GetLineNo( nCaretPos );

    // special handling for cursor positioned at end of text line via End key
    if ( nCaretPos != 0 )
    {
        i18n::Boundary aLineBound;
        GetPortionData().GetBoundaryOfLine( nLineNo, aLineBound );
        if ( nCaretPos == aLineBound.startPos )
        {
            SwCrsrShell* pCrsrShell = SwAccessibleParagraph::GetCrsrShell();
            if ( pCrsrShell != 0 )
            {
                const awt::Rectangle aCharRect = getCharacterBounds( nCaretPos );

                const SwRect& aCursorCoreRect = pCrsrShell->GetCharRect();
                // translate core coordinates into accessibility coordinates
                Window *pWin = GetWindow();
                CHECK_FOR_WINDOW( XAccessibleComponent, pWin );

                Rectangle aScreenRect( GetMap()->CoreToPixel( aCursorCoreRect.SVRect() ));

                SwRect aFrmLogBounds( GetBounds( *(GetMap()) ) ); // twip rel to doc root
                Point aFrmPixPos( GetMap()->CoreToPixel( aFrmLogBounds.SVRect() ).TopLeft() );
                aScreenRect.Move( -aFrmPixPos.X(), -aFrmPixPos.Y() );

                // convert into AWT Rectangle
                const awt::Rectangle aCursorRect( aScreenRect.Left(),
                                                  aScreenRect.Top(),
                                                  aScreenRect.GetWidth(),
                                                  aScreenRect.GetHeight() );

                if ( aCharRect.X != aCursorRect.X ||
                     aCharRect.Y != aCursorRect.Y )
                {
                    --nLineNo;
                }
            }
        }
    }

    return nLineNo;
}

// --> OD 2010-02-19 #i108125#
void SwAccessibleParagraph::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    mpParaChangeTrackInfo->reset();

    CheckRegistration( pOld, pNew );
}
// <--

sal_Bool SwAccessibleParagraph::GetSelectionAtIndex(
    sal_Int32& nIndex, sal_Int32& nStart, sal_Int32& nEnd)
{
        if(nIndex < 0) return sal_False;


    sal_Bool bRet = sal_False;
    nStart = -1;
    nEnd = -1;
    sal_Int32 nSelected = nIndex;

    // get the selection, and test whether it affects our text node
    SwPaM* pCrsr = GetCursor( true );
    if( pCrsr != NULL )
    {
        // get SwPosition for my node
        const SwTxtNode* pNode = GetTxtNode();
        sal_uLong nHere = pNode->GetIndex();

        // iterate over ring
        SwPaM* pRingStart = pCrsr;
        do
        {
            // ignore, if no mark
            if( pCrsr->HasMark() )
            {
                // check whether nHere is 'inside' pCrsr
                SwPosition* pStart = pCrsr->Start();
                sal_uLong nStartIndex = pStart->nNode.GetIndex();
                SwPosition* pEnd = pCrsr->End();
                sal_uLong nEndIndex = pEnd->nNode.GetIndex();
                if( ( nHere >= nStartIndex ) &&
                    ( nHere <= nEndIndex )      )
                {
                    if( nSelected == 0 )
                    {
                        // translate start and end positions

                        // start position
                        sal_Int32 nLocalStart = -1;
                        if( nHere > nStartIndex )
                        {
                            // selection starts in previous node:
                            // then our local selection starts with the paragraph
                            nLocalStart = 0;
                        }
                        else
                        {
                            DBG_ASSERT( nHere == nStartIndex,
                                        "miscalculated index" );

                            // selection starts in this node:
                            // then check whether it's before or inside our part of
                            // the paragraph, and if so, get the proper position
                            sal_uInt16 nCoreStart = pStart->nContent.GetIndex();
                            if( nCoreStart <
                                GetPortionData().GetFirstValidCorePosition() )
                            {
                                nLocalStart = 0;
                            }
                            else if( nCoreStart <=
                                     GetPortionData().GetLastValidCorePosition() )
                            {
                                DBG_ASSERT(
                                    GetPortionData().IsValidCorePosition(
                                                                      nCoreStart ),
                                     "problem determining valid core position" );

                                nLocalStart =
                                    GetPortionData().GetAccessiblePosition(
                                                                      nCoreStart );
                            }
                        }

                        // end position
                        sal_Int32 nLocalEnd = -1;
                        if( nHere < nEndIndex )
                        {
                            // selection ends in following node:
                            // then our local selection extends to the end
                            nLocalEnd = GetPortionData().GetAccessibleString().
                                                                       getLength();
                        }
                        else
                        {
                            DBG_ASSERT( nHere == nStartIndex,
                                        "miscalculated index" );

                            // selection ends in this node: then select everything
                            // before our part of the node
                            sal_uInt16 nCoreEnd = pEnd->nContent.GetIndex();
                            if( nCoreEnd >
                                    GetPortionData().GetLastValidCorePosition() )
                            {
                                // selection extends beyond out part of this para
                                nLocalEnd = GetPortionData().GetAccessibleString().
                                                                       getLength();
                            }
                            else if( nCoreEnd >=
                                     GetPortionData().GetFirstValidCorePosition() )
                            {
                                // selection is inside our part of this para
                                DBG_ASSERT(
                                    GetPortionData().IsValidCorePosition(
                                                                      nCoreEnd ),
                                     "problem determining valid core position" );

                                nLocalEnd = GetPortionData().GetAccessiblePosition(
                                                                       nCoreEnd );
                            }
                        }

                        if( ( nLocalStart != -1 ) && ( nLocalEnd != -1 ) )
                        {
                            nStart = nLocalStart;
                            nEnd = nLocalEnd;
                            bRet = sal_True;
                        }
                    } // if hit the index
                    else
                    {
                        nSelected--;
                    }
                }
                // else: this PaM doesn't point to this paragraph
            }
            // else: this PaM is collapsed and doesn't select anything

            // next PaM in ring
            pCrsr = static_cast<SwPaM*>( pCrsr->GetNext() );
        }
        while( !bRet && (pCrsr != pRingStart) );
    }
    // else: nocursor -> no selection

    if( bRet )
    {
        sal_Int32 nCaretPos = GetCaretPos();
        if( nStart == nCaretPos )
        {
            sal_Int32 tmp = nStart;
            nStart = nEnd;
            nEnd = tmp;
        }
    }
    return bRet;
}

sal_Int16 SAL_CALL SwAccessibleParagraph::getAccessibleRole (void) throw (::com::sun::star::uno::RuntimeException)
{
    //Get the real heading level, Heading1 ~ Heading10
    if (nHeadingLevel > 0)
    {
        return AccessibleRole::HEADING;
    }
    else
    {
        return AccessibleRole::PARAGRAPH;
    }
}

// End Add


/* This funcion is already defined in accpara.cxx(Added by yanjun)
sal_Int32 SAL_CALL SwAccessibleParagraph::getBackground()
        throw (::com::sun::star::uno::RuntimeException)
{
// Test Code
//     Sequence<OUString> seNames(1);
//     OUString* pStrings = seNames.getArray();
//  pStrings[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("ParaBackColor"));
//
//     Sequence<Any> aAnys(1);
//  Reference<XMultiPropertySet> xPortion = CreateUnoPortion( 0, 0 );
//     aAnys = xPortion->getPropertyValues( seNames );
//  const Any* pAnys = aAnys.getConstArray();
//
//  sal_uInt32 crColorT=0;
//  pAnys[0] >>= crColorT;
// End Test Code

    const SvxBrushItem &rBack = GetFrm()->GetAttrSet()->GetBackground();
    sal_uInt32 crBack = rBack.GetColor().GetColor();

    if (COL_AUTO == crBack)
    {
        Reference<XAccessible> xAccDoc = getAccessibleParent();
        if (xAccDoc.is())
        {
            Reference<XAccessibleComponent> xCompoentDoc(xAccDoc,UNO_QUERY);
            if (xCompoentDoc.is())
            {
                crBack = (sal_uInt32)xCompoentDoc->getBackground();
            }
        }
    }
    return crBack;
}
*/

//Get the real heading level, Heading1 ~ Heading10
sal_Int32 SwAccessibleParagraph::GetRealHeadingLevel()
{
    uno::Reference< ::com::sun::star::beans::XPropertySet > xPortion = CreateUnoPortion( 0, 0 );
    ::rtl::OUString pString = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaStyleName"));
    uno::Any styleAny = xPortion->getPropertyValue( pString );
    ::rtl::OUString sValue;
    if (styleAny >>= sValue)
    {
        //Modified by yanjun for acc migration
        sal_Int32 length = sValue.getLength/*GetCharCount*/();
        if (length == 9 || length == 10)
        {
            ::rtl::OUString headStr = sValue.copy(0, 7);
            if (headStr.equals(::rtl::OUString::createFromAscii("Heading")))
            {
                ::rtl::OUString intStr = sValue.copy(8);
                sal_Int32 headingLevel = intStr.toInt32(10);
                return headingLevel;
            }
        }
    }
    return -1;
}

uno::Any SAL_CALL SwAccessibleParagraph::getExtendedAttributes()
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    uno::Any Ret;
    ::rtl::OUString strHeading(::rtl::OUString::createFromAscii("heading-level:"));
    if( nHeadingLevel >= 0 )
        strHeading += OUString::valueOf(nHeadingLevel, 10);
    strHeading += OUString::createFromAscii(";");

    Ret <<= strHeading;

    return Ret;
}

//Tab will be return when call WORDTYPE
sal_Bool SwAccessibleParagraph::tabCharInWord( sal_Int32 nIndex, i18n::Boundary& aBound)
{
    sal_Bool bFind =  sal_False;
    if( aBound.startPos != nIndex)
    {
        OUString tabStr;
        if(aBound.startPos>nIndex)
            tabStr = GetString().copy(nIndex,(aBound.startPos - nIndex) );

        sal_Unicode tabChar('\t');
        sal_Int32 tabIndex = tabStr.indexOf(tabChar);
        if( tabIndex > -1 )
        {
            aBound.startPos = nIndex + tabIndex ;
            aBound.endPos = aBound.startPos + 1;
            bFind = sal_True;
        }
    }
    return bFind;
}
