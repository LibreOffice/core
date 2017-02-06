/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <memory>
#include <utility>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/awt/FocusEvent.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <unotools/accessiblerelationsethelper.hxx>

#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/settings.hxx>

#include <tools/gen.hxx>
#include <osl/mutex.hxx>
#include <svl/itemset.hxx>

#include <editeng/editobj.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editview.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unoedhlp.hxx>


#include "accessibility.hxx"
#include <unomodel.hxx>
#include <document.hxx>
#include <view.hxx>
#include <o3tl/make_unique.hxx>
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;


static awt::Rectangle lcl_GetBounds( vcl::Window *pWin )
{
    // !! see VCLXAccessibleComponent::implGetBounds()

    //! the coordinates returned are relative to the parent window !
    //! Thus the top-left point may be different from (0, 0) !

    awt::Rectangle aBounds;
    if (pWin)
    {
        Rectangle aRect = pWin->GetWindowExtentsRelative( nullptr );
        aBounds.X       = aRect.Left();
        aBounds.Y       = aRect.Top();
        aBounds.Width   = aRect.GetWidth();
        aBounds.Height  = aRect.GetHeight();
        vcl::Window* pParent = pWin->GetAccessibleParentWindow();
        if (pParent)
        {
            Rectangle aParentRect = pParent->GetWindowExtentsRelative( nullptr );
            awt::Point aParentScreenLoc( aParentRect.Left(), aParentRect.Top() );
            aBounds.X -= aParentScreenLoc.X;
            aBounds.Y -= aParentScreenLoc.Y;
        }
    }
    return aBounds;
}

static awt::Point lcl_GetLocationOnScreen( vcl::Window *pWin )
{
    // !! see VCLXAccessibleComponent::getLocationOnScreen()

    awt::Point aPos;
    if (pWin)
    {
        Rectangle aRect = pWin->GetWindowExtentsRelative( nullptr );
        aPos.X = aRect.Left();
        aPos.Y = aRect.Top();
    }
    return aPos;
}


SmGraphicAccessible::SmGraphicAccessible( SmGraphicWindow *pGraphicWin ) :
    aAccName            (SM_RESSTR(RID_DOCUMENTSTR)),
    nClientId           (0),
    pWin                (pGraphicWin)
{
    OSL_ENSURE( pWin, "SmGraphicAccessible: window missing" );
}

SmGraphicAccessible::~SmGraphicAccessible()
{
}


SmDocShell * SmGraphicAccessible::GetDoc_Impl()
{
    SmViewShell *pView = pWin ? pWin->GetView() : nullptr;
    return pView ? pView->GetDoc() : nullptr;
}

OUString SmGraphicAccessible::GetAccessibleText_Impl()
{
    OUString aTxt;
    SmDocShell *pDoc = GetDoc_Impl();
    if (pDoc)
        aTxt = pDoc->GetAccessibleText();
    return aTxt;
}

void SmGraphicAccessible::ClearWin()
{
    pWin = nullptr;   // implicitly results in AccessibleStateType::DEFUNC set

    if ( nClientId )
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nClientId, *this );
        nClientId =  0;
    }
}

void SmGraphicAccessible::LaunchEvent(
        const sal_Int16 nAccesibleEventId,
        const uno::Any &rOldVal,
        const uno::Any &rNewVal)
{
    AccessibleEventObject aEvt;
    aEvt.Source     = static_cast<XAccessible *>(this);
    aEvt.EventId    = nAccesibleEventId;
    aEvt.OldValue   = rOldVal;
    aEvt.NewValue   = rNewVal ;

    // pass event on to event-listener's
    if (nClientId)
        comphelper::AccessibleEventNotifier::addEvent( nClientId, aEvt );
}

uno::Reference< XAccessibleContext > SAL_CALL SmGraphicAccessible::getAccessibleContext()
{
    return this;
}

sal_Bool SAL_CALL SmGraphicAccessible::containsPoint( const awt::Point& aPoint )
{
    //! the arguments coordinates are relative to the current window !
    //! Thus the top-left point is (0, 0)

    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();

    Size aSz( pWin->GetSizePixel() );
    return  aPoint.X >= 0  &&  aPoint.Y >= 0  &&
            aPoint.X < aSz.Width()  &&  aPoint.Y < aSz.Height();
}

uno::Reference< XAccessible > SAL_CALL SmGraphicAccessible::getAccessibleAtPoint(
        const awt::Point& aPoint )
{
    SolarMutexGuard aGuard;
    XAccessible *pRes = nullptr;
    if (containsPoint( aPoint ))
        pRes = this;
    return pRes;
}

awt::Rectangle SAL_CALL SmGraphicAccessible::getBounds()
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();
    OSL_ENSURE(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );
    return lcl_GetBounds( pWin );
}

awt::Point SAL_CALL SmGraphicAccessible::getLocation()
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();
    OSL_ENSURE(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );
    awt::Rectangle aRect( lcl_GetBounds( pWin ) );
    return awt::Point( aRect.X, aRect.Y );
}

awt::Point SAL_CALL SmGraphicAccessible::getLocationOnScreen()
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();
    OSL_ENSURE(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );
    return lcl_GetLocationOnScreen( pWin );
}

awt::Size SAL_CALL SmGraphicAccessible::getSize()
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();
    OSL_ENSURE(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );

    Size aSz( pWin->GetSizePixel() );
#if OSL_DEBUG_LEVEL > 0
    awt::Rectangle aRect( lcl_GetBounds( pWin ) );
    Size aSz2( aRect.Width, aRect.Height );
    assert(aSz == aSz2 && "mismatch in width" );
#endif
    return awt::Size( aSz.Width(), aSz.Height() );
}

void SAL_CALL SmGraphicAccessible::grabFocus()
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();

    pWin->GrabFocus();
}

sal_Int32 SAL_CALL SmGraphicAccessible::getForeground()
{
    SolarMutexGuard aGuard;

    if (!pWin)
        throw RuntimeException();
    return static_cast<sal_Int32>(pWin->GetTextColor().GetColor());
}

sal_Int32 SAL_CALL SmGraphicAccessible::getBackground()
{
    SolarMutexGuard aGuard;

    if (!pWin)
        throw RuntimeException();
    Wallpaper aWall( pWin->GetDisplayBackground() );
    ColorData nCol;
    if (aWall.IsBitmap() || aWall.IsGradient())
        nCol = pWin->GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    else
        nCol = aWall.GetColor().GetColor();
    return static_cast<sal_Int32>(nCol);
}

sal_Int32 SAL_CALL SmGraphicAccessible::getAccessibleChildCount()
{
    return 0;
}

Reference< XAccessible > SAL_CALL SmGraphicAccessible::getAccessibleChild(
        sal_Int32 /*i*/ )
{
    throw IndexOutOfBoundsException();  // there is no child...
}

