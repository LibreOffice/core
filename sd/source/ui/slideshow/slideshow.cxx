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

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svx/svdpool.hxx>
#include <svl/itemprop.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>

#include <svx/unoprov.hxx>

#include <framework/FrameworkHelper.hxx>

#include <FrameView.hxx>
#include <createpresentation.hxx>
#include <unomodel.hxx>
#include <slideshow.hxx>
#include "slideshowimpl.hxx"
#include <sdattr.hrc>
#include <sdmod.hxx>
#include <FactoryIds.hxx>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include "SlideShowRestarter.hxx"
#include <DrawController.hxx>
#include <PresentationViewShell.hxx>
#include <customshowlist.hxx>
#include <unopage.hxx>
#include <sdpage.hxx>
#include <cusshow.hxx>
#include <optsitem.hxx>
#include <strings.hrc>
#include <sdresid.hxx>

using ::com::sun::star::presentation::XSlideShowController;
using ::sd::framework::FrameworkHelper;
using ::com::sun::star::awt::XWindow;
using namespace ::sd;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::drawing::framework;

namespace {
    /** This local version of the work window overrides DataChanged() so that it
        can restart the slide show when a display is added or removed.
    */
    class FullScreenWorkWindow : public WorkWindow
    {
    public:
        FullScreenWorkWindow (
            const ::rtl::Reference<SlideShow>& rpSlideShow,
            ViewShellBase* pViewShellBase)
            : WorkWindow(nullptr, WB_HIDE | WB_CLIPCHILDREN),
              mpRestarter(new SlideShowRestarter(rpSlideShow, pViewShellBase))
        {}

        void Restart(bool bForce)
        {
            mpRestarter->Restart(bForce);
        }

        virtual void DataChanged (const DataChangedEvent& rEvent) override
        {
            if (rEvent.GetType() == DataChangedEventType::DISPLAY)
                Restart(false);
        }

    private:
        ::std::shared_ptr<SlideShowRestarter> mpRestarter;
    };
}

