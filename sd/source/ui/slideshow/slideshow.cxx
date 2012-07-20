/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <comphelper/serviceinfohelper.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>

#include <cppuhelper/bootstrap.hxx>

#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>

#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svx/svdpool.hxx>
#include <svl/itemprop.hxx>

#include <sfx2/viewfrm.hxx>

#include <toolkit/unohlp.hxx>
#include <svx/unoprov.hxx>

#include "framework/FrameworkHelper.hxx"

#include "FrameView.hxx"
#include "unomodel.hxx"
#include "slideshow.hxx"
#include "slideshowimpl.hxx"
#include "sdattr.hrc"
#include "FactoryIds.hxx"
#include "ViewShell.hxx"
#include "SlideShowRestarter.hxx"
#include "DrawController.hxx"
#include <boost/bind.hpp>

using ::com::sun::star::presentation::XSlideShowController;
using ::com::sun::star::container::XIndexAccess;
using ::sd::framework::FrameworkHelper;
using ::rtl::OUString;
using ::com::sun::star::awt::XWindow;
using namespace ::sd;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::drawing::framework;

extern String getUiNameFromPageApiNameImpl( const ::rtl::OUString& rApiName );


namespace {
    /** This local version of the work window overloads DataChanged() so that it
        can restart the slide show when a display is added or removed.
    */
    class FullScreenWorkWindow : public WorkWindow
    {
    public:
        FullScreenWorkWindow (
            const ::rtl::Reference<SlideShow>& rpSlideShow,
            ViewShellBase* pViewShellBase)
            : WorkWindow(NULL, WB_HIDE | WB_CLIPCHILDREN),
              mpRestarter(new SlideShowRestarter(rpSlideShow, pViewShellBase))
        {}

        void Restart(bool bForce)
        {
            mpRestarter->Restart(bForce);
        }

        virtual void DataChanged (const DataChangedEvent& rEvent)
        {
            if (rEvent.GetType() == DATACHANGED_DISPLAY)
                Restart(false);
        }

    private:
        ::boost::shared_ptr<SlideShowRestarter> mpRestarter;
    };
}


//////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------------------

const SfxItemPropertyMapEntry* ImplGetPresentationPropertyMap()
{
    // NOTE: First member must be sorted
    static const SfxItemPropertyMapEntry aPresentationPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("AllowAnimations"),          ATTR_PRESENT_ANIMATION_ALLOWED, &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("CustomShow"),               ATTR_PRESENT_CUSTOMSHOW,        &::getCppuType((const OUString*)0),     0, 0 },
        { MAP_CHAR_LEN("Display"),                  ATTR_PRESENT_DISPLAY,           &::getCppuType((const sal_Int32*)0),    0, 0 },
        { MAP_CHAR_LEN("FirstPage"),                ATTR_PRESENT_DIANAME,           &::getCppuType((const OUString*)0),     0, 0 },
        { MAP_CHAR_LEN("IsAlwaysOnTop"),            ATTR_PRESENT_ALWAYS_ON_TOP,     &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("IsAutomatic"),              ATTR_PRESENT_MANUEL,            &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("IsEndless"),                ATTR_PRESENT_ENDLESS,           &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("IsFullScreen"),             ATTR_PRESENT_FULLSCREEN,        &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("IsShowAll"),                ATTR_PRESENT_ALL,               &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("IsMouseVisible"),           ATTR_PRESENT_MOUSE,             &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("IsShowLogo"),               ATTR_PRESENT_SHOW_PAUSELOGO,    &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("IsTransitionOnClick"),      ATTR_PRESENT_CHANGE_PAGE,       &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("Pause"),                    ATTR_PRESENT_PAUSE_TIMEOUT,     &::getCppuType((const sal_Int32*)0),    0, 0 },
        { MAP_CHAR_LEN("StartWithNavigator"),       ATTR_PRESENT_NAVIGATOR,         &::getBooleanCppuType(),                0, 0 },
        { MAP_CHAR_LEN("UsePen"),                   ATTR_PRESENT_PEN,               &::getBooleanCppuType(),                0, 0 },
        { 0,0,0,0,0,0}
    };

    return aPresentationPropertyMap_Impl;
}

// --------------------------------------------------------------------
// class SlideShow
// --------------------------------------------------------------------

SlideShow::SlideShow( SdDrawDocument* pDoc )
: SlideshowBase( m_aMutex )
, maPropSet(ImplGetPresentationPropertyMap(), SdrObject::GetGlobalDrawObjectItemPool())
, mbIsInStartup(false)
, mpDoc( pDoc )
, mpCurrentViewShellBase( 0 )
, mpFullScreenViewShellBase( 0 )
, mpFullScreenFrameView( 0 )
, mnInPlaceConfigEvent( 0 )
{
}

// --------------------------------------------------------------------

void SlideShow::ThrowIfDisposed() throw (RuntimeException)
{
    if( mpDoc == 0 )
        throw DisposedException();
}

// --------------------------------------------------------------------

