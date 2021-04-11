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
#include <toolkit/helper/accessiblefactory.hxx>
#include <com/sun/star/awt/LineEndFormat.hpp>
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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <com/sun/star/awt/XItemList.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <sal/log.hxx>

#include <awt/vclxwindows.hxx>
#include <controls/filectrl.hxx>
#include <controls/svmedit.hxx>
#include <svl/zforlist.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/fmtfield.hxx>
#include <vcl/graph.hxx>
#include <vcl/toolkit/lstbox.hxx>
#include <vcl/toolkit/combobox.hxx>
#include <vcl/toolkit/field.hxx>
#include <vcl/toolkit/fixedhyper.hxx>
#include <vcl/toolkit/imgctrl.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <vcl/toolkit/prgsbar.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/settings.hxx>
#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>

#include <helper/imagealign.hxx>
#include <helper/msgbox.hxx>
#include <helper/tkresmgr.hxx>
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
    void setButtonLikeFaceColor( vcl::Window* _pWindow, const css::uno::Any& _rColorValue )
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
            Color nBackgroundColor;
            _rColorValue >>= nBackgroundColor;
            aStyleSettings.SetFaceColor( nBackgroundColor );

            // for the real background (everything except the buttons and the thumb),
            // use an average between the desired color and "white"
            Color aWhite( COL_WHITE );
            Color aCheckedBackground( nBackgroundColor );
            aCheckedBackground.SetRed( ( aCheckedBackground.GetRed() + aWhite.GetRed() ) / 2 );
            aCheckedBackground.SetGreen( ( aCheckedBackground.GetGreen() + aWhite.GetGreen() ) / 2 );
            aCheckedBackground.SetBlue( ( aCheckedBackground.GetBlue() + aWhite.GetBlue() ) / 2 );
            aStyleSettings.SetCheckedColor( aCheckedBackground );

            sal_Int32 nBackgroundLuminance = nBackgroundColor.GetLuminance();
            sal_Int32 nWhiteLuminance = COL_WHITE.GetLuminance();

            Color aLightShadow( nBackgroundColor );
            aLightShadow.IncreaseLuminance( static_cast<sal_uInt8>( ( nWhiteLuminance - nBackgroundLuminance ) * 2 / 3 ) );
            aStyleSettings.SetLightBorderColor( aLightShadow );

            Color aLight( nBackgroundColor );
            aLight.IncreaseLuminance( static_cast<sal_uInt8>( ( nWhiteLuminance - nBackgroundLuminance ) * 1 / 3 ) );
            aStyleSettings.SetLightColor( aLight );

            Color aShadow( nBackgroundColor );
            aShadow.DecreaseLuminance( static_cast<sal_uInt8>( nBackgroundLuminance * 1 / 3 ) );
            aStyleSettings.SetShadowColor( aShadow );

            Color aDarkShadow( nBackgroundColor );
            aDarkShadow.DecreaseLuminance( static_cast<sal_uInt8>( nBackgroundLuminance * 2 / 3 ) );
            aStyleSettings.SetDarkShadowColor( aDarkShadow );
        }

        aSettings.SetStyleSettings( aStyleSettings );
        _pWindow->SetSettings( aSettings, true );
    }

    Any getButtonLikeFaceColor( const vcl::Window* _pWindow )
    {
        Color nBackgroundColor = _pWindow->GetSettings().GetStyleSettings().GetFaceColor();
        return makeAny( sal_Int32(nBackgroundColor) );
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

    static Any getVisualEffect( vcl::Window const * _pWindow )
    {
        Any aEffect;

        StyleSettings aStyleSettings = _pWindow->GetSettings().GetStyleSettings();
        if ( aStyleSettings.GetOptions() & StyleSettingsOptions::Mono )
            aEffect <<= sal_Int16(FLAT);
        else
            aEffect <<= sal_Int16(LOOK3D);
        return aEffect;
    }
}




void VCLXGraphicControl::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    VCLXWindow::ImplGetPropertyIds( rIds );
}

void VCLXGraphicControl::ImplSetNewImage()
{
    OSL_PRECOND( GetWindow(), "VCLXGraphicControl::ImplSetNewImage: window is required to be not-NULL!" );
    VclPtr< Button > pButton = GetAsDynamic< Button >();
    pButton->SetModeImage( GetImage() );
}

void VCLXGraphicControl::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags )
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

void VCLXGraphicControl::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
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
            if (  ( eType == WindowType::PUSHBUTTON )
               || ( eType == WindowType::RADIOBUTTON )
               || ( eType == WindowType::CHECKBOX )
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
            if (  ( eType == WindowType::PUSHBUTTON )
               || ( eType == WindowType::RADIOBUTTON )
               || ( eType == WindowType::CHECKBOX )
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

css::uno::Any VCLXGraphicControl::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
    if ( !GetWindow() )
        return aProp;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_GRAPHIC:
            aProp <<= Graphic(maImage.GetBitmapEx()).GetXGraphic();
            break;
        case BASEPROPERTY_IMAGEALIGN:
        {
            WindowType eType = GetWindow()->GetType();
            if  (  ( eType == WindowType::PUSHBUTTON )
                || ( eType == WindowType::RADIOBUTTON )
                || ( eType == WindowType::CHECKBOX )
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
            if  (  ( eType == WindowType::PUSHBUTTON )
                || ( eType == WindowType::RADIOBUTTON )
                || ( eType == WindowType::CHECKBOX )
                )
            {
                aProp <<= ::toolkit::translateImagePosition(
                        GetAs< Button >()->GetImageAlign() );
            }
        }
        break;
        default:
        {
            aProp = VCLXWindow::getProperty( PropertyName );
        }
        break;
    }
    return aProp;
}




void VCLXButton::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

css::uno::Reference< css::accessibility::XAccessibleContext > VCLXButton::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXButton::dispose()
{
    SolarMutexGuard aGuard;

    css::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maActionListeners.disposeAndClear( aObj );
    maItemListeners.disposeAndClear( aObj );
    VCLXGraphicControl::dispose();
}

void VCLXButton::addActionListener( const css::uno::Reference< css::awt::XActionListener > & l  )
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXButton::removeActionListener( const css::uno::Reference< css::awt::XActionListener > & l )
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXButton::addItemListener( const css::uno::Reference< css::awt::XItemListener > & l  )
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXButton::removeItemListener( const css::uno::Reference< css::awt::XItemListener > & l )
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXButton::setLabel( const OUString& rLabel )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rLabel );
}

void VCLXButton::setActionCommand( const OUString& rCommand )
{
    SolarMutexGuard aGuard;

    maActionCommand = rCommand;
}

css::awt::Size VCLXButton::getMinimumSize(  )
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< PushButton > pButton = GetAs< PushButton >();
    if ( pButton )
        aSz = pButton->CalcMinimumSize();
    return AWTSize(aSz);
}

css::awt::Size VCLXButton::getPreferredSize(  )
{
    css::awt::Size aSz = getMinimumSize();
    aSz.Width += 16;
    aSz.Height += 10;
    return aSz;
}

css::awt::Size VCLXButton::calcAdjustedSize( const css::awt::Size& rNewSize )
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
                aSz.setWidth( aMinSz.Width() );
            if ( aSz.Height() < aMinSz.Height() )
                aSz.setHeight( aMinSz.Height() );
        }
        else
        {
            if ( ( aSz.Width() > aMinSz.Width() ) && ( aSz.Height() < aMinSz.Height() ) )
                aSz.setHeight( aMinSz.Height() );
            else
                aSz = aMinSz;
        }
    }
    return AWTSize(aSz);
}

void VCLXButton::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< Button > pButton = GetAs< Button >();
    if ( !pButton )
        return;

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
            if ( GetWindow()->GetType() == WindowType::PUSHBUTTON )
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                    static_cast<PushButton*>(pButton.get())->SetState( static_cast<TriState>(n) );
            }
        }
        break;
        default:
        {
            VCLXGraphicControl::setProperty( PropertyName, Value );
        }
    }
}

css::uno::Any VCLXButton::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
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
                if ( GetWindow()->GetType() == WindowType::PUSHBUTTON )
                {
                     aProp <<= static_cast<sal_Int16>(static_cast<PushButton*>(pButton.get())->GetState());
                }
            }
            break;
            default:
            {
                aProp = VCLXGraphicControl::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXButton::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::ButtonClick:
        {
            css::uno::Reference< css::awt::XWindow > xKeepAlive( this );
                // since we call listeners below, there is a potential that we will be destroyed
                // during the listener call. To prevent the resulting crashes, we keep us
                // alive as long as we're here

            if ( maActionListeners.getLength() )
            {
                css::awt::ActionEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                aEvent.ActionCommand = maActionCommand;

                Callback aCallback = [ this, aEvent ]()
                                     { this->maActionListeners.actionPerformed( aEvent ); };

                ImplExecuteAsyncWithoutSolarLock( aCallback );
            }
        }
        break;

        case VclEventId::PushbuttonToggle:
        {
            PushButton& rButton = dynamic_cast< PushButton& >( *rVclWindowEvent.GetWindow() );

            css::uno::Reference< css::awt::XWindow > xKeepAlive( this );
            if ( maItemListeners.getLength() )
            {
                css::awt::ItemEvent aEvent;
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




void VCLXImageControl::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

css::awt::Size VCLXImageControl::getMinimumSize(  )
{
    SolarMutexGuard aGuard;

    Size aSz = GetImage().GetSizePixel();
    aSz = ImplCalcWindowSize( aSz );

    return AWTSize(aSz);
}

css::awt::Size VCLXImageControl::getPreferredSize(  )
{
    return getMinimumSize();
}

css::awt::Size VCLXImageControl::calcAdjustedSize( const css::awt::Size& rNewSize )
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz = rNewSize;
    css::awt::Size aMinSz = getMinimumSize();
    if ( aSz.Width < aMinSz.Width )
        aSz.Width = aMinSz.Width;
    if ( aSz.Height < aMinSz.Height )
        aSz.Height = aMinSz.Height;
    return aSz;
}

void VCLXImageControl::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
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

css::uno::Any VCLXImageControl::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
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




void VCLXCheckBox::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

// css::uno::XInterface
css::uno::Any VCLXCheckBox::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XButton* >(this),
                                        static_cast< css::awt::XCheckBox* >(this) );
    return (aRet.hasValue() ? aRet : VCLXGraphicControl::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXCheckBox )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXCheckBox::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XButton>::get(),
        cppu::UnoType<css::awt::XCheckBox>::get(),
        VCLXGraphicControl::getTypes()
    );
    return aTypeList.getTypes();
}

css::uno::Reference< css::accessibility::XAccessibleContext > VCLXCheckBox::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXCheckBox::dispose()
{
    SolarMutexGuard aGuard;

    css::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maItemListeners.disposeAndClear( aObj );
    VCLXGraphicControl::dispose();
}

void VCLXCheckBox::addItemListener( const css::uno::Reference< css::awt::XItemListener > & l )
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXCheckBox::removeItemListener( const css::uno::Reference< css::awt::XItemListener > & l )
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXCheckBox::addActionListener( const css::uno::Reference< css::awt::XActionListener > & l  )
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXCheckBox::removeActionListener( const css::uno::Reference< css::awt::XActionListener > & l )
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXCheckBox::setActionCommand( const OUString& rCommand )
{
    SolarMutexGuard aGuard;
    maActionCommand = rCommand;
}

void VCLXCheckBox::setLabel( const OUString& rLabel )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rLabel );
}

void VCLXCheckBox::setState( sal_Int16 n )
{
    SolarMutexGuard aGuard;

    VclPtr< CheckBox> pCheckBox = GetAs< CheckBox >();
    if ( !pCheckBox)
        return;

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

sal_Int16 VCLXCheckBox::getState()
{
    SolarMutexGuard aGuard;

    sal_Int16 nState = -1;
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

void VCLXCheckBox::enableTriState( sal_Bool b )
{
    SolarMutexGuard aGuard;

    VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
    if ( pCheckBox)
        pCheckBox->EnableTriState( b );
}

css::awt::Size VCLXCheckBox::getMinimumSize()
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
    if ( pCheckBox )
        aSz = pCheckBox->CalcMinimumSize();
    return AWTSize(aSz);
}

css::awt::Size VCLXCheckBox::getPreferredSize()
{
    return getMinimumSize();
}

css::awt::Size VCLXCheckBox::calcAdjustedSize( const css::awt::Size& rNewSize )
{
    SolarMutexGuard aGuard;

    Size aSz = VCLSize(rNewSize);
    VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
    if ( pCheckBox )
    {
        Size aMinSz = pCheckBox->CalcMinimumSize(rNewSize.Width);
        if ( ( aSz.Width() > aMinSz.Width() ) && ( aSz.Height() < aMinSz.Height() ) )
            aSz.setHeight( aMinSz.Height() );
        else
            aSz = aMinSz;
    }
    return AWTSize(aSz);
}

void VCLXCheckBox::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
    if ( !pCheckBox )
        return;

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

css::uno::Any VCLXCheckBox::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
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
                 aProp <<= static_cast<sal_Int16>(pCheckBox->GetState());
                break;
            default:
            {
                aProp = VCLXGraphicControl::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXCheckBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::CheckboxToggle:
        {
            css::uno::Reference< css::awt::XWindow > xKeepAlive( this );
                // since we call listeners below, there is a potential that we will be destroyed
                // in during the listener call. To prevent the resulting crashes, we keep us
                // alive as long as we're here

            VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
            if ( pCheckBox )
            {
                if ( maItemListeners.getLength() )
                {
                    css::awt::ItemEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                    aEvent.Highlighted = 0;
                    aEvent.Selected = pCheckBox->GetState();
                    maItemListeners.itemStateChanged( aEvent );
                }
                if ( !IsSynthesizingVCLEvent() && maActionListeners.getLength() )
                {
                    css::awt::ActionEvent aEvent;
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



void VCLXRadioButton::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

// css::uno::XInterface
css::uno::Any VCLXRadioButton::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XRadioButton* >(this),
                                        static_cast< css::awt::XButton* >(this) );
    return (aRet.hasValue() ? aRet : VCLXGraphicControl::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXRadioButton )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXRadioButton::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XRadioButton>::get(),
        cppu::UnoType<css::awt::XButton>::get(),
        VCLXGraphicControl::getTypes()
    );
    return aTypeList.getTypes();
}

css::uno::Reference< css::accessibility::XAccessibleContext > VCLXRadioButton::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXRadioButton::dispose()
{
    SolarMutexGuard aGuard;

    css::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maItemListeners.disposeAndClear( aObj );
    VCLXGraphicControl::dispose();
}

void VCLXRadioButton::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< RadioButton > pButton = GetAs< RadioButton >();
    if ( !pButton )
        return;

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

css::uno::Any VCLXRadioButton::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
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
                aProp <<= static_cast<sal_Int16>( pButton->IsChecked() ? 1 : 0 );
                break;
            case BASEPROPERTY_AUTOTOGGLE:
                aProp <<= pButton->IsRadioCheckEnabled();
                break;
            default:
            {
                aProp = VCLXGraphicControl::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXRadioButton::addItemListener( const css::uno::Reference< css::awt::XItemListener > & l )
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXRadioButton::removeItemListener( const css::uno::Reference< css::awt::XItemListener > & l )
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXRadioButton::addActionListener( const css::uno::Reference< css::awt::XActionListener > & l  )
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXRadioButton::removeActionListener( const css::uno::Reference< css::awt::XActionListener > & l )
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXRadioButton::setLabel( const OUString& rLabel )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rLabel );
}

void VCLXRadioButton::setActionCommand( const OUString& rCommand )
{
    SolarMutexGuard aGuard;
    maActionCommand = rCommand;
}

void VCLXRadioButton::setState( sal_Bool b )
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

sal_Bool VCLXRadioButton::getState()
{
    SolarMutexGuard aGuard;

    VclPtr< RadioButton > pRadioButton = GetAs< RadioButton >();
    return pRadioButton && pRadioButton->IsChecked();
}

css::awt::Size VCLXRadioButton::getMinimumSize(  )
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< RadioButton > pRadioButton = GetAs< RadioButton >();
    if ( pRadioButton )
        aSz = pRadioButton->CalcMinimumSize();
    return AWTSize(aSz);
}

css::awt::Size VCLXRadioButton::getPreferredSize(  )
{
    return getMinimumSize();
}

css::awt::Size VCLXRadioButton::calcAdjustedSize( const css::awt::Size& rNewSize )
{
    SolarMutexGuard aGuard;

    Size aSz = VCLSize(rNewSize);
    VclPtr< RadioButton > pRadioButton = GetAs< RadioButton >();
    if ( pRadioButton )
    {
        Size aMinSz = pRadioButton->CalcMinimumSize(rNewSize.Width);
        if ( ( aSz.Width() > aMinSz.Width() ) && ( aSz.Height() < aMinSz.Height() ) )
            aSz.setHeight( aMinSz.Height() );
        else
            aSz = aMinSz;
    }
    return AWTSize(aSz);
}

