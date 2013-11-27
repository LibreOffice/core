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
#include <vcl/window.hxx>
#include <rootfrm.hxx>


#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <tools/link.hxx>
#include <sfx2/viewsh.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <viewsh.hxx>
#include <doc.hxx>
#include <accmap.hxx>
#include <accdoc.hxx>
#ifndef _ACCESS_HRC
#include "access.hrc"
#endif
#include <pagefrm.hxx>

//IAccessibility2 Implementation 2009-----
#include <editeng/brshitem.hxx>
#include <swatrset.hxx>
#include <frmatr.hxx>
#include "unostyle.hxx"
#include "viewsh.hxx"
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
//-----IAccessibility2 Implementation 2009
const sal_Char sServiceName[] = "com.sun.star.text.AccessibleTextDocumentView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleDocumentView";


using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

using lang::IndexOutOfBoundsException;



//
// SwAccessibleDocumentBase: base class for SwAccessibleDocument and
// SwAccessiblePreview
//

SwAccessibleDocumentBase::SwAccessibleDocumentBase ( SwAccessibleMap *_pMap ) :
    SwAccessibleContext( _pMap, AccessibleRole::DOCUMENT,
                         _pMap->GetShell()->GetLayout() ),//swmod 071107//swmod 071225
    mxParent( _pMap->GetShell()->GetWin()->GetAccessibleParentWindow()->GetAccessible() ),
    mpChildWin( 0 )
{
}

SwAccessibleDocumentBase::~SwAccessibleDocumentBase()
{
}

void SwAccessibleDocumentBase::SetVisArea()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwRect aOldVisArea( GetVisArea() );
    const SwRect& rNewVisArea = GetMap()->GetVisArea();
    if( aOldVisArea != rNewVisArea )
    {
        SwAccessibleFrame::SetVisArea( GetMap()->GetVisArea() );
        // --> OD 2007-12-07 #i58139#
        // showing state of document view needs also be updated.
        // Thus, call method <Scrolled(..)> instead of <ChildrenScrolled(..)>
//        ChildrenScrolled( GetFrm(), aOldVisArea );
        Scrolled( aOldVisArea );
        // <--
    }
}

void SwAccessibleDocumentBase::AddChild( Window *pWin, sal_Bool bFireEvent )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    ASSERT( !mpChildWin, "only one child window is supported" );
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

void SwAccessibleDocumentBase::RemoveChild( Window *pWin )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    ASSERT( !mpChildWin || pWin == mpChildWin, "invalid child window to remove" );
    if( mpChildWin && pWin == mpChildWin )
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.OldValue <<= mpChildWin->GetAccessible();
        FireAccessibleEvent( aEvent );

        mpChildWin = 0;
    }
}

sal_Int32 SAL_CALL SwAccessibleDocumentBase::getAccessibleChildCount( void )
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // CHECK_FOR_DEFUNC is called by parent

    sal_Int32 nChildren = SwAccessibleContext::getAccessibleChildCount();
    if( !IsDisposing() && mpChildWin )
        nChildren++;

    return nChildren;
}

uno::Reference< XAccessible> SAL_CALL
    SwAccessibleDocumentBase::getAccessibleChild( sal_Int32 nIndex )
        throw (uno::RuntimeException,
                lang::IndexOutOfBoundsException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

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


uno::Reference< XAccessible> SAL_CALL SwAccessibleDocumentBase::getAccessibleParent (void)
        throw (uno::RuntimeException)
{
    return mxParent;
}

sal_Int32 SAL_CALL SwAccessibleDocumentBase::getAccessibleIndexInParent (void)
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Reference < XAccessibleContext > xAcc( mxParent->getAccessibleContext() );
    uno::Reference < XAccessible > xThis( this );
    sal_Int32 nCount = xAcc->getAccessibleChildCount();

    for( sal_Int32 i=0; i < nCount; i++ )
    {
        //IAccessibility2 Implementation 2009-----
        try
        {
            if( xAcc->getAccessibleChild( i ) == xThis )
                return i;
        }
        catch(::com::sun::star::lang::IndexOutOfBoundsException e)
        {
            return -1L;
        }
        //-----IAccessibility2 Implementation 2009
    }
    return -1L;
}

