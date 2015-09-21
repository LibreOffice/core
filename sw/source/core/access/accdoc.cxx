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
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <sfx2/viewsh.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <viewsh.hxx>
#include <doc.hxx>
#include <accmap.hxx>
#include <accdoc.hxx>
#include "access.hrc"
#include <pagefrm.hxx>

#include <editeng/brushitem.hxx>
#include <swatrset.hxx>
#include <frmatr.hxx>
#include "unostyle.hxx"
#include "docsh.hxx"
#include <crsrsh.hxx>
#include "fesh.hxx"
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

SwAccessibleDocumentBase::SwAccessibleDocumentBase ( SwAccessibleMap *_pMap ) :
    SwAccessibleContext( _pMap, AccessibleRole::DOCUMENT_TEXT,
                         _pMap->GetShell()->GetLayout() ),
    mxParent( _pMap->GetShell()->GetWin()->GetAccessibleParentWindow()->GetAccessible() ),
    mpChildWin( 0 )
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
        // ChildrenScrolled( GetFrm(), aOldVisArea );
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

        mpChildWin = 0;
    }
}

sal_Int32 SAL_CALL SwAccessibleDocumentBase::getAccessibleChildCount()
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    // CHECK_FOR_DEFUNC is called by parent

    sal_Int32 nChildren = SwAccessibleContext::getAccessibleChildCount();
    if( !IsDisposing() && mpChildWin )
        nChildren++;

    return nChildren;
}

uno::Reference< XAccessible> SAL_CALL
    SwAccessibleDocumentBase::getAccessibleChild( sal_Int32 nIndex )
        throw (uno::RuntimeException,
                lang::IndexOutOfBoundsException, std::exception)
{
    SolarMutexGuard aGuard;

    if( mpChildWin  )
    {
        CHECK_FOR_DEFUNC( XAccessibleContext )
        if ( nIndex == GetChildCount( *(GetMap()) ) )
        {
            return mpChildWin->GetAccessible();
        }
    }

    return SwAccessibleContext::getAccessibleChild( nIndex );
}

uno::Reference< XAccessible> SAL_CALL SwAccessibleDocumentBase::getAccessibleParent()
        throw (uno::RuntimeException, std::exception)
{
    return mxParent;
}

sal_Int32 SAL_CALL SwAccessibleDocumentBase::getAccessibleIndexInParent()
        throw (uno::RuntimeException, std::exception)
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
        catch(const ::com::sun::star::lang::IndexOutOfBoundsException &)
        {
            return -1L;
        }
    }
    return -1L;
}

OUString SAL_CALL SwAccessibleDocumentBase::getAccessibleDescription()
    throw (uno::RuntimeException, std::exception)
{
    return GetResource( STR_ACCESS_DOC_DESC );
}

OUString SAL_CALL SwAccessibleDocumentBase::getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    OUString sAccName = GetResource( STR_ACCESS_DOC_WORDPROCESSING );
    SwDoc *pDoc = GetMap() ? GetShell()->GetDoc() : 0;
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
        OUString sReadOnly;
        if(pDoc->getDocReadOnly())
        {
            sReadOnly = GetResource( STR_ACCESS_DOC_WORDPROCESSING_READONLY );
        }

        if ( !sFileName.isEmpty() )
        {
            sAccName = sFileName + sReadOnly + " - " + sAccName;
        }
    }

    return sAccName;
}

awt::Rectangle SAL_CALL SwAccessibleDocumentBase::getBounds()
        throw (uno::RuntimeException, std::exception)
{
    try
    {
        SolarMutexGuard aGuard;

        vcl::Window *pWin = GetWindow();

        CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

        Rectangle aPixBounds( pWin->GetWindowExtentsRelative( pWin->GetAccessibleParentWindow() ) );
        awt::Rectangle aBox( aPixBounds.Left(), aPixBounds.Top(),
                             aPixBounds.GetWidth(), aPixBounds.GetHeight() );

        return aBox;
    }
    catch(const ::com::sun::star::lang::IndexOutOfBoundsException &)
    {
        return awt::Rectangle();
    }
}

awt::Point SAL_CALL SwAccessibleDocumentBase::getLocation()
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPos( pWin->GetWindowExtentsRelative( pWin->GetAccessibleParentWindow() ).TopLeft() );
    awt::Point aLoc( aPixPos.getX(), aPixPos.getY() );

    return aLoc;
}

::com::sun::star::awt::Point SAL_CALL SwAccessibleDocumentBase::getLocationOnScreen()
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPos( pWin->GetWindowExtentsRelative( 0 ).TopLeft() );
    awt::Point aLoc( aPixPos.getX(), aPixPos.getY() );

    return aLoc;
}

