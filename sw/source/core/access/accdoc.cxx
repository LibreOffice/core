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

#include <vcl/window.hxx>
#include <rootfrm.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <viewsh.hxx>
#include <doc.hxx>
#include <accmap.hxx>
#include "accdoc.hxx"
#include <strings.hrc>
#include <pagefrm.hxx>

#include <swatrset.hxx>
#include <docsh.hxx>
#include <crsrsh.hxx>
#include <fesh.hxx>
#include <fmtclds.hxx>
#include <flyfrm.hxx>
#include <txtfrm.hxx>
#include <sectfrm.hxx>
#include <section.hxx>
#include <swmodule.hxx>
#include <svtools/colorcfg.hxx>

#include <fmtanchr.hxx>
#include <viewimp.hxx>
#include <dview.hxx>
#include <dcontact.hxx>
#include <svx/svdmark.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using lang::IndexOutOfBoundsException;

// SwAccessibleDocumentBase: base class for SwAccessibleDocument and
// SwAccessiblePreview

SwAccessibleDocumentBase::SwAccessibleDocumentBase(
        std::shared_ptr<SwAccessibleMap> const& pMap)
    : SwAccessibleContext(pMap, AccessibleRole::DOCUMENT_TEXT,
                          pMap->GetShell().GetLayout())
    , mxParent(pMap->GetShell().GetWin()->GetAccessibleParent())
    , mpChildWin(nullptr)
{
}

SwAccessibleDocumentBase::~SwAccessibleDocumentBase()
{
}

void SwAccessibleDocumentBase::SetVisArea()
{
    SolarMutexGuard aGuard;

    SwRect aOldVisArea( GetVisArea() );
    const SwRect& rNewVisArea = GetMap()->GetVisArea();
    if( aOldVisArea != rNewVisArea )
    {
        SwAccessibleFrame::SetVisArea( GetMap()->GetVisArea() );
        // #i58139# - showing state of document view needs also be updated.
        // Thus, call method <Scrolled(..)> instead of <ChildrenScrolled(..)>
        // ChildrenScrolled( GetFrame(), aOldVisArea );
        Scrolled( aOldVisArea );
    }
}

void SwAccessibleDocumentBase::AddChild( vcl::Window *pWin, bool bFireEvent )
{
    SolarMutexGuard aGuard;

    OSL_ENSURE( !mpChildWin, "only one child window is supported" );
    if( !mpChildWin )
    {
        mpChildWin = pWin;

        if( bFireEvent )
        {
            FireAccessibleEvent(AccessibleEventId::CHILD, uno::Any(),
                                uno::Any(uno::Reference<XAccessible>(mpChildWin->GetAccessible())));
        }
    }
}

void SwAccessibleDocumentBase::RemoveChild( vcl::Window *pWin )
{
    SolarMutexGuard aGuard;

    OSL_ENSURE( !mpChildWin || pWin == mpChildWin, "invalid child window to remove" );
    if( mpChildWin && pWin == mpChildWin )
    {
        FireAccessibleEvent(AccessibleEventId::CHILD,
                            uno::Any(uno::Reference<XAccessible>(mpChildWin->GetAccessible())),
                            uno::Any());

        mpChildWin = nullptr;
    }
}

sal_Int64 SAL_CALL SwAccessibleDocumentBase::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;

    // ThrowIfDisposed is called by parent

    sal_Int64 nChildren = SwAccessibleContext::getAccessibleChildCount();
    if( !IsDisposing() && mpChildWin )
        nChildren++;

    return nChildren;
}

uno::Reference< XAccessible> SAL_CALL
    SwAccessibleDocumentBase::getAccessibleChild( sal_Int64 nIndex )
{
    SolarMutexGuard aGuard;

    if( mpChildWin  )
    {
        ThrowIfDisposed();

        if ( nIndex == GetChildCount( *(GetMap()) ) )
        {
            return mpChildWin->GetAccessible();
        }
    }

    return SwAccessibleContext::getAccessibleChild( nIndex );
}