/// used by the model to create a slideshow for it
rtl::Reference< SlideShow > SlideShow::Create( SdDrawDocument* pDoc )
{
    return new SlideShow( pDoc );
}

// --------------------------------------------------------------------

rtl::Reference< SlideShow > SlideShow::GetSlideShow( SdDrawDocument* pDocument )
{
    rtl::Reference< SlideShow > xRet;

    if( pDocument )
        xRet = GetSlideShow( *pDocument );

    return xRet;
}

// --------------------------------------------------------------------

rtl::Reference< SlideShow > SlideShow::GetSlideShow( SdDrawDocument& rDocument )
{
    return rtl::Reference< SlideShow >(
        dynamic_cast< SlideShow* >( rDocument.getPresentation().get() ) );
}

// --------------------------------------------------------------------

rtl::Reference< SlideShow > SlideShow::GetSlideShow( ViewShellBase& rBase )
{
    return GetSlideShow( rBase.GetDocument() );
}

// --------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::presentation::XSlideShowController > SlideShow::GetSlideShowController(ViewShellBase& rBase )
{
    rtl::Reference< SlideShow > xSlideShow( GetSlideShow( rBase ) );

    Reference< XSlideShowController > xRet;
    if( xSlideShow.is() )
        xRet = xSlideShow->getController();

    return xRet;
}

// --------------------------------------------------------------------

bool SlideShow::StartPreview( ViewShellBase& rBase,
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xDrawPage,
    const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xAnimationNode,
    ::Window* pParent /* = 0 */ )
{
    rtl::Reference< SlideShow > xSlideShow( GetSlideShow( rBase ) );
    if( xSlideShow.is() )
        return xSlideShow->startPreview( xDrawPage, xAnimationNode, pParent );

    return false;
}

// --------------------------------------------------------------------

void SlideShow::Stop( ViewShellBase& rBase )
{
    rtl::Reference< SlideShow > xSlideShow( GetSlideShow( rBase ) );
    if( xSlideShow.is() )
        xSlideShow->end();
}

// --------------------------------------------------------------------

bool SlideShow::IsRunning( ViewShellBase& rBase )
{
    rtl::Reference< SlideShow > xSlideShow( GetSlideShow( rBase ) );
    return xSlideShow.is() && xSlideShow->isRunning();
}

// --------------------------------------------------------------------

bool SlideShow::IsRunning( ViewShell& rViewShell )
{
    rtl::Reference< SlideShow > xSlideShow( GetSlideShow( rViewShell.GetViewShellBase() ) );
    return xSlideShow.is() && xSlideShow->isRunning() && (xSlideShow->mxController->getViewShell() == &rViewShell);
}

// --------------------------------------------------------------------

void SlideShow::CreateController(  ViewShell* pViewSh, ::sd::View* pView, ::Window* pParentWindow )
{
    DBG_ASSERT( !mxController.is(), "sd::SlideShow::CreateController(), clean up old controller first!" );

    Reference< XPresentation2 > xThis( this );

    rtl::Reference<SlideshowImpl> xController (
        new SlideshowImpl(xThis, pViewSh, pView, mpDoc, pParentWindow));

    // Reset mbIsInStartup.  From here mxController.is() is used to prevent
    // multiple slide show instances for one document.
    mxController = xController;
    mbIsInStartup = false;

}

// --------------------------------------------------------------------
// XServiceInfo
// --------------------------------------------------------------------

OUString SAL_CALL SlideShow::getImplementationName(  ) throw(RuntimeException)
{
    return OUString( "com.sun.star.comp.sd.SlideShow" );
}

// --------------------------------------------------------------------

sal_Bool SAL_CALL SlideShow::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames(  ) );
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL SlideShow::getSupportedServiceNames(  ) throw(RuntimeException)
{
    OUString aService( "com.sun.star.presentation.Presentation" );
    Sequence< OUString > aSeq( &aService, 1 );
    return aSeq;
}

// --------------------------------------------------------------------
// XPropertySet
// --------------------------------------------------------------------

Reference< XPropertySetInfo > SAL_CALL SlideShow::getPropertySetInfo() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    static Reference< XPropertySetInfo > xInfo = maPropSet.getPropertySetInfo();
    return xInfo;
 }

// --------------------------------------------------------------------