void VCLXRadioButton::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    css::uno::Reference< css::awt::XWindow > xKeepAlive( this );
        // since we call listeners below, there is a potential that we will be destroyed
        // in during the listener call. To prevent the resulting crashes, we keep us
        // alive as long as we're here

    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::ButtonClick:
            if ( !IsSynthesizingVCLEvent() && maActionListeners.getLength() )
            {
                css::awt::ActionEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                aEvent.ActionCommand = maActionCommand;
                maActionListeners.actionPerformed( aEvent );
            }
            ImplClickedOrToggled( false );
            break;

        case VclEventId::RadiobuttonToggle:
            ImplClickedOrToggled( true );
            break;

        default:
            VCLXGraphicControl::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

void VCLXRadioButton::ImplClickedOrToggled( bool bToggled )
{
    // In the forms, RadioChecked is not enabled, call itemStateChanged only for click
    // In the dialog editor, RadioChecked is enabled, call itemStateChanged only for bToggled
    VclPtr< RadioButton > pRadioButton = GetAs< RadioButton >();
    if ( pRadioButton && ( pRadioButton->IsRadioCheckEnabled() == bToggled ) && ( bToggled || pRadioButton->IsStateChanged() ) && maItemListeners.getLength() )
    {
        css::awt::ItemEvent aEvent;
        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
        aEvent.Highlighted = 0;
        aEvent.Selected = pRadioButton->IsChecked() ? 1 : 0;
        maItemListeners.itemStateChanged( aEvent );
    }
}



void VCLXSpinField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR,
                     0 );
    VCLXEdit::ImplGetPropertyIds( rIds );
}

VCLXSpinField::VCLXSpinField() : maSpinListeners( *this )
{
}

// css::uno::XInterface
css::uno::Any VCLXSpinField::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XSpinField* >(this) );
    return (aRet.hasValue() ? aRet : VCLXEdit::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXSpinField )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXSpinField::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XSpinField>::get(),
        VCLXEdit::getTypes()
    );
    return aTypeList.getTypes();
}

void VCLXSpinField::addSpinListener( const css::uno::Reference< css::awt::XSpinListener > & l )
{
    SolarMutexGuard aGuard;
    maSpinListeners.addInterface( l );
}

void VCLXSpinField::removeSpinListener( const css::uno::Reference< css::awt::XSpinListener > & l )
{
    SolarMutexGuard aGuard;
    maSpinListeners.removeInterface( l );
}

void VCLXSpinField::up()
{
    SolarMutexGuard aGuard;

    VclPtr< SpinField > pSpinField = GetAs< SpinField >();
    if ( pSpinField )
        pSpinField->Up();
}

void VCLXSpinField::down()
{
    SolarMutexGuard aGuard;

    VclPtr< SpinField > pSpinField = GetAs< SpinField >();
    if ( pSpinField )
        pSpinField->Down();
}

void VCLXSpinField::first()
{
    SolarMutexGuard aGuard;

    VclPtr< SpinField > pSpinField = GetAs< SpinField >();
    if ( pSpinField )
        pSpinField->First();
}

void VCLXSpinField::last()
{
    SolarMutexGuard aGuard;

    VclPtr< SpinField > pSpinField = GetAs< SpinField >();
    if ( pSpinField )
        pSpinField->Last();
}

void VCLXSpinField::enableRepeat( sal_Bool bRepeat )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pWindow = GetWindow();
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
        case VclEventId::SpinfieldUp:
        case VclEventId::SpinfieldDown:
        case VclEventId::SpinfieldFirst:
        case VclEventId::SpinfieldLast:
        {
            css::uno::Reference< css::awt::XWindow > xKeepAlive( this );
                // since we call listeners below, there is a potential that we will be destroyed
                // in during the listener call. To prevent the resulting crashes, we keep us
                // alive as long as we're here

            if ( maSpinListeners.getLength() )
            {
                css::awt::SpinEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                switch ( rVclWindowEvent.GetId() )
                {
                    case VclEventId::SpinfieldUp:     maSpinListeners.up( aEvent );
                                                    break;
                    case VclEventId::SpinfieldDown:   maSpinListeners.down( aEvent );
                                                    break;
                    case VclEventId::SpinfieldFirst:  maSpinListeners.first( aEvent );
                                                    break;
                    case VclEventId::SpinfieldLast:   maSpinListeners.last( aEvent );
                                                    break;
                    default: break;
                }

            }
        }
        break;

        default:
            VCLXEdit::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}



void VCLXListBox::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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
                     BASEPROPERTY_TYPEDITEMLIST,
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

void VCLXListBox::dispose()
{
    SolarMutexGuard aGuard;

    css::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maItemListeners.disposeAndClear( aObj );
    maActionListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void VCLXListBox::addItemListener( const css::uno::Reference< css::awt::XItemListener > & l )
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXListBox::removeItemListener( const css::uno::Reference< css::awt::XItemListener > & l )
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXListBox::addActionListener( const css::uno::Reference< css::awt::XActionListener > & l )
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXListBox::removeActionListener( const css::uno::Reference< css::awt::XActionListener > & l )
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXListBox::addItem( const OUString& aItem, sal_Int16 nPos )
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        pBox->InsertEntry( aItem, nPos );
}

void VCLXListBox::addItems( const css::uno::Sequence< OUString>& aItems, sal_Int16 nPos )
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( !pBox )
        return;

    sal_uInt16 nP = nPos;
    for ( auto const & item : aItems )
    {
        if ( nP == 0xFFFF )
        {
            OSL_FAIL( "VCLXListBox::addItems: too many entries!" );
            // skip remaining entries, list cannot hold them, anyway
            break;
        }

        pBox->InsertEntry( item, nP++ );
    }
}

void VCLXListBox::removeItems( sal_Int16 nPos, sal_Int16 nCount )
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
    {
        for ( sal_Int16 n = nCount; n; )
            pBox->RemoveEntry( nPos + (--n) );
    }
}

sal_Int16 VCLXListBox::getItemCount()
{
    SolarMutexGuard aGuard;

    VclPtr< ListBox > pBox = GetAs< ListBox >();
    return pBox ? pBox->GetEntryCount() : 0;
}

OUString VCLXListBox::getItem( sal_Int16 nPos )
{
    SolarMutexGuard aGuard;

    OUString aItem;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        aItem = pBox->GetEntry( nPos );
    return aItem;
}

css::uno::Sequence< OUString> VCLXListBox::getItems()
{
    SolarMutexGuard aGuard;

    css::uno::Sequence< OUString> aSeq;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
    {
        auto n = pBox->GetEntryCount();
        aSeq = css::uno::Sequence< OUString>( n );
        while (n)
        {
            --n;
            aSeq.getArray()[n] = pBox->GetEntry( n );
        }
    }
    return aSeq;
}

sal_Int16 VCLXListBox::getSelectedItemPos()
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    return pBox ? pBox->GetSelectedEntryPos() : 0;
}

css::uno::Sequence<sal_Int16> VCLXListBox::getSelectedItemsPos()
{
    SolarMutexGuard aGuard;

    css::uno::Sequence<sal_Int16> aSeq;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
    {
        const sal_Int32 nSelEntries = pBox->GetSelectedEntryCount();
        aSeq = css::uno::Sequence<sal_Int16>( nSelEntries );
        for ( sal_Int32 n = 0; n < nSelEntries; ++n )
            aSeq.getArray()[n] = pBox->GetSelectedEntryPos( n );
    }
    return aSeq;
}

OUString VCLXListBox::getSelectedItem()
{
    SolarMutexGuard aGuard;

    OUString aItem;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        aItem = pBox->GetSelectedEntry();
    return aItem;
}

css::uno::Sequence< OUString> VCLXListBox::getSelectedItems()
{
    SolarMutexGuard aGuard;

    css::uno::Sequence< OUString> aSeq;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
    {
        const sal_Int32 nSelEntries = pBox->GetSelectedEntryCount();
        aSeq = css::uno::Sequence< OUString>( nSelEntries );
        for ( sal_Int32 n = 0; n < nSelEntries; ++n )
            aSeq.getArray()[n] = pBox->GetSelectedEntry( n );
    }
    return aSeq;
}

void VCLXListBox::selectItemPos( sal_Int16 nPos, sal_Bool bSelect )
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

void VCLXListBox::selectItemsPos( const css::uno::Sequence<sal_Int16>& aPositions, sal_Bool bSelect )
{
    SolarMutexGuard aGuard;

    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( !pBox )
        return;

    std::vector<sal_Int32> aPositionVec;
    aPositionVec.reserve(aPositions.getLength());

    bool bChanged = false;
    for ( auto n = aPositions.getLength(); n; )
    {
        const auto nPos = aPositions.getConstArray()[--n];
        if ( pBox->IsEntryPosSelected( nPos ) != bool(bSelect) )
        {
            aPositionVec.push_back(nPos);
            bChanged = true;
        }
    }

    if ( !bChanged )
        return;

    bool bOrigUpdateMode = pBox->IsUpdateMode();
    pBox->SetUpdateMode(false);

    pBox->SelectEntriesPos(aPositionVec, bSelect);

    pBox->SetUpdateMode(bOrigUpdateMode);

    // VCL doesn't call select handler after API call.
    // ImplCallItemListeners();

    // #107218# Call same listeners like VCL would do after user interaction
    SetSynthesizingVCLEvent( true );
    pBox->Select();
    SetSynthesizingVCLEvent( false );
}

void VCLXListBox::selectItem( const OUString& rItemText, sal_Bool bSelect )
{
    SolarMutexGuard aGuard;

    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
    {
        selectItemPos( pBox->GetEntryPos( rItemText ), bSelect );
    }
}

void VCLXListBox::setDropDownLineCount( sal_Int16 nLines )
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        pBox->SetDropDownLineCount( nLines );
}

sal_Int16 VCLXListBox::getDropDownLineCount()
{
    SolarMutexGuard aGuard;

    sal_Int16 nLines = 0;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        nLines = pBox->GetDropDownLineCount();
    return nLines;
}

sal_Bool VCLXListBox::isMutipleMode()
{
    SolarMutexGuard aGuard;
    bool bMulti = false;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        bMulti = pBox->IsMultiSelectionEnabled();
    return bMulti;
}

void VCLXListBox::setMultipleMode( sal_Bool bMulti )
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        pBox->EnableMultiSelection( bMulti );
}

void VCLXListBox::makeVisible( sal_Int16 nEntry )
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if ( pBox )
        pBox->SetTopEntry( nEntry );
}

void VCLXListBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    css::uno::Reference< css::awt::XWindow > xKeepAlive( this );
        // since we call listeners below, there is a potential that we will be destroyed
        // in during the listener call. To prevent the resulting crashes, we keep us
        // alive as long as we're here

    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::ListboxSelect:
        {
            VclPtr< ListBox > pListBox = GetAs< ListBox >();
            if( pListBox )
            {
                bool bDropDown = ( pListBox->GetStyle() & WB_DROPDOWN ) != 0;
                if ( bDropDown && !IsSynthesizingVCLEvent() && maActionListeners.getLength() )
                {
                    // Call ActionListener on DropDown event
                    css::awt::ActionEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                    aEvent.ActionCommand = pListBox->GetSelectedEntry();
                    maActionListeners.actionPerformed( aEvent );
                }

                if ( maItemListeners.getLength() )
                {
                    ImplCallItemListeners();
                }
            }
        }
        break;

        case VclEventId::ListboxDoubleClick:
            if ( GetWindow() && maActionListeners.getLength() )
            {
                css::awt::ActionEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                aEvent.ActionCommand = GetAs<ListBox>()->GetSelectedEntry();
                maActionListeners.actionPerformed( aEvent );
            }
            break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

css::uno::Reference< css::accessibility::XAccessibleContext > VCLXListBox::CreateAccessibleContext()
{
    SolarMutexGuard aGuard;

    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXListBox::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( !pListBox )
        return;

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
            bool b = false;
            if ( Value >>= b )
                 pListBox->SetReadOnly( b);
        }
        break;
        case BASEPROPERTY_MULTISELECTION:
        {
            bool b = false;
            if ( Value >>= b )
                 pListBox->EnableMultiSelection( b );
        }
        break;
        case BASEPROPERTY_MULTISELECTION_SIMPLEMODE:
            ::toolkit::adjustBooleanWindowStyle( Value, pListBox, WB_SIMPLEMODE, false );
            break;
        case BASEPROPERTY_LINECOUNT:
        {
            sal_Int16 n = 0;
            if ( Value >>= n )
                 pListBox->SetDropDownLineCount( n );
        }
        break;
        case BASEPROPERTY_STRINGITEMLIST:
        {
            css::uno::Sequence< OUString> aItems;
            if ( Value >>= aItems )
            {
                pListBox->Clear();
                addItems( aItems, 0 );
            }
        }
        break;
        case BASEPROPERTY_SELECTEDITEMS:
        {
            css::uno::Sequence<sal_Int16> aItems;
            if ( Value >>= aItems )
            {
                for ( auto n = pListBox->GetEntryCount(); n; )
                    pListBox->SelectEntryPos( --n, false );

                if ( aItems.hasElements() )
                    selectItemsPos( aItems, true );
                else
                    pListBox->SetNoSelection();

                if ( !pListBox->GetSelectedEntryCount() )
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

css::uno::Any VCLXListBox::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;
    css::uno::Any aProp;
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
                 aProp <<= static_cast<sal_Int16>(pListBox->GetDropDownLineCount());
            }
            break;
            case BASEPROPERTY_STRINGITEMLIST:
            {
                const sal_Int32 nItems = pListBox->GetEntryCount();
                css::uno::Sequence< OUString> aSeq( nItems );
                OUString* pStrings = aSeq.getArray();
                for ( sal_Int32 n = 0; n < nItems; ++n )
                    pStrings[n] = pListBox->GetEntry( n );
                aProp <<= aSeq;

            }
            break;
            default:
            {
                aProp = VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

css::awt::Size VCLXListBox::getMinimumSize(  )
{
    SolarMutexGuard aGuard;
    Size aSz;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( pListBox )
        aSz = pListBox->CalcMinimumSize();
    return AWTSize(aSz);
}

css::awt::Size VCLXListBox::getPreferredSize(  )
{
    SolarMutexGuard aGuard;
    Size aSz;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( pListBox )
    {
        aSz = pListBox->CalcMinimumSize();
        if ( pListBox->GetStyle() & WB_DROPDOWN )
            aSz.AdjustHeight(4 );
    }
    return AWTSize(aSz);
}

css::awt::Size VCLXListBox::calcAdjustedSize( const css::awt::Size& rNewSize )
{
    SolarMutexGuard aGuard;
    Size aSz = VCLSize(rNewSize);
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( pListBox )
        aSz = pListBox->CalcAdjustedSize( aSz );
    return AWTSize(aSz);
}

css::awt::Size VCLXListBox::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines )
{
    SolarMutexGuard aGuard;
    Size aSz;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if ( pListBox )
        aSz = pListBox->CalcBlockSize( nCols, nLines );
    return AWTSize(aSz);
}

void VCLXListBox::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines )
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
        css::awt::ItemEvent aEvent;
        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
        aEvent.Highlighted = 0;

        // Set to 0xFFFF on multiple selection, selected entry ID otherwise
        aEvent.Selected = (pListBox->GetSelectedEntryCount() == 1 ) ? pListBox->GetSelectedEntryPos() : 0xFFFF;

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
             DBG_UNHANDLED_EXCEPTION("toolkit");
        }
        return Image();
     }
}
void SAL_CALL VCLXListBox::listItemInserted( const ItemListEvent& i_rEvent )
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();

    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemInserted: no ListBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition <= pListBox->GetEntryCount() ),
        "VCLXListBox::listItemInserted: illegal (inconsistent) item position!" );
    pListBox->InsertEntry(
        i_rEvent.ItemText.IsPresent ? i_rEvent.ItemText.Value : OUString(),
        i_rEvent.ItemImageURL.IsPresent ? TkResMgr::getImageFromURL( i_rEvent.ItemImageURL.Value ) : Image(),
        i_rEvent.ItemPosition );
}

void SAL_CALL VCLXListBox::listItemRemoved( const ItemListEvent& i_rEvent )
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();

    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemRemoved: no ListBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition < pListBox->GetEntryCount() ),
        "VCLXListBox::listItemRemoved: illegal (inconsistent) item position!" );

    pListBox->RemoveEntry( i_rEvent.ItemPosition );
}