::com::sun::star::awt::Size SAL_CALL SwAccessibleDocumentBase::getSize()
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Size aPixSize( pWin->GetWindowExtentsRelative( 0 ).GetSize() );
    awt::Size aSize( aPixSize.Width(), aPixSize.Height() );

    return aSize;
}

sal_Bool SAL_CALL SwAccessibleDocumentBase::containsPoint(
            const awt::Point& aPoint )
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Rectangle aPixBounds( pWin->GetWindowExtentsRelative( 0 ) );
    aPixBounds.Move(-aPixBounds.Left(), -aPixBounds.Top());

    Point aPixPoint( aPoint.X, aPoint.Y );
    return aPixBounds.IsInside( aPixPoint );
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleDocumentBase::getAccessibleAtPoint(
                const awt::Point& aPoint )
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if( mpChildWin  )
    {
        CHECK_FOR_DEFUNC( XAccessibleComponent )

        vcl::Window *pWin = GetWindow();
        CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

        Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to window
        if( mpChildWin->GetWindowExtentsRelative( pWin ).IsInside( aPixPoint ) )
            return mpChildWin->GetAccessible();
    }

    return SwAccessibleContext::getAccessibleAtPoint( aPoint );
}

// SwAccessibeDocument

void SwAccessibleDocument::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // MULTISELECTABLE
    rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
    rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
}

SwAccessibleDocument::SwAccessibleDocument ( SwAccessibleMap* pInitMap ) :
    SwAccessibleDocumentBase( pInitMap ),
    maSelectionHelper( *this )
{
    SetName( GetResource( STR_ACCESS_DOC_NAME ) );
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
    vcl::Window *pWin = GetMap() ? GetMap()->GetShell()->GetWin() : 0;
    if( pWin )
        pWin->RemoveChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
}

void SwAccessibleDocument::Dispose( bool bRecursive )
{
    OSL_ENSURE( GetFrm() && GetMap(), "already disposed" );

    vcl::Window *pWin = GetMap() ? GetMap()->GetShell()->GetWin() : 0;
    if( pWin )
        pWin->RemoveChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
    SwAccessibleContext::Dispose( bRecursive );
}

IMPL_LINK_TYPED( SwAccessibleDocument, WindowChildEventListener, VclWindowEvent&, rEvent, void )
{
    OSL_ENSURE( rEvent.GetWindow(), "Window???" );
    switch ( rEvent.GetId() )
    {
    case VCLEVENT_WINDOW_SHOW:  // send create on show for direct accessible children
        {
            vcl::Window* pChildWin = static_cast< vcl::Window* >( rEvent.GetData() );
            if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
            {
                AddChild( pChildWin );
            }
        }
        break;
    case VCLEVENT_WINDOW_HIDE:  // send destroy on hide for direct accessible children
        {
            vcl::Window* pChildWin = static_cast< vcl::Window* >( rEvent.GetData() );
            if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
            {
                RemoveChild( pChildWin );
            }
        }
        break;
    case VCLEVENT_OBJECT_DYING:  // send destroy on hide for direct accessible children
        {
            vcl::Window* pChildWin = rEvent.GetWindow();
            if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
            {
                RemoveChild( pChildWin );
            }
        }
        break;
    }
}

OUString SAL_CALL SwAccessibleDocument::getImplementationName()
        throw( uno::RuntimeException, std::exception )
{
    return OUString(sImplementationName);
}

sal_Bool SAL_CALL SwAccessibleDocument::supportsService(const OUString& sTestServiceName)
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sTestServiceName);
}

uno::Sequence< OUString > SAL_CALL SwAccessibleDocument::getSupportedServiceNames()
        throw( uno::RuntimeException, std::exception )
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
    throw ( uno::RuntimeException, std::exception )
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
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleDocumentBase::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    //Reset types memory alloc
    aTypes.realloc( nIndex + 1 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex] = cppu::UnoType<XAccessibleSelection>::get();
    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleDocument::getImplementationId()
        throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XAccessibleSelection

void SwAccessibleDocument::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException, std::exception )
{
    maSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleDocument::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException, std::exception )
{
    return maSelectionHelper.isAccessibleChildSelected(nChildIndex);
}

void SwAccessibleDocument::clearAccessibleSelection(  )
    throw ( uno::RuntimeException, std::exception )
{
}

void SwAccessibleDocument::selectAllAccessibleChildren(  )
    throw ( uno::RuntimeException, std::exception )
{
    maSelectionHelper.selectAllAccessibleChildren();
}

sal_Int32 SwAccessibleDocument::getSelectedAccessibleChildCount(  )
    throw ( uno::RuntimeException, std::exception )
{
    return maSelectionHelper.getSelectedAccessibleChildCount();
}

uno::Reference<XAccessible> SwAccessibleDocument::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException, std::exception)
{
    return maSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

// index has to be treated as global child index.
void SwAccessibleDocument::deselectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException, std::exception )
{
    maSelectionHelper.deselectAccessibleChild( nChildIndex );
}