void SAL_CALL SlideShow::setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    ThrowIfDisposed();

    sd::PresentationSettings& rPresSettings = mpDoc->getPresentationSettings();

    const SfxItemPropertySimpleEntry* pEntry = maPropSet.getPropertyMapEntry(aPropertyName);

    if( pEntry && ((pEntry->nFlags & PropertyAttribute::READONLY) != 0) )
        throw PropertyVetoException();

    bool bValuesChanged = false;
    bool bIllegalArgument = true;

    switch( pEntry ? pEntry->nWID : -1 )
    {
    case ATTR_PRESENT_ALL:
    {
        sal_Bool bVal = sal_False;

        if( aValue >>= bVal )
        {
            bIllegalArgument = false;

            if( rPresSettings.mbAll != bVal )
            {
                rPresSettings.mbAll = bVal;
                bValuesChanged = true;
                if( bVal )
                    rPresSettings.mbCustomShow = sal_False;
            }
        }
        break;
    }
    case ATTR_PRESENT_CHANGE_PAGE:
    {
        sal_Bool bVal = sal_False;

        if( aValue >>= bVal )
        {
            bIllegalArgument = false;

            if( bVal == rPresSettings.mbLockedPages )
            {
                bValuesChanged = true;
                rPresSettings.mbLockedPages = !bVal;
            }
        }
        break;
    }

    case ATTR_PRESENT_ANIMATION_ALLOWED:
    {
        sal_Bool bVal = sal_False;

        if( aValue >>= bVal )
        {
            bIllegalArgument = false;

            if(rPresSettings.mbAnimationAllowed != bVal)
            {
                bValuesChanged = true;
                rPresSettings.mbAnimationAllowed = bVal;
            }
        }
        break;
    }
    case ATTR_PRESENT_CUSTOMSHOW:
    {
        OUString aShow;
        if( aValue >>= aShow )
        {
            bIllegalArgument = false;

            const String aShowName( aShow );

            List* pCustomShowList = mpDoc->GetCustomShowList(sal_False);
            if(pCustomShowList)
            {
                SdCustomShow* pCustomShow;
                for( pCustomShow = (SdCustomShow*) pCustomShowList->First(); pCustomShow != NULL; pCustomShow = (SdCustomShow*) pCustomShowList->Next() )
                {
                    if( pCustomShow->GetName() == aShowName )
                        break;
                }

                rPresSettings.mbCustomShow = sal_True;
                bValuesChanged = true;
            }
        }
        break;
    }
    case ATTR_PRESENT_ENDLESS:
    {
        sal_Bool bVal = sal_False;

        if( aValue >>= bVal )
        {
            bIllegalArgument = false;

            if( rPresSettings.mbEndless != bVal)
            {
                bValuesChanged = true;
                rPresSettings.mbEndless = bVal;
            }
        }
        break;
    }
    case ATTR_PRESENT_FULLSCREEN:
    {
        sal_Bool bVal = sal_False;

        if( aValue >>= bVal )
        {
            bIllegalArgument = false;
            if( rPresSettings.mbFullScreen != bVal)
            {
                bValuesChanged = true;
                rPresSettings.mbFullScreen = bVal;
            }
        }
        break;
    }
    case ATTR_PRESENT_DIANAME:
    {
        OUString aPresPage;
        aValue >>= aPresPage;
        bIllegalArgument = false;
        if( (rPresSettings.maPresPage != aPresPage) || !rPresSettings.mbCustomShow || !rPresSettings.mbAll )
        {
            bValuesChanged = true;
            rPresSettings.maPresPage = getUiNameFromPageApiNameImpl(aPresPage);
            rPresSettings.mbCustomShow = sal_False;
            rPresSettings.mbAll = sal_False;
        }
        break;
    }
    case ATTR_PRESENT_MANUEL:
    {
        sal_Bool bVal = sal_False;

        if( aValue >>= bVal )
        {
            bIllegalArgument = false;

            if( rPresSettings.mbManual != bVal)
            {
                bValuesChanged = true;
                rPresSettings.mbManual = bVal;
            }
        }
        break;
    }
    case ATTR_PRESENT_MOUSE:
    {
        sal_Bool bVal = sal_False;

        if( aValue >>= bVal )
        {
            bIllegalArgument = false;
            if( rPresSettings.mbMouseVisible != bVal)
            {
                bValuesChanged = true;
                rPresSettings.mbMouseVisible = bVal;
            }
        }
        break;
    }
    case ATTR_PRESENT_ALWAYS_ON_TOP:
    {
        sal_Bool bVal = sal_False;

        if( aValue >>= bVal )
        {
            bIllegalArgument = false;

            if( rPresSettings.mbAlwaysOnTop != bVal)
            {
                bValuesChanged = true;
                rPresSettings.mbAlwaysOnTop = bVal;
            }
        }
        break;
    }
    case ATTR_PRESENT_NAVIGATOR:
    {
        sal_Bool bVal = sal_False;

        if( aValue >>= bVal )
        {
            bIllegalArgument = false;

            if( rPresSettings.mbStartWithNavigator != bVal)
            {
                bValuesChanged = true;
                rPresSettings.mbStartWithNavigator = bVal;
            }
        }
        break;
    }
    case ATTR_PRESENT_PEN:
    {
        sal_Bool bVal = sal_False;

        if( aValue >>= bVal )
        {
            bIllegalArgument = false;

            if(rPresSettings.mbMouseAsPen != bVal)
            {
                bValuesChanged = true;
                rPresSettings.mbMouseAsPen = bVal;
            }
        }
        break;
    }
    case ATTR_PRESENT_PAUSE_TIMEOUT:
    {
        sal_Int32 nValue = 0;
        if( (aValue >>= nValue) && (nValue >= 0) )
        {
            bIllegalArgument = false;
            if( rPresSettings.mnPauseTimeout != nValue )
            {
                bValuesChanged = true;
                rPresSettings.mnPauseTimeout = nValue;
            }
        }
        break;
    }
    case ATTR_PRESENT_SHOW_PAUSELOGO:
    {
        sal_Bool bVal = sal_False;

        if( aValue >>= bVal )
        {
            bIllegalArgument = false;

            if( rPresSettings.mbShowPauseLogo != bVal )
            {
                bValuesChanged = true;
                rPresSettings.mbShowPauseLogo = bVal;
            }
        }
        break;
    }
    case ATTR_PRESENT_DISPLAY:
    {
        sal_Int32 nDisplay = 0;
        if( aValue >>= nDisplay )
        {
            bIllegalArgument = false;

            SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
            pOptions->SetDisplay( nDisplay );

            FullScreenWorkWindow *pWin = dynamic_cast<FullScreenWorkWindow *>(GetWorkWindow());
            if( !pWin )
                return;
            pWin->Restart(true);
        }
        break;
    }

    default:
        throw UnknownPropertyException();
    }

    if( bIllegalArgument )
        throw IllegalArgumentException();

    if( bValuesChanged )
        mpDoc->SetChanged( true );
}

