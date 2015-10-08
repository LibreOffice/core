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

#include <toolkit/awt/vclxwindows.hxx>
#include "toolkit/awt/scrollabledialog.hxx"
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/convert.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <com/sun/star/awt/XItemList.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/longcurr.hxx>
#include <vcl/imgctrl.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/settings.hxx>
#include <tools/diagnose_ex.h>

#include <boost/bind.hpp>

#include <vcl/group.hxx>

#include "helper/accessibilityclient.hxx"
#include "helper/imagealign.hxx"
#include "helper/tkresmgr.hxx"
#include "vclxwindows_internal.hxx"

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::awt::ItemListEvent;
using ::com::sun::star::awt::XItemList;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::graphic::XGraphicProvider;

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt::VisualEffect;
namespace ImageScaleMode = ::com::sun::star::awt::ImageScaleMode;

static double ImplCalcLongValue( double nValue, sal_uInt16 nDigits )
{
    double n = nValue;
    for ( sal_uInt16 d = 0; d < nDigits; d++ )
        n *= 10;
    return n;
}

static double ImplCalcDoubleValue( double nValue, sal_uInt16 nDigits )
{
    double n = nValue;
    for ( sal_uInt16 d = 0; d < nDigits; d++ )
        n /= 10;
    return n;
}

namespace toolkit
{
    /** sets the "face color" for button like controls (scroll bar, spin button)
    */
    void setButtonLikeFaceColor( vcl::Window* _pWindow, const ::com::sun::star::uno::Any& _rColorValue )
    {
        AllSettings aSettings = _pWindow->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();

        if ( !_rColorValue.hasValue() )
        {
            const StyleSettings& aAppStyle = Application::GetSettings().GetStyleSettings();
            aStyleSettings.SetFaceColor( aAppStyle.GetFaceColor( ) );
            aStyleSettings.SetCheckedColor( aAppStyle.GetCheckedColor( ) );
            aStyleSettings.SetLightBorderColor( aAppStyle.GetLightBorderColor() );
            aStyleSettings.SetLightColor( aAppStyle.GetLightColor() );
            aStyleSettings.SetShadowColor( aAppStyle.GetShadowColor() );
            aStyleSettings.SetDarkShadowColor( aAppStyle.GetDarkShadowColor() );
        }
        else
        {
            sal_Int32 nBackgroundColor = 0;
            _rColorValue >>= nBackgroundColor;
            aStyleSettings.SetFaceColor( nBackgroundColor );

            // for the real background (everything except the buttons and the thumb),
            // use an average between the desired color and "white"
            Color aWhite( COL_WHITE );
            Color aBackground( nBackgroundColor );
            aBackground.SetRed( ( aBackground.GetRed() + aWhite.GetRed() ) / 2 );
            aBackground.SetGreen( ( aBackground.GetGreen() + aWhite.GetGreen() ) / 2 );
            aBackground.SetBlue( ( aBackground.GetBlue() + aWhite.GetBlue() ) / 2 );
            aStyleSettings.SetCheckedColor( aBackground );

            sal_Int32 nBackgroundLuminance = Color( nBackgroundColor ).GetLuminance();
            sal_Int32 nWhiteLuminance = Color( COL_WHITE ).GetLuminance();

            Color aLightShadow( nBackgroundColor );
            aLightShadow.IncreaseLuminance( (sal_uInt8)( ( nWhiteLuminance - nBackgroundLuminance ) * 2 / 3 ) );
            aStyleSettings.SetLightBorderColor( aLightShadow );

            Color aLight( nBackgroundColor );
            aLight.IncreaseLuminance( (sal_uInt8)( ( nWhiteLuminance - nBackgroundLuminance ) * 1 / 3 ) );
            aStyleSettings.SetLightColor( aLight );

            Color aShadow( nBackgroundColor );
            aShadow.DecreaseLuminance( (sal_uInt8)( nBackgroundLuminance * 1 / 3 ) );
            aStyleSettings.SetShadowColor( aShadow );

            Color aDarkShadow( nBackgroundColor );
            aDarkShadow.DecreaseLuminance( (sal_uInt8)( nBackgroundLuminance * 2 / 3 ) );
            aStyleSettings.SetDarkShadowColor( aDarkShadow );
        }

        aSettings.SetStyleSettings( aStyleSettings );
        _pWindow->SetSettings( aSettings, true );
    }

    Any getButtonLikeFaceColor( const vcl::Window* _pWindow )
    {
        sal_Int32 nBackgroundColor = _pWindow->GetSettings().GetStyleSettings().GetFaceColor().GetColor();
        return makeAny( nBackgroundColor );
    }

    static void adjustBooleanWindowStyle( const Any& _rValue, vcl::Window* _pWindow, WinBits _nBits, bool _bInverseSemantics )
    {
        WinBits nStyle = _pWindow->GetStyle();
        bool bValue( false );
        OSL_VERIFY( _rValue >>= bValue );
        if ( bValue != _bInverseSemantics )
            nStyle |= _nBits;
        else
            nStyle &= ~_nBits;
        _pWindow->SetStyle( nStyle );
    }

    static void setVisualEffect( const Any& _rValue, vcl::Window* _pWindow )
    {
        AllSettings aSettings = _pWindow->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();

        sal_Int16 nStyle = LOOK3D;
        OSL_VERIFY( _rValue >>= nStyle );
        switch ( nStyle )
        {
        case FLAT:
            aStyleSettings.SetOptions( aStyleSettings.GetOptions() | StyleSettingsOptions::Mono );
            break;
        case LOOK3D:
        default:
            aStyleSettings.SetOptions( aStyleSettings.GetOptions() & ~StyleSettingsOptions::Mono );
        }
        aSettings.SetStyleSettings( aStyleSettings );
        _pWindow->SetSettings( aSettings );
    }

    static Any getVisualEffect( vcl::Window* _pWindow )
    {
        Any aEffect;

        StyleSettings aStyleSettings = _pWindow->GetSettings().GetStyleSettings();
        if ( (aStyleSettings.GetOptions() & StyleSettingsOptions::Mono) )
            aEffect <<= (sal_Int16)FLAT;
        else
            aEffect <<= (sal_Int16)LOOK3D;
        return aEffect;
    }
}


//  class VCLXGraphicControl


void VCLXGraphicControl::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    VCLXWindow::ImplGetPropertyIds( rIds );
}

void VCLXGraphicControl::ImplSetNewImage()
{
    OSL_PRECOND( GetWindow(), "VCLXGraphicControl::ImplSetNewImage: window is required to be not-NULL!" );
    VclPtr< Button > pButton = GetAsDynamic< Button >();
    pButton->SetModeImage( GetImage() );
}

void VCLXGraphicControl::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, short Flags ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        Size aOldSize = GetWindow()->GetSizePixel();
        VCLXWindow::setPosSize( X, Y, Width, Height, Flags );
        if ( ( aOldSize.Width() != Width ) || ( aOldSize.Height() != Height ) )
            ImplSetNewImage();
    }
}

void VCLXGraphicControl::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( !GetWindow() )
        return;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_GRAPHIC:
        {
            Reference< XGraphic > xGraphic;
            OSL_VERIFY( Value >>= xGraphic );
            maImage = Image( xGraphic );
            ImplSetNewImage();
        }
        break;

        case BASEPROPERTY_IMAGEALIGN:
        {
            WindowType eType = GetWindow()->GetType();
            if (  ( eType == WINDOW_PUSHBUTTON )
               || ( eType == WINDOW_RADIOBUTTON )
               || ( eType == WINDOW_CHECKBOX )
               )
            {
                sal_Int16 nAlignment = sal_Int16();
                if ( Value >>= nAlignment )
                    GetAs< Button >()->SetImageAlign( static_cast< ImageAlign >( nAlignment ) );
            }
        }
        break;
        case BASEPROPERTY_IMAGEPOSITION:
        {
            WindowType eType = GetWindow()->GetType();
            if (  ( eType == WINDOW_PUSHBUTTON )
               || ( eType == WINDOW_RADIOBUTTON )
               || ( eType == WINDOW_CHECKBOX )
               )
            {
                sal_Int16 nImagePosition = 2;
                OSL_VERIFY( Value >>= nImagePosition );
                GetAs<Button>()->SetImageAlign( ::toolkit::translateImagePosition( nImagePosition ) );
            }
        }
        break;
        default:
            VCLXWindow::setProperty( PropertyName, Value );
            break;
    }
}

::com::sun::star::uno::Any VCLXGraphicControl::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    if ( !GetWindow() )
        return aProp;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_GRAPHIC:
            aProp <<= maImage.GetXGraphic();
            break;
        case BASEPROPERTY_IMAGEALIGN:
        {
            WindowType eType = GetWindow()->GetType();
            if  (  ( eType == WINDOW_PUSHBUTTON )
                || ( eType == WINDOW_RADIOBUTTON )
                || ( eType == WINDOW_CHECKBOX )
                )
            {
                 aProp <<= ::toolkit::getCompatibleImageAlign(
                                GetAs<Button>()->GetImageAlign() );
            }
        }
        break;
        case BASEPROPERTY_IMAGEPOSITION:
        {
            WindowType eType = GetWindow()->GetType();
            if  (  ( eType == WINDOW_PUSHBUTTON )
                || ( eType == WINDOW_RADIOBUTTON )
                || ( eType == WINDOW_CHECKBOX )
                )
            {
                aProp <<= ::toolkit::translateImagePosition(
                        GetAs< Button >()->GetImageAlign() );
            }
        }
        break;
        default:
        {
            aProp <<= VCLXWindow::getProperty( PropertyName );
        }
        break;
    }
    return aProp;
}


//  class VCLXButton


void VCLXButton::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_DEFAULTBUTTON,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_GRAPHIC,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_IMAGEALIGN,
                     BASEPROPERTY_IMAGEPOSITION,
                     BASEPROPERTY_IMAGEURL,
                     BASEPROPERTY_LABEL,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_PUSHBUTTONTYPE,
                     BASEPROPERTY_REPEAT,
                     BASEPROPERTY_REPEAT_DELAY,
                     BASEPROPERTY_STATE,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_TOGGLE,
                     BASEPROPERTY_FOCUSONCLICK,
                     BASEPROPERTY_MULTILINE,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_VERTICALALIGN,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     BASEPROPERTY_REFERENCE_DEVICE,
                     0);
    VCLXGraphicControl::ImplGetPropertyIds( rIds );
}

VCLXButton::VCLXButton()
    :maActionListeners( *this )
    ,maItemListeners( *this )
{
}

VCLXButton::~VCLXButton()
{
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXButton::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXButton::dispose() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maActionListeners.disposeAndClear( aObj );
    maItemListeners.disposeAndClear( aObj );
    VCLXGraphicControl::dispose();
}

void VCLXButton::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l  )throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXButton::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXButton::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l  )throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXButton::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXButton::setLabel( const OUString& rLabel ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rLabel );
}

void VCLXButton::setActionCommand( const OUString& rCommand ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    maActionCommand = rCommand;
}

::com::sun::star::awt::Size VCLXButton::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< PushButton > pButton = GetAs< PushButton >();
    if ( pButton )
        aSz = pButton->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXButton::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::awt::Size aSz = getMinimumSize();
    aSz.Width += 16;
    aSz.Height += 10;
    return aSz;
}

::com::sun::star::awt::Size VCLXButton::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz = VCLSize(rNewSize);
    VclPtr< PushButton > pButton = GetAs< PushButton >();
    if ( pButton )
    {
        Size aMinSz = pButton->CalcMinimumSize();
        // no text, thus image
        if ( pButton->GetText().isEmpty() )
        {
            if ( aSz.Width() < aMinSz.Width() )
                aSz.Width() = aMinSz.Width();
            if ( aSz.Height() < aMinSz.Height() )
                aSz.Height() = aMinSz.Height();
        }
        else
        {
            if ( ( aSz.Width() > aMinSz.Width() ) && ( aSz.Height() < aMinSz.Height() ) )
                aSz.Height() = aMinSz.Height();
            else
                aSz = aMinSz;
        }
    }
    return AWTSize(aSz);
}

void VCLXButton::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< Button > pButton = GetAs< Button >();
    if ( pButton )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_FOCUSONCLICK:
                ::toolkit::adjustBooleanWindowStyle( Value, pButton, WB_NOPOINTERFOCUS, true );
                break;

            case BASEPROPERTY_TOGGLE:
                ::toolkit::adjustBooleanWindowStyle( Value, pButton, WB_TOGGLE, false );
                break;

            case BASEPROPERTY_DEFAULTBUTTON:
            {
                WinBits nStyle = pButton->GetStyle() | WB_DEFBUTTON;
                bool b = bool();
                if ( ( Value >>= b ) && !b )
                    nStyle &= ~WB_DEFBUTTON;
                pButton->SetStyle( nStyle );
            }
            break;
            case BASEPROPERTY_STATE:
            {
                if ( GetWindow()->GetType() == WINDOW_PUSHBUTTON )
                {
                    sal_Int16 n = sal_Int16();
                    if ( Value >>= n )
                        static_cast<PushButton*>(pButton.get())->SetState( (TriState)n );
                }
            }
            break;
            default:
            {
                VCLXGraphicControl::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXButton::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    VclPtr< Button > pButton = GetAs< Button >();
    if ( pButton )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_FOCUSONCLICK:
                aProp <<= ( ( pButton->GetStyle() & WB_NOPOINTERFOCUS ) == 0 );
                break;

            case BASEPROPERTY_TOGGLE:
                aProp <<= ( ( pButton->GetStyle() & WB_TOGGLE ) != 0 );
                break;

            case BASEPROPERTY_DEFAULTBUTTON:
            {
                aProp <<= ( pButton->GetStyle() & WB_DEFBUTTON ) != 0;
            }
            break;
            case BASEPROPERTY_STATE:
            {
                if ( GetWindow()->GetType() == WINDOW_PUSHBUTTON )
                {
                     aProp <<= (sal_Int16)static_cast<PushButton*>(pButton.get())->GetState();
                }
            }
            break;
            default:
            {
                aProp <<= VCLXGraphicControl::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXButton::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_BUTTON_CLICK:
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
                // since we call listeners below, there is a potential that we will be destroyed
                // during the listener call. To prevent the resulting crashs, we keep us
                // alive as long as we're here

            if ( maActionListeners.getLength() )
            {
                ::com::sun::star::awt::ActionEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                aEvent.ActionCommand = maActionCommand;

                Callback aCallback = ::boost::bind(
                    &ActionListenerMultiplexer::actionPerformed,
                    &maActionListeners,
                    aEvent
                );
                ImplExecuteAsyncWithoutSolarLock( aCallback );
            }
        }
        break;

        case VCLEVENT_PUSHBUTTON_TOGGLE:
        {
            PushButton& rButton = dynamic_cast< PushButton& >( *rVclWindowEvent.GetWindow() );

            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
            if ( maItemListeners.getLength() )
            {
                ::com::sun::star::awt::ItemEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                aEvent.Selected = ( rButton.GetState() == TRISTATE_TRUE ) ? 1 : 0;
                maItemListeners.itemStateChanged( aEvent );
            }
        }
        break;

        default:
            VCLXGraphicControl::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}


//  class VCLXImageControl


void VCLXImageControl::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_GRAPHIC,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_IMAGEURL,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_SCALEIMAGE,
                     BASEPROPERTY_IMAGE_SCALE_MODE,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     0);
    VCLXGraphicControl::ImplGetPropertyIds( rIds );
}

VCLXImageControl::VCLXImageControl()
{
}

VCLXImageControl::~VCLXImageControl()
{
}

void VCLXImageControl::ImplSetNewImage()
{
    OSL_PRECOND( GetWindow(), "VCLXImageControl::ImplSetNewImage: window is required to be not-NULL!" );
    VclPtr<ImageControl> pControl = GetAs< ImageControl >();
    pControl->SetImage( GetImage() );
}

::com::sun::star::awt::Size VCLXImageControl::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz = GetImage().GetSizePixel();
    aSz = ImplCalcWindowSize( aSz );

    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXImageControl::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXImageControl::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::Size aSz = rNewSize;
    ::com::sun::star::awt::Size aMinSz = getMinimumSize();
    if ( aSz.Width < aMinSz.Width )
        aSz.Width = aMinSz.Width;
    if ( aSz.Height < aMinSz.Height )
        aSz.Height = aMinSz.Height;
    return aSz;
}