Reference< XAccessible > SAL_CALL SmGraphicAccessible::getAccessibleParent()
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();

    vcl::Window *pAccParent = pWin->GetAccessibleParentWindow();
    OSL_ENSURE( pAccParent, "accessible parent missing" );
    return pAccParent ? pAccParent->GetAccessible() : Reference< XAccessible >();
}

sal_Int32 SAL_CALL SmGraphicAccessible::getAccessibleIndexInParent()
{
    SolarMutexGuard aGuard;
    sal_Int32 nIdx = -1;
    vcl::Window *pAccParent = pWin ? pWin->GetAccessibleParentWindow() : nullptr;
    if (pAccParent)
    {
        sal_uInt16 nCnt = pAccParent->GetAccessibleChildWindowCount();
        for (sal_uInt16 i = 0;  i < nCnt  &&  nIdx == -1;  ++i)
            if (pAccParent->GetAccessibleChildWindow( i ) == pWin)
                nIdx = i;
    }
    return nIdx;
}

sal_Int16 SAL_CALL SmGraphicAccessible::getAccessibleRole()
{
    return AccessibleRole::DOCUMENT;
}

OUString SAL_CALL SmGraphicAccessible::getAccessibleDescription()
{
    SolarMutexGuard aGuard;
    SmDocShell *pDoc = GetDoc_Impl();
    return pDoc ? OUString(pDoc->GetText()) : OUString();
}

OUString SAL_CALL SmGraphicAccessible::getAccessibleName()
{
    SolarMutexGuard aGuard;
    return aAccName;
}

Reference< XAccessibleRelationSet > SAL_CALL SmGraphicAccessible::getAccessibleRelationSet()
{
    SolarMutexGuard aGuard;
    Reference< XAccessibleRelationSet > xRelSet = new utl::AccessibleRelationSetHelper();
    return xRelSet;   // empty relation set
}

Reference< XAccessibleStateSet > SAL_CALL SmGraphicAccessible::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;
    ::utl::AccessibleStateSetHelper *pStateSet =
            new ::utl::AccessibleStateSetHelper;

    Reference<XAccessibleStateSet> xStateSet( pStateSet );

    if (!pWin)
        pStateSet->AddState( AccessibleStateType::DEFUNC );
    else
    {
        pStateSet->AddState( AccessibleStateType::ENABLED );
        pStateSet->AddState( AccessibleStateType::FOCUSABLE );
        if (pWin->HasFocus())
            pStateSet->AddState( AccessibleStateType::FOCUSED );
        if (pWin->IsActive())
            pStateSet->AddState( AccessibleStateType::ACTIVE );
        if (pWin->IsVisible())
            pStateSet->AddState( AccessibleStateType::SHOWING );
        if (pWin->IsReallyVisible())
            pStateSet->AddState( AccessibleStateType::VISIBLE );
        if (COL_TRANSPARENT != pWin->GetBackground().GetColor().GetColor())
            pStateSet->AddState( AccessibleStateType::OPAQUE );
    }

    return xStateSet;
}

Locale SAL_CALL SmGraphicAccessible::getLocale()
{
    SolarMutexGuard aGuard;
    // should be the document language...
    // We use the language of the localized symbol names here.
    return Application::GetSettings().GetUILanguageTag().getLocale();
}


void SAL_CALL SmGraphicAccessible::addAccessibleEventListener(
        const Reference< XAccessibleEventListener >& xListener )
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (pWin)
        {
            if (!nClientId)
                nClientId = comphelper::AccessibleEventNotifier::registerClient( );
            comphelper::AccessibleEventNotifier::addEventListener( nClientId, xListener );
        }
    }
}

void SAL_CALL SmGraphicAccessible::removeAccessibleEventListener(
        const Reference< XAccessibleEventListener >& xListener )
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( nClientId, xListener );
        if ( !nListenerCount )
        {
            // no listeners anymore
            // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
            // and at least to us not firing any events anymore, in case somebody calls
            // NotifyAccessibleEvent, again
            comphelper::AccessibleEventNotifier::revokeClient( nClientId );
            nClientId = 0;
        }
    }
}

sal_Int32 SAL_CALL SmGraphicAccessible::getCaretPosition()
{
    return 0;
}

sal_Bool SAL_CALL SmGraphicAccessible::setCaretPosition( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    OUString aTxt( GetAccessibleText_Impl() );
    if (!(nIndex < aTxt.getLength()))
        throw IndexOutOfBoundsException();
    return false;
}

sal_Unicode SAL_CALL SmGraphicAccessible::getCharacter( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    OUString aTxt( GetAccessibleText_Impl() );
    if (!(nIndex < aTxt.getLength()))
        throw IndexOutOfBoundsException();
    return aTxt[nIndex];
}

Sequence< beans::PropertyValue > SAL_CALL SmGraphicAccessible::getCharacterAttributes(
        sal_Int32 nIndex,
        const uno::Sequence< OUString > & /*rRequestedAttributes*/ )
{
    SolarMutexGuard aGuard;
    sal_Int32 nLen = GetAccessibleText_Impl().getLength();
    if (!(0 <= nIndex  &&  nIndex < nLen))
        throw IndexOutOfBoundsException();
    return Sequence< beans::PropertyValue >();
}

