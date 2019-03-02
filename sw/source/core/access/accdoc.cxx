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
#include <unotools/accessiblestatesethelper.hxx>
#include <sfx2/viewsh.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <viewsh.hxx>
#include <doc.hxx>
#include <accmap.hxx>
#include "accdoc.hxx"
#include <strings.hrc>
#include <pagefrm.hxx>

#include <editeng/brushitem.hxx>
#include <swatrset.hxx>
#include <frmatr.hxx>
#include <unostyle.hxx>
#include <docsh.hxx>
#include <crsrsh.hxx>
#include <fesh.hxx>
#include <fmtclds.hxx>
#include <flyfrm.hxx>
#include <colfrm.hxx>
#include <txtfrm.hxx>
#include <sectfrm.hxx>
#include <section.hxx>
#include <svx/unoapi.hxx>
#include <swmodule.hxx>
#include <svtools/colorcfg.hxx>

#include <fmtanchr.hxx>
#include <viewimp.hxx>
#include <dview.hxx>
#include <dcontact.hxx>
#include <svx/svdmark.hxx>
const sal_Char sServiceName[] = "com.sun.star.text.AccessibleTextDocumentView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleDocumentView";

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using lang::IndexOutOfBoundsException;

// SwAccessibleDocumentBase: base class for SwAccessibleDocument and
// SwAccessiblePreview

SwAccessibleDocumentBase::SwAccessibleDocumentBase(
        std::shared_ptr<SwAccessibleMap> const& pMap)
    : SwAccessibleContext(pMap, AccessibleRole::DOCUMENT_TEXT,
                          pMap->GetShell()->GetLayout())
    , mxParent(pMap->GetShell()->GetWin()->GetAccessibleParentWindow()->GetAccessible())
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
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::CHILD;
            aEvent.NewValue <<= mpChildWin->GetAccessible();
            FireAccessibleEvent( aEvent );
        }
    }
}

void SwAccessibleDocumentBase::RemoveChild( vcl::Window *pWin )
{
    SolarMutexGuard aGuard;

    OSL_ENSURE( !mpChildWin || pWin == mpChildWin, "invalid child window to remove" );
    if( mpChildWin && pWin == mpChildWin )
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.OldValue <<= mpChildWin->GetAccessible();
        FireAccessibleEvent( aEvent );

        mpChildWin = nullptr;
    }
}

sal_Int32 SAL_CALL SwAccessibleDocumentBase::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;

    // ThrowIfDisposed is called by parent

    sal_Int32 nChildren = SwAccessibleContext::getAccessibleChildCount();
    if( !IsDisposing() && mpChildWin )
        nChildren++;

    return nChildren;
}

uno::Reference< XAccessible> SAL_CALL
    SwAccessibleDocumentBase::getAccessibleChild( sal_Int32 nIndex )
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