OUString SAL_CALL SwAccessibleDocumentBase::getAccessibleDescription (void)
    throw (uno::RuntimeException)
{
    return GetResource( STR_ACCESS_DOC_DESC );
}

//IAccessibility2 Implementation 2009-----
OUString SAL_CALL SwAccessibleDocumentBase::getAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    OUString sAccName = GetResource( STR_ACCESS_DOC_WORDPROCESSING );
    SwDoc *pDoc = GetShell()->GetDoc();
    if ( pDoc )
    {
        OUString sFileName = pDoc->getDocAccTitle();
        if ( !sFileName.getLength() )
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

        if ( sFileName.getLength() )
        {
            sAccName = sFileName + sReadOnly + OUString(RTL_CONSTASCII_USTRINGPARAM(" - ")) + sAccName;
        }
    }

    return sAccName;
}
//-----IAccessibility2 Implementation 2009

awt::Rectangle SAL_CALL SwAccessibleDocumentBase::getBounds()
        throw (uno::RuntimeException)
{
    //IAccessibility2 Implementation 2009-----
    try
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        Window *pWin = GetWindow();

        CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

            Rectangle aPixBounds( pWin->GetWindowExtentsRelative( pWin->GetAccessibleParentWindow() ) );
        awt::Rectangle aBox( aPixBounds.Left(), aPixBounds.Top(),
            aPixBounds.GetWidth(), aPixBounds.GetHeight() );

        return aBox;
    }
    catch(::com::sun::star::lang::IndexOutOfBoundsException e)
    {
        return awt::Rectangle();
    }
    //-----IAccessibility2 Implementation 2009
}


awt::Point SAL_CALL SwAccessibleDocumentBase::getLocation()
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPos( pWin->GetWindowExtentsRelative( pWin->GetAccessibleParentWindow() ).TopLeft() );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Point SAL_CALL SwAccessibleDocumentBase::getLocationOnScreen()
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPos( pWin->GetWindowExtentsRelative( 0 ).TopLeft() );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Size SAL_CALL SwAccessibleDocumentBase::getSize()
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Size aPixSize( pWin->GetWindowExtentsRelative( 0 ).GetSize() );
    awt::Size aSize( aPixSize.Width(), aPixSize.Height() );

    return aSize;
}

sal_Bool SAL_CALL SwAccessibleDocumentBase::containsPoint(
            const awt::Point& aPoint )
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Rectangle aPixBounds( pWin->GetWindowExtentsRelative( 0 ) );
    aPixBounds.Move(-aPixBounds.Left(), -aPixBounds.Top());

    Point aPixPoint( aPoint.X, aPoint.Y );
    return aPixBounds.IsInside( aPixPoint );
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleDocumentBase::getAccessibleAtPoint(
                const awt::Point& aPoint )
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( mpChildWin  )
    {
        CHECK_FOR_DEFUNC( XAccessibleComponent )

        Window *pWin = GetWindow();
        CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

        Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to window
        if( mpChildWin->GetWindowExtentsRelative( pWin ).IsInside( aPixPoint ) )
            return mpChildWin->GetAccessible();
    }

    return SwAccessibleContext::getAccessibleAtPoint( aPoint );
}

//
// SwAccessibeDocument
//

void SwAccessibleDocument::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // MULTISELECTABLE
    rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
    //IAccessibility2 Implementation 2009-----
    rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
    //-----IAccessibility2 Implementation 2009
}