awt::Rectangle SAL_CALL SmGraphicAccessible::getCharacterBounds( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;

    awt::Rectangle aRes;

    if (!pWin)
        throw RuntimeException();
    else
    {
        // get accessible text
        SmViewShell *pView = pWin->GetView();
        SmDocShell  *pDoc  = pView ? pView->GetDoc() : nullptr;
        if (!pDoc)
            throw RuntimeException();
        OUString aTxt( GetAccessibleText_Impl() );
        if (!(0 <= nIndex  &&  nIndex <= aTxt.getLength()))   // aTxt.getLength() is valid
            throw IndexOutOfBoundsException();

        // find a reasonable rectangle for position aTxt.getLength().
        bool bWasBehindText = (nIndex == aTxt.getLength());
        if (bWasBehindText && nIndex)
            --nIndex;

        const SmNode *pTree = pDoc->GetFormulaTree();
        const SmNode *pNode = pTree->FindNodeWithAccessibleIndex( nIndex );
        //! pNode may be 0 if the index belongs to a char that was inserted
        //! only for the accessible text!
        if (pNode)
        {
            sal_Int32 nAccIndex = pNode->GetAccessibleIndex();
            OSL_ENSURE( nAccIndex >= 0, "invalid accessible index" );
            OSL_ENSURE( nIndex >= nAccIndex, "index out of range" );

            OUStringBuffer aBuf;
            pNode->GetAccessibleText(aBuf);
            OUString aNodeText = aBuf.makeStringAndClear();
            sal_Int32 nNodeIndex = nIndex - nAccIndex;
            if (0 <= nNodeIndex  &&  nNodeIndex < aNodeText.getLength())
            {
                // get appropriate rectangle
                Point aOffset(pNode->GetTopLeft() - pTree->GetTopLeft());
                Point aTLPos (pWin->GetFormulaDrawPos() + aOffset);
                aTLPos.X() -= 0;
                Size  aSize (pNode->GetSize());

                long* pXAry = new long[ aNodeText.getLength() ];
                pWin->SetFont( pNode->GetFont() );
                pWin->GetTextArray( aNodeText, pXAry, 0, aNodeText.getLength() );
                aTLPos.X()    += nNodeIndex > 0 ? pXAry[nNodeIndex - 1] : 0;
                aSize.Width()  = nNodeIndex > 0 ? pXAry[nNodeIndex] - pXAry[nNodeIndex - 1] : pXAry[nNodeIndex];
                delete[] pXAry;

                aTLPos = pWin->LogicToPixel( aTLPos );
                aSize  = pWin->LogicToPixel( aSize );
                aRes.X = aTLPos.X();
                aRes.Y = aTLPos.Y();
                aRes.Width  = aSize.Width();
                aRes.Height = aSize.Height();
            }
        }

        // take rectangle from last character and move it to the right
        if (bWasBehindText)
            aRes.X += aRes.Width;
    }

    return aRes;
}

sal_Int32 SAL_CALL SmGraphicAccessible::getCharacterCount()
{
    SolarMutexGuard aGuard;
    return GetAccessibleText_Impl().getLength();
}

sal_Int32 SAL_CALL SmGraphicAccessible::getIndexAtPoint( const awt::Point& aPoint )
{
    SolarMutexGuard aGuard;

    sal_Int32 nRes = -1;
    if (pWin)
    {
        const SmNode *pTree = pWin->GetView()->GetDoc()->GetFormulaTree();
        // can be NULL! e.g. if one clicks within the window already during loading of the
        // document (before the parser even started)
        if (!pTree)
            return nRes;

        // get position relative to formula draw position
        Point  aPos( aPoint.X, aPoint.Y );
        aPos = pWin->PixelToLogic( aPos );
        aPos -= pWin->GetFormulaDrawPos();

        // if it was inside the formula then get the appropriate node
        const SmNode *pNode = nullptr;
        if (pTree->OrientedDist(aPos) <= 0)
            pNode = pTree->FindRectClosestTo(aPos);

        if (pNode)
        {
            // get appropriate rectangle
            Point   aOffset( pNode->GetTopLeft() - pTree->GetTopLeft() );
            Point   aTLPos ( aOffset );
            aTLPos.X() -= 0;
            Size  aSize( pNode->GetSize() );

            Rectangle aRect( aTLPos, aSize );
            if (aRect.IsInside( aPos ))
            {
                OSL_ENSURE( pNode->IsVisible(), "node is not a leaf" );
                OUStringBuffer aBuf;
                pNode->GetAccessibleText(aBuf);
                OUString aTxt = aBuf.makeStringAndClear();
                OSL_ENSURE( !aTxt.isEmpty(), "no accessible text available" );

                long nNodeX = pNode->GetLeft();

                long* pXAry = new long[ aTxt.getLength() ];
                pWin->SetFont( pNode->GetFont() );
                pWin->GetTextArray( aTxt, pXAry, 0, aTxt.getLength() );
                for (sal_Int32 i = 0;  i < aTxt.getLength()  &&  nRes == -1;  ++i)
                {
                    if (pXAry[i] + nNodeX > aPos.X())
                        nRes = i;
                }
                delete[] pXAry;
                OSL_ENSURE( nRes >= 0  &&  nRes < aTxt.getLength(), "index out of range" );
                OSL_ENSURE( pNode->GetAccessibleIndex() >= 0,
                        "invalid accessible index" );

                nRes = pNode->GetAccessibleIndex() + nRes;
            }
        }
    }
    return nRes;
}

OUString SAL_CALL SmGraphicAccessible::getSelectedText()
{
    return OUString();
}

sal_Int32 SAL_CALL SmGraphicAccessible::getSelectionStart()
{
    return -1;
}

sal_Int32 SAL_CALL SmGraphicAccessible::getSelectionEnd()
{
    return -1;
}

sal_Bool SAL_CALL SmGraphicAccessible::setSelection(
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex )
{
    SolarMutexGuard aGuard;
    sal_Int32 nLen = GetAccessibleText_Impl().getLength();
    if (!(0 <= nStartIndex  &&  nStartIndex < nLen) ||
        !(0 <= nEndIndex    &&  nEndIndex   < nLen))
        throw IndexOutOfBoundsException();
    return false;
}

OUString SAL_CALL SmGraphicAccessible::getText()
{
    SolarMutexGuard aGuard;
    return GetAccessibleText_Impl();
}

OUString SAL_CALL SmGraphicAccessible::getTextRange(
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex )
{
    //!! nEndIndex may be the string length per definition of the interface !!
    //!! text should be copied exclusive that end index though. And arguments
    //!! may be switched.

    SolarMutexGuard aGuard;
    OUString aTxt( GetAccessibleText_Impl() );
    sal_Int32 nStart = std::min(nStartIndex, nEndIndex);
    sal_Int32 nEnd   = std::max(nStartIndex, nEndIndex);
    if (!(nStart <= aTxt.getLength()) ||
        !(nEnd   <= aTxt.getLength()))
        throw IndexOutOfBoundsException();
    return aTxt.copy( nStart, nEnd - nStart );
}

css::accessibility::TextSegment SAL_CALL SmGraphicAccessible::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    SolarMutexGuard aGuard;
    OUString aTxt( GetAccessibleText_Impl() );
    //!! nIndex is allowed to be the string length
    if (!(nIndex <= aTxt.getLength()))
        throw IndexOutOfBoundsException();

    css::accessibility::TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;
    if ( (AccessibleTextType::CHARACTER == aTextType)  &&  (nIndex < aTxt.getLength()) )
    {
        aResult.SegmentText = aTxt.copy(nIndex, 1);
        aResult.SegmentStart = nIndex;
        aResult.SegmentEnd = nIndex+1;
    }
    return aResult;
}

css::accessibility::TextSegment SAL_CALL SmGraphicAccessible::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    SolarMutexGuard aGuard;
    OUString aTxt( GetAccessibleText_Impl() );
    //!! nIndex is allowed to be the string length
    if (!(nIndex <= aTxt.getLength()))
        throw IndexOutOfBoundsException();

    css::accessibility::TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;

    if ( (AccessibleTextType::CHARACTER == aTextType)  && nIndex )
    {
        aResult.SegmentText = aTxt.copy(nIndex-1, 1);
        aResult.SegmentStart = nIndex-1;
        aResult.SegmentEnd = nIndex;
    }
    return aResult;
}

