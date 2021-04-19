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

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <unotools/accessiblerelationsethelper.hxx>

#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <svx/AccessibleTextHelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/settings.hxx>

#include <tools/gen.hxx>
#include <svl/itemset.hxx>

#include <editeng/editobj.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editview.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unoedhlp.hxx>


#include "accessibility.hxx"
#include <document.hxx>
#include <view.hxx>
#include <strings.hrc>
#include <smmod.hxx>

using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

SmGraphicAccessible::SmGraphicAccessible(SmGraphicWidget *pGraphicWin) :
    aAccName            (SmResId(RID_DOCUMENTSTR)),
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
    SmViewShell *pView = pWin ? &pWin->GetView() : nullptr;
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
        const sal_Int16 nAccessibleEventId,
        const uno::Any &rOldVal,
        const uno::Any &rNewVal)
{
    AccessibleEventObject aEvt;
    aEvt.Source     = static_cast<XAccessible *>(this);
    aEvt.EventId    = nAccessibleEventId;
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

    Size aSz( pWin->GetOutputSizePixel() );
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

    const Point aOutPos;
    const Size aOutSize(pWin->GetOutputSizePixel());
    css::awt::Rectangle aRet;

    aRet.X = aOutPos.X();
    aRet.Y = aOutPos.Y();
    aRet.Width = aOutSize.Width();
    aRet.Height = aOutSize.Height();

    return aRet;
}

awt::Point SAL_CALL SmGraphicAccessible::getLocation()
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();

    const css::awt::Rectangle aRect(getBounds());
    css::awt::Point aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

awt::Point SAL_CALL SmGraphicAccessible::getLocationOnScreen()
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();

    css::awt::Point aScreenLoc(0, 0);

    css::uno::Reference<css::accessibility::XAccessible> xParent(getAccessibleParent());
    if (xParent)
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xParentContext(
            xParent->getAccessibleContext());
        css::uno::Reference<css::accessibility::XAccessibleComponent> xParentComponent(
            xParentContext, css::uno::UNO_QUERY);
        OSL_ENSURE(xParentComponent.is(),
                   "WeldEditAccessible::getLocationOnScreen: no parent component!");
        if (xParentComponent.is())
        {
            css::awt::Point aParentScreenLoc(xParentComponent->getLocationOnScreen());
            css::awt::Point aOwnRelativeLoc(getLocation());
            aScreenLoc.X = aParentScreenLoc.X + aOwnRelativeLoc.X;
            aScreenLoc.Y = aParentScreenLoc.Y + aOwnRelativeLoc.Y;
        }
    }

    return aScreenLoc;
}

awt::Size SAL_CALL SmGraphicAccessible::getSize()
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();
    Size aSz(pWin->GetOutputSizePixel());
    return css::awt::Size(aSz.Width(), aSz.Height());
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

    weld::DrawingArea* pDrawingArea = pWin->GetDrawingArea();
    OutputDevice& rDevice = pDrawingArea->get_ref_device();

    return static_cast<sal_Int32>(rDevice.GetTextColor());
}

sal_Int32 SAL_CALL SmGraphicAccessible::getBackground()
{
    SolarMutexGuard aGuard;
    if (!pWin)
        throw RuntimeException();

    weld::DrawingArea* pDrawingArea = pWin->GetDrawingArea();
    OutputDevice& rDevice = pDrawingArea->get_ref_device();

    Wallpaper aWall(rDevice.GetBackground());
    Color nCol;
    if (aWall.IsBitmap() || aWall.IsGradient())
        nCol = Application::GetSettings().GetStyleSettings().GetWindowColor();
    else
        nCol = aWall.GetColor();
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

    return pWin->GetDrawingArea()->get_accessible_parent();
}

