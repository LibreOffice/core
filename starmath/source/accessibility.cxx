/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_starmath.hxx"
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/awt/FocusEvent.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <unotools/accessiblerelationsethelper.hxx>


#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/unohelp2.hxx>
#include <tools/gen.hxx>
#include <vos/mutex.hxx>
#include <svl/itemset.hxx>

#include <editeng/editobj.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editview.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unoedhlp.hxx>


#include "accessibility.hxx"
#include <applicat.hxx>
#include <document.hxx>
#include <view.hxx>

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

#define A2OU(cChar)  rtl::OUString::createFromAscii(cChar)

//////////////////////////////////////////////////////////////////////

static awt::Rectangle lcl_GetBounds( Window *pWin )
{
    // !! see VCLXAccessibleComponent::implGetBounds()

    //! the coordinates returned are relativ to the parent window !
    //! Thus the top-left point may be different from (0, 0) !

    awt::Rectangle aBounds;
    if (pWin)
    {
        Rectangle aRect = pWin->GetWindowExtentsRelative( NULL );
        aBounds.X       = aRect.Left();
        aBounds.Y       = aRect.Top();
        aBounds.Width   = aRect.GetWidth();
        aBounds.Height  = aRect.GetHeight();
        Window* pParent = pWin->GetAccessibleParentWindow();
        if (pParent)
        {
            Rectangle aParentRect = pParent->GetWindowExtentsRelative( NULL );
            awt::Point aParentScreenLoc( aParentRect.Left(), aParentRect.Top() );
            aBounds.X -= aParentScreenLoc.X;
            aBounds.Y -= aParentScreenLoc.Y;
        }
    }
    return aBounds;
}

static awt::Point lcl_GetLocationOnScreen( Window *pWin )
{
    // !! see VCLXAccessibleComponent::getLocationOnScreen()

    awt::Point aPos;
    if (pWin)
    {
        Rectangle aRect = pWin->GetWindowExtentsRelative( NULL );
        aPos.X = aRect.Left();
        aPos.Y = aRect.Top();
    }
    return aPos;
}

//////////////////////////////////////////////////////////////////////

SmGraphicAccessible::SmGraphicAccessible( SmGraphicWindow *pGraphicWin ) :
    aAccName            ( String(SmResId(RID_DOCUMENTSTR)) ),
    nClientId           (0),
    pWin                (pGraphicWin)
{
    DBG_ASSERT( pWin, "SmGraphicAccessible: window missing" );
    //++aRefCount;
}


SmGraphicAccessible::SmGraphicAccessible( const SmGraphicAccessible &rSmAcc ) :
    SmGraphicAccessibleBaseClass(),
    aAccName            ( String(SmResId(RID_DOCUMENTSTR)) ),
    nClientId           (0)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    pWin = rSmAcc.pWin;
    DBG_ASSERT( pWin, "SmGraphicAccessible: window missing" );
    //++aRefCount;
}


SmGraphicAccessible::~SmGraphicAccessible()
{
/*
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (--aRefCount == 0)
    {
    }
*/
}


SmDocShell * SmGraphicAccessible::GetDoc_Impl()
{
    SmViewShell *pView = pWin ? pWin->GetView() : 0;
    return pView ? pView->GetDoc() : 0;
}

String SmGraphicAccessible::GetAccessibleText_Impl()
{
    String aTxt;
    SmDocShell *pDoc = GetDoc_Impl();
    if (pDoc)
        aTxt = pDoc->GetAccessibleText();
    return aTxt;
}

void SmGraphicAccessible::ClearWin()
{
    pWin = 0;   // implicitly results in AccessibleStateType::DEFUNC set

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
    aEvt.Source     = (XAccessible *) this;
    aEvt.EventId    = nAccesibleEventId;
    aEvt.OldValue   = rOldVal;
    aEvt.NewValue   = rNewVal ;

    // pass event on to event-listener's
    if (nClientId)
        comphelper::AccessibleEventNotifier::addEvent( nClientId, aEvt );
}

uno::Reference< XAccessibleContext > SAL_CALL SmGraphicAccessible::getAccessibleContext()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return this;
}

sal_Bool SAL_CALL SmGraphicAccessible::containsPoint( const awt::Point& aPoint )
    throw (RuntimeException)
{
    //! the arguments coordinates are relativ to the current window !
    //! Thus the top-left point is (0, 0)

    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    Size aSz( pWin->GetSizePixel() );
    return  aPoint.X >= 0  &&  aPoint.Y >= 0  &&
            aPoint.X < aSz.Width()  &&  aPoint.Y < aSz.Height();
}

uno::Reference< XAccessible > SAL_CALL SmGraphicAccessible::getAccessibleAtPoint(
        const awt::Point& aPoint )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    XAccessible *pRes = 0;
    if (containsPoint( aPoint ))
        pRes = this;
    return pRes;
}

awt::Rectangle SAL_CALL SmGraphicAccessible::getBounds()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();
    DBG_ASSERT(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );
    return lcl_GetBounds( pWin );
}

awt::Point SAL_CALL SmGraphicAccessible::getLocation()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();
    DBG_ASSERT(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );
    awt::Rectangle aRect( lcl_GetBounds( pWin ) );
    return awt::Point( aRect.X, aRect.Y );
}

awt::Point SAL_CALL SmGraphicAccessible::getLocationOnScreen()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();
    DBG_ASSERT(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );
    return lcl_GetLocationOnScreen( pWin );
}

awt::Size SAL_CALL SmGraphicAccessible::getSize()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();
    DBG_ASSERT(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );

    Size aSz( pWin->GetSizePixel() );
#if OSL_DEBUG_LEVEL > 1
    awt::Rectangle aRect( lcl_GetBounds( pWin ) );
    Size aSz2( aRect.Width, aRect.Height );
    DBG_ASSERT( aSz == aSz2, "mismatch in width" );
#endif
    return awt::Size( aSz.Width(), aSz.Height() );
}

void SAL_CALL SmGraphicAccessible::grabFocus()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    pWin->GrabFocus();
}

sal_Int32 SAL_CALL SmGraphicAccessible::getForeground()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!pWin)
        throw RuntimeException();
    return (sal_Int32) pWin->GetTextColor().GetColor();
}