static const SfxItemPropertyMapEntry* ImplGetPresentationPropertyMap()
{
    // NOTE: First member must be sorted
    static const SfxItemPropertyMapEntry aPresentationPropertyMap_Impl[] =
    {
        { OUString("AllowAnimations"),          ATTR_PRESENT_ANIMATION_ALLOWED, cppu::UnoType<bool>::get(),                0, 0 },
        { OUString("CustomShow"),               ATTR_PRESENT_CUSTOMSHOW,        ::cppu::UnoType<OUString>::get(),     0, 0 },
        { OUString("Display"),                  ATTR_PRESENT_DISPLAY,           ::cppu::UnoType<sal_Int32>::get(),    0, 0 },
        { OUString("FirstPage"),                ATTR_PRESENT_DIANAME,           ::cppu::UnoType<OUString>::get(),     0, 0 },
        { OUString("IsAlwaysOnTop"),            ATTR_PRESENT_ALWAYS_ON_TOP,     cppu::UnoType<bool>::get(),                0, 0 },
        { OUString("IsAutomatic"),              ATTR_PRESENT_MANUEL,            cppu::UnoType<bool>::get(),                0, 0 },
        { OUString("IsEndless"),                ATTR_PRESENT_ENDLESS,           cppu::UnoType<bool>::get(),                0, 0 },
        { OUString("IsFullScreen"),             ATTR_PRESENT_FULLSCREEN,        cppu::UnoType<bool>::get(),                0, 0 },
        { OUString("IsShowAll"),                ATTR_PRESENT_ALL,               cppu::UnoType<bool>::get(),                0, 0 },
        { OUString("IsMouseVisible"),           ATTR_PRESENT_MOUSE,             cppu::UnoType<bool>::get(),                0, 0 },
        { OUString("IsShowLogo"),               ATTR_PRESENT_SHOW_PAUSELOGO,    cppu::UnoType<bool>::get(),                0, 0 },
        { OUString("IsTransitionOnClick"),      ATTR_PRESENT_CHANGE_PAGE,       cppu::UnoType<bool>::get(),                0, 0 },
        { OUString("Pause"),                    ATTR_PRESENT_PAUSE_TIMEOUT,     ::cppu::UnoType<sal_Int32>::get(),    0, 0 },
        { OUString("StartWithNavigator"),       ATTR_PRESENT_NAVIGATOR,         cppu::UnoType<bool>::get(),                0, 0 },
        { OUString("UsePen"),                   ATTR_PRESENT_PEN,               cppu::UnoType<bool>::get(),                0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aPresentationPropertyMap_Impl;
}

// class SlideShow

SlideShow::SlideShow( SdDrawDocument* pDoc )
: SlideshowBase( m_aMutex )
, maPropSet(ImplGetPresentationPropertyMap(), SdrObject::GetGlobalDrawObjectItemPool())
, mbIsInStartup(false)
, mpDoc( pDoc )
, mpCurrentViewShellBase( nullptr )
, mpFullScreenViewShellBase( nullptr )
, mpFullScreenFrameView( nullptr )
, mnInPlaceConfigEvent( nullptr )
{
}

void SlideShow::ThrowIfDisposed() const
{
    if( mpDoc == nullptr )
        throw DisposedException();
}

/// used by the model to create a slideshow for it
rtl::Reference< SlideShow > SlideShow::Create( SdDrawDocument* pDoc )
{
    return new SlideShow( pDoc );
}

rtl::Reference< SlideShow > SlideShow::GetSlideShow( SdDrawDocument const * pDocument )
{
    rtl::Reference< SlideShow > xRet;

    if( pDocument )
        xRet = GetSlideShow( *pDocument );

    return xRet;
}

rtl::Reference< SlideShow > SlideShow::GetSlideShow( SdDrawDocument const & rDocument )
{
    return rtl::Reference< SlideShow >(
        dynamic_cast< SlideShow* >( rDocument.getPresentation().get() ) );
}

rtl::Reference< SlideShow > SlideShow::GetSlideShow( ViewShellBase const & rBase )
{
    return GetSlideShow( rBase.GetDocument() );
}

css::uno::Reference< css::presentation::XSlideShowController > SlideShow::GetSlideShowController(ViewShellBase const & rBase )
{
    rtl::Reference< SlideShow > xSlideShow( GetSlideShow( rBase ) );

    Reference< XSlideShowController > xRet;
    if( xSlideShow.is() )
        xRet = xSlideShow->getController();

    return xRet;
}

bool SlideShow::StartPreview( ViewShellBase const & rBase,
    const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage,
    const css::uno::Reference< css::animations::XAnimationNode >& xAnimationNode )
{
    rtl::Reference< SlideShow > xSlideShow( GetSlideShow( rBase ) );
    if( !xSlideShow.is() )
        return false;

    xSlideShow->startPreview( xDrawPage, xAnimationNode );
    return true;
}

void SlideShow::Stop( ViewShellBase const & rBase )
{
    rtl::Reference< SlideShow > xSlideShow( GetSlideShow( rBase ) );
    if( xSlideShow.is() )
        xSlideShow->end();
}

bool SlideShow::IsRunning( ViewShellBase const & rBase )
{
    rtl::Reference< SlideShow > xSlideShow( GetSlideShow( rBase ) );
    return xSlideShow.is() && xSlideShow->isRunning();
}

bool SlideShow::IsRunning( ViewShell& rViewShell )
{
    rtl::Reference< SlideShow > xSlideShow( GetSlideShow( rViewShell.GetViewShellBase() ) );
    return xSlideShow.is() && xSlideShow->isRunning() && (xSlideShow->mxController->getViewShell() == &rViewShell);
}

void SlideShow::CreateController(  ViewShell* pViewSh, ::sd::View* pView, vcl::Window* pParentWindow )
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

// XServiceInfo
OUString SAL_CALL SlideShow::getImplementationName(  )
{
    return OUString( "com.sun.star.comp.sd.SlideShow" );
}

sal_Bool SAL_CALL SlideShow::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL SlideShow::getSupportedServiceNames(  )
{
    OUString aService( "com.sun.star.presentation.Presentation" );
    Sequence< OUString > aSeq( &aService, 1 );
    return aSeq;
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL SlideShow::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static Reference< XPropertySetInfo > xInfo = maPropSet.getPropertySetInfo();
    return xInfo;
 }

void SAL_CALL SlideShow::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
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
        bool bVal = false;

        if( aValue >>= bVal )
        {
            bIllegalArgument = false;

            if( rPresSettings.mbAll != bVal )
            {
                rPresSettings.mbAll = bVal;
                bValuesChanged = true;
                if( bVal )
                    rPresSettings.mbCustomShow = false;
            }
        }
        break;
    }
    case ATTR_PRESENT_CHANGE_PAGE:
    {
        bool bVal = false;

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
        bool bVal = false;

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
        OUString aShowName;
        if( aValue >>= aShowName )
        {
            bIllegalArgument = false;

            SdCustomShowList* pCustomShowList = mpDoc->GetCustomShowList();
            if(pCustomShowList)
            {
                SdCustomShow* pCustomShow;
                for( pCustomShow = pCustomShowList->First(); pCustomShow != nullptr; pCustomShow = pCustomShowList->Next() )
                {
                    if( pCustomShow->GetName() == aShowName )
                        break;
                }

                rPresSettings.mbCustomShow = true;
                bValuesChanged = true;
            }
        }
        break;
    }
    case ATTR_PRESENT_ENDLESS:
    {
        bool bVal = false;

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
        bool bVal = false;

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
            rPresSettings.mbCustomShow = false;
            rPresSettings.mbAll = false;
        }
        break;
    }
    case ATTR_PRESENT_MANUEL:
    {
        bool bVal = false;

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
        bool bVal = false;

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
        bool bVal = false;

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
        bIllegalArgument = false;
        //ignored, but exists in some older documents
        break;
    case ATTR_PRESENT_PEN:
    {
        bool bVal = false;

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
        bool bVal = false;

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

            SdOptions* pOptions = SD_MOD()->GetSdOptions(DocumentType::Impress);
            pOptions->SetDisplay( nDisplay );

            FullScreenWorkWindow *pWin = dynamic_cast<FullScreenWorkWindow *>(GetWorkWindow());
            if( !pWin )
                return;
            pWin->Restart(true);
        }
        break;
    }

    default:
        throw UnknownPropertyException( OUString::number(pEntry ? pEntry->nWID : -1), static_cast<cppu::OWeakObject*>(this));
    }

    if( bIllegalArgument )
        throw IllegalArgumentException();

    if( bValuesChanged )
        mpDoc->SetChanged();
}

