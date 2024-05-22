/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column:100 -*- */
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

#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/util/thePathSettings.hpp>
#include <com/sun/star/awt/XDevice.hpp>

#include "vbaapplication.hxx"
#include "vbadocument.hxx"
#include "vbafilterpropsfromformat.hxx"
#include <sal/log.hxx>
#include <osl/file.hxx>
#include <vcl/svapp.hxx>
#include <vbahelper/vbahelper.hxx>
#include "vbawindow.hxx"
#include "vbasystem.hxx"
#include "vbaoptions.hxx"
#include "vbaselection.hxx"
#include "vbadocuments.hxx"
#include "vbaaddins.hxx"
#include "vbamailmerge.hxx"
#include "vbadialogs.hxx"
#include "vbawordbasic.hxx"
#include <ooo/vba/XConnectionPoint.hpp>
#include <ooo/vba/word/WdEnableCancelKey.hpp>
#include <ooo/vba/word/WdWindowState.hpp>
#include <ooo/vba/word/XApplicationOutgoing.hpp>
#include <ooo/vba/word/XBookmarks.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <cppu/unotype.hxx>
#include <editeng/acorrcfg.hxx>
#include <swdll.hxx>
#include <swmodule.hxx>
#include "vbalistgalleries.hxx"
#include <tools/urlobj.hxx>

using namespace ::ooo;
using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace {

class SwVbaApplicationOutgoingConnectionPoint : public cppu::WeakImplHelper<XConnectionPoint>
{
private:
    SwVbaApplication* mpApp;

public:
    SwVbaApplicationOutgoingConnectionPoint( SwVbaApplication* pApp );

    // XConnectionPoint
    sal_uInt32 SAL_CALL Advise(const uno::Reference< XSink >& Sink ) override;
    void SAL_CALL Unadvise( sal_uInt32 Cookie ) override;
};

}

SwVbaApplication::SwVbaApplication( uno::Reference<uno::XComponentContext >& xContext ):
    SwVbaApplication_BASE( xContext )
{
}

SwVbaApplication::~SwVbaApplication()
{
}

sal_uInt32
SwVbaApplication::AddSink( const uno::Reference< XSink >& xSink )
{
    {
        SolarMutexGuard aGuard;
        SwGlobals::ensure();
    }
    // No harm in potentially calling this several times
    SW_MOD()->RegisterAutomationApplicationEventsCaller( uno::Reference< XSinkCaller >(this) );
    mvSinks.push_back(xSink);
    return mvSinks.size();
}

void
SwVbaApplication::RemoveSink( sal_uInt32 nNumber )
{
    if (nNumber < 1 || nNumber > mvSinks.size())
        return;

    mvSinks[nNumber-1] = uno::Reference< XSink >();
}

OUString SAL_CALL
SwVbaApplication::getName()
{
    return u"Microsoft Word"_ustr;
}

uno::Reference< word::XDocument > SAL_CALL
SwVbaApplication::getActiveDocument()
{
    return new SwVbaDocument( this, mxContext, getCurrentDocument() );
}

rtl::Reference<SwVbaWindow>
SwVbaApplication::getActiveSwVbaWindow()
{
    // #FIXME so far can't determine Parent
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_SET_THROW );
    uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
    return new SwVbaWindow( uno::Reference< XHelperInterface >(), mxContext, xModel, xController );
}

uno::Reference< css::uno::XComponentContext > const &
SwVbaApplication::getContext() const
{
    return mxContext;
}

uno::Reference< word::XWindow > SAL_CALL
SwVbaApplication::getActiveWindow()
{
    return getActiveSwVbaWindow();
}

uno::Reference<word::XSystem > SAL_CALL
SwVbaApplication::getSystem()
{
    return uno::Reference< word::XSystem >( new SwVbaSystem( mxContext ) );
}

uno::Reference<word::XOptions > SAL_CALL
SwVbaApplication::getOptions()
{
    return uno::Reference< word::XOptions >( new SwVbaOptions( mxContext ) );
}