sal_Int32 SAL_CALL SmGraphicAccessible::getBackground()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!pWin)
        throw RuntimeException();
    Wallpaper aWall( pWin->GetDisplayBackground() );
    ColorData nCol;
    if (aWall.IsBitmap() || aWall.IsGradient())
        nCol = pWin->GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    else
        nCol = aWall.GetColor().GetColor();
    return (sal_Int32) nCol;
}

sal_Int32 SAL_CALL SmGraphicAccessible::getAccessibleChildCount()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return 0;
}

Reference< XAccessible > SAL_CALL SmGraphicAccessible::getAccessibleChild(
        sal_Int32 /*i*/ )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    throw IndexOutOfBoundsException();  // there is no child...
    /*return 0;*/
}

Reference< XAccessible > SAL_CALL SmGraphicAccessible::getAccessibleParent()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    Window *pAccParent = pWin->GetAccessibleParentWindow();
    DBG_ASSERT( pAccParent, "accessible parent missing" );
    return pAccParent ? pAccParent->GetAccessible() : Reference< XAccessible >();
}

sal_Int32 SAL_CALL SmGraphicAccessible::getAccessibleIndexInParent()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int32 nIdx = -1;
    Window *pAccParent = pWin ? pWin->GetAccessibleParentWindow() : 0;
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
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return AccessibleRole::DOCUMENT;
}

OUString SAL_CALL SmGraphicAccessible::getAccessibleDescription()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SmDocShell *pDoc = GetDoc_Impl();
    return pDoc ? OUString(pDoc->GetText()) : OUString();
}

OUString SAL_CALL SmGraphicAccessible::getAccessibleName()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return aAccName;
}

Reference< XAccessibleRelationSet > SAL_CALL SmGraphicAccessible::getAccessibleRelationSet()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Reference< XAccessibleRelationSet > xRelSet = new utl::AccessibleRelationSetHelper();
    return xRelSet;   // empty relation set
}

Reference< XAccessibleStateSet > SAL_CALL SmGraphicAccessible::getAccessibleStateSet()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    ::utl::AccessibleStateSetHelper *pStateSet =
            new ::utl::AccessibleStateSetHelper;

    Reference<XAccessibleStateSet> xStateSet( pStateSet );

    if (!pWin)
        pStateSet->AddState( AccessibleStateType::DEFUNC );
    else
    {
        //pStateSet->AddState( AccessibleStateType::EDITABLE );
        //pStateSet->AddState( AccessibleStateType::HORIZONTAL );
        //pStateSet->AddState( AccessibleStateType::TRANSIENT );
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
    throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    // should be the document language...
    // We use the language of the localized symbol names here.
    return Application::GetSettings().GetUILocale();
}


void SAL_CALL SmGraphicAccessible::addEventListener(
        const Reference< XAccessibleEventListener >& xListener )
    throw (RuntimeException)
{
    if (xListener.is())
    {
        vos::OGuard aGuard(Application::GetSolarMutex());
        if (pWin)
        {
            if (!nClientId)
                nClientId = comphelper::AccessibleEventNotifier::registerClient( );
            comphelper::AccessibleEventNotifier::addEventListener( nClientId, xListener );
        }
    }
}

void SAL_CALL SmGraphicAccessible::removeEventListener(
        const Reference< XAccessibleEventListener >& xListener )
    throw (RuntimeException)
{
    if (xListener.is())
    {
        vos::OGuard aGuard(Application::GetSolarMutex());
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
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return 0;
}

sal_Bool SAL_CALL SmGraphicAccessible::setCaretPosition( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    xub_StrLen nIdx = (xub_StrLen) nIndex;
    String aTxt( GetAccessibleText_Impl() );
    if (!(/*0 <= nIdx  &&*/  nIdx < aTxt.Len()))
        throw IndexOutOfBoundsException();
    return sal_False;
}

sal_Unicode SAL_CALL SmGraphicAccessible::getCharacter( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    xub_StrLen nIdx = (xub_StrLen) nIndex;
    String aTxt( GetAccessibleText_Impl() );
    if (!(/*0 <= nIdx  &&*/  nIdx < aTxt.Len()))
        throw IndexOutOfBoundsException();
    return aTxt.GetChar( nIdx );
}

Sequence< beans::PropertyValue > SAL_CALL SmGraphicAccessible::getCharacterAttributes(
        sal_Int32 nIndex,
        const uno::Sequence< ::rtl::OUString > & /*rRequestedAttributes*/ )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int32 nLen = GetAccessibleText_Impl().Len();
    if (!(0 <= nIndex  &&  nIndex < nLen))
        throw IndexOutOfBoundsException();
    return Sequence< beans::PropertyValue >();
}