void SAL_CALL VCLXListBox::listItemModified( const ItemListEvent& i_rEvent )
{
    SolarMutexGuard aGuard;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();

    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemModified: no ListBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition < pListBox->GetEntryCount() ),
        "VCLXListBox::listItemModified: illegal (inconsistent) item position!" );

    // VCL's ListBox does not support changing an entry's text or image, so remove and re-insert

    const OUString sNewText = i_rEvent.ItemText.IsPresent ? i_rEvent.ItemText.Value : pListBox->GetEntry( i_rEvent.ItemPosition );
    const Image aNewImage( i_rEvent.ItemImageURL.IsPresent ? TkResMgr::getImageFromURL( i_rEvent.ItemImageURL.Value ) : pListBox->GetEntryImage( i_rEvent.ItemPosition  ) );

    pListBox->RemoveEntry( i_rEvent.ItemPosition );
    pListBox->InsertEntry( sNewText, aNewImage, i_rEvent.ItemPosition );
}

void SAL_CALL VCLXListBox::allItemsRemoved( const EventObject& )
{
    SolarMutexGuard aGuard;

    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemModified: no ListBox?!" );

    pListBox->Clear();
}

void SAL_CALL VCLXListBox::itemListChanged( const EventObject& i_rEvent )
{
    SolarMutexGuard aGuard;

    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemModified: no ListBox?!" );

    pListBox->Clear();

    uno::Reference< beans::XPropertySet > xPropSet( i_rEvent.Source, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySetInfo > xPSI( xPropSet->getPropertySetInfo(), uno::UNO_SET_THROW );
    uno::Reference< resource::XStringResourceResolver > xStringResourceResolver;
    if ( xPSI->hasPropertyByName("ResourceResolver") )
    {
        xStringResourceResolver.set(
            xPropSet->getPropertyValue("ResourceResolver"),
            uno::UNO_QUERY
        );
    }


    Reference< XItemList > xItemList( i_rEvent.Source, uno::UNO_QUERY_THROW );
    const uno::Sequence< beans::Pair< OUString, OUString > > aItems = xItemList->getAllItems();
    for ( const auto& rItem : aItems )
    {
        OUString aLocalizationKey( rItem.First );
        if ( xStringResourceResolver.is() && aLocalizationKey.startsWith("&") )
        {
            aLocalizationKey = xStringResourceResolver->resolveString(aLocalizationKey.copy( 1 ));
        }
        pListBox->InsertEntry( aLocalizationKey, lcl_getImageFromURL( rItem.Second ) );
    }
}

void SAL_CALL VCLXListBox::disposing( const EventObject& i_rEvent )
{
    // just disambiguate
    VCLXWindow::disposing( i_rEvent );
}




void VCLXMessageBox::GetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    VCLXTopWindow::ImplGetPropertyIds( rIds );
}

VCLXMessageBox::VCLXMessageBox()
{
}

VCLXMessageBox::~VCLXMessageBox()
{
}

// css::uno::XInterface
css::uno::Any VCLXMessageBox::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XMessageBox* >(this) );
    return (aRet.hasValue() ? aRet : VCLXTopWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXMessageBox )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXMessageBox::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XMessageBox>::get(),
        VCLXTopWindow::getTypes()
    );
    return aTypeList.getTypes();
}

void VCLXMessageBox::setCaptionText( const OUString& rText )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rText );
}

OUString VCLXMessageBox::getCaptionText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

void VCLXMessageBox::setMessageText( const OUString& rText )
{
    SolarMutexGuard aGuard;
    VclPtr< MessBox > pBox = GetAs< MessBox >();
    if ( pBox )
        pBox->SetMessText( rText );
}

OUString VCLXMessageBox::getMessageText()
{
    SolarMutexGuard aGuard;
    OUString aText;
    VclPtr< MessBox > pBox = GetAs< MessBox >();
    if ( pBox )
        aText = pBox->GetMessText();
    return aText;
}

sal_Int16 VCLXMessageBox::execute()
{
    SolarMutexGuard aGuard;
    VclPtr< MessBox > pBox = GetAs< MessBox >();
    return pBox ? pBox->Execute() : 0;
}

css::awt::Size SAL_CALL VCLXMessageBox::getMinimumSize()
{
    SolarMutexGuard aGuard;
    return css::awt::Size( 250, 100 );
}



void VCLXDialog::GetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    VCLXTopWindow::ImplGetPropertyIds( rIds );
}

VCLXDialog::VCLXDialog()
{
    SAL_INFO("toolkit", "XDialog created");
}

VCLXDialog::~VCLXDialog()
{
    SAL_INFO("toolkit", __FUNCTION__);
}

// css::uno::XInterface
css::uno::Any VCLXDialog::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XDialog2* >(this),
                                        static_cast< css::awt::XDialog* >(this) );
    return (aRet.hasValue() ? aRet : VCLXTopWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXDialog )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXDialog::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XDialog2>::get(),
        cppu::UnoType<css::awt::XDialog>::get(),
        VCLXTopWindow::getTypes()
    );
    return aTypeList.getTypes();
}

void SAL_CALL VCLXDialog::endDialog( ::sal_Int32 i_result )
{
    SolarMutexGuard aGuard;
    VclPtr<Dialog> pDialog = GetAsDynamic< Dialog >();
    if ( pDialog )
        pDialog->EndDialog( i_result );
}

void SAL_CALL VCLXDialog::setHelpId( const OUString& rId )
{
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetHelpId( OUStringToOString( rId, RTL_TEXTENCODING_UTF8 ) );
}

void VCLXDialog::setTitle( const OUString& Title )
{
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( Title );
}

OUString VCLXDialog::getTitle()
{
    SolarMutexGuard aGuard;

    OUString aTitle;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        aTitle = pWindow->GetText();
    return aTitle;
}

sal_Int16 VCLXDialog::execute()
{
    SolarMutexGuard aGuard;

    sal_Int16 nRet = 0;
    if ( GetWindow() )
    {
        VclPtr< Dialog > pDlg = GetAs< Dialog >();
        vcl::Window* pParent = pDlg->GetWindow( GetWindowType::ParentOverlap );
        vcl::Window* pOldParent = nullptr;
        vcl::Window* pSetParent = nullptr;
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

void VCLXDialog::endExecute()
{
    endDialog(0);
}

void SAL_CALL VCLXDialog::draw( sal_Int32 nX, sal_Int32 nY )
{
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );
        pWindow->Draw( pDev, aPos, DrawFlags::NoControls );
    }
}

css::awt::DeviceInfo VCLXDialog::getInfo()
{
    css::awt::DeviceInfo aInfo = VCLXDevice::getInfo();

    SolarMutexGuard aGuard;
    VclPtr< Dialog > pDlg = GetAs< Dialog >();
    if ( pDlg )
        pDlg->GetDrawWindowBorder( aInfo.LeftInset, aInfo.TopInset, aInfo.RightInset, aInfo.BottomInset );

    return aInfo;
}

void SAL_CALL VCLXDialog::setProperty(
    const OUString& PropertyName,
    const css::uno::Any& Value )
{
    SolarMutexGuard aGuard;
    VclPtr< Dialog > pDialog = GetAs< Dialog >();
    if ( !pDialog )
        return;

    bool bVoid = Value.getValueType().getTypeClass() == css::uno::TypeClass_VOID;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_GRAPHIC:
        {
            Reference< XGraphic > xGraphic;
            if (( Value >>= xGraphic ) && xGraphic.is() )
            {
                Graphic aImage(xGraphic);

                Wallpaper aWallpaper(aImage.GetBitmapEx());
                aWallpaper.SetStyle( WallpaperStyle::Scale );
                pDialog->SetBackground( aWallpaper );
            }
            else if ( bVoid || !xGraphic.is() )
            {
                Color aColor = pDialog->GetControlBackground();
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



VCLXMultiPage::VCLXMultiPage() : maTabListeners( *this ), mTabId( 1 )
{
    SAL_INFO("toolkit", "VCLXMultiPage::VCLXMultiPage()" );
}

void VCLXMultiPage::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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
void SAL_CALL VCLXMultiPage::dispose()
{
    SolarMutexGuard aGuard;

    css::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maTabListeners.disposeAndClear( aObj );
    VCLXContainer::dispose();
}
css::uno::Any SAL_CALL VCLXMultiPage::queryInterface(const css::uno::Type & rType )
{
    uno::Any aRet = ::cppu::queryInterface( rType, static_cast< awt::XSimpleTabController*>( this ) );

    return ( aRet.hasValue() ? aRet : VCLXContainer::queryInterface( rType ) );
}
IMPL_IMPLEMENTATION_ID( VCLXMultiPage )
// css::awt::XView
void SAL_CALL VCLXMultiPage::draw( sal_Int32 nX, sal_Int32 nY )
{
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );
        pWindow->Draw( pDev, aPos, DrawFlags::NoControls );
    }
}

uno::Any SAL_CALL VCLXMultiPage::getProperty( const OUString& PropertyName )
{
    SAL_INFO("toolkit", " **** VCLXMultiPage::getProperty " << PropertyName );
    SolarMutexGuard aGuard;
    css::uno::Any aProp;
    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {

        case BASEPROPERTY_MULTIPAGEVALUE:
        {
            aProp <<= getActiveTabID();
        }
        break;
        default:
            aProp = VCLXContainer::getProperty( PropertyName );
    }
    return aProp;
}

void SAL_CALL VCLXMultiPage::setProperty(
    const OUString& PropertyName,
    const css::uno::Any& Value )
{
    SAL_INFO("toolkit", " **** VCLXMultiPage::setProperty " << PropertyName );
    SolarMutexGuard aGuard;

    VclPtr< TabControl > pTabControl = GetAs< TabControl >();
    if ( !pTabControl )
        return;

    bool bVoid = Value.getValueType().getTypeClass() == css::uno::TypeClass_VOID;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_MULTIPAGEVALUE:
        {
            SAL_INFO("toolkit", "***MULTIPAGE VALUE");
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
                Graphic aImage(xGraphic);

                Wallpaper aWallpaper(aImage.GetBitmapEx());
                aWallpaper.SetStyle( WallpaperStyle::Scale );
                pTabControl->SetBackground( aWallpaper );
            }
            else if ( bVoid || !xGraphic.is() )
            {
                Color aColor = pTabControl->GetControlBackground();
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

TabControl *VCLXMultiPage::getTabControl() const
{
    VclPtr<TabControl> pTabControl = GetAsDynamic< TabControl >();
    if ( pTabControl )
        return pTabControl;
    throw uno::RuntimeException();
}
sal_Int32 SAL_CALL VCLXMultiPage::insertTab()
{
    TabControl *pTabControl = getTabControl();
    VclPtrInstance<TabPage> pTab( pTabControl );
    return static_cast< sal_Int32 >( insertTab( pTab, OUString() ) );
}

sal_uInt16 VCLXMultiPage::insertTab( TabPage* pPage, OUString const & sTitle )
{
    TabControl *pTabControl = getTabControl();
    sal_uInt16 id = sal::static_int_cast< sal_uInt16 >( mTabId++ );
    pTabControl->InsertPage( id, sTitle );
    pTabControl->SetTabPage( id, pPage );
    return id;
}

void SAL_CALL VCLXMultiPage::removeTab( sal_Int32 ID )
{
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == nullptr )
        throw lang::IndexOutOfBoundsException();
    pTabControl->RemovePage( sal::static_int_cast< sal_uInt16 >( ID ) );
}

void SAL_CALL VCLXMultiPage::activateTab( sal_Int32 ID )
{
    TabControl *pTabControl = getTabControl();
    SAL_INFO(
        "toolkit",
        "Attempting to activate tab " << ID << ", active tab is "
            << getActiveTabID() << ", numtabs is " << getWindows().getLength());
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == nullptr )
        throw lang::IndexOutOfBoundsException();
    pTabControl->SelectTabPage( sal::static_int_cast< sal_uInt16 >( ID ) );
}

sal_Int32 SAL_CALL VCLXMultiPage::getActiveTabID()
{
    return getTabControl()->GetCurPageId( );
}

void SAL_CALL VCLXMultiPage::addTabListener( const uno::Reference< awt::XTabListener >& xListener )
{
    SolarMutexGuard aGuard;
    maTabListeners.addInterface( xListener );
}

void SAL_CALL VCLXMultiPage::removeTabListener( const uno::Reference< awt::XTabListener >& xListener )
{
    SolarMutexGuard aGuard;
    maTabListeners.addInterface( xListener );
}

void SAL_CALL VCLXMultiPage::setTabProps( sal_Int32 ID, const uno::Sequence< beans::NamedValue >& Properties )
{
    SolarMutexGuard aGuard;
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == nullptr )
        throw lang::IndexOutOfBoundsException();

    for (const auto& rProp : Properties)
    {
        const OUString &name = rProp.Name;
        const uno::Any &value = rProp.Value;

        if (name == "Title")
        {
            OUString title = value.get<OUString>();
            pTabControl->SetPageText( sal::static_int_cast< sal_uInt16 >( ID ), title );
        }
    }
}

uno::Sequence< beans::NamedValue > SAL_CALL VCLXMultiPage::getTabProps( sal_Int32 ID )
{
    SolarMutexGuard aGuard;
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == nullptr )
        throw lang::IndexOutOfBoundsException();

    uno::Sequence< beans::NamedValue > props
    {
        { "Title",    css::uno::makeAny(pTabControl->GetPageText( sal::static_int_cast< sal_uInt16 >( ID ) )) },
        { "Position", css::uno::makeAny(pTabControl->GetPagePos( sal::static_int_cast< sal_uInt16 >( ID ) )) }
    };
    return props;
}
void VCLXMultiPage::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    css::uno::Reference< css::awt::XWindow > xKeepAlive( this );
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::TabpageDeactivate:
        {
            sal_uLong nPageID = reinterpret_cast<sal_uLong>( rVclWindowEvent.GetData() );
            maTabListeners.deactivated( nPageID );
            break;

        }
        case VclEventId::TabpageActivate:
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



VCLXTabPage::VCLXTabPage()
{
}

void VCLXTabPage::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

IMPL_IMPLEMENTATION_ID( VCLXTabPage )

// css::awt::XView
void SAL_CALL VCLXTabPage::draw( sal_Int32 nX, sal_Int32 nY )
{
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );
        pWindow->Draw( pDev, aPos, DrawFlags::NoControls );
    }
}

void SAL_CALL VCLXTabPage::setProperty(
    const OUString& PropertyName,
    const css::uno::Any& Value )
{
    SolarMutexGuard aGuard;
    VclPtr< TabPage > pTabPage = GetAs< TabPage >();
    if ( !pTabPage )
        return;

    bool bVoid = Value.getValueType().getTypeClass() == css::uno::TypeClass_VOID;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_GRAPHIC:
        {
            Reference< XGraphic > xGraphic;
            if (( Value >>= xGraphic ) && xGraphic.is() )
            {
                Graphic aImage(xGraphic);

                Wallpaper aWallpaper(aImage.GetBitmapEx());
                aWallpaper.SetStyle( WallpaperStyle::Scale );
                pTabPage->SetBackground( aWallpaper );
            }
            else if ( bVoid || !xGraphic.is() )
            {
                Color aColor = pTabPage->GetControlBackground();
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

TabPage *VCLXTabPage::getTabPage() const
{
    VclPtr< TabPage > pTabPage = GetAsDynamic< TabPage >();
    if ( pTabPage )
        return pTabPage;
    throw uno::RuntimeException();
}




VCLXFixedHyperlink::VCLXFixedHyperlink() :

    maActionListeners( *this )

{
}

VCLXFixedHyperlink::~VCLXFixedHyperlink()
{
}

// css::uno::XInterface
css::uno::Any VCLXFixedHyperlink::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XFixedHyperlink* >(this) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

void VCLXFixedHyperlink::dispose()
{
        SolarMutexGuard aGuard;

        css::lang::EventObject aObj;
        aObj.Source = static_cast<cppu::OWeakObject*>(this);
        maActionListeners.disposeAndClear( aObj );
        VCLXWindow::dispose();
}

IMPL_IMPLEMENTATION_ID( VCLXFixedHyperlink )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXFixedHyperlink::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XFixedHyperlink>::get(),
        VCLXWindow::getTypes()
    );
    return aTypeList.getTypes();
}

void VCLXFixedHyperlink::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::ButtonClick:
        {
            if ( maActionListeners.getLength() )
            {
                css::awt::ActionEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                maActionListeners.actionPerformed( aEvent );
            }
            [[fallthrough]];
        }
        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

css::uno::Reference< css::accessibility::XAccessibleContext > VCLXFixedHyperlink::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXFixedHyperlink::setText( const OUString& Text )
{
    SolarMutexGuard aGuard;
    VclPtr< FixedHyperlink > pBase = GetAs< FixedHyperlink >();
    if (pBase)
        pBase->SetText(Text);
}

OUString VCLXFixedHyperlink::getText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

void VCLXFixedHyperlink::setURL( const OUString& URL )
{
    SolarMutexGuard aGuard;
    VclPtr< FixedHyperlink > pBase = GetAs< FixedHyperlink >();
    if ( pBase )
        pBase->SetURL( URL );
}

OUString VCLXFixedHyperlink::getURL(  )
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< FixedHyperlink > pBase = GetAs< FixedHyperlink >();
    if ( pBase )
        aText = pBase->GetURL();
    return aText;
}