Any SAL_CALL SlideShow::getPropertyValue( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;
    ThrowIfDisposed();

    const sd::PresentationSettings& rPresSettings = mpDoc->getPresentationSettings();

    const SfxItemPropertySimpleEntry* pEntry = maPropSet.getPropertyMapEntry(PropertyName);

    switch( pEntry ? pEntry->nWID : -1 )
    {
    case ATTR_PRESENT_ALL:
        return Any( !rPresSettings.mbCustomShow && rPresSettings.mbAll );
    case ATTR_PRESENT_CHANGE_PAGE:
        return Any( !rPresSettings.mbLockedPages );
    case ATTR_PRESENT_ANIMATION_ALLOWED:
        return Any( rPresSettings.mbAnimationAllowed );
    case ATTR_PRESENT_CUSTOMSHOW:
        {
            SdCustomShowList* pList = mpDoc->GetCustomShowList();
            SdCustomShow* pShow = (pList && rPresSettings.mbCustomShow) ? pList->GetCurObject() : nullptr;
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
        return Any( false );
    case ATTR_PRESENT_PEN:
        return Any( rPresSettings.mbMouseAsPen );
    case ATTR_PRESENT_PAUSE_TIMEOUT:
        return Any( rPresSettings.mnPauseTimeout );
    case ATTR_PRESENT_SHOW_PAUSELOGO:
        return Any( rPresSettings.mbShowPauseLogo );
    case ATTR_PRESENT_DISPLAY:
    {
        SdOptions* pOptions = SD_MOD()->GetSdOptions(DocumentType::Impress);
        return Any(pOptions->GetDisplay());
    }

    default:
        throw UnknownPropertyException( OUString::number(pEntry ? pEntry->nWID : -1), static_cast<cppu::OWeakObject*>(this));
    }
}

void SAL_CALL SlideShow::addPropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >&  )
{
}