awt::Rectangle SAL_CALL SmGraphicAccessible::getCharacterBounds( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    awt::Rectangle aRes;

    if (!pWin)
        throw RuntimeException();
    else
    {
        // get accessible text
        SmViewShell *pView = pWin->GetView();
        SmDocShell  *pDoc  = pView ? pView->GetDoc() : 0;
        if (!pDoc)
            throw RuntimeException();
        String aTxt( GetAccessibleText_Impl() );
        if (!(0 <= nIndex  &&  nIndex <= aTxt.Len()))   // #108812# aTxt.Len() is valid
            throw IndexOutOfBoundsException();

        // #108812# find a reasonable rectangle for position aTxt.Len().
        bool bWasBehindText = (nIndex == aTxt.Len());
        if (bWasBehindText && nIndex)
            --nIndex;

        const SmNode *pTree = pDoc->GetFormulaTree();
        const SmNode *pNode = pTree->FindNodeWithAccessibleIndex( (xub_StrLen) nIndex );
        //! pNode may be 0 if the index belongs to a char that was inserted
        //! only for the accessible text!
        if (pNode)
        {
            sal_Int32 nAccIndex = pNode->GetAccessibleIndex();
            DBG_ASSERT( nAccIndex >= 0, "invalid accessible index" );
            DBG_ASSERT( nIndex >= nAccIndex, "index out of range" );

            String    aNodeText;
            pNode->GetAccessibleText( aNodeText );
            sal_Int32 nNodeIndex = nIndex - nAccIndex;
            if (0 <= nNodeIndex  &&  nNodeIndex < aNodeText.Len())
            {
                // get appropriate rectangle
                Point aOffset(pNode->GetTopLeft() - pTree->GetTopLeft());
                Point aTLPos (pWin->GetFormulaDrawPos() + aOffset);
//                aTLPos.X() -= pNode->GetItalicLeftSpace();
//                Size  aSize (pNode->GetItalicSize());
                aTLPos.X() -= 0;
                Size  aSize (pNode->GetSize());

                sal_Int32 *pXAry = new sal_Int32[ aNodeText.Len() ];
                pWin->SetFont( pNode->GetFont() );
                pWin->GetTextArray( aNodeText, pXAry, 0, aNodeText.Len() );
                aTLPos.X()    += nNodeIndex > 0 ? pXAry[nNodeIndex - 1] : 0;
                aSize.Width()  = nNodeIndex > 0 ? pXAry[nNodeIndex] - pXAry[nNodeIndex - 1] : pXAry[nNodeIndex];
                delete[] pXAry;

#if OSL_DEBUG_LEVEL > 1
    Point aLP00( pWin->LogicToPixel( Point(0,0)) );
    Point aPL00( pWin->PixelToLogic( Point(0,0)) );
#endif
                aTLPos = pWin->LogicToPixel( aTLPos );
                aSize  = pWin->LogicToPixel( aSize );
                aRes.X = aTLPos.X();
                aRes.Y = aTLPos.Y();
                aRes.Width  = aSize.Width();
                aRes.Height = aSize.Height();
            }
        }

        // #108812# take rectangle from last character and move it to the right
        if (bWasBehindText)
            aRes.X += aRes.Width;
    }

    return aRes;
}

sal_Int32 SAL_CALL SmGraphicAccessible::getCharacterCount()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return GetAccessibleText_Impl().Len();
}

sal_Int32 SAL_CALL SmGraphicAccessible::getIndexAtPoint( const awt::Point& aPoint )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    sal_Int32 nRes = -1;
    if (pWin)
    {
        const SmNode *pTree = pWin->GetView()->GetDoc()->GetFormulaTree();
        //! kann NULL sein! ZB wenn bereits beim laden des Dokuments (bevor der
        //! Parser angeworfen wurde) ins Fenster geklickt wird.
        if (!pTree)
            return nRes;

        // get position relativ to formula draw position
        Point  aPos( aPoint.X, aPoint.Y );
        aPos = pWin->PixelToLogic( aPos );
        aPos -= pWin->GetFormulaDrawPos();

        // if it was inside the formula then get the appropriate node
        const SmNode *pNode = 0;
        if (pTree->OrientedDist(aPos) <= 0)
            pNode = pTree->FindRectClosestTo(aPos);

        if (pNode)
        {
            // get appropriate rectangle
            Point   aOffset( pNode->GetTopLeft() - pTree->GetTopLeft() );
            Point   aTLPos ( /*pWin->GetFormulaDrawPos() +*/ aOffset );
//            aTLPos.X() -= pNode->GetItalicLeftSpace();
//            Size  aSize( pNode->GetItalicSize() );
            aTLPos.X() -= 0;
            Size  aSize( pNode->GetSize() );
#if OSL_DEBUG_LEVEL > 1
    Point aLP00( pWin->LogicToPixel( Point(0,0)) );
    Point aPL00( pWin->PixelToLogic( Point(0,0)) );
#endif

            Rectangle aRect( aTLPos, aSize );
            if (aRect.IsInside( aPos ))
            {
                DBG_ASSERT( pNode->IsVisible(), "node is not a leaf" );
                String aTxt;
                pNode->GetAccessibleText( aTxt );
                DBG_ASSERT( aTxt.Len(), "no accessible text available" );

                long nNodeX = pNode->GetLeft();

                sal_Int32 *pXAry = new sal_Int32[ aTxt.Len() ];
                pWin->SetFont( pNode->GetFont() );
                pWin->GetTextArray( aTxt, pXAry, 0, aTxt.Len() );
                for (sal_Int32 i = 0;  i < aTxt.Len()  &&  nRes == -1;  ++i)
                {
                    if (pXAry[i] + nNodeX > aPos.X())
                        nRes = i;
                }
                delete[] pXAry;
                DBG_ASSERT( nRes >= 0  &&  nRes < aTxt.Len(), "index out of range" );
                DBG_ASSERT( pNode->GetAccessibleIndex() >= 0,
                        "invalid accessible index" );

                nRes = pNode->GetAccessibleIndex() + nRes;
            }
        }
    }
    return nRes;
}

OUString SAL_CALL SmGraphicAccessible::getSelectedText()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return OUString();
}

sal_Int32 SAL_CALL SmGraphicAccessible::getSelectionStart()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return -1;
}

sal_Int32 SAL_CALL SmGraphicAccessible::getSelectionEnd()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return -1;
}

sal_Bool SAL_CALL SmGraphicAccessible::setSelection(
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int32 nLen = GetAccessibleText_Impl().Len();
    if (!(0 <= nStartIndex  &&  nStartIndex < nLen) ||
        !(0 <= nEndIndex    &&  nEndIndex   < nLen))
        throw IndexOutOfBoundsException();
    return sal_False;
}

OUString SAL_CALL SmGraphicAccessible::getText()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return GetAccessibleText_Impl();
}

OUString SAL_CALL SmGraphicAccessible::getTextRange(
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    //!! nEndIndex may be the string length per definition of the interface !!
    //!! text should be copied exclusive that end index though. And arguments
    //!! may be switched.

    vos::OGuard aGuard(Application::GetSolarMutex());
    String aTxt( GetAccessibleText_Impl() );
    xub_StrLen nStart = (xub_StrLen) Min(nStartIndex, nEndIndex);
    xub_StrLen nEnd   = (xub_StrLen) Max(nStartIndex, nEndIndex);
    if (!(/*0 <= nStart  &&*/  nStart <= aTxt.Len()) ||
        !(/*0 <= nEnd    &&*/  nEnd   <= aTxt.Len()))
        throw IndexOutOfBoundsException();
    return aTxt.Copy( nStart, nEnd - nStart );
}