void VCLXFixedHyperlink::setAlignment( sal_Int16 nAlign )
{
    SolarMutexGuard aGuard;

    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( !pWindow )
        return;

    WinBits nNewBits = 0;
    if ( nAlign == css::awt::TextAlign::LEFT )
        nNewBits = WB_LEFT;
    else if ( nAlign == css::awt::TextAlign::CENTER )
        nNewBits = WB_CENTER;
    else
        nNewBits = WB_RIGHT;

    WinBits nStyle = pWindow->GetStyle();
    nStyle &= ~(WB_LEFT|WB_CENTER|WB_RIGHT);
    pWindow->SetStyle( nStyle | nNewBits );
}

sal_Int16 VCLXFixedHyperlink::getAlignment()
{
    SolarMutexGuard aGuard;

    sal_Int16 nAlign = 0;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        if ( nStyle & WB_LEFT )
            nAlign = css::awt::TextAlign::LEFT;
        else if ( nStyle & WB_CENTER )
            nAlign = css::awt::TextAlign::CENTER;
        else
            nAlign = css::awt::TextAlign::RIGHT;
    }
    return nAlign;
}

void VCLXFixedHyperlink::addActionListener( const css::uno::Reference< css::awt::XActionListener > & l  )
{
        SolarMutexGuard aGuard;
        maActionListeners.addInterface( l );
}

void VCLXFixedHyperlink::removeActionListener( const css::uno::Reference< css::awt::XActionListener > & l )
{
        SolarMutexGuard aGuard;
        maActionListeners.removeInterface( l );
}

css::awt::Size VCLXFixedHyperlink::getMinimumSize(  )
{
    SolarMutexGuard aGuard;
    Size aSz;
    VclPtr< FixedText > pFixedText = GetAs< FixedText >();
    if ( pFixedText )
        aSz = pFixedText->CalcMinimumSize();
    return AWTSize(aSz);
}

css::awt::Size VCLXFixedHyperlink::getPreferredSize(  )
{
    return getMinimumSize();
}

css::awt::Size VCLXFixedHyperlink::calcAdjustedSize( const css::awt::Size& rNewSize )
{
    SolarMutexGuard aGuard;
    Size aSz( VCLUnoHelper::ConvertToVCLSize( rNewSize ));
    VclPtr< FixedText > pFixedText = GetAs< FixedText >();
    if (pFixedText)
    {
        Size aMinSz = pFixedText->CalcMinimumSize(rNewSize.Width);
        if ( ( aSz.Width() > aMinSz.Width() ) && ( aSz.Height() < aMinSz.Height() ) )
            aSz.setHeight( aMinSz.Height() );
        else
            aSz = aMinSz;
    }

    return VCLUnoHelper::ConvertToAWTSize(aSz);
}

void VCLXFixedHyperlink::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< FixedHyperlink > pBase = GetAs< FixedHyperlink >();
    if ( !pBase )
        return;

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

css::uno::Any VCLXFixedHyperlink::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
    VclPtr< FixedHyperlink > pBase = GetAs< FixedHyperlink >();
    if ( pBase )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_URL:
            {
                aProp <<= pBase->GetURL();
                break;
            }

            default:
            {
                aProp = VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXFixedHyperlink::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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



void VCLXFixedText::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

// css::uno::XInterface
css::uno::Any VCLXFixedText::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XFixedText* >(this) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXFixedText )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXFixedText::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XFixedText>::get(),
        VCLXWindow::getTypes()
    );
    return aTypeList.getTypes();
}

css::uno::Reference< css::accessibility::XAccessibleContext > VCLXFixedText::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXFixedText::setText( const OUString& Text )
{
    SolarMutexGuard aGuard;

    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( Text );
}

OUString VCLXFixedText::getText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

void VCLXFixedText::setAlignment( sal_Int16 nAlign )
{
    SolarMutexGuard aGuard;

    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( !pWindow )
        return;

    WinBits nNewBits = 0;
    if ( nAlign == css::awt::TextAlign::LEFT )
        nNewBits = WB_LEFT;
    else if ( nAlign == css::awt::TextAlign::CENTER )
        nNewBits = WB_CENTER;
    else
        nNewBits = WB_RIGHT;

    WinBits nStyle = pWindow->GetStyle();
    nStyle &= ~(WB_LEFT|WB_CENTER|WB_RIGHT);
    pWindow->SetStyle( nStyle | nNewBits );
}

sal_Int16 VCLXFixedText::getAlignment()
{
    SolarMutexGuard aGuard;

    sal_Int16 nAlign = 0;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        if ( nStyle & WB_LEFT )
            nAlign = css::awt::TextAlign::LEFT;
        else if ( nStyle & WB_CENTER )
            nAlign = css::awt::TextAlign::CENTER;
        else
            nAlign = css::awt::TextAlign::RIGHT;
    }
    return nAlign;
}

css::awt::Size VCLXFixedText::getMinimumSize(  )
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< FixedText > pFixedText = GetAs< FixedText >();
    if ( pFixedText )
        aSz = pFixedText->CalcMinimumSize();
    return AWTSize(aSz);
}

css::awt::Size VCLXFixedText::getPreferredSize(  )
{
    return getMinimumSize();
}

css::awt::Size VCLXFixedText::calcAdjustedSize( const css::awt::Size& rMaxSize )
{
    SolarMutexGuard aGuard;

    Size aAdjustedSize( VCLUnoHelper::ConvertToVCLSize( rMaxSize ) );
    VclPtr< FixedText > pFixedText = GetAs< FixedText >();
    if ( pFixedText )
        aAdjustedSize = pFixedText->CalcMinimumSize( rMaxSize.Width );
    return VCLUnoHelper::ConvertToAWTSize( aAdjustedSize );
}



void VCLXScrollBar::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

// css::uno::XInterface
css::uno::Any VCLXScrollBar::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XScrollBar* >(this) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXScrollBar )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXScrollBar::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XScrollBar>::get(),
        VCLXWindow::getTypes()
    );
    return aTypeList.getTypes();
}

css::uno::Reference< css::accessibility::XAccessibleContext > VCLXScrollBar::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

// css::lang::XComponent
void VCLXScrollBar::dispose()
{
    SolarMutexGuard aGuard;

    css::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maAdjustmentListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

// css::awt::XScrollbar
void VCLXScrollBar::addAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener > & l )
{
    SolarMutexGuard aGuard;
    maAdjustmentListeners.addInterface( l );
}

void VCLXScrollBar::removeAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener > & l )
{
    SolarMutexGuard aGuard;
    maAdjustmentListeners.removeInterface( l );
}

void VCLXScrollBar::setValue( sal_Int32 n )
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
        pScrollBar->DoScroll( n );
}

void VCLXScrollBar::setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax )
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

sal_Int32 VCLXScrollBar::getValue()
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    return pScrollBar ? pScrollBar->GetThumbPos() : 0;
}

void VCLXScrollBar::setMaximum( sal_Int32 n )
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
        pScrollBar->SetRangeMax( n );
}

sal_Int32 VCLXScrollBar::getMaximum()
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    return pScrollBar ? pScrollBar->GetRangeMax() : 0;
}

void VCLXScrollBar::setMinimum( sal_Int32 n )
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
        pScrollBar->SetRangeMin( n );
}

sal_Int32 VCLXScrollBar::getMinimum() const
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    return pScrollBar ? pScrollBar->GetRangeMin() : 0;
}

void VCLXScrollBar::setLineIncrement( sal_Int32 n )
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
        pScrollBar->SetLineSize( n );
}

sal_Int32 VCLXScrollBar::getLineIncrement()
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    return pScrollBar ? pScrollBar->GetLineSize() : 0;
}

void VCLXScrollBar::setBlockIncrement( sal_Int32 n )
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
        pScrollBar->SetPageSize( n );
}

sal_Int32 VCLXScrollBar::getBlockIncrement()
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    return pScrollBar ? pScrollBar->GetPageSize() : 0;
}

void VCLXScrollBar::setVisibleSize( sal_Int32 n )
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
        pScrollBar->SetVisibleSize( n );
}

sal_Int32 VCLXScrollBar::getVisibleSize()
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    return pScrollBar ? pScrollBar->GetVisibleSize() : 0;
}

void VCLXScrollBar::setOrientation( sal_Int32 n )
{
    SolarMutexGuard aGuard;

    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        nStyle &= ~(WB_HORZ|WB_VERT);
        if ( n == css::awt::ScrollBarOrientation::HORIZONTAL )
            nStyle |= WB_HORZ;
        else
            nStyle |= WB_VERT;

        pWindow->SetStyle( nStyle );
        pWindow->Resize();
    }
}

sal_Int32 VCLXScrollBar::getOrientation()
{
    SolarMutexGuard aGuard;

    sal_Int32 n = 0;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        if ( nStyle & WB_HORZ )
            n = css::awt::ScrollBarOrientation::HORIZONTAL;
        else
            n = css::awt::ScrollBarOrientation::VERTICAL;
    }
    return n;

}

// css::awt::VclWindowPeer
void VCLXScrollBar::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( !pScrollBar )
        return;

    bool bVoid = Value.getValueType().getTypeClass() == css::uno::TypeClass_VOID;

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

css::uno::Any VCLXScrollBar::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
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
                aProp <<= getValue();
            }
            break;
            case BASEPROPERTY_SCROLLVALUE_MAX:
            {
                aProp <<= getMaximum();
            }
            break;
            case BASEPROPERTY_SCROLLVALUE_MIN:
            {
                aProp <<= getMinimum();
            }
            break;
            case BASEPROPERTY_LINEINCREMENT:
            {
                aProp <<= getLineIncrement();
            }
            break;
            case BASEPROPERTY_BLOCKINCREMENT:
            {
                aProp <<= getBlockIncrement();
            }
            break;
            case BASEPROPERTY_VISIBLESIZE:
            {
                aProp <<= getVisibleSize();
            }
            break;
            case BASEPROPERTY_ORIENTATION:
            {
                aProp <<= getOrientation();
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
                aProp = VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXScrollBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::ScrollbarScroll:
        {
            css::uno::Reference< css::awt::XWindow > xKeepAlive( this );
                // since we call listeners below, there is a potential that we will be destroyed
                // in during the listener call. To prevent the resulting crashes, we keep us
                // alive as long as we're here

            if ( maAdjustmentListeners.getLength() )
            {
                VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();

                if( pScrollBar )
                {
                    css::awt::AdjustmentEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                    aEvent.Value = pScrollBar->GetThumbPos();

                    // set adjustment type
                    ScrollType aType = pScrollBar->GetType();
                    if ( aType == ScrollType::LineUp || aType == ScrollType::LineDown )
                    {
                        aEvent.Type = css::awt::AdjustmentType_ADJUST_LINE;
                    }
                    else if ( aType == ScrollType::PageUp || aType == ScrollType::PageDown )
                    {
                        aEvent.Type = css::awt::AdjustmentType_ADJUST_PAGE;
                    }
                    else if ( aType == ScrollType::Drag )
                    {
                        aEvent.Type = css::awt::AdjustmentType_ADJUST_ABS;
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

css::awt::Size VCLXScrollBar::implGetMinimumSize( vcl::Window const * p )
{
    tools::Long n = p->GetSettings().GetStyleSettings().GetScrollBarSize();
    return css::awt::Size( n, n );
}

css::awt::Size SAL_CALL VCLXScrollBar::getMinimumSize()
{
    SolarMutexGuard aGuard;
    return implGetMinimumSize( GetWindow() );
}




void VCLXEdit::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

// css::uno::XInterface
css::uno::Any VCLXEdit::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XTextComponent* >(this),
                                        static_cast< css::awt::XTextEditField* >(this),
                                        static_cast< css::awt::XTextLayoutConstrains* >(this) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXEdit )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXEdit::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XTextComponent>::get(),
        cppu::UnoType<css::awt::XTextEditField>::get(),
        cppu::UnoType<css::awt::XTextLayoutConstrains>::get(),
        VCLXWindow::getTypes()
    );
    return aTypeList.getTypes();
}

css::uno::Reference< css::accessibility::XAccessibleContext > VCLXEdit::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXEdit::dispose()
{
    SolarMutexGuard aGuard;

    css::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maTextListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void VCLXEdit::addTextListener( const css::uno::Reference< css::awt::XTextListener > & l )
{
    SolarMutexGuard aGuard;
    GetTextListeners().addInterface( l );
}

void VCLXEdit::removeTextListener( const css::uno::Reference< css::awt::XTextListener > & l )
{
    SolarMutexGuard aGuard;
    GetTextListeners().removeInterface( l );
}

void VCLXEdit::setText( const OUString& aText )
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

void VCLXEdit::insertText( const css::awt::Selection& rSel, const OUString& aText )
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

OUString VCLXEdit::getText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

OUString VCLXEdit::getSelectedText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit)
        aText = pEdit->GetSelected();
    return aText;

}

void VCLXEdit::setSelection( const css::awt::Selection& aSelection )
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        pEdit->SetSelection( Selection( aSelection.Min, aSelection.Max ) );
}

css::awt::Selection VCLXEdit::getSelection()
{
    SolarMutexGuard aGuard;

    Selection aSel;
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        aSel = pEdit->GetSelection();
    return css::awt::Selection( aSel.Min(), aSel.Max() );
}

sal_Bool VCLXEdit::isEditable()
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    return pEdit && !pEdit->IsReadOnly() && pEdit->IsEnabled();
}

void VCLXEdit::setEditable( sal_Bool bEditable )
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        pEdit->SetReadOnly( !bEditable );
}


void VCLXEdit::setMaxTextLen( sal_Int16 nLen )
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        pEdit->SetMaxTextLen( nLen );
}

sal_Int16 VCLXEdit::getMaxTextLen()
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    return pEdit ? pEdit->GetMaxTextLen() : 0;
}

void VCLXEdit::setEchoChar( sal_Unicode cEcho )
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        pEdit->SetEchoChar( cEcho );
}

void VCLXEdit::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( !pEdit )
        return;

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

css::uno::Any VCLXEdit::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
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
                 aProp <<= static_cast<sal_Int16>(pEdit->GetEchoChar());
                break;
            case BASEPROPERTY_MAXTEXTLEN:
                 aProp <<= static_cast<sal_Int16>(pEdit->GetMaxTextLen());
                break;
            default:
            {
                aProp = VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

css::awt::Size VCLXEdit::getMinimumSize(  )
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
        aSz = pEdit->CalcMinimumSize();
    return AWTSize(aSz);
}

css::awt::Size VCLXEdit::getPreferredSize(  )
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
    {
        aSz = pEdit->CalcMinimumSize();
        aSz.AdjustHeight(4 );
    }
    return AWTSize(aSz);
}

css::awt::Size VCLXEdit::calcAdjustedSize( const css::awt::Size& rNewSize )
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz = rNewSize;
    css::awt::Size aMinSz = getMinimumSize();
    if ( aSz.Height != aMinSz.Height )
        aSz.Height = aMinSz.Height;

    return aSz;
}

css::awt::Size VCLXEdit::getMinimumSize( sal_Int16 nCols, sal_Int16 )
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

