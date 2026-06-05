/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>

#include <comphelper/propertyvalue.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdlayer.hxx>
#include <svl/itemprop.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxsids.hrc>

#include <framework/FrameworkHelper.hxx>
#include <framework/ConfigurationChangeEvent.hxx>
#include <comphelper/extract.hxx>

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
#include <DrawController.hxx>
#include <PresentationViewShell.hxx>
#include <customshowlist.hxx>
#include <unopage.hxx>
#include <sdpage.hxx>
#include <cusshow.hxx>
#include <optsitem.hxx>
#include <strings.hrc>
#include <sdresid.hxx>
#include <ResourceId.hxx>

using ::com::sun::star::presentation::XSlideShowController;
using ::sd::framework::FrameworkHelper;
using ::com::sun::star::awt::XWindow;
using namespace ::sd;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace cpo::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::animations;

static std::span<const SfxItemPropertyMapEntry> ImplGetPresentationPropertyMap()
{
    // NOTE: First member must be sorted
    static const SfxItemPropertyMapEntry aPresentationPropertyMap_Impl[] =
    {
        { u"AllowAnimations"_ustr,          ATTR_PRESENT_ANIMATION_ALLOWED,       cppu::UnoType<bool>::get(),                0, 0 },
        { u"CustomShow"_ustr,               ATTR_PRESENT_CUSTOMSHOW,              ::cppu::UnoType<OUString>::get(),     0, 0 },
        { u"Display"_ustr,                  ATTR_PRESENT_DISPLAY,                 ::cppu::UnoType<sal_Int32>::get(),    0, 0 },
        { u"FirstPage"_ustr,                ATTR_PRESENT_DIANAME,                 ::cppu::UnoType<OUString>::get(),     0, 0 },
        { u"IsAlwaysOnTop"_ustr,            ATTR_PRESENT_ALWAYS_ON_TOP,           cppu::UnoType<bool>::get(),                0, 0 },
        { u"IsAutomatic"_ustr,              ATTR_PRESENT_MANUEL,                  cppu::UnoType<bool>::get(),                0, 0 },
        { u"IsEndless"_ustr,                ATTR_PRESENT_ENDLESS,                 cppu::UnoType<bool>::get(),                0, 0 },
        { u"IsFullScreen"_ustr,             ATTR_PRESENT_FULLSCREEN,              cppu::UnoType<bool>::get(),                0, 0 },
        { u"IsShowAll"_ustr,                ATTR_PRESENT_ALL,                     cppu::UnoType<bool>::get(),                0, 0 },
        { u"IsMouseVisible"_ustr,           ATTR_PRESENT_MOUSE,                   cppu::UnoType<bool>::get(),                0, 0 },
        { u"IsShowLogo"_ustr,               ATTR_PRESENT_SHOW_PAUSELOGO,          cppu::UnoType<bool>::get(),                0, 0 },
        { u"IsTransitionOnClick"_ustr,      ATTR_PRESENT_CHANGE_PAGE,             cppu::UnoType<bool>::get(),                0, 0 },
        { u"Pause"_ustr,                    ATTR_PRESENT_PAUSE_TIMEOUT,           ::cppu::UnoType<sal_Int32>::get(),    0, 0 },
        { u"StartWithNavigator"_ustr,       ATTR_PRESENT_NAVIGATOR,               cppu::UnoType<bool>::get(),                0, 0 },
        { u"UsePen"_ustr,                   ATTR_PRESENT_PEN,                     cppu::UnoType<bool>::get(),                0, 0 },
    };

    return aPresentationPropertyMap_Impl;
}


SlideShow::SlideShow( SdDrawDocument* pDoc )
: maPropSet(ImplGetPresentationPropertyMap(), SdrObject::GetGlobalDrawObjectItemPool())
, mpDoc( pDoc )
, mpCurrentViewShellBase( nullptr )
, mpFullScreenViewShellBase( nullptr )
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

/// returns true if the interactive slideshow mode is activated
bool SlideShow::IsInteractiveSlideshow(const ViewShellBase& rViewShellBase)
{
    rtl::Reference< SlideShow > xSlideShow(GetSlideShow(rViewShellBase));
    if (!xSlideShow.is())
        return false;
    return xSlideShow->IsInteractiveSlideshow();
}

bool SlideShow::IsInteractiveSlideshow() const
{
    return mpDoc->getPresentationSettings().mbInteractive;
}

// XServiceInfo
OUString SAL_CALL SlideShow::getImplementationName(  )
{
    return u"com.sun.star.comp.sd.SlideShow"_ustr;
}

bool SAL_CALL SlideShow::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL SlideShow::getSupportedServiceNames(  )
{
    return { u"com.sun.star.presentation.Presentation"_ustr };
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

    const SfxItemPropertyMapEntry* pEntry = maPropSet.getPropertyMapEntry(aPropertyName);

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

            SdOptions* pOptions = SdModule::get()->GetSdOptions(DocumentType::Impress);
            pOptions->SetDisplay( nDisplay );
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

    const SfxItemPropertyMapEntry* pEntry = maPropSet.getPropertyMapEntry(PropertyName);

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
        SdOptions* pOptions = SdModule::get()->GetSdOptions(DocumentType::Impress);
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

WorkWindow *SlideShow::GetWorkWindow()
{
    if( !mpFullScreenViewShellBase )
        return nullptr;

    PresentationViewShell* pShell = dynamic_cast<PresentationViewShell*>(mpFullScreenViewShellBase->GetMainViewShell().get());

    if( !pShell)
        return nullptr;

    SfxViewFrame* pFrame = pShell->GetViewFrame();
    if (!pFrame)
        return nullptr;

    return dynamic_cast<WorkWindow*>(pFrame->GetFrame().GetWindow().GetParent());
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

// XComponent

void SlideShow::disposing(std::unique_lock<std::mutex>&)
{
    SolarMutexGuard aGuard;

    mpCurrentViewShellBase = nullptr;
    mpFullScreenViewShellBase = nullptr;
    mpDoc = nullptr;
}

/// convert configuration setting display concept to real screens
sal_Int32 SlideShow::GetDisplay()
{
    sal_Int32 nDisplay = 0;

    SdOptions* pOptions = SdModule::get()->GetSdOptions(DocumentType::Impress);
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

Reference< presentation::XPresentation2 > CreatePresentation( const SdDrawDocument& rDocument )
{
    return Reference< presentation::XPresentation2 >( SlideShow::Create( const_cast< SdDrawDocument* >( &rDocument ) ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
