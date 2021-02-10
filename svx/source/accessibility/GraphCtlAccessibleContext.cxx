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
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <o3tl/safeint.hxx>
#include <osl/mutex.hxx>
#include <tools/gen.hxx>
#include <svtools/colorcfg.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <svx/sdrpaintwindow.hxx>

#include <svx/ShapeTypeHandler.hxx>
#include <svx/AccessibleShapeInfo.hxx>
#include <GraphCtlAccessibleContext.hxx>
#include <svx/graphctl.hxx>
#include <svx/strings.hrc>
#include <svx/svdpage.hxx>
#include <svx/dialmgr.hxx>
#include <svx/sdrhittesthelper.hxx>

// namespaces
using namespace ::cppu;
using namespace ::osl;
using namespace ::accessibility;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;

// internal
/** initialize this component and set default values */
SvxGraphCtrlAccessibleContext::SvxGraphCtrlAccessibleContext(
    GraphCtrl&                   rRepr ) :

    SvxGraphCtrlAccessibleContext_Base( m_aMutex ),
    mpControl( &rRepr ),
    mpModel (nullptr),
    mpPage (nullptr),
    mpView (nullptr),
    mnClientId( 0 ),
    mbDisposed( false )
{
    if (mpControl != nullptr)
    {
        mpModel = mpControl->GetSdrModel();
        if (mpModel != nullptr)
            mpPage = mpModel->GetPage( 0 );
        mpView = mpControl->GetSdrView();

        if( mpModel == nullptr || mpPage == nullptr || mpView == nullptr )
        {
            mbDisposed = true;
            // Set all the pointers to NULL just in case they are used as
            // a disposed flag.
            mpModel = nullptr;
            mpPage = nullptr;
            mpView = nullptr;
        }
    }

    {
        ::SolarMutexGuard aSolarGuard;
        msName = SvxResId( RID_SVXSTR_GRAPHCTRL_ACC_NAME );
        msDescription = SvxResId( RID_SVXSTR_GRAPHCTRL_ACC_DESCRIPTION );
    }

    maTreeInfo.SetSdrView( mpView );
    maTreeInfo.SetWindow(&dynamic_cast<vcl::Window&>(mpControl->GetDrawingArea()->get_ref_device()));
    maTreeInfo.SetViewForwarder( this );
}


/** on destruction, this component is disposed and all dispose listeners
    are called, except if this component was already disposed */
SvxGraphCtrlAccessibleContext::~SvxGraphCtrlAccessibleContext()
{
    disposing();
}


/** returns the XAccessible interface for a given SdrObject.
    Multiple calls for the same SdrObject return the same XAccessible.
*/
Reference< XAccessible > SvxGraphCtrlAccessibleContext::getAccessible( const SdrObject* pObj )
{
    Reference<XAccessible> xAccessibleShape;

    if( pObj )
    {
        // see if we already created an XAccessible for the given SdrObject
        ShapesMapType::const_iterator iter = mxShapes.find( pObj );

        if( iter != mxShapes.end() )
        {
            // if we already have one, return it
            xAccessibleShape = (*iter).second.get();
        }
        else
        {
            // create a new one and remember in our internal map
            Reference< XShape > xShape( Reference< XShape >::query( const_cast<SdrObject*>(pObj)->getUnoShape() ) );

            css::uno::Reference<css::accessibility::XAccessible> xParent(getAccessibleParent());
            AccessibleShapeInfo aShapeInfo (xShape,xParent);
            // Create accessible object that corresponds to the descriptor's shape.
            rtl::Reference<AccessibleShape> pAcc(ShapeTypeHandler::Instance().CreateAccessibleObject(
                aShapeInfo, maTreeInfo));
            xAccessibleShape = pAcc.get();
            if (pAcc.is())
            {
                pAcc->Init ();
            }
            mxShapes[pObj] = pAcc;

            // Create event and inform listeners of the object creation.
            CommitChange( AccessibleEventId::CHILD, makeAny( xAccessibleShape ), makeAny( Reference<XAccessible>() ) );
        }
    }

    return xAccessibleShape;
}

// XAccessible
Reference< XAccessibleContext > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleContext()
{
    return this;
}