void VCLXImageControl::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ImageControl > pImageControl = GetAs< ImageControl >();

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_IMAGE_SCALE_MODE:
        {
            sal_Int16 nScaleMode( ImageScaleMode::ANISOTROPIC );
            if ( pImageControl && ( Value >>= nScaleMode ) )
            {
                pImageControl->SetScaleMode( nScaleMode );
            }
        }
        break;

        case BASEPROPERTY_SCALEIMAGE:
        {
            // this is for compatibility only, nowadays, the ImageScaleMode property should be used
            bool bScaleImage = false;
            if ( pImageControl && ( Value >>= bScaleImage ) )
            {
                pImageControl->SetScaleMode( bScaleImage ? ImageScaleMode::ANISOTROPIC : ImageScaleMode::NONE );
            }
        }
        break;

        default:
            VCLXGraphicControl::setProperty( PropertyName, Value );
            break;
    }
}

::com::sun::star::uno::Any VCLXImageControl::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    VclPtr< ImageControl > pImageControl = GetAs< ImageControl >();
    sal_uInt16 nPropType = GetPropertyId( PropertyName );

    switch ( nPropType )
    {
        case BASEPROPERTY_IMAGE_SCALE_MODE:
            aProp <<= ( pImageControl ? pImageControl->GetScaleMode() : ImageScaleMode::ANISOTROPIC );
            break;

        case BASEPROPERTY_SCALEIMAGE:
            aProp <<= ( pImageControl && pImageControl->GetScaleMode() != ImageScaleMode::NONE );
            break;

        default:
            aProp = VCLXGraphicControl::getProperty( PropertyName );
            break;
    }
    return aProp;
}


//  class VCLXCheckBox



void VCLXCheckBox::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_GRAPHIC,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_IMAGEPOSITION,
                     BASEPROPERTY_IMAGEURL,
                     BASEPROPERTY_LABEL,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_STATE,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_TRISTATE,
                     BASEPROPERTY_VISUALEFFECT,
                     BASEPROPERTY_MULTILINE,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_VERTICALALIGN,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     BASEPROPERTY_REFERENCE_DEVICE,
                     0);
    VCLXGraphicControl::ImplGetPropertyIds( rIds );
}

VCLXCheckBox::VCLXCheckBox() :  maActionListeners( *this ), maItemListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXCheckBox::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XButton* >(this)),
                                        (static_cast< ::com::sun::star::awt::XCheckBox* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXGraphicControl::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXCheckBox )
    cppu::UnoType<com::sun::star::awt::XButton>::get(),
    cppu::UnoType<com::sun::star::awt::XCheckBox>::get(),
    VCLXGraphicControl::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXCheckBox::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXCheckBox::dispose() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maItemListeners.disposeAndClear( aObj );
    VCLXGraphicControl::dispose();
}

void VCLXCheckBox::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXCheckBox::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXCheckBox::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l  )throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXCheckBox::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXCheckBox::setActionCommand( const OUString& rCommand ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maActionCommand = rCommand;
}

void VCLXCheckBox::setLabel( const OUString& rLabel ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rLabel );
}

void VCLXCheckBox::setState( short n ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< CheckBox> pCheckBox = GetAs< CheckBox >();
    if ( pCheckBox)
    {
        TriState eState;
        switch ( n )
        {
            case 0:     eState = TRISTATE_FALSE;     break;
            case 1:     eState = TRISTATE_TRUE;       break;
            case 2:     eState = TRISTATE_INDET;    break;
            default:    eState = TRISTATE_FALSE;
        }
        pCheckBox->SetState( eState );

        // #105198# call C++ click listeners (needed for accessibility)
        // pCheckBox->GetClickHdl().Call( pCheckBox );

        // #107218# Call same virtual methods and listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( true );
        pCheckBox->Toggle();
        pCheckBox->Click();
        SetSynthesizingVCLEvent( false );
    }
}

short VCLXCheckBox::getState() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    short nState = -1;
    VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
    if ( pCheckBox )
    {
        switch ( pCheckBox->GetState() )
        {
            case TRISTATE_FALSE:     nState = 0; break;
            case TRISTATE_TRUE:       nState = 1; break;
            case TRISTATE_INDET:    nState = 2; break;
            default:                OSL_FAIL( "VCLXCheckBox::getState(): unknown TriState!" );
        }
    }

    return nState;
}

void VCLXCheckBox::enableTriState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
    if ( pCheckBox)
        pCheckBox->EnableTriState( b );
}

::com::sun::star::awt::Size VCLXCheckBox::getMinimumSize() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
    if ( pCheckBox )
        aSz = pCheckBox->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXCheckBox::getPreferredSize() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXCheckBox::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz = VCLSize(rNewSize);
    VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
    if ( pCheckBox )
    {
        Size aMinSz = pCheckBox->CalcMinimumSize();
        if ( ( aSz.Width() > aMinSz.Width() ) && ( aSz.Height() < aMinSz.Height() ) )
            aSz.Height() = aMinSz.Height();
        else
            aSz = aMinSz;
    }
    return AWTSize(aSz);
}

void VCLXCheckBox::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
    if ( pCheckBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VISUALEFFECT:
                ::toolkit::setVisualEffect( Value, pCheckBox );
                break;

            case BASEPROPERTY_TRISTATE:
            {
                bool b = bool();
                if ( Value >>= b )
                     pCheckBox->EnableTriState( b );
            }
            break;
            case BASEPROPERTY_STATE:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                    setState( n );
            }
            break;
            default:
            {
                VCLXGraphicControl::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXCheckBox::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
    if ( pCheckBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VISUALEFFECT:
                aProp = ::toolkit::getVisualEffect( pCheckBox );
                break;
            case BASEPROPERTY_TRISTATE:
                 aProp <<= pCheckBox->IsTriStateEnabled();
                break;
            case BASEPROPERTY_STATE:
                 aProp <<= (sal_Int16)pCheckBox->GetState();
                break;
            default:
            {
                aProp <<= VCLXGraphicControl::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXCheckBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_CHECKBOX_TOGGLE:
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
                // since we call listeners below, there is a potential that we will be destroyed
                // in during the listener call. To prevent the resulting crashs, we keep us
                // alive as long as we're here

            VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
            if ( pCheckBox )
            {
                if ( maItemListeners.getLength() )
                {
                    ::com::sun::star::awt::ItemEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                    aEvent.Highlighted = 0;
                    aEvent.Selected = pCheckBox->GetState();
                    maItemListeners.itemStateChanged( aEvent );
                }
                if ( !IsSynthesizingVCLEvent() && maActionListeners.getLength() )
                {
                    ::com::sun::star::awt::ActionEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                    aEvent.ActionCommand = maActionCommand;
                    maActionListeners.actionPerformed( aEvent );
                }
            }
        }
        break;

        default:
            VCLXGraphicControl::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}


//  class VCLXRadioButton

void VCLXRadioButton::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_GRAPHIC,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_IMAGEPOSITION,
                     BASEPROPERTY_IMAGEURL,
                     BASEPROPERTY_LABEL,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_STATE,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_VISUALEFFECT,
                     BASEPROPERTY_MULTILINE,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_VERTICALALIGN,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     BASEPROPERTY_REFERENCE_DEVICE,
                     BASEPROPERTY_GROUPNAME,
                     0);
    VCLXGraphicControl::ImplGetPropertyIds( rIds );
}


VCLXRadioButton::VCLXRadioButton() : maItemListeners( *this ), maActionListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXRadioButton::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XRadioButton* >(this)),
                                        (static_cast< ::com::sun::star::awt::XButton* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXGraphicControl::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXRadioButton )
    cppu::UnoType<com::sun::star::awt::XRadioButton>::get(),
    cppu::UnoType<com::sun::star::awt::XButton>::get(),
    VCLXGraphicControl::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXRadioButton::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXRadioButton::dispose() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maItemListeners.disposeAndClear( aObj );
    VCLXGraphicControl::dispose();
}

void VCLXRadioButton::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< RadioButton > pButton = GetAs< RadioButton >();
    if ( pButton )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VISUALEFFECT:
                ::toolkit::setVisualEffect( Value, pButton );
                break;

            case BASEPROPERTY_STATE:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                {
                    bool b = n != 0;
                    if ( pButton->IsRadioCheckEnabled() )
                        pButton->Check( b );
                    else
                        pButton->SetState( b );
                }
            }
            break;
            case BASEPROPERTY_AUTOTOGGLE:
            {
                bool b = bool();
                if ( Value >>= b )
                    pButton->EnableRadioCheck( b );
            }
            break;
            default:
            {
                VCLXGraphicControl::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXRadioButton::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    VclPtr< RadioButton > pButton = GetAs< RadioButton >();
    if ( pButton )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VISUALEFFECT:
                aProp = ::toolkit::getVisualEffect( pButton );
                break;
            case BASEPROPERTY_STATE:
                aProp <<= (sal_Int16) ( pButton->IsChecked() ? 1 : 0 );
                break;
            case BASEPROPERTY_AUTOTOGGLE:
                aProp <<= pButton->IsRadioCheckEnabled();
                break;
            default:
            {
                aProp <<= VCLXGraphicControl::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXRadioButton::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXRadioButton::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXRadioButton::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l  )throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXRadioButton::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXRadioButton::setLabel( const OUString& rLabel ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rLabel );
}

void VCLXRadioButton::setActionCommand( const OUString& rCommand ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maActionCommand = rCommand;
}

void VCLXRadioButton::setState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< RadioButton > pRadioButton = GetAs< RadioButton >();
    if ( pRadioButton)
    {
        pRadioButton->Check( b );
        // #102717# item listeners are called, but not C++ click listeners in StarOffice code => call click hdl
        // But this is needed in old code because Accessibility API uses it.
        // pRadioButton->GetClickHdl().Call( pRadioButton );

        // #107218# Call same virtual methods and listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( true );
        pRadioButton->Click();
        SetSynthesizingVCLEvent( false );
    }
}

sal_Bool VCLXRadioButton::getState() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< RadioButton > pRadioButton = GetAs< RadioButton >();
    return pRadioButton && pRadioButton->IsChecked();
}

::com::sun::star::awt::Size VCLXRadioButton::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< RadioButton > pRadioButton = GetAs< RadioButton >();
    if ( pRadioButton )
        aSz = pRadioButton->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXRadioButton::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXRadioButton::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz = VCLSize(rNewSize);
    VclPtr< RadioButton > pRadioButton = GetAs< RadioButton >();
    if ( pRadioButton )
    {
        Size aMinSz = pRadioButton->CalcMinimumSize();
        if ( ( aSz.Width() > aMinSz.Width() ) && ( aSz.Height() < aMinSz.Height() ) )
            aSz.Height() = aMinSz.Height();
        else
            aSz = aMinSz;
    }
    return AWTSize(aSz);
}

void VCLXRadioButton::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
        // since we call listeners below, there is a potential that we will be destroyed
        // in during the listener call. To prevent the resulting crashs, we keep us
        // alive as long as we're here

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_BUTTON_CLICK:
            if ( !IsSynthesizingVCLEvent() && maActionListeners.getLength() )
            {
                ::com::sun::star::awt::ActionEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                aEvent.ActionCommand = maActionCommand;
                maActionListeners.actionPerformed( aEvent );
            }
            ImplClickedOrToggled( false );
            break;

        case VCLEVENT_RADIOBUTTON_TOGGLE:
            ImplClickedOrToggled( true );
            break;

        default:
            VCLXGraphicControl::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

void VCLXRadioButton::ImplClickedOrToggled( bool bToggled )
{
    // In the formulars, RadioChecked is not enabled, call itemStateChanged only for click
    // In the dialog editor, RadioChecked is enabled, call itemStateChanged only for bToggled
    VclPtr< RadioButton > pRadioButton = GetAs< RadioButton >();
    if ( pRadioButton && ( pRadioButton->IsRadioCheckEnabled() == bToggled ) && ( bToggled || pRadioButton->IsStateChanged() ) && maItemListeners.getLength() )
    {
        ::com::sun::star::awt::ItemEvent aEvent;
        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
        aEvent.Highlighted = 0;
        aEvent.Selected = pRadioButton->IsChecked() ? 1 : 0;
        maItemListeners.itemStateChanged( aEvent );
    }
}


//  class VCLXSpinField

void VCLXSpinField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR,
                     0 );
    VCLXEdit::ImplGetPropertyIds( rIds );
}

VCLXSpinField::VCLXSpinField() : maSpinListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXSpinField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XSpinField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXEdit::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXSpinField )
    cppu::UnoType<com::sun::star::awt::XSpinField>::get(),
    VCLXEdit::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXSpinField::addSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maSpinListeners.addInterface( l );
}

void VCLXSpinField::removeSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maSpinListeners.removeInterface( l );
}

void VCLXSpinField::up() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< SpinField > pSpinField = GetAs< SpinField >();
    if ( pSpinField )
        pSpinField->Up();
}

void VCLXSpinField::down() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< SpinField > pSpinField = GetAs< SpinField >();
    if ( pSpinField )
        pSpinField->Down();
}

void VCLXSpinField::first() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< SpinField > pSpinField = GetAs< SpinField >();
    if ( pSpinField )
        pSpinField->First();
}

void VCLXSpinField::last() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< SpinField > pSpinField = GetAs< SpinField >();
    if ( pSpinField )
        pSpinField->Last();
}

void VCLXSpinField::enableRepeat( sal_Bool bRepeat ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        if ( bRepeat )
            nStyle |= WB_REPEAT;
        else
            nStyle &= ~WB_REPEAT;
        pWindow->SetStyle( nStyle );
    }
}

void VCLXSpinField::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_SPINFIELD_UP:
        case VCLEVENT_SPINFIELD_DOWN:
        case VCLEVENT_SPINFIELD_FIRST:
        case VCLEVENT_SPINFIELD_LAST:
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
                // since we call listeners below, there is a potential that we will be destroyed
                // in during the listener call. To prevent the resulting crashs, we keep us
                // alive as long as we're here

            if ( maSpinListeners.getLength() )
            {
                ::com::sun::star::awt::SpinEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                switch ( rVclWindowEvent.GetId() )
                {
                    case VCLEVENT_SPINFIELD_UP:     maSpinListeners.up( aEvent );
                                                    break;
                    case VCLEVENT_SPINFIELD_DOWN:   maSpinListeners.down( aEvent );
                                                    break;
                    case VCLEVENT_SPINFIELD_FIRST:  maSpinListeners.first( aEvent );
                                                    break;
                    case VCLEVENT_SPINFIELD_LAST:   maSpinListeners.last( aEvent );
                                                    break;
                }

            }
        }
        break;

        default:
            VCLXEdit::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}



//  class VCLXListBox

void VCLXListBox::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_DROPDOWN,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_LINECOUNT,
                     BASEPROPERTY_MULTISELECTION,
                     BASEPROPERTY_MULTISELECTION_SIMPLEMODE,
                     BASEPROPERTY_ITEM_SEPARATOR_POS,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_SELECTEDITEMS,
                     BASEPROPERTY_STRINGITEMLIST,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_READONLY,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     BASEPROPERTY_REFERENCE_DEVICE,
                     BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds );
}


VCLXListBox::VCLXListBox()
    : maActionListeners( *this ),
      maItemListeners( *this )
{
}

void VCLXListBox::dispose() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maItemListeners.disposeAndClear( aObj );
    maActionListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void VCLXListBox::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXListBox::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXListBox::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXListBox::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXListBox::addItem( const OUString& aItem, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        pBox->InsertEntry( aItem, nPos );
}

void VCLXListBox::addItems( const ::com::sun::star::uno::Sequence< OUString>& aItems, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
    {
        sal_uInt16 nP = nPos;
        const OUString* pItems = aItems.getConstArray();
        const OUString* pItemsEnd = aItems.getConstArray() + aItems.getLength();
        while ( pItems != pItemsEnd )
        {
            if ( (sal_uInt16)nP == 0xFFFF )
            {
                OSL_FAIL( "VCLXListBox::addItems: too many entries!" );
                // skip remaining entries, list cannot hold them, anyway
                break;
            }

            pBox->InsertEntry( *pItems++, nP++ );
        }
    }
}

void VCLXListBox::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
    {
        for ( sal_Int16 n = nCount; n; )
            pBox->RemoveEntry( nPos + (--n) );
    }
}

sal_Int16 VCLXListBox::getItemCount() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ListBox > pBox = GetAs< ListBox >();
    return pBox ? pBox->GetEntryCount() : 0;
}