// --------------------------------------------------------------------

Any SAL_CALL SlideShow::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    ThrowIfDisposed();

    const sd::PresentationSettings& rPresSettings = mpDoc->getPresentationSettings();

    const SfxItemPropertySimpleEntry* pEntry = maPropSet.getPropertyMapEntry(PropertyName);

    switch( pEntry ? pEntry->nWID : -1 )
    {
    case ATTR_PRESENT_ALL:
        return Any( (sal_Bool) ( !rPresSettings.mbCustomShow && rPresSettings.mbAll ) );
    case ATTR_PRESENT_CHANGE_PAGE:
        return Any( (sal_Bool) !rPresSettings.mbLockedPages );
    case ATTR_PRESENT_ANIMATION_ALLOWED:
        return Any( rPresSettings.mbAnimationAllowed );
    case ATTR_PRESENT_CUSTOMSHOW:
        {
            List* pList = mpDoc->GetCustomShowList(sal_False);
            SdCustomShow* pShow = (pList && rPresSettings.mbCustomShow)?(SdCustomShow*)pList->GetCurObject():NULL;
            OUString aShowName;

            if(pShow)
                aShowName = pShow->GetName();

            return Any( aShowName );
        }
    case ATTR_PRESENT_ENDLESS:
        return Any( rPresSettings.mbEndless );
    case ATTR_PRESENT_FULLSCREEN:
        return Any( rPresSettings.mbFullScreen );
    case ATTR_PRESENT_DIANAME:
        {
            OUString aSlideName;

            if( !rPresSettings.mbCustomShow && !rPresSettings.mbAll )
                aSlideName = getPageApiNameFromUiName( rPresSettings.maPresPage );

            return Any( aSlideName );
        }
    case ATTR_PRESENT_MANUEL:
        return Any( rPresSettings.mbManual );
    case ATTR_PRESENT_MOUSE:
        return Any( rPresSettings.mbMouseVisible );
    case ATTR_PRESENT_ALWAYS_ON_TOP:
        return Any( rPresSettings.mbAlwaysOnTop );
    case ATTR_PRESENT_NAVIGATOR:
        return Any( rPresSettings.mbStartWithNavigator );
    case ATTR_PRESENT_PEN:
        return Any( rPresSettings.mbMouseAsPen );
    case ATTR_PRESENT_PAUSE_TIMEOUT:
        return Any( rPresSettings.mnPauseTimeout );
    case ATTR_PRESENT_SHOW_PAUSELOGO:
        return Any( rPresSettings.mbShowPauseLogo );
    case ATTR_PRESENT_DISPLAY:
    {
        SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
        return Any( pOptions->GetDisplay() );
    }

    default:
        throw UnknownPropertyException();
    }
}

// --------------------------------------------------------------------

void SAL_CALL SlideShow::addPropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >&  ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

// --------------------------------------------------------------------

void SAL_CALL SlideShow::removePropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >&  ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

// --------------------------------------------------------------------

void SAL_CALL SlideShow::addVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

// --------------------------------------------------------------------

void SAL_CALL SlideShow::removeVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

// --------------------------------------------------------------------
// XPresentation
// --------------------------------------------------------------------

void SAL_CALL SlideShow::start() throw(RuntimeException)
{
    const Sequence< PropertyValue > aArguments;
    startWithArguments( aArguments );
}

// --------------------------------------------------------------------

WorkWindow *SlideShow::GetWorkWindow()
{
    if( !mpFullScreenViewShellBase )
        return NULL;

    PresentationViewShell* pShell = dynamic_cast<PresentationViewShell*>(mpFullScreenViewShellBase->GetMainViewShell().get());

    if( !pShell || !pShell->GetViewFrame() )
        return NULL;

    return dynamic_cast<WorkWindow*>(pShell->GetViewFrame()->GetTopFrame().GetWindow().GetParent());
}