uno::Reference< XAccessible> SAL_CALL SwAccessibleDocumentBase::getAccessibleParent()
{
    return mxParent;
}

sal_Int64 SAL_CALL SwAccessibleDocumentBase::getAccessibleIndexInParent()
{
    SolarMutexGuard aGuard;

    uno::Reference < XAccessibleContext > xAcc( mxParent->getAccessibleContext() );
    uno::Reference < XAccessible > xThis( this );
    sal_Int64 nCount = xAcc->getAccessibleChildCount();

    for( sal_Int64 i=0; i < nCount; i++ )
    {
        try
        {
            if( xAcc->getAccessibleChild( i ) == xThis )
                return i;
        }
        catch(const css::lang::IndexOutOfBoundsException &)
        {
            return -1;
        }
    }
    return -1;
}

OUString SAL_CALL SwAccessibleDocumentBase::getAccessibleDescription()
{
    return GetResource( STR_ACCESS_DOC_DESC );
}

OUString SAL_CALL SwAccessibleDocumentBase::getAccessibleName()
{
    SolarMutexGuard g;

    OUString sAccName = GetResource( STR_ACCESS_DOC_WORDPROCESSING );
    SwDoc* pDoc = GetMap() ? GetShell().GetDoc() : nullptr;
    if ( pDoc )
    {
        OUString sFileName = pDoc->getDocAccTitle();
        if ( sFileName.isEmpty() )
        {
            SwDocShell* pDocSh = pDoc->GetDocShell();
            if ( pDocSh )
            {
                sFileName = pDocSh->GetTitle( SFX_TITLE_APINAME );
            }
        }

        if ( !sFileName.isEmpty() )
        {
            sAccName = sFileName + " - " + sAccName;
        }
    }

    return sAccName;
}

awt::Rectangle SwAccessibleDocumentBase::implGetBounds()
{
    try
    {
        vcl::Window *pWin = GetWindow();
        if (!pWin)
        {
            throw uno::RuntimeException(u"no Window"_ustr, getXWeak());
        }

        tools::Rectangle aPixBounds( pWin->GetWindowExtentsRelative( *pWin->GetAccessibleParentWindow() ) );
        return vcl::unohelper::ConvertToAWTRect(aPixBounds);
    }
    catch(const css::lang::IndexOutOfBoundsException &)
    {
        return awt::Rectangle();
    }
}