::com::sun::star::accessibility::TextSegment SAL_CALL SmGraphicAccessible::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    String aTxt( GetAccessibleText_Impl() );
    xub_StrLen nIdx = (xub_StrLen) nIndex;
    //!! nIndex is allowed to be the string length
    if (!(/*0 <= nIdx  &&*/  nIdx <= aTxt.Len()))
        throw IndexOutOfBoundsException();

    ::com::sun::star::accessibility::TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;
    if ( (AccessibleTextType::CHARACTER == aTextType)  &&  (nIdx < aTxt.Len()) )
    {
        aResult.SegmentText = aTxt.Copy(nIdx, 1);
        aResult.SegmentStart = nIdx;
        aResult.SegmentEnd = nIdx+1;
    }
    return aResult;
}

::com::sun::star::accessibility::TextSegment SAL_CALL SmGraphicAccessible::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    String aTxt( GetAccessibleText_Impl() );
    xub_StrLen nIdx = (xub_StrLen) nIndex;
    //!! nIndex is allowed to be the string length
    if (!(/*0 <= nIdx  &&*/  nIdx <= aTxt.Len()))
        throw IndexOutOfBoundsException();

    ::com::sun::star::accessibility::TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;

    if ( (AccessibleTextType::CHARACTER == aTextType)  && nIdx )
    {
        aResult.SegmentText = aTxt.Copy(nIdx-1, 1);
        aResult.SegmentStart = nIdx-1;
        aResult.SegmentEnd = nIdx;
    }
    return aResult;
}

::com::sun::star::accessibility::TextSegment SAL_CALL SmGraphicAccessible::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    String aTxt( GetAccessibleText_Impl() );
    xub_StrLen nIdx = (xub_StrLen) nIndex;
    //!! nIndex is allowed to be the string length
    if (!(/*0 <= nIdx  &&*/  nIdx <= aTxt.Len()))
        throw IndexOutOfBoundsException();

    ::com::sun::star::accessibility::TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;

    nIdx++; // text *behind*
    if ( (AccessibleTextType::CHARACTER == aTextType)  &&  (nIdx < aTxt.Len()) )
    {
        aResult.SegmentText = aTxt.Copy(nIdx, 1);
        aResult.SegmentStart = nIdx;
        aResult.SegmentEnd = nIdx+1;
    }
    return aResult;
}

sal_Bool SAL_CALL SmGraphicAccessible::copyText(
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bReturn = sal_False;

    if (!pWin)
        throw RuntimeException();
    else
    {
        Reference< datatransfer::clipboard::XClipboard > xClipboard = pWin->GetClipboard();
        if ( xClipboard.is() )
        {
            ::rtl::OUString sText( getTextRange(nStartIndex, nEndIndex) );

            ::vcl::unohelper::TextDataObject* pDataObj = new ::vcl::unohelper::TextDataObject( sText );
            const sal_uInt32 nRef = Application::ReleaseSolarMutex();
            xClipboard->setContents( pDataObj, NULL );

            Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( xClipboard, uno::UNO_QUERY );
            if( xFlushableClipboard.is() )
                xFlushableClipboard->flushClipboard();

            Application::AcquireSolarMutex( nRef );

            bReturn = sal_True;
        }
    }

    return bReturn;
}

OUString SAL_CALL SmGraphicAccessible::getImplementationName()
    throw (RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    return A2OU("SmGraphicAccessible");
}

sal_Bool SAL_CALL SmGraphicAccessible::supportsService(
        const OUString& rServiceName )
    throw (RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    return  rServiceName == A2OU( "com::sun::star::accessibility::Accessible" ) ||
            rServiceName == A2OU( "com::sun::star::accessibility::AccessibleComponent" ) ||
            rServiceName == A2OU( "com::sun::star::accessibility::AccessibleContext" ) ||
            rServiceName == A2OU( "com::sun::star::accessibility::AccessibleText" );
}

Sequence< OUString > SAL_CALL SmGraphicAccessible::getSupportedServiceNames()
    throw (RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< OUString > aNames(4);
    OUString *pNames = aNames.getArray();
    pNames[0] = A2OU( "com::sun::star::accessibility::Accessible" );
    pNames[1] = A2OU( "com::sun::star::accessibility::AccessibleComponent" );
    pNames[2] = A2OU( "com::sun::star::accessibility::AccessibleContext" );
    pNames[3] = A2OU( "com::sun::star::accessibility::AccessibleText" );
    return aNames;
}

//////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------

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
    //aBroadCaster;     can be completely new
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

SvxEditViewForwarder* SmEditSource::GetEditViewForwarder( sal_Bool /*bCreate*/ )
{
    return &aEditViewFwd;
}

void SmEditSource::UpdateData()
{
    // would possibly only by needed if the XText inteface is implemented
    // and its text needs to be updated.
}

SfxBroadcaster & SmEditSource::GetBroadcaster() const
{
    return ((SmEditSource *) this)->aBroadCaster;
}

//------------------------------------------------------------------------

SmViewForwarder::SmViewForwarder( SmEditAccessible &rAcc ) :
    rEditAcc(rAcc)
{
}

SmViewForwarder::~SmViewForwarder()
{
}

sal_Bool SmViewForwarder::IsValid() const
{
    return rEditAcc.GetEditView() != 0;
}

Rectangle SmViewForwarder::GetVisArea() const
{
    EditView *pEditView = rEditAcc.GetEditView();
    OutputDevice* pOutDev = pEditView ? pEditView->GetWindow() : 0;

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
    OutputDevice* pOutDev = pEditView ? pEditView->GetWindow() : 0;

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
    OutputDevice* pOutDev = pEditView ? pEditView->GetWindow() : 0;

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


//------------------------------------------------------------------------

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
        pEditEngine->SetNotifyHdl( Link() );
}

IMPL_LINK(SmTextForwarder, NotifyHdl, EENotify*, aNotify)
{
    if (aNotify)
    {
        ::std::auto_ptr< SfxHint > aHint = SvxEditSourceHelper::EENotification2Hint( aNotify );
        if (aHint.get())
            rEditSource.GetBroadcaster().Broadcast( *aHint.get() );
    }

    return 0;
}

sal_uInt16 SmTextForwarder::GetParagraphCount() const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetParagraphCount() : 0;
}

sal_uInt16 SmTextForwarder::GetTextLen( sal_uInt16 nParagraph ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetTextLen( nParagraph ) : 0;
}