void SAL_CALL SlideShow::end() throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    // The mbIsInStartup flag should have been reset during the start of the
    // slide show.  Reset it here just in case that something has horribly
    // gone wrong.
    OSL_ASSERT(!mbIsInStartup);
    mbIsInStartup = false;

    rtl::Reference< SlideshowImpl > xController( mxController );
    if( xController.is() )
    {
        mxController.clear();

        if( mpFullScreenFrameView )
        {
            delete mpFullScreenFrameView;
            mpFullScreenFrameView = 0;
        }

        ViewShellBase* pFullScreenViewShellBase = mpFullScreenViewShellBase;
        mpFullScreenViewShellBase = 0;

        // dispose before fullscreen window changes screens
        // (potentially). If this needs to be moved behind
        // pWorkWindow->StartPresentationMode() again, read issue
        // pWorkWindow->i94007 & implement the solution outlined
        // there.
        xController->dispose();

        if( pFullScreenViewShellBase )
        {
            PresentationViewShell* pShell = dynamic_cast<PresentationViewShell*>(pFullScreenViewShellBase->GetMainViewShell().get());

            if( pShell && pShell->GetViewFrame() )
            {
                WorkWindow* pWorkWindow = dynamic_cast<WorkWindow*>(pShell->GetViewFrame()->GetTopFrame().GetWindow().GetParent());
                if( pWorkWindow )
                {
                    pWorkWindow->StartPresentationMode( sal_False, isAlwaysOnTop() );
                }
            }
        }

        if( pFullScreenViewShellBase )
        {
            PresentationViewShell* pShell = NULL;
            {
                // Get the shell pointer in its own scope to be sure that
                // the shared_ptr to the shell is released before DoClose()
                // is called.
                ::boost::shared_ptr<ViewShell> pSharedView (pFullScreenViewShellBase->GetMainViewShell());
                pShell = dynamic_cast<PresentationViewShell*>(pSharedView.get());
            }
            if( pShell && pShell->GetViewFrame() )
                pShell->GetViewFrame()->DoClose();
        }
        else if( mpCurrentViewShellBase )
        {
            ViewShell* pViewShell = mpCurrentViewShellBase->GetMainViewShell().get();

            if( pViewShell )
            {
                FrameView* pFrameView = pViewShell->GetFrameView();

                if( pFrameView && (pFrameView->GetPresentationViewShellId() != SID_VIEWSHELL0) )
                {
                    ViewShell::ShellType ePreviousType (pFrameView->GetPreviousViewShellType());
                    pFrameView->SetPreviousViewShellType(ViewShell::ST_NONE);

                    pFrameView->SetPresentationViewShellId(SID_VIEWSHELL0);
                    pFrameView->SetSlotId(SID_OBJECT_SELECT);
                    pFrameView->SetPreviousViewShellType(pViewShell->GetShellType());

                    framework::FrameworkHelper::Instance(*mpCurrentViewShellBase)->RequestView(
                        framework::FrameworkHelper::GetViewURL(ePreviousType),
                        framework::FrameworkHelper::msCenterPaneURL);

                    pViewShell->GetViewFrame()->GetBindings().InvalidateAll( sal_True );
                }
            }
        }

        if( mpCurrentViewShellBase )
        {
            ViewShell* pViewShell = mpCurrentViewShellBase->GetMainViewShell().get();
            if( pViewShell )
            {
                // invalidate the view shell so the presentation slot will be re-enabled
                // and the rehersing will be updated
                pViewShell->Invalidate();

                if( xController->meAnimationMode ==ANIMATIONMODE_SHOW )
                {
                    // switch to the previously visible Slide
                    DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>( pViewShell );
                    if( pDrawViewShell )
                        pDrawViewShell->SwitchPage( (sal_uInt16)xController->getRestoreSlide() );
                    else
                    {
                        Reference<XDrawView> xDrawView (
                            Reference<XWeak>(&mpCurrentViewShellBase->GetDrawController()), UNO_QUERY);
                        if (xDrawView.is())
                            xDrawView->setCurrentPage(
                                Reference<XDrawPage>(
                                    mpDoc->GetSdPage(xController->getRestoreSlide(), PK_STANDARD)->getUnoPage(),
                                    UNO_QUERY));
                    }
                }

                if( pViewShell->GetDoc()->IsStartWithPresentation() )
                {
                    pViewShell->GetDoc()->SetStartWithPresentation( false );

                    Reference<frame::XDispatchProvider> xProvider(pViewShell->GetViewShellBase().GetController()->getFrame(),
                                                                  UNO_QUERY);
                    if( xProvider.is() )
                    {
                        util::URL aURL;
                        aURL.Complete = ".uno:CloseFrame";

                        uno::Reference< frame::XDispatch > xDispatch(
                            xProvider->queryDispatch(
                                aURL, ::rtl::OUString(), 0));
                        if( xDispatch.is() )
                        {
                            xDispatch->dispatch(aURL,
                                                uno::Sequence< beans::PropertyValue >());
                        }
                    }
                }
            }
        }
        mpCurrentViewShellBase = 0;
    }
}