css::accessibility::TextSegment SAL_CALL SmGraphicAccessible::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    SolarMutexGuard aGuard;
    OUString aTxt( GetAccessibleText_Impl() );
    //!! nIndex is allowed to be the string length
    if (!(nIndex <= aTxt.getLength()))
        throw IndexOutOfBoundsException();

    css::accessibility::TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;

    nIndex++; // text *behind*
    if ( (AccessibleTextType::CHARACTER == aTextType)  &&  (nIndex < aTxt.getLength()) )
    {
        aResult.SegmentText = aTxt.copy(nIndex, 1);
        aResult.SegmentStart = nIndex;
        aResult.SegmentEnd = nIndex+1;
    }
    return aResult;
}

sal_Bool SAL_CALL SmGraphicAccessible::copyText(
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex )
{
    SolarMutexGuard aGuard;
    bool bReturn = false;

    if (!pWin)
        throw RuntimeException();
    else
    {
        Reference< datatransfer::clipboard::XClipboard > xClipboard = pWin->GetClipboard();
        if ( xClipboard.is() )
        {
            OUString sText( getTextRange(nStartIndex, nEndIndex) );

            vcl::unohelper::TextDataObject* pDataObj = new vcl::unohelper::TextDataObject( sText );
            SolarMutexReleaser aReleaser;
            xClipboard->setContents( pDataObj, nullptr );

            Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( xClipboard, uno::UNO_QUERY );
            if( xFlushableClipboard.is() )
                xFlushableClipboard->flushClipboard();

            bReturn = true;
        }
    }

    return bReturn;
}

OUString SAL_CALL SmGraphicAccessible::getImplementationName()
{
    return OUString("SmGraphicAccessible");
}

sal_Bool SAL_CALL SmGraphicAccessible::supportsService(
        const OUString& rServiceName )
{
    return  cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL SmGraphicAccessible::getSupportedServiceNames()
{
    return Sequence< OUString >{
        "css::accessibility::Accessible",
        "css::accessibility::AccessibleComponent",
        "css::accessibility::AccessibleContext",
        "css::accessibility::AccessibleText"
    };
}


SmEditSource::SmEditSource( SmEditWindow * /*pWin*/, SmEditAccessible &rAcc ) :
    aViewFwd    (rAcc),
    aTextFwd    (rAcc, *this),
    aEditViewFwd(rAcc),
    rEditAcc (rAcc)
{
}

SmEditSource::SmEditSource( const SmEditSource &rSrc ) :
    SvxEditSource(),
    aViewFwd    (rSrc.rEditAcc),
    aTextFwd    (rSrc.rEditAcc, *this),
    aEditViewFwd(rSrc.rEditAcc),
    rEditAcc    (rSrc.rEditAcc)
{
}

SmEditSource::~SmEditSource()
{
}

SvxEditSource* SmEditSource::Clone() const
{
    return new SmEditSource( *this );
}

SvxTextForwarder* SmEditSource::GetTextForwarder()
{
    return &aTextFwd;
}

SvxViewForwarder* SmEditSource::GetViewForwarder()
{
    return &aViewFwd;
}

SvxEditViewForwarder* SmEditSource::GetEditViewForwarder( bool /*bCreate*/ )
{
    return &aEditViewFwd;
}

void SmEditSource::UpdateData()
{
    // would possibly only by needed if the XText interface is implemented
    // and its text needs to be updated.
}

SfxBroadcaster & SmEditSource::GetBroadcaster() const
{
    return const_cast<SmEditSource*>(this)->aBroadCaster;
}

SmViewForwarder::SmViewForwarder( SmEditAccessible &rAcc ) :
    rEditAcc(rAcc)
{
}

SmViewForwarder::~SmViewForwarder()
{
}

bool SmViewForwarder::IsValid() const
{
    return rEditAcc.GetEditView() != nullptr;
}

Rectangle SmViewForwarder::GetVisArea() const
{
    EditView *pEditView = rEditAcc.GetEditView();
    OutputDevice* pOutDev = pEditView ? pEditView->GetWindow() : nullptr;

    if( pOutDev && pEditView)
    {
        Rectangle aVisArea = pEditView->GetVisArea();

        // figure out map mode from edit engine
        EditEngine* pEditEngine = pEditView->GetEditEngine();

        if( pEditEngine )
        {
            MapMode aMapMode(pOutDev->GetMapMode());
            aVisArea = OutputDevice::LogicToLogic( aVisArea,
                                                   pEditEngine->GetRefMapMode(),
                                                   aMapMode.GetMapUnit() );
            aMapMode.SetOrigin(Point());
            return pOutDev->LogicToPixel( aVisArea, aMapMode );
        }
    }

    return Rectangle();
}

Point SmViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    EditView *pEditView = rEditAcc.GetEditView();
    OutputDevice* pOutDev = pEditView ? pEditView->GetWindow() : nullptr;

    if( pOutDev )
    {
        MapMode aMapMode(pOutDev->GetMapMode());
        Point aPoint( OutputDevice::LogicToLogic( rPoint, rMapMode,
                                                  aMapMode.GetMapUnit() ) );
        aMapMode.SetOrigin(Point());
        return pOutDev->LogicToPixel( aPoint, aMapMode );
    }

    return Point();
}

Point SmViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    EditView *pEditView = rEditAcc.GetEditView();
    OutputDevice* pOutDev = pEditView ? pEditView->GetWindow() : nullptr;

    if( pOutDev )
    {
        MapMode aMapMode(pOutDev->GetMapMode());
        aMapMode.SetOrigin(Point());
        Point aPoint( pOutDev->PixelToLogic( rPoint, aMapMode ) );
        return OutputDevice::LogicToLogic( aPoint,
                                           aMapMode.GetMapUnit(),
                                           rMapMode );
    }

    return Point();
}


SmTextForwarder::SmTextForwarder( SmEditAccessible& rAcc, SmEditSource & rSource) :
    rEditAcc ( rAcc ),
    rEditSource (rSource)
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetNotifyHdl( LINK(this, SmTextForwarder, NotifyHdl) );
}

SmTextForwarder::~SmTextForwarder()
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetNotifyHdl( Link<EENotify&,void>() );
}

IMPL_LINK(SmTextForwarder, NotifyHdl, EENotify&, rNotify, void)
{
    ::std::unique_ptr< SfxHint > aHint = SvxEditSourceHelper::EENotification2Hint( &rNotify );
    if (aHint.get())
        rEditSource.GetBroadcaster().Broadcast( *aHint.get() );
}