css::awt::Point SAL_CALL SwAccessibleDocumentBase::getLocationOnScreen()
{
    SolarMutexGuard aGuard;

    vcl::Window *pWin = GetWindow();
    if (!pWin)
    {
        throw uno::RuntimeException(u"no Window"_ustr, getXWeak());
    }

    Point aPixPos( pWin->GetWindowExtentsAbsolute().TopLeft() );
    awt::Point aLoc( aPixPos.getX(), aPixPos.getY() );

    return aLoc;
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleDocumentBase::getAccessibleAtPoint(
                const awt::Point& aPoint )
{
    SolarMutexGuard aGuard;

    if( mpChildWin  )
    {
        ThrowIfDisposed();

        vcl::Window *pWin = GetWindow();
        if (!pWin)
        {
            throw uno::RuntimeException(u"no Window"_ustr, getXWeak());
        }
        if (pWin->isDisposed()) // tdf#147967
            return nullptr;

        Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to window
        if( mpChildWin->GetWindowExtentsRelative( *pWin ).Contains( aPixPoint ) )
            return mpChildWin->GetAccessible();
    }

    return SwAccessibleContext::getAccessibleAtPoint( aPoint );
}

// SwAccessibleDocument

void SwAccessibleDocument::GetStates( sal_Int64& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // MULTISELECTABLE
    rStateSet |= AccessibleStateType::MULTI_SELECTABLE;
    rStateSet |= AccessibleStateType::MANAGES_DESCENDANTS;
}

SwAccessibleDocument::SwAccessibleDocument(
        std::shared_ptr<SwAccessibleMap> const& pInitMap)
    : SwAccessibleDocument_BASE(pInitMap)
    , maSelectionHelper(*this)
{
    SetName(pInitMap->GetDocName());
    vcl::Window* pWin = pInitMap->GetShell().GetWin();
    if( pWin )
    {
        pWin->AddChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
        sal_uInt16 nCount =   pWin->GetChildCount();
        for( sal_uInt16 i=0; i < nCount; i++ )
        {
            vcl::Window* pChildWin = pWin->GetChild( i );
            if( pChildWin &&
                AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                AddChild( pChildWin, false );
        }
    }
}

SwAccessibleDocument::~SwAccessibleDocument()
{
    vcl::Window* pWin = GetMap() ? GetMap()->GetShell().GetWin() : nullptr;
    if( pWin )
        pWin->RemoveChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
}

void SwAccessibleDocument::Dispose(bool bRecursive, bool bCanSkipInvisible)
{
    OSL_ENSURE( GetFrame() && GetMap(), "already disposed" );

    vcl::Window *pWin = GetMap() ? GetMap()->GetShell().GetWin() : nullptr;
    if( pWin )
        pWin->RemoveChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
    SwAccessibleContext::Dispose(bRecursive, bCanSkipInvisible);
}

IMPL_LINK( SwAccessibleDocument, WindowChildEventListener, VclWindowEvent&, rEvent, void )
{
    OSL_ENSURE( rEvent.GetWindow(), "Window???" );
    switch ( rEvent.GetId() )
    {
    case VclEventId::WindowShow:  // send create on show for direct accessible children
        {
            vcl::Window* pChildWin = static_cast< vcl::Window* >( rEvent.GetData() );
            if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
            {
                AddChild( pChildWin );
            }
        }
        break;
    case VclEventId::WindowHide:  // send destroy on hide for direct accessible children
        {
            vcl::Window* pChildWin = static_cast< vcl::Window* >( rEvent.GetData() );
            if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
            {
                RemoveChild( pChildWin );
            }
        }
        break;
    case VclEventId::ObjectDying:  // send destroy on hide for direct accessible children
        {
            vcl::Window* pChildWin = rEvent.GetWindow();
            if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
            {
                RemoveChild( pChildWin );
            }
        }
        break;
    default: break;
    }
}

// XAccessibleSelection

void SwAccessibleDocument::selectAccessibleChild(
    sal_Int64 nChildIndex )
{
    maSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleDocument::isAccessibleChildSelected(
    sal_Int64 nChildIndex )
{
    return maSelectionHelper.isAccessibleChildSelected(nChildIndex);
}

void SwAccessibleDocument::clearAccessibleSelection(  )
{
}

void SwAccessibleDocument::selectAllAccessibleChildren(  )
{
    maSelectionHelper.selectAllAccessibleChildren();
}

sal_Int64 SwAccessibleDocument::getSelectedAccessibleChildCount(  )
{
    return maSelectionHelper.getSelectedAccessibleChildCount();
}

uno::Reference<XAccessible> SwAccessibleDocument::getSelectedAccessibleChild(
    sal_Int64 nSelectedChildIndex )
{
    return maSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

// index has to be treated as global child index.
void SwAccessibleDocument::deselectAccessibleChild(
    sal_Int64 nChildIndex )
{
    maSelectionHelper.deselectAccessibleChild( nChildIndex );
}

OUString SAL_CALL SwAccessibleDocument::getExtendedAttributes()
{
    SolarMutexGuard g;

    SwDoc* pDoc = GetMap() ? GetShell().GetDoc() : nullptr;

    if (!pDoc)
        return OUString();
    SwCursorShell* pCursorShell = GetCursorShell();
    if( !pCursorShell )
        return OUString();

    SwFEShell* pFEShell = dynamic_cast<SwFEShell*>(pCursorShell);
    if (!pFEShell)
        return OUString();

    OUString sDisplay;
    sal_uInt16 nPage, nLogPage;
    pFEShell->GetPageNumber(-1,true,nPage,nLogPage,sDisplay);

    OUString sValue = "page-name:" + sDisplay +
        ";page-number:" +
        OUString::number( nPage ) +
        ";total-pages:" +
        OUString::number( pCursorShell->GetPageCnt() ) + ";";

    // cursor position relative to the page
    Point aCursorPagePos = pFEShell->GetCursorPagePos();
    sValue += "cursor-position-in-page-horizontal:" + OUString::number(aCursorPagePos.getX())
            + ";cursor-position-in-page-vertical:" + OUString::number(aCursorPagePos.getY()) + ";";

    SwContentFrame* pCurrFrame = pCursorShell->GetCurrFrame();
    SwPageFrame* pCurrPage = pCurrFrame->FindPageFrame();
    sal_Int32 nLineNum = 0;
    SwTextFrame* pCurrTextFrame = nullptr;
    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pCurrPage->ContainsContent());
    if (pCurrFrame->IsInFly())//such as, graphic,chart
    {
        SwFlyFrame *pFlyFrame = pCurrFrame->FindFlyFrame();
        const SwFormatAnchor& rAnchor = pFlyFrame->GetFormat()->GetAnchor();
        RndStdIds eAnchorId = rAnchor.GetAnchorId();
        if(eAnchorId == RndStdIds::FLY_AS_CHAR)
        {
            const SwFrame *pSwFrame = pFlyFrame->GetAnchorFrame();
            if(pSwFrame->IsTextFrame())
                pCurrTextFrame = const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pSwFrame));
        }
    }
    else
    {
        assert(dynamic_cast<SwTextFrame*>(pCurrFrame));
        pCurrTextFrame = static_cast<SwTextFrame* >(pCurrFrame);
    }
    //check whether the text frame where the Graph/OLE/Frame anchored is in the Header/Footer
    SwFrame* pFrame = pCurrTextFrame;
    while ( pFrame && !pFrame->IsHeaderFrame() && !pFrame->IsFooterFrame() )
        pFrame = pFrame->GetUpper();
    if ( pFrame )
        pCurrTextFrame = nullptr;
    //check shape
    if(pCursorShell->Imp()->GetDrawView())
    {
        const SdrMarkList &rMrkList = pCursorShell->Imp()->GetDrawView()->GetMarkedObjectList();
        for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark(i)->GetMarkedSdrObj();
            SwFrameFormat* pFormat = static_cast<SwDrawContact*>(pObj->GetUserCall())->GetFormat();
            const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
            if( RndStdIds::FLY_AS_CHAR != rAnchor.GetAnchorId() )
                pCurrTextFrame = nullptr;
        }
    }
    //calculate line number
    if (pCurrTextFrame && pTextFrame)
    {
        if (!(pCurrTextFrame->IsInTab() || pCurrTextFrame->IsInFootnote()))
        {
            while( pTextFrame && pTextFrame != pCurrTextFrame )
            {
                //check header/footer
                pFrame = pTextFrame;
                while ( pFrame && !pFrame->IsHeaderFrame() && !pFrame->IsFooterFrame() )
                    pFrame = pFrame->GetUpper();
                if ( pFrame )
                {
                    pTextFrame = static_cast< SwTextFrame*>(pTextFrame->GetNextContentFrame());
                    continue;
                }
                if (!(pTextFrame->IsInTab() || pTextFrame->IsInFootnote() || pTextFrame->IsInFly()))
                    nLineNum += pTextFrame->GetThisLines();
                pTextFrame = static_cast< SwTextFrame* >(pTextFrame ->GetNextContentFrame());
            }
            SwPaM* pCaret = pCursorShell->GetCursor();
            if (!pCurrTextFrame->IsEmpty() && pCaret)
            {
                assert(pCurrTextFrame->IsTextFrame());
                const SwPosition* pPoint = nullptr;
                if (pCurrTextFrame->IsInFly())
                {
                    SwFlyFrame *pFlyFrame = pCurrTextFrame->FindFlyFrame();
                    const SwFormatAnchor& rAnchor = pFlyFrame->GetFormat()->GetAnchor();
                    pPoint = rAnchor.GetContentAnchor();
                    SwContentNode *const pNode(pPoint->GetNode().GetContentNode());
                    pCurrTextFrame = pNode
                        ? static_cast<SwTextFrame*>(pNode->getLayoutFrame(
                                    pCurrTextFrame->getRootFrame(), pPoint))
                        : nullptr;
                }
                else
                    pPoint = pCaret->GetPoint();
                if (pCurrTextFrame)
                {
                    TextFrameIndex const nActPos(pCurrTextFrame->MapModelToViewPos(*pPoint));
                    nLineNum += pCurrTextFrame->GetLineCount( nActPos );
                }
            }
            else
                ++nLineNum;
        }
    }

    sValue += "line-number:" + OUString::number( nLineNum ) + ";";

    SwFrame* pCurrCol=static_cast<SwFrame*>(pCurrFrame)->FindColFrame();

    sValue += "column-number:";

    int nCurrCol = 1;
    if(pCurrCol!=nullptr)
    {
        //SwLayoutFrame* pParent = pCurrCol->GetUpper();
        SwFrame* pCurrPageCol=static_cast<SwFrame*>(pCurrFrame)->FindColFrame();
        while(pCurrPageCol && pCurrPageCol->GetUpper() && pCurrPageCol->GetUpper()->IsPageFrame())
        {
            pCurrPageCol = pCurrPageCol->GetUpper();
        }

        SwLayoutFrame* pParent = pCurrPageCol->GetUpper();

        if(pParent!=nullptr)
        {
            SwFrame* pCol = pParent->Lower();
            while(pCol&&(pCol!=pCurrPageCol))
            {
                pCol = pCol->GetNext();
                ++nCurrCol;
            }
        }
    }
    sValue += OUString::number( nCurrCol ) + ";";

    const SwFormatCol &rFormatCol=pCurrPage->GetAttrSet()->GetCol();
    sal_uInt16 nColCount=rFormatCol.GetNumCols();
    nColCount = nColCount>0?nColCount:1;
    sValue += "total-columns:" + OUString::number( nColCount ) + ";";

    SwSectionFrame* pCurrSctFrame=static_cast<SwFrame*>(pCurrFrame)->FindSctFrame();
    if(pCurrSctFrame!=nullptr && pCurrSctFrame->GetSection()!=nullptr )
    {
        OUString sectionName = pCurrSctFrame->GetSection()->GetSectionName().toString();

        sectionName = sectionName.replaceFirst( "\\" , "\\\\" );
        sectionName = sectionName.replaceFirst( "=" , "\\=" );
        sectionName = sectionName.replaceFirst( ";" , "\\;" );
        sectionName = sectionName.replaceFirst( "," , "\\," );
        sectionName = sectionName.replaceFirst( ":" , "\\:" );

        sValue += "section-name:" + sectionName + ";";

        //section-columns-number

        nCurrCol = 1;

        if(pCurrCol!=nullptr)
        {
            SwLayoutFrame* pParent = pCurrCol->GetUpper();
            if(pParent!=nullptr)
            {
                SwFrame* pCol = pParent->Lower();
                while(pCol&&(pCol!=pCurrCol))
                {
                    pCol = pCol->GetNext();
                    nCurrCol +=1;
                }
            }
        }
        sValue += "section-columns-number:" +
            OUString::number( nCurrCol ) + ";";

        //section-total-columns
        const SwFormatCol &rFormatSctCol=pCurrSctFrame->GetAttrSet()->GetCol();
        sal_uInt16 nSctColCount=rFormatSctCol.GetNumCols();
        nSctColCount = nSctColCount>0?nSctColCount:1;
        sValue += "section-total-columns:" +
            OUString::number( nSctColCount ) + ";";
    }

    return sValue;
}

sal_Int32 SAL_CALL SwAccessibleDocument::getBackground()
{
    SolarMutexGuard aGuard;
    return sal_Int32(SwModule::get()->GetColorConfig().GetColorValue(::svtools::DOCCOLOR).nColor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