// XAccessibleComponent
sal_Bool SAL_CALL SvxGraphCtrlAccessibleContext::containsPoint( const awt::Point& rPoint )
{
    // no guard -> done in getSize()
    awt::Size aSize (getSize());
    return (rPoint.X >= 0)
        && (rPoint.X < aSize.Width)
        && (rPoint.Y >= 0)
        && (rPoint.Y < aSize.Height);
}


Reference< XAccessible > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleAtPoint( const awt::Point& rPoint )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    Reference< XAccessible > xAccessible;

    if( !mpControl )
    {
        throw DisposedException();
    }

    Point aPnt( rPoint.X, rPoint.Y );
    mpControl->GetDrawingArea()->get_ref_device().PixelToLogic( aPnt );

    SdrObject* pObj = nullptr;

    if(mpView && mpView->GetSdrPageView())
    {
        pObj = SdrObjListPrimitiveHit(*mpPage, aPnt, 1, *mpView->GetSdrPageView(), nullptr, false);
    }

    if( pObj )
        xAccessible = getAccessible( pObj );

    return xAccessible;
}

awt::Rectangle SAL_CALL SvxGraphCtrlAccessibleContext::getBounds()
{
    const SolarMutexGuard aSolarGuard;

    if (nullptr == mpControl)
        throw DisposedException();

    const Point         aOutPos;
    const Size          aOutSize( mpControl->GetOutputSizePixel() );
    awt::Rectangle      aRet;

    aRet.X = aOutPos.X();
    aRet.Y = aOutPos.Y();
    aRet.Width = aOutSize.Width();
    aRet.Height = aOutSize.Height();

    return aRet;
}

awt::Point SAL_CALL SvxGraphCtrlAccessibleContext::getLocation()
{
    const SolarMutexGuard aSolarGuard;

    if (nullptr == mpControl)
        throw DisposedException();

    const awt::Rectangle    aRect( getBounds() );
    awt::Point              aRet;

    aRet.X = aRect.X;
    aRet.Y = aRect.Y;

    return aRet;
}

awt::Point SAL_CALL SvxGraphCtrlAccessibleContext::getLocationOnScreen()
{
    const SolarMutexGuard aSolarGuard;

    if (nullptr == mpControl)
        throw DisposedException();

    awt::Point aScreenLoc(0, 0);

    auto xParent(getAccessibleParent());
    if (xParent)
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xParentContext(xParent->getAccessibleContext());
        css::uno::Reference<css::accessibility::XAccessibleComponent> xParentComponent(xParentContext, css::uno::UNO_QUERY);
        OSL_ENSURE( xParentComponent.is(), "ValueSetAcc::getLocationOnScreen: no parent component!" );
        if ( xParentComponent.is() )
        {
            awt::Point aParentScreenLoc( xParentComponent->getLocationOnScreen() );
            awt::Point aOwnRelativeLoc( getLocation() );
            aScreenLoc.X = aParentScreenLoc.X + aOwnRelativeLoc.X;
            aScreenLoc.Y = aParentScreenLoc.Y + aOwnRelativeLoc.Y;
        }
    }

    return aScreenLoc;
}

awt::Size SAL_CALL SvxGraphCtrlAccessibleContext::getSize()
{
    const SolarMutexGuard aSolarGuard;

    if (nullptr == mpControl)
        throw DisposedException();

    const awt::Rectangle    aRect( getBounds() );
    awt::Size               aRet;

    aRet.Width = aRect.Width;
    aRet.Height = aRect.Height;

    return aRet;
}

// XAccessibleContext
sal_Int32 SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleChildCount()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpPage )
        throw DisposedException();

    return mpPage->GetObjCount();
}


/** returns the SdrObject at index nIndex from the model of this graph */
SdrObject* SvxGraphCtrlAccessibleContext::getSdrObject( sal_Int32 nIndex )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpPage )
        throw DisposedException();

    if( (nIndex < 0) || ( o3tl::make_unsigned(nIndex) >= mpPage->GetObjCount() ) )
        throw lang::IndexOutOfBoundsException();

    return mpPage->GetObj( nIndex );
}