uno::Any SAL_CALL SwAccessibleDocument::getExtendedAttributes()
    throw (::com::sun::star::lang::IndexOutOfBoundsException,
           ::com::sun::star::uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard g;

    uno::Any anyAtrribute;
    SwDoc *pDoc = GetMap() ? GetShell()->GetDoc() : 0;

    if (!pDoc)
        return anyAtrribute;
    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( !pCrsrShell )
        return anyAtrribute;

    SwFEShell* pFEShell = pCrsrShell->ISA( SwFEShell )
                                ? static_cast<SwFEShell*>( pCrsrShell )
                            : 0;
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
        sValue += OUString::number( pCrsrShell->GetPageCnt() ) ;
        sValue += ";";

        sAttrName = "line-number:";

        SwContentFrm* pCurrFrm = pCrsrShell->GetCurrFrm();
        SwPageFrm* pCurrPage=static_cast<SwFrm*>(pCurrFrm)->FindPageFrm();
        sal_uLong nLineNum = 0;
        SwTextFrm* pTextFrm = NULL;
        SwTextFrm* pCurrTextFrm = NULL;
        pTextFrm = static_cast< SwTextFrm* >(static_cast< SwPageFrm* > (pCurrPage)->ContainsContent());
        if (pCurrFrm->IsInFly())//such as, graphic,chart
        {
            SwFlyFrm *pFlyFrm = pCurrFrm->FindFlyFrm();
            const SwFormatAnchor& rAnchor = pFlyFrm->GetFormat()->GetAnchor();
            RndStdIds eAnchorId = rAnchor.GetAnchorId();
            if(eAnchorId == FLY_AS_CHAR)
            {
                const SwFrm *pSwFrm = pFlyFrm->GetAnchorFrm();
                if(pSwFrm->IsTextFrm())
                    pCurrTextFrm = const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pSwFrm));
            }
        }
        else
            pCurrTextFrm = const_cast<SwTextFrm*>(static_cast<const SwTextFrm* >(pCurrFrm));
        //check whether the text frame where the Graph/OLE/Frame anchored is in the Header/Footer
        SwFrm* pFrm = pCurrTextFrm;
        while ( pFrm && !pFrm->IsHeaderFrm() && !pFrm->IsFooterFrm() )
            pFrm = pFrm->GetUpper();
        if ( pFrm )
            pCurrTextFrm = NULL;
        //check shape
        if(pCrsrShell->Imp()->GetDrawView())
        {
            const SdrMarkList &rMrkList = pCrsrShell->Imp()->GetDrawView()->GetMarkedObjectList();
            for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
            {
                SdrObject *pObj = rMrkList.GetMark(i)->GetMarkedSdrObj();
                SwFrameFormat* pFormat = static_cast<SwDrawContact*>(pObj->GetUserCall())->GetFormat();
                const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
                if( FLY_AS_CHAR != rAnchor.GetAnchorId() )
                    pCurrTextFrm = NULL;
            }
        }
        //calculate line number
        if (pCurrTextFrm && pTextFrm)
        {
            if (!(pCurrTextFrm->IsInTab() || pCurrTextFrm->IsInFootnote()))
            {
                while( pTextFrm != pCurrTextFrm )
                {
                    //check header/footer
                    pFrm = pTextFrm;
                    while ( pFrm && !pFrm->IsHeaderFrm() && !pFrm->IsFooterFrm() )
                        pFrm = pFrm->GetUpper();
                    if ( pFrm )
                    {
                        pTextFrm = static_cast< SwTextFrm*>(pTextFrm->GetNextContentFrm());
                        continue;
                    }
                    if (!(pTextFrm->IsInTab() || pTextFrm->IsInFootnote() || pTextFrm->IsInFly()))
                        nLineNum += pTextFrm->GetThisLines();
                    pTextFrm = static_cast< SwTextFrm* >(pTextFrm ->GetNextContentFrm());
                }
                SwPaM* pCaret = pCrsrShell->GetCrsr();
                if (!pCurrTextFrm->IsEmpty() && pCaret)
                {
                    if (pCurrTextFrm->IsTextFrm())
                    {
                        const SwPosition* pPoint = NULL;
                        if(pCurrTextFrm->IsInFly())
                        {
                            SwFlyFrm *pFlyFrm = pCurrTextFrm->FindFlyFrm();
                            const SwFormatAnchor& rAnchor = pFlyFrm->GetFormat()->GetAnchor();
                            pPoint= rAnchor.GetContentAnchor();
                        }
                        else
                            pPoint = pCaret->GetPoint();
                        const sal_Int32 nActPos = pPoint->nContent.GetIndex();
                        nLineNum += pCurrTextFrm->GetLineCount( nActPos );
                    }
                    else//graphic, form, shape, etc.
                    {
                        SwPosition* pPoint =  pCaret->GetPoint();
                        Point aPt = pCrsrShell->_GetCrsr()->GetPtPos();
                        if( pCrsrShell->GetLayout()->GetCrsrOfst( pPoint, aPt/*,* &eTmpState*/ ) )
                        {
                            const sal_Int32 nActPos = pPoint->nContent.GetIndex();
                            nLineNum += pCurrTextFrm->GetLineCount( nActPos );
                        }
                    }
                }
                else
                    ++nLineNum;
            }
        }

        sValue += sAttrName;
        sValue += OUString::number( nLineNum ) ;

        sValue += ";";

        SwFrm* pCurrCol=static_cast<SwFrm*>(pCurrFrm)->FindColFrm();

        sAttrName = "column-number:";
        sValue += sAttrName;

        int nCurrCol = 1;
        if(pCurrCol!=NULL)
        {
            //SwLayoutFrm* pParent = pCurrCol->GetUpper();
            SwFrm* pCurrPageCol=static_cast<SwFrm*>(pCurrFrm)->FindColFrm();
            while(pCurrPageCol && pCurrPageCol->GetUpper() && pCurrPageCol->GetUpper()->IsPageFrm())
            {
                pCurrPageCol = pCurrPageCol->GetUpper();
            }

            SwLayoutFrm* pParent = pCurrPageCol->GetUpper();

            if(pParent!=NULL)
            {
                SwFrm* pCol = pParent->Lower();
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

        SwSectionFrm* pCurrSctFrm=static_cast<SwFrm*>(pCurrFrm)->FindSctFrm();
        if(pCurrSctFrm!=NULL && pCurrSctFrm->GetSection()!=NULL )
        {
            sAttrName = "section-name:";

            sValue += sAttrName;
            OUString sectionName = pCurrSctFrm->GetSection()->GetSectionName();

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

            if(pCurrCol!=NULL)
            {
                SwLayoutFrm* pParent = pCurrCol->GetUpper();
                if(pParent!=NULL)
                {
                    SwFrm* pCol = pParent->Lower();
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
            const SwFormatCol &rFormatSctCol=pCurrSctFrm->GetAttrSet()->GetCol();
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
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return SW_MOD()->GetColorConfig().GetColorValue( ::svtools::DOCCOLOR ).nColor;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
        SAL_CALL SwAccessibleDocument::getAccFlowTo(const ::com::sun::star::uno::Any& rAny, sal_Int32 nType)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception)
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
        uno::Reference< ::com::sun::star::drawing::XShape > xShape;
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
                                        aRet[0] = uno::makeAny( xSel );
                                        return aRet;
                                    }
                                }
                            }
                        }
                    }
                    catch ( const com::sun::star::lang::IndexOutOfBoundsException& )
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
                aRet[0] = uno::makeAny( xAcc );
                return aRet;
            }
        }
    }
    else if ( nType == FORFINDREPLACEFLOWTO )
    {
        SwCrsrShell* pCrsrShell = GetCrsrShell();
        if ( pCrsrShell )
        {
            SwPaM *_pStartCrsr = pCrsrShell->GetCrsr(), *__pStartCrsr = _pStartCrsr;
            SwContentNode* pPrevNode = NULL;
            std::vector<SwFrm*> vFrmList;
            do
            {
                if ( _pStartCrsr && _pStartCrsr->HasMark() )
                {
                    SwContentNode* pContentNode = _pStartCrsr->GetContentNode();
                    if ( pContentNode == pPrevNode )
                    {
                        continue;
                    }
                    SwFrm* pFrm = pContentNode ? pContentNode->getLayoutFrm( pCrsrShell->GetLayout() ) : NULL;
                    if ( pFrm )
                    {
                        vFrmList.push_back( pFrm );
                    }

                    pPrevNode = pContentNode;
                }
            }

            while( _pStartCrsr && ( (_pStartCrsr = _pStartCrsr->GetNext()) != __pStartCrsr) );

            if ( vFrmList.size() )
            {
                uno::Sequence< uno::Any > aRet(vFrmList.size());
                std::vector<SwFrm*>::iterator aIter = vFrmList.begin();
                for ( sal_Int32 nIndex = 0; aIter != vFrmList.end(); ++aIter, nIndex++ )
                {
                    uno::Reference< XAccessible > xAcc = pAccMap->GetContext(*aIter, false);
                    if ( xAcc.is() )
                    {
                        SwAccessibleContext *pAccImpl = static_cast< SwAccessibleContext *>( xAcc.get() );
                        if ( pAccImpl && pAccImpl->getAccessibleRole() == AccessibleRole::PARAGRAPH )
                        {
                            aRet[nIndex] = uno::makeAny( xAcc );
                        }
                    }
                }

                return aRet;
            }
        }
    }

    return uno::Sequence< uno::Any >();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