OUString VCLXListBox::getItem( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aItem;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        aItem = pBox->GetEntry( nPos );
    return aItem;
}

::com::sun::star::uno::Sequence< OUString> VCLXListBox::getItems() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Sequence< OUString> aSeq;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
    {
        auto n = pBox->GetEntryCount();
        aSeq = ::com::sun::star::uno::Sequence< OUString>( n );
        while (n)
        {
            --n;
            aSeq.getArray()[n] = pBox->GetEntry( n );
        }
    }
    return aSeq;
}

sal_Int16 VCLXListBox::getSelectedItemPos() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    return pBox ? pBox->GetSelectEntryPos() : 0;
}

::com::sun::star::uno::Sequence<sal_Int16> VCLXListBox::getSelectedItemsPos() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Sequence<sal_Int16> aSeq;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
    {
        const sal_Int32 nSelEntries = pBox->GetSelectEntryCount();
        aSeq = ::com::sun::star::uno::Sequence<sal_Int16>( nSelEntries );
        for ( sal_Int32 n = 0; n < nSelEntries; ++n )
            aSeq.getArray()[n] = pBox->GetSelectEntryPos( n );
    }
    return aSeq;
}

OUString VCLXListBox::getSelectedItem() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aItem;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        aItem = pBox->GetSelectEntry();
    return aItem;
}

::com::sun::star::uno::Sequence< OUString> VCLXListBox::getSelectedItems() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Sequence< OUString> aSeq;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
    {
        const sal_Int32 nSelEntries = pBox->GetSelectEntryCount();
        aSeq = ::com::sun::star::uno::Sequence< OUString>( nSelEntries );
        for ( sal_Int32 n = 0; n < nSelEntries; ++n )
            aSeq.getArray()[n] = pBox->GetSelectEntry( n );
    }
    return aSeq;
}

void VCLXListBox::selectItemPos( sal_Int16 nPos, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox && ( pBox->IsEntryPosSelected( nPos ) != bool(bSelect) ) )
    {
        pBox->SelectEntryPos( nPos, bSelect );

        // VCL doesn't call select handler after API call.
        // ImplCallItemListeners();

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( true );
        pBox->Select();
        SetSynthesizingVCLEvent( false );
    }
}

void VCLXListBox::selectItemsPos( const ::com::sun::star::uno::Sequence<sal_Int16>& aPositions, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
    {
        bool bChanged = false;
        for ( auto n = aPositions.getLength(); n; )
        {
            const auto nPos = aPositions.getConstArray()[--n];
            if ( pBox->IsEntryPosSelected( nPos ) != bool(bSelect) )
            {
                pBox->SelectEntryPos( nPos, bSelect );
                bChanged = true;
            }
        }

        if ( bChanged )
        {
            // VCL doesn't call select handler after API call.
            // ImplCallItemListeners();

            // #107218# Call same listeners like VCL would do after user interaction
            SetSynthesizingVCLEvent( true );
            pBox->Select();
            SetSynthesizingVCLEvent( false );
        }
    }
}

void VCLXListBox::selectItem( const OUString& rItemText, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
    {
        OUString aItemText( rItemText );
        selectItemPos( pBox->GetEntryPos( aItemText ), bSelect );
    }
}

void VCLXListBox::setDropDownLineCount( sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        pBox->SetDropDownLineCount( nLines );
}

sal_Int16 VCLXListBox::getDropDownLineCount() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    sal_Int16 nLines = 0;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        nLines = pBox->GetDropDownLineCount();
    return nLines;
}

sal_Bool VCLXListBox::isMutipleMode() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bMulti = false;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        bMulti = pBox->IsMultiSelectionEnabled();
    return bMulti;
}

void VCLXListBox::setMultipleMode( sal_Bool bMulti ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        pBox->EnableMultiSelection( bMulti );
}

void VCLXListBox::makeVisible( sal_Int16 nEntry ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        pBox->SetTopEntry( nEntry );
}

void VCLXListBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
        // since we call listeners below, there is a potential that we will be destroyed
        // in during the listener call. To prevent the resulting crashs, we keep us
        // alive as long as we're here

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_LISTBOX_SELECT:
        {
            VclPtr< ListBox > pListBox = GetAs< ListBox >();
            if( pListBox )
            {
                bool bDropDown = ( pListBox->GetStyle() & WB_DROPDOWN ) != 0;
                if ( bDropDown && !IsSynthesizingVCLEvent() && maActionListeners.getLength() )
                {
                    // Call ActionListener on DropDown event
                    ::com::sun::star::awt::ActionEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                    aEvent.ActionCommand = pListBox->GetSelectEntry();
                    maActionListeners.actionPerformed( aEvent );
                }

                if ( maItemListeners.getLength() )
                {
                    ImplCallItemListeners();
                }
            }
        }
        break;

        case VCLEVENT_LISTBOX_DOUBLECLICK:
            if ( GetWindow() && maActionListeners.getLength() )
            {
                ::com::sun::star::awt::ActionEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                aEvent.ActionCommand = GetAs<ListBox>()->GetSelectEntry();
                maActionListeners.actionPerformed( aEvent );
            }
            break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXListBox::CreateAccessibleContext()
{
    SolarMutexGuard aGuard;

    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXListBox::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( pListBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_ITEM_SEPARATOR_POS:
            {
                sal_Int16 nSeparatorPos(0);
                if ( Value >>= nSeparatorPos )
                    pListBox->SetSeparatorPos( nSeparatorPos );
            }
            break;
            case BASEPROPERTY_READONLY:
            {
                bool b = bool();
                if ( Value >>= b )
                     pListBox->SetReadOnly( b);
            }
            break;
            case BASEPROPERTY_MULTISELECTION:
            {
                bool b = bool();
                if ( Value >>= b )
                     pListBox->EnableMultiSelection( b );
            }
            break;
            case BASEPROPERTY_MULTISELECTION_SIMPLEMODE:
                ::toolkit::adjustBooleanWindowStyle( Value, pListBox, WB_SIMPLEMODE, false );
                break;
            case BASEPROPERTY_LINECOUNT:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                     pListBox->SetDropDownLineCount( n );
            }
            break;
            case BASEPROPERTY_STRINGITEMLIST:
            {
                ::com::sun::star::uno::Sequence< OUString> aItems;
                if ( Value >>= aItems )
                {
                    pListBox->Clear();
                    addItems( aItems, 0 );
                }
            }
            break;
            case BASEPROPERTY_SELECTEDITEMS:
            {
                ::com::sun::star::uno::Sequence<sal_Int16> aItems;
                if ( Value >>= aItems )
                {
                    for ( auto n = pListBox->GetEntryCount(); n; )
                        pListBox->SelectEntryPos( --n, false );

                    if ( aItems.getLength() )
                        selectItemsPos( aItems, sal_True );
                    else
                        pListBox->SetNoSelection();

                    if ( !pListBox->GetSelectEntryCount() )
                        pListBox->SetTopEntry( 0 );
                }
            }
            break;
            default:
            {
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXListBox::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ::com::sun::star::uno::Any aProp;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( pListBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_ITEM_SEPARATOR_POS:
                aProp <<= sal_Int16( pListBox->GetSeparatorPos() );
                break;
            case BASEPROPERTY_READONLY:
            {
                 aProp <<= pListBox->IsReadOnly();
            }
            break;
            case BASEPROPERTY_MULTISELECTION:
            {
                 aProp <<= pListBox->IsMultiSelectionEnabled();
            }
            break;
            case BASEPROPERTY_MULTISELECTION_SIMPLEMODE:
            {
                aProp <<= ( ( pListBox->GetStyle() & WB_SIMPLEMODE ) == 0 );
            }
            break;
            case BASEPROPERTY_LINECOUNT:
            {
                 aProp <<= (sal_Int16) pListBox->GetDropDownLineCount();
            }
            break;
            case BASEPROPERTY_STRINGITEMLIST:
            {
                const sal_Int32 nItems = pListBox->GetEntryCount();
                ::com::sun::star::uno::Sequence< OUString> aSeq( nItems );
                OUString* pStrings = aSeq.getArray();
                for ( sal_Int32 n = 0; n < nItems; ++n )
                    pStrings[n] = pListBox->GetEntry( n );
                aProp <<= aSeq;

            }
            break;
            default:
            {
                aProp <<= VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

::com::sun::star::awt::Size VCLXListBox::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    Size aSz;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( pListBox )
        aSz = pListBox->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXListBox::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    Size aSz;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( pListBox )
    {
        aSz = pListBox->CalcMinimumSize();
        if ( pListBox->GetStyle() & WB_DROPDOWN )
            aSz.Height() += 4;
    }
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXListBox::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    Size aSz = VCLSize(rNewSize);
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( pListBox )
        aSz = pListBox->CalcAdjustedSize( aSz );
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXListBox::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    Size aSz;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( pListBox )
        aSz = pListBox->CalcBlockSize( nCols, nLines );
    return AWTSize(aSz);
}

void VCLXListBox::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    nCols = nLines = 0;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( pListBox )
    {
        sal_uInt16 nC, nL;
        pListBox->GetMaxVisColumnsAndLines( nC, nL );
        nCols = nC;
        nLines = nL;
    }
}

void VCLXListBox::ImplCallItemListeners()
{
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( pListBox && maItemListeners.getLength() )
    {
        ::com::sun::star::awt::ItemEvent aEvent;
        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
        aEvent.Highlighted = 0;

        // Set to 0xFFFF on multiple selection, selected entry ID otherwise
        aEvent.Selected = (pListBox->GetSelectEntryCount() == 1 ) ? pListBox->GetSelectEntryPos() : 0xFFFF;

        maItemListeners.itemStateChanged( aEvent );
    }
}
namespace
{
     Image lcl_getImageFromURL( const OUString& i_rImageURL )
     {
         if ( i_rImageURL.isEmpty() )
             return Image();

        try
        {
             Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
             Reference< XGraphicProvider > xProvider(graphic::GraphicProvider::create(xContext));
             ::comphelper::NamedValueCollection aMediaProperties;
             aMediaProperties.put( "URL", i_rImageURL );
             Reference< XGraphic > xGraphic = xProvider->queryGraphic( aMediaProperties.getPropertyValues() );
             return Image( xGraphic );
         }
         catch( const uno::Exception& )
         {
             DBG_UNHANDLED_EXCEPTION();
         }
         return Image();
     }
}
void SAL_CALL VCLXListBox::listItemInserted( const ItemListEvent& i_rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();

    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemInserted: no ListBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition <= sal_Int32( pListBox->GetEntryCount() ) ),
        "VCLXListBox::listItemInserted: illegal (inconsistent) item position!" );
    pListBox->InsertEntry(
        i_rEvent.ItemText.IsPresent ? i_rEvent.ItemText.Value : OUString(),
        i_rEvent.ItemImageURL.IsPresent ? TkResMgr::getImageFromURL( i_rEvent.ItemImageURL.Value ) : Image(),
        i_rEvent.ItemPosition );
}

void SAL_CALL VCLXListBox::listItemRemoved( const ItemListEvent& i_rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();

    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemRemoved: no ListBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition < sal_Int32( pListBox->GetEntryCount() ) ),
        "VCLXListBox::listItemRemoved: illegal (inconsistent) item position!" );

    pListBox->RemoveEntry( i_rEvent.ItemPosition );
}

void SAL_CALL VCLXListBox::listItemModified( const ItemListEvent& i_rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();

    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemModified: no ListBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition < sal_Int32( pListBox->GetEntryCount() ) ),
        "VCLXListBox::listItemModified: illegal (inconsistent) item position!" );

    // VCL's ListBox does not support changing an entry's text or image, so remove and re-insert

    const OUString sNewText = i_rEvent.ItemText.IsPresent ? i_rEvent.ItemText.Value : OUString( pListBox->GetEntry( i_rEvent.ItemPosition ) );
    const Image aNewImage( i_rEvent.ItemImageURL.IsPresent ? TkResMgr::getImageFromURL( i_rEvent.ItemImageURL.Value ) : pListBox->GetEntryImage( i_rEvent.ItemPosition  ) );

    pListBox->RemoveEntry( i_rEvent.ItemPosition );
    pListBox->InsertEntry( sNewText, aNewImage, i_rEvent.ItemPosition );
}

void SAL_CALL VCLXListBox::allItemsRemoved( const EventObject& i_rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemModified: no ListBox?!" );

    pListBox->Clear();

    (void)i_rEvent;
}

void SAL_CALL VCLXListBox::itemListChanged( const EventObject& i_rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemModified: no ListBox?!" );

    pListBox->Clear();

    uno::Reference< beans::XPropertySet > xPropSet( i_rEvent.Source, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySetInfo > xPSI( xPropSet->getPropertySetInfo(), uno::UNO_QUERY_THROW );
    uno::Reference< resource::XStringResourceResolver > xStringResourceResolver;
    if ( xPSI->hasPropertyByName("ResourceResolver") )
    {
        xStringResourceResolver.set(
            xPropSet->getPropertyValue("ResourceResolver"),
            uno::UNO_QUERY
        );
    }


    Reference< XItemList > xItemList( i_rEvent.Source, uno::UNO_QUERY_THROW );
    uno::Sequence< beans::Pair< OUString, OUString > > aItems = xItemList->getAllItems();
    for ( sal_Int32 i=0; i<aItems.getLength(); ++i )
    {
        OUString aLocalizationKey( aItems[i].First );
        if ( xStringResourceResolver.is() && aLocalizationKey.startsWith("&") )
        {
            aLocalizationKey = xStringResourceResolver->resolveString(aLocalizationKey.copy( 1 ));
        }
        pListBox->InsertEntry( aLocalizationKey, lcl_getImageFromURL( aItems[i].Second ) );
    }
}

void SAL_CALL VCLXListBox::disposing( const EventObject& i_rEvent ) throw (RuntimeException, std::exception)
{
    // just disambiguate
    VCLXWindow::disposing( i_rEvent );
}


//  class VCLXMessageBox


void VCLXMessageBox::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    VCLXTopWindow::ImplGetPropertyIds( rIds );
}

VCLXMessageBox::VCLXMessageBox()
{
}

VCLXMessageBox::~VCLXMessageBox()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXMessageBox::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XMessageBox* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXTopWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXMessageBox )
    cppu::UnoType<com::sun::star::awt::XMessageBox>::get(),
    VCLXTopWindow::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXMessageBox::setCaptionText( const OUString& rText ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rText );
}

OUString VCLXMessageBox::getCaptionText() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aText;
    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

void VCLXMessageBox::setMessageText( const OUString& rText ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< MessBox > pBox = GetAs< MessBox >();
    if ( pBox )
        pBox->SetMessText( rText );
}

OUString VCLXMessageBox::getMessageText() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aText;
    VclPtr< MessBox > pBox = GetAs< MessBox >();
    if ( pBox )
        aText = pBox->GetMessText();
    return aText;
}

sal_Int16 VCLXMessageBox::execute() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< MessBox > pBox = GetAs< MessBox >();
    return pBox ? pBox->Execute() : 0;
}

::com::sun::star::awt::Size SAL_CALL VCLXMessageBox::getMinimumSize() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ::com::sun::star::awt::Size( 250, 100 );
}


//  class VCLXDialog

void VCLXDialog::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    VCLXTopWindow::ImplGetPropertyIds( rIds );
}

VCLXDialog::VCLXDialog()
{
    OSL_TRACE("XDialog created");
}

VCLXDialog::~VCLXDialog()
{
    OSL_TRACE ("%s", __FUNCTION__);
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXDialog::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XDialog2* >(this)),
                                        (static_cast< ::com::sun::star::awt::XDialog* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXTopWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXDialog )
    cppu::UnoType<com::sun::star::awt::XDialog2>::get(),
    cppu::UnoType<com::sun::star::awt::XDialog>::get(),
    VCLXTopWindow::getTypes()
IMPL_XTYPEPROVIDER_END

void SAL_CALL VCLXDialog::endDialog( ::sal_Int32 i_result ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr<Dialog> pDialog = GetAsDynamic< Dialog >();
    if ( pDialog )
        pDialog->EndDialog( i_result );
}

void SAL_CALL VCLXDialog::setHelpId( const OUString& rId ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetHelpId( OUStringToOString( rId, RTL_TEXTENCODING_UTF8 ) );
}

void VCLXDialog::setTitle( const OUString& Title ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( Title );
}

OUString VCLXDialog::getTitle() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aTitle;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        aTitle = pWindow->GetText();
    return aTitle;
}