void VCLXEdit::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines )
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
        case VclEventId::EditModify:
        {
            css::uno::Reference< css::awt::XWindow > xKeepAlive( this );
                // since we call listeners below, there is a potential that we will be destroyed
                // during the listener call. To prevent the resulting crashes, we keep us
                // alive as long as we're here

            if ( GetTextListeners().getLength() )
            {
                css::awt::TextEvent aEvent;
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




void VCLXComboBox::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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
                     BASEPROPERTY_TYPEDITEMLIST,
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
    SAL_INFO("toolkit", __FUNCTION__);
}

css::uno::Reference< css::accessibility::XAccessibleContext > VCLXComboBox::CreateAccessibleContext()
{
    SolarMutexGuard aGuard;

    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXComboBox::dispose()
{
    SolarMutexGuard aGuard;

    css::lang::EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    maItemListeners.disposeAndClear( aObj );
    maActionListeners.disposeAndClear( aObj );
    VCLXEdit::dispose();
}


void VCLXComboBox::addItemListener( const css::uno::Reference< css::awt::XItemListener > & l )
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXComboBox::removeItemListener( const css::uno::Reference< css::awt::XItemListener > & l )
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXComboBox::addActionListener( const css::uno::Reference< css::awt::XActionListener > & l )
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXComboBox::removeActionListener( const css::uno::Reference< css::awt::XActionListener > & l )
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXComboBox::addItem( const OUString& aItem, sal_Int16 nPos )
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
        pBox->InsertEntry( aItem, nPos );
}

void VCLXComboBox::addItems( const css::uno::Sequence< OUString>& aItems, sal_Int16 nPos )
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( !pBox )
        return;

    sal_uInt16 nP = nPos;
    for ( const auto& rItem : aItems )
    {
        pBox->InsertEntry( rItem, nP );
        if ( nP == 0xFFFF )
        {
            OSL_FAIL( "VCLXComboBox::addItems: too many entries!" );
            // skip remaining entries, list cannot hold them, anyway
            break;
        }
    }
}

void VCLXComboBox::removeItems( sal_Int16 nPos, sal_Int16 nCount )
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
    {
        for ( sal_uInt16 n = nCount; n; )
            pBox->RemoveEntryAt( nPos + (--n) );
    }
}

sal_Int16 VCLXComboBox::getItemCount()
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    return pBox ? pBox->GetEntryCount() : 0;
}

OUString VCLXComboBox::getItem( sal_Int16 nPos )
{
    SolarMutexGuard aGuard;

    OUString aItem;
    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
        aItem = pBox->GetEntry( nPos );
    return aItem;
}

css::uno::Sequence< OUString> VCLXComboBox::getItems()
{
    SolarMutexGuard aGuard;

    css::uno::Sequence< OUString> aSeq;
    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
    {
        auto n = pBox->GetEntryCount();
        aSeq = css::uno::Sequence< OUString>( n );
        while ( n )
        {
            --n;
            aSeq.getArray()[n] = pBox->GetEntry( n );
        }
    }
    return aSeq;
}

void VCLXComboBox::setDropDownLineCount( sal_Int16 nLines )
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
        pBox->SetDropDownLineCount( nLines );
}

sal_Int16 VCLXComboBox::getDropDownLineCount()
{
    SolarMutexGuard aGuard;

    sal_Int16 nLines = 0;
    VclPtr< ComboBox > pBox = GetAs< ComboBox >();
    if ( pBox )
        nLines = pBox->GetDropDownLineCount();
    return nLines;
}

void VCLXComboBox::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( !pComboBox )
        return;

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
            css::uno::Sequence< OUString> aItems;
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

css::uno::Any VCLXComboBox::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( pComboBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LINECOUNT:
            {
                 aProp <<= static_cast<sal_Int16>(pComboBox->GetDropDownLineCount());
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
                css::uno::Sequence< OUString> aSeq( nItems );
                OUString* pStrings = aSeq.getArray();
                for ( sal_Int32 n = 0; n < nItems; ++n )
                    pStrings[n] = pComboBox->GetEntry( n );
                aProp <<= aSeq;

            }
            break;
            default:
            {
                aProp = VCLXEdit::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXComboBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    css::uno::Reference< css::awt::XWindow > xKeepAlive( this );
        // since we call listeners below, there is a potential that we will be destroyed
        // during the listener call. To prevent the resulting crashes, we keep us
        // alive as long as we're here

    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::ComboboxSelect:
            if ( maItemListeners.getLength() )
            {
                VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
                if( pComboBox )
                {
                    if ( !pComboBox->IsTravelSelect() )
                    {
                        css::awt::ItemEvent aEvent;
                        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                        aEvent.Highlighted = 0;

                        // Set to 0xFFFF on multiple selection, selected entry ID otherwise
                        aEvent.Selected = pComboBox->GetEntryPos( pComboBox->GetText() );

                        maItemListeners.itemStateChanged( aEvent );
                    }
                }
            }
            break;

        case VclEventId::ComboboxDoubleClick:
            if ( maActionListeners.getLength() )
            {
                css::awt::ActionEvent aEvent;
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

css::awt::Size VCLXComboBox::getMinimumSize(  )
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( pComboBox )
        aSz = pComboBox->CalcMinimumSize();
    return AWTSize(aSz);
}

css::awt::Size VCLXComboBox::getPreferredSize(  )
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( pComboBox )
    {
        aSz = pComboBox->CalcMinimumSize();
        if ( pComboBox->GetStyle() & WB_DROPDOWN )
            aSz.AdjustHeight(4 );
    }
    return AWTSize(aSz);
}

css::awt::Size VCLXComboBox::calcAdjustedSize( const css::awt::Size& rNewSize )
{
    SolarMutexGuard aGuard;

    Size aSz = VCLSize(rNewSize);
    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( pComboBox )
        aSz = pComboBox->CalcAdjustedSize( aSz );
    return AWTSize(aSz);
}

css::awt::Size VCLXComboBox::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines )
{
    SolarMutexGuard aGuard;

    Size aSz;
    VclPtr< ComboBox > pComboBox = GetAs< ComboBox >();
    if ( pComboBox )
        aSz = pComboBox->CalcBlockSize( nCols, nLines );
    return AWTSize(aSz);
}

void VCLXComboBox::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines )
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
void SAL_CALL VCLXComboBox::listItemInserted( const ItemListEvent& i_rEvent )
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pComboBox = GetAsDynamic< ComboBox >();

    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemInserted: no ComboBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition <= pComboBox->GetEntryCount() ),
        "VCLXComboBox::listItemInserted: illegal (inconsistent) item position!" );
    pComboBox->InsertEntryWithImage(
        i_rEvent.ItemText.IsPresent ? i_rEvent.ItemText.Value : OUString(),
        i_rEvent.ItemImageURL.IsPresent ? lcl_getImageFromURL( i_rEvent.ItemImageURL.Value ) : Image(),
        i_rEvent.ItemPosition );
}

void SAL_CALL VCLXComboBox::listItemRemoved( const ItemListEvent& i_rEvent )
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pComboBox = GetAsDynamic< ComboBox >();

    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemRemoved: no ComboBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition < pComboBox->GetEntryCount() ),
        "VCLXComboBox::listItemRemoved: illegal (inconsistent) item position!" );

    pComboBox->RemoveEntryAt( i_rEvent.ItemPosition );
}

void SAL_CALL VCLXComboBox::listItemModified( const ItemListEvent& i_rEvent )
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pComboBox = GetAsDynamic< ComboBox >();

    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemModified: no ComboBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition < pComboBox->GetEntryCount() ),
        "VCLXComboBox::listItemModified: illegal (inconsistent) item position!" );

    // VCL's ComboBox does not support changing an entry's text or image, so remove and re-insert

    const OUString sNewText = i_rEvent.ItemText.IsPresent ? i_rEvent.ItemText.Value : pComboBox->GetEntry( i_rEvent.ItemPosition );
    const Image aNewImage( i_rEvent.ItemImageURL.IsPresent ? lcl_getImageFromURL( i_rEvent.ItemImageURL.Value ) : pComboBox->GetEntryImage( i_rEvent.ItemPosition  ) );

    pComboBox->RemoveEntryAt( i_rEvent.ItemPosition );
    pComboBox->InsertEntryWithImage(sNewText, aNewImage, i_rEvent.ItemPosition);
}

void SAL_CALL VCLXComboBox::allItemsRemoved( const EventObject& )
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pComboBox = GetAsDynamic< ComboBox >();
    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemModified: no ComboBox?!" );

    pComboBox->Clear();
}

void SAL_CALL VCLXComboBox::itemListChanged( const EventObject& i_rEvent )
{
    SolarMutexGuard aGuard;

    VclPtr< ComboBox > pComboBox = GetAsDynamic< ComboBox >();
    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemModified: no ComboBox?!" );

    pComboBox->Clear();

    uno::Reference< beans::XPropertySet > xPropSet( i_rEvent.Source, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySetInfo > xPSI( xPropSet->getPropertySetInfo(), uno::UNO_SET_THROW );
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
    const uno::Sequence< beans::Pair< OUString, OUString > > aItems = xItemList->getAllItems();
    for ( const auto& rItem : aItems )
    {
        OUString aLocalizationKey( rItem.First );
        if ( xStringResourceResolver.is() && !aLocalizationKey.isEmpty() && aLocalizationKey[0] == '&' )
        {
            aLocalizationKey = xStringResourceResolver->resolveString(aLocalizationKey.copy( 1 ));
        }
        pComboBox->InsertEntryWithImage(aLocalizationKey,
                lcl_getImageFromURL(rItem.Second));
    }
}
void SAL_CALL VCLXComboBox::disposing( const EventObject& i_rEvent )
{
    // just disambiguate
    VCLXEdit::disposing( i_rEvent );
}



void VCLXFormattedSpinField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    // Interestingly in the UnoControl API this is
    // - not derived from XEdit ultimately, (correct ?) - so cut this here ...
//    VCLXSpinField::ImplGetPropertyIds( rIds );
    VCLXWindow::ImplGetPropertyIds( rIds );
}

VCLXFormattedSpinField::VCLXFormattedSpinField()
    : mpFormatter(nullptr)
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

bool VCLXFormattedSpinField::isStrictFormat() const
{
    FormatterBase* pFormatter = GetFormatter();
    return pFormatter && pFormatter->IsStrictFormat();
}


void VCLXFormattedSpinField::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    FormatterBase* pFormatter = GetFormatter();
    if ( !pFormatter )
        return;

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

css::uno::Any VCLXFormattedSpinField::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
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
                aProp = VCLXSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}




void VCLXDateField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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
css::uno::Reference< css::accessibility::XAccessibleContext > VCLXDateField::CreateAccessibleContext()
{
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->SetType( WindowType::DATEFIELD );
    }
    return getAccessibleFactory().createAccessibleContext( this );
}

// css::uno::XInterface
css::uno::Any VCLXDateField::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XDateField* >(this) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXDateField )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXDateField::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XDateField>::get(),
        VCLXFormattedSpinField::getTypes()
    );
    return aTypeList.getTypes();
}

void VCLXDateField::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    if ( !(GetWindow()) )
        return;

    bool bVoid = Value.getValueType().getTypeClass() == css::uno::TypeClass_VOID;

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
                GetAs< DateField >()->SetExtDateFormat( static_cast<ExtDateFieldFormat>(n) );
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

css::uno::Any VCLXDateField::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
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
                aProp = VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}


void VCLXDateField::setDate( const util::Date& aDate )
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

util::Date VCLXDateField::getDate()
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        return pDateField->GetDate().GetUNODate();
    else
        return util::Date();
}

void VCLXDateField::setMin( const util::Date& aDate )
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        pDateField->SetMin( aDate );
}

util::Date VCLXDateField::getMin()
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        return pDateField->GetMin().GetUNODate();
    else
        return util::Date();
}

void VCLXDateField::setMax( const util::Date& aDate )
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        pDateField->SetMax( aDate );
}

util::Date VCLXDateField::getMax()
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        return pDateField->GetMax().GetUNODate();
    else
        return util::Date();
}

void VCLXDateField::setFirst( const util::Date& aDate )
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        pDateField->SetFirst( aDate );
}

util::Date VCLXDateField::getFirst()
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        return pDateField->GetFirst().GetUNODate();
    else
        return util::Date();
}

void VCLXDateField::setLast( const util::Date& aDate )
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        pDateField->SetLast( aDate );
}

util::Date VCLXDateField::getLast()
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        return pDateField->GetLast().GetUNODate();
    else
        return util::Date();
}

void VCLXDateField::setLongFormat( sal_Bool bLong )
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    if ( pDateField )
        pDateField->SetLongFormat( bLong );
}

sal_Bool VCLXDateField::isLongFormat()
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    return pDateField && pDateField->IsLongFormat();
}

void VCLXDateField::setEmpty()
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

sal_Bool VCLXDateField::isEmpty()
{
    SolarMutexGuard aGuard;

    VclPtr< DateField > pDateField = GetAs< DateField >();
    return pDateField && pDateField->IsEmptyDate();
}

void VCLXDateField::setStrictFormat( sal_Bool bStrict )
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXDateField::isStrictFormat()
{
    return VCLXFormattedSpinField::isStrictFormat();
}




void VCLXTimeField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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
css::uno::Reference< css::accessibility::XAccessibleContext > VCLXTimeField::CreateAccessibleContext()
{
    VclPtr< vcl::Window > pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->SetType( WindowType::TIMEFIELD );
    }
    return getAccessibleFactory().createAccessibleContext( this );
}

// css::uno::XInterface
css::uno::Any VCLXTimeField::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XTimeField* >(this) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXTimeField )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXTimeField::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XTimeField>::get(),
        VCLXFormattedSpinField::getTypes()
    );
    return aTypeList.getTypes();
}

void VCLXTimeField::setTime( const util::Time& aTime )
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

util::Time VCLXTimeField::getTime()
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        return pTimeField->GetTime().GetUNOTime();
    else
        return util::Time();
}

void VCLXTimeField::setMin( const util::Time& aTime )
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        pTimeField->SetMin( aTime );
}

util::Time VCLXTimeField::getMin()
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        return pTimeField->GetMin().GetUNOTime();
    else
        return util::Time();
}

void VCLXTimeField::setMax( const util::Time& aTime )
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        pTimeField->SetMax( aTime );
}

util::Time VCLXTimeField::getMax()
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        return pTimeField->GetMax().GetUNOTime();
    else
        return util::Time();
}

void VCLXTimeField::setFirst( const util::Time& aTime )
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        pTimeField->SetFirst( aTime );
}

util::Time VCLXTimeField::getFirst()
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        return pTimeField->GetFirst().GetUNOTime();
    else
        return util::Time();
}

void VCLXTimeField::setLast( const util::Time& aTime )
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        pTimeField->SetLast( aTime );
}

util::Time VCLXTimeField::getLast()
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        return pTimeField->GetLast().GetUNOTime();
    else
        return util::Time();
}

void VCLXTimeField::setEmpty()
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    if ( pTimeField )
        pTimeField->SetEmptyTime();
}

sal_Bool VCLXTimeField::isEmpty()
{
    SolarMutexGuard aGuard;

    VclPtr< TimeField > pTimeField = GetAs< TimeField >();
    return pTimeField && pTimeField->IsEmptyTime();
}

void VCLXTimeField::setStrictFormat( sal_Bool bStrict )
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXTimeField::isStrictFormat()
{
    return VCLXFormattedSpinField::isStrictFormat();
}


void VCLXTimeField::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    if ( !(GetWindow()) )
        return;

    bool bVoid = Value.getValueType().getTypeClass() == css::uno::TypeClass_VOID;

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
                GetAs< TimeField >()->SetExtFormat( static_cast<ExtTimeFieldFormat>(n) );
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

css::uno::Any VCLXTimeField::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
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
                aProp = VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}




void VCLXNumericField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

// css::uno::XInterface
css::uno::Any VCLXNumericField::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XNumericField* >(this) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXNumericField )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXNumericField::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XNumericField>::get(),
        VCLXFormattedSpinField::getTypes()
    );
    return aTypeList.getTypes();
}

void VCLXNumericField::setValue( double Value )
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    if ( !pNumericFormatter )
        return;

    // shift long value using decimal digits
    // (e.g., input 105 using 2 digits returns 1,05)
    // Thus, to set a value of 1,05, insert 105 and 2 digits
    pNumericFormatter->SetValue(
        static_cast<tools::Long>(ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() )) );

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

double VCLXNumericField::getValue()
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter
        ? ImplCalcDoubleValue( static_cast<double>(pNumericFormatter->GetValue()), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setMin( double Value )
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    if ( pNumericFormatter )
        pNumericFormatter->SetMin(
            static_cast<tools::Long>(ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() )) );
}

double VCLXNumericField::getMin()
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter
        ? ImplCalcDoubleValue( static_cast<double>(pNumericFormatter->GetMin()), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setMax( double Value )
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    if ( pNumericFormatter )
        pNumericFormatter->SetMax(
            static_cast<tools::Long>(ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() )) );
}

double VCLXNumericField::getMax()
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter
        ? ImplCalcDoubleValue( static_cast<double>(pNumericFormatter->GetMax()), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setFirst( double Value )
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    if ( pNumericFormatter )
        pNumericFormatter->SetFirst(
            static_cast<tools::Long>(ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() )) );
}

double VCLXNumericField::getFirst()
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter
        ? ImplCalcDoubleValue( static_cast<double>(pNumericFormatter->GetFirst()), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setLast( double Value )
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    if ( pNumericFormatter )
        pNumericFormatter->SetLast(
            static_cast<tools::Long>(ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() )) );
}