String SmTextForwarder::GetText( const ESelection& rSel ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    String aRet;
    if (pEditEngine)
        aRet = pEditEngine->GetText( rSel, LINEEND_LF );
    aRet.ConvertLineEnd();
    return aRet;
}

SfxItemSet SmTextForwarder::GetAttribs( const ESelection& rSel, sal_Bool bOnlyHardAttrib ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    DBG_ASSERT( pEditEngine, "EditEngine missing" );
    if( rSel.nStartPara == rSel.nEndPara )
    {
        sal_uInt8 nFlags = 0;
        switch( bOnlyHardAttrib )
        {
        case EditEngineAttribs_All:
            nFlags = GETATTRIBS_ALL;
            break;
        case EditEngineAttribs_HardAndPara:
            nFlags = GETATTRIBS_PARAATTRIBS|GETATTRIBS_CHARATTRIBS;
            break;
        case EditEngineAttribs_OnlyHard:
            nFlags = GETATTRIBS_CHARATTRIBS;
            break;
        default:
            DBG_ERROR("unknown flags for SmTextForwarder::GetAttribs");
        }

        return pEditEngine->GetAttribs( rSel.nStartPara, rSel.nStartPos, rSel.nEndPos, nFlags );
    }
    else
    {
        return pEditEngine->GetAttribs( rSel, bOnlyHardAttrib );
    }
}

SfxItemSet SmTextForwarder::GetParaAttribs( sal_uInt16 nPara ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    DBG_ASSERT( pEditEngine, "EditEngine missing" );

    SfxItemSet aSet( pEditEngine->GetParaAttribs( nPara ) );

    sal_uInt16 nWhich = EE_PARA_START;
    while( nWhich <= EE_PARA_END )
    {
        if( aSet.GetItemState( nWhich, sal_True ) != SFX_ITEM_ON )
        {
            if( pEditEngine->HasParaAttrib( nPara, nWhich ) )
                aSet.Put( pEditEngine->GetParaAttrib( nPara, nWhich ) );
        }
        nWhich++;
    }

    return aSet;
}

void SmTextForwarder::SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rSet )
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetParaAttribs( nPara, rSet );
}

SfxItemPool* SmTextForwarder::GetPool() const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetEmptyItemSet().GetPool() : 0;
}

void SmTextForwarder::RemoveAttribs( const ESelection& rSelection, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->RemoveAttribs( rSelection, bRemoveParaAttribs, nWhich );
}

void SmTextForwarder::GetPortions( sal_uInt16 nPara, SvUShorts& rList ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        pEditEngine->GetPortions( nPara, rList );
}

void SmTextForwarder::QuickInsertText( const String& rText, const ESelection& rSel )
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

sal_Bool SmTextForwarder::IsValid() const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    // cannot reliably query EditEngine state
    // while in the middle of an update
    return pEditEngine ? pEditEngine->GetUpdateMode() : sal_False;
}

XubString SmTextForwarder::CalcFieldValue( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    XubString aTxt;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        aTxt = pEditEngine->CalcFieldValue( rField, nPara, nPos, rpTxtColor, rpFldColor );
    return aTxt;
}

void SmTextForwarder::FieldClicked(const SvxFieldItem&, sal_uInt16, sal_uInt16)
{
}

sal_uInt16 GetSvxEditEngineItemState( EditEngine& rEditEngine, const ESelection& rSel, sal_uInt16 nWhich )
{
    EECharAttribArray aAttribs;

    const SfxPoolItem*  pLastItem = NULL;

    SfxItemState eState = SFX_ITEM_DEFAULT;

    // check all paragraphs inside the selection
    for( sal_uInt16 nPara = rSel.nStartPara; nPara <= rSel.nEndPara; nPara++ )
    {
        SfxItemState eParaState = SFX_ITEM_DEFAULT;

        // calculate start and endpos for this paragraph
        sal_uInt16 nPos = 0;
        if( rSel.nStartPara == nPara )
            nPos = rSel.nStartPos;

        sal_uInt16 nEndPos = rSel.nEndPos;
        if( rSel.nEndPara != nPara )
            nEndPos = rEditEngine.GetTextLen( nPara );


        // get list of char attribs
        rEditEngine.GetCharAttribs( nPara, aAttribs );

        sal_Bool bEmpty = sal_True;     // we found no item inside the selektion of this paragraph
        sal_Bool bGaps  = sal_False;    // we found items but theire gaps between them
        sal_uInt16 nLastEnd = nPos;

        const SfxPoolItem* pParaItem = NULL;

        for( sal_uInt16 nAttrib = 0; nAttrib < aAttribs.Count(); nAttrib++ )
        {
            struct EECharAttrib aAttrib = aAttribs.GetObject( nAttrib );
            DBG_ASSERT( aAttrib.pAttr, "GetCharAttribs gives corrupt data" );

            const sal_Bool bEmptyPortion = aAttrib.nStart == aAttrib.nEnd;
            if( (!bEmptyPortion && (aAttrib.nStart >= nEndPos)) || (bEmptyPortion && (aAttrib.nStart > nEndPos)) )
                break;  // break if we are already behind our selektion

            if( (!bEmptyPortion && (aAttrib.nEnd <= nPos)) || (bEmptyPortion && (aAttrib.nEnd < nPos)) )
                continue;   // or if the attribute ends before our selektion

            if( aAttrib.pAttr->Which() != nWhich )
                continue; // skip if is not the searched item

            // if we already found an item
            if( pParaItem )
            {
                // ... and its different to this one than the state is dont care
                if( *pParaItem != *aAttrib.pAttr )
                    return SFX_ITEM_DONTCARE;
            }
            else
            {
                pParaItem = aAttrib.pAttr;
            }

            if( bEmpty )
                bEmpty = sal_False;

            if( !bGaps && aAttrib.nStart > nLastEnd )
                bGaps = sal_True;

            nLastEnd = aAttrib.nEnd;
        }

        if( !bEmpty && !bGaps && nLastEnd < ( nEndPos - 1 ) )
            bGaps = sal_True;
/*
        // since we have no portion with our item or if there were gaps
        if( bEmpty || bGaps )
        {
            // we need to check the paragraph item
            const SfxItemSet& rParaSet = rEditEngine.GetParaAttribs( nPara );
            if( rParaSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                eState = SFX_ITEM_SET;
                // get item from the paragraph
                const SfxPoolItem* pTempItem = rParaSet.GetItem( nWhich );
                if( pParaItem )
                {
                    if( *pParaItem != *pTempItem )
                        return SFX_ITEM_DONTCARE;
                }
                else
                {
                    pParaItem = pTempItem;
                }

                // set if theres no last item or if its the same
                eParaState = SFX_ITEM_SET;
            }
            else if( bEmpty )
            {
                eParaState = SFX_ITEM_DEFAULT;
            }
            else if( bGaps )
            {
                // gaps and item not set in paragraph, thats a dont care
                return SFX_ITEM_DONTCARE;
            }
        }
        else
        {
            eParaState = SFX_ITEM_SET;
        }
*/
        if( bEmpty )
            eParaState = SFX_ITEM_DEFAULT;
        else if( bGaps )
            eParaState = SFX_ITEM_DONTCARE;
        else
            eParaState = SFX_ITEM_SET;

        // if we already found an item check if we found the same
        if( pLastItem )
        {
            if( (pParaItem == NULL) || (*pLastItem != *pParaItem) )
                return SFX_ITEM_DONTCARE;
        }
        else
        {
            pLastItem = pParaItem;
            eState = eParaState;
        }
    }

    return eState;
}