sal_Int16 VCLXDialog::execute() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    sal_Int16 nRet = 0;
    if ( GetWindow() )
    {
        VclPtr< Dialog > pDlg = GetAs< Dialog >();
        vcl::Window* pParent = pDlg->GetWindow( GetWindowType::ParentOverlap );
        vcl::Window* pOldParent = NULL;
        vcl::Window* pSetParent = NULL;
        if ( pParent && !pParent->IsReallyVisible() )
        {
            pOldParent = pDlg->GetParent();
            vcl::Window* pFrame = pDlg->GetWindow( GetWindowType::Frame );
            if( pFrame != pDlg )
            {
                pDlg->SetParent( pFrame );
                pSetParent = pFrame;
            }
        }

        nRet = pDlg->Execute();

        // set the parent back only in case no new parent was set from outside
        // in other words, revert only own changes
        if ( pOldParent && pDlg->GetParent() == pSetParent )
            pDlg->SetParent( pOldParent );
    }
    return nRet;
}

void VCLXDialog::endExecute() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    endDialog(0);
}

void SAL_CALL VCLXDialog::draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Size aSize = pDev->PixelToLogic( pWindow->GetSizePixel() );
        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );

        pWindow->Draw( pDev, aPos, aSize, DrawFlags::NoControls );
    }
}

::com::sun::star::awt::DeviceInfo VCLXDialog::getInfo() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::awt::DeviceInfo aInfo = VCLXDevice::getInfo();

    SolarMutexGuard aGuard;
    VclPtr< Dialog > pDlg = GetAs< Dialog >();
    if ( pDlg )
        pDlg->GetDrawWindowBorder( aInfo.LeftInset, aInfo.TopInset, aInfo.RightInset, aInfo.BottomInset );

    return aInfo;
}

void SAL_CALL VCLXDialog::setProperty(
    const OUString& PropertyName,
    const ::com::sun::star::uno::Any& Value )
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< Dialog > pDialog = GetAs< Dialog >();
    if ( pDialog )
    {
        bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_GRAPHIC:
            {
                Reference< XGraphic > xGraphic;
                if (( Value >>= xGraphic ) && xGraphic.is() )
                {
                    Image aImage( xGraphic );

                    Wallpaper aWallpaper( aImage.GetBitmapEx());
                    aWallpaper.SetStyle( WallpaperStyle::Scale );
                    pDialog->SetBackground( aWallpaper );
                }
                else if ( bVoid || !xGraphic.is() )
                {
                    Color aColor = pDialog->GetControlBackground().GetColor();
                    if ( aColor == COL_AUTO )
                        aColor = pDialog->GetSettings().GetStyleSettings().GetDialogColor();

                    Wallpaper aWallpaper( aColor );
                    pDialog->SetBackground( aWallpaper );
                }
            }
            break;

            default:
            {
                VCLXContainer::setProperty( PropertyName, Value );
            }
        }
    }
}



//  class VCLXTabPage

VCLXMultiPage::VCLXMultiPage() : maTabListeners( *this ), mTabId( 1 )
{
    OSL_TRACE("VCLXMultiPage::VCLXMultiPage()" );
}

void VCLXMultiPage::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_MULTIPAGEVALUE,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_GRAPHIC,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_IMAGEALIGN,
                     BASEPROPERTY_IMAGEPOSITION,
                     BASEPROPERTY_IMAGEURL,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_FOCUSONCLICK,
                     0);
    VCLXContainer::ImplGetPropertyIds( rIds );
}

VCLXMultiPage::~VCLXMultiPage()
{
}
void SAL_CALL VCLXMultiPage::dispose() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maTabListeners.disposeAndClear( aObj );
    VCLXContainer::dispose();
}
::com::sun::star::uno::Any SAL_CALL VCLXMultiPage::queryInterface(const ::com::sun::star::uno::Type & rType )
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    uno::Any aRet = ::cppu::queryInterface( rType, static_cast< awt::XSimpleTabController*>( this ) );

    return ( aRet.hasValue() ? aRet : VCLXContainer::queryInterface( rType ) );
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXMultiPage )
    VCLXContainer::getTypes()
IMPL_XTYPEPROVIDER_END

// ::com::sun::star::awt::XView
void SAL_CALL VCLXMultiPage::draw( sal_Int32 nX, sal_Int32 nY )
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Size aSize = pDev->PixelToLogic( pWindow->GetSizePixel() );
        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );

        pWindow->Draw( pDev, aPos, aSize, DrawFlags::NoControls );
    }
}

// ::com::sun::star::awt::XDevice,
::com::sun::star::awt::DeviceInfo SAL_CALL VCLXMultiPage::getInfo()
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::awt::DeviceInfo aInfo = VCLXDevice::getInfo();
    return aInfo;
}

uno::Any SAL_CALL VCLXMultiPage::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OSL_TRACE(" **** VCLXMultiPage::getProperty( %s )",
        OUStringToOString( PropertyName,
        RTL_TEXTENCODING_UTF8 ).getStr() );
    ::com::sun::star::uno::Any aProp;
    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {

        case BASEPROPERTY_MULTIPAGEVALUE:
        {
            aProp <<= getActiveTabID();
        }
        break;
        default:
            aProp <<= VCLXContainer::getProperty( PropertyName );
    }
    return aProp;
}

void SAL_CALL VCLXMultiPage::setProperty(
    const OUString& PropertyName,
    const ::com::sun::star::uno::Any& Value )
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OSL_TRACE(" **** VCLXMultiPage::setProperty( %s )", OUStringToOString( PropertyName, RTL_TEXTENCODING_UTF8 ).getStr() );

    VclPtr< TabControl > pTabControl = GetAs< TabControl >();
    if ( pTabControl )
    {
        bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_MULTIPAGEVALUE:
            {
                OSL_TRACE("***MULTIPAGE VALUE");
                sal_Int32 nId(0);
                Value >>= nId;
                // when the multipage is created we attempt to set the activepage
                // but no pages created
                if ( nId && nId <= getWindows().getLength() )
                    activateTab( nId );
                break;
            }
            case BASEPROPERTY_GRAPHIC:
            {
                Reference< XGraphic > xGraphic;
                if (( Value >>= xGraphic ) && xGraphic.is() )
                {
                    Image aImage( xGraphic );

                    Wallpaper aWallpaper( aImage.GetBitmapEx());
                    aWallpaper.SetStyle( WallpaperStyle::Scale );
                    pTabControl->SetBackground( aWallpaper );
                }
                else if ( bVoid || !xGraphic.is() )
                {
                    Color aColor = pTabControl->GetControlBackground().GetColor();
                    if ( aColor == COL_AUTO )
                        aColor = pTabControl->GetSettings().GetStyleSettings().GetDialogColor();

                    Wallpaper aWallpaper( aColor );
                    pTabControl->SetBackground( aWallpaper );
                }
            }
            break;

            default:
            {
                VCLXContainer::setProperty( PropertyName, Value );
            }
        }
    }
}

TabControl *VCLXMultiPage::getTabControl() const throw (uno::RuntimeException)
{
    VclPtr<TabControl> pTabControl = GetAsDynamic< TabControl >();
    if ( pTabControl )
        return pTabControl;
    throw uno::RuntimeException();
}
sal_Int32 SAL_CALL VCLXMultiPage::insertTab() throw (uno::RuntimeException, std::exception)
{
    TabControl *pTabControl = getTabControl();
    VclPtrInstance<TabPage> pTab( pTabControl );
    OUString title ("");
    return static_cast< sal_Int32 >( insertTab( pTab, title ) );
}

sal_uInt16 VCLXMultiPage::insertTab( TabPage* pPage, OUString& sTitle )
{
    TabControl *pTabControl = getTabControl();
    sal_uInt16 id = sal::static_int_cast< sal_uInt16 >( mTabId++ );
    pTabControl->InsertPage( id, sTitle );
    pTabControl->SetTabPage( id, pPage );
    return id;
}

void SAL_CALL VCLXMultiPage::removeTab( sal_Int32 ID ) throw (uno::RuntimeException, lang::IndexOutOfBoundsException, std::exception)
{
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == NULL )
        throw lang::IndexOutOfBoundsException();
    pTabControl->RemovePage( sal::static_int_cast< sal_uInt16 >( ID ) );
}

void SAL_CALL VCLXMultiPage::activateTab( sal_Int32 ID ) throw (uno::RuntimeException, lang::IndexOutOfBoundsException, std::exception)
{
    TabControl *pTabControl = getTabControl();
    SAL_INFO(
        "toolkit",
        "Attempting to activate tab " << ID << ", active tab is "
            << getActiveTabID() << ", numtabs is " << getWindows().getLength());
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == NULL )
        throw lang::IndexOutOfBoundsException();
    pTabControl->SelectTabPage( sal::static_int_cast< sal_uInt16 >( ID ) );
}

sal_Int32 SAL_CALL VCLXMultiPage::getActiveTabID() throw (uno::RuntimeException, std::exception)
{
    return getTabControl()->GetCurPageId( );
}

void SAL_CALL VCLXMultiPage::addTabListener( const uno::Reference< awt::XTabListener >& xListener ) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maTabListeners.addInterface( xListener );
}

void SAL_CALL VCLXMultiPage::removeTabListener( const uno::Reference< awt::XTabListener >& xListener ) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maTabListeners.addInterface( xListener );
}

void SAL_CALL VCLXMultiPage::setTabProps( sal_Int32 ID, const uno::Sequence< beans::NamedValue >& Properties ) throw (uno::RuntimeException, lang::IndexOutOfBoundsException, std::exception)
{
    SolarMutexGuard aGuard;
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == NULL )
        throw lang::IndexOutOfBoundsException();

    for (sal_Int32 i = 0; i < Properties.getLength(); ++i)
    {
        const OUString &name = Properties[i].Name;
        const uno::Any &value = Properties[i].Value;

        if (name == "Title")
        {
            OUString title = value.get<OUString>();
            pTabControl->SetPageText( sal::static_int_cast< sal_uInt16 >( ID ), title );
        }
    }
}

uno::Sequence< beans::NamedValue > SAL_CALL VCLXMultiPage::getTabProps( sal_Int32 ID )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == NULL )
        throw lang::IndexOutOfBoundsException();

#define ADD_PROP( seq, i, name, val ) {                                \
        beans::NamedValue value;                                                  \
        value.Name = name; \
        value.Value = uno::makeAny( val );                                      \
        seq[i] = value;                                                    \
    }

    uno::Sequence< beans::NamedValue > props( 2 );
    ADD_PROP( props, 0, "Title", OUString( pTabControl->GetPageText( sal::static_int_cast< sal_uInt16 >( ID ) ) ) );
    ADD_PROP( props, 1, "Position", pTabControl->GetPagePos( sal::static_int_cast< sal_uInt16 >( ID ) ) );
#undef ADD_PROP
    return props;
}
void VCLXMultiPage::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_TABPAGE_DEACTIVATE:
        {
            sal_uLong nPageID = reinterpret_cast<sal_uLong>( rVclWindowEvent.GetData() );
            maTabListeners.deactivated( nPageID );
            break;

        }
        case VCLEVENT_TABPAGE_ACTIVATE:
        {
            sal_uLong nPageID = reinterpret_cast<sal_uLong>( rVclWindowEvent.GetData() );
            maTabListeners.activated( nPageID );
            break;
        }
        default:
            VCLXContainer::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}


//  class VCLXTabPage

VCLXTabPage::VCLXTabPage()
{
}

void VCLXTabPage::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_GRAPHIC,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_IMAGEALIGN,
                     BASEPROPERTY_IMAGEPOSITION,
                     BASEPROPERTY_IMAGEURL,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_FOCUSONCLICK,
                     0);
    VCLXContainer::ImplGetPropertyIds( rIds );
}

VCLXTabPage::~VCLXTabPage()
{
}

::com::sun::star::uno::Any SAL_CALL VCLXTabPage::queryInterface(const ::com::sun::star::uno::Type & rType )
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return VCLXContainer::queryInterface( rType );
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXTabPage )
    VCLXContainer::getTypes()
IMPL_XTYPEPROVIDER_END

// ::com::sun::star::awt::XView
void SAL_CALL VCLXTabPage::draw( sal_Int32 nX, sal_Int32 nY )
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Size aSize = pDev->PixelToLogic( pWindow->GetSizePixel() );
        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );

        pWindow->Draw( pDev, aPos, aSize, DrawFlags::NoControls );
    }
}

// ::com::sun::star::awt::XDevice,
::com::sun::star::awt::DeviceInfo SAL_CALL VCLXTabPage::getInfo()
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::awt::DeviceInfo aInfo = VCLXDevice::getInfo();
    return aInfo;
}

void SAL_CALL VCLXTabPage::setProperty(
    const OUString& PropertyName,
    const ::com::sun::star::uno::Any& Value )
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< TabPage > pTabPage = GetAs< TabPage >();
    if ( pTabPage )
    {
        bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_GRAPHIC:
            {
                Reference< XGraphic > xGraphic;
                if (( Value >>= xGraphic ) && xGraphic.is() )
                {
                    Image aImage( xGraphic );

                    Wallpaper aWallpaper( aImage.GetBitmapEx());
                    aWallpaper.SetStyle( WallpaperStyle::Scale );
                    pTabPage->SetBackground( aWallpaper );
                }
                else if ( bVoid || !xGraphic.is() )
                {
                    Color aColor = pTabPage->GetControlBackground().GetColor();
                    if ( aColor == COL_AUTO )
                        aColor = pTabPage->GetSettings().GetStyleSettings().GetDialogColor();

                    Wallpaper aWallpaper( aColor );
                    pTabPage->SetBackground( aWallpaper );
                }
            }
            break;
            case BASEPROPERTY_TITLE:
                {
                    OUString sTitle;
                    if ( Value >>= sTitle )
                    {
                        pTabPage->SetText(sTitle);
                    }
                }
                break;

            default:
            {
                VCLXContainer::setProperty( PropertyName, Value );
            }
        }
    }
}

TabPage *VCLXTabPage::getTabPage() const throw (uno::RuntimeException)
{
    VclPtr< TabPage > pTabPage = GetAsDynamic< TabPage >();
    if ( pTabPage )
        return pTabPage;
    throw uno::RuntimeException();
}


//  class VCLXFixedHyperlink


VCLXFixedHyperlink::VCLXFixedHyperlink() :

    maActionListeners( *this )

{
}

VCLXFixedHyperlink::~VCLXFixedHyperlink()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXFixedHyperlink::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XFixedHyperlink* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

void VCLXFixedHyperlink::dispose() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
        SolarMutexGuard aGuard;

        ::com::sun::star::lang::EventObject aObj;
        aObj.Source = static_cast<cppu::OWeakObject*>(this);
        maActionListeners.disposeAndClear( aObj );
        VCLXWindow::dispose();
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXFixedHyperlink )
    cppu::UnoType<com::sun::star::awt::XFixedHyperlink>::get(),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXFixedHyperlink::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_BUTTON_CLICK:
        {
            if ( maActionListeners.getLength() )
            {
                ::com::sun::star::awt::ActionEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                maActionListeners.actionPerformed( aEvent );
            }
            else
            {
                // open the URL
                OUString sURL;
                VclPtr< FixedHyperlink > pBase = GetAs< FixedHyperlink >();
                if ( pBase )
                    sURL = pBase->GetURL();
                Reference< ::com::sun::star::system::XSystemShellExecute > xSystemShellExecute( ::com::sun::star::system::SystemShellExecute::create(
                    ::comphelper::getProcessComponentContext() ) );
                if ( !sURL.isEmpty() )
                {
                    try
                    {
                        // start browser
                        xSystemShellExecute->execute(
                            sURL, OUString(), ::com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY );
                    }
                    catch( uno::Exception& )
                    {
                    }
                }
            }
        }
        //fall-through
        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXFixedHyperlink::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXFixedHyperlink::setText( const OUString& Text ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< FixedHyperlink > pBase = GetAs< FixedHyperlink >();
    if (pBase)
        pBase->SetText(Text);
}

OUString VCLXFixedHyperlink::getText() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

void VCLXFixedHyperlink::setURL( const OUString& URL ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< FixedHyperlink > pBase = GetAs< FixedHyperlink >();
    if ( pBase )
        pBase->SetURL( URL );
}

OUString VCLXFixedHyperlink::getURL(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< FixedHyperlink > pBase = GetAs< FixedHyperlink >();
    if ( pBase )
        aText = pBase->GetURL();
    return aText;
}

