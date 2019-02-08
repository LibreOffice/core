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
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <osl/mutex.hxx>
#include <tools/gen.hxx>
#include <toolkit/helper/convert.hxx>
#include <svtools/colorcfg.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <svx/sdrpaintwindow.hxx>

#include <svx/ShapeTypeHandler.hxx>
#include <svx/AccessibleShapeInfo.hxx>
#include <GraphCtlAccessibleContext.hxx>
#include <svx/graphctl.hxx>
#include <svx/strings.hrc>
#include <svx/svdpage.hxx>
#include <svx/unomod.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svdetc.hxx>
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
    const Reference< XAccessible >& rxParent,
    GraphCtrl&                      rRepr ) :

    SvxGraphCtrlAccessibleContext_Base( m_aMutex ),
    mxParent( rxParent ),
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
    maTreeInfo.SetWindow( mpControl );
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

            AccessibleShapeInfo aShapeInfo (xShape,mxParent);
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
    mpControl->PixelToLogic( aPnt );

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
    // no guard -> done in GetBoundingBox()
    tools::Rectangle           aCoreBounds( GetBoundingBox() );
    awt::Rectangle      aBounds;
    aBounds.X = aCoreBounds.getX();
    aBounds.Y = aCoreBounds.getY();
    aBounds.Width = aCoreBounds.getWidth();
    aBounds.Height = aCoreBounds.getHeight();
    return aBounds;
}


awt::Point SAL_CALL SvxGraphCtrlAccessibleContext::getLocation()
{
    // no guard -> done in GetBoundingBox()
    tools::Rectangle   aRect( GetBoundingBox() );
    return awt::Point( aRect.getX(), aRect.getY() );
}


awt::Point SAL_CALL SvxGraphCtrlAccessibleContext::getLocationOnScreen()
{
    // no guard -> done in GetBoundingBoxOnScreen()
    tools::Rectangle   aRect( GetBoundingBoxOnScreen() );
    return awt::Point( aRect.getX(), aRect.getY() );
}


awt::Size SAL_CALL SvxGraphCtrlAccessibleContext::getSize()
{
    // no guard -> done in GetBoundingBox()
    tools::Rectangle   aRect( GetBoundingBox() );
    return awt::Size( aRect.getWidth(), aRect.getHeight() );
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

    if( (nIndex < 0) || ( static_cast<size_t>(nIndex) >= mpPage->GetObjCount() ) )
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
    return mxParent;
}


sal_Int32 SAL_CALL SvxGraphCtrlAccessibleContext::getAccessibleIndexInParent()
{
    ::SolarMutexGuard aGuard;
    //  Use a simple but slow solution for now.  Optimize later.

    //  Iterate over all the parent's children and search for this object.
    if( mxParent.is() )
    {
        Reference< XAccessibleContext > xParentContext( mxParent->getAccessibleContext() );
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

    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;

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

    if( mxParent.is() )
    {
        Reference< XAccessibleContext > xParentContext( mxParent->getAccessibleContext() );
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
    if (xListener.is())
    {
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
    return OUString( "com.sun.star.comp.ui.SvxGraphCtrlAccessibleContext" );
}

sal_Bool SAL_CALL SvxGraphCtrlAccessibleContext::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

Sequence< OUString > SAL_CALL SvxGraphCtrlAccessibleContext::getSupportedServiceNames()
{
    Sequence< OUString > aSNs( 3 );

    aSNs[0] = "com.sun.star.accessibility.Accessible";
    aSNs[1] = "com.sun.star.accessibility.AccessibleContext";
    aSNs[2] = "com.sun.star.drawing.AccessibleGraphControl";

    return aSNs;
}

// XTypeProvider
Sequence<sal_Int8> SAL_CALL SvxGraphCtrlAccessibleContext::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceName
OUString SvxGraphCtrlAccessibleContext::getServiceName()
{
    return OUString( "com.sun.star.accessibility.AccessibleContext" );
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

    if( mpView )
    {
        const SdrMarkList& rList = mpView->GetMarkedObjectList();

        SdrObject* pObj = getSdrObject( nIndex );
        if( pObj )
        {
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
    }
}

// internals
void SvxGraphCtrlAccessibleContext::checkChildIndexOnSelection( long nIndex )
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
            rtl::Reference<XAccessible> pAcc(rEntry.second.get());
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


tools::Rectangle SvxGraphCtrlAccessibleContext::GetBoundingBoxOnScreen()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpControl )
        throw DisposedException();

    return tools::Rectangle(
        mpControl->GetAccessibleParentWindow()->OutputToAbsoluteScreenPixel(
            mpControl->GetPosPixel() ),
        mpControl->GetSizePixel() );
}


/** Calculate the relative coordinates of the bounding box as difference
    between the absolute coordinates of the bounding boxes of this control
    and its parent in the accessibility tree.
*/
tools::Rectangle SvxGraphCtrlAccessibleContext::GetBoundingBox()
{
    ::SolarMutexGuard aGuard;

    tools::Rectangle aBounds ( 0, 0, 0, 0 );

    vcl::Window* pWindow = mpControl;
    if (pWindow == nullptr)
        throw DisposedException();

    aBounds = pWindow->GetWindowExtentsRelative (nullptr);
    vcl::Window* pParent = pWindow->GetAccessibleParentWindow();
    if (pParent != nullptr)
    {
        tools::Rectangle aParentRect = pParent->GetWindowExtentsRelative (nullptr);
        aBounds -= aParentRect.TopLeft();
    }

    return aBounds;
}

void SvxGraphCtrlAccessibleContext::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>( &rHint );

    if( pSdrHint )
    {
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
        tools::Rectangle aBBox(mpControl->GetWindowExtentsRelative(nullptr));
        return mpControl->LogicToPixel (rPoint) + aBBox.TopLeft();
    }
    else
    {
        return rPoint;
    }
}


Size SvxGraphCtrlAccessibleContext::LogicToPixel (const Size& rSize) const
{
    if( mpControl )
        return mpControl->LogicToPixel (rSize);
    else
        return rSize;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