sal_uInt16 SmTextForwarder::GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const
{
    sal_uInt16 nState = SFX_ITEM_DISABLED;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
        nState = GetSvxEditEngineItemState( *pEditEngine, rSel, nWhich );
    return nState;
}

sal_uInt16 SmTextForwarder::GetItemState( sal_uInt16 nPara, sal_uInt16 nWhich ) const
{
    sal_uInt16 nState = SFX_ITEM_DISABLED;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        const SfxItemSet& rSet = pEditEngine->GetParaAttribs( nPara );
        nState = rSet.GetItemState( nWhich );
    }
    return nState;
}

LanguageType SmTextForwarder::GetLanguage( sal_uInt16 nPara, sal_uInt16 nIndex ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetLanguage(nPara, nIndex) : LANGUAGE_NONE;
}

sal_uInt16 SmTextForwarder::GetFieldCount( sal_uInt16 nPara ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetFieldCount(nPara) : 0;
}

EFieldInfo SmTextForwarder::GetFieldInfo( sal_uInt16 nPara, sal_uInt16 nField ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetFieldInfo( nPara, nField ) : EFieldInfo();
}

EBulletInfo SmTextForwarder::GetBulletInfo( sal_uInt16 /*nPara*/ ) const
{
    return EBulletInfo();
}

Rectangle SmTextForwarder::GetCharBounds( sal_uInt16 nPara, sal_uInt16 nIndex ) const
{
    Rectangle aRect(0,0,0,0);
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();

    if (pEditEngine)
    {
        // #108900# Handle virtual position one-past-the end of the string
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

Rectangle SmTextForwarder::GetParaBounds( sal_uInt16 nPara ) const
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
    return pEditEngine ? pEditEngine->GetRefMapMode() : MapMode( MAP_100TH_MM );
}

OutputDevice* SmTextForwarder::GetRefDevice() const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetRefDevice() : 0;
}

sal_Bool SmTextForwarder::GetIndexAtPoint( const Point& rPos, sal_uInt16& nPara, sal_uInt16& nIndex ) const
{
    sal_Bool bRes = sal_False;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        EPosition aDocPos = pEditEngine->FindDocPosition( rPos );
        nPara   = aDocPos.nPara;
        nIndex  = aDocPos.nIndex;
        bRes = sal_True;
    }
    return bRes;
}

sal_Bool SmTextForwarder::GetWordIndices( sal_uInt16 nPara, sal_uInt16 nIndex, sal_uInt16& nStart, sal_uInt16& nEnd ) const
{
    sal_Bool bRes = sal_False;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        ESelection aRes = pEditEngine->GetWord( ESelection(nPara, nIndex, nPara, nIndex), com::sun::star::i18n::WordType::DICTIONARY_WORD );

        if( aRes.nStartPara == nPara &&
            aRes.nStartPara == aRes.nEndPara )
        {
            nStart = aRes.nStartPos;
            nEnd = aRes.nEndPos;

            bRes = sal_True;
        }
    }

    return bRes;
}

sal_Bool SmTextForwarder::GetAttributeRun( sal_uInt16& nStartIndex, sal_uInt16& nEndIndex, sal_uInt16 nPara, sal_uInt16 nIndex ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ?
                SvxEditSourceHelper::GetAttributeRun( nStartIndex, nEndIndex, *pEditEngine, nPara, nIndex )
                : sal_False;
}

sal_uInt16 SmTextForwarder::GetLineCount( sal_uInt16 nPara ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetLineCount(nPara) : 0;
}

sal_uInt16 SmTextForwarder::GetLineLen( sal_uInt16 nPara, sal_uInt16 nLine ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetLineLen(nPara, nLine) : 0;
}

void SmTextForwarder::GetLineBoundaries( /*out*/sal_uInt16 &rStart, /*out*/sal_uInt16 &rEnd, sal_uInt16 nPara, sal_uInt16 nLine ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    pEditEngine->GetLineBoundaries(rStart, rEnd, nPara, nLine);
}

sal_uInt16 SmTextForwarder::GetLineNumberAtIndex( sal_uInt16 nPara, sal_uInt16 nIndex ) const
{
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    return pEditEngine ? pEditEngine->GetLineNumberAtIndex(nPara, nIndex) : 0;
}

sal_Bool SmTextForwarder::QuickFormatDoc( sal_Bool /*bFull*/ )
{
    sal_Bool bRes = sal_False;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        pEditEngine->QuickFormatDoc();
        bRes = sal_True;
    }
    return bRes;
}

sal_Int16 SmTextForwarder::GetDepth( sal_uInt16 /*nPara*/ ) const
{
    // math has no outliner...
    return -1;
}

sal_Bool SmTextForwarder::SetDepth( sal_uInt16 /*nPara*/, sal_Int16 nNewDepth )
{
    // math has no outliner...
    return -1 == nNewDepth;  // is it the value from 'GetDepth' ?
}