sal_Int32 SAL_CALL SwAccessibleDocumentBase::getAccessibleIndexInParent()
{
    SolarMutexGuard aGuard;

    uno::Reference < XAccessibleContext > xAcc( mxParent->getAccessibleContext() );
    uno::Reference < XAccessible > xThis( this );
    sal_Int32 nCount = xAcc->getAccessibleChildCount();

    for( sal_Int32 i=0; i < nCount; i++ )
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
    SwDoc *pDoc = GetMap() ? GetShell()->GetDoc() : nullptr;
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

awt::Rectangle SAL_CALL SwAccessibleDocumentBase::getBounds()
{
    try
    {
        SolarMutexGuard aGuard;

        vcl::Window *pWin = GetWindow();
        if (!pWin)
        {
            throw uno::RuntimeException("no Window", static_cast<cppu::OWeakObject*>(this));
        }

        tools::Rectangle aPixBounds( pWin->GetWindowExtentsRelative( pWin->GetAccessibleParentWindow() ) );
        awt::Rectangle aBox( aPixBounds.Left(), aPixBounds.Top(),
                             aPixBounds.GetWidth(), aPixBounds.GetHeight() );

        return aBox;
    }
    catch(const css::lang::IndexOutOfBoundsException &)
    {
        return awt::Rectangle();
    }
}

awt::Point SAL_CALL SwAccessibleDocumentBase::getLocation()
{
    SolarMutexGuard aGuard;

    vcl::Window *pWin = GetWindow();
    if (!pWin)
    {
        throw uno::RuntimeException("no Window", static_cast<cppu::OWeakObject*>(this));
    }

    Point aPixPos( pWin->GetWindowExtentsRelative( pWin->GetAccessibleParentWindow() ).TopLeft() );
    awt::Point aLoc( aPixPos.getX(), aPixPos.getY() );

    return aLoc;
}

css::awt::Point SAL_CALL SwAccessibleDocumentBase::getLocationOnScreen()
{
    SolarMutexGuard aGuard;

    vcl::Window *pWin = GetWindow();
    if (!pWin)
    {
        throw uno::RuntimeException("no Window", static_cast<cppu::OWeakObject*>(this));
    }

    Point aPixPos( pWin->GetWindowExtentsRelative( nullptr ).TopLeft() );
    awt::Point aLoc( aPixPos.getX(), aPixPos.getY() );

    return aLoc;
}

css::awt::Size SAL_CALL SwAccessibleDocumentBase::getSize()
{
    SolarMutexGuard aGuard;

    vcl::Window *pWin = GetWindow();
    if (!pWin)
    {
        throw uno::RuntimeException("no Window", static_cast<cppu::OWeakObject*>(this));
    }

    Size aPixSize( pWin->GetWindowExtentsRelative( nullptr ).GetSize() );
    awt::Size aSize( aPixSize.Width(), aPixSize.Height() );

    return aSize;
}

sal_Bool SAL_CALL SwAccessibleDocumentBase::containsPoint(
            const awt::Point& aPoint )
{
    SolarMutexGuard aGuard;

    vcl::Window *pWin = GetWindow();
    if (!pWin)
    {
        throw uno::RuntimeException("no Window", static_cast<cppu::OWeakObject*>(this));
    }

    tools::Rectangle aPixBounds( pWin->GetWindowExtentsRelative( nullptr ) );
    aPixBounds.Move(-aPixBounds.Left(), -aPixBounds.Top());

    Point aPixPoint( aPoint.X, aPoint.Y );
    return aPixBounds.IsInside( aPixPoint );
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
            throw uno::RuntimeException("no Window", static_cast<cppu::OWeakObject*>(this));
        }

        Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to window
        if( mpChildWin->GetWindowExtentsRelative( pWin ).IsInside( aPixPoint ) )
            return mpChildWin->GetAccessible();
    }

    return SwAccessibleContext::getAccessibleAtPoint( aPoint );
}

// SwAccessibleDocument

void SwAccessibleDocument::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // MULTISELECTABLE
    rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
    rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
}

SwAccessibleDocument::SwAccessibleDocument(
        std::shared_ptr<SwAccessibleMap> const& pInitMap)
    : SwAccessibleDocumentBase(pInitMap)
    , maSelectionHelper(*this)
{
    SetName(pInitMap->GetDocName());
    vcl::Window *pWin = pInitMap->GetShell()->GetWin();
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
    vcl::Window *pWin = GetMap() ? GetMap()->GetShell()->GetWin() : nullptr;
    if( pWin )
        pWin->RemoveChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
}

void SwAccessibleDocument::Dispose(bool bRecursive, bool bCanSkipInvisible)
{
    OSL_ENSURE( GetFrame() && GetMap(), "already disposed" );

    vcl::Window *pWin = GetMap() ? GetMap()->GetShell()->GetWin() : nullptr;
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

OUString SAL_CALL SwAccessibleDocument::getImplementationName()
{
    return OUString(sImplementationName);
}

sal_Bool SAL_CALL SwAccessibleDocument::supportsService(const OUString& sTestServiceName)
{
    return cppu::supportsService(this, sTestServiceName);
}

uno::Sequence< OUString > SAL_CALL SwAccessibleDocument::getSupportedServiceNames()
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = sServiceName;
    pArray[1] = sAccessibleServiceName;
    return aRet;
}

// XInterface