double VCLXNumericField::getLast()
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter
        ? ImplCalcDoubleValue( static_cast<double>(pNumericFormatter->GetLast()), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setStrictFormat( sal_Bool bStrict )
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXNumericField::isStrictFormat()
{
    return VCLXFormattedSpinField::isStrictFormat();
}

void VCLXNumericField::setSpinSize( double Value )
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    if ( pNumericFormatter )
        pNumericFormatter->SetSpinSize(
            static_cast<tools::Long>(ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() )) );
}

double VCLXNumericField::getSpinSize()
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter
        ? ImplCalcDoubleValue( static_cast<double>(pNumericFormatter->GetSpinSize()), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setDecimalDigits( sal_Int16 Value )
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

sal_Int16 VCLXNumericField::getDecimalDigits()
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter ? pNumericFormatter->GetDecimalDigits() : 0;
}

void VCLXNumericField::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    if ( !(GetWindow()) )
        return;

    bool bVoid = Value.getValueType().getTypeClass() == css::uno::TypeClass_VOID;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_VALUE_DOUBLE:
        {
            if ( bVoid )
            {
                NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
                if (!pNumericFormatter)
                    return;
                pNumericFormatter->EnableEmptyFieldValue( true );
                pNumericFormatter->SetEmptyFieldValue();
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
            {
                NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
                if (!pNumericFormatter)
                    return;
                pNumericFormatter->SetUseThousandSep( b );
            }
        }
        break;
        default:
        {
            VCLXFormattedSpinField::setProperty( PropertyName, Value );
        }
    }
}

css::uno::Any VCLXNumericField::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                aProp <<= getValue();
            }
            break;
            case BASEPROPERTY_VALUEMIN_DOUBLE:
            {
                aProp <<= getMin();
            }
            break;
            case BASEPROPERTY_VALUEMAX_DOUBLE:
            {
                aProp <<= getMax();
            }
            break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:
            {
                aProp <<= getSpinSize();
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
                aProp <<= pNumericFormatter->IsUseThousandSep();
            }
            break;
            default:
            {
                aProp = VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}


//    ----------------------------------------------------
//    ----------------------------------------------------

void VCLXMetricField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

MetricFormatter *VCLXMetricField::GetMetricFormatter()
{
    MetricFormatter *pFormatter = static_cast<MetricFormatter *>(GetFormatter());
    if (!pFormatter)
        throw css::uno::RuntimeException();
    return pFormatter;
}

MetricField *VCLXMetricField::GetMetricField()
{
    VclPtr< MetricField > pField = GetAs< MetricField >();
    if (!pField)
        throw css::uno::RuntimeException();
    return pField;
}

// css::uno::XInterface
css::uno::Any VCLXMetricField::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType, static_cast< css::awt::XMetricField* >(this) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXMetricField )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXMetricField::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XMetricField>::get(),
        VCLXFormattedSpinField::getTypes()
    );
    return aTypeList.getTypes();
}

// FIXME: later ...
#define MetricUnitUnoToVcl(a) (static_cast<FieldUnit>(a))

#define METRIC_MAP_PAIR(method,parent) \
    sal_Int64 VCLXMetricField::get##method( sal_Int16 nUnit ) \
    { \
        SolarMutexGuard aGuard; \
        return GetMetric##parent()->Get##method( MetricUnitUnoToVcl( nUnit ) ); \
    } \
    void VCLXMetricField::set##method( sal_Int64 nValue, sal_Int16 nUnit ) \
    { \
        SolarMutexGuard aGuard; \
        GetMetric##parent()->Set##method( nValue, MetricUnitUnoToVcl( nUnit ) ); \
    }

METRIC_MAP_PAIR(Min, Formatter)
METRIC_MAP_PAIR(Max, Formatter)
METRIC_MAP_PAIR(First, Field)
METRIC_MAP_PAIR(Last,  Field)

#undef METRIC_MAP_PAIR

::sal_Int64 VCLXMetricField::getValue( ::sal_Int16 nUnit )
{
    SolarMutexGuard aGuard;
    return GetMetricFormatter()->GetValue( MetricUnitUnoToVcl( nUnit ) );
}

::sal_Int64 VCLXMetricField::getCorrectedValue( ::sal_Int16 nUnit )
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

void VCLXMetricField::setValue( ::sal_Int64 Value, ::sal_Int16 Unit )
{
    SolarMutexGuard aGuard;
    GetMetricFormatter()->SetValue( Value, MetricUnitUnoToVcl( Unit ) );
    CallListeners();
}

void VCLXMetricField::setUserValue( ::sal_Int64 Value, ::sal_Int16 Unit )
{
    SolarMutexGuard aGuard;
    GetMetricFormatter()->SetUserValue( Value, MetricUnitUnoToVcl( Unit ) );
    CallListeners();
}

void VCLXMetricField::setStrictFormat( sal_Bool bStrict )
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXMetricField::isStrictFormat()
{
    return VCLXFormattedSpinField::isStrictFormat();
}

void VCLXMetricField::setSpinSize( sal_Int64 Value )
{
    SolarMutexGuard aGuard;
    GetMetricField()->SetSpinSize( Value );
}

sal_Int64 VCLXMetricField::getSpinSize()
{
    SolarMutexGuard aGuard;
    return GetMetricField()->GetSpinSize();
}

void VCLXMetricField::setDecimalDigits( sal_Int16 Value )
{
    SolarMutexGuard aGuard;
    GetMetricFormatter()->SetDecimalDigits( Value );
}

sal_Int16 VCLXMetricField::getDecimalDigits()
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
    return pNumericFormatter ? pNumericFormatter->GetDecimalDigits() : 0;
}

void VCLXMetricField::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    if ( !(GetWindow()) )
        return;

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
            {
                NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
                if (!pNumericFormatter)
                    return;
                pNumericFormatter->SetUseThousandSep( b );
            }
        }
        break;
        case BASEPROPERTY_UNIT:
        {
            sal_uInt16 nVal = 0;
            if ( Value >>= nVal )
                GetAs< MetricField >()->SetUnit( static_cast<FieldUnit>(nVal) );
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

css::uno::Any VCLXMetricField::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                NumericFormatter* pNumericFormatter = static_cast<NumericFormatter*>(GetFormatter());
                aProp <<= pNumericFormatter->IsUseThousandSep();
                break;
            }
            case BASEPROPERTY_UNIT:
                aProp <<= static_cast<sal_uInt16>(GetAs< MetricField >()->GetUnit());
                break;
            case BASEPROPERTY_CUSTOMUNITTEXT:
                aProp <<= GetAs< MetricField >()->GetCustomUnitText();
                break;
            default:
            {
                aProp = VCLXFormattedSpinField::getProperty( PropertyName );
                break;
            }
        }
    }
    return aProp;
}

void VCLXPatternField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

// css::uno::XInterface
css::uno::Any VCLXPatternField::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XPatternField* >(this) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXPatternField )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXPatternField::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XPatternField>::get(),
        VCLXFormattedSpinField::getTypes()
    );
    return aTypeList.getTypes();
}

void VCLXPatternField::setMasks( const OUString& EditMask, const OUString& LiteralMask )
{
    SolarMutexGuard aGuard;

    VclPtr< PatternField > pPatternField = GetAs< PatternField >();
    if ( pPatternField )
    {
        pPatternField->SetMask( OUStringToOString(EditMask, RTL_TEXTENCODING_ASCII_US), LiteralMask );
    }
}

void VCLXPatternField::getMasks( OUString& EditMask, OUString& LiteralMask )
{
    SolarMutexGuard aGuard;

    VclPtr< PatternField > pPatternField = GetAs< PatternField >();
    if ( pPatternField )
    {
        EditMask = OStringToOUString(pPatternField->GetEditMask(), RTL_TEXTENCODING_ASCII_US);
        LiteralMask = pPatternField->GetLiteralMask();
    }
}

void VCLXPatternField::setString( const OUString& Str )
{
    SolarMutexGuard aGuard;
    VclPtr< PatternField > pPatternField = GetAs< PatternField >();
    if ( pPatternField )
        pPatternField->SetString( Str );
}

OUString VCLXPatternField::getString()
{
    SolarMutexGuard aGuard;

    OUString aString;
    VclPtr< PatternField > pPatternField = GetAs< PatternField >();
    if ( pPatternField )
        aString = pPatternField->GetString();
    return aString;
}

void VCLXPatternField::setStrictFormat( sal_Bool bStrict )
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXPatternField::isStrictFormat()
{
    return VCLXFormattedSpinField::isStrictFormat();
}

void VCLXPatternField::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    if ( !(GetWindow()) )
        return;

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

css::uno::Any VCLXPatternField::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
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
                aProp = VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}



VCLXToolBox::VCLXToolBox()
{
}

VCLXToolBox::~VCLXToolBox()
{
}

css::uno::Reference< css::accessibility::XAccessibleContext > VCLXToolBox::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

VCLXHeaderBar::VCLXHeaderBar()
{
}

VCLXHeaderBar::~VCLXHeaderBar()
{
}

css::uno::Reference< css::accessibility::XAccessibleContext > VCLXHeaderBar::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}


VCLXFrame::VCLXFrame()
{
}

void VCLXFrame::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

IMPL_IMPLEMENTATION_ID( VCLXFrame )

// css::awt::XView
void SAL_CALL VCLXFrame::draw( sal_Int32 nX, sal_Int32 nY )
{
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );
        pWindow->Draw( pDev, aPos, DrawFlags::NoControls );
    }
}

void SAL_CALL VCLXFrame::setProperty(
    const OUString& PropertyName,
    const css::uno::Any& Value )
{
    SolarMutexGuard aGuard;

    VCLXContainer::setProperty( PropertyName, Value );
}

void VCLXFrame::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    css::uno::Reference< css::awt::XWindow > xKeepAlive( this );
    VCLXContainer::ProcessWindowEvent( rVclWindowEvent );
}

VCLXProgressBar::VCLXProgressBar()
            :m_nValue(0)
            ,m_nValueMin(0)
            ,m_nValueMax(100)
{
}

VCLXProgressBar::~VCLXProgressBar()
{
}

void VCLXProgressBar::ImplUpdateValue()
{
    VclPtr< ProgressBar > pProgressBar = GetAs< ProgressBar >();
    if ( !pProgressBar )
        return;

    sal_Int32 nVal;
    sal_Int32 nValMin;
    sal_Int32 nValMax;

    // check min and max
    if (m_nValueMin < m_nValueMax)
    {
        nValMin = m_nValueMin;
        nValMax = m_nValueMax;
    }
    else
    {
        nValMin = m_nValueMax;
        nValMax = m_nValueMin;
    }

    // check value
    if (m_nValue < nValMin)
    {
        nVal = nValMin;
    }
    else if (m_nValue > nValMax)
    {
        nVal = nValMax;
    }
    else
    {
        nVal = m_nValue;
    }

    // calculate percent
    sal_Int32 nPercent;
    if (nValMin != nValMax)
    {
        nPercent = 100 * (nVal - nValMin) / (nValMax - nValMin);
    }
    else
    {
        nPercent = 0;
    }

    // set progressbar value
    pProgressBar->SetValue( static_cast<sal_uInt16>(nPercent) );
}

// css::uno::XInterface
css::uno::Any VCLXProgressBar::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XProgressBar* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXProgressBar )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXProgressBar::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XProgressBar>::get(),
        VCLXWindow::getTypes()
    );
    return aTypeList.getTypes();
}

// css::awt::XProgressBar
void VCLXProgressBar::setForegroundColor( sal_Int32 nColor )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->SetControlForeground( Color(ColorTransparency, nColor) );
    }
}

void VCLXProgressBar::setBackgroundColor( sal_Int32 nColor )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
    {
        Color aColor( ColorTransparency, nColor );
        pWindow->SetBackground( aColor );
        pWindow->SetControlBackground( aColor );
        pWindow->Invalidate();
    }
}

void VCLXProgressBar::setValue( sal_Int32 nValue )
{
    SolarMutexGuard aGuard;

    m_nValue = nValue;
    ImplUpdateValue();
}

void VCLXProgressBar::setRange( sal_Int32 nMin, sal_Int32 nMax )
{
    SolarMutexGuard aGuard;

    if ( nMin < nMax )
    {
        // take correct min and max
        m_nValueMin = nMin;
        m_nValueMax = nMax;
    }
    else
    {
        // change min and max
        m_nValueMin = nMax;
        m_nValueMax = nMin;
    }

    ImplUpdateValue();
}

sal_Int32 VCLXProgressBar::getValue()
{
    SolarMutexGuard aGuard;

    return m_nValue;
}

// css::awt::VclWindowPeer
void VCLXProgressBar::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< ProgressBar > pProgressBar = GetAs< ProgressBar >();
    if ( !pProgressBar )
        return;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_PROGRESSVALUE:
        {
            if ( Value >>= m_nValue )
                ImplUpdateValue();
        }
        break;
        case BASEPROPERTY_PROGRESSVALUE_MIN:
        {
            if ( Value >>= m_nValueMin )
                ImplUpdateValue();
        }
        break;
        case BASEPROPERTY_PROGRESSVALUE_MAX:
        {
            if ( Value >>= m_nValueMax )
                ImplUpdateValue();
        }
        break;
        case BASEPROPERTY_FILLCOLOR:
        {
            VclPtr<vcl::Window> pWindow = GetWindow();
            if ( pWindow )
            {
                bool bVoid = Value.getValueType().getTypeClass() == css::uno::TypeClass_VOID;

                if ( bVoid )
                {
                    pWindow->SetControlForeground();
                }
                else
                {
                    Color nColor;
                    if ( Value >>= nColor )
                        pWindow->SetControlForeground( nColor );
                }
            }
        }
        break;
        default:
            VCLXWindow::setProperty( PropertyName, Value );
            break;
    }
}

css::uno::Any VCLXProgressBar::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
    VclPtr< ProgressBar > pProgressBar = GetAs< ProgressBar >();
    if ( pProgressBar )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_PROGRESSVALUE:
            {
                 aProp <<= m_nValue;
            }
            break;
            case BASEPROPERTY_PROGRESSVALUE_MIN:
            {
                 aProp <<= m_nValueMin;
            }
            break;
            case BASEPROPERTY_PROGRESSVALUE_MAX:
            {
                 aProp <<= m_nValueMax;
            }
            break;
            default:
                aProp = VCLXWindow::getProperty( PropertyName );
                break;
        }
    }
    return aProp;
}

void VCLXProgressBar::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_PROGRESSVALUE,
                     BASEPROPERTY_PROGRESSVALUE_MIN,
                     BASEPROPERTY_PROGRESSVALUE_MAX,
                     BASEPROPERTY_FILLCOLOR,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );
}

VCLXFileControl::VCLXFileControl() : maTextListeners( *this )
{
}

VCLXFileControl::~VCLXFileControl()
{
    VclPtr< FileControl > pControl = GetAs< FileControl >();
    if ( pControl )
        pControl->GetEdit().SetModifyHdl( Link<Edit&,void>() );
}

css::uno::Any VCLXFileControl::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XTextComponent* >(this),
                                        static_cast< css::awt::XTextLayoutConstrains* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXFileControl )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXFileControl::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XTextComponent>::get(),
        cppu::UnoType<css::awt::XTextLayoutConstrains>::get(),
        VCLXWindow::getTypes()
    );
    return aTypeList.getTypes();
}

namespace
{
    void lcl_setWinBits( vcl::Window* _pWindow, WinBits _nBits, bool _bSet )
    {
        WinBits nStyle = _pWindow->GetStyle();
        if ( _bSet )
            nStyle |= _nBits;
        else
            nStyle &= ~_nBits;
        _pWindow->SetStyle( nStyle );
    }
}

void SAL_CALL VCLXFileControl::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pControl = GetAs< FileControl >();
    if ( !pControl )
        return;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
    case BASEPROPERTY_HIDEINACTIVESELECTION:
    {
        bool bValue(false);
        OSL_VERIFY( Value >>= bValue );

        lcl_setWinBits( pControl, WB_NOHIDESELECTION, !bValue );
        lcl_setWinBits( &pControl->GetEdit(), WB_NOHIDESELECTION, !bValue );
    }
    break;

    default:
        VCLXWindow::setProperty( PropertyName, Value );
        break;
    }
}