sal_Bool SmTextForwarder::Delete( const ESelection& rSelection )
{
    sal_Bool bRes = sal_False;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        pEditEngine->QuickDelete( rSelection );
        pEditEngine->QuickFormatDoc();
        bRes = sal_True;
    }
    return bRes;
}

sal_Bool SmTextForwarder::InsertText( const String& rStr, const ESelection& rSelection )
{
    sal_Bool bRes = sal_False;
    EditEngine *pEditEngine = rEditAcc.GetEditEngine();
    if (pEditEngine)
    {
        pEditEngine->QuickInsertText( rStr, rSelection );
        pEditEngine->QuickFormatDoc();
        bRes = sal_True;
    }
    return bRes;
}

const SfxItemSet*   SmTextForwarder::GetEmptyItemSetPtr()
{
    const SfxItemSet *pItemSet = 0;
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
        sal_uInt16 nParaCount = pEditEngine->GetParagraphCount();
        pEditEngine->InsertParagraph( nParaCount, String() );
    }
}

xub_StrLen SmTextForwarder::AppendTextPortion( sal_uInt16 nPara, const String &rText, const SfxItemSet &rSet )
{
    xub_StrLen nRes = 0;
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

//------------------------------------------------------------------------

SmEditViewForwarder::SmEditViewForwarder( SmEditAccessible& rAcc ) :
    rEditAcc( rAcc )
{
}

SmEditViewForwarder::~SmEditViewForwarder()
{
}

sal_Bool SmEditViewForwarder::IsValid() const
{
    return rEditAcc.GetEditView() != 0;
}

Rectangle SmEditViewForwarder::GetVisArea() const
{
    Rectangle aRect(0,0,0,0);

    EditView *pEditView = rEditAcc.GetEditView();
    OutputDevice* pOutDev = pEditView ? pEditView->GetWindow() : 0;

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
    OutputDevice* pOutDev = pEditView ? pEditView->GetWindow() : 0;

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
    OutputDevice* pOutDev = pEditView ? pEditView->GetWindow() : 0;

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

sal_Bool SmEditViewForwarder::GetSelection( ESelection& rSelection ) const
{
    sal_Bool bRes = sal_False;
    EditView *pEditView = rEditAcc.GetEditView();
    if (pEditView)
    {
        rSelection = pEditView->GetSelection();
        bRes = sal_True;
    }
    return bRes;
}

sal_Bool SmEditViewForwarder::SetSelection( const ESelection& rSelection )
{
    sal_Bool bRes = sal_False;
    EditView *pEditView = rEditAcc.GetEditView();
    if (pEditView)
    {
        pEditView->SetSelection( rSelection );
        bRes = sal_True;
    }
    return bRes;
}

sal_Bool SmEditViewForwarder::Copy()
{
    sal_Bool bRes = sal_False;
    EditView *pEditView = rEditAcc.GetEditView();
    if (pEditView)
    {
        pEditView->Copy();
        bRes = sal_True;
    }
    return bRes;
}

sal_Bool SmEditViewForwarder::Cut()
{
    sal_Bool bRes = sal_False;
    EditView *pEditView = rEditAcc.GetEditView();
    if (pEditView)
    {
        pEditView->Cut();
        bRes = sal_True;
    }
    return bRes;
}

sal_Bool SmEditViewForwarder::Paste()
{
    sal_Bool bRes = sal_False;
    EditView *pEditView = rEditAcc.GetEditView();
    if (pEditView)
    {
        pEditView->Paste();
        bRes = sal_True;
    }
    return bRes;
}

//------------------------------------------------------------------------

SmEditAccessible::SmEditAccessible( SmEditWindow *pEditWin ) :
    aAccName            ( String(SmResId(STR_CMDBOXWINDOW)) ),
    pTextHelper         (0),
    pWin                (pEditWin)
{
    DBG_ASSERT( pWin, "SmEditAccessible: window missing" );
    //++aRefCount;
}


SmEditAccessible::SmEditAccessible( const SmEditAccessible &rSmAcc ) :
    SmEditAccessibleBaseClass(),
    aAccName            ( String(SmResId(STR_CMDBOXWINDOW)) )
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    pWin = rSmAcc.pWin;
    DBG_ASSERT( pWin, "SmEditAccessible: window missing" );
    //++aRefCount;
}

SmEditAccessible::~SmEditAccessible()
{
    delete pTextHelper;
/*
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (--aRefCount == 0)
    {
    }
*/
}

void SmEditAccessible::Init()
{
    DBG_ASSERT( pWin, "SmEditAccessible: window missing" );
    if (pWin)
    {
        EditEngine *pEditEngine = pWin->GetEditEngine();
        EditView   *pEditView   = pWin->GetEditView();
        if (pEditEngine && pEditView)
        {
            ::std::auto_ptr< SvxEditSource > pEditSource(
                    new SmEditSource( pWin, *this ) );
            pTextHelper = new ::accessibility::AccessibleTextHelper( pEditSource );
            pTextHelper->SetEventSource( this );
        }
    }
}

#ifdef TL_NOT_YET_USED
SmDocShell * SmEditAccessible::GetDoc_Impl()
{
    SmViewShell *pView = pWin ? pWin->GetView() : 0;
    return pView ? pView->GetDoc() : 0;
}
#endif // TL_NOT_YET_USED

void SmEditAccessible::ClearWin()
{
    // #112565# remove handler before current object gets destroyed
    // (avoid handler being called for already dead object)
    EditEngine *pEditEngine = GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetNotifyHdl( Link() );

    pWin = 0;   // implicitly results in AccessibleStateType::DEFUNC set

    //! make TextHelper implicitly release C++ references to some core objects
    pTextHelper->SetEditSource( ::std::auto_ptr<SvxEditSource>(NULL) );
    //! make TextHelper release references
    //! (e.g. the one set by the 'SetEventSource' call)
    pTextHelper->Dispose();
    delete pTextHelper;     pTextHelper = 0;
}

// XAccessible
uno::Reference< XAccessibleContext > SAL_CALL SmEditAccessible::getAccessibleContext(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return this;
}

// XAccessibleComponent
sal_Bool SAL_CALL SmEditAccessible::containsPoint( const awt::Point& aPoint )
    throw (RuntimeException)
{
    //! the arguments coordinates are relativ to the current window !
    //! Thus the top left-point is (0, 0)

    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    Size aSz( pWin->GetSizePixel() );
    return  aPoint.X >= 0  &&  aPoint.Y >= 0  &&
            aPoint.X < aSz.Width()  &&  aPoint.Y < aSz.Height();
}

uno::Reference< XAccessible > SAL_CALL SmEditAccessible::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pTextHelper)
        throw RuntimeException();
    return pTextHelper->GetAt( aPoint );
}