sal_Int32 SmTextForwarder::GetParagraphCount() const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetParagraphCount() : 0;
}

sal_Int32 SmTextForwarder::GetTextLen( sal_Int32 nParagraph ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetTextLen( nParagraph ) : 0;
}

OUString SmTextForwarder::GetText( const ESelection& rSel ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    OUString aRet;
    if (pEditEngine)
        aRet = pEditEngine->GetText( rSel );
    return convertLineEnd(aRet, GetSystemLineEnd());
}

SfxItemSet SmTextForwarder::GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    assert(pEditEngine && "EditEngine missing");
    if( rSel.nStartPara == rSel.nEndPara )
    {
        GetAttribsFlags nFlags = GetAttribsFlags::NONE;
        switch( nOnlyHardAttrib )
        {
        case EditEngineAttribs_All:
            nFlags = GetAttribsFlags::ALL;
            break;
        case EditEngineAttribs_HardAndPara:
            nFlags = GetAttribsFlags::PARAATTRIBS|GetAttribsFlags::CHARATTRIBS;
            break;
        case EditEngineAttribs_OnlyHard:
            nFlags = GetAttribsFlags::CHARATTRIBS;
            break;
        default:
            SAL_WARN("starmath", "unknown flags for SmTextForwarder::GetAttribs");
        }

        return pEditEngine->GetAttribs( rSel.nStartPara, rSel.nStartPos, rSel.nEndPos, nFlags );
    }
    else
    {
        return pEditEngine->GetAttribs( rSel, nOnlyHardAttrib );
    }
}

SfxItemSet SmTextForwarder::GetParaAttribs( sal_Int32 nPara ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    assert(pEditEngine && "EditEngine missing");

    SfxItemSet aSet( pEditEngine->GetParaAttribs( nPara ) );

    sal_uInt16 nWhich = EE_PARA_START;
    while( nWhich <= EE_PARA_END )
    {
        if( aSet.GetItemState( nWhich ) != SfxItemState::SET )
        {
            if( pEditEngine->HasParaAttrib( nPara, nWhich ) )
                aSet.Put( pEditEngine->GetParaAttrib( nPara, nWhich ) );
        }
        nWhich++;
    }

    return aSet;
}

void SmTextForwarder::SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet )
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetParaAttribs( nPara, rSet );
}

SfxItemPool* SmTextForwarder::GetPool() const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetEmptyItemSet().GetPool() : nullptr;
}

void SmTextForwarder::RemoveAttribs( const ESelection& rSelection )
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->RemoveAttribs( rSelection, false/*bRemoveParaAttribs*/, 0 );
}

void SmTextForwarder::GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->GetPortions( nPara, rList );
}

void SmTextForwarder::QuickInsertText( const OUString& rText, const ESelection& rSel )
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->QuickInsertText( rText, rSel );
}

void SmTextForwarder::QuickInsertLineBreak( const ESelection& rSel )
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->QuickInsertLineBreak( rSel );
}

void SmTextForwarder::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->QuickInsertField( rFld, rSel );
}

void SmTextForwarder::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->QuickSetAttribs( rSet, rSel );
}

bool SmTextForwarder::IsValid() const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    // cannot reliably query EditEngine state
    // while in the middle of an update
    return pEditEngine && pEditEngine->GetUpdateMode();
}

OUString SmTextForwarder::CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->CalcFieldValue(rField, nPara, nPos, rpTxtColor, rpFldColor) : OUString();
}

void SmTextForwarder::FieldClicked(const SvxFieldItem&, sal_Int32, sal_Int32)
{
}

static SfxItemState GetSvxEditEngineItemState( EditEngine& rEditEngine, const ESelection& rSel, sal_uInt16 nWhich )
{
    std::vector<EECharAttrib> aAttribs;

    const SfxPoolItem*  pLastItem = nullptr;

    SfxItemState eState = SfxItemState::DEFAULT;

    // check all paragraphs inside the selection
    for( sal_Int32 nPara = rSel.nStartPara; nPara <= rSel.nEndPara; nPara++ )
    {
        SfxItemState eParaState = SfxItemState::DEFAULT;

        // calculate start and endpos for this paragraph
        sal_Int32 nPos = 0;
        if( rSel.nStartPara == nPara )
            nPos = rSel.nStartPos;

        sal_Int32 nEndPos = rSel.nEndPos;
        if( rSel.nEndPara != nPara )
            nEndPos = rEditEngine.GetTextLen( nPara );


        // get list of char attribs
        rEditEngine.GetCharAttribs( nPara, aAttribs );

        bool bEmpty = true;     // we found no item inside the selection of this paragraph
        bool bGaps  = false;    // we found items but theire gaps between them
        sal_Int32 nLastEnd = nPos;

        const SfxPoolItem* pParaItem = nullptr;

        for(std::vector<EECharAttrib>::const_iterator i = aAttribs.begin(); i < aAttribs.end(); ++i)
        {
            OSL_ENSURE( i->pAttr, "GetCharAttribs gives corrupt data" );

            const bool bEmptyPortion = (i->nStart == i->nEnd);
            if( (!bEmptyPortion && (i->nStart >= nEndPos)) || (bEmptyPortion && (i->nStart > nEndPos)) )
                break;  // break if we are already behind our selection

            if( (!bEmptyPortion && (i->nEnd <= nPos)) || (bEmptyPortion && (i->nEnd < nPos)) )
                continue;   // or if the attribute ends before our selection

            if( i->pAttr->Which() != nWhich )
                continue; // skip if is not the searched item

            // if we already found an item
            if( pParaItem )
            {
                // ... and its different to this one than the state is don't care
                if( *pParaItem != *(i->pAttr) )
                    return SfxItemState::DONTCARE;
            }
            else
            {
                pParaItem = i->pAttr;
            }

            if( bEmpty )
                bEmpty = false;

            if( !bGaps && i->nStart > nLastEnd )
                bGaps = true;

            nLastEnd = i->nEnd;
        }

        if( !bEmpty && !bGaps && nLastEnd < ( nEndPos - 1 ) )
            bGaps = true;
        if( bEmpty )
            eParaState = SfxItemState::DEFAULT;
        else if( bGaps )
            eParaState = SfxItemState::DONTCARE;
        else
            eParaState = SfxItemState::SET;

        // if we already found an item check if we found the same
        if( pLastItem )
        {
            if( (pParaItem == nullptr) || (*pLastItem != *pParaItem) )
                return SfxItemState::DONTCARE;
        }
        else
        {
            pLastItem = pParaItem;
            eState = eParaState;
        }
    }

    return eState;
}