void VCLXFixedHyperlink::setAlignment( short nAlign ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nNewBits = 0;
        if ( nAlign == ::com::sun::star::awt::TextAlign::LEFT )
            nNewBits = WB_LEFT;
        else if ( nAlign == ::com::sun::star::awt::TextAlign::CENTER )
            nNewBits = WB_CENTER;
        else
            nNewBits = WB_RIGHT;

        WinBits nStyle = pWindow->GetStyle();
        nStyle &= ~(WB_LEFT|WB_CENTER|WB_RIGHT);
        pWindow->SetStyle( nStyle | nNewBits );
    }
}

short VCLXFixedHyperlink::getAlignment() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    short nAlign = 0;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        if ( nStyle & WB_LEFT )
            nAlign = ::com::sun::star::awt::TextAlign::LEFT;
        else if ( nStyle & WB_CENTER )
            nAlign = ::com::sun::star::awt::TextAlign::CENTER;
        else
            nAlign = ::com::sun::star::awt::TextAlign::RIGHT;
    }
    return nAlign;
}

void VCLXFixedHyperlink::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l  )throw(::com::sun::star::uno::RuntimeException, std::exception)
{
        SolarMutexGuard aGuard;
        maActionListeners.addInterface( l );
}

void VCLXFixedHyperlink::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
        SolarMutexGuard aGuard;
        maActionListeners.removeInterface( l );
}

::com::sun::star::awt::Size VCLXFixedHyperlink::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    Size aSz;
    VclPtr< FixedText > pFixedText = GetAs< FixedText >();
    if ( pFixedText )
        aSz = pFixedText->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXFixedHyperlink::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXFixedHyperlink::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::Size aSz = rNewSize;
    ::com::sun::star::awt::Size aMinSz = getMinimumSize();
    if ( aSz.Height != aMinSz.Height )
        aSz.Height = aMinSz.Height;

    return aSz;
}

void VCLXFixedHyperlink::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< FixedHyperlink > pBase = GetAs< FixedHyperlink >();
    if ( pBase )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LABEL:
            {
                OUString sNewLabel;
                if ( Value >>= sNewLabel )
                    pBase->SetText(sNewLabel);
                break;
            }

            case BASEPROPERTY_URL:
            {
                OUString sNewURL;
                if ( Value >>= sNewURL )
                    pBase->SetURL( sNewURL );
                break;
            }

            default:
            {
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXFixedHyperlink::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    VclPtr< FixedHyperlink > pBase = GetAs< FixedHyperlink >();
    if ( pBase )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_URL:
            {
                aProp = makeAny( OUString( pBase->GetURL() ) );
                break;
            }

            default:
            {
                aProp <<= VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXFixedHyperlink::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_LABEL,
                     BASEPROPERTY_MULTILINE,
                     BASEPROPERTY_NOLABEL,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_VERTICALALIGN,
                     BASEPROPERTY_URL,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds );
}


//  class VCLXFixedText

void VCLXFixedText::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_LABEL,
                     BASEPROPERTY_MULTILINE,
                     BASEPROPERTY_NOLABEL,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_VERTICALALIGN,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     BASEPROPERTY_REFERENCE_DEVICE,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds );
}

VCLXFixedText::VCLXFixedText()
{
}

VCLXFixedText::~VCLXFixedText()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXFixedText::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XFixedText* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXFixedText )
    cppu::UnoType<com::sun::star::awt::XFixedText>::get(),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXFixedText::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXFixedText::setText( const OUString& Text ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( Text );
}

OUString VCLXFixedText::getText() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

void VCLXFixedText::setAlignment( short nAlign ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nNewBits = 0;
        if ( nAlign == ::com::sun::star::awt::TextAlign::LEFT )
            nNewBits = WB_LEFT;
        else if ( nAlign == ::com::sun::star::awt::TextAlign::CENTER )
            nNewBits = WB_CENTER;
        else
            nNewBits = WB_RIGHT;

        WinBits nStyle = pWindow->GetStyle();
        nStyle &= ~(WB_LEFT|WB_CENTER|WB_RIGHT);
        pWindow->SetStyle( nStyle | nNewBits );
    }
}

short VCLXFixedText::getAlignment() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    short nAlign = 0;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        if ( nStyle & WB_LEFT )
            nAlign = ::com::sun::star::awt::TextAlign::LEFT;
        else if ( nStyle & WB_CENTER )
            nAlign = ::com::sun::star::awt::TextAlign::CENTER;
        else
            nAlign = ::com::sun::star::awt::TextAlign::RIGHT;
    }
    return nAlign;
}

::com::sun::star::awt::Size VCLXFixedText::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< FixedText > pFixedText = GetAs< FixedText >();
    if ( pFixedText )
        aSz = pFixedText->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXFixedText::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXFixedText::calcAdjustedSize( const ::com::sun::star::awt::Size& rMaxSize ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aAdjustedSize( VCLUnoHelper::ConvertToVCLSize( rMaxSize ) );
    VclPtr< FixedText > pFixedText = GetAs< FixedText >();
    if ( pFixedText )
        aAdjustedSize = pFixedText->CalcMinimumSize( rMaxSize.Width );
    return VCLUnoHelper::ConvertToAWTSize( aAdjustedSize );
}


//  class VCLXScrollBar

void VCLXScrollBar::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BLOCKINCREMENT,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_LINEINCREMENT,
                     BASEPROPERTY_LIVE_SCROLL,
                     BASEPROPERTY_ORIENTATION,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_REPEAT_DELAY,
                     BASEPROPERTY_SCROLLVALUE,
                     BASEPROPERTY_SCROLLVALUE_MAX,
                     BASEPROPERTY_SCROLLVALUE_MIN,
                     BASEPROPERTY_SYMBOL_COLOR,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_VISIBLESIZE,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds );
}

VCLXScrollBar::VCLXScrollBar() : maAdjustmentListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXScrollBar::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XScrollBar* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXScrollBar )
    cppu::UnoType<com::sun::star::awt::XScrollBar>::get(),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXScrollBar::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

// ::com::sun::star::lang::XComponent
void VCLXScrollBar::dispose() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maAdjustmentListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

// ::com::sun::star::awt::XScrollbar
void VCLXScrollBar::addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maAdjustmentListeners.addInterface( l );
}

void VCLXScrollBar::removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maAdjustmentListeners.removeInterface( l );
}

void VCLXScrollBar::setValue( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
        pScrollBar->DoScroll( n );
}

void VCLXScrollBar::setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
    {
        pScrollBar->SetVisibleSize( nVisible );
        pScrollBar->SetRangeMax( nMax );
        pScrollBar->DoScroll( nValue );
    }
}

sal_Int32 VCLXScrollBar::getValue() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    return pScrollBar ? pScrollBar->GetThumbPos() : 0;
}

void VCLXScrollBar::setMaximum( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
        pScrollBar->SetRangeMax( n );
}

sal_Int32 VCLXScrollBar::getMaximum() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    return pScrollBar ? pScrollBar->GetRangeMax() : 0;
}

void VCLXScrollBar::setMinimum( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
        pScrollBar->SetRangeMin( n );
}

sal_Int32 VCLXScrollBar::getMinimum() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    return pScrollBar ? pScrollBar->GetRangeMin() : 0;
}

void VCLXScrollBar::setLineIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
        pScrollBar->SetLineSize( n );
}

sal_Int32 VCLXScrollBar::getLineIncrement() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    return pScrollBar ? pScrollBar->GetLineSize() : 0;
}

void VCLXScrollBar::setBlockIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
        pScrollBar->SetPageSize( n );
}

sal_Int32 VCLXScrollBar::getBlockIncrement() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    return pScrollBar ? pScrollBar->GetPageSize() : 0;
}

void VCLXScrollBar::setVisibleSize( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
        pScrollBar->SetVisibleSize( n );
}

sal_Int32 VCLXScrollBar::getVisibleSize() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    return pScrollBar ? pScrollBar->GetVisibleSize() : 0;
}

void VCLXScrollBar::setOrientation( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        nStyle &= ~(WB_HORZ|WB_VERT);
        if ( n == ::com::sun::star::awt::ScrollBarOrientation::HORIZONTAL )
            nStyle |= WB_HORZ;
        else
            nStyle |= WB_VERT;

        pWindow->SetStyle( nStyle );
        pWindow->Resize();
    }
}

sal_Int32 VCLXScrollBar::getOrientation() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    sal_Int32 n = 0;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        if ( nStyle & WB_HORZ )
            n = ::com::sun::star::awt::ScrollBarOrientation::HORIZONTAL;
        else
            n = ::com::sun::star::awt::ScrollBarOrientation::VERTICAL;
    }
    return n;

}

// ::com::sun::star::awt::VclWindowPeer
void VCLXScrollBar::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
    {
        bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LIVE_SCROLL:
            {
                bool bDo = false;
                if ( !bVoid )
                {
                    OSL_VERIFY( Value >>= bDo );
                }
                AllSettings aSettings( pScrollBar->GetSettings() );
                StyleSettings aStyle( aSettings.GetStyleSettings() );
                DragFullOptions nDragOptions = aStyle.GetDragFullOptions();
                if ( bDo )
                    nDragOptions |= DragFullOptions::Scroll;
                else
                    nDragOptions &= ~DragFullOptions::Scroll;
                aStyle.SetDragFullOptions( nDragOptions );
                aSettings.SetStyleSettings( aStyle );
                pScrollBar->SetSettings( aSettings );
            }
            break;

            case BASEPROPERTY_SCROLLVALUE:
            {
                if ( !bVoid )
                {
                    sal_Int32 n = 0;
                    if ( Value >>= n )
                        setValue( n );
                }
            }
            break;
            case BASEPROPERTY_SCROLLVALUE_MAX:
            case BASEPROPERTY_SCROLLVALUE_MIN:
            {
                if ( !bVoid )
                {
                    sal_Int32 n = 0;
                    if ( Value >>= n )
                    {
                        if ( nPropType == BASEPROPERTY_SCROLLVALUE_MAX )
                            setMaximum( n );
                        else
                            setMinimum( n );
                    }
                }
            }
            break;
            case BASEPROPERTY_LINEINCREMENT:
            {
                if ( !bVoid )
                {
                    sal_Int32 n = 0;
                    if ( Value >>= n )
                        setLineIncrement( n );
                }
            }
            break;
            case BASEPROPERTY_BLOCKINCREMENT:
            {
                if ( !bVoid )
                {
                    sal_Int32 n = 0;
                    if ( Value >>= n )
                        setBlockIncrement( n );
                }
            }
            break;
            case BASEPROPERTY_VISIBLESIZE:
            {
                if ( !bVoid )
                {
                    sal_Int32 n = 0;
                    if ( Value >>= n )
                        setVisibleSize( n );
                }
            }
            break;
            case BASEPROPERTY_ORIENTATION:
            {
                if ( !bVoid )
                {
                    sal_Int32 n = 0;
                    if ( Value >>= n )
                        setOrientation( n );
                }
            }
            break;

            case BASEPROPERTY_BACKGROUNDCOLOR:
            {
                // the default implementation of the base class doesn't work here, since our
                // interpretation for this property is slightly different
                ::toolkit::setButtonLikeFaceColor( pScrollBar, Value);
            }
            break;

            default:
            {
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXScrollBar::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );

        switch ( nPropType )
        {
            case BASEPROPERTY_LIVE_SCROLL:
            {
                aProp <<= bool( pScrollBar->GetSettings().GetStyleSettings().GetDragFullOptions() & DragFullOptions::Scroll );
            }
            break;
            case BASEPROPERTY_SCROLLVALUE:
            {
                aProp <<= (sal_Int32) getValue();
            }
            break;
            case BASEPROPERTY_SCROLLVALUE_MAX:
            {
                aProp <<= (sal_Int32) getMaximum();
            }
            break;
            case BASEPROPERTY_SCROLLVALUE_MIN:
            {
                aProp <<= (sal_Int32) getMinimum();
            }
            break;
            case BASEPROPERTY_LINEINCREMENT:
            {
                aProp <<= (sal_Int32) getLineIncrement();
            }
            break;
            case BASEPROPERTY_BLOCKINCREMENT:
            {
                aProp <<= (sal_Int32) getBlockIncrement();
            }
            break;
            case BASEPROPERTY_VISIBLESIZE:
            {
                aProp <<= (sal_Int32) getVisibleSize();
            }
            break;
            case BASEPROPERTY_ORIENTATION:
            {
                aProp <<= (sal_Int32) getOrientation();
            }
            break;
            case BASEPROPERTY_BACKGROUNDCOLOR:
            {
                // the default implementation of the base class doesn't work here, since our
                // interpretation for this property is slightly different
                aProp = ::toolkit::getButtonLikeFaceColor( pScrollBar );
            }
            break;

            default:
            {
                aProp <<= VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXScrollBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_SCROLLBAR_SCROLL:
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
                // since we call listeners below, there is a potential that we will be destroyed
                // in during the listener call. To prevent the resulting crashs, we keep us
                // alive as long as we're here

            if ( maAdjustmentListeners.getLength() )
            {
                VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();

                if( pScrollBar )
                {
                    ::com::sun::star::awt::AdjustmentEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                    aEvent.Value = pScrollBar->GetThumbPos();

                    // set adjustment type
                    ScrollType aType = pScrollBar->GetType();
                    if ( aType == SCROLL_LINEUP || aType == SCROLL_LINEDOWN )
                    {
                        aEvent.Type = ::com::sun::star::awt::AdjustmentType_ADJUST_LINE;
                    }
                    else if ( aType == SCROLL_PAGEUP || aType == SCROLL_PAGEDOWN )
                    {
                        aEvent.Type = ::com::sun::star::awt::AdjustmentType_ADJUST_PAGE;
                    }
                    else if ( aType == SCROLL_DRAG )
                    {
                        aEvent.Type = ::com::sun::star::awt::AdjustmentType_ADJUST_ABS;
                    }

                    maAdjustmentListeners.adjustmentValueChanged( aEvent );
                }
            }
        }
        break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

::com::sun::star::awt::Size SAL_CALL VCLXScrollBar::implGetMinimumSize( vcl::Window* p ) throw(::com::sun::star::uno::RuntimeException)
{
    long n = p->GetSettings().GetStyleSettings().GetScrollBarSize();
    return ::com::sun::star::awt::Size( n, n );
}

::com::sun::star::awt::Size SAL_CALL VCLXScrollBar::getMinimumSize() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return implGetMinimumSize( GetWindow() );
}



//  class VCLXEdit


void VCLXEdit::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ECHOCHAR,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_HARDLINEBREAKS,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_HSCROLL,
                     BASEPROPERTY_LINE_END_FORMAT,
                     BASEPROPERTY_MAXTEXTLEN,
                     BASEPROPERTY_MULTILINE,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_READONLY,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_TEXT,
                     BASEPROPERTY_VSCROLL,
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     BASEPROPERTY_PAINTTRANSPARENT,
                     BASEPROPERTY_AUTOHSCROLL,
                     BASEPROPERTY_AUTOVSCROLL,
                     BASEPROPERTY_VERTICALALIGN,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds );
}

VCLXEdit::VCLXEdit() : maTextListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXEdit::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XTextComponent* >(this)),
                                        (static_cast< ::com::sun::star::awt::XTextEditField* >(this)),
                                        (static_cast< ::com::sun::star::awt::XTextLayoutConstrains* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXEdit )
    cppu::UnoType<com::sun::star::awt::XTextComponent>::get(),
    cppu::UnoType<com::sun::star::awt::XTextEditField>::get(),
    cppu::UnoType<com::sun::star::awt::XTextLayoutConstrains>::get(),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXEdit::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXEdit::dispose() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maTextListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void VCLXEdit::addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    GetTextListeners().addInterface( l );
}

void VCLXEdit::removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    GetTextListeners().removeInterface( l );
}

void VCLXEdit::setText( const OUString& aText ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
    {
        pEdit->SetText( aText );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( true );
        pEdit->SetModifyFlag();
        pEdit->Modify();
        SetSynthesizingVCLEvent( false );
    }
}

void VCLXEdit::insertText( const ::com::sun::star::awt::Selection& rSel, const OUString& aText ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
    {
        pEdit->SetSelection( Selection( rSel.Min, rSel.Max ) );
        pEdit->ReplaceSelected( aText );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( true );
        pEdit->SetModifyFlag();
        pEdit->Modify();
        SetSynthesizingVCLEvent( false );
    }
}