awt::Rectangle SAL_CALL SmEditAccessible::getBounds(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();
    DBG_ASSERT(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );
    return lcl_GetBounds( pWin );
}

awt::Point SAL_CALL SmEditAccessible::getLocation(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();
    DBG_ASSERT(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );
    awt::Rectangle aRect( lcl_GetBounds( pWin ) );
    return awt::Point( aRect.X, aRect.Y );
}

awt::Point SAL_CALL SmEditAccessible::getLocationOnScreen(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();
    DBG_ASSERT(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );
    return lcl_GetLocationOnScreen( pWin );
}

awt::Size SAL_CALL SmEditAccessible::getSize(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();
    DBG_ASSERT(pWin->GetParent()->GetAccessible() == getAccessibleParent(),
            "mismatch of window parent and accessible parent" );

    Size aSz( pWin->GetSizePixel() );
#if OSL_DEBUG_LEVEL > 1
    awt::Rectangle aRect( lcl_GetBounds( pWin ) );
    Size aSz2( aRect.Width, aRect.Height );
    DBG_ASSERT( aSz == aSz2, "mismatch in width" );
#endif
    return awt::Size( aSz.Width(), aSz.Height() );
}

void SAL_CALL SmEditAccessible::grabFocus(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    pWin->GrabFocus();
}

sal_Int32 SAL_CALL SmEditAccessible::getForeground()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!pWin)
        throw RuntimeException();
    return (sal_Int32) pWin->GetTextColor().GetColor();
}

sal_Int32 SAL_CALL SmEditAccessible::getBackground()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!pWin)
        throw RuntimeException();
    Wallpaper aWall( pWin->GetDisplayBackground() );
    ColorData nCol;
    if (aWall.IsBitmap() || aWall.IsGradient())
        nCol = pWin->GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    else
        nCol = aWall.GetColor().GetColor();
    return (sal_Int32) nCol;
}

// XAccessibleContext
sal_Int32 SAL_CALL SmEditAccessible::getAccessibleChildCount(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pTextHelper)
        throw RuntimeException();
    return pTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL SmEditAccessible::getAccessibleChild( sal_Int32 i )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pTextHelper)
        throw RuntimeException();
    return pTextHelper->GetChild( i );
}

uno::Reference< XAccessible > SAL_CALL SmEditAccessible::getAccessibleParent(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    Window *pAccParent = pWin->GetAccessibleParentWindow();
    DBG_ASSERT( pAccParent, "accessible parent missing" );
    return pAccParent ? pAccParent->GetAccessible() : Reference< XAccessible >();
}

sal_Int32 SAL_CALL SmEditAccessible::getAccessibleIndexInParent(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int32 nIdx = -1;
    Window *pAccParent = pWin ? pWin->GetAccessibleParentWindow() : 0;
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
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return AccessibleRole::PANEL /*TEXT ?*/;
}

rtl::OUString SAL_CALL SmEditAccessible::getAccessibleDescription(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return OUString();  // empty as agreed with product-management
}

rtl::OUString SAL_CALL SmEditAccessible::getAccessibleName(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    // same name as displayed by the window when not docked
    return aAccName;
}

uno::Reference< XAccessibleRelationSet > SAL_CALL SmEditAccessible::getAccessibleRelationSet(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Reference< XAccessibleRelationSet > xRelSet = new utl::AccessibleRelationSetHelper();
    return xRelSet;   // empty relation set
}

uno::Reference< XAccessibleStateSet > SAL_CALL SmEditAccessible::getAccessibleStateSet(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    ::utl::AccessibleStateSetHelper *pStateSet =
            new ::utl::AccessibleStateSetHelper;

    Reference<XAccessibleStateSet> xStateSet( pStateSet );

    if (!pWin || !pTextHelper)
        pStateSet->AddState( AccessibleStateType::DEFUNC );
    else
    {
        //pStateSet->AddState( AccessibleStateType::EDITABLE );
        pStateSet->AddState( AccessibleStateType::MULTI_LINE );
        //pStateSet->AddState( AccessibleStateType::HORIZONTAL );
        //pStateSet->AddState( AccessibleStateType::TRANSIENT );
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
    throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    // should be the document language...
    // We use the language of the localized symbol names here.
    return Application::GetSettings().GetUILocale();
}


// XAccessibleEventBroadcaster
void SAL_CALL SmEditAccessible::addEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    throw (RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());   if (pTextHelper)   // not disposing (about to destroy view shell)
        pTextHelper->AddEventListener( xListener );
}

void SAL_CALL SmEditAccessible::removeEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    throw (RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
   if (pTextHelper)   // not disposing (about to destroy view shell)
        pTextHelper->RemoveEventListener( xListener );
}

OUString SAL_CALL SmEditAccessible::getImplementationName()
    throw (RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    return A2OU("SmEditAccessible");
}

sal_Bool SAL_CALL SmEditAccessible::supportsService(
        const OUString& rServiceName )
    throw (RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    return  rServiceName == A2OU( "com::sun::star::accessibility::Accessible" ) ||
            rServiceName == A2OU( "com::sun::star::accessibility::AccessibleComponent" ) ||
            rServiceName == A2OU( "com::sun::star::accessibility::AccessibleContext" );
}

Sequence< OUString > SAL_CALL SmEditAccessible::getSupportedServiceNames()
    throw (RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< OUString > aNames(3);
    OUString *pNames = aNames.getArray();
    pNames[0] = A2OU( "com::sun::star::accessibility::Accessible" );
    pNames[1] = A2OU( "com::sun::star::accessibility::AccessibleComponent" );
    pNames[2] = A2OU( "com::sun::star::accessibility::AccessibleContext" );
    return aNames;
}

//////////////////////////////////////////////////////////////////////