SfxItemState SmTextForwarder::GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const
{
    SfxItemState nState = SfxItemState::DISABLED;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        nState = GetSvxEditEngineItemState( *pEditEngine, rSel, nWhich );
    return nState;
}

SfxItemState SmTextForwarder::GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    SfxItemState nState = SfxItemState::DISABLED;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        const SfxItemSet& rSet = pEditEngine->GetParaAttribs( nPara );
        nState = rSet.GetItemState( nWhich );
    }
    return nState;
}

LanguageType SmTextForwarder::GetLanguage( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetLanguage(nPara, nIndex) : LANGUAGE_NONE;
}

sal_Int32 SmTextForwarder::GetFieldCount( sal_Int32 nPara ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetFieldCount(nPara) : 0;
}

EFieldInfo SmTextForwarder::GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetFieldInfo( nPara, nField ) : EFieldInfo();
}

EBulletInfo SmTextForwarder::GetBulletInfo( sal_Int32 /*nPara*/ ) const
{
    return EBulletInfo();
}

Rectangle SmTextForwarder::GetCharBounds( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    Rectangle aRect(0,0,0,0);
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();

    if (pEditEngine)
    {
        // Handle virtual position one-past-the end of the string
        if( nIndex >= pEditEngine->GetTextLen(nPara) )
        {
            if( nIndex )
                aRect = pEditEngine->GetCharacterBounds( EPosition(nPara, nIndex-1) );

            aRect.Move( aRect.Right() - aRect.Left(), 0 );
            aRect.SetSize( Size(1, pEditEngine->GetTextHeight()) );
        }
        else
        {
            aRect = pEditEngine->GetCharacterBounds( EPosition(nPara, nIndex) );
        }
    }
    return aRect;
}

Rectangle SmTextForwarder::GetParaBounds( sal_Int32 nPara ) const
{
    Rectangle aRect(0,0,0,0);
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();

    if (pEditEngine)
    {
        const Point aPnt = pEditEngine->GetDocPosTopLeft( nPara );
        const sal_uLong nWidth = pEditEngine->CalcTextWidth();
        const sal_uLong nHeight = pEditEngine->GetTextHeight( nPara );
        aRect = Rectangle( aPnt.X(), aPnt.Y(), aPnt.X() + nWidth, aPnt.Y() + nHeight );
    }

    return aRect;
}

MapMode SmTextForwarder::GetMapMode() const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetRefMapMode() : MapMode( MapUnit::Map100thMM );
}

OutputDevice* SmTextForwarder::GetRefDevice() const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetRefDevice() : nullptr;
}

bool SmTextForwarder::GetIndexAtPoint( const Point& rPos, sal_Int32& nPara, sal_Int32& nIndex ) const
{
    bool bRes = false;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        EPosition aDocPos = pEditEngine->FindDocPosition( rPos );
        nPara   = aDocPos.nPara;
        nIndex  = aDocPos.nIndex;
        bRes = true;
    }
    return bRes;
}

bool SmTextForwarder::GetWordIndices( sal_Int32 nPara, sal_Int32 nIndex, sal_Int32& nStart, sal_Int32& nEnd ) const
{
    bool bRes = false;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        ESelection aRes = pEditEngine->GetWord( ESelection(nPara, nIndex, nPara, nIndex), css::i18n::WordType::DICTIONARY_WORD );

        if( aRes.nStartPara == nPara &&
            aRes.nStartPara == aRes.nEndPara )
        {
            nStart = aRes.nStartPos;
            nEnd = aRes.nEndPos;

            bRes = true;
        }
    }

    return bRes;
}

bool SmTextForwarder::GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nPara, sal_Int32 nIndex, bool bInCell ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine &&
           SvxEditSourceHelper::GetAttributeRun( nStartIndex, nEndIndex, *pEditEngine, nPara, nIndex, bInCell );
}

sal_Int32 SmTextForwarder::GetLineCount( sal_Int32 nPara ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetLineCount(nPara) : 0;
}

sal_Int32 SmTextForwarder::GetLineLen( sal_Int32 nPara, sal_Int32 nLine ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetLineLen(nPara, nLine) : 0;
}

void SmTextForwarder::GetLineBoundaries( /*out*/sal_Int32 &rStart, /*out*/sal_Int32 &rEnd, sal_Int32 nPara, sal_Int32 nLine ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->GetLineBoundaries(rStart, rEnd, nPara, nLine);
    else
        rStart = rEnd = 0;
}

sal_Int32 SmTextForwarder::GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetLineNumberAtIndex(nPara, nIndex) : 0;
}

bool SmTextForwarder::QuickFormatDoc( bool /*bFull*/ )
{
    bool bRes = false;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        pEditEngine->QuickFormatDoc();
        bRes = true;
    }
    return bRes;
}

sal_Int16 SmTextForwarder::GetDepth( sal_Int32 /*nPara*/ ) const
{
    // math has no outliner...
    return -1;
}

bool SmTextForwarder::SetDepth( sal_Int32 /*nPara*/, sal_Int16 nNewDepth )
{
    // math has no outliner...
    return -1 == nNewDepth;  // is it the value from 'GetDepth' ?
}

bool SmTextForwarder::Delete( const ESelection& rSelection )
{
    bool bRes = false;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        pEditEngine->QuickDelete( rSelection );
        pEditEngine->QuickFormatDoc();
        bRes = true;
    }
    return bRes;
}

bool SmTextForwarder::InsertText( const OUString& rStr, const ESelection& rSelection )
{
    bool bRes = false;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        pEditEngine->QuickInsertText( rStr, rSelection );
        pEditEngine->QuickFormatDoc();
        bRes = true;
    }
    return bRes;
}

const SfxItemSet*   SmTextForwarder::GetEmptyItemSetPtr()
{
    const SfxItemSet *pItemSet = nullptr;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        pItemSet = &pEditEngine->GetEmptyItemSet();
    }
    return pItemSet;
}

void SmTextForwarder::AppendParagraph()
{
    // append an empty paragraph
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        sal_Int32 nParaCount = pEditEngine->GetParagraphCount();
        pEditEngine->InsertParagraph( nParaCount, OUString() );
    }
}

sal_Int32 SmTextForwarder::AppendTextPortion( sal_Int32 nPara, const OUString &rText, const SfxItemSet &rSet )
{
    sal_uInt16 nRes = 0;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine && nPara < pEditEngine->GetParagraphCount())
    {
        // append text
        ESelection aSel( nPara, pEditEngine->GetTextLen( nPara ) );
        pEditEngine->QuickInsertText( rText, aSel );

        // set attributes for new appended text
        nRes = aSel.nEndPos = pEditEngine->GetTextLen( nPara );
        pEditEngine->QuickSetAttribs( rSet, aSel );
    }
    return nRes;
}

