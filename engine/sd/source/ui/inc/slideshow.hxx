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

#pragma once

#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <rtl/ref.hxx>

#include <tools/link.hxx>

#include <comphelper/compbase.hxx>

#include <editeng/unoipset.hxx>

#include <memory>
#include <sddllapi.h>

namespace com::sun::star {
    namespace drawing {
        class XDrawPage;
    }
    namespace animations {
        class XAnimationNode;
    }
}
class SdDrawDocument;
class KeyEvent;
class OutputDevice;
class Size;
namespace vcl { class Window; }
class SfxRequest;
class WorkWindow;
class CommandGestureSwipeData;
class CommandGestureLongPressData;
struct ImplSVEvent;

// TODO: Remove
#define PAGE_NO_END         65535

/* Definition of SlideShow class */

namespace sd
{

class SlideshowImpl;
class View;
class ViewShell;
class ViewShellBase;
struct PresentationSettingsEx;
class FrameView;

enum AnimationMode
{
    ANIMATIONMODE_SHOW,
    ANIMATIONMODE_PREVIEW
};

typedef comphelper::WeakComponentImplHelper< css::presentation::XPresentation2, css::lang::XServiceInfo > SlideshowBase;

class SlideShow final : public SlideshowBase
{
public:
    /// used by the model to create a slideshow for it
    static rtl::Reference< SlideShow > Create( SdDrawDocument* pDoc );

    // static helper api
    static rtl::Reference< SlideShow > GetSlideShow( SdDrawDocument const * pDocument );
    static rtl::Reference< SlideShow > GetSlideShow( SdDrawDocument const & rDocument );
    static rtl::Reference< SlideShow > GetSlideShow( ViewShellBase const & rBase );

    /// returns true if the interactive slideshow mode is activated
    static bool IsInteractiveSlideshow(const ViewShellBase& rViewShellBase);
    bool IsInteractiveSlideshow() const;

    // uno api

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // legacy api

    static sal_Int32 GetDisplay();

    bool IsExitAfterPresenting() const;
    void SetExitAfterPresenting(bool bExit);

private:
    SlideShow( SdDrawDocument* pDoc );

    /// @throws css::uno::RuntimeException
    void ThrowIfDisposed() const;

    WorkWindow *GetWorkWindow();

    SlideShow(const SlideShow&) = delete;
    SlideShow& operator=( const SlideShow& ) = delete;

    SvxItemPropertySet  maPropSet;

    SdDrawDocument* mpDoc;

    std::shared_ptr< PresentationSettingsEx > mxCurrentSettings;

    ViewShellBase* mpCurrentViewShellBase;
    ViewShellBase* mpFullScreenViewShellBase;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