// --------------------------------------------------------------------

void SAL_CALL SlideShow::rehearseTimings() throw(RuntimeException)
{
    Sequence< PropertyValue > aArguments(1);
    aArguments[0].Name = "RehearseTimings";
    aArguments[0].Value <<= sal_True;
    startWithArguments( aArguments );
}

// --------------------------------------------------------------------
// XPresentation2
// --------------------------------------------------------------------

void SAL_CALL SlideShow::startWithArguments( const Sequence< PropertyValue >& rArguments ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    ThrowIfDisposed();

    // Stop a running show before starting a new one.
    if( mxController.is() )
    {
        OSL_ASSERT(!mbIsInStartup);
        end();
    }
    else if (mbIsInStartup)
    {
        // We are already somewhere in process of starting a slide show but
        // have not yet got to the point where mxController is set.  There
        // is not yet a slide show to end so return silently.
        return;
    }

    // Prevent multiple instance of the SlideShow class for one document.
    mbIsInStartup = true;

    mxCurrentSettings.reset( new PresentationSettingsEx( mpDoc->getPresentationSettings() ) );
    mxCurrentSettings->SetArguments( rArguments );

    // if there is no view shell base set, use the current one or the first using this document
    if( mpCurrentViewShellBase == 0 )
    {
        // first check current
        ::sd::ViewShellBase* pBase = ::sd::ViewShellBase::GetViewShellBase( SfxViewFrame::Current() );
        if( pBase && pBase->GetDocument() == mpDoc )
        {
            mpCurrentViewShellBase = pBase;
        }
        else
        {
            // current is not ours, so get first from ours
            mpCurrentViewShellBase = ::sd::ViewShellBase::GetViewShellBase( SfxViewFrame::GetFirst( mpDoc->GetDocSh() ) );
        }
    }

    // #i118456# make sure TextEdit changes get pushed to model.
    // mpDrawView is tested against NULL above already.
    if(mpCurrentViewShellBase)
    {
        ViewShell* pViewShell = mpCurrentViewShellBase->GetMainViewShell().get();

        if(pViewShell && pViewShell->GetView())
        {
            pViewShell->GetView()->SdrEndTextEdit();
        }
    }

    // Start either a full-screen or an in-place show.
    if(mxCurrentSettings->mbFullScreen && !mxCurrentSettings->mbPreview)
        StartFullscreenPresentation();
    else
        StartInPlacePresentation();

}

// --------------------------------------------------------------------

::sal_Bool SAL_CALL SlideShow::isRunning(  ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    return mxController.is() && mxController->isRunning();
}

// --------------------------------------------------------------------

Reference< XSlideShowController > SAL_CALL SlideShow::getController(  ) throw (RuntimeException)
{
    ThrowIfDisposed();

    Reference< XSlideShowController > xController( mxController.get() );
    return xController;
}

// --------------------------------------------------------------------
// XComponent
// --------------------------------------------------------------------

void SAL_CALL SlideShow::disposing (void)
{
    SolarMutexGuard aGuard;

    if( mnInPlaceConfigEvent )
    {
        Application::RemoveUserEvent( mnInPlaceConfigEvent );
        mnInPlaceConfigEvent = 0;
    }

    if( mxController.is() )
    {
        mxController->dispose();
        mxController.clear();
    }

    mpCurrentViewShellBase = 0;
    mpFullScreenViewShellBase = 0;
    mpDoc = 0;
}

// ---------------------------------------------------------

bool SlideShow::startPreview( const Reference< XDrawPage >& xDrawPage, const Reference< XAnimationNode >& xAnimationNode, ::Window* pParent )
{
    Sequence< PropertyValue > aArguments(4);

    aArguments[0].Name = "Preview";
    aArguments[0].Value <<= sal_True;

    aArguments[1].Name = "FirstPage";
    aArguments[1].Value <<= xDrawPage;

    aArguments[2].Name = "AnimationNode";
    aArguments[2].Value <<= xAnimationNode;

    Reference< XWindow > xParentWindow;
    if( pParent )
        xParentWindow = VCLUnoHelper::GetInterface( pParent );

    aArguments[3].Name = "ParentWindow";
    aArguments[3].Value <<= xParentWindow;

    startWithArguments( aArguments );

    return true;
}

// ---------------------------------------------------------

ShowWindow* SlideShow::getShowWindow()
{
    return mxController.is() ? mxController->mpShowWindow : 0;
}

// ---------------------------------------------------------

int SlideShow::getAnimationMode()
{
    return mxController.is() ? mxController->meAnimationMode : ANIMATIONMODE_SHOW;
}

// ---------------------------------------------------------

void SlideShow::jumpToPageIndex( sal_Int32 nPageIndex )
{
    if( mxController.is() )
        mxController->displaySlideIndex( nPageIndex );
}