void SmTextForwarder::CopyText(const SvxTextForwarder& rSource)
{

    const SmTextForwarder* pSourceForwarder = dynamic_cast< const SmTextForwarder* >( &rSource );
    if( !pSourceForwarder )
        return;
    EditEngine* pSourceEditEngine = pSourceForwarder->rEditAcc.GetEditEngine();
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine && pSourceEditEngine )
    {
        EditTextObject* pNewTextObject = pSourceEditEngine->CreateTextObject();
        pEditEngine->SetText( *pNewTextObject );
        delete pNewTextObject;
    }
}


SmEditViewForwarder::SmEditViewForwarder( SmEditAccessible& rAcc ) :
    rEditAcc( rAcc )
{
}

SmEditViewForwarder::~SmEditViewForwarder()
{
}

bool SmEditViewForwarder::IsValid() const
{
    return rEditAcc.GetEditView() != nullptr;
}

Rectangle SmEditViewForwarder::GetVisArea() const
{
    Rectangle aRect(0,0,0,0);

    EditView *pEditView = rEditAcc.GetEditView();
    OutputDevice* pOutDev = pEditView ? pEditView->GetWindow() : nullptr;

    if( pOutDev && pEditView)
    {
        Rectangle aVisArea = pEditView->GetVisArea();

        // figure out map mode from edit engine
        EditEngine* pEditEngine = pEditView->GetEditEngine();

        if( pEditEngine )
        {
            MapMode aMapMode(pOutDev->GetMapMode());
            aVisArea = OutputDevice::LogicToLogic( aVisArea,
                                                   pEditEngine->GetRefMapMode(),
                                                   aMapMode.GetMapUnit() );
            aMapMode.SetOrigin(Point());
            aRect = pOutDev->LogicToPixel( aVisArea, aMapMode );
        }
    }

    return aRect;
}

Point SmEditViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    EditView *pEditView = rEditAcc.GetEditView();
    OutputDevice* pOutDev = pEditView ? pEditView->GetWindow() : nullptr;

    if( pOutDev )
    {
        MapMode aMapMode(pOutDev->GetMapMode());
        Point aPoint( OutputDevice::LogicToLogic( rPoint, rMapMode,
                                                  aMapMode.GetMapUnit() ) );
        aMapMode.SetOrigin(Point());
        return pOutDev->LogicToPixel( aPoint, aMapMode );
    }

    return Point();
}

Point SmEditViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    EditView *pEditView = rEditAcc.GetEditView();
    OutputDevice* pOutDev = pEditView ? pEditView->GetWindow() : nullptr;

    if( pOutDev )
    {
        MapMode aMapMode(pOutDev->GetMapMode());
        aMapMode.SetOrigin(Point());
        Point aPoint( pOutDev->PixelToLogic( rPoint, aMapMode ) );
        return OutputDevice::LogicToLogic( aPoint,
                                           aMapMode.GetMapUnit(),
                                           rMapMode );
    }

    return Point();
}

bool SmEditViewForwarder::GetSelection( ESelection& rSelection ) const
{
    bool bRes = false;
    EditView *pEditView = rEditAcc.GetEditView();
    if (pEditView)
    {
        rSelection = pEditView->GetSelection();
        bRes = true;
    }
    return bRes;
}

bool SmEditViewForwarder::SetSelection( const ESelection& rSelection )
{
    bool bRes = false;
    EditView *pEditView = rEditAcc.GetEditView();
    if (pEditView)
    {
        pEditView->SetSelection( rSelection );
        bRes = true;
    }
    return bRes;
}

bool SmEditViewForwarder::Copy()
{
    bool bRes = false;
    EditView *pEditView = rEditAcc.GetEditView();
    if (pEditView)
    {
        pEditView->Copy();
        bRes = true;
    }
    return bRes;
}

bool SmEditViewForwarder::Cut()
{
    bool bRes = false;
    EditView *pEditView = rEditAcc.GetEditView();
    if (pEditView)
    {
        pEditView->Cut();
        bRes = true;
    }
    return bRes;
}

bool SmEditViewForwarder::Paste()
{
    bool bRes = false;
    EditView *pEditView = rEditAcc.GetEditView();
    if (pEditView)
    {
        pEditView->Paste();
        bRes = true;
    }
    return bRes;
}


SmEditAccessible::SmEditAccessible( SmEditWindow *pEditWin ) :
    aAccName            (SM_RESSTR(STR_CMDBOXWINDOW)),
    pTextHelper         (),
    pWin                (pEditWin)
{
    OSL_ENSURE( pWin, "SmEditAccessible: window missing" );
}

SmEditAccessible::~SmEditAccessible()
{
}

::accessibility::AccessibleTextHelper *SmEditAccessible::GetTextHelper()
{
    return pTextHelper.get();
}

void SmEditAccessible::Init()
{
    OSL_ENSURE( pWin, "SmEditAccessible: window missing" );
    if (pWin)
    {
        EditEngine *pEditEngine = pWin->GetEditEngine();
        EditView   *pEditView   = pWin->GetEditView();
        if (pEditEngine && pEditView)
        {
            assert(!pTextHelper);
            pTextHelper.reset(new ::accessibility::AccessibleTextHelper( o3tl::make_unique<SmEditSource>( pWin, *this ) ));
            pTextHelper->SetEventSource( this );
        }
    }
}

void SmEditAccessible::ClearWin()
{
    // remove handler before current object gets destroyed
    // (avoid handler being called for already dead object)
    EditEngine *pEditEngine = GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetNotifyHdl( Link<EENotify&,void>() );

    pWin = nullptr;   // implicitly results in AccessibleStateType::DEFUNC set

    //! make TextHelper implicitly release C++ references to some core objects
    pTextHelper->SetEditSource( ::std::unique_ptr<SvxEditSource>() );
    //! make TextHelper release references
    //! (e.g. the one set by the 'SetEventSource' call)
    pTextHelper->Dispose();
    pTextHelper.reset();
}

// XAccessible
uno::Reference< XAccessibleContext > SAL_CALL SmEditAccessible::getAccessibleContext(  )
{
    SolarMutexGuard aGuard;
    return this;
}

// XAccessibleComponent
sal_Bool SAL_CALL SmEditAccessible::containsPoint( const awt::Point& aPoint )
{
    //! the arguments coordinates are relative to the current window !
    //! Thus the top left-point is (0, 0)

    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();

    Size aSz( pWin->GetSizePixel() );
    return  aPoint.X >= 0  &&  aPoint.Y >= 0  &&
            aPoint.X < aSz.Width()  &&  aPoint.Y < aSz.Height();
}