uno::Any SAL_CALL
SwVbaApplication::CommandBars( const uno::Any& aIndex )
{
    try
    {
        return VbaApplicationBase::CommandBars( aIndex );
    }
    catch (const uno::RuntimeException&)
    {
        return uno::Any();
    }
}

uno::Reference< word::XSelection > SAL_CALL
SwVbaApplication::getSelection()
{
    return new SwVbaSelection( this, mxContext, getCurrentDocument() );
}

uno::Reference< word::XWordBasic > SAL_CALL
SwVbaApplication::getWordBasic()
{
    uno::Reference< word::XWordBasic > xWB( new SwWordBasic( this ) );
    return xWB;
}

uno::Any SAL_CALL
SwVbaApplication::Documents( const uno::Any& index )
{
    uno::Reference< XCollection > xCol( new SwVbaDocuments( this, mxContext ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaApplication::Addins( const uno::Any& index )
{
    static uno::Reference< XCollection > xCol( new SwVbaAddins( this, mxContext ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaApplication::Dialogs( const uno::Any& index )
{
    uno::Reference< word::XDialogs > xCol( new SwVbaDialogs( this, mxContext, getCurrentDocument() ));
    if ( index.hasValue() )
        return xCol->Item( index );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaApplication::ListGalleries( const uno::Any& index )
{
    uno::Reference< text::XTextDocument > xTextDoc( getCurrentDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xCol( new SwVbaListGalleries( this, mxContext, xTextDoc ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

sal_Bool SAL_CALL SwVbaApplication::getDisplayAutoCompleteTips()
{
    return SvxAutoCorrCfg::Get().IsAutoTextTip();
}

void SAL_CALL SwVbaApplication::setDisplayAutoCompleteTips( sal_Bool _displayAutoCompleteTips )
{
    SvxAutoCorrCfg::Get().SetAutoTextTip( _displayAutoCompleteTips );
}

sal_Int32 SAL_CALL SwVbaApplication::getEnableCancelKey()
{
    // the default value is wdCancelInterrupt in Word
    return word::WdEnableCancelKey::wdCancelInterrupt;
}

void SAL_CALL SwVbaApplication::setEnableCancelKey( sal_Int32/* _enableCancelKey */)
{
    // seems not supported in Writer
}

sal_Int32 SAL_CALL SwVbaApplication::getWindowState()
{
    auto xWindow = getActiveWindow();
    if (xWindow.is())
    {
        uno::Any aState = xWindow->getWindowState();
        sal_Int32 nState;
        if (aState >>= nState)
            return nState;
    }

    return word::WdWindowState::wdWindowStateNormal; // ?
}

void SAL_CALL SwVbaApplication::setWindowState( sal_Int32 _windowstate )
{
    try
    {
        auto xWindow = getActiveWindow();
        if (xWindow.is())
        {
            uno::Any aState;
            aState <<= _windowstate;
            xWindow->setWindowState( aState );
        }
    }
    catch (const uno::RuntimeException&)
    {
    }
}

sal_Int32 SAL_CALL SwVbaApplication::getWidth()
{
    auto pWindow = getActiveSwVbaWindow();
    return pWindow->getWidth();
}

void SAL_CALL SwVbaApplication::setWidth( sal_Int32 _width )
{
    auto pWindow = getActiveSwVbaWindow();
    pWindow->setWidth( _width );
}

sal_Int32 SAL_CALL SwVbaApplication::getHeight()
{
    auto pWindow = getActiveSwVbaWindow();
    return pWindow->getHeight();
}

void SAL_CALL SwVbaApplication::setHeight( sal_Int32 _height )
{
    auto pWindow = getActiveSwVbaWindow();
    pWindow->setHeight( _height );
}

sal_Int32 SAL_CALL SwVbaApplication::getLeft()
{
    auto pWindow = getActiveSwVbaWindow();
    return pWindow->getLeft();
}

void SAL_CALL SwVbaApplication::setLeft( sal_Int32 _left )
{
    auto pWindow = getActiveSwVbaWindow();
    pWindow->setLeft( _left );
}

sal_Int32 SAL_CALL SwVbaApplication::getTop()
{
    auto pWindow = getActiveSwVbaWindow();
    return pWindow->getTop();
}

void SAL_CALL SwVbaApplication::setTop( sal_Int32 _top )
{
    auto pWindow = getActiveSwVbaWindow();
    pWindow->setTop( _top );
}

OUString SAL_CALL SwVbaApplication::getStatusBar()
{
    return u""_ustr;
}

uno::Any SAL_CALL SwVbaApplication::getCustomizationContext()
{
    return uno::Any(); // ???
}

void SAL_CALL SwVbaApplication::setCustomizationContext(const uno::Any& /*_customizationcontext*/)
{
    // ???
}

void SAL_CALL SwVbaApplication::setStatusBar( const OUString& _statusbar )
{
    // ScVbaAppSettings::setStatusBar() also uses the XStatusIndicator to show this, so maybe that is OK?
    uno::Reference< frame::XModel > xModel = getCurrentDocument();
    if (xModel.is())
    {
        uno::Reference< task::XStatusIndicatorSupplier > xStatusIndicatorSupplier( xModel->getCurrentController(), uno::UNO_QUERY );
        if (xStatusIndicatorSupplier.is())
        {
            uno::Reference< task::XStatusIndicator > xStatusIndicator = xStatusIndicatorSupplier->getStatusIndicator();
            if (xStatusIndicator.is())
                xStatusIndicator->start( _statusbar, 100 );
        }
    }

    // Yes, we intentionally use the "extensions.olebridge" tag here even if this is sw. We
    // interpret setting the StatusBar property as a request from an Automation client to display
    // the string in LibreOffice's debug output, and all other generic Automation support debug
    // output (in extensions/source/ole) uses that tag. If the check for "cross-module" or mixed log
    // areas in compilerplugins/clang/sallogareas.cxx is re-activated, this will have to be added as
    // a special case.

    SAL_INFO("extensions.olebridge", "Client debug output: " << _statusbar);
}

float SAL_CALL SwVbaApplication::CentimetersToPoints( float Centimeters )
{
    return o3tl::convert(Centimeters, o3tl::Length::cm, o3tl::Length::pt);
}

float SAL_CALL SwVbaApplication::PointsToCentimeters( float Points )
{
    return o3tl::convert(Points, o3tl::Length::pt, o3tl::Length::cm);
}

float SAL_CALL SwVbaApplication::PixelsToPoints( float Pixels, ::sal_Bool fVertical )
{
    //Set up xDevice
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_SET_THROW );
    uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
    uno::Reference< frame::XFrame > xFrame( xController->getFrame(), uno::UNO_SET_THROW );
    uno::Reference< awt::XWindow > xWindow( xFrame->getContainerWindow(), uno::UNO_SET_THROW );
    css::uno::Reference< css::awt::XDevice > xDevice( xWindow, css::uno::UNO_QUERY );

    return ooo::vba::PixelsToPoints(xDevice, Pixels, fVertical);
}

float SAL_CALL SwVbaApplication::PointsToPixels( float Pixels, ::sal_Bool fVertical )
{
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_SET_THROW );
    uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
    uno::Reference< frame::XFrame > xFrame( xController->getFrame(), uno::UNO_SET_THROW );
    uno::Reference< awt::XWindow > xWindow( xFrame->getContainerWindow(), uno::UNO_SET_THROW );
    css::uno::Reference< css::awt::XDevice > xDevice( xWindow, css::uno::UNO_QUERY );

    return ooo::vba::PointsToPixels(xDevice, Pixels, fVertical);
}

float SAL_CALL SwVbaApplication::InchesToPoints( float Inches )
{
    return o3tl::convert(Inches, o3tl::Length::in, o3tl::Length::pt);
}

float SAL_CALL SwVbaApplication::PointsToInches( float Points )
{
    return o3tl::convert(Points, o3tl::Length::pt, o3tl::Length::in);
}

float SAL_CALL SwVbaApplication::MillimetersToPoints( float Millimeters )
{
    return o3tl::convert(Millimeters, o3tl::Length::mm, o3tl::Length::pt);
}

float SAL_CALL SwVbaApplication::PointsToMillimeters( float Points )
{
    return o3tl::convert(Points, o3tl::Length::pt, o3tl::Length::mm);
}

float SAL_CALL SwVbaApplication::PicasToPoints( float Picas )
{
    return o3tl::convert(Picas, o3tl::Length::pc, o3tl::Length::pt);
}

float SAL_CALL SwVbaApplication::PointsToPicas( float Points )
{
    return o3tl::convert(Points, o3tl::Length::pt, o3tl::Length::pc);
}

void SAL_CALL SwVbaApplication::ShowMe()
{
    // Method no longer supported in word - deprecated
}

void SAL_CALL SwVbaApplication::Resize( sal_Int32 Width, sal_Int32 Height )
{
    // Have to do it like this as the Width and Height are hidden away in the ooo::vba::XWindowBase
    // which ooo::vba::word::XApplication does not inherit from. SwVbaWindow, however, does inherit
    // from XWindowBase. Ugh.
    auto pWindow = getActiveSwVbaWindow();
    pWindow->setWidth( Width );
    pWindow->setHeight( Height );
}

void SAL_CALL SwVbaApplication::Move( sal_Int32 Left, sal_Int32 Top )
{
    // See comment in Resize().
    auto pWindow = getActiveSwVbaWindow();
    pWindow->setLeft( Left );
    pWindow->setTop( Top );
}

// XInterfaceWithIID

OUString SAL_CALL
SwVbaApplication::getIID()
{
    return u"{82154421-0FBF-11d4-8313-005004526AB4}"_ustr;
}

// XConnectable

OUString SAL_CALL
SwVbaApplication::GetIIDForClassItselfNotCoclass()
{
    return u"{82154423-0FBF-11D4-8313-005004526AB4}"_ustr;
}

TypeAndIID SAL_CALL
SwVbaApplication::GetConnectionPoint()
{
    TypeAndIID aResult =
        { cppu::UnoType<word::XApplicationOutgoing>::get(),
          u"{82154422-0FBF-11D4-8313-005004526AB4}"_ustr
        };

    return aResult;
}

uno::Reference<XConnectionPoint> SAL_CALL
SwVbaApplication::FindConnectionPoint()
{
    uno::Reference<XConnectionPoint> xCP(new SwVbaApplicationOutgoingConnectionPoint(this));
    return xCP;
}

OUString
SwVbaApplication::getServiceImplName()
{
    return u"SwVbaApplication"_ustr;
}

uno::Sequence< OUString >
SwVbaApplication::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Application"_ustr
    };
    return aServiceNames;
}

uno::Reference< frame::XModel >
SwVbaApplication::getCurrentDocument()
{
    return getCurrentWordDoc( mxContext );
}

// XSinkCaller

void SAL_CALL
SwVbaApplication::CallSinks( const OUString& Method, uno::Sequence< uno::Any >& Arguments )
{
    for (auto& i : mvSinks)
    {
        if (i.is())
            i->Call(Method, Arguments);
    }
}

// SwVbaApplicationOutgoingConnectionPoint

SwVbaApplicationOutgoingConnectionPoint::SwVbaApplicationOutgoingConnectionPoint( SwVbaApplication* pApp ) :
    mpApp(pApp)
{
}

// XConnectionPoint
sal_uInt32 SAL_CALL
SwVbaApplicationOutgoingConnectionPoint::Advise( const uno::Reference< XSink >& Sink )
{
    return mpApp->AddSink(Sink);
}

void SAL_CALL
SwVbaApplicationOutgoingConnectionPoint::Unadvise( sal_uInt32 Cookie )
{
    mpApp->RemoveSink( Cookie );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