/** sends an AccessibleEventObject to all added XAccessibleEventListeners */
void SvxGraphCtrlAccessibleContext::CommitChange (
    sal_Int16 nEventId,
    const uno::Any& rNewValue,
    const uno::Any& rOldValue)
{
    AccessibleEventObject aEvent (
        static_cast<uno::XWeak*>(this),
        nEventId,
        rNewValue,
        rOldValue);

    if (mnClientId)
        comphelper::AccessibleEventNotifier::addEvent( mnClientId, aEvent );
}

Reference< XAccessible > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleChild( sal_Int32 nIndex )
{
    ::SolarMutexGuard aGuard;

    return getAccessible( getSdrObject( nIndex ) );
}

Reference< XAccessible > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleParent()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpControl )
        throw DisposedException();

    return mpControl->GetDrawingArea()->get_accessible_parent();
}

sal_Int32 SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleIndexInParent()
{
    ::SolarMutexGuard aGuard;
    //  Use a simple but slow solution for now.  Optimize later.

    //  Iterate over all the parent's children and search for this object.
    css::uno::Reference<css::accessibility::XAccessible> xParent(getAccessibleParent());
    if (xParent.is())
    {
        Reference< XAccessibleContext > xParentContext( xParent->getAccessibleContext() );
        if( xParentContext.is() )
        {
            sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
            for( sal_Int32 i = 0 ; i < nChildCount ; ++i )
            {
                Reference< XAccessible > xChild( xParentContext->getAccessibleChild( i ) );
                if( xChild.is() )
                {
                    Reference< XAccessibleContext > xChildContext = xChild->getAccessibleContext();
                    if( xChildContext == static_cast<XAccessibleContext*>(this) )
                        return i;
                }
            }
        }
    }

    //   Return -1 to indicate that this object's parent does not know about the
    //   object.
    return -1;
}


sal_Int16 SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleRole()
{
    return AccessibleRole::PANEL;
}


OUString SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleDescription()
{
    ::SolarMutexGuard aGuard;
    return msDescription;
}


OUString SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleName()
{
    ::SolarMutexGuard aGuard;
    return msName;
}


/** Return empty reference to indicate that the relation set is not
    supported.
*/
Reference< XAccessibleRelationSet > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleRelationSet()
{
    return Reference< XAccessibleRelationSet >();
}


Reference< XAccessibleStateSet > SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleStateSet()
{
    ::SolarMutexGuard aGuard;

    rtl::Reference<utl::AccessibleStateSetHelper> pStateSetHelper = new utl::AccessibleStateSetHelper;

    if ( rBHelper.bDisposed || mbDisposed )
    {
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );
    }
    else
    {
        pStateSetHelper->AddState( AccessibleStateType::FOCUSABLE );
        if( mpControl->HasFocus() )
            pStateSetHelper->AddState( AccessibleStateType::FOCUSED );
        pStateSetHelper->AddState( AccessibleStateType::OPAQUE );
        pStateSetHelper->AddState( AccessibleStateType::SHOWING );
        pStateSetHelper->AddState( AccessibleStateType::VISIBLE );
    }

    return pStateSetHelper;
}


lang::Locale SAL_CALL SvxGraphCtrlAccessibleContext::getLocale()
{
    ::SolarMutexGuard aGuard;

    css::uno::Reference<css::accessibility::XAccessible> xParent(getAccessibleParent());
    if (xParent.is())
    {
        Reference< XAccessibleContext > xParentContext( xParent->getAccessibleContext() );
        if( xParentContext.is() )
            return xParentContext->getLocale();
    }

    //  No parent.  Therefore throw exception to indicate this cluelessness.
    throw IllegalAccessibleComponentStateException();
}

// XAccessibleEventListener
void SAL_CALL SvxGraphCtrlAccessibleContext::addAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener )
{
    if (xListener.is())
    {
        ::SolarMutexGuard aGuard;
        if (!mnClientId)
            mnClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( mnClientId, xListener );
    }
}


void SAL_CALL SvxGraphCtrlAccessibleContext::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener )
{
    if (!xListener.is())
        return;

    ::SolarMutexGuard aGuard;

    sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( mnClientId, xListener );
    if ( !nListenerCount )
    {
        // no listeners anymore
        // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
        // and at least to us not firing any events anymore, in case somebody calls
        // NotifyAccessibleEvent, again
        comphelper::AccessibleEventNotifier::revokeClient( mnClientId );
        mnClientId = 0;
    }
}