uno::Reference< XAccessible > SAL_CALL SmEditAccessible::getAccessibleAtPoint( const awt::Point& aPoint )
{
    SolarMutexGuard aGuard;
    if (!pTextHelper)
        throw RuntimeException();
    return pTextHelper->GetAt( aPoint );
}

awt::Rectangle SAL_CALL SmEditAccessible::getBounds(  )
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();
    OSL_ENSURE(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );
    return lcl_GetBounds( pWin );
}

awt::Point SAL_CALL SmEditAccessible::getLocation(  )
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();
    OSL_ENSURE(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );
    awt::Rectangle aRect( lcl_GetBounds( pWin ) );
    return awt::Point( aRect.X, aRect.Y );
}

awt::Point SAL_CALL SmEditAccessible::getLocationOnScreen(  )
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();
    OSL_ENSURE(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );
    return lcl_GetLocationOnScreen( pWin );
}

awt::Size SAL_CALL SmEditAccessible::getSize(  )
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();
    OSL_ENSURE(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );

    Size aSz( pWin->GetSizePixel() );
#if OSL_DEBUG_LEVEL > 0
    awt::Rectangle aRect( lcl_GetBounds( pWin ) );
    Size aSz2( aRect.Width, aRect.Height );
    assert(aSz == aSz2 && "mismatch in width");
#endif
    return awt::Size( aSz.Width(), aSz.Height() );
}

void SAL_CALL SmEditAccessible::grabFocus(  )
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();

    pWin->GrabFocus();
}

sal_Int32 SAL_CALL SmEditAccessible::getForeground()
{
    SolarMutexGuard aGuard;

    if (!pWin)
        throw RuntimeException();
    return static_cast<sal_Int32>(pWin->GetTextColor().GetColor());
}

sal_Int32 SAL_CALL SmEditAccessible::getBackground()
{
    SolarMutexGuard aGuard;

    if (!pWin)
        throw RuntimeException();
    Wallpaper aWall( pWin->GetDisplayBackground() );
    ColorData nCol;
    if (aWall.IsBitmap() || aWall.IsGradient())
        nCol = pWin->GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    else
        nCol = aWall.GetColor().GetColor();
    return static_cast<sal_Int32>(nCol);
}

// XAccessibleContext
sal_Int32 SAL_CALL SmEditAccessible::getAccessibleChildCount(  )
{
    SolarMutexGuard aGuard;
    if (!pTextHelper)
        throw RuntimeException();
    return pTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL SmEditAccessible::getAccessibleChild( sal_Int32 i )
{
    SolarMutexGuard aGuard;
    if (!pTextHelper)
        throw RuntimeException();
    return pTextHelper->GetChild( i );
}

uno::Reference< XAccessible > SAL_CALL SmEditAccessible::getAccessibleParent(  )
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();

    vcl::Window *pAccParent = pWin->GetAccessibleParentWindow();
    OSL_ENSURE( pAccParent, "accessible parent missing" );
    return pAccParent ? pAccParent->GetAccessible() : Reference< XAccessible >();
}

sal_Int32 SAL_CALL SmEditAccessible::getAccessibleIndexInParent(  )
{
    SolarMutexGuard aGuard;
    sal_Int32 nIdx = -1;
    vcl::Window *pAccParent = pWin ? pWin->GetAccessibleParentWindow() : nullptr;
    if (pAccParent)
    {
        sal_uInt16 nCnt = pAccParent->GetAccessibleChildWindowCount();
        for (sal_uInt16 i = 0;  i < nCnt  &&  nIdx == -1;  ++i)
            if (pAccParent->GetAccessibleChildWindow( i ) == pWin)
                nIdx = i;
    }
    return nIdx;
}

sal_Int16 SAL_CALL SmEditAccessible::getAccessibleRole(  )
{
    return AccessibleRole::PANEL /*TEXT ?*/;
}

OUString SAL_CALL SmEditAccessible::getAccessibleDescription(  )
{
    return OUString();  // empty as agreed with product-management
}

OUString SAL_CALL SmEditAccessible::getAccessibleName(  )
{
    SolarMutexGuard aGuard;
    // same name as displayed by the window when not docked
    return aAccName;
}

uno::Reference< XAccessibleRelationSet > SAL_CALL SmEditAccessible::getAccessibleRelationSet(  )
{
    Reference< XAccessibleRelationSet > xRelSet = new utl::AccessibleRelationSetHelper();
    return xRelSet;   // empty relation set
}

uno::Reference< XAccessibleStateSet > SAL_CALL SmEditAccessible::getAccessibleStateSet(  )
{
    SolarMutexGuard aGuard;
    ::utl::AccessibleStateSetHelper *pStateSet =
            new ::utl::AccessibleStateSetHelper;

    Reference<XAccessibleStateSet> xStateSet( pStateSet );

    if (!pWin || !pTextHelper)
        pStateSet->AddState( AccessibleStateType::DEFUNC );
    else
    {
        pStateSet->AddState( AccessibleStateType::MULTI_LINE );
        pStateSet->AddState( AccessibleStateType::ENABLED );
        pStateSet->AddState( AccessibleStateType::FOCUSABLE );
        if (pWin->HasFocus())
            pStateSet->AddState( AccessibleStateType::FOCUSED );
        if (pWin->IsActive())
            pStateSet->AddState( AccessibleStateType::ACTIVE );
        if (pWin->IsVisible())
            pStateSet->AddState( AccessibleStateType::SHOWING );
        if (pWin->IsReallyVisible())
            pStateSet->AddState( AccessibleStateType::VISIBLE );
        if (COL_TRANSPARENT != pWin->GetBackground().GetColor().GetColor())
            pStateSet->AddState( AccessibleStateType::OPAQUE );
    }

    return xStateSet;
}

Locale SAL_CALL SmEditAccessible::getLocale(  )
{
    SolarMutexGuard aGuard;
    // should be the document language...
    // We use the language of the localized symbol names here.
    return Application::GetSettings().GetUILanguageTag().getLocale();
}


// XAccessibleEventBroadcaster
void SAL_CALL SmEditAccessible::addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
{
    if (pTextHelper)   // not disposing (about to destroy view shell)
        pTextHelper->AddEventListener( xListener );
}

void SAL_CALL SmEditAccessible::removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
{
   if (pTextHelper)   // not disposing (about to destroy view shell)
        pTextHelper->RemoveEventListener( xListener );
}

OUString SAL_CALL SmEditAccessible::getImplementationName()
{
    return OUString("SmEditAccessible");
}

sal_Bool SAL_CALL SmEditAccessible::supportsService(
        const OUString& rServiceName )
{
    return  cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL SmEditAccessible::getSupportedServiceNames()
{
    return Sequence< OUString >{
        "css::accessibility::Accessible",
        "css::accessibility::AccessibleComponent",
        "css::accessibility::AccessibleContext"
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