void SAL_CALL SlideShow::removePropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >&  )
{
}

void SAL_CALL SlideShow::addVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  )
{
}

void SAL_CALL SlideShow::removeVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  )
{
}

// XPresentation

void SAL_CALL SlideShow::start()
{
    const Sequence< PropertyValue > aArguments;
    startWithArguments( aArguments );
}

WorkWindow *SlideShow::GetWorkWindow()
{
    if( !mpFullScreenViewShellBase )
        return nullptr;

    PresentationViewShell* pShell = dynamic_cast<PresentationViewShell*>(mpFullScreenViewShellBase->GetMainViewShell().get());

    if( !pShell || !pShell->GetViewFrame() )
        return nullptr;

    return dynamic_cast<WorkWindow*>(pShell->GetViewFrame()->GetFrame().GetWindow().GetParent());
}

bool SlideShow::IsExitAfterPresenting() const
{
    SolarMutexGuard aGuard;
    ThrowIfDisposed();
    return mpDoc->IsExitAfterPresenting();
}

void SlideShow::SetExitAfterPresenting(bool bExit)
{
    SolarMutexGuard aGuard;
    ThrowIfDisposed();
    mpDoc->SetExitAfterPresenting(bExit);
}

void SAL_CALL SlideShow::end()
{
    SolarMutexGuard aGuard;

    // The mbIsInStartup flag should have been reset during the start of the
    // slide show.  Reset it here just in case that something has horribly
    // gone wrong.
    OSL_ASSERT(!mbIsInStartup);
    mbIsInStartup = false;

    rtl::Reference< SlideshowImpl > xController( mxController );
    if( !xController.is() )
        return;

    mxController.clear();

    if( mpFullScreenFrameView )
    {
        delete mpFullScreenFrameView;
        mpFullScreenFrameView = nullptr;
    }

    ViewShellBase* pFullScreenViewShellBase = mpFullScreenViewShellBase;
    mpFullScreenViewShellBase = nullptr;

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
            WorkWindow* pWorkWindow = dynamic_cast<WorkWindow*>(pShell->GetViewFrame()->GetFrame().GetWindow().GetParent());
            if( pWorkWindow )
            {
                pWorkWindow->StartPresentationMode(   (mxController.is() && mxController->maPresSettings.mbAlwaysOnTop)
                                                    ? PresentationFlags::HideAllApps : PresentationFlags::NONE );
            }
        }
    }

    if( pFullScreenViewShellBase )
    {
        PresentationViewShell* pShell = nullptr;
        {
            // Get the shell pointer in its own scope to be sure that
            // the shared_ptr to the shell is released before DoClose()
            // is called.
            ::std::shared_ptr<ViewShell> pSharedView (pFullScreenViewShellBase->GetMainViewShell());
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
                pFrameView->SetPreviousViewShellType(pViewShell->GetShellType());

                framework::FrameworkHelper::Instance(*mpCurrentViewShellBase)->RequestView(
                    framework::FrameworkHelper::GetViewURL(ePreviousType),
                    framework::FrameworkHelper::msCenterPaneURL);

                pViewShell->GetViewFrame()->GetBindings().InvalidateAll( true );
            }
        }
    }

    if( mpCurrentViewShellBase )
    {
        ViewShell* pViewShell = mpCurrentViewShellBase->GetMainViewShell().get();
        if( pViewShell )
        {
            // invalidate the view shell so the presentation slot will be re-enabled
            // and the rehearsing will be updated
            pViewShell->Invalidate();

            if( xController->meAnimationMode ==ANIMATIONMODE_SHOW )
            {
                // switch to the previously visible Slide
                DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>( pViewShell );
                if( pDrawViewShell )
                    pDrawViewShell->SwitchPage( static_cast<sal_uInt16>(xController->getRestoreSlide()) );
                else
                {
                    Reference<XDrawView> xDrawView (
                        Reference<XWeak>(&mpCurrentViewShellBase->GetDrawController()), UNO_QUERY);
                    if (xDrawView.is())
                        xDrawView->setCurrentPage(
                            Reference<XDrawPage>(
                                mpDoc->GetSdPage(xController->getRestoreSlide(), PageKind::Standard)->getUnoPage(),
                                UNO_QUERY));
                }
            }

            if( pViewShell->GetDoc()->IsExitAfterPresenting() )
            {
                pViewShell->GetDoc()->SetExitAfterPresenting( false );

                Reference<frame::XDispatchProvider> xProvider(pViewShell->GetViewShellBase().GetController()->getFrame(),
                                                              UNO_QUERY);
                if( xProvider.is() )
                {
                    util::URL aURL;
                    aURL.Complete = ".uno:CloseFrame";

                    uno::Reference< frame::XDispatch > xDispatch(
                        xProvider->queryDispatch(
                            aURL, OUString(), 0));
                    if( xDispatch.is() )
                    {
                        xDispatch->dispatch(aURL,
                                            uno::Sequence< beans::PropertyValue >());
                    }
                }
            }
        }
        //Fire the acc focus event when focus is switched back. The above method mpCurrentViewShellBase->GetWindow()->GrabFocus() will
        //set focus to WorkWindow instead of the sd::window, so here call Shell's method to fire the focus event
        if (pViewShell)
            pViewShell->SwitchActiveViewFireFocus();
    }
    mpCurrentViewShellBase = nullptr;
}