sal_Int32 SAL_CALL SmGraphicAccessible::getAccessibleIndexInParent()
{
    SolarMutexGuard aGuard;

    // -1 for child not found/no parent (according to specification)
    sal_Int32 nRet = -1;

    css::uno::Reference<css::accessibility::XAccessible> xParent(getAccessibleParent());
    if (!xParent)
        return nRet;

    try
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xParentContext(
            xParent->getAccessibleContext());

        //  iterate over parent's children and search for this object
        if (xParentContext.is())
        {
            sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
            for (sal_Int32 nChild = 0; (nChild < nChildCount) && (-1 == nRet); ++nChild)
            {
                css::uno::Reference<css::accessibility::XAccessible> xChild(
                    xParentContext->getAccessibleChild(nChild));
                if (xChild.get() == this)
                    nRet = nChild;
            }
        }
    }
    catch (const css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("svx", "WeldEditAccessible::getAccessibleIndexInParent");
    }

    return nRet;
}

sal_Int16 SAL_CALL SmGraphicAccessible::getAccessibleRole()
{
    return AccessibleRole::DOCUMENT;
}

OUString SAL_CALL SmGraphicAccessible::getAccessibleDescription()
{
    SolarMutexGuard aGuard;
    SmDocShell *pDoc = GetDoc_Impl();
    return pDoc ? pDoc->GetText() : OUString();
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
    rtl::Reference<::utl::AccessibleStateSetHelper> pStateSet =
            new ::utl::AccessibleStateSetHelper;

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
        weld::DrawingArea* pDrawingArea = pWin->GetDrawingArea();
        OutputDevice& rDevice = pDrawingArea->get_ref_device();
        if (COL_TRANSPARENT != rDevice.GetBackground().GetColor())
            pStateSet->AddState( AccessibleStateType::OPAQUE );
    }

    return pStateSet;
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
    if (!(xListener.is() && nClientId))
        return;

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

sal_Int32 SAL_CALL SmGraphicAccessible::getCaretPosition()
{
    return 0;
}

sal_Bool SAL_CALL SmGraphicAccessible::setCaretPosition( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    OUString aTxt( GetAccessibleText_Impl() );
    if (nIndex >= aTxt.getLength())
        throw IndexOutOfBoundsException();
    return false;
}

sal_Unicode SAL_CALL SmGraphicAccessible::getCharacter( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    OUString aTxt( GetAccessibleText_Impl() );
    if (nIndex >= aTxt.getLength())
        throw IndexOutOfBoundsException();
    return aTxt[nIndex];
}

Sequence< beans::PropertyValue > SAL_CALL SmGraphicAccessible::getCharacterAttributes(
        sal_Int32 nIndex,
        const uno::Sequence< OUString > & /*rRequestedAttributes*/ )
{
    SolarMutexGuard aGuard;
    sal_Int32 nLen = GetAccessibleText_Impl().getLength();
    if (0 > nIndex  ||  nIndex >= nLen)
        throw IndexOutOfBoundsException();
    return Sequence< beans::PropertyValue >();
}