OUString VCLXEdit::getText() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

OUString VCLXEdit::getSelectedText() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit)
        aText = pEdit->GetSelected();
    return aText;

}

void VCLXEdit::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        pEdit->SetSelection( Selection( aSelection.Min, aSelection.Max ) );
}

::com::sun::star::awt::Selection VCLXEdit::getSelection() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Selection aSel;
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        aSel = pEdit->GetSelection();
    return ::com::sun::star::awt::Selection( aSel.Min(), aSel.Max() );
}

sal_Bool VCLXEdit::isEditable() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    return pEdit && !pEdit->IsReadOnly() && pEdit->IsEnabled();
}

void VCLXEdit::setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        pEdit->SetReadOnly( !bEditable );
}


void VCLXEdit::setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        pEdit->SetMaxTextLen( nLen );
}

sal_Int16 VCLXEdit::getMaxTextLen() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    return pEdit ? pEdit->GetMaxTextLen() : 0;
}

void VCLXEdit::setEchoChar( sal_Unicode cEcho ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        pEdit->SetEchoChar( cEcho );
}

void VCLXEdit::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_HIDEINACTIVESELECTION:
                ::toolkit::adjustBooleanWindowStyle( Value, pEdit, WB_NOHIDESELECTION, true );
                if ( pEdit->GetSubEdit() )
                    ::toolkit::adjustBooleanWindowStyle( Value, pEdit->GetSubEdit(), WB_NOHIDESELECTION, true );
                break;

            case BASEPROPERTY_READONLY:
            {
                bool b = bool();
                if ( Value >>= b )
                     pEdit->SetReadOnly( b );
            }
            break;
            case BASEPROPERTY_ECHOCHAR:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                     pEdit->SetEchoChar( n );
            }
            break;
            case BASEPROPERTY_MAXTEXTLEN:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                     pEdit->SetMaxTextLen( n );
            }
            break;
            default:
            {
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXEdit::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_HIDEINACTIVESELECTION:
                aProp <<= ( ( pEdit->GetStyle() & WB_NOHIDESELECTION ) == 0 );
                break;
            case BASEPROPERTY_READONLY:
                 aProp <<= pEdit->IsReadOnly();
                break;
            case BASEPROPERTY_ECHOCHAR:
                 aProp <<= (sal_Int16) pEdit->GetEchoChar();
                break;
            case BASEPROPERTY_MAXTEXTLEN:
                 aProp <<= (sal_Int16) pEdit->GetMaxTextLen();
                break;
            default:
            {
                aProp = VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

::com::sun::star::awt::Size VCLXEdit::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        aSz = pEdit->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXEdit::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
    {
        aSz = pEdit->CalcMinimumSize();
        aSz.Height() += 4;
    }
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXEdit::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::Size aSz = rNewSize;
    ::com::sun::star::awt::Size aMinSz = getMinimumSize();
    if ( aSz.Height != aMinSz.Height )
        aSz.Height = aMinSz.Height;

    return aSz;
}

::com::sun::star::awt::Size VCLXEdit::getMinimumSize( sal_Int16 nCols, sal_Int16 ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
    {
        if ( nCols )
            aSz = pEdit->CalcSize( nCols );
        else
            aSz = pEdit->CalcMinimumSize();
    }
    return AWTSize(aSz);
}

void VCLXEdit::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    nLines = 1;
    nCols = 0;
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        nCols = pEdit->GetMaxVisChars();
}

void VCLXEdit::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_EDIT_MODIFY:
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
                // since we call listeners below, there is a potential that we will be destroyed
                // during the listener call. To prevent the resulting crashs, we keep us
                // alive as long as we're here

            if ( GetTextListeners().getLength() )
            {
                ::com::sun::star::awt::TextEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                GetTextListeners().textChanged( aEvent );
            }
        }
        break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}


//  class VCLXComboBox


void VCLXComboBox::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_AUTOCOMPLETE,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_DROPDOWN,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_LINECOUNT,
                     BASEPROPERTY_MAXTEXTLEN,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_READONLY,
                     BASEPROPERTY_STRINGITEMLIST,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_TEXT,
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     BASEPROPERTY_REFERENCE_DEVICE,
                     BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR,
                     0);
    // no, don't call VCLXEdit here - it has properties which we do *not* want to have at combo box
    // #i92690# / 2008-08-12 / frank.schoenheit@sun.com
    // VCLXEdit::ImplGetPropertyIds( rIds );
    VCLXWindow::ImplGetPropertyIds( rIds );
}

VCLXComboBox::VCLXComboBox()
    : maActionListeners( *this ), maItemListeners( *this )
{
}

VCLXComboBox::~VCLXComboBox()
{
    OSL_TRACE ("%s", __FUNCTION__);
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXComboBox::CreateAccessibleContext()
{
    SolarMutexGuard aGuard;

    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXComboBox::dispose() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maItemListeners.disposeAndClear( aObj );
    maActionListeners.disposeAndClear( aObj );
    VCLXEdit::dispose();
}


void VCLXComboBox::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXComboBox::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXComboBox::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXComboBox::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXComboBox::addItem( const OUString& aItem, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
        pBox->InsertEntry( aItem, nPos );
}

void VCLXComboBox::addItems( const ::com::sun::star::uno::Sequence< OUString>& aItems, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
    {
        sal_uInt16 nP = nPos;
        for ( sal_Int32 n = 0; n < aItems.getLength(); n++ )
        {
            pBox->InsertEntry( aItems.getConstArray()[n], nP );
            if ( nP == 0xFFFF )
            {
                OSL_FAIL( "VCLXComboBox::addItems: too many entries!" );
                // skip remaining entries, list cannot hold them, anyway
                break;
            }
        }
    }
}

void VCLXComboBox::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
    {
        for ( sal_uInt16 n = nCount; n; )
            pBox->RemoveEntryAt( nPos + (--n) );
    }
}

sal_Int16 VCLXComboBox::getItemCount() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    return pBox ? pBox->GetEntryCount() : 0;
}

OUString VCLXComboBox::getItem( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aItem;
    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
        aItem = pBox->GetEntry( nPos );
    return aItem;
}

::com::sun::star::uno::Sequence< OUString> VCLXComboBox::getItems() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Sequence< OUString> aSeq;
    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
    {
        auto n = pBox->GetEntryCount();
        aSeq = ::com::sun::star::uno::Sequence< OUString>( n );
        while ( n )
        {
            --n;
            aSeq.getArray()[n] = pBox->GetEntry( n );
        }
    }
    return aSeq;
}

void VCLXComboBox::setDropDownLineCount( sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
        pBox->SetDropDownLineCount( nLines );
}

sal_Int16 VCLXComboBox::getDropDownLineCount() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    sal_Int16 nLines = 0;
    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
        nLines = pBox->GetDropDownLineCount();
    return nLines;
}

void VCLXComboBox::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( pComboBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LINECOUNT:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                     pComboBox->SetDropDownLineCount( n );
            }
            break;
            case BASEPROPERTY_AUTOCOMPLETE:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                     pComboBox->EnableAutocomplete( n != 0 );
                else
                {
                    bool b = bool();
                    if ( Value >>= b )
                        pComboBox->EnableAutocomplete( b );
                }
            }
            break;
            case BASEPROPERTY_STRINGITEMLIST:
            {
                ::com::sun::star::uno::Sequence< OUString> aItems;
                if ( Value >>= aItems )
                {
                    pComboBox->Clear();
                    addItems( aItems, 0 );
                }
            }
            break;
            default:
            {
                VCLXEdit::setProperty( PropertyName, Value );

                // #109385# SetBorderStyle is not virtual
                if ( nPropType == BASEPROPERTY_BORDER )
                {
                    sal_uInt16 nBorder = sal_uInt16();
                    if ( (Value >>= nBorder) && nBorder != 0 )
                        pComboBox->SetBorderStyle( static_cast<WindowBorderStyle>(nBorder) );
                }
            }
        }
    }
}

::com::sun::star::uno::Any VCLXComboBox::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( pComboBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LINECOUNT:
            {
                 aProp <<= (sal_Int16)  pComboBox->GetDropDownLineCount();
            }
            break;
            case BASEPROPERTY_AUTOCOMPLETE:
            {
                 aProp <<= pComboBox->IsAutocompleteEnabled();
            }
            break;
            case BASEPROPERTY_STRINGITEMLIST:
            {
                const sal_Int32 nItems = pComboBox->GetEntryCount();
                ::com::sun::star::uno::Sequence< OUString> aSeq( nItems );
                OUString* pStrings = aSeq.getArray();
                for ( sal_Int32 n = 0; n < nItems; ++n )
                    pStrings[n] = pComboBox->GetEntry( n );
                aProp <<= aSeq;

            }
            break;
            default:
            {
                aProp <<= VCLXEdit::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXComboBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
        // since we call listeners below, there is a potential that we will be destroyed
        // during the listener call. To prevent the resulting crashs, we keep us
        // alive as long as we're here

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_COMBOBOX_SELECT:
            if ( maItemListeners.getLength() )
            {
                VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
                if( pComboBox )
                {
                    if ( !pComboBox->IsTravelSelect() )
                    {
                        ::com::sun::star::awt::ItemEvent aEvent;
                        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                        aEvent.Highlighted = 0;

                        // Set to 0xFFFF on multiple selection, selected entry ID otherwise
                        aEvent.Selected = pComboBox->GetEntryPos( pComboBox->GetText() );

                        maItemListeners.itemStateChanged( aEvent );
                    }
                }
            }
            break;

        case VCLEVENT_COMBOBOX_DOUBLECLICK:
            if ( maActionListeners.getLength() )
            {
                ::com::sun::star::awt::ActionEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
//              aEvent.ActionCommand = ...;
                maActionListeners.actionPerformed( aEvent );
            }
            break;

        default:
            VCLXEdit::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

::com::sun::star::awt::Size VCLXComboBox::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( pComboBox )
        aSz = pComboBox->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXComboBox::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( pComboBox )
    {
        aSz = pComboBox->CalcMinimumSize();
        if ( pComboBox->GetStyle() & WB_DROPDOWN )
            aSz.Height() += 4;
    }
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXComboBox::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz = VCLSize(rNewSize);
    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( pComboBox )
        aSz = pComboBox->CalcAdjustedSize( aSz );
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXComboBox::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( pComboBox )
        aSz = pComboBox->CalcBlockSize( nCols, nLines );
    return AWTSize(aSz);
}

void VCLXComboBox::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    nCols = nLines = 0;
    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( pComboBox )
    {
        sal_uInt16 nC, nL;
        pComboBox->GetMaxVisColumnsAndLines( nC, nL );
        nCols = nC;
        nLines = nL;
    }
}
void SAL_CALL VCLXComboBox::listItemInserted( const ItemListEvent& i_rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pComboBox = GetAsDynamic< ComboBox >();

    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemInserted: no ComboBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition <= sal_Int32( pComboBox->GetEntryCount() ) ),
        "VCLXComboBox::listItemInserted: illegal (inconsistent) item position!" );
    pComboBox->InsertEntryWithImage(
        i_rEvent.ItemText.IsPresent ? i_rEvent.ItemText.Value : OUString(),
        i_rEvent.ItemImageURL.IsPresent ? lcl_getImageFromURL( i_rEvent.ItemImageURL.Value ) : Image(),
        i_rEvent.ItemPosition );
}

void SAL_CALL VCLXComboBox::listItemRemoved( const ItemListEvent& i_rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pComboBox = GetAsDynamic< ComboBox >();

    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemRemoved: no ComboBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition < sal_Int32( pComboBox->GetEntryCount() ) ),
        "VCLXComboBox::listItemRemoved: illegal (inconsistent) item position!" );

    pComboBox->RemoveEntryAt( i_rEvent.ItemPosition );
}

void SAL_CALL VCLXComboBox::listItemModified( const ItemListEvent& i_rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pComboBox = GetAsDynamic< ComboBox >();

    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemModified: no ComboBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition < sal_Int32( pComboBox->GetEntryCount() ) ),
        "VCLXComboBox::listItemModified: illegal (inconsistent) item position!" );

    // VCL's ComboBox does not support changing an entry's text or image, so remove and re-insert

    const OUString sNewText = i_rEvent.ItemText.IsPresent ? i_rEvent.ItemText.Value : OUString( pComboBox->GetEntry( i_rEvent.ItemPosition ) );
    const Image aNewImage( i_rEvent.ItemImageURL.IsPresent ? lcl_getImageFromURL( i_rEvent.ItemImageURL.Value ) : pComboBox->GetEntryImage( i_rEvent.ItemPosition  ) );

    pComboBox->RemoveEntryAt( i_rEvent.ItemPosition );
    pComboBox->InsertEntryWithImage(sNewText, aNewImage, i_rEvent.ItemPosition);
}

void SAL_CALL VCLXComboBox::allItemsRemoved( const EventObject& i_rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pComboBox = GetAsDynamic< ComboBox >();
    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemModified: no ComboBox?!" );

    pComboBox->Clear();

    (void)i_rEvent;
}

void SAL_CALL VCLXComboBox::itemListChanged( const EventObject& i_rEvent ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pComboBox = GetAsDynamic< ComboBox >();
    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemModified: no ComboBox?!" );

    pComboBox->Clear();

    uno::Reference< beans::XPropertySet > xPropSet( i_rEvent.Source, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySetInfo > xPSI( xPropSet->getPropertySetInfo(), uno::UNO_QUERY_THROW );
    // bool localize = xPSI->hasPropertyByName("ResourceResolver");
    uno::Reference< resource::XStringResourceResolver > xStringResourceResolver;
    if ( xPSI->hasPropertyByName("ResourceResolver") )
    {
        xStringResourceResolver.set(
            xPropSet->getPropertyValue("ResourceResolver"),
            uno::UNO_QUERY
        );
    }


    Reference< XItemList > xItemList( i_rEvent.Source, uno::UNO_QUERY_THROW );
    uno::Sequence< beans::Pair< OUString, OUString > > aItems = xItemList->getAllItems();
    for ( sal_Int32 i=0; i<aItems.getLength(); ++i )
    {
        OUString aLocalizationKey( aItems[i].First );
        if ( xStringResourceResolver.is() && !aLocalizationKey.isEmpty() && aLocalizationKey[0] == '&' )
        {
            aLocalizationKey = xStringResourceResolver->resolveString(aLocalizationKey.copy( 1 ));
        }
        pComboBox->InsertEntryWithImage(aLocalizationKey,
                lcl_getImageFromURL(aItems[i].Second));
    }
}
void SAL_CALL VCLXComboBox::disposing( const EventObject& i_rEvent ) throw (RuntimeException, std::exception)
{
    // just disambiguate
    VCLXEdit::disposing( i_rEvent );
}


//  class VCLXFormattedSpinField

void VCLXFormattedSpinField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    // Interestingly in the UnoControl API this is
    // - not derived from XEdit ultimately, (correct ?) - so cut this here ...
//    VCLXSpinField::ImplGetPropertyIds( rIds );
    VCLXWindow::ImplGetPropertyIds( rIds );
}

VCLXFormattedSpinField::VCLXFormattedSpinField()
    : mpFormatter(NULL)
{
}

VCLXFormattedSpinField::~VCLXFormattedSpinField()
{
}

void VCLXFormattedSpinField::setStrictFormat( bool bStrict )
{
    SolarMutexGuard aGuard;

    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
        pFormatter->SetStrictFormat( bStrict );
}

bool VCLXFormattedSpinField::isStrictFormat()
{
    FormatterBase* pFormatter = GetFormatter();
    return pFormatter && pFormatter->IsStrictFormat();
}