// ---------------------------------------------------------

void SlideShow::jumpToPageNumber( sal_Int32 nPageNumber )
{
    if( mxController.is() )
        mxController->displaySlideNumber( nPageNumber );
}

// ---------------------------------------------------------

sal_Int32 SlideShow::getCurrentPageNumber()
{
    return mxController.is() ? mxController->getCurrentSlideNumber() : 0;
}

// ---------------------------------------------------------

void SlideShow::jumpToBookmark( const OUString& sBookmark )
{
    if( mxController.is() )
        mxController->jumpToBookmark( sBookmark );
}

// ---------------------------------------------------------

bool SlideShow::isFullScreen()
{
    return mxController.is() ? mxController->maPresSettings.mbFullScreen : false;
}

// ---------------------------------------------------------

void SlideShow::resize( const Size &rSize )
{
    if( mxController.is() )
        mxController->resize( rSize );
}

// ---------------------------------------------------------

void SlideShow::activate( ViewShellBase& rBase )
{
    if( (mpFullScreenViewShellBase == &rBase) && !mxController.is() )
    {
        ::boost::shared_ptr<PresentationViewShell> pShell = ::boost::dynamic_pointer_cast<PresentationViewShell>(rBase.GetMainViewShell());
        if(pShell.get() != NULL)
        {
            pShell->FinishInitialization( mpFullScreenFrameView );
            mpFullScreenFrameView = 0;

            CreateController( pShell.get(), pShell->GetView(), rBase.GetViewWindow() );

            if( mxController->startShow(mxCurrentSettings.get()) )
            {
                pShell->Resize();
            }
            else
            {
                end();
                return;
            }
        }
    }

    if( mxController.is() )
        mxController->activate();

}

// ---------------------------------------------------------

void SlideShow::deactivate( ViewShellBase& /*rBase*/ )
{
    mxController->deactivate();
}

// ---------------------------------------------------------

bool SlideShow::keyInput(const KeyEvent& rKEvt)
{
    return mxController.is() ? mxController->keyInput(rKEvt) : false;
}

// ---------------------------------------------------------

void SlideShow::paint( const Rectangle& rRect )
{
    if( mxController.is() )
        mxController->paint( rRect );
}

// ---------------------------------------------------------

bool SlideShow::isAlwaysOnTop()
{
    return mxController.is() ? mxController->maPresSettings.mbAlwaysOnTop : false;
}

// ---------------------------------------------------------

bool SlideShow::pause( bool bPause )
{
    if( mxController.is() )
    {
        if( bPause )
            mxController->pause();
        else
            mxController->resume();
    }
    return true;
}

// ---------------------------------------------------------

void SlideShow::receiveRequest(SfxRequest& rReq)
{
    if( mxController.is() )
        mxController->receiveRequest( rReq );
}

// ---------------------------------------------------------

sal_Int32 SlideShow::getFirstPageNumber()
{
    return mxController.is() ? mxController->getFirstSlideNumber() : 0;
}

// ---------------------------------------------------------

sal_Int32 SlideShow::getLastPageNumber()
{
    return mxController.is() ? mxController->getLastSlideNumber() : 0;
}

// ---------------------------------------------------------

bool SlideShow::isEndless()
{
    return mxController.is() ? mxController->isEndless() : false;
}

// ---------------------------------------------------------

bool SlideShow::isDrawingPossible()
{
    return mxController.is() ? mxController->getUsePen() : false;
}

// ---------------------------------------------------------

void SlideShow::StartInPlacePresentationConfigurationCallback()
{
    if( mnInPlaceConfigEvent != 0 )
        Application::RemoveUserEvent( mnInPlaceConfigEvent );

    mnInPlaceConfigEvent = Application::PostUserEvent( LINK( this, SlideShow, StartInPlacePresentationConfigurationHdl ) );
}

// ---------------------------------------------------------

IMPL_LINK_NOARG(SlideShow, StartInPlacePresentationConfigurationHdl)
{
    mnInPlaceConfigEvent = 0;
    StartInPlacePresentation();
    return 0;
}

// ---------------------------------------------------------