void SAL_CALL SlideShow::rehearseTimings()
{
    Sequence< PropertyValue > aArguments(1);
    aArguments[0].Name = "RehearseTimings";
    aArguments[0].Value <<= true;
    startWithArguments( aArguments );
}

// XPresentation2

void SAL_CALL SlideShow::startWithArguments(const Sequence< PropertyValue >& rArguments)
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
    if( mpCurrentViewShellBase == nullptr )
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

sal_Bool SAL_CALL SlideShow::isRunning(  )
{
    SolarMutexGuard aGuard;
    return mxController.is() && mxController->isRunning();
}

Reference< XSlideShowController > SAL_CALL SlideShow::getController(  )
{
    ThrowIfDisposed();

    Reference< XSlideShowController > xController( mxController.get() );
    return xController;
}

// XComponent

void SAL_CALL SlideShow::disposing()
{
    SolarMutexGuard aGuard;

    if( mnInPlaceConfigEvent )
    {
        Application::RemoveUserEvent( mnInPlaceConfigEvent );
        mnInPlaceConfigEvent = nullptr;
    }

    if( mxController.is() )
    {
        mxController->dispose();
        mxController.clear();
    }

    mpCurrentViewShellBase = nullptr;
    mpFullScreenViewShellBase = nullptr;
    mpDoc = nullptr;
}

void SlideShow::startPreview( const Reference< XDrawPage >& xDrawPage, const Reference< XAnimationNode >& xAnimationNode )
{
    Sequence< PropertyValue > aArguments(4);

    aArguments[0].Name = "Preview";
    aArguments[0].Value <<= true;

    aArguments[1].Name = "FirstPage";
    aArguments[1].Value <<= xDrawPage;

    aArguments[2].Name = "AnimationNode";
    aArguments[2].Value <<= xAnimationNode;

    aArguments[3].Name = "ParentWindow";
    aArguments[3].Value <<= Reference< XWindow >();

    startWithArguments( aArguments );
}

OutputDevice* SlideShow::getShowWindow()
{
    return mxController.is() ? mxController->mpShowWindow.get() : nullptr;
}

int SlideShow::getAnimationMode()
{
    return mxController.is() ? mxController->meAnimationMode : ANIMATIONMODE_SHOW;
}

void SlideShow::jumpToPageIndex( sal_Int32 nPageIndex )
{
    if( mxController.is() )
        mxController->displaySlideIndex( nPageIndex );
}

void SlideShow::jumpToPageNumber( sal_Int32 nPageNumber )
{
    if( mxController.is() )
        mxController->displaySlideNumber( nPageNumber );
}