void VCLXFormattedSpinField::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_SPIN:
            {
                bool b = bool();
                if ( Value >>= b )
                {
                    WinBits nStyle = GetWindow()->GetStyle() | WB_SPIN;
                    if ( !b )
                        nStyle &= ~WB_SPIN;
                    GetWindow()->SetStyle( nStyle );
                }
            }
            break;
            case BASEPROPERTY_STRICTFORMAT:
            {
                bool b = bool();
                if ( Value >>= b )
                {
                     pFormatter->SetStrictFormat( b );
                }
            }
            break;
            default:
            {
                VCLXSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXFormattedSpinField::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_TABSTOP:
            {
                aProp <<= ( GetWindow()->GetStyle() & WB_SPIN ) != 0;
            }
            break;
            case BASEPROPERTY_STRICTFORMAT:
            {
                aProp <<= pFormatter->IsStrictFormat();
            }
            break;
            default:
            {
                aProp <<= VCLXSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}



//  class VCLXDateField


void VCLXDateField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_DATE,
                     BASEPROPERTY_DATEMAX,
                     BASEPROPERTY_DATEMIN,
                     BASEPROPERTY_DATESHOWCENTURY,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_DROPDOWN,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_EXTDATEFORMAT,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_READONLY,
                     BASEPROPERTY_REPEAT,
                     BASEPROPERTY_REPEAT_DELAY,
                     BASEPROPERTY_SPIN,
                     BASEPROPERTY_STRICTFORMAT,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_ENFORCE_FORMAT,
                     BASEPROPERTY_TEXT,
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     BASEPROPERTY_VERTICALALIGN,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR,
                     0);
    VCLXFormattedSpinField::ImplGetPropertyIds( rIds );
}

VCLXDateField::VCLXDateField()
{
}

VCLXDateField::~VCLXDateField()
{
}

//change the window type here to match the role
::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXDateField::CreateAccessibleContext()
{
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->SetType( WINDOW_DATEFIELD );
    }
    return getAccessibleFactory().createAccessibleContext( this );
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXDateField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XDateField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXDateField )
    cppu::UnoType<com::sun::star::awt::XDateField>::get(),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXDateField::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_DATE:
            {
                if ( bVoid )
                {
                    GetAs< DateField >()->EnableEmptyFieldValue( true );
                    GetAs< DateField >()->SetEmptyFieldValue();
                }
                else
                {
                    util::Date d;
                    if ( Value >>= d )
                         setDate( d );
                }
            }
            break;
            case BASEPROPERTY_DATEMIN:
            {
                util::Date d;
                if ( Value >>= d )
                     setMin( d );
            }
            break;
            case BASEPROPERTY_DATEMAX:
            {
                util::Date d;
                if ( Value >>= d )
                     setMax( d );
            }
            break;
            case BASEPROPERTY_EXTDATEFORMAT:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                    GetAs< DateField >()->SetExtDateFormat( (ExtDateFieldFormat) n );
            }
            break;
            case BASEPROPERTY_DATESHOWCENTURY:
            {
                bool b = bool();
                if ( Value >>= b )
                     GetAs< DateField >()->SetShowDateCentury( b );
            }
            break;
            case BASEPROPERTY_ENFORCE_FORMAT:
            {
                bool bEnforce( true );
                OSL_VERIFY( Value >>= bEnforce );
                GetAs< DateField >()->EnforceValidValue( bEnforce );
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXDateField::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_DATE:
            {
                aProp <<= getDate();
            }
            break;
            case BASEPROPERTY_DATEMIN:
            {
                aProp <<= getMin();
            }
            break;
            case BASEPROPERTY_DATEMAX:
            {
                aProp <<= getMax();
            }
            break;
            case BASEPROPERTY_DATESHOWCENTURY:
            {
                aProp <<= GetAs< DateField >()->IsShowDateCentury();
            }
            break;
            case BASEPROPERTY_ENFORCE_FORMAT:
            {
                aProp <<= GetAs< DateField >()->IsEnforceValidValue( );
            }
            break;
            default:
            {
                aProp <<= VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}


void VCLXDateField::setDate( const util::Date& aDate ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
    {
        pDateField->SetDate( aDate );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( true );
        pDateField->SetModifyFlag();
        pDateField->Modify();
        SetSynthesizingVCLEvent( false );
    }
}

util::Date VCLXDateField::getDate() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        return pDateField->GetDate().GetUNODate();
    else
        return util::Date();
}

void VCLXDateField::setMin( const util::Date& aDate ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        pDateField->SetMin( aDate );
}

util::Date VCLXDateField::getMin() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        return pDateField->GetMin().GetUNODate();
    else
        return util::Date();
}

void VCLXDateField::setMax( const util::Date& aDate ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        pDateField->SetMax( aDate );
}

util::Date VCLXDateField::getMax() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        return pDateField->GetMax().GetUNODate();
    else
        return util::Date();
}

void VCLXDateField::setFirst( const util::Date& aDate ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        pDateField->SetFirst( aDate );
}

util::Date VCLXDateField::getFirst() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        return pDateField->GetFirst().GetUNODate();
    else
        return util::Date();
}

void VCLXDateField::setLast( const util::Date& aDate ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        pDateField->SetLast( aDate );
}

util::Date VCLXDateField::getLast() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        return pDateField->GetLast().GetUNODate();
    else
        return util::Date();
}

void VCLXDateField::setLongFormat( sal_Bool bLong ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        pDateField->SetLongFormat( bLong );
}

sal_Bool VCLXDateField::isLongFormat() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    return pDateField && pDateField->IsLongFormat();
}

void VCLXDateField::setEmpty() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
    {
        pDateField->SetEmptyDate();

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( true );
        pDateField->SetModifyFlag();
        pDateField->Modify();
        SetSynthesizingVCLEvent( false );
    }
}

sal_Bool VCLXDateField::isEmpty() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    return pDateField && pDateField->IsEmptyDate();
}

void VCLXDateField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXDateField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return VCLXFormattedSpinField::isStrictFormat();
}



//  class VCLXTimeField


void VCLXTimeField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_EXTTIMEFORMAT,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_READONLY,
                     BASEPROPERTY_REPEAT,
                     BASEPROPERTY_REPEAT_DELAY,
                     BASEPROPERTY_SPIN,
                     BASEPROPERTY_STRICTFORMAT,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_TIME,
                     BASEPROPERTY_TIMEMAX,
                     BASEPROPERTY_TIMEMIN,
                     BASEPROPERTY_ENFORCE_FORMAT,
                     BASEPROPERTY_TEXT,
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     BASEPROPERTY_VERTICALALIGN,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR,
                     0);
    VCLXFormattedSpinField::ImplGetPropertyIds( rIds );
}

VCLXTimeField::VCLXTimeField()
{
}

VCLXTimeField::~VCLXTimeField()
{
}

//change the window type here to match the role
::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXTimeField::CreateAccessibleContext()
{
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->SetType( WINDOW_TIMEFIELD );
    }
    return getAccessibleFactory().createAccessibleContext( this );
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXTimeField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XTimeField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXTimeField )
    cppu::UnoType<com::sun::star::awt::XTimeField>::get(),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXTimeField::setTime( const util::Time& aTime ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
    {
        pTimeField->SetTime( aTime );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( true );
        pTimeField->SetModifyFlag();
        pTimeField->Modify();
        SetSynthesizingVCLEvent( false );
    }
}

util::Time VCLXTimeField::getTime() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        return pTimeField->GetTime().GetUNOTime();
    else
        return util::Time();
}

void VCLXTimeField::setMin( const util::Time& aTime ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        pTimeField->SetMin( aTime );
}

util::Time VCLXTimeField::getMin() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        return pTimeField->GetMin().GetUNOTime();
    else
        return util::Time();
}

void VCLXTimeField::setMax( const util::Time& aTime ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        pTimeField->SetMax( aTime );
}

util::Time VCLXTimeField::getMax() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        return pTimeField->GetMax().GetUNOTime();
    else
        return util::Time();
}

void VCLXTimeField::setFirst( const util::Time& aTime ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        pTimeField->SetFirst( aTime );
}

util::Time VCLXTimeField::getFirst() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        return pTimeField->GetFirst().GetUNOTime();
    else
        return util::Time();
}

void VCLXTimeField::setLast( const util::Time& aTime ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        pTimeField->SetLast( aTime );
}

util::Time VCLXTimeField::getLast() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        return pTimeField->GetLast().GetUNOTime();
    else
        return util::Time();
}

void VCLXTimeField::setEmpty() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        pTimeField->SetEmptyTime();
}

sal_Bool VCLXTimeField::isEmpty() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    return pTimeField && pTimeField->IsEmptyTime();
}

void VCLXTimeField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXTimeField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return VCLXFormattedSpinField::isStrictFormat();
}


void VCLXTimeField::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_TIME:
            {
                if ( bVoid )
                {
                    GetAs< TimeField >()->EnableEmptyFieldValue( true );
                    GetAs< TimeField >()->SetEmptyFieldValue();
                }
                else
                {
                    util::Time t;
                    if ( Value >>= t )
                         setTime( t );
                }
            }
            break;
            case BASEPROPERTY_TIMEMIN:
            {
                util::Time t;
                if ( Value >>= t )
                     setMin( t );
            }
            break;
            case BASEPROPERTY_TIMEMAX:
            {
                util::Time t;
                if ( Value >>= t )
                     setMax( t );
            }
            break;
            case BASEPROPERTY_EXTTIMEFORMAT:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                    GetAs< TimeField >()->SetExtFormat( (ExtTimeFieldFormat) n );
            }
            break;
            case BASEPROPERTY_ENFORCE_FORMAT:
            {
                bool bEnforce( true );
                OSL_VERIFY( Value >>= bEnforce );
                GetAs< TimeField >()->EnforceValidValue( bEnforce );
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXTimeField::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    if ( GetWindow() )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_TIME:
            {
                aProp <<= getTime();
            }
            break;
            case BASEPROPERTY_TIMEMIN:
            {
                aProp <<= getMin();
            }
            break;
            case BASEPROPERTY_TIMEMAX:
            {
                aProp <<= getMax();
            }
            break;
            case BASEPROPERTY_ENFORCE_FORMAT:
            {
                aProp <<= GetAs< TimeField >()->IsEnforceValidValue( );
            }
            break;
            default:
            {
                aProp <<= VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}


//  class VCLXNumericField


void VCLXNumericField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_DECIMALACCURACY,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_NUMSHOWTHOUSANDSEP,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_READONLY,
                     BASEPROPERTY_REPEAT,
                     BASEPROPERTY_REPEAT_DELAY,
                     BASEPROPERTY_SPIN,
                     BASEPROPERTY_STRICTFORMAT,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_VALUEMAX_DOUBLE,
                     BASEPROPERTY_VALUEMIN_DOUBLE,
                     BASEPROPERTY_VALUESTEP_DOUBLE,
                     BASEPROPERTY_VALUE_DOUBLE,
                     BASEPROPERTY_ENFORCE_FORMAT,
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     BASEPROPERTY_VERTICALALIGN,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR,
                     0);
    VCLXFormattedSpinField::ImplGetPropertyIds( rIds );
}

VCLXNumericField::VCLXNumericField()
{
}

VCLXNumericField::~VCLXNumericField()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXNumericField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XNumericField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXNumericField )
    cppu::UnoType<com::sun::star::awt::XNumericField>::get(),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXNumericField::setValue( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    if ( pNumericFormatter )
    {
        // shift long value using decimal digits
        // (e.g., input 105 using 2 digits returns 1,05)
        // Thus, to set a value of 1,05, insert 105 and 2 digits
        pNumericFormatter->SetValue(
            (long)ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() ) );

        // #107218# Call same listeners like VCL would do after user interaction
        VclPtr< Edit > pEdit = GetAs< Edit >();
        if ( pEdit )
        {
            SetSynthesizingVCLEvent( true );
            pEdit->SetModifyFlag();
            pEdit->Modify();
            SetSynthesizingVCLEvent( false );
        }
    }
}

double VCLXNumericField::getValue() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter
        ? ImplCalcDoubleValue( (double)pNumericFormatter->GetValue(), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setMin( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    if ( pNumericFormatter )
        pNumericFormatter->SetMin(
            (long)ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() ) );
}

double VCLXNumericField::getMin() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter
        ? ImplCalcDoubleValue( (double)pNumericFormatter->GetMin(), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setMax( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    if ( pNumericFormatter )
        pNumericFormatter->SetMax(
            (long)ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() ) );
}

double VCLXNumericField::getMax() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter
        ? ImplCalcDoubleValue( (double)pNumericFormatter->GetMax(), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< NumericField > pNumericField = GetAs< NumericField >();
    if ( pNumericField )
        pNumericField->SetFirst(
            (long)ImplCalcLongValue( Value, pNumericField->GetDecimalDigits() ) );
}

double VCLXNumericField::getFirst() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< NumericField > pNumericField = GetAs< NumericField >();
    return pNumericField
        ? ImplCalcDoubleValue( (double)pNumericField->GetFirst(), pNumericField->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setLast( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< NumericField > pNumericField = GetAs< NumericField >();
    if ( pNumericField )
        pNumericField->SetLast(
            (long)ImplCalcLongValue( Value, pNumericField->GetDecimalDigits() ) );
}

double VCLXNumericField::getLast() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< NumericField > pNumericField = GetAs< NumericField >();
    return pNumericField
        ? ImplCalcDoubleValue( (double)pNumericField->GetLast(), pNumericField->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXNumericField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return VCLXFormattedSpinField::isStrictFormat();
}


void VCLXNumericField::setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< NumericField > pNumericField = GetAs< NumericField >();
    if ( pNumericField )
        pNumericField->SetSpinSize(
            (long)ImplCalcLongValue( Value, pNumericField->GetDecimalDigits() ) );
}

double VCLXNumericField::getSpinSize() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< NumericField > pNumericField = GetAs< NumericField >();
    return pNumericField
        ? ImplCalcDoubleValue( (double)pNumericField->GetSpinSize(), pNumericField->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setDecimalDigits( sal_Int16 Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    if ( pNumericFormatter )
    {
        double n = getValue();
        pNumericFormatter->SetDecimalDigits( Value );
        setValue( n );
       }
}

sal_Int16 VCLXNumericField::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter ? pNumericFormatter->GetDecimalDigits() : 0;
}

void VCLXNumericField::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                if ( bVoid )
                {
                    GetAs< NumericField >()->EnableEmptyFieldValue( true );
                    GetAs< NumericField >()->SetEmptyFieldValue();
                }
                else
                {
                    double d = 0;
                    if ( Value >>= d )
                         setValue( d );
                }
            }
            break;
            case BASEPROPERTY_VALUEMIN_DOUBLE:
            {
                double d = 0;
                if ( Value >>= d )
                     setMin( d );
            }
            break;
            case BASEPROPERTY_VALUEMAX_DOUBLE:
            {
                double d = 0;
                if ( Value >>= d )
                     setMax( d );
            }
            break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:
            {
                double d = 0;
                if ( Value >>= d )
                     setSpinSize( d );
            }
            break;
            case BASEPROPERTY_DECIMALACCURACY:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                     setDecimalDigits( n );
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                bool b = bool();
                if ( Value >>= b )
                     GetAs< NumericField >()->SetUseThousandSep( b );
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXNumericField::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                aProp <<= (double) getValue();
            }
            break;
            case BASEPROPERTY_VALUEMIN_DOUBLE:
            {
                aProp <<= (double) getMin();
            }
            break;
            case BASEPROPERTY_VALUEMAX_DOUBLE:
            {
                aProp <<= (double) getMax();
            }
            break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:
            {
                aProp <<= (double) getSpinSize();
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                aProp <<= GetAs< NumericField >()->IsUseThousandSep();
            }
            break;
            default:
            {
                aProp <<= VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}


//    ----------------------------------------------------
//    class VCLXMetricField
//    ----------------------------------------------------

void VCLXMetricField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_DECIMALACCURACY,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_NUMSHOWTHOUSANDSEP,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_READONLY,
                     BASEPROPERTY_REPEAT,
                     BASEPROPERTY_REPEAT_DELAY,
                     BASEPROPERTY_SPIN,
                     BASEPROPERTY_STRICTFORMAT,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_ENFORCE_FORMAT,
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     BASEPROPERTY_UNIT,
                     BASEPROPERTY_CUSTOMUNITTEXT,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR,
                     0);
    VCLXFormattedSpinField::ImplGetPropertyIds( rIds );
}

VCLXMetricField::VCLXMetricField()
{
}

VCLXMetricField::~VCLXMetricField()
{
}

MetricFormatter *VCLXMetricField::GetMetricFormatter() throw(::com::sun::star::uno::RuntimeException)
{
    MetricFormatter *pFormatter = static_cast<MetricFormatter *>(GetFormatter());
    if (!pFormatter)
        throw ::com::sun::star::uno::RuntimeException();
    return pFormatter;
}