void SAL_CALL SvxGraphCtrlAccessibleContext::grabFocus()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpControl )
        throw DisposedException();

    mpControl->GrabFocus();
}

sal_Int32 SAL_CALL SvxGraphCtrlAccessibleContext::getForeground()
{
    svtools::ColorConfig aColorConfig;
    Color nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL SvxGraphCtrlAccessibleContext::getBackground()
{
    Color nColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
    return static_cast<sal_Int32>(nColor);
}

// XServiceInfo
OUString SAL_CALL SvxGraphCtrlAccessibleContext::getImplementationName()
{
    return "com.sun.star.comp.ui.SvxGraphCtrlAccessibleContext";
}

sal_Bool SAL_CALL SvxGraphCtrlAccessibleContext::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

Sequence< OUString > SAL_CALL SvxGraphCtrlAccessibleContext::getSupportedServiceNames()
{
    return { "com.sun.star.accessibility.Accessible",
             "com.sun.star.accessibility.AccessibleContext",
             "com.sun.star.drawing.AccessibleGraphControl" };
}

// XTypeProvider
Sequence<sal_Int8> SAL_CALL SvxGraphCtrlAccessibleContext::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceName
OUString SvxGraphCtrlAccessibleContext::getServiceName()
{
    return "com.sun.star.accessibility.AccessibleContext";
}

// XAccessibleSelection
void SAL_CALL SvxGraphCtrlAccessibleContext::selectAccessibleChild( sal_Int32 nIndex )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpView )
        throw DisposedException();

    SdrObject* pObj = getSdrObject( nIndex );

    if( pObj )
        mpView->MarkObj( pObj, mpView->GetSdrPageView());
}


sal_Bool SAL_CALL SvxGraphCtrlAccessibleContext::isAccessibleChildSelected( sal_Int32 nIndex )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpView )
        throw DisposedException();

    return mpView->IsObjMarked( getSdrObject( nIndex ) );
}


void SAL_CALL SvxGraphCtrlAccessibleContext::clearAccessibleSelection()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpView )
        throw DisposedException();

    mpView->UnmarkAllObj();
}


void SAL_CALL SvxGraphCtrlAccessibleContext::selectAllAccessibleChildren()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpView )
        throw DisposedException();

    mpView->MarkAllObj();
}


sal_Int32 SAL_CALL SvxGraphCtrlAccessibleContext::getSelectedAccessibleChildCount()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpView )
        throw DisposedException();

    const SdrMarkList& rList = mpView->GetMarkedObjectList();
    return static_cast<sal_Int32>(rList.GetMarkCount());
}


Reference< XAccessible > SAL_CALL SvxGraphCtrlAccessibleContext::getSelectedAccessibleChild( sal_Int32 nIndex )
{
    ::SolarMutexGuard aGuard;

    checkChildIndexOnSelection( nIndex );

    Reference< XAccessible > xAccessible;

    const SdrMarkList& rList = mpView->GetMarkedObjectList();
    SdrObject* pObj = rList.GetMark(static_cast<size_t>(nIndex))->GetMarkedSdrObj();
    if( pObj )
        xAccessible = getAccessible( pObj );

    return xAccessible;
}


void SAL_CALL SvxGraphCtrlAccessibleContext::deselectAccessibleChild( sal_Int32 nIndex )
{
    ::SolarMutexGuard aGuard;

    checkChildIndexOnSelection( nIndex );

    if( !mpView )
        return;

    const SdrMarkList& rList = mpView->GetMarkedObjectList();

    SdrObject* pObj = getSdrObject( nIndex );
    if( !pObj )
        return;

    SdrMarkList aRefList( rList );

    SdrPageView* pPV = mpView->GetSdrPageView();
    mpView->UnmarkAllObj( pPV );

    const size_t nCount = aRefList.GetMarkCount();
    for( size_t nMark = 0; nMark < nCount; ++nMark )
    {
        if( aRefList.GetMark(nMark)->GetMarkedSdrObj() != pObj )
            mpView->MarkObj( aRefList.GetMark(nMark)->GetMarkedSdrObj(), pPV );
    }
}