void SlideShow::StartInPlacePresentation()
{
    if( mpCurrentViewShellBase )
    {
        // Save the current view shell type so that it can be restored after the
        // show has ended.  If there already is a saved shell type then that is
        // not overwritten.

        ViewShell::ShellType eShell = ViewShell::ST_NONE;

        ::boost::shared_ptr<FrameworkHelper> pHelper(FrameworkHelper::Instance(*mpCurrentViewShellBase));
        ::boost::shared_ptr<ViewShell> pMainViewShell(pHelper->GetViewShell(FrameworkHelper::msCenterPaneURL));

        if( pMainViewShell.get() )
            eShell = pMainViewShell->GetShellType();

        if( eShell != ViewShell::ST_IMPRESS )
        {
            // Switch temporary to a DrawViewShell which supports the in-place presentation.

            if( pMainViewShell.get() )
            {
                FrameView* pFrameView = pMainViewShell->GetFrameView();
                pFrameView->SetPresentationViewShellId(SID_VIEWSHELL1);
                pFrameView->SetPreviousViewShellType (pMainViewShell->GetShellType());
                pFrameView->SetPageKind (PK_STANDARD);
            }

            pHelper->RequestView( FrameworkHelper::msImpressViewURL, FrameworkHelper::msCenterPaneURL );
            pHelper->RunOnConfigurationEvent( FrameworkHelper::msConfigurationUpdateEndEvent, ::boost::bind(&SlideShow::StartInPlacePresentationConfigurationCallback, this) );
            return;
        }
        else
        {
            ::Window* pParentWindow = mxCurrentSettings->mpParentWindow;
            if( pParentWindow == 0 )
                pParentWindow = mpCurrentViewShellBase->GetViewWindow();

            CreateController( pMainViewShell.get(), pMainViewShell->GetView(), pParentWindow );
        }
    }
    else if( mxCurrentSettings->mpParentWindow )
    {
        // no current view shell, but parent window
        CreateController( 0, 0, mxCurrentSettings->mpParentWindow );
    }

    if( mxController.is() )
    {
        sal_Bool bSuccess = sal_False;
        if( mxCurrentSettings.get() && mxCurrentSettings->mbPreview )
        {
            bSuccess = mxController->startPreview(mxCurrentSettings->mxStartPage, mxCurrentSettings->mxAnimationNode, mxCurrentSettings->mpParentWindow );
        }
        else
        {
            bSuccess = mxController->startShow(mxCurrentSettings.get());
        }

        if( !bSuccess )
            end();
    }
}

// ---------------------------------------------------------

void SlideShow::StartFullscreenPresentation( )
{
    // Create the top level window in which the PresentationViewShell(Base)
    // will be created.  This is done here explicitly so that we can make it
    // fullscreen.
    const sal_Int32 nDisplay (GetDisplay());
    WorkWindow* pWorkWindow = new FullScreenWorkWindow(this, mpCurrentViewShellBase);
    pWorkWindow->SetBackground(Wallpaper(COL_BLACK));
    pWorkWindow->StartPresentationMode( sal_True, mpDoc->getPresentationSettings().mbAlwaysOnTop ? PRESENTATION_HIDEALLAPPS : 0, nDisplay);
    //    pWorkWindow->ShowFullScreenMode(sal_False, nDisplay);

    if (pWorkWindow->IsVisible())
    {
        // Initialize the new presentation view shell with a copy of the
        // frame view of the current view shell.  This avoids that
        // changes made by the presentation have an effect on the other
        // view shells.
        FrameView* pOriginalFrameView = mpCurrentViewShellBase ? mpCurrentViewShellBase->GetMainViewShell()->GetFrameView() : 0;

        if( mpFullScreenFrameView )
            delete mpFullScreenFrameView;
        mpFullScreenFrameView = new FrameView(mpDoc, pOriginalFrameView);

        // The new frame is created hidden.  To make it visible and activate the
        // new view shell--a prerequisite to process slot calls and initialize
        // its panes--a GrabFocus() has to be called later on.
        SfxFrame* pNewFrame = SfxFrame::Create( *mpDoc->GetDocSh(), *pWorkWindow, PRESENTATION_FACTORY_ID, true );
        pNewFrame->SetPresentationMode(sal_True);

        mpFullScreenViewShellBase = static_cast<ViewShellBase*>(pNewFrame->GetCurrentViewFrame()->GetViewShell());
        if(mpFullScreenViewShellBase != NULL)
        {
            // The following GrabFocus() is responsible for activating the
            // new view shell.  Without it the screen remains blank (under
            // Windows and some Linux variants.)
            mpFullScreenViewShellBase->GetWindow()->GrabFocus();
        }
    }
}

// ---------------------------------------------------------

sal_Int32 SlideShow::GetDisplay()

{
    sal_Int32 nDisplay = 0;

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    if( pOptions )
        nDisplay = pOptions->GetDisplay();

    if (nDisplay <= 0 )
    {
        try
        {
            Reference<XMultiServiceFactory > xFactory(
                ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW);
            Reference<XPropertySet> xMonitorProperties(
                xFactory->createInstance( "com.sun.star.awt.DisplayAccess" ),
                UNO_QUERY_THROW);
            xMonitorProperties->getPropertyValue("ExternalDisplay") >>= nDisplay;
        }
        catch( Exception& )
        {
        }
    }
    else
    {
        nDisplay--;
    }

    return nDisplay;
}

// ---------------------------------------------------------


bool SlideShow::dependsOn( ViewShellBase* pViewShellBase )
{
    return mxController.is() && (pViewShellBase == mpCurrentViewShellBase) && mpFullScreenViewShellBase;
}

// ---------------------------------------------------------

Reference< XPresentation2 > CreatePresentation( const SdDrawDocument& rDocument )
{
    return Reference< XPresentation2 >( SlideShow::Create( const_cast< SdDrawDocument* >( &rDocument ) ).get() );
}

// ---------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