awt::Rectangle SAL_CALL SmGraphicAccessible::getCharacterBounds( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;

    awt::Rectangle aRes;

    if (!pWin)
        throw RuntimeException();

    // get accessible text
    SmDocShell* pDoc  = pWin->GetView().GetDoc();
    if (!pDoc)
        throw RuntimeException();
    OUString aTxt( GetAccessibleText_Impl() );
    if (0 > nIndex  ||  nIndex > aTxt.getLength())   // aTxt.getLength() is valid
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
            Size  aSize (pNode->GetSize());

            weld::DrawingArea* pDrawingArea = pWin->GetDrawingArea();
            OutputDevice& rDevice = pDrawingArea->get_ref_device();

            std::unique_ptr<tools::Long[]> pXAry(new tools::Long[ aNodeText.getLength() ]);
            rDevice.SetFont( pNode->GetFont() );
            rDevice.GetTextArray( aNodeText, pXAry.get(), 0, aNodeText.getLength() );
            aTLPos.AdjustX(nNodeIndex > 0 ? pXAry[nNodeIndex - 1] : 0 );
            aSize.setWidth( nNodeIndex > 0 ? pXAry[nNodeIndex] - pXAry[nNodeIndex - 1] : pXAry[nNodeIndex] );
            pXAry.reset();

            aTLPos = rDevice.LogicToPixel( aTLPos );
            aSize  = rDevice.LogicToPixel( aSize );
            aRes.X = aTLPos.X();
            aRes.Y = aTLPos.Y();
            aRes.Width  = aSize.Width();
            aRes.Height = aSize.Height();
        }
    }

    // take rectangle from last character and move it to the right
    if (bWasBehindText)
        aRes.X += aRes.Width;

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
        const SmNode *pTree = pWin->GetView().GetDoc()->GetFormulaTree();
        // can be NULL! e.g. if one clicks within the window already during loading of the
        // document (before the parser even started)
        if (!pTree)
            return nRes;

        weld::DrawingArea* pDrawingArea = pWin->GetDrawingArea();
        OutputDevice& rDevice = pDrawingArea->get_ref_device();

        // get position relative to formula draw position
        Point  aPos( aPoint.X, aPoint.Y );
        aPos = rDevice.PixelToLogic( aPos );
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
            Size  aSize( pNode->GetSize() );

            tools::Rectangle aRect( aTLPos, aSize );
            if (aRect.IsInside( aPos ))
            {
                OSL_ENSURE( pNode->IsVisible(), "node is not a leaf" );
                OUStringBuffer aBuf;
                pNode->GetAccessibleText(aBuf);
                OUString aTxt = aBuf.makeStringAndClear();
                OSL_ENSURE( !aTxt.isEmpty(), "no accessible text available" );

                tools::Long nNodeX = pNode->GetLeft();

                std::unique_ptr<tools::Long[]> pXAry(new tools::Long[ aTxt.getLength() ]);
                rDevice.SetFont( pNode->GetFont() );
                rDevice.GetTextArray( aTxt, pXAry.get(), 0, aTxt.getLength() );
                for (sal_Int32 i = 0;  i < aTxt.getLength()  &&  nRes == -1;  ++i)
                {
                    if (pXAry[i] + nNodeX > aPos.X())
                        nRes = i;
                }
                pXAry.reset();
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
    if (0 > nStartIndex  ||  nStartIndex >= nLen ||
        0 > nEndIndex    ||  nEndIndex   >= nLen)
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
    if ((nStart > aTxt.getLength()) ||
        (nEnd   > aTxt.getLength()))
        throw IndexOutOfBoundsException();
    return aTxt.copy( nStart, nEnd - nStart );
}

css::accessibility::TextSegment SAL_CALL SmGraphicAccessible::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    SolarMutexGuard aGuard;
    OUString aTxt( GetAccessibleText_Impl() );
    //!! nIndex is allowed to be the string length
    if (nIndex > aTxt.getLength())
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
    if (nIndex > aTxt.getLength())
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
    if (nIndex > aTxt.getLength())
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

    Reference< datatransfer::clipboard::XClipboard > xClipboard = GetSystemClipboard();
    if ( xClipboard.is() )
    {
        OUString sText( getTextRange(nStartIndex, nEndIndex) );

        rtl::Reference<vcl::unohelper::TextDataObject> pDataObj = new vcl::unohelper::TextDataObject( sText );
        SolarMutexReleaser aReleaser;
        xClipboard->setContents( pDataObj, nullptr );

        Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( xClipboard, uno::UNO_QUERY );
        if( xFlushableClipboard.is() )
            xFlushableClipboard->flushClipboard();

        bReturn = true;
    }


    return bReturn;
}

sal_Bool SAL_CALL SmGraphicAccessible::scrollSubstringTo( sal_Int32, sal_Int32, AccessibleScrollType )
{
    return false;
}

OUString SAL_CALL SmGraphicAccessible::getImplementationName()
{
    return "SmGraphicAccessible";
}

sal_Bool SAL_CALL SmGraphicAccessible::supportsService(
        const OUString& rServiceName )
{
    return  cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL SmGraphicAccessible::getSupportedServiceNames()
{
    return {
        "css::accessibility::Accessible",
        "css::accessibility::AccessibleComponent",
        "css::accessibility::AccessibleContext",
        "css::accessibility::AccessibleText"
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