// internals
void SvxGraphCtrlAccessibleContext::checkChildIndexOnSelection( tools::Long nIndex )
{
    if( nIndex < 0 || nIndex >= getSelectedAccessibleChildCount() )
        throw lang::IndexOutOfBoundsException();
}


/** Replace the model, page, and view pointers by the ones provided
    (explicitly and implicitly).
*/
void SvxGraphCtrlAccessibleContext::setModelAndView (
    SdrModel* pModel,
    SdrView* pView)
{
    ::SolarMutexGuard aGuard;

    mpModel = pModel;
    if (mpModel != nullptr)
        mpPage = mpModel->GetPage( 0 );
    mpView = pView;

    if (mpModel == nullptr || mpPage == nullptr || mpView == nullptr)
    {
        mbDisposed = true;

        // Set all the pointers to NULL just in case they are used as
        // a disposed flag.
        mpModel = nullptr;
        mpPage = nullptr;
        mpView = nullptr;
    }

    maTreeInfo.SetSdrView (mpView);
}


void SAL_CALL SvxGraphCtrlAccessibleContext::disposing()
{
    ::SolarMutexGuard aGuard;

    if( mbDisposed )
        return;

    mbDisposed = true;

    mpControl = nullptr;       // object dies with representation
    mpView = nullptr;
    mpPage = nullptr;

    {
        for (const auto& rEntry : mxShapes)
        {
            rtl::Reference<XAccessible> pAcc(rEntry.second);
            Reference< XComponent > xComp( pAcc.get(), UNO_QUERY );
            if( xComp.is() )
                xComp->dispose();
        }

        mxShapes.clear();
    }

    // Send a disposing to all listeners.
    if ( mnClientId )
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
        mnClientId =  0;
    }
}

void SvxGraphCtrlAccessibleContext::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if (rHint.GetId() == SfxHintId::ThisIsAnSdrHint)
    {
        const SdrHint* pSdrHint = static_cast<const SdrHint*>( &rHint );
        switch( pSdrHint->GetKind() )
        {
            case SdrHintKind::ObjectChange:
                {
                    ShapesMapType::iterator iter = mxShapes.find( pSdrHint->GetObject() );

                    if( iter != mxShapes.end() )
                    {
                        // if we already have one, return it
                        rtl::Reference<AccessibleShape> pShape((*iter).second);

                        if( pShape.is() )
                            pShape->CommitChange( AccessibleEventId::VISIBLE_DATA_CHANGED, uno::Any(), uno::Any() );
                    }
                }
                break;

            case SdrHintKind::ObjectInserted:
                CommitChange( AccessibleEventId::CHILD, makeAny( getAccessible( pSdrHint->GetObject() ) ) , uno::Any());
                break;
            case SdrHintKind::ObjectRemoved:
                CommitChange( AccessibleEventId::CHILD, uno::Any(), makeAny( getAccessible( pSdrHint->GetObject() ) )  );
                break;
            case SdrHintKind::ModelCleared:
                dispose();
                break;
            default:
                break;
        }
    }
    else
    {
        // Has our SdDrawDocument just died?
        if(rHint.GetId() == SfxHintId::Dying)
        {
            dispose();
        }
    }
}

// IAccessibleViewforwarder
tools::Rectangle SvxGraphCtrlAccessibleContext::GetVisibleArea() const
{
    tools::Rectangle aVisArea;

    if( mpView && mpView->PaintWindowCount())
    {
        SdrPaintWindow* pPaintWindow = mpView->GetPaintWindow(0);
        aVisArea = pPaintWindow->GetVisibleArea();
    }

    return aVisArea;
}

Point SvxGraphCtrlAccessibleContext::LogicToPixel (const Point& rPoint) const
{
    if( mpControl )
    {
        return mpControl->GetDrawingArea()->get_ref_device().LogicToPixel (rPoint) + mpControl->GetPositionInDialog();
    }
    else
    {
        return rPoint;
    }
}

Size SvxGraphCtrlAccessibleContext::LogicToPixel (const Size& rSize) const
{
    if( mpControl )
        return mpControl->GetDrawingArea()->get_ref_device().LogicToPixel(rSize);
    else
        return rSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