SwAccessibleDocument::SwAccessibleDocument ( SwAccessibleMap* pInitMap ) :
    SwAccessibleDocumentBase( pInitMap ),
    maSelectionHelper( *this )
{
    SetName( GetResource( STR_ACCESS_DOC_NAME ) );
    Window *pWin = pInitMap->GetShell()->GetWin();
    if( pWin )
    {
        pWin->AddChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
        sal_uInt16 nCount =   pWin->GetChildCount();
        for( sal_uInt16 i=0; i < nCount; i++ )
        {
            Window* pChildWin = pWin->GetChild( i );
            if( pChildWin &&
                AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                AddChild( pChildWin, sal_False );
        }
    }
}

SwAccessibleDocument::~SwAccessibleDocument()
{
    Window *pWin = GetMap() ? GetMap()->GetShell()->GetWin() : 0;
    if( pWin )
        pWin->RemoveChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
}

void SwAccessibleDocument::Dispose( sal_Bool bRecursive )
{
    ASSERT( GetFrm() && GetMap(), "already disposed" );

    Window *pWin = GetMap() ? GetMap()->GetShell()->GetWin() : 0;
    if( pWin )
        pWin->RemoveChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
    SwAccessibleContext::Dispose( bRecursive );
}

IMPL_LINK( SwAccessibleDocument, WindowChildEventListener, VclSimpleEvent*, pEvent )
{
    VclWindowEvent *pVclEvent = dynamic_cast< VclWindowEvent * >( pEvent );
    DBG_ASSERT( pVclEvent, "Unknown WindowEvent!" );

    if ( pVclEvent )
    {
        DBG_ASSERT( pVclEvent->GetWindow(), "Window???" );
        switch ( pVclEvent->GetId() )
        {
        case VCLEVENT_WINDOW_SHOW:  // send create on show for direct accessible children
            {
                Window* pChildWin = static_cast< Window* >( pVclEvent->GetData() );
                if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                {
                    AddChild( pChildWin );
                }
            }
            break;
        case VCLEVENT_WINDOW_HIDE:  // send destroy on hide for direct accessible children
            {
                Window* pChildWin = static_cast< Window* >( pVclEvent->GetData() );
                if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                {
                    RemoveChild( pChildWin );
                }
            }
            break;
        case VCLEVENT_OBJECT_DYING:  // send destroy on hide for direct accessible children
            {
                Window* pChildWin = pVclEvent->GetWindow();
                if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                {
                    RemoveChild( pChildWin );
                }
            }
            break;
        }
    }
    return 0;
}


OUString SAL_CALL SwAccessibleDocument::getImplementationName()
        throw( uno::RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SAL_CALL SwAccessibleDocument::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

uno::Sequence< OUString > SAL_CALL SwAccessibleDocument::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

//=====  XInterface  ======================================================

uno::Any SwAccessibleDocument::queryInterface(
    const uno::Type& rType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet;
    if ( rType == ::getCppuType( static_cast< uno::Reference< XAccessibleSelection > * >( 0 ) ) )
    {
        uno::Reference<XAccessibleSelection> aSelect = this;
        aRet <<= aSelect;
    }
    //IAccessibility2 Implementation 2009-----
    //Solution:Add XEventListener interface support.
    else if ( (rType == ::getCppuType((uno::Reference<com::sun::star::document::XEventListener> *)NULL)) )
    {
        uno::Reference<com::sun::star::document::XEventListener> aSelect = this;
        aRet <<= aSelect;
    }
    else  if ( rType == ::getCppuType((uno::Reference<XAccessibleExtendedAttributes> *)NULL) )
    {
        uno::Reference<XAccessibleExtendedAttributes> aAttribute = this;
        aRet <<= aAttribute;
    }
    else if(rType == ::getCppuType((uno::Reference<XAccessibleGetAccFlowTo> *)NULL) )
    {
        uno::Reference<XAccessibleGetAccFlowTo> AccFlowTo = this;
        aRet <<= AccFlowTo;
    }
    //-----IAccessibility2 Implementation 2009
    else
        aRet = SwAccessibleContext::queryInterface( rType );
    return aRet;
}

//====== XTypeProvider ====================================================
uno::Sequence< uno::Type > SAL_CALL SwAccessibleDocument::getTypes()
    throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleDocumentBase::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    //IAccessibility2 Implementation 2009-----
    //Solution:Reset types memory alloc
    //aTypes.realloc( nIndex + 1 );
    aTypes.realloc( nIndex + 2 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex] = ::getCppuType( static_cast< uno::Reference< XAccessibleSelection > * >( 0 ) );
    //Solution:Add XEventListener interface support.
    pTypes[nIndex + 1 ] = ::getCppuType( static_cast< uno::Reference< com::sun::star::document::XEventListener > * >( 0 ) );
    //-----IAccessibility2 Implementation 2009
    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleDocument::getImplementationId()
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

//=====  XAccessibleSelection  ============================================

void SwAccessibleDocument::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    maSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleDocument::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    return maSelectionHelper.isAccessibleChildSelected(nChildIndex);
}

void SwAccessibleDocument::clearAccessibleSelection(  )
    throw ( uno::RuntimeException )
{
    maSelectionHelper.clearAccessibleSelection();
}

void SwAccessibleDocument::selectAllAccessibleChildren(  )
    throw ( uno::RuntimeException )
{
    maSelectionHelper.selectAllAccessibleChildren();
}

sal_Int32 SwAccessibleDocument::getSelectedAccessibleChildCount(  )
    throw ( uno::RuntimeException )
{
    return maSelectionHelper.getSelectedAccessibleChildCount();
}

uno::Reference<XAccessible> SwAccessibleDocument::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException)
{
    return maSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

// --> OD 2004-11-16 #111714# - index has to be treated as global child index.
void SwAccessibleDocument::deselectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    maSelectionHelper.deselectAccessibleChild( nChildIndex );
}
//IAccessibility2 Implementation 2009-----
//Solution:Implement XEventListener interfaces
void SAL_CALL SwAccessibleDocument::notifyEvent( const ::com::sun::star::document::EventObject& Event )
            throw (::com::sun::star::uno::RuntimeException)
{
    if ( Event.EventName.equalsAscii( "FirstPageShows" ) )
    {
        FireStateChangedEvent( AccessibleStateType::FOCUSED,sal_True );
    }
    else if ( Event.EventName.equalsAscii( "LoadFinished" ) )
    {
        // IA2 CWS. MT: OFFSCREEN == !SHOWING, should stay consistent
        // FireStateChangedEvent( AccessibleStateType::OFFSCREEN,sal_True );
        // MT: LoadFinished => Why not SHOWING == TRUE?
        FireStateChangedEvent( AccessibleStateType::SHOWING,sal_False );
    }
    else if ( Event.EventName.equalsAscii( "FormatFinished" ) )
    {
        FireStateChangedEvent( AccessibleStateType::BUSY,sal_False );
        // FireStateChangedEvent( AccessibleStateType::OFFSCREEN,sal_False );
        FireStateChangedEvent( AccessibleStateType::SHOWING,sal_True );
    }
    else
    {
        isIfAsynLoad = sal_False;
    }
}