uno::Any SwAccessibleDocument::queryInterface(
    const uno::Type& rType )
{
    uno::Any aRet;
    if ( rType == cppu::UnoType<XAccessibleSelection>::get() )
    {
        uno::Reference<XAccessibleSelection> aSelect = this;
        aRet <<= aSelect;
    }
    else  if ( rType == cppu::UnoType<XAccessibleExtendedAttributes>::get())
    {
        uno::Reference<XAccessibleExtendedAttributes> aAttribute = this;
        aRet <<= aAttribute;
    }
    else if(rType == cppu::UnoType<XAccessibleGetAccFlowTo>::get())
    {
        uno::Reference<XAccessibleGetAccFlowTo> AccFlowTo = this;
        aRet <<= AccFlowTo;
    }
    else
        aRet = SwAccessibleContext::queryInterface( rType );
    return aRet;
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SwAccessibleDocument::getTypes()
{
    return cppu::OTypeCollection(
            cppu::UnoType<XAccessibleSelection>::get(),
            SwAccessibleDocumentBase::getTypes() ).getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleDocument::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XAccessibleSelection

void SwAccessibleDocument::selectAccessibleChild(
    sal_Int32 nChildIndex )
{
    maSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleDocument::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
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

sal_Int32 SwAccessibleDocument::getSelectedAccessibleChildCount(  )
{
    return maSelectionHelper.getSelectedAccessibleChildCount();
}

uno::Reference<XAccessible> SwAccessibleDocument::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
{
    return maSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

// index has to be treated as global child index.
void SwAccessibleDocument::deselectAccessibleChild(
    sal_Int32 nChildIndex )
{
    maSelectionHelper.deselectAccessibleChild( nChildIndex );
}

uno::Any SAL_CALL SwAccessibleDocument::getExtendedAttributes()
{
    SolarMutexGuard g;

    uno::Any anyAtrribute;
    SwDoc *pDoc = GetMap() ? GetShell()->GetDoc() : nullptr;

    if (!pDoc)
        return anyAtrribute;
    SwCursorShell* pCursorShell = GetCursorShell();
    if( !pCursorShell )
        return anyAtrribute;

    SwFEShell* pFEShell = dynamic_cast<const SwFEShell*>( pCursorShell) !=  nullptr
                                ? static_cast<SwFEShell*>( pCursorShell )
                            : nullptr;
    OUString sAttrName;
    OUString sValue;
    sal_uInt16 nPage, nLogPage;
    OUString sDisplay;

    if( pFEShell )
    {
        pFEShell->GetPageNumber(-1,true,nPage,nLogPage,sDisplay);
        sAttrName = "page-name:";

        sValue = sAttrName + sDisplay ;
        sAttrName = ";page-number:";
        sValue += sAttrName;
        sValue += OUString::number( nPage ) ;
        sAttrName = ";total-pages:";
        sValue += sAttrName;
        sValue += OUString::number( pCursorShell->GetPageCnt() ) ;
        sValue += ";";

        sAttrName = "line-number:";

        SwContentFrame* pCurrFrame = pCursorShell->GetCurrFrame();
        SwPageFrame* pCurrPage=static_cast<SwFrame*>(pCurrFrame)->FindPageFrame();
        sal_uLong nLineNum = 0;
        SwTextFrame* pTextFrame = nullptr;
        SwTextFrame* pCurrTextFrame = nullptr;
        pTextFrame = static_cast< SwTextFrame* >(pCurrPage->ContainsContent());
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
                while( pTextFrame != pCurrTextFrame )
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
                        SwContentNode *const pNode(pPoint->nNode.GetNode().GetContentNode());
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

        sValue += sAttrName;
        sValue += OUString::number( nLineNum ) ;

        sValue += ";";

        SwFrame* pCurrCol=static_cast<SwFrame*>(pCurrFrame)->FindColFrame();

        sAttrName = "column-number:";
        sValue += sAttrName;

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
        sValue += OUString::number( nCurrCol ) ;
        sValue += ";";

        sAttrName = "total-columns:";

        const SwFormatCol &rFormatCol=pCurrPage->GetAttrSet()->GetCol();
        sal_uInt16 nColCount=rFormatCol.GetNumCols();
        nColCount = nColCount>0?nColCount:1;
        sValue += sAttrName;
        sValue += OUString::number( nColCount ) ;

        sValue += ";";

        SwSectionFrame* pCurrSctFrame=static_cast<SwFrame*>(pCurrFrame)->FindSctFrame();
        if(pCurrSctFrame!=nullptr && pCurrSctFrame->GetSection()!=nullptr )
        {
            sAttrName = "section-name:";

            sValue += sAttrName;
            OUString sectionName = pCurrSctFrame->GetSection()->GetSectionName();

            sectionName = sectionName.replaceFirst( "\\" , "\\\\" );
            sectionName = sectionName.replaceFirst( "=" , "\\=" );
            sectionName = sectionName.replaceFirst( ";" , "\\;" );
            sectionName = sectionName.replaceFirst( "," , "\\," );
            sectionName = sectionName.replaceFirst( ":" , "\\:" );

            sValue += sectionName;

            sValue += ";";

            //section-columns-number
            sAttrName = "section-columns-number:";

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
            sValue += sAttrName;
            sValue += OUString::number( nCurrCol ) ;
            sValue += ";";

            //section-total-columns
            sAttrName = "section-total-columns:";
            const SwFormatCol &rFormatSctCol=pCurrSctFrame->GetAttrSet()->GetCol();
            sal_uInt16 nSctColCount=rFormatSctCol.GetNumCols();
            nSctColCount = nSctColCount>0?nSctColCount:1;
            sValue += sAttrName;
            sValue += OUString::number( nSctColCount ) ;

            sValue += ";";
        }

        anyAtrribute <<= sValue;
    }
    return anyAtrribute;
}

sal_Int32 SAL_CALL SwAccessibleDocument::getBackground()
{
    SolarMutexGuard aGuard;
    return sal_Int32(SW_MOD()->GetColorConfig().GetColorValue( ::svtools::DOCCOLOR ).nColor);
}

css::uno::Sequence< css::uno::Any >
        SAL_CALL SwAccessibleDocument::getAccFlowTo(const css::uno::Any& rAny, sal_Int32 nType)
{
    SolarMutexGuard g;

    const sal_Int32 FORSPELLCHECKFLOWTO = 1;
    const sal_Int32 FORFINDREPLACEFLOWTO = 2;
    SwAccessibleMap* pAccMap = GetMap();
    if ( !pAccMap )
    {
        return uno::Sequence< uno::Any >();
    }

    if ( nType == FORSPELLCHECKFLOWTO )
    {
        uno::Reference< css::drawing::XShape > xShape;
        rAny >>= xShape;
        if( xShape.is() )
        {
            SdrObject* pObj = GetSdrObjectFromXShape(xShape);
            if( pObj )
            {
                uno::Reference<XAccessible> xAcc = pAccMap->GetContext(pObj, this, false);
                uno::Reference < XAccessibleSelection > xAccSelection( xAcc, uno::UNO_QUERY );
                if ( xAccSelection.is() )
                {
                    try
                    {
                        if ( xAccSelection->getSelectedAccessibleChildCount() )
                        {
                            uno::Reference < XAccessible > xSel = xAccSelection->getSelectedAccessibleChild( 0 );
                            if ( xSel.is() )
                            {
                                uno::Reference < XAccessibleContext > xSelContext( xSel->getAccessibleContext() );
                                if ( xSelContext.is() )
                                {
                                    //if in sw we find the selected paragraph here
                                    if ( xSelContext->getAccessibleRole() == AccessibleRole::PARAGRAPH )
                                    {
                                        uno::Sequence<uno::Any> aRet( 1 );
                                        aRet[0] <<= xSel;
                                        return aRet;
                                    }
                                }
                            }
                        }
                    }
                    catch ( const css::lang::IndexOutOfBoundsException& )
                    {
                        return uno::Sequence< uno::Any >();
                    }
                    //end of try...catch
                }
            }
        }
        else
        {
            uno::Reference< XAccessible > xAcc = pAccMap->GetCursorContext();
            SwAccessibleContext *pAccImpl = static_cast< SwAccessibleContext *>( xAcc.get() );
            if ( pAccImpl && pAccImpl->getAccessibleRole() == AccessibleRole::PARAGRAPH )
            {
                uno::Sequence< uno::Any > aRet(1);
                aRet[0] <<= xAcc;
                return aRet;
            }
        }
    }
    else if ( nType == FORFINDREPLACEFLOWTO )
    {
        SwCursorShell* pCursorShell = GetCursorShell();
        if ( pCursorShell )
        {
            SwPaM *_pStartCursor = pCursorShell->GetCursor(), *_pStartCursor2 = _pStartCursor;
            std::set<SwFrame*> vFrameList;
            do
            {
                if ( _pStartCursor && _pStartCursor->HasMark() )
                {
                    SwContentNode* pContentNode = _pStartCursor->GetContentNode();
                    SwFrame *const pFrame = pContentNode
                        ? pContentNode->getLayoutFrame(pCursorShell->GetLayout(), _pStartCursor->GetPoint())
                        : nullptr;
                    if ( pFrame )
                    {
                        vFrameList.insert( pFrame );
                    }
                }
            }

            while( _pStartCursor && ( (_pStartCursor = _pStartCursor->GetNext()) != _pStartCursor2) );

            if ( !vFrameList.empty() )
            {
                uno::Sequence< uno::Any > aRet(vFrameList.size());
                sal_Int32 nIndex = 0;
                for ( const auto& rpFrame : vFrameList )
                {
                    uno::Reference< XAccessible > xAcc = pAccMap->GetContext(rpFrame, false);
                    if ( xAcc.is() )
                    {
                        SwAccessibleContext *pAccImpl = static_cast< SwAccessibleContext *>( xAcc.get() );
                        if ( pAccImpl && pAccImpl->getAccessibleRole() == AccessibleRole::PARAGRAPH )
                        {
                            aRet[nIndex] <<= xAcc;
                        }
                    }
                    nIndex++;
                }

                return aRet;
            }
        }
    }

    return uno::Sequence< uno::Any >();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