sal_Int32 SlideShow::getCurrentPageNumber()
{
    return mxController.is() ? mxController->getCurrentSlideNumber() : 0;
}

void SlideShow::jumpToBookmark( const OUString& sBookmark )
{
    if( mxController.is() )
        mxController->jumpToBookmark( sBookmark );
}

bool SlideShow::isFullScreen()
{
    return mxController.is() && mxController->maPresSettings.mbFullScreen;
}

void SlideShow::resize( const Size &rSize )
{
    if( mxController.is() )
        mxController->resize( rSize );
}

void SlideShow::activate( ViewShellBase& rBase )
{
    if( (mpFullScreenViewShellBase == &rBase) && !mxController.is() )
    {
        ::std::shared_ptr<PresentationViewShell> pShell = std::dynamic_pointer_cast<PresentationViewShell>(rBase.GetMainViewShell());
        if (pShell != nullptr)
        {
            pShell->FinishInitialization( mpFullScreenFrameView );
            mpFullScreenFrameView = nullptr;

            CreateController( pShell.get(), pShell->GetView(), rBase.GetViewWindow() );

            if( mxController->startShow(mxCurrentSettings.get()) )
            {
                pShell->Resize();
                // Defer the sd::ShowWindow's GrabFocus to here. so that the accessible event can be fired correctly.
                pShell->GetActiveWindow()->GrabFocus();
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

void SlideShow::deactivate()
{
    mxController->deactivate();
}

bool SlideShow::keyInput(const KeyEvent& rKEvt)
{
    return mxController.is() && mxController->keyInput(rKEvt);
}

void SlideShow::paint()
{
    if( mxController.is() )
        mxController->paint();
}

void SlideShow::pause( bool bPause )
{
    if( mxController.is() )
    {
        if( bPause )
            mxController->pause();
        else
            mxController->resume();
    }
}

bool SlideShow::swipe(const CommandSwipeData& rSwipeData)
{
    return mxController.is() && mxController->swipe(rSwipeData);
}

bool SlideShow::longpress(const CommandLongPressData& rLongPressData)
{
    return mxController.is() && mxController->longpress(rLongPressData);
}

void SlideShow::StartInPlacePresentationConfigurationCallback()
{
    if( mnInPlaceConfigEvent != nullptr )
        Application::RemoveUserEvent( mnInPlaceConfigEvent );

    mnInPlaceConfigEvent = Application::PostUserEvent( LINK( this, SlideShow, StartInPlacePresentationConfigurationHdl ) );
}

IMPL_LINK_NOARG(SlideShow, StartInPlacePresentationConfigurationHdl, void*, void)
{
    mnInPlaceConfigEvent = nullptr;
    StartInPlacePresentation();
}

void SlideShow::StartInPlacePresentation()
{
    if( mpCurrentViewShellBase )
    {
        // Save the current view shell type so that it can be restored after the
        // show has ended.  If there already is a saved shell type then that is
        // not overwritten.

        ViewShell::ShellType eShell = ViewShell::ST_NONE;

        ::std::shared_ptr<FrameworkHelper> pHelper(FrameworkHelper::Instance(*mpCurrentViewShellBase));
        ::std::shared_ptr<ViewShell> pMainViewShell(pHelper->GetViewShell(FrameworkHelper::msCenterPaneURL));

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
                pFrameView->SetPageKind (PageKind::Standard);
            }

            pHelper->RequestView( FrameworkHelper::msImpressViewURL, FrameworkHelper::msCenterPaneURL );
            pHelper->RunOnConfigurationEvent(
                FrameworkHelper::msConfigurationUpdateEndEvent,
                [this] (bool const) { return this->StartInPlacePresentationConfigurationCallback(); } );
            return;
        }
        else
        {
            vcl::Window* pParentWindow = mxCurrentSettings->mpParentWindow;
            if( pParentWindow == nullptr )
                pParentWindow = mpCurrentViewShellBase->GetViewWindow();

            CreateController( pMainViewShell.get(), pMainViewShell->GetView(), pParentWindow );
        }
    }
    else if( mxCurrentSettings->mpParentWindow )
    {
        // no current view shell, but parent window
        CreateController( nullptr, nullptr, mxCurrentSettings->mpParentWindow );
    }

    if( !mxController.is() )
        return;

    bool bSuccess = false;
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
    else
    {
        if( mpCurrentViewShellBase && ( !mxCurrentSettings.get() || ( mxCurrentSettings.get() && !mxCurrentSettings->mbPreview ) ) )
            mpCurrentViewShellBase->GetWindow()->GrabFocus();
    }
}

void SlideShow::StartFullscreenPresentation( )
{
    // Create the top level window in which the PresentationViewShell(Base)
    // will be created.  This is done here explicitly so that we can make it
    // fullscreen.
    const sal_Int32 nDisplay (GetDisplay());
    VclPtr<WorkWindow> pWorkWindow = VclPtr<FullScreenWorkWindow>::Create(this, mpCurrentViewShellBase);
    pWorkWindow->SetBackground(Wallpaper(COL_BLACK));
    OUString Title(SdResId(STR_FULLSCREEN_SLIDESHOW));
    Title = Title.replaceFirst("%s",
                               mpCurrentViewShellBase->GetDocShell()->GetTitle(SFX_TITLE_DETECT));
    pWorkWindow->SetText(Title);
    pWorkWindow->StartPresentationMode( true, mpDoc->getPresentationSettings().mbAlwaysOnTop ? PresentationFlags::HideAllApps : PresentationFlags::NONE, nDisplay);
    //    pWorkWindow->ShowFullScreenMode(sal_False, nDisplay);

    if (!pWorkWindow->IsVisible())
        return;

    // Initialize the new presentation view shell with a copy of the
    // frame view of the current view shell.  This avoids that
    // changes made by the presentation have an effect on the other
    // view shells.
    FrameView* pOriginalFrameView = nullptr;
    ::std::shared_ptr<ViewShell> xShell(mpCurrentViewShellBase->GetMainViewShell());
    if (xShell.get())
        pOriginalFrameView = xShell->GetFrameView();

    delete mpFullScreenFrameView;
    mpFullScreenFrameView = new FrameView(mpDoc, pOriginalFrameView);

    // The new frame is created hidden.  To make it visible and activate the
    // new view shell--a prerequisite to process slot calls and initialize
    // its panes--a GrabFocus() has to be called later on.
    SfxFrame* pNewFrame = SfxFrame::CreateHidden( *mpDoc->GetDocSh(), *pWorkWindow, PRESENTATION_FACTORY_ID );
    pNewFrame->SetPresentationMode(true);

    mpFullScreenViewShellBase = static_cast<ViewShellBase*>(pNewFrame->GetCurrentViewFrame()->GetViewShell());
    if(mpFullScreenViewShellBase != nullptr)
    {
        // The following GrabFocus() is responsible for activating the
        // new view shell.  Without it the screen remains blank (under
        // Windows and some Linux variants.)
        mpFullScreenViewShellBase->GetWindow()->GrabFocus();
    }
}

/// convert configuration setting display concept to real screens
sal_Int32 SlideShow::GetDisplay()
{
    sal_Int32 nDisplay = 0;

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DocumentType::Impress);
    if( pOptions )
        nDisplay = pOptions->GetDisplay();

    if( nDisplay < 0 )
        nDisplay = -1;
    else if( nDisplay == 0)
        nDisplay = static_cast<sal_Int32>(Application::GetDisplayExternalScreen());
    else
        nDisplay--;

    SAL_INFO("sd", "Presenting on real screen " << nDisplay);

    return nDisplay;
}

bool SlideShow::dependsOn( ViewShellBase const * pViewShellBase )
{
    return mxController.is() && (pViewShellBase == mpCurrentViewShellBase) && mpFullScreenViewShellBase;
}

Reference< presentation::XPresentation2 > CreatePresentation( const SdDrawDocument& rDocument )
{
    return Reference< presentation::XPresentation2 >( SlideShow::Create( const_cast< SdDrawDocument* >( &rDocument ) ).get() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