void SAL_CALL SwAccessibleDocument::disposing( const ::com::sun::star::lang::EventObject& )
            throw (::com::sun::star::uno::RuntimeException)
{
}

uno::Any SAL_CALL SwAccessibleDocument::getExtendedAttributes()
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    uno::Any anyAtrribute;
    SwDoc *pDoc = GetShell()->GetDoc();

    if (!pDoc)
        return anyAtrribute;
    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( !pCrsrShell )
        return anyAtrribute;

    SwFEShell* pFEShell = dynamic_cast< SwFEShell* >(pCrsrShell);
    rtl::OUString sAttrName;
    rtl::OUString sValue;
    sal_uInt16 nPage, nLogPage;
    String sDisplay;

    if( pFEShell )
    {
        pFEShell->GetPageNumber(-1,sal_True,nPage,nLogPage,sDisplay);
        sAttrName = rtl::OUString::createFromAscii("page-name:");


        sValue = sAttrName + sDisplay ;
        sAttrName = rtl::OUString::createFromAscii(";page-number:");
        sValue += sAttrName;
        sValue += String::CreateFromInt32( nPage ) ;
        sAttrName = rtl::OUString::createFromAscii(";total-pages:");
        sValue += sAttrName;
        sValue += String::CreateFromInt32( pCrsrShell->GetPageCnt() ) ;
        sValue +=  rtl::OUString::createFromAscii(";");


        sAttrName=rtl::OUString::createFromAscii("line-number:");



        SwCntntFrm* pCurrFrm = pCrsrShell->GetCurrFrm();
        SwPageFrm* pCurrPage=((SwFrm*)pCurrFrm)->FindPageFrm();
        sal_uLong nLineNum = 0;
        //IAccessibility2 Implementation 2009-----
        SwTxtFrm* pTxtFrm = NULL;
        SwTxtFrm* pCurrTxtFrm = NULL;
        pTxtFrm = static_cast< SwTxtFrm* >(static_cast< SwPageFrm* > (pCurrPage)->ContainsCntnt());
        if (pCurrFrm->IsInFly())//such as, graphic,chart
        {
            SwFlyFrm *pFlyFrm = pCurrFrm->FindFlyFrm();
            const SwFmtAnchor& rAnchor = pFlyFrm->GetFmt()->GetAnchor();
            RndStdIds eAnchorId = rAnchor.GetAnchorId();
            if(eAnchorId == FLY_AS_CHAR)
            {
                const SwFrm *pSwFrm = pFlyFrm->GetAnchorFrm();
                if(pSwFrm->IsTxtFrm())
                    pCurrTxtFrm = ((SwTxtFrm*)(pSwFrm));
            }
        }
        else
            pCurrTxtFrm = static_cast< SwTxtFrm* >(pCurrFrm);
        //check whether the text frame where the Graph/OLE/Frame anchored is in the Header/Footer
        SwFrm* pFrm = pCurrTxtFrm;
        while ( pFrm && !pFrm->IsHeaderFrm() && !pFrm->IsFooterFrm() )
            pFrm = pFrm->GetUpper();
        if ( pFrm )
            pCurrTxtFrm = NULL;
        //check shape
        if(pCrsrShell->Imp()->GetDrawView())
        {
            const SdrObjectVector aSdrObjectVector(pCrsrShell->Imp()->GetDrawView()->getSelectedSdrObjectVectorFromSdrMarkView());

            for(sal_uInt32 i(0); i < aSdrObjectVector.size(); ++i )
            {
                SdrObject* pObj = aSdrObjectVector[i];

                // replace formally used 'GetUserCall()' by new notify/listener mechanism
                const SwDrawContact* pContact = static_cast< const SwDrawContact* >(findConnectionToSdrObject(pObj));

                const SwFrmFmt* pFmt = pContact->GetFmt();
                const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                if( FLY_AS_CHAR != rAnchor.GetAnchorId() )
                    pCurrTxtFrm = NULL;
            }
        }
        //calculate line number
        if (pCurrTxtFrm && pTxtFrm)
        {
            if (!(pCurrTxtFrm->IsInTab() || pCurrTxtFrm->IsInFtn()))
            {
                while( pTxtFrm != pCurrTxtFrm )
                {
                    //check header/footer
                    pFrm = pTxtFrm;
                    while ( pFrm && !pFrm->IsHeaderFrm() && !pFrm->IsFooterFrm() )
                        pFrm = pFrm->GetUpper();
                    if ( pFrm )
                    {
                        pTxtFrm = static_cast< SwTxtFrm*>(pTxtFrm->GetNextCntntFrm());
                        continue;
                    }
                    if (!(pTxtFrm->IsInTab() || pTxtFrm->IsInFtn() || pTxtFrm->IsInFly()))
                        nLineNum += pTxtFrm->GetThisLines();
                    pTxtFrm = static_cast< SwTxtFrm* >(pTxtFrm ->GetNextCntntFrm());
                }
                SwPaM* pCaret = pCrsrShell->GetCrsr();
                if (!pCurrTxtFrm->IsEmpty() && pCaret)
                {
                    sal_uInt16 nActPos = 0;
                    if (pCurrTxtFrm->IsTxtFrm())
                    {
                        const SwPosition* pPoint = NULL;
                        if(pCurrTxtFrm->IsInFly())
                        {
                            SwFlyFrm *pFlyFrm = pCurrTxtFrm->FindFlyFrm();
                            const SwFmtAnchor& rAnchor = pFlyFrm->GetFmt()->GetAnchor();
                            pPoint= rAnchor.GetCntntAnchor();
                        }
                        else
                            pPoint = pCaret->GetPoint();
                        nActPos = pPoint->nContent.GetIndex();
                        nLineNum += pCurrTxtFrm->GetLineCount( nActPos );
                    }
                    else//graphic, form, shape, etc.
                    {
                        SwPosition* pPoint =  pCaret->GetPoint();
                        Point aPt = pCrsrShell->_GetCrsr()->GetPtPos();
                        if( pCrsrShell->GetLayout()->GetCrsrOfst( pPoint, aPt/*,* &eTmpState*/ ) )
                        {
                            nActPos = pPoint->nContent.GetIndex();
                            nLineNum += pCurrTxtFrm->GetLineCount( nActPos );
                        }
                    }
                }
                else
                    ++nLineNum;
            }
        }
        //-----IAccessibility2 Implementation 2009

        sValue += sAttrName;
        sValue += String::CreateFromInt32( nLineNum ) ;

        sValue +=  rtl::OUString::createFromAscii(";");


        SwFrm* pCurrCol=((SwFrm*)pCurrFrm)->FindColFrm();

        sAttrName=rtl::OUString::createFromAscii("column-number:");
        sValue += sAttrName;

        sal_uInt16 nCurrCol = 1;
        if(pCurrCol!=NULL)
        {
            //SwLayoutFrm* pParent = pCurrCol->GetUpper();
            SwFrm* pCurrPageCol=((SwFrm*)pCurrFrm)->FindColFrm();
            while(pCurrPageCol && pCurrPageCol->GetUpper() && pCurrPageCol->GetUpper()->IsPageFrm())
            {
                pCurrPageCol = pCurrPageCol->GetUpper();
            }

            SwLayoutFrm* pParent = (SwLayoutFrm*)(pCurrPageCol->GetUpper());

            if(pParent!=NULL)
            {
                SwFrm* pCol = pParent->Lower();
                while(pCol&&(pCol!=pCurrPageCol))
                {
                    pCol = pCol->GetNext();
                    nCurrCol +=1;
                }
            }
        }
        sValue += String::CreateFromInt32( nCurrCol ) ;
        sValue +=  rtl::OUString::createFromAscii(";");

        sAttrName=rtl::OUString::createFromAscii("total-columns:");

        const SwFmtCol &rFmtCol=pCurrPage->GetAttrSet()->GetCol();
        sal_uInt16 nColCount=rFmtCol.GetNumCols();
        nColCount = nColCount>0?nColCount:1;
        sValue += sAttrName;
        sValue += String::CreateFromInt32( nColCount ) ;

        sValue +=  rtl::OUString::createFromAscii(";");

        if(pCurrFrm!=NULL)
        {
            SwSectionFrm* pCurrSctFrm=((SwFrm*)pCurrFrm)->FindSctFrm();
            if(pCurrSctFrm!=NULL && pCurrSctFrm->GetSection()!=NULL )
            {
                sAttrName = rtl::OUString::createFromAscii("section-name:");

                sValue += sAttrName;
                String sectionName = pCurrSctFrm->GetSection()->GetSectionName();

                sectionName.SearchAndReplace( String::CreateFromAscii( "\\" ), String::CreateFromAscii("\\\\" ));
                sectionName.SearchAndReplace( String::CreateFromAscii( "=" ), String::CreateFromAscii("\\=" ) );
                sectionName.SearchAndReplace( String::CreateFromAscii( ";" ), String::CreateFromAscii("\\;" ) );
                sectionName.SearchAndReplace( String::CreateFromAscii( "," ), String::CreateFromAscii("\\," ) );
                sectionName.SearchAndReplace( String::CreateFromAscii( ":" ), String::CreateFromAscii("\\:" ) );

                sValue += sectionName;
                //sValue += pCurrSctFrm->GetSection()->GetName();

                sValue += rtl::OUString::createFromAscii(";");

                //section-columns-number
                {
                sAttrName=rtl::OUString::createFromAscii("section-columns-number:");

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
                sValue += String::CreateFromInt32( nCurrCol ) ;
                sValue +=  rtl::OUString::createFromAscii(";");
                }

                //section-total-columns
                {
                sAttrName=rtl::OUString::createFromAscii("section-total-columns:");
                const SwFmtCol &rFmtSctCol=pCurrSctFrm->GetAttrSet()->GetCol();
                sal_uInt16 nSctColCount=rFmtSctCol.GetNumCols();
                nSctColCount = nSctColCount>0?nSctColCount:1;
                sValue += sAttrName;
                sValue += String::CreateFromInt32( nSctColCount ) ;

                sValue +=  rtl::OUString::createFromAscii(";");
                }
            }
        }
        anyAtrribute <<= sValue;
    }
    return anyAtrribute;
}