MetricField *VCLXMetricField::GetMetricField() throw(::com::sun::star::uno::RuntimeException)
{
    VclPtr< MetricField > pField = GetAs< MetricField >();
    if (!pField)
        throw ::com::sun::star::uno::RuntimeException();
    return pField;
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXMetricField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                                              (static_cast< ::com::sun::star::awt::XMetricField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXMetricField )
    cppu::UnoType<com::sun::star::awt::XMetricField>::get(),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

// FIXME: later ...
#define MetricUnitUnoToVcl(a) ((FieldUnit)(a))

#define METRIC_MAP_PAIR(method,parent) \
    sal_Int64 VCLXMetricField::get##method( sal_Int16 nUnit ) throw (::com::sun::star::uno::RuntimeException, std::exception) \
    { \
        SolarMutexGuard aGuard; \
        return GetMetric##parent()->Get##method( MetricUnitUnoToVcl( nUnit ) ); \
    } \
    void VCLXMetricField::set##method( sal_Int64 nValue, sal_Int16 nUnit ) throw (::com::sun::star::uno::RuntimeException, std::exception) \
    { \
        SolarMutexGuard aGuard; \
        GetMetric##parent()->Set##method( nValue, MetricUnitUnoToVcl( nUnit ) ); \
    }

METRIC_MAP_PAIR(Min, Formatter)
METRIC_MAP_PAIR(Max, Formatter)
METRIC_MAP_PAIR(First, Field)
METRIC_MAP_PAIR(Last,  Field)

#undef METRIC_MAP_PAIR

::sal_Int64 VCLXMetricField::getValue( ::sal_Int16 nUnit ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return GetMetricFormatter()->GetValue( MetricUnitUnoToVcl( nUnit ) );
}

::sal_Int64 VCLXMetricField::getCorrectedValue( ::sal_Int16 nUnit ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return GetMetricFormatter()->GetCorrectedValue( MetricUnitUnoToVcl( nUnit ) );
}

// FIXME: acute cut/paste evilness - move this to the parent Edit class ?
void VCLXMetricField::CallListeners()
{
    // #107218# Call same listeners like VCL would do after user interaction
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
    {
        SetSynthesizingVCLEvent( true );
        pEdit->SetModifyFlag();
        pEdit->Modify();
        SetSynthesizingVCLEvent( false );
    }
}

void VCLXMetricField::setValue( ::sal_Int64 Value, ::sal_Int16 Unit ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    GetMetricFormatter()->SetValue( Value, MetricUnitUnoToVcl( Unit ) );
    CallListeners();
}

void VCLXMetricField::setUserValue( ::sal_Int64 Value, ::sal_Int16 Unit ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    GetMetricFormatter()->SetUserValue( Value, MetricUnitUnoToVcl( Unit ) );
    CallListeners();
}

void VCLXMetricField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXMetricField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return VCLXFormattedSpinField::isStrictFormat();
}

void VCLXMetricField::setSpinSize( sal_Int64 Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    GetMetricField()->SetSpinSize( Value );
}

sal_Int64 VCLXMetricField::getSpinSize() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return GetMetricField()->GetSpinSize();
}

void VCLXMetricField::setDecimalDigits( sal_Int16 Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    GetMetricFormatter()->SetDecimalDigits( Value );
}

sal_Int16 VCLXMetricField::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter ? pNumericFormatter->GetDecimalDigits() : 0;
}

void VCLXMetricField::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_DECIMALACCURACY:
            {
                sal_Int16 n = 0;
                if ( Value >>= n )
                     setDecimalDigits( n );
                break;
            }
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                bool b = false;
                if ( Value >>= b )
                     GetAs< NumericField >()->SetUseThousandSep( b );
            }
            break;
            case BASEPROPERTY_UNIT:
            {
                sal_uInt16 nVal = 0;
                if ( Value >>= nVal )
                    GetAs< MetricField >()->SetUnit( (FieldUnit) nVal );
                break;
            }
            case BASEPROPERTY_CUSTOMUNITTEXT:
            {
                OUString aStr;
                if ( Value >>= aStr )
                    GetAs< MetricField >()->SetCustomUnitText( aStr );
                break;
            }
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
                break;
            }
        }
    }
}

::com::sun::star::uno::Any VCLXMetricField::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
                aProp <<= GetAs< NumericField >()->IsUseThousandSep();
                break;
            case BASEPROPERTY_UNIT:
                aProp <<= (sal_uInt16) (GetAs< MetricField >()->GetUnit());
                break;
            case BASEPROPERTY_CUSTOMUNITTEXT:
                aProp <<= OUString( GetAs< MetricField >()->GetCustomUnitText() );
                break;
            default:
            {
                aProp <<= VCLXFormattedSpinField::getProperty( PropertyName );
                break;
            }
        }
    }
    return aProp;
}



//  class VCLXCurrencyField


void VCLXCurrencyField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_CURRENCYSYMBOL,
                     BASEPROPERTY_CURSYM_POSITION,
                     BASEPROPERTY_DECIMALACCURACY,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_NUMSHOWTHOUSANDSEP,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_READONLY,
                     BASEPROPERTY_REPEAT,
                     BASEPROPERTY_REPEAT_DELAY,
                     BASEPROPERTY_SPIN,
                     BASEPROPERTY_STRICTFORMAT,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_VALUEMAX_DOUBLE,
                     BASEPROPERTY_VALUEMIN_DOUBLE,
                     BASEPROPERTY_VALUESTEP_DOUBLE,
                     BASEPROPERTY_VALUE_DOUBLE,
                     BASEPROPERTY_ENFORCE_FORMAT,
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     BASEPROPERTY_VERTICALALIGN,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR,
                     0);
    VCLXFormattedSpinField::ImplGetPropertyIds( rIds );
}

VCLXCurrencyField::VCLXCurrencyField()
{
}

VCLXCurrencyField::~VCLXCurrencyField()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXCurrencyField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XCurrencyField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXCurrencyField )
    cppu::UnoType<com::sun::star::awt::XCurrencyField>::get(),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXCurrencyField::setValue( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = static_cast<LongCurrencyFormatter*>(GetFormatter());
    if ( pCurrencyFormatter )
    {
        // shift long value using decimal digits
        // (e.g., input 105 using 2 digits returns 1,05)
        // Thus, to set a value of 1,05, insert 105 and 2 digits
        pCurrencyFormatter->SetValue(
            ImplCalcLongValue( Value, pCurrencyFormatter->GetDecimalDigits() ) );

        // #107218# Call same listeners like VCL would do after user interaction
        VclPtr< Edit > pEdit = GetAs< Edit >();
        if ( pEdit )
        {
            SetSynthesizingVCLEvent( true );
            pEdit->SetModifyFlag();
            pEdit->Modify();
            SetSynthesizingVCLEvent( false );
        }
    }
}

double VCLXCurrencyField::getValue() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = static_cast<LongCurrencyFormatter*>(GetFormatter());
    return pCurrencyFormatter
        ? ImplCalcDoubleValue( (double)pCurrencyFormatter->GetValue(), pCurrencyFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setMin( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = static_cast<LongCurrencyFormatter*>(GetFormatter());
    if ( pCurrencyFormatter )
        pCurrencyFormatter->SetMin(
            ImplCalcLongValue( Value, pCurrencyFormatter->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getMin() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = static_cast<LongCurrencyFormatter*>(GetFormatter());
    return pCurrencyFormatter
        ? ImplCalcDoubleValue( (double)pCurrencyFormatter->GetMin(), pCurrencyFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setMax( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = static_cast<LongCurrencyFormatter*>(GetFormatter());
    if ( pCurrencyFormatter )
        pCurrencyFormatter->SetMax(
            ImplCalcLongValue( Value, pCurrencyFormatter->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getMax() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = static_cast<LongCurrencyFormatter*>(GetFormatter());
    return pCurrencyFormatter
        ? ImplCalcDoubleValue( (double)pCurrencyFormatter->GetMax(), pCurrencyFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< LongCurrencyField > pCurrencyField = GetAs< LongCurrencyField >();
    if ( pCurrencyField )
        pCurrencyField->SetFirst(
            ImplCalcLongValue( Value, pCurrencyField->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getFirst() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< LongCurrencyField > pCurrencyField = GetAs< LongCurrencyField >();
    return pCurrencyField
        ? ImplCalcDoubleValue( (double)pCurrencyField->GetFirst(), pCurrencyField->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setLast( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< LongCurrencyField > pCurrencyField = GetAs< LongCurrencyField >();
    if ( pCurrencyField )
        pCurrencyField->SetLast(
            ImplCalcLongValue( Value, pCurrencyField->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getLast() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< LongCurrencyField > pCurrencyField = GetAs< LongCurrencyField >();
    return pCurrencyField
        ? ImplCalcDoubleValue( (double)pCurrencyField->GetLast(), pCurrencyField->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< LongCurrencyField > pCurrencyField = GetAs< LongCurrencyField >();
    if ( pCurrencyField )
        pCurrencyField->SetSpinSize(
            ImplCalcLongValue( Value, pCurrencyField->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getSpinSize() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< LongCurrencyField > pCurrencyField = GetAs< LongCurrencyField >();
    return pCurrencyField
        ? ImplCalcDoubleValue( (double)pCurrencyField->GetSpinSize(), pCurrencyField->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXCurrencyField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return VCLXFormattedSpinField::isStrictFormat();
}


void VCLXCurrencyField::setDecimalDigits( sal_Int16 Value ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = static_cast<LongCurrencyFormatter*>(GetFormatter());
    if ( pCurrencyFormatter )
    {
        double n = getValue();
        pCurrencyFormatter->SetDecimalDigits( Value );
        setValue( n );
       }
}

sal_Int16 VCLXCurrencyField::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = static_cast<LongCurrencyFormatter*>(GetFormatter());
    return pCurrencyFormatter ? pCurrencyFormatter->GetDecimalDigits() : 0;
}

void VCLXCurrencyField::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                if ( bVoid )
                {
                    GetAs< LongCurrencyField >()->EnableEmptyFieldValue( true );
                    GetAs< LongCurrencyField >()->SetEmptyFieldValue();
                }
                else
                {
                    double d = 0;
                    if ( Value >>= d )
                         setValue( d );
                }
            }
            break;
            case BASEPROPERTY_VALUEMIN_DOUBLE:
            {
                double d = 0;
                if ( Value >>= d )
                     setMin( d );
            }
            break;
            case BASEPROPERTY_VALUEMAX_DOUBLE:
            {
                double d = 0;
                if ( Value >>= d )
                     setMax( d );
            }
            break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:
            {
                double d = 0;
                if ( Value >>= d )
                     setSpinSize( d );
            }
            break;
            case BASEPROPERTY_DECIMALACCURACY:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                     setDecimalDigits( n );
            }
            break;
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
                OUString aString;
                if ( Value >>= aString )
                     GetAs< LongCurrencyField >()->SetCurrencySymbol( aString );
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                bool b = bool();
                if ( Value >>= b )
                     GetAs< LongCurrencyField >()->SetUseThousandSep( b );
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXCurrencyField::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                aProp <<= (double) getValue();
            }
            break;
            case BASEPROPERTY_VALUEMIN_DOUBLE:
            {
                aProp <<= (double) getMin();
            }
            break;
            case BASEPROPERTY_VALUEMAX_DOUBLE:
            {
                aProp <<= (double) getMax();
            }
            break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:
            {
                aProp <<= (double) getSpinSize();
            }
            break;
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
                aProp <<= OUString( GetAs< LongCurrencyField >()->GetCurrencySymbol() );
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                aProp <<= GetAs< LongCurrencyField >()->IsUseThousandSep();
            }
            break;
            default:
            {
                aProp <<= VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}


//  class VCLXPatternField


void VCLXPatternField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_ALIGN,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_BORDER,
                     BASEPROPERTY_BORDERCOLOR,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_EDITMASK,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_LITERALMASK,
                     BASEPROPERTY_MAXTEXTLEN,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_READONLY,
                     BASEPROPERTY_STRICTFORMAT,
                     BASEPROPERTY_TABSTOP,
                     BASEPROPERTY_TEXT,
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     BASEPROPERTY_VERTICALALIGN,
                     BASEPROPERTY_WRITING_MODE,
                     BASEPROPERTY_CONTEXT_WRITING_MODE,
                     BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR,
                     0);
    VCLXFormattedSpinField::ImplGetPropertyIds( rIds );
}

VCLXPatternField::VCLXPatternField()
{
}

VCLXPatternField::~VCLXPatternField()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXPatternField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XPatternField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXPatternField )
    cppu::UnoType<com::sun::star::awt::XPatternField>::get(),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXPatternField::setMasks( const OUString& EditMask, const OUString& LiteralMask ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< PatternField > pPatternField = GetAs< PatternField >();
    if ( pPatternField )
    {
        pPatternField->SetMask( OUStringToOString(EditMask, RTL_TEXTENCODING_ASCII_US), LiteralMask );
    }
}

void VCLXPatternField::getMasks( OUString& EditMask, OUString& LiteralMask ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< PatternField > pPatternField = GetAs< PatternField >();
    if ( pPatternField )
    {
        EditMask = OStringToOUString(pPatternField->GetEditMask(), RTL_TEXTENCODING_ASCII_US);
        LiteralMask = pPatternField->GetLiteralMask();
    }
}

void VCLXPatternField::setString( const OUString& Str ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< PatternField > pPatternField = GetAs< PatternField >();
    if ( pPatternField )
        pPatternField->SetString( Str );
}

OUString VCLXPatternField::getString() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aString;
    VclPtr< PatternField > pPatternField = GetAs< PatternField >();
    if ( pPatternField )
        aString = pPatternField->GetString();
    return aString;
}

void VCLXPatternField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXPatternField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return VCLXFormattedSpinField::isStrictFormat();
}

void VCLXPatternField::setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_EDITMASK:
            case BASEPROPERTY_LITERALMASK:
            {
                OUString aString;
                if ( Value >>= aString )
                {
                    OUString aEditMask, aLiteralMask;
                    getMasks( aEditMask, aLiteralMask );
                    if ( nPropType == BASEPROPERTY_EDITMASK )
                        aEditMask = aString;
                    else
                        aLiteralMask = aString;
                     setMasks( aEditMask, aLiteralMask );
                }
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXPatternField::getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    if ( GetWindow() )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_EDITMASK:
            case BASEPROPERTY_LITERALMASK:
            {
                OUString aEditMask, aLiteralMask;
                getMasks( aEditMask, aLiteralMask );
                if ( nPropType == BASEPROPERTY_EDITMASK )
                    aProp <<= aEditMask;
                else
                    aProp <<= aLiteralMask;
            }
            break;
            default:
            {
                aProp <<= VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}


//  class VCLXToolBox

VCLXToolBox::VCLXToolBox()
{
}

VCLXToolBox::~VCLXToolBox()
{
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXToolBox::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}


//  class VCLXFrame

VCLXFrame::VCLXFrame()
{
}

void VCLXFrame::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_BACKGROUNDCOLOR,
                     BASEPROPERTY_DEFAULTCONTROL,
                     BASEPROPERTY_ENABLED,
                     BASEPROPERTY_ENABLEVISIBLE,
                     BASEPROPERTY_FONTDESCRIPTOR,
                     BASEPROPERTY_GRAPHIC,
                     BASEPROPERTY_HELPTEXT,
                     BASEPROPERTY_HELPURL,
                     BASEPROPERTY_PRINTABLE,
                     BASEPROPERTY_LABEL,
                     0);
    VCLXContainer::ImplGetPropertyIds( rIds );
}

VCLXFrame::~VCLXFrame()
{
}

::com::sun::star::uno::Any SAL_CALL VCLXFrame::queryInterface(const ::com::sun::star::uno::Type & rType )
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return VCLXContainer::queryInterface( rType );
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXFrame )
    VCLXContainer::getTypes()
IMPL_XTYPEPROVIDER_END

// ::com::sun::star::awt::XView
void SAL_CALL VCLXFrame::draw( sal_Int32 nX, sal_Int32 nY )
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Size aSize = pDev->PixelToLogic( pWindow->GetSizePixel() );
        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );

        pWindow->Draw( pDev, aPos, aSize, DrawFlags::NoControls );
    }
}

// ::com::sun::star::awt::XDevice,
::com::sun::star::awt::DeviceInfo SAL_CALL VCLXFrame::getInfo()
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::awt::DeviceInfo aInfo = VCLXDevice::getInfo();
    return aInfo;
}

void SAL_CALL VCLXFrame::setProperty(
    const OUString& PropertyName,
    const ::com::sun::star::uno::Any& Value )
throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VCLXContainer::setProperty( PropertyName, Value );
}

void VCLXFrame::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
    VCLXContainer::ProcessWindowEvent( rVclWindowEvent );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