void VCLXFileControl::SetWindow( const VclPtr< vcl::Window > &pWindow )
{
    VclPtr< FileControl > pPrevFileControl = GetAsDynamic< FileControl >();
    if ( pPrevFileControl )
        pPrevFileControl->SetEditModifyHdl( Link<Edit&,void>() );

    FileControl* pNewFileControl = dynamic_cast<FileControl*>( pWindow.get() );
    if ( pNewFileControl )
        pNewFileControl->SetEditModifyHdl( LINK( this, VCLXFileControl, ModifyHdl ) );

    VCLXWindow::SetWindow( pWindow );
}

void VCLXFileControl::addTextListener( const css::uno::Reference< css::awt::XTextListener > & l )
{
    maTextListeners.addInterface( l );
}

void VCLXFileControl::removeTextListener( const css::uno::Reference< css::awt::XTextListener > & l )
{
    maTextListeners.removeInterface( l );
}

void VCLXFileControl::setText( const OUString& aText )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->SetText( aText );

        // also in Java a textChanged is triggered, not in VCL.
        // css::awt::Toolkit should be JAVA-compliant...
        ModifyHdl();
    }
}

void VCLXFileControl::insertText( const css::awt::Selection& rSel, const OUString& aText )
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    if ( pFileControl )
    {
        pFileControl->GetEdit().SetSelection( Selection( rSel.Min, rSel.Max ) );
        pFileControl->GetEdit().ReplaceSelected( aText );
    }
}

OUString VCLXFileControl::getText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

OUString VCLXFileControl::getSelectedText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    if ( pFileControl)
        aText = pFileControl->GetEdit().GetSelected();
    return aText;

}

void VCLXFileControl::setSelection( const css::awt::Selection& aSelection )
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    if ( pFileControl )
        pFileControl->GetEdit().SetSelection( Selection( aSelection.Min, aSelection.Max ) );
}

css::awt::Selection VCLXFileControl::getSelection()
{
    SolarMutexGuard aGuard;

    css::awt::Selection aSel;
    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    if ( pFileControl )
    {
        aSel.Min = pFileControl->GetEdit().GetSelection().Min();
        aSel.Max = pFileControl->GetEdit().GetSelection().Max();
    }
    return aSel;
}

sal_Bool VCLXFileControl::isEditable()
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    return pFileControl && !pFileControl->GetEdit().IsReadOnly() && pFileControl->GetEdit().IsEnabled();
}

void VCLXFileControl::setEditable( sal_Bool bEditable )
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    if ( pFileControl )
        pFileControl->GetEdit().SetReadOnly( !bEditable );
}

void VCLXFileControl::setMaxTextLen( sal_Int16 nLen )
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    if ( pFileControl )
        pFileControl->GetEdit().SetMaxTextLen( nLen );
}

sal_Int16 VCLXFileControl::getMaxTextLen()
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    return pFileControl ? pFileControl->GetEdit().GetMaxTextLen() : 0;
}


IMPL_LINK_NOARG(VCLXFileControl, ModifyHdl, Edit&, void)
{
    ModifyHdl();
}

void VCLXFileControl::ModifyHdl()
{
    css::awt::TextEvent aEvent;
    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
    maTextListeners.textChanged( aEvent );
}

css::awt::Size VCLXFileControl::getMinimumSize()
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz;
    VclPtr< FileControl > pControl = GetAs< FileControl >();
    if ( pControl )
    {
        Size aTmpSize = pControl->GetEdit().CalcMinimumSize();
        aTmpSize.AdjustWidth(pControl->GetButton().CalcMinimumSize().Width() );
        aSz = AWTSize(pControl->CalcWindowSize( aTmpSize ));
    }
    return aSz;
}

css::awt::Size VCLXFileControl::getPreferredSize()
{
    css::awt::Size aSz = getMinimumSize();
    aSz.Height += 4;
    return aSz;
}

css::awt::Size VCLXFileControl::calcAdjustedSize( const css::awt::Size& rNewSize )
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz =rNewSize;
    VclPtr< FileControl > pControl = GetAs< FileControl >();
    if ( pControl )
    {
        css::awt::Size aMinSz = getMinimumSize();
        if ( aSz.Height != aMinSz.Height )
            aSz.Height = aMinSz.Height;
    }
    return aSz;
}

css::awt::Size VCLXFileControl::getMinimumSize( sal_Int16 nCols, sal_Int16 )
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz;
    VclPtr< FileControl > pControl = GetAs< FileControl >();
    if ( pControl )
    {
        aSz = AWTSize(pControl->GetEdit().CalcSize( nCols ));
        aSz.Width += pControl->GetButton().CalcMinimumSize().Width();
    }
    return aSz;
}

void VCLXFileControl::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines )
{
    SolarMutexGuard aGuard;

    nCols = 0;
    nLines = 1;
    VclPtr< FileControl > pControl = GetAs< FileControl >();
    if ( pControl )
        nCols = pControl->GetEdit().GetMaxVisChars();
}

void VCLXFileControl::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     // FIXME: elide duplication ?
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );
}

SVTXFormattedField::SVTXFormattedField()
    :bIsStandardSupplier(true)
    ,nKeyToSetDelayed(-1)
{
}

SVTXFormattedField::~SVTXFormattedField()
{
}

void SVTXFormattedField::SetWindow( const VclPtr< vcl::Window > &_pWindow )
{
    VCLXSpinField::SetWindow(_pWindow);
    if (GetAs< FormattedField >())
        GetAs< FormattedField >()->GetFormatter().SetAutoColor(true);
}

void SVTXFormattedField::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
    {
        Formatter& rFormatter = pField->GetFormatter();
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_ENFORCE_FORMAT:
            {
                bool bEnable( true );
                if ( Value >>= bEnable )
                    rFormatter.EnableNotANumber( !bEnable );
            }
            break;

            case BASEPROPERTY_EFFECTIVE_MIN:
            case BASEPROPERTY_VALUEMIN_DOUBLE:
                SetMinValue(Value);
                break;

            case BASEPROPERTY_EFFECTIVE_MAX:
            case BASEPROPERTY_VALUEMAX_DOUBLE:
                SetMaxValue(Value);
                break;

            case BASEPROPERTY_EFFECTIVE_DEFAULT:
                SetDefaultValue(Value);
                break;

            case BASEPROPERTY_TREATASNUMBER:
            {
                bool b;
                if ( Value >>= b )
                    SetTreatAsNumber(b);
            }
            break;

            case BASEPROPERTY_FORMATSSUPPLIER:
                if (!Value.hasValue())
                    setFormatsSupplier(css::uno::Reference< css::util::XNumberFormatsSupplier > (nullptr));
                else
                {
                    css::uno::Reference< css::util::XNumberFormatsSupplier > xNFS;
                    if ( Value >>= xNFS )
                        setFormatsSupplier(xNFS);
                }
                break;
            case BASEPROPERTY_FORMATKEY:
                if (!Value.hasValue())
                    setFormatKey(0);
                else
                {
                    sal_Int32 n = 0;
                    if ( Value >>= n )
                        setFormatKey(n);
                }
                break;

            case BASEPROPERTY_EFFECTIVE_VALUE:
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                const css::uno::TypeClass rTC = Value.getValueType().getTypeClass();
                if (rTC != css::uno::TypeClass_STRING)
                    // no string
                    if (rTC != css::uno::TypeClass_DOUBLE)
                        // no double
                        if (Value.hasValue())
                        {   // but a value
                            // try if it is something convertible
                            sal_Int32 nValue = 0;
                            if (!(Value >>= nValue))
                                throw css::lang::IllegalArgumentException();
                            SetValue(css::uno::makeAny(static_cast<double>(nValue)));
                            break;
                        }

                SetValue(Value);
            }
            break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:
            {
                double d = 0.0;
                if ( Value >>= d )
                     rFormatter.SetSpinSize( d );
                else
                {
                    sal_Int32 n = 0;
                    if ( Value >>= n )
                         rFormatter.SetSpinSize( n );
                }
            }
            break;
            case BASEPROPERTY_DECIMALACCURACY:
            {
                sal_Int32 n = 0;
                if ( Value >>= n )
                     rFormatter.SetDecimalDigits( static_cast<sal_uInt16>(n) );
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                bool b;
                if ( Value >>= b )
                    rFormatter.SetThousandsSep( b );
            }
            break;

            default:
                VCLXSpinField::setProperty( PropertyName, Value );
        }

        if (BASEPROPERTY_TEXTCOLOR == nPropType)
        {   // after setting a new text color, think again about the AutoColor flag of the control
            // 17.05.2001 - 86859 - frank.schoenheit@germany.sun.com
            rFormatter.SetAutoColor(!Value.hasValue());
        }
    }
    else
        VCLXSpinField::setProperty( PropertyName, Value );
}

css::uno::Any SVTXFormattedField::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aReturn;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
    {
        Formatter& rFormatter = pField->GetFormatter();
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_EFFECTIVE_MIN:
            case BASEPROPERTY_VALUEMIN_DOUBLE:
                aReturn = GetMinValue();
                break;

            case BASEPROPERTY_EFFECTIVE_MAX:
            case BASEPROPERTY_VALUEMAX_DOUBLE:
                aReturn = GetMaxValue();
                break;

            case BASEPROPERTY_EFFECTIVE_DEFAULT:
                aReturn = GetDefaultValue();
                break;

            case BASEPROPERTY_TREATASNUMBER:
                aReturn <<= GetTreatAsNumber();
                break;

            case BASEPROPERTY_EFFECTIVE_VALUE:
            case BASEPROPERTY_VALUE_DOUBLE:
                aReturn = GetValue();
                break;

            case BASEPROPERTY_VALUESTEP_DOUBLE:
                aReturn <<= rFormatter.GetSpinSize();
                break;

            case BASEPROPERTY_DECIMALACCURACY:
                aReturn <<= rFormatter.GetDecimalDigits();
                break;

            case BASEPROPERTY_FORMATSSUPPLIER:
            {
                if (!bIsStandardSupplier)
                {   // ansonsten void
                    css::uno::Reference< css::util::XNumberFormatsSupplier >  xSupplier = m_xCurrentSupplier;
                    aReturn <<= xSupplier;
                }
            }
            break;

            case BASEPROPERTY_FORMATKEY:
            {
                if (!bIsStandardSupplier)
                    aReturn <<= getFormatKey();
            }
            break;

            default:
                aReturn = VCLXSpinField::getProperty(PropertyName);
        }
    }
    return aReturn;
}

css::uno::Any SVTXFormattedField::convertEffectiveValue(const css::uno::Any& rValue)
{
    css::uno::Any aReturn;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return aReturn;

    Formatter& rFieldFormatter = pField->GetFormatter();
    switch (rValue.getValueType().getTypeClass())
    {
        case css::uno::TypeClass_DOUBLE:
            if (rFieldFormatter.TreatingAsNumber())
            {
                double d = 0.0;
                rValue >>= d;
                aReturn <<= d;
            }
            else
            {
                SvNumberFormatter* pFormatter = rFieldFormatter.GetFormatter();
                if (!pFormatter)
                    pFormatter = rFieldFormatter.StandardFormatter();
                    // should never fail

                const Color* pDum;
                double d = 0.0;
                rValue >>= d;
                OUString sConverted;
                pFormatter->GetOutputString(d, 0, sConverted, &pDum);
                aReturn <<= sConverted;
            }
            break;
        case css::uno::TypeClass_STRING:
        {
            OUString aStr;
            rValue >>= aStr;
            if (rFieldFormatter.TreatingAsNumber())
            {
                SvNumberFormatter* pFormatter = rFieldFormatter.GetFormatter();
                if (!pFormatter)
                    pFormatter = rFieldFormatter.StandardFormatter();

                double dVal;
                sal_uInt32 nTestFormat(0);
                if (!pFormatter->IsNumberFormat(aStr, nTestFormat, dVal))
                    aReturn.clear();
                aReturn <<= dVal;
            }
            else
                aReturn <<= aStr;
        }
        break;
        default:
            aReturn.clear();
            break;
    }
    return aReturn;
}

void SVTXFormattedField::SetMinValue(const css::uno::Any& rValue)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return;

    Formatter& rFormatter = pField->GetFormatter();
    switch (rValue.getValueType().getTypeClass())

    {
        case css::uno::TypeClass_DOUBLE:
        {
            double d = 0.0;
            rValue >>= d;
            rFormatter.SetMinValue(d);
            break;
        }
        default:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == css::uno::TypeClass_VOID, "SVTXFormattedField::SetMinValue : invalid argument (an exception will be thrown) !");
            if ( rValue.getValueType().getTypeClass() != css::uno::TypeClass_VOID )

            {
                throw css::lang::IllegalArgumentException();
            }
            rFormatter.ClearMinValue();
            break;
    }
}

css::uno::Any SVTXFormattedField::GetMinValue() const
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return css::uno::Any();
    Formatter& rFormatter = pField->GetFormatter();
    if (!rFormatter.HasMinValue())
        return css::uno::Any();

    css::uno::Any aReturn;
    aReturn <<= rFormatter.GetMinValue();
    return aReturn;
}

void SVTXFormattedField::SetMaxValue(const css::uno::Any& rValue)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return;

    Formatter& rFormatter = pField->GetFormatter();
    switch (rValue.getValueType().getTypeClass())
    {
        case css::uno::TypeClass_DOUBLE:
        {
            double d = 0.0;
            rValue >>= d;
            rFormatter.SetMaxValue(d);
            break;
        }
        default:
            if (rValue.getValueType().getTypeClass() != css::uno::TypeClass_VOID)

            {
                throw css::lang::IllegalArgumentException();
            }
            rFormatter.ClearMaxValue();
            break;
    }
}

css::uno::Any SVTXFormattedField::GetMaxValue() const
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return css::uno::Any();
    Formatter& rFormatter = pField->GetFormatter();
    if (!rFormatter.HasMaxValue())
        return css::uno::Any();

    css::uno::Any aReturn;
    aReturn <<= rFormatter.GetMaxValue();
    return aReturn;
}

void SVTXFormattedField::SetDefaultValue(const css::uno::Any& rValue)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return;

    css::uno::Any aConverted = convertEffectiveValue(rValue);

    Formatter& rFormatter = pField->GetFormatter();
    switch (aConverted.getValueType().getTypeClass())
    {
        case css::uno::TypeClass_DOUBLE:
        {
            double d = 0.0;
            aConverted >>= d;
            rFormatter.SetDefaultValue(d);
        }
        break;
        case css::uno::TypeClass_STRING:
        {
            OUString aStr;
            aConverted >>= aStr;
            rFormatter.SetDefaultText( aStr );
        }
        break;
        default:
            rFormatter.EnableEmptyField(true);
                // only void accepted
            break;
    }
}

css::uno::Any SVTXFormattedField::GetDefaultValue() const
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return css::uno::Any();
    Formatter& rFormatter = pField->GetFormatter();
    if (rFormatter.IsEmptyFieldEnabled())
        return css::uno::Any();

    css::uno::Any aReturn;
    if (rFormatter.TreatingAsNumber())
        aReturn <<= rFormatter.GetDefaultValue();
    else
        aReturn <<= rFormatter.GetDefaultText();
    return aReturn;
}

bool SVTXFormattedField::GetTreatAsNumber() const
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (pField)
        return pField->GetFormatter().TreatingAsNumber();

    return true;
}

void SVTXFormattedField::SetTreatAsNumber(bool bSet)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (pField)
        pField->GetFormatter().TreatAsNumber(bSet);
}

css::uno::Any SVTXFormattedField::GetValue() const
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return css::uno::Any();

    Formatter& rFormatter = pField->GetFormatter();
    css::uno::Any aReturn;
    if (!rFormatter.TreatingAsNumber())
    {
        OUString sText = rFormatter.GetTextValue();
        aReturn <<= sText;
    }
    else
    {
        if (!pField->GetText().isEmpty())    // empty is returned as void by default
            aReturn <<= rFormatter.GetValue();
    }

    return aReturn;
}

void SVTXFormattedField::SetValue(const css::uno::Any& rValue)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return;

    if (!rValue.hasValue())
    {
        pField->SetText("");
    }
    else
    {
        Formatter& rFormatter = pField->GetFormatter();
        if (rValue.getValueType().getTypeClass() == css::uno::TypeClass_DOUBLE )
        {
            double d = 0.0;
            rValue >>= d;
            rFormatter.SetValue(d);
        }
        else
        {
            DBG_ASSERT(rValue.getValueType().getTypeClass() == css::uno::TypeClass_STRING, "SVTXFormattedField::SetValue : invalid argument !");

            OUString sText;
            rValue >>= sText;
            if (!rFormatter.TreatingAsNumber())
                rFormatter.SetTextFormatted(sText);
            else
                rFormatter.SetTextValue(sText);
        }
    }
//  NotifyTextListeners();
}