sal_Int32 SAL_CALL SwAccessibleDocument::getBackground()
        throw (::com::sun::star::uno::RuntimeException)
{
    //IAccessibility2 Implementation 2009-----
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SW_MOD()->GetColorConfig().GetColorValue( ::svtools::DOCCOLOR ).nColor;
    //-----IAccessibility2 Implementation 2009
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
        SAL_CALL SwAccessibleDocument::get_AccFlowTo(const ::com::sun::star::uno::Any& rAny, sal_Int32 nType)
        throw ( ::com::sun::star::uno::RuntimeException )
{
    const sal_Int32 FORSPELLCHECKFLOWTO = 1;
    const sal_Int32 FORFINDREPLACEFLOWTO = 2;
    SwAccessibleMap* pAccMap = GetMap();
    if ( !pAccMap )
    {
        goto Rt;
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
                uno::Reference<XAccessible> xAcc = pAccMap->GetContext(pObj, this, sal_False);
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
                    catch ( com::sun::star::lang::IndexOutOfBoundsException )
                    {
                        //return empty sequence
                        goto Rt;
                    }
                    //end of try...catch
                }
                /*uno::Sequence< uno::Any > aRet(1);
                aRet[0] = uno::makeAny( xAcc );
                return aRet;*/
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
            SwCntntNode* pPrevNode = NULL;
            std::vector<SwFrm*> vFrmList;
            do
            {
                if ( _pStartCrsr && _pStartCrsr->HasMark() )
                {
                    SwCntntNode* pCntntNode = _pStartCrsr->GetCntntNode();
                    if ( pCntntNode == pPrevNode )
                    {
                        continue;
                    }
                    SwFrm* pFrm = pCntntNode ? pCntntNode->getLayoutFrm( pCrsrShell->GetLayout() ) : NULL;
                    if ( pFrm )
                    {
                        vFrmList.push_back( pFrm );
                    }

                    pPrevNode = pCntntNode;
                }
            }

            while( _pStartCrsr && ( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != __pStartCrsr) );

            if ( vFrmList.size() )
            {
                uno::Sequence< uno::Any > aRet(vFrmList.size());
                std::vector<SwFrm*>::iterator aIter = vFrmList.begin();
                for ( sal_Int32 nIndex = 0; aIter != vFrmList.end(); aIter++, nIndex++ )
                {
                    uno::Reference< XAccessible > xAcc = pAccMap->GetContext(*aIter, sal_False);
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

Rt:
    uno::Sequence< uno::Any > aEmpty;
    return aEmpty;
}
//-----IAccessibility2 Implementation 2009