void SVTXFormattedField::setFormatsSupplier(const css::uno::Reference< css::util::XNumberFormatsSupplier > & xSupplier)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();

    rtl::Reference<SvNumberFormatsSupplierObj> pNew;
    if (!xSupplier.is())
    {
        if (pField)
        {
            Formatter& rFormatter = pField->GetFormatter();
            pNew = new SvNumberFormatsSupplierObj(rFormatter.StandardFormatter());
            bIsStandardSupplier = true;
        }
    }
    else
    {
        pNew = comphelper::getUnoTunnelImplementation<SvNumberFormatsSupplierObj>(xSupplier);
        bIsStandardSupplier = false;
    }

    if (!pNew)
        return;     // TODO : how to process ?

    m_xCurrentSupplier = pNew;
    if (!pField)
        return;

    // save the actual value
    css::uno::Any aCurrent = GetValue();
    Formatter& rFormatter = pField->GetFormatter();
    rFormatter.SetFormatter(m_xCurrentSupplier->GetNumberFormatter(), false);
    if (nKeyToSetDelayed != -1)
    {
        rFormatter.SetFormatKey(nKeyToSetDelayed);
        nKeyToSetDelayed = -1;
    }
    SetValue(aCurrent);
    NotifyTextListeners();
}

sal_Int32 SVTXFormattedField::getFormatKey() const
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetFormatKey() : 0;
}

void SVTXFormattedField::setFormatKey(sal_Int32 nKey)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return;

    Formatter& rFormatter = pField->GetFormatter();
    if (rFormatter.GetFormatter())
        rFormatter.SetFormatKey(nKey);
    else
    {
        // probably I am in a block, in which first the key and next the formatter will be set,
        // initially this happens quite certain, as the properties are set in alphabetic sequence,
        // and the FormatsSupplier is processed before the FormatKey
        nKeyToSetDelayed = nKey;
    }
    NotifyTextListeners();
}

void SVTXFormattedField::NotifyTextListeners()
{
    if ( GetTextListeners().getLength() )
    {
        css::awt::TextEvent aEvent;
        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
        GetTextListeners().textChanged( aEvent );
    }
}

void SVTXFormattedField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     // FIXME: elide duplication ?
                     BASEPROPERTY_EFFECTIVE_MIN,
                     BASEPROPERTY_VALUEMIN_DOUBLE,
                     BASEPROPERTY_EFFECTIVE_MAX,
                     BASEPROPERTY_VALUEMAX_DOUBLE,
                     BASEPROPERTY_EFFECTIVE_DEFAULT,
                     BASEPROPERTY_TREATASNUMBER,
                     BASEPROPERTY_EFFECTIVE_VALUE,
                     BASEPROPERTY_VALUE_DOUBLE,
                     BASEPROPERTY_VALUESTEP_DOUBLE,
                     BASEPROPERTY_DECIMALACCURACY,
                     BASEPROPERTY_FORMATSSUPPLIER,
                     BASEPROPERTY_NUMSHOWTHOUSANDSEP,
                     BASEPROPERTY_FORMATKEY,
                     BASEPROPERTY_TREATASNUMBER,
                     BASEPROPERTY_ENFORCE_FORMAT,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );
    VCLXSpinField::ImplGetPropertyIds( rIds );
}

SVTXCurrencyField::SVTXCurrencyField()
{
}

SVTXCurrencyField::~SVTXCurrencyField()
{
}

css::uno::Any SVTXCurrencyField::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XCurrencyField* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : SVTXFormattedField::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( SVTXCurrencyField )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > SVTXCurrencyField::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XCurrencyField>::get(),
        SVTXFormattedField::getTypes()
    );
    return aTypeList.getTypes();
}

void SVTXCurrencyField::setValue( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetValue( Value );
}

double SVTXCurrencyField::getValue()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetValue() : 0;
}

void SVTXCurrencyField::setMin( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetMinValue( Value );
}

double SVTXCurrencyField::getMin()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetMinValue() : 0;
}

void SVTXCurrencyField::setMax( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetMaxValue( Value );
}

double SVTXCurrencyField::getMax()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetMaxValue() : 0;
}

void SVTXCurrencyField::setFirst( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetSpinFirst( Value );
}

double SVTXCurrencyField::getFirst()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetSpinFirst() : 0;
}

void SVTXCurrencyField::setLast( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetSpinLast( Value );
}

double SVTXCurrencyField::getLast()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetSpinLast() : 0;
}

void SVTXCurrencyField::setSpinSize( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetSpinSize( Value );
}

double SVTXCurrencyField::getSpinSize()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetSpinSize() : 0;
}

void SVTXCurrencyField::setDecimalDigits( sal_Int16 Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetDecimalDigits( Value );
}

sal_Int16 SVTXCurrencyField::getDecimalDigits()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetDecimalDigits() : 0;
}

void SVTXCurrencyField::setStrictFormat( sal_Bool bStrict )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetStrictFormat( bStrict );
}

sal_Bool SVTXCurrencyField::isStrictFormat()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField && pField->GetFormatter().IsStrictFormat();
}

void SVTXCurrencyField::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< DoubleCurrencyField > pField = GetAs< DoubleCurrencyField >();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
                OUString aStr;
                Value >>= aStr;
                pField->setCurrencySymbol( aStr );
            }
            break;
            case BASEPROPERTY_CURSYM_POSITION:
            {
                bool b = false;
                Value >>= b;
                pField->setPrependCurrSym(b);
            }
            break;

            default:
                SVTXFormattedField::setProperty(PropertyName, Value);
        }
    }
    else
        SVTXFormattedField::setProperty(PropertyName, Value);
}

css::uno::Any SVTXCurrencyField::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aReturn;

    VclPtr< DoubleCurrencyField > pField = GetAs< DoubleCurrencyField >();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
                aReturn <<= pField->getCurrencySymbol();
            }
            break;
            case BASEPROPERTY_CURSYM_POSITION:
            {
                aReturn <<= pField->getPrependCurrSym();
            }
            break;
            default:
                return SVTXFormattedField::getProperty(PropertyName);
        }
    }
    return SVTXFormattedField::getProperty(PropertyName);
}

void SVTXCurrencyField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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
    VCLXWindow::ImplGetPropertyIds( rIds );
}

SVTXNumericField::SVTXNumericField()
{
}

SVTXNumericField::~SVTXNumericField()
{
}

css::uno::Any SVTXNumericField::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XNumericField* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : SVTXFormattedField::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( SVTXNumericField )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > SVTXNumericField::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XNumericField>::get(),
        SVTXFormattedField::getTypes()
    );
    return aTypeList.getTypes();
}

void SVTXNumericField::setValue( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetValue( Value );
}

double SVTXNumericField::getValue()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetValue() : 0;
}

void SVTXNumericField::setMin( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetMinValue( Value );
}

double SVTXNumericField::getMin()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetMinValue() : 0;
}

void SVTXNumericField::setMax( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetMaxValue( Value );
}

double SVTXNumericField::getMax()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetMaxValue() : 0;
}

void SVTXNumericField::setFirst( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetSpinFirst( Value );
}

double SVTXNumericField::getFirst()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetSpinFirst() : 0;
}

void SVTXNumericField::setLast( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetSpinLast( Value );
}

double SVTXNumericField::getLast()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetSpinLast() : 0;
}

void SVTXNumericField::setSpinSize( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetSpinSize( Value );
}

double SVTXNumericField::getSpinSize()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetSpinSize() : 0;
}

void SVTXNumericField::setDecimalDigits( sal_Int16 Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetDecimalDigits( Value );
}

sal_Int16 SVTXNumericField::getDecimalDigits()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatter().GetDecimalDigits() : 0;
}

void SVTXNumericField::setStrictFormat( sal_Bool bStrict )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->GetFormatter().SetStrictFormat( bStrict );
}

sal_Bool SVTXNumericField::isStrictFormat()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField && pField->GetFormatter().IsStrictFormat();
}

void SVTXNumericField::GetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    SVTXFormattedField::ImplGetPropertyIds( rIds );
}

SVTXDateField::SVTXDateField()
    :VCLXDateField()
{
}

SVTXDateField::~SVTXDateField()
{
}

void SAL_CALL SVTXDateField::setProperty( const OUString& PropertyName, const css::uno::Any& Value )
{
    VCLXDateField::setProperty( PropertyName, Value );

    // some properties need to be forwarded to the sub edit, too
    SolarMutexGuard g;
    VclPtr< Edit > pSubEdit = GetWindow() ? GetAs<Edit>()->GetSubEdit() : nullptr;
    if ( !pSubEdit )
        return;

    switch ( GetPropertyId( PropertyName ) )
    {
    case BASEPROPERTY_TEXTLINECOLOR:
        if ( !Value.hasValue() )
            pSubEdit->SetTextLineColor();
        else
        {
            Color nColor;
            if ( Value >>= nColor )
                pSubEdit->SetTextLineColor( nColor );
        }
        break;
    }
}

void SVTXDateField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_TEXTLINECOLOR,
                     0);
    VCLXDateField::ImplGetPropertyIds( rIds );
}

VCLXMultiLineEdit::VCLXMultiLineEdit()
    :maTextListeners( *this )
    ,meLineEndType( LINEEND_LF )    // default behavior before introducing this property: LF (unix-like)
{
}

VCLXMultiLineEdit::~VCLXMultiLineEdit()
{
}

css::uno::Any VCLXMultiLineEdit::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XTextComponent* >(this),
                                        static_cast< css::awt::XTextArea* >(this),
                                        static_cast< css::awt::XTextLayoutConstrains* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXMultiLineEdit )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXMultiLineEdit::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XTextComponent>::get(),
        cppu::UnoType<css::awt::XTextArea>::get(),
        cppu::UnoType<css::awt::XTextLayoutConstrains>::get(),
        VCLXWindow::getTypes()
    );
    return aTypeList.getTypes();
}

void VCLXMultiLineEdit::addTextListener( const css::uno::Reference< css::awt::XTextListener > & l )
{
    maTextListeners.addInterface( l );
}

void VCLXMultiLineEdit::removeTextListener( const css::uno::Reference< css::awt::XTextListener > & l )
{
    maTextListeners.removeInterface( l );
}

void VCLXMultiLineEdit::setText( const OUString& aText )
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
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

void VCLXMultiLineEdit::insertText( const css::awt::Selection& rSel, const OUString& aText )
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
    {
        setSelection( rSel );
        pEdit->ReplaceSelected( aText );
    }
}

OUString VCLXMultiLineEdit::getText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
        aText = pEdit->GetText( meLineEndType );
    return aText;
}

OUString VCLXMultiLineEdit::getSelectedText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( pMultiLineEdit)
        aText = pMultiLineEdit->GetSelected( meLineEndType );
    return aText;

}

void VCLXMultiLineEdit::setSelection( const css::awt::Selection& aSelection )
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( pMultiLineEdit )
    {
        pMultiLineEdit->SetSelection( Selection( aSelection.Min, aSelection.Max ) );
    }
}

css::awt::Selection VCLXMultiLineEdit::getSelection()
{
    SolarMutexGuard aGuard;

    css::awt::Selection aSel;
    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( pMultiLineEdit )
    {
        aSel.Min = pMultiLineEdit->GetSelection().Min();
        aSel.Max = pMultiLineEdit->GetSelection().Max();
    }
    return aSel;
}

sal_Bool VCLXMultiLineEdit::isEditable()
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    return pMultiLineEdit && !pMultiLineEdit->IsReadOnly() && pMultiLineEdit->IsEnabled();
}

void VCLXMultiLineEdit::setEditable( sal_Bool bEditable )
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( pMultiLineEdit )
        pMultiLineEdit->SetReadOnly( !bEditable );
}

void VCLXMultiLineEdit::setMaxTextLen( sal_Int16 nLen )
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( pMultiLineEdit )
        pMultiLineEdit->SetMaxTextLen( nLen );
}

sal_Int16 VCLXMultiLineEdit::getMaxTextLen()
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    return pMultiLineEdit ? static_cast<sal_Int16>(pMultiLineEdit->GetMaxTextLen()) : sal_Int16(0);
}

OUString VCLXMultiLineEdit::getTextLines()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
        aText = pEdit->GetTextLines( meLineEndType );
    return aText;
}

css::awt::Size VCLXMultiLineEdit::getMinimumSize()
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz;
    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcMinimumSize());
    return aSz;
}

css::awt::Size VCLXMultiLineEdit::getPreferredSize()
{
    return getMinimumSize();
}

css::awt::Size VCLXMultiLineEdit::calcAdjustedSize( const css::awt::Size& rNewSize )
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz = rNewSize;
    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcAdjustedSize( VCLSize(rNewSize )));
    return aSz;
}

css::awt::Size VCLXMultiLineEdit::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines )
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz;
    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcBlockSize( nCols, nLines ));
    return aSz;
}

void VCLXMultiLineEdit::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines )
{
    SolarMutexGuard aGuard;

    nCols = nLines = 0;
    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
    {
        sal_uInt16 nC, nL;
        pEdit->GetMaxVisColumnsAndLines( nC, nL );
        nCols = nC;
        nLines = nL;
    }
}

void VCLXMultiLineEdit::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::EditModify:
        {
            if ( maTextListeners.getLength() )
            {
                css::awt::TextEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                maTextListeners.textChanged( aEvent );
            }
        }
        break;
        default:
        {
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
        }
        break;
    }
}

void VCLXMultiLineEdit::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( !pMultiLineEdit )
        return;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_LINE_END_FORMAT:
        {
            sal_Int16 nLineEndType = css::awt::LineEndFormat::LINE_FEED;
            OSL_VERIFY( Value >>= nLineEndType );
            switch ( nLineEndType )
            {
            case css::awt::LineEndFormat::CARRIAGE_RETURN:           meLineEndType = LINEEND_CR; break;
            case css::awt::LineEndFormat::LINE_FEED:                 meLineEndType = LINEEND_LF; break;
            case css::awt::LineEndFormat::CARRIAGE_RETURN_LINE_FEED: meLineEndType = LINEEND_CRLF; break;
            default: OSL_FAIL( "VCLXMultiLineEdit::setProperty: invalid line end value!" ); break;
            }
        }
        break;

        case BASEPROPERTY_READONLY:
        {
            bool b;
            if ( Value >>= b )
                pMultiLineEdit->SetReadOnly( b );
        }
        break;
        case BASEPROPERTY_MAXTEXTLEN:
        {
            sal_Int16 n = sal_Int16();
            if ( Value >>= n )
                pMultiLineEdit->SetMaxTextLen( n );
        }
        break;
        case BASEPROPERTY_HIDEINACTIVESELECTION:
        {
            bool b;
            if ( Value >>= b )
            {
                pMultiLineEdit->EnableFocusSelectionHide( b );
                lcl_setWinBits( pMultiLineEdit, WB_NOHIDESELECTION, !b );
            }
        }
        break;
        default:
        {
            VCLXWindow::setProperty( PropertyName, Value );
        }
    }
}

css::uno::Any VCLXMultiLineEdit::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( pMultiLineEdit )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LINE_END_FORMAT:
            {
                sal_Int16 nLineEndType = css::awt::LineEndFormat::LINE_FEED;
                switch ( meLineEndType )
                {
                case LINEEND_CR:   nLineEndType = css::awt::LineEndFormat::CARRIAGE_RETURN; break;
                case LINEEND_LF:   nLineEndType = css::awt::LineEndFormat::LINE_FEED; break;
                case LINEEND_CRLF: nLineEndType = css::awt::LineEndFormat::CARRIAGE_RETURN_LINE_FEED; break;
                default: OSL_FAIL( "VCLXMultiLineEdit::getProperty: invalid line end value!" ); break;
                }
                aProp <<= nLineEndType;
            }
            break;

            case BASEPROPERTY_READONLY:
            {
                aProp <<= pMultiLineEdit->IsReadOnly();
            }
            break;
            case BASEPROPERTY_MAXTEXTLEN:
            {
                aProp <<= static_cast<sal_Int16>(pMultiLineEdit->GetMaxTextLen());
            }
            break;
            default:
            {
                aProp = VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void SAL_CALL VCLXMultiLineEdit::setFocus(  )
{
    SolarMutexGuard aGuard;

    // don't grab the focus if we already have it. Reason is that the only thing which the edit
    // does is forwarding the focus to its text window. This text window then does a "select all".
    // So if the text window already has the focus, and we give the focus to the multi line
    // edit, then all which happens is that everything is selected.
    // #i27072#
    if ( GetWindow() && !GetWindow()->HasChildPathFocus() )
        GetWindow()->GrabFocus();
}

void VCLXMultiLineEdit::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     // FIXME: elide duplication ?
                     BASEPROPERTY_LINE_END_FORMAT,
                     BASEPROPERTY_READONLY,
                     BASEPROPERTY_MAXTEXTLEN,
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
