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

#include <toolkit/awt/vclxwindows.hxx>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/imagealign.hxx>
#include <toolkit/helper/accessibilityclient.hxx>
#include <toolkit/helper/fixedhyperbase.hxx>
#include <toolkit/helper/tkresmgr.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <com/sun/star/awt/XItemList.hpp>
#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/field.hxx>
#include <vcl/longcurr.hxx>
#include <vcl/imgctrl.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <tools/diagnose_ex.h>

#include <boost/bind.hpp>
#include <boost/function.hpp>

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
    void setButtonLikeFaceColor( Window* _pWindow, const ::com::sun::star::uno::Any& _rColorValue )
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
        _pWindow->SetSettings( aSettings, sal_True );
    }

    Any getButtonLikeFaceColor( const Window* _pWindow )
    {
        sal_Int32 nBackgroundColor = _pWindow->GetSettings().GetStyleSettings().GetFaceColor().GetColor();
        return makeAny( nBackgroundColor );
    }

    static void adjustBooleanWindowStyle( const Any& _rValue, Window* _pWindow, WinBits _nBits, sal_Bool _bInverseSemantics )
    {
        WinBits nStyle = _pWindow->GetStyle();
        sal_Bool bValue( sal_False );
        OSL_VERIFY( _rValue >>= bValue );
        if ( bValue != _bInverseSemantics )
            nStyle |= _nBits;
        else
            nStyle &= ~_nBits;
        _pWindow->SetStyle( nStyle );
    }

    static void setVisualEffect( const Any& _rValue, Window* _pWindow )
    {
        AllSettings aSettings = _pWindow->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();

        sal_Int16 nStyle = LOOK3D;
        OSL_VERIFY( _rValue >>= nStyle );
        switch ( nStyle )
        {
        case FLAT:
            aStyleSettings.SetOptions( aStyleSettings.GetOptions() & ~STYLE_OPTION_MONO );
            break;
        case LOOK3D:
        default:
            aStyleSettings.SetOptions( aStyleSettings.GetOptions() | STYLE_OPTION_MONO );
        }
        aSettings.SetStyleSettings( aStyleSettings );
        _pWindow->SetSettings( aSettings );
    }

    static Any getVisualEffect( Window* _pWindow )
    {
        Any aEffect;

        StyleSettings aStyleSettings = _pWindow->GetSettings().GetStyleSettings();
        if ( (aStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
            aEffect <<= (sal_Int16)FLAT;
        else
            aEffect <<= (sal_Int16)LOOK3D;
        return aEffect;
    }
}

//  ----------------------------------------------------
//  class VCLXGraphicControl
//  ----------------------------------------------------

void VCLXGraphicControl::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    VCLXWindow::ImplGetPropertyIds( rIds );
}

void VCLXGraphicControl::ImplSetNewImage()
{
    OSL_PRECOND( GetWindow(), "VCLXGraphicControl::ImplSetNewImage: window is required to be not-NULL!" );
    Button* pButton = static_cast< Button* >( GetWindow() );
    pButton->SetModeImage( GetImage() );
}

void VCLXGraphicControl::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, short Flags ) throw(::com::sun::star::uno::RuntimeException)
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

void VCLXGraphicControl::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Button* pButton = static_cast< Button* >( GetWindow() );
    if ( !pButton )
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
                    pButton->SetImageAlign( static_cast< ImageAlign >( nAlignment ) );
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
                pButton->SetImageAlign( ::toolkit::translateImagePosition( nImagePosition ) );
            }
        }
        break;
        default:
            VCLXWindow::setProperty( PropertyName, Value );
            break;
    }
}

::com::sun::star::uno::Any VCLXGraphicControl::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
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
                 aProp <<= ::toolkit::getCompatibleImageAlign( static_cast< Button* >( GetWindow() )->GetImageAlign() );
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
                aProp <<= ::toolkit::translateImagePosition( static_cast< Button* >( GetWindow() )->GetImageAlign() );
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

//--------------------------------------------------------------------
//  class VCLXButton
//  ----------------------------------------------------

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

void VCLXButton::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aObj );
    maItemListeners.disposeAndClear( aObj );
    VCLXGraphicControl::dispose();
}

void VCLXButton::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l  )throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXButton::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXButton::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l  )throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXButton::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXButton::setLabel( const ::rtl::OUString& rLabel ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rLabel );
}

void VCLXButton::setActionCommand( const ::rtl::OUString& rCommand ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    maActionCommand = rCommand;
}

::com::sun::star::awt::Size VCLXButton::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    PushButton* pButton = (PushButton*) GetWindow();
    if ( pButton )
        aSz = pButton->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXButton::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::Size aSz = getMinimumSize();
    aSz.Width += 16;
    aSz.Height += 10;
    return aSz;
}

::com::sun::star::awt::Size VCLXButton::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz = VCLSize(rNewSize);
    PushButton* pButton = (PushButton*) GetWindow();
    if ( pButton )
    {
        Size aMinSz = pButton->CalcMinimumSize();
        // Kein Text, also Image
        if ( !pButton->GetText().Len() )
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

void VCLXButton::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Button* pButton = (Button*)GetWindow();
    if ( pButton )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_FOCUSONCLICK:
                ::toolkit::adjustBooleanWindowStyle( Value, pButton, WB_NOPOINTERFOCUS, sal_True );
                break;

            case BASEPROPERTY_TOGGLE:
                ::toolkit::adjustBooleanWindowStyle( Value, pButton, WB_TOGGLE, sal_False );
                break;

            case BASEPROPERTY_DEFAULTBUTTON:
            {
                WinBits nStyle = pButton->GetStyle() | WB_DEFBUTTON;
                sal_Bool b = sal_Bool();
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
                        ((PushButton*)pButton)->SetState( (TriState)n );
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

::com::sun::star::uno::Any VCLXButton::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    Button* pButton = (Button*)GetWindow();
    if ( pButton )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_FOCUSONCLICK:
                aProp <<= (sal_Bool)( ( pButton->GetStyle() & WB_NOPOINTERFOCUS ) == 0 );
                break;

            case BASEPROPERTY_TOGGLE:
                aProp <<= (sal_Bool)( ( pButton->GetStyle() & WB_TOGGLE ) != 0 );
                break;

            case BASEPROPERTY_DEFAULTBUTTON:
            {
                aProp <<= (sal_Bool) ( ( pButton->GetStyle() & WB_DEFBUTTON ) ? sal_True : sal_False );
            }
            break;
            case BASEPROPERTY_STATE:
            {
                if ( GetWindow()->GetType() == WINDOW_PUSHBUTTON )
                {
                     aProp <<= (sal_Int16)((PushButton*)pButton)->GetState();
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
                aEvent.Source = (::cppu::OWeakObject*)this;
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
                aEvent.Source = (::cppu::OWeakObject*)this;
                aEvent.Selected = ( rButton.GetState() == STATE_CHECK ) ? 1 : 0;
                maItemListeners.itemStateChanged( aEvent );
            }
        }
        break;

        default:
            VCLXGraphicControl::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

//  ----------------------------------------------------
//  class VCLXImageControl
//  ----------------------------------------------------

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
    ImageControl* pControl = static_cast< ImageControl* >( GetWindow() );
    pControl->SetImage( GetImage() );
}

::com::sun::star::awt::Size VCLXImageControl::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz = GetImage().GetSizePixel();
    aSz = ImplCalcWindowSize( aSz );

    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXImageControl::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXImageControl::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
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

void VCLXImageControl::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ImageControl* pImageControl = (ImageControl*)GetWindow();

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_IMAGE_SCALE_MODE:
        {
            sal_Int16 nScaleMode( ImageScaleMode::Anisotropic );
            if ( pImageControl && ( Value >>= nScaleMode ) )
            {
                pImageControl->SetScaleMode( nScaleMode );
            }
        }
        break;

        case BASEPROPERTY_SCALEIMAGE:
        {
            // this is for compatibility only, nowadays, the ImageScaleMode property should be used
            sal_Bool bScaleImage = sal_False;
            if ( pImageControl && ( Value >>= bScaleImage ) )
            {
                pImageControl->SetScaleMode( bScaleImage ? ImageScaleMode::Anisotropic : ImageScaleMode::None );
            }
        }
        break;

        default:
            VCLXGraphicControl::setProperty( PropertyName, Value );
            break;
    }
}

::com::sun::star::uno::Any VCLXImageControl::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    ImageControl* pImageControl = (ImageControl*)GetWindow();
    sal_uInt16 nPropType = GetPropertyId( PropertyName );

    switch ( nPropType )
    {
        case BASEPROPERTY_IMAGE_SCALE_MODE:
            aProp <<= ( pImageControl ? pImageControl->GetScaleMode() : ImageScaleMode::Anisotropic );
            break;

        case BASEPROPERTY_SCALEIMAGE:
            aProp <<= ( pImageControl && pImageControl->GetScaleMode() != ImageScaleMode::None ) ? sal_True : sal_False;
            break;

        default:
            aProp = VCLXGraphicControl::getProperty( PropertyName );
            break;
    }
    return aProp;
}

//  ----------------------------------------------------
//  class VCLXCheckBox
//  ----------------------------------------------------


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
::com::sun::star::uno::Any VCLXCheckBox::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XButton* >(this)),
                                        (static_cast< ::com::sun::star::awt::XCheckBox* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXGraphicControl::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXCheckBox )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XButton>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCheckBox>* ) NULL ),
    VCLXGraphicControl::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXCheckBox::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXCheckBox::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aObj );
    VCLXGraphicControl::dispose();
}

void VCLXCheckBox::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXCheckBox::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXCheckBox::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l  )throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXCheckBox::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXCheckBox::setActionCommand( const ::rtl::OUString& rCommand ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maActionCommand = rCommand;
}

void VCLXCheckBox::setLabel( const ::rtl::OUString& rLabel ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rLabel );
}

void VCLXCheckBox::setState( short n ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CheckBox* pCheckBox = (CheckBox*)GetWindow();
    if ( pCheckBox)
    {
        TriState eState;
        switch ( n )
        {
            case 0:     eState = STATE_NOCHECK;     break;
            case 1:     eState = STATE_CHECK;       break;
            case 2:     eState = STATE_DONTKNOW;    break;
            default:    eState = STATE_NOCHECK;
        }
        pCheckBox->SetState( eState );

        // #105198# call C++ click listeners (needed for accessibility)
        // pCheckBox->GetClickHdl().Call( pCheckBox );

        // #107218# Call same virtual methods and listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pCheckBox->Toggle();
        pCheckBox->Click();
        SetSynthesizingVCLEvent( sal_False );
    }
}

short VCLXCheckBox::getState() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    short nState = -1;
    CheckBox* pCheckBox = (CheckBox*)GetWindow();
    if ( pCheckBox )
    {
        switch ( pCheckBox->GetState() )
        {
            case STATE_NOCHECK:     nState = 0; break;
            case STATE_CHECK:       nState = 1; break;
            case STATE_DONTKNOW:    nState = 2; break;
            default:                OSL_FAIL( "VCLXCheckBox::getState(): unknown TriState!" );
        }
    }

    return nState;
}

void VCLXCheckBox::enableTriState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CheckBox* pCheckBox = (CheckBox*)GetWindow();
    if ( pCheckBox)
        pCheckBox->EnableTriState( b );
}

::com::sun::star::awt::Size VCLXCheckBox::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    CheckBox* pCheckBox = (CheckBox*) GetWindow();
    if ( pCheckBox )
        aSz = pCheckBox->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXCheckBox::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXCheckBox::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz = VCLSize(rNewSize);
    CheckBox* pCheckBox = (CheckBox*) GetWindow();
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

void VCLXCheckBox::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CheckBox* pCheckBox = (CheckBox*)GetWindow();
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
                sal_Bool b = sal_Bool();
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

::com::sun::star::uno::Any VCLXCheckBox::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    CheckBox* pCheckBox = (CheckBox*)GetWindow();
    if ( pCheckBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VISUALEFFECT:
                aProp = ::toolkit::getVisualEffect( pCheckBox );
                break;
            case BASEPROPERTY_TRISTATE:
                 aProp <<= (sal_Bool)pCheckBox->IsTriStateEnabled();
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

            CheckBox* pCheckBox = (CheckBox*)GetWindow();
            if ( pCheckBox )
            {
                if ( maItemListeners.getLength() )
                {
                    ::com::sun::star::awt::ItemEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.Highlighted = sal_False;
                    aEvent.Selected = pCheckBox->GetState();
                    maItemListeners.itemStateChanged( aEvent );
                }
                if ( !IsSynthesizingVCLEvent() && maActionListeners.getLength() )
                {
                    ::com::sun::star::awt::ActionEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
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

//  ----------------------------------------------------
//  class VCLXRadioButton
//  ----------------------------------------------------
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
::com::sun::star::uno::Any VCLXRadioButton::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XRadioButton* >(this)),
                                        (static_cast< ::com::sun::star::awt::XButton* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXGraphicControl::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXRadioButton )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRadioButton>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XButton>* ) NULL ),
    VCLXGraphicControl::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXRadioButton::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXRadioButton::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aObj );
    VCLXGraphicControl::dispose();
}

void VCLXRadioButton::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    RadioButton* pButton = (RadioButton*)GetWindow();
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
                    sal_Bool b = n ? sal_True : sal_False;
                    if ( pButton->IsRadioCheckEnabled() )
                        pButton->Check( b );
                    else
                        pButton->SetState( b );
                }
            }
            break;
            case BASEPROPERTY_AUTOTOGGLE:
            {
                sal_Bool b = sal_Bool();
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

::com::sun::star::uno::Any VCLXRadioButton::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    RadioButton* pButton = (RadioButton*)GetWindow();
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
                aProp <<= (sal_Bool) pButton->IsRadioCheckEnabled();
                break;
            default:
            {
                aProp <<= VCLXGraphicControl::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXRadioButton::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXRadioButton::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXRadioButton::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l  )throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXRadioButton::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXRadioButton::setLabel( const ::rtl::OUString& rLabel ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rLabel );
}

void VCLXRadioButton::setActionCommand( const ::rtl::OUString& rCommand ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maActionCommand = rCommand;
}

void VCLXRadioButton::setState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    RadioButton* pRadioButton = (RadioButton*)GetWindow();
    if ( pRadioButton)
    {
        pRadioButton->Check( b );
        // #102717# item listeners are called, but not C++ click listeners in StarOffice code => call click hdl
        // But this is needed in old code because Accessibility API uses it.
        // pRadioButton->GetClickHdl().Call( pRadioButton );

        // #107218# Call same virtual methods and listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pRadioButton->Click();
        SetSynthesizingVCLEvent( sal_False );
    }
}

sal_Bool VCLXRadioButton::getState() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    RadioButton* pRadioButton = (RadioButton*)GetWindow();
    return pRadioButton ? pRadioButton->IsChecked() : sal_False;
}

::com::sun::star::awt::Size VCLXRadioButton::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    RadioButton* pRadioButton = (RadioButton*) GetWindow();
    if ( pRadioButton )
        aSz = pRadioButton->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXRadioButton::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXRadioButton::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz = VCLSize(rNewSize);
    RadioButton* pRadioButton = (RadioButton*) GetWindow();
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
                aEvent.Source = (::cppu::OWeakObject*)this;
                aEvent.ActionCommand = maActionCommand;
                maActionListeners.actionPerformed( aEvent );
            }
            ImplClickedOrToggled( sal_False );
            break;

        case VCLEVENT_RADIOBUTTON_TOGGLE:
            ImplClickedOrToggled( sal_True );
            break;

        default:
            VCLXGraphicControl::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

void VCLXRadioButton::ImplClickedOrToggled( sal_Bool bToggled )
{
    // In the formulars, RadioChecked is not enabled, call itemStateChanged only for click
    // In the dialog editor, RadioChecked is enabled, call itemStateChanged only for bToggled
    RadioButton* pRadioButton = (RadioButton*)GetWindow();
    if ( pRadioButton && ( pRadioButton->IsRadioCheckEnabled() == bToggled ) && ( bToggled || pRadioButton->IsStateChanged() ) && maItemListeners.getLength() )
    {
        ::com::sun::star::awt::ItemEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)this;
        aEvent.Highlighted = sal_False;
        aEvent.Selected = pRadioButton->IsChecked();
        maItemListeners.itemStateChanged( aEvent );
    }
}

//  ----------------------------------------------------
//  class VCLXSpinField
//  ----------------------------------------------------
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
::com::sun::star::uno::Any VCLXSpinField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XSpinField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXEdit::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXSpinField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinField>* ) NULL ),
    VCLXEdit::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXSpinField::addSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maSpinListeners.addInterface( l );
}

void VCLXSpinField::removeSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maSpinListeners.removeInterface( l );
}

void VCLXSpinField::up() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SpinField* pSpinField = (SpinField*) GetWindow();
    if ( pSpinField )
        pSpinField->Up();
}

void VCLXSpinField::down() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SpinField* pSpinField = (SpinField*) GetWindow();
    if ( pSpinField )
        pSpinField->Down();
}

void VCLXSpinField::first() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SpinField* pSpinField = (SpinField*) GetWindow();
    if ( pSpinField )
        pSpinField->First();
}

void VCLXSpinField::last() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SpinField* pSpinField = (SpinField*) GetWindow();
    if ( pSpinField )
        pSpinField->Last();
}

void VCLXSpinField::enableRepeat( sal_Bool bRepeat ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
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
                aEvent.Source = (::cppu::OWeakObject*)this;
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


//  ----------------------------------------------------
//  class VCLXListBox
//  ----------------------------------------------------
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

void VCLXListBox::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aObj );
    maActionListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void VCLXListBox::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXListBox::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXListBox::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXListBox::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXListBox::addItem( const ::rtl::OUString& aItem, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        pBox->InsertEntry( aItem, nPos );
}

void VCLXListBox::addItems( const ::com::sun::star::uno::Sequence< ::rtl::OUString>& aItems, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        sal_uInt16 nP = nPos;
        const ::rtl::OUString* pItems = aItems.getConstArray();
        const ::rtl::OUString* pItemsEnd = aItems.getConstArray() + aItems.getLength();
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

void VCLXListBox::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        for ( sal_uInt16 n = nCount; n; )
            pBox->RemoveEntry( nPos + (--n) );
    }
}

sal_Int16 VCLXListBox::getItemCount() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pBox = (ListBox*) GetWindow();
    return pBox ? pBox->GetEntryCount() : 0;
}

::rtl::OUString VCLXListBox::getItem( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    String aItem;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        aItem = pBox->GetEntry( nPos );
    return aItem;
}

::com::sun::star::uno::Sequence< ::rtl::OUString> VCLXListBox::getItems() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        sal_uInt16 nEntries = pBox->GetEntryCount();
        aSeq = ::com::sun::star::uno::Sequence< ::rtl::OUString>( nEntries );
        for ( sal_uInt16 n = nEntries; n; )
        {
            --n;
            aSeq.getArray()[n] = ::rtl::OUString( pBox->GetEntry( n ) );
        }
    }
    return aSeq;
}

sal_Int16 VCLXListBox::getSelectedItemPos() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pBox = (ListBox*) GetWindow();
    return pBox ? pBox->GetSelectEntryPos() : 0;
}

::com::sun::star::uno::Sequence<sal_Int16> VCLXListBox::getSelectedItemsPos() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Sequence<sal_Int16> aSeq;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        sal_uInt16 nSelEntries = pBox->GetSelectEntryCount();
        aSeq = ::com::sun::star::uno::Sequence<sal_Int16>( nSelEntries );
        for ( sal_uInt16 n = 0; n < nSelEntries; n++ )
            aSeq.getArray()[n] = pBox->GetSelectEntryPos( n );
    }
    return aSeq;
}

::rtl::OUString VCLXListBox::getSelectedItem() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    String aItem;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        aItem = pBox->GetSelectEntry();
    return aItem;
}

::com::sun::star::uno::Sequence< ::rtl::OUString> VCLXListBox::getSelectedItems() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        sal_uInt16 nSelEntries = pBox->GetSelectEntryCount();
        aSeq = ::com::sun::star::uno::Sequence< ::rtl::OUString>( nSelEntries );
        for ( sal_uInt16 n = 0; n < nSelEntries; n++ )
            aSeq.getArray()[n] = ::rtl::OUString( pBox->GetSelectEntry( n ) );
    }
    return aSeq;
}

void VCLXListBox::selectItemPos( sal_Int16 nPos, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox && ( pBox->IsEntryPosSelected( nPos ) != bSelect ) )
    {
        pBox->SelectEntryPos( nPos, bSelect );

        // VCL doesn't call select handler after API call.
        // ImplCallItemListeners();

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pBox->Select();
        SetSynthesizingVCLEvent( sal_False );
    }
}

void VCLXListBox::selectItemsPos( const ::com::sun::star::uno::Sequence<sal_Int16>& aPositions, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        sal_Bool bChanged = sal_False;
        for ( sal_uInt16 n = (sal_uInt16)aPositions.getLength(); n; )
        {
            sal_uInt16 nPos = (sal_uInt16) aPositions.getConstArray()[--n];
            if ( pBox->IsEntryPosSelected( nPos ) != bSelect )
            {
                pBox->SelectEntryPos( nPos, bSelect );
                bChanged = sal_True;
            }
        }

        if ( bChanged )
        {
            // VCL doesn't call select handler after API call.
            // ImplCallItemListeners();

            // #107218# Call same listeners like VCL would do after user interaction
            SetSynthesizingVCLEvent( sal_True );
            pBox->Select();
            SetSynthesizingVCLEvent( sal_False );
        }
    }
}

void VCLXListBox::selectItem( const ::rtl::OUString& rItemText, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        String aItemText( rItemText );
        selectItemPos( pBox->GetEntryPos( aItemText ), bSelect );
    }
}


void VCLXListBox::setDropDownLineCount( sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        pBox->SetDropDownLineCount( nLines );
}

sal_Int16 VCLXListBox::getDropDownLineCount() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int16 nLines = 0;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        nLines = pBox->GetDropDownLineCount();
    return nLines;
}

sal_Bool VCLXListBox::isMutipleMode() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Bool bMulti = sal_False;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        bMulti = pBox->IsMultiSelectionEnabled();
    return bMulti;
}

void VCLXListBox::setMultipleMode( sal_Bool bMulti ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        pBox->EnableMultiSelection( bMulti );
}

void VCLXListBox::makeVisible( sal_Int16 nEntry ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pBox = (ListBox*) GetWindow();
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
            ListBox* pListBox = (ListBox*)GetWindow();

            if( pListBox )
            {
                sal_Bool bDropDown = ( pListBox->GetStyle() & WB_DROPDOWN ) ? sal_True : sal_False;
                if ( bDropDown && !IsSynthesizingVCLEvent() && maActionListeners.getLength() )
                {
                    // Call ActionListener on DropDown event
                    ::com::sun::star::awt::ActionEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
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
                aEvent.Source = (::cppu::OWeakObject*)this;
                aEvent.ActionCommand = ((ListBox*)GetWindow())->GetSelectEntry();
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

void VCLXListBox::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pListBox = (ListBox*)GetWindow();
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
                sal_Bool b = sal_Bool();
                if ( Value >>= b )
                     pListBox->SetReadOnly( b);
            }
            break;
            case BASEPROPERTY_MULTISELECTION:
            {
                sal_Bool b = sal_Bool();
                if ( Value >>= b )
                     pListBox->EnableMultiSelection( b );
            }
            break;
            case BASEPROPERTY_MULTISELECTION_SIMPLEMODE:
                ::toolkit::adjustBooleanWindowStyle( Value, pListBox, WB_SIMPLEMODE, sal_False );
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
                ::com::sun::star::uno::Sequence< ::rtl::OUString> aItems;
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
                    for ( sal_uInt16 n = pListBox->GetEntryCount(); n; )
                        pListBox->SelectEntryPos( --n, sal_False );

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

::com::sun::star::uno::Any VCLXListBox::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    ListBox* pListBox = (ListBox*)GetWindow();
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
                 aProp <<= (sal_Bool) pListBox->IsReadOnly();
            }
            break;
            case BASEPROPERTY_MULTISELECTION:
            {
                 aProp <<= (sal_Bool) pListBox->IsMultiSelectionEnabled();
            }
            break;
            case BASEPROPERTY_MULTISELECTION_SIMPLEMODE:
            {
                aProp <<= (sal_Bool)( ( pListBox->GetStyle() & WB_SIMPLEMODE ) == 0 );
            }
            break;
            case BASEPROPERTY_LINECOUNT:
            {
                 aProp <<= (sal_Int16) pListBox->GetDropDownLineCount();
            }
            break;
            case BASEPROPERTY_STRINGITEMLIST:
            {
                sal_uInt16 nItems = pListBox->GetEntryCount();
                ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq( nItems );
                ::rtl::OUString* pStrings = aSeq.getArray();
                for ( sal_uInt16 n = 0; n < nItems; n++ )
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

::com::sun::star::awt::Size VCLXListBox::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    ListBox* pListBox = (ListBox*) GetWindow();
    if ( pListBox )
        aSz = pListBox->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXListBox::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    ListBox* pListBox = (ListBox*) GetWindow();
    if ( pListBox )
    {
        aSz = pListBox->CalcMinimumSize();
        if ( pListBox->GetStyle() & WB_DROPDOWN )
            aSz.Height() += 4;
    }
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXListBox::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz = VCLSize(rNewSize);
    ListBox* pListBox = (ListBox*) GetWindow();
    if ( pListBox )
        aSz = pListBox->CalcAdjustedSize( aSz );
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXListBox::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    ListBox* pListBox = (ListBox*) GetWindow();
    if ( pListBox )
        aSz = pListBox->CalcSize( nCols, nLines );
    return AWTSize(aSz);
}

void VCLXListBox::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    nCols = nLines = 0;
    ListBox* pListBox = (ListBox*) GetWindow();
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
    ListBox* pListBox = (ListBox*) GetWindow();
    if ( pListBox && maItemListeners.getLength() )
    {
        ::com::sun::star::awt::ItemEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)this;
        aEvent.Highlighted = sal_False;

        // Set to 0xFFFF on multiple selection, selected entry ID otherwise
        aEvent.Selected = (pListBox->GetSelectEntryCount() == 1 ) ? pListBox->GetSelectEntryPos() : 0xFFFF;

        maItemListeners.itemStateChanged( aEvent );
    }
}
namespace
{
     Image lcl_getImageFromURL( const ::rtl::OUString& i_rImageURL )
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
void SAL_CALL VCLXListBox::listItemInserted( const ItemListEvent& i_rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pListBox = dynamic_cast< ListBox* >( GetWindow() );

    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemInserted: no ListBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition <= sal_Int32( pListBox->GetEntryCount() ) ),
        "VCLXListBox::listItemInserted: illegal (inconsistent) item position!" );
    pListBox->InsertEntry(
        i_rEvent.ItemText.IsPresent ? i_rEvent.ItemText.Value : ::rtl::OUString(),
        i_rEvent.ItemImageURL.IsPresent ? TkResMgr::getImageFromURL( i_rEvent.ItemImageURL.Value ) : Image(),
        i_rEvent.ItemPosition );
}

void SAL_CALL VCLXListBox::listItemRemoved( const ItemListEvent& i_rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pListBox = dynamic_cast< ListBox* >( GetWindow() );

    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemRemoved: no ListBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition < sal_Int32( pListBox->GetEntryCount() ) ),
        "VCLXListBox::listItemRemoved: illegal (inconsistent) item position!" );

    pListBox->RemoveEntry( i_rEvent.ItemPosition );
}

void SAL_CALL VCLXListBox::listItemModified( const ItemListEvent& i_rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pListBox = dynamic_cast< ListBox* >( GetWindow() );

    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemModified: no ListBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition < sal_Int32( pListBox->GetEntryCount() ) ),
        "VCLXListBox::listItemModified: illegal (inconsistent) item position!" );

    // VCL's ListBox does not support changing an entry's text or image, so remove and re-insert

    const ::rtl::OUString sNewText = i_rEvent.ItemText.IsPresent ? i_rEvent.ItemText.Value : ::rtl::OUString( pListBox->GetEntry( i_rEvent.ItemPosition ) );
    const Image aNewImage( i_rEvent.ItemImageURL.IsPresent ? TkResMgr::getImageFromURL( i_rEvent.ItemImageURL.Value ) : pListBox->GetEntryImage( i_rEvent.ItemPosition  ) );

    pListBox->RemoveEntry( i_rEvent.ItemPosition );
    pListBox->InsertEntry( sNewText, aNewImage, i_rEvent.ItemPosition );
}

void SAL_CALL VCLXListBox::allItemsRemoved( const EventObject& i_rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pListBox = dynamic_cast< ListBox* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemModified: no ListBox?!" );

    pListBox->Clear();

    (void)i_rEvent;
}

void SAL_CALL VCLXListBox::itemListChanged( const EventObject& i_rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    ListBox* pListBox = dynamic_cast< ListBox* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pListBox, "VCLXListBox::listItemModified: no ListBox?!" );

    pListBox->Clear();

    uno::Reference< beans::XPropertySet > xPropSet( i_rEvent.Source, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySetInfo > xPSI( xPropSet->getPropertySetInfo(), uno::UNO_QUERY_THROW );
    uno::Reference< resource::XStringResourceResolver > xStringResourceResolver;
    if ( xPSI->hasPropertyByName( ::rtl::OUString( "ResourceResolver" ) ) )
    {
        xStringResourceResolver.set(
            xPropSet->getPropertyValue( ::rtl::OUString( "ResourceResolver" ) ),
            uno::UNO_QUERY
        );
    }


    Reference< XItemList > xItemList( i_rEvent.Source, uno::UNO_QUERY_THROW );
    uno::Sequence< beans::Pair< ::rtl::OUString, ::rtl::OUString > > aItems = xItemList->getAllItems();
    for ( sal_Int32 i=0; i<aItems.getLength(); ++i )
    {
        ::rtl::OUString aLocalizationKey( aItems[i].First );
        if ( xStringResourceResolver.is() && !aLocalizationKey.isEmpty() && aLocalizationKey[0] == '&' )
        {
            aLocalizationKey = xStringResourceResolver->resolveString(aLocalizationKey.copy( 1 ));
        }
        pListBox->InsertEntry( aLocalizationKey, lcl_getImageFromURL( aItems[i].Second ) );
    }
}

void SAL_CALL VCLXListBox::disposing( const EventObject& i_rEvent ) throw (RuntimeException)
{
    // just disambiguate
    VCLXWindow::disposing( i_rEvent );
}

//  ----------------------------------------------------
//  class VCLXMessageBox
//  ----------------------------------------------------

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
::com::sun::star::uno::Any VCLXMessageBox::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XMessageBox* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXTopWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXMessageBox )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMessageBox>* ) NULL ),
    VCLXTopWindow::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXMessageBox::setCaptionText( const ::rtl::OUString& rText ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rText );
}

::rtl::OUString VCLXMessageBox::getCaptionText() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    String aText;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

void VCLXMessageBox::setMessageText( const ::rtl::OUString& rText ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    MessBox* pBox = (MessBox*)GetWindow();
    if ( pBox )
        pBox->SetMessText( rText );
}

::rtl::OUString VCLXMessageBox::getMessageText() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aText;
    MessBox* pBox = (MessBox*)GetWindow();
    if ( pBox )
        aText = pBox->GetMessText();
    return aText;
}

sal_Int16 VCLXMessageBox::execute() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    MessBox* pBox = (MessBox*)GetWindow();
    return pBox ? pBox->Execute() : 0;
}

::com::sun::star::awt::Size SAL_CALL VCLXMessageBox::getMinimumSize() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ::com::sun::star::awt::Size( 250, 100 );
}

//  ----------------------------------------------------
//  class VCLXDialog
//  ----------------------------------------------------
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
#ifndef __SUNPRO_CC
    OSL_TRACE ("%s", __FUNCTION__);
#endif
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXDialog::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::document::XVbaMethodParameter* >(this)),
                                        (static_cast< ::com::sun::star::awt::XDialog2* >(this)),
                                        (static_cast< ::com::sun::star::awt::XDialog* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXTopWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXDialog )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::document::XVbaMethodParameter>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDialog2>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDialog>* ) NULL ),
    VCLXTopWindow::getTypes()
IMPL_XTYPEPROVIDER_END

void SAL_CALL VCLXDialog::endDialog( ::sal_Int32 i_result ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    Dialog* pDialog = dynamic_cast< Dialog* >( GetWindow() );
    if ( pDialog )
        pDialog->EndDialog( i_result );
}

void SAL_CALL VCLXDialog::setHelpId( const ::rtl::OUString& rId ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetHelpId( rtl::OUStringToOString( rId, RTL_TEXTENCODING_UTF8 ) );
}

void VCLXDialog::setTitle( const ::rtl::OUString& Title ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( Title );
}

::rtl::OUString VCLXDialog::getTitle() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aTitle;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aTitle = pWindow->GetText();
    return aTitle;
}

sal_Int16 VCLXDialog::execute() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int16 nRet = 0;
    if ( GetWindow() )
    {
        Dialog* pDlg = (Dialog*) GetWindow();
        Window* pParent = pDlg->GetWindow( WINDOW_PARENTOVERLAP );
        Window* pOldParent = NULL;
        Window* pSetParent = NULL;
        if ( pParent && !pParent->IsReallyVisible() )
        {
            pOldParent = pDlg->GetParent();
            Window* pFrame = pDlg->GetWindow( WINDOW_FRAME );
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

void VCLXDialog::endExecute() throw(::com::sun::star::uno::RuntimeException)
{
    endDialog(0);
}

void SAL_CALL VCLXDialog::draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Window* pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Size aSize = pDev->PixelToLogic( pWindow->GetSizePixel() );
        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );

        pWindow->Draw( pDev, aPos, aSize, WINDOW_DRAW_NOCONTROLS );
    }
}

::com::sun::star::awt::DeviceInfo VCLXDialog::getInfo() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::DeviceInfo aInfo = VCLXDevice::getInfo();

    SolarMutexGuard aGuard;
    Dialog* pDlg = (Dialog*) GetWindow();
    if ( pDlg )
        pDlg->GetDrawWindowBorder( aInfo.LeftInset, aInfo.TopInset, aInfo.RightInset, aInfo.BottomInset );

    return aInfo;
}

// ::com::sun::star::document::XVbaMethodParameter
void SAL_CALL VCLXDialog::setVbaMethodParameter(
    const ::rtl::OUString& PropertyName,
    const ::com::sun::star::uno::Any& Value )
throw(::com::sun::star::uno::RuntimeException)
{
    if (rtl::OUString("Cancel") == PropertyName)
    {
        SolarMutexGuard aGuard;
        if ( GetWindow() )
        {
            sal_Int8 nCancel = 0;
            Value >>= nCancel;

            Dialog* pDlg = (Dialog*) GetWindow();
            pDlg->SetCloseFlag(nCancel);
        }
    }
}

::com::sun::star::uno::Any SAL_CALL VCLXDialog::getVbaMethodParameter(
    const ::rtl::OUString& /*PropertyName*/ )
throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aRet;
    return aRet;
}

void SAL_CALL VCLXDialog::setProperty(
    const ::rtl::OUString& PropertyName,
    const ::com::sun::star::uno::Any& Value )
throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Dialog* pDialog = (Dialog*)GetWindow();
    if ( pDialog )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

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
                    aWallpaper.SetStyle( WALLPAPER_SCALE );
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
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}


//  ----------------------------------------------------
//  class VCLXTabPage
//  ----------------------------------------------------
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
void SAL_CALL VCLXMultiPage::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maTabListeners.disposeAndClear( aObj );
    VCLXContainer::dispose();
}
::com::sun::star::uno::Any SAL_CALL VCLXMultiPage::queryInterface(const ::com::sun::star::uno::Type & rType )
throw(::com::sun::star::uno::RuntimeException)
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
throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Window* pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Size aSize = pDev->PixelToLogic( pWindow->GetSizePixel() );
        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );

        pWindow->Draw( pDev, aPos, aSize, WINDOW_DRAW_NOCONTROLS );
    }
}

// ::com::sun::star::awt::XDevice,
::com::sun::star::awt::DeviceInfo SAL_CALL VCLXMultiPage::getInfo()
throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::DeviceInfo aInfo = VCLXDevice::getInfo();
    return aInfo;
}

uno::Any SAL_CALL VCLXMultiPage::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    OSL_TRACE(" **** VCLXMultiPage::getProperty( %s )",
        rtl::OUStringToOString( PropertyName,
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
    const ::rtl::OUString& PropertyName,
    const ::com::sun::star::uno::Any& Value )
throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    OSL_TRACE(" **** VCLXMultiPage::setProperty( %s )", rtl::OUStringToOString( PropertyName, RTL_TEXTENCODING_UTF8 ).getStr() );

    TabControl* pTabControl = (TabControl*)GetWindow();
    if ( pTabControl )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

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
            }
            case BASEPROPERTY_GRAPHIC:
            {
                Reference< XGraphic > xGraphic;
                if (( Value >>= xGraphic ) && xGraphic.is() )
                {
                    Image aImage( xGraphic );

                    Wallpaper aWallpaper( aImage.GetBitmapEx());
                    aWallpaper.SetStyle( WALLPAPER_SCALE );
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
    TabControl *pTabControl = dynamic_cast< TabControl* >( GetWindow() );
    if ( pTabControl )
        return pTabControl;
    throw uno::RuntimeException();
}
sal_Int32 SAL_CALL VCLXMultiPage::insertTab() throw (uno::RuntimeException)
{
    TabControl *pTabControl = getTabControl();
    TabPage* pTab = new TabPage( pTabControl );
    rtl::OUString title ("");
    return static_cast< sal_Int32 >( insertTab( pTab, title ) );
}

sal_uInt16 VCLXMultiPage::insertTab( TabPage* pPage, rtl::OUString& sTitle )
{
    TabControl *pTabControl = getTabControl();
    sal_uInt16 id = sal::static_int_cast< sal_uInt16 >( mTabId++ );
    pTabControl->InsertPage( id, sTitle, TAB_APPEND );
    pTabControl->SetTabPage( id, pPage );
    return id;
}

void SAL_CALL VCLXMultiPage::removeTab( sal_Int32 ID ) throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == NULL )
        throw lang::IndexOutOfBoundsException();
    pTabControl->RemovePage( sal::static_int_cast< sal_uInt16 >( ID ) );
}

void SAL_CALL VCLXMultiPage::activateTab( sal_Int32 ID ) throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    TabControl *pTabControl = getTabControl();
    OSL_TRACE("Attempting to activate tab %d, active tab is %d, numtabs is %d", ID, getActiveTabID(), getWindows().getLength() );
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == NULL )
        throw lang::IndexOutOfBoundsException();
    pTabControl->SelectTabPage( sal::static_int_cast< sal_uInt16 >( ID ) );
}

sal_Int32 SAL_CALL VCLXMultiPage::getActiveTabID() throw (uno::RuntimeException)
{
    return getTabControl()->GetCurPageId( );
}

void SAL_CALL VCLXMultiPage::addTabListener( const uno::Reference< awt::XTabListener >& xListener ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maTabListeners.addInterface( xListener );
}

void SAL_CALL VCLXMultiPage::removeTabListener( const uno::Reference< awt::XTabListener >& xListener ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maTabListeners.addInterface( xListener );
}

void SAL_CALL VCLXMultiPage::setTabProps( sal_Int32 ID, const uno::Sequence< beans::NamedValue >& Properties ) throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == NULL )
        throw lang::IndexOutOfBoundsException();

    for (sal_Int32 i = 0; i < Properties.getLength(); ++i)
    {
        const rtl::OUString &name = Properties[i].Name;
        const uno::Any &value = Properties[i].Value;

        if (name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Title")))
        {
            rtl::OUString title = value.get<rtl::OUString>();
            pTabControl->SetPageText( sal::static_int_cast< sal_uInt16 >( ID ), title );
        }
    }
}

uno::Sequence< beans::NamedValue > SAL_CALL VCLXMultiPage::getTabProps( sal_Int32 ID )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == NULL )
        throw lang::IndexOutOfBoundsException();

#define ADD_PROP( seq, i, name, val ) {                                \
        beans::NamedValue value;                                                  \
        value.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( name ) ); \
        value.Value = uno::makeAny( val );                                      \
        seq[i] = value;                                                    \
    }

    uno::Sequence< beans::NamedValue > props( 2 );
    ADD_PROP( props, 0, "Title", rtl::OUString( pTabControl->GetPageText( sal::static_int_cast< sal_uInt16 >( ID ) ) ) );
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
            sal_uLong nPageID = (sal_uLong)( rVclWindowEvent.GetData() );
            maTabListeners.deactivated( nPageID );
            break;

        }
        case VCLEVENT_TABPAGE_ACTIVATE:
        {
            sal_uLong nPageID = (sal_uLong)( rVclWindowEvent.GetData() );
            maTabListeners.activated( nPageID );
            break;
        }
        default:
            VCLXContainer::ProcessWindowEvent( rVclWindowEvent );
            break;
    };
}

//  ----------------------------------------------------
//  class VCLXTabPage
//  ----------------------------------------------------
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
throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXContainer::queryInterface( rType );
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXTabPage )
    VCLXContainer::getTypes()
IMPL_XTYPEPROVIDER_END

// ::com::sun::star::awt::XView
void SAL_CALL VCLXTabPage::draw( sal_Int32 nX, sal_Int32 nY )
throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Window* pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Size aSize = pDev->PixelToLogic( pWindow->GetSizePixel() );
        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );

        pWindow->Draw( pDev, aPos, aSize, WINDOW_DRAW_NOCONTROLS );
    }
}

// ::com::sun::star::awt::XDevice,
::com::sun::star::awt::DeviceInfo SAL_CALL VCLXTabPage::getInfo()
throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::DeviceInfo aInfo = VCLXDevice::getInfo();
    return aInfo;
}

void SAL_CALL VCLXTabPage::setProperty(
    const ::rtl::OUString& PropertyName,
    const ::com::sun::star::uno::Any& Value )
throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    TabPage* pTabPage = (TabPage*)GetWindow();
    if ( pTabPage )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

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
                    aWallpaper.SetStyle( WALLPAPER_SCALE );
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
                    ::rtl::OUString sTitle;
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
    TabPage *pTabPage = dynamic_cast< TabPage* >( GetWindow() );
    if ( pTabPage )
        return pTabPage;
    throw uno::RuntimeException();
}

//  ----------------------------------------------------
//  class VCLXFixedHyperlink
//  ----------------------------------------------------

VCLXFixedHyperlink::VCLXFixedHyperlink() :

    maActionListeners( *this )

{
}

VCLXFixedHyperlink::~VCLXFixedHyperlink()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXFixedHyperlink::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XFixedHyperlink* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

void VCLXFixedHyperlink::dispose() throw(::com::sun::star::uno::RuntimeException)
{
        SolarMutexGuard aGuard;

        ::com::sun::star::lang::EventObject aObj;
        aObj.Source = (::cppu::OWeakObject*)this;
        maActionListeners.disposeAndClear( aObj );
        VCLXWindow::dispose();
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXFixedHyperlink )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFixedHyperlink>* ) NULL ),
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
                aEvent.Source = (::cppu::OWeakObject*)this;
                maActionListeners.actionPerformed( aEvent );
            }
            else
            {
                // open the URL
                ::rtl::OUString sURL;
                ::toolkit::FixedHyperlinkBase* pBase = (::toolkit::FixedHyperlinkBase*)GetWindow();
                if ( pBase )
                    sURL = pBase->GetURL();
                Reference< ::com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
                    ::comphelper::getProcessServiceFactory()->createInstance(
                        ::rtl::OUString("com.sun.star.system.SystemShellExecute")), uno::UNO_QUERY );
                if ( !sURL.isEmpty() && xSystemShellExecute.is() )
                {
                    try
                    {
                        // start browser
                        xSystemShellExecute->execute(
                            sURL, ::rtl::OUString(), ::com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY );
                    }
                    catch( uno::Exception& )
                    {
                    }
                }
            }
        }

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXFixedHyperlink::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXFixedHyperlink::setText( const ::rtl::OUString& Text ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::toolkit::FixedHyperlinkBase* pBase = (::toolkit::FixedHyperlinkBase*)GetWindow();
    if ( pBase )
        pBase->SetDescription( Text );
}

::rtl::OUString VCLXFixedHyperlink::getText() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aText;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

void VCLXFixedHyperlink::setURL( const ::rtl::OUString& URL ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::toolkit::FixedHyperlinkBase* pBase = (::toolkit::FixedHyperlinkBase*)GetWindow();
    if ( pBase )
        pBase->SetURL( URL );
}

::rtl::OUString VCLXFixedHyperlink::getURL(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aText;
    ::toolkit::FixedHyperlinkBase* pBase = (::toolkit::FixedHyperlinkBase*)GetWindow();
    if ( pBase )
        aText = pBase->GetURL();
    return aText;
}

void VCLXFixedHyperlink::setAlignment( short nAlign ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
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

short VCLXFixedHyperlink::getAlignment() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    short nAlign = 0;
    Window* pWindow = GetWindow();
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

void VCLXFixedHyperlink::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l  )throw(::com::sun::star::uno::RuntimeException)
{
        SolarMutexGuard aGuard;
        maActionListeners.addInterface( l );
}

void VCLXFixedHyperlink::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
        SolarMutexGuard aGuard;
        maActionListeners.removeInterface( l );
}

::com::sun::star::awt::Size VCLXFixedHyperlink::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    FixedText* pFixedText = (FixedText*)GetWindow();
    if ( pFixedText )
        aSz = pFixedText->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXFixedHyperlink::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXFixedHyperlink::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::Size aSz = rNewSize;
    ::com::sun::star::awt::Size aMinSz = getMinimumSize();
    if ( aSz.Height != aMinSz.Height )
        aSz.Height = aMinSz.Height;

    return aSz;
}

void VCLXFixedHyperlink::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::toolkit::FixedHyperlinkBase* pBase = (::toolkit::FixedHyperlinkBase*)GetWindow();
    if ( pBase )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LABEL:
            {
                ::rtl::OUString sNewLabel;
                if ( Value >>= sNewLabel )
                    pBase->SetDescription( sNewLabel );
                break;
            }

            case BASEPROPERTY_URL:
            {
                ::rtl::OUString sNewURL;
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

::com::sun::star::uno::Any VCLXFixedHyperlink::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    ::toolkit::FixedHyperlinkBase* pBase = (::toolkit::FixedHyperlinkBase*)GetWindow();
    if ( pBase )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_URL:
            {
                aProp = makeAny( ::rtl::OUString( pBase->GetURL() ) );
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

//  ----------------------------------------------------
//  class VCLXFixedText
//  ----------------------------------------------------
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
::com::sun::star::uno::Any VCLXFixedText::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XFixedText* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXFixedText )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFixedText>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXFixedText::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXFixedText::setText( const ::rtl::OUString& Text ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( Text );
}

::rtl::OUString VCLXFixedText::getText() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aText;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

void VCLXFixedText::setAlignment( short nAlign ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
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

short VCLXFixedText::getAlignment() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    short nAlign = 0;
    Window* pWindow = GetWindow();
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

::com::sun::star::awt::Size VCLXFixedText::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    FixedText* pFixedText = (FixedText*)GetWindow();
    if ( pFixedText )
        aSz = pFixedText->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXFixedText::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXFixedText::calcAdjustedSize( const ::com::sun::star::awt::Size& rMaxSize ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aAdjustedSize( VCLUnoHelper::ConvertToVCLSize( rMaxSize ) );
    FixedText* pFixedText = (FixedText*)GetWindow();
    if ( pFixedText )
        aAdjustedSize = pFixedText->CalcMinimumSize( rMaxSize.Width );
    return VCLUnoHelper::ConvertToAWTSize( aAdjustedSize );
}

//  ----------------------------------------------------
//  class VCLXScrollBar
//  ----------------------------------------------------
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
::com::sun::star::uno::Any VCLXScrollBar::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XScrollBar* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXScrollBar )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XScrollBar>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXScrollBar::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

// ::com::sun::star::lang::XComponent
void VCLXScrollBar::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maAdjustmentListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

// ::com::sun::star::awt::XScrollbar
void VCLXScrollBar::addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maAdjustmentListeners.addInterface( l );
}

void VCLXScrollBar::removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maAdjustmentListeners.removeInterface( l );
}

void VCLXScrollBar::setValue( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    if ( pScrollBar )
        pScrollBar->DoScroll( n );
}

void VCLXScrollBar::setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    if ( pScrollBar )
    {
        pScrollBar->SetVisibleSize( nVisible );
        pScrollBar->SetRangeMax( nMax );
        pScrollBar->DoScroll( nValue );
    }
}

sal_Int32 VCLXScrollBar::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    return pScrollBar ? pScrollBar->GetThumbPos() : 0;
}

void VCLXScrollBar::setMaximum( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    if ( pScrollBar )
        pScrollBar->SetRangeMax( n );
}

sal_Int32 VCLXScrollBar::getMaximum() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    return pScrollBar ? pScrollBar->GetRangeMax() : 0;
}

void VCLXScrollBar::setMinimum( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = static_cast< ScrollBar* >( GetWindow() );
    if ( pScrollBar )
        pScrollBar->SetRangeMin( n );
}

sal_Int32 VCLXScrollBar::getMinimum() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = static_cast< ScrollBar* >( GetWindow() );
    return pScrollBar ? pScrollBar->GetRangeMin() : 0;
}

void VCLXScrollBar::setLineIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    if ( pScrollBar )
        pScrollBar->SetLineSize( n );
}

sal_Int32 VCLXScrollBar::getLineIncrement() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    return pScrollBar ? pScrollBar->GetLineSize() : 0;
}

void VCLXScrollBar::setBlockIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    if ( pScrollBar )
        pScrollBar->SetPageSize( n );
}

sal_Int32 VCLXScrollBar::getBlockIncrement() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    return pScrollBar ? pScrollBar->GetPageSize() : 0;
}

void VCLXScrollBar::setVisibleSize( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    if ( pScrollBar )
        pScrollBar->SetVisibleSize( n );
}

sal_Int32 VCLXScrollBar::getVisibleSize() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    return pScrollBar ? pScrollBar->GetVisibleSize() : 0;
}

void VCLXScrollBar::setOrientation( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
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

sal_Int32 VCLXScrollBar::getOrientation() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 n = 0;
    Window* pWindow = GetWindow();
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
void VCLXScrollBar::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ScrollBar* pScrollBar = (ScrollBar*)GetWindow();
    if ( pScrollBar )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LIVE_SCROLL:
            {
                sal_Bool bDo = sal_False;
                if ( !bVoid )
                {
                    OSL_VERIFY( Value >>= bDo );
                }
                AllSettings aSettings( pScrollBar->GetSettings() );
                StyleSettings aStyle( aSettings.GetStyleSettings() );
                sal_uLong nDragOptions = aStyle.GetDragFullOptions();
                if ( bDo )
                    nDragOptions |= DRAGFULL_OPTION_SCROLL;
                else
                    nDragOptions &= ~DRAGFULL_OPTION_SCROLL;
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

::com::sun::star::uno::Any VCLXScrollBar::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    ScrollBar* pScrollBar = (ScrollBar*)GetWindow();
    if ( pScrollBar )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );

        switch ( nPropType )
        {
            case BASEPROPERTY_LIVE_SCROLL:
            {
                aProp <<= (sal_Bool)( 0 != ( pScrollBar->GetSettings().GetStyleSettings().GetDragFullOptions() & DRAGFULL_OPTION_SCROLL ) );
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
                ScrollBar* pScrollBar = (ScrollBar*)GetWindow();

                if( pScrollBar )
                {
                    ::com::sun::star::awt::AdjustmentEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
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

::com::sun::star::awt::Size SAL_CALL VCLXScrollBar::implGetMinimumSize( Window* p ) throw(::com::sun::star::uno::RuntimeException)
{
    long n = p->GetSettings().GetStyleSettings().GetScrollBarSize();
    return ::com::sun::star::awt::Size( n, n );
}

::com::sun::star::awt::Size SAL_CALL VCLXScrollBar::getMinimumSize() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return implGetMinimumSize( GetWindow() );
}


//  ----------------------------------------------------
//  class VCLXEdit
//  ----------------------------------------------------

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
::com::sun::star::uno::Any VCLXEdit::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XTextComponent* >(this)),
                                        (static_cast< ::com::sun::star::awt::XTextEditField* >(this)),
                                        (static_cast< ::com::sun::star::awt::XTextLayoutConstrains* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXEdit )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextEditField>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXEdit::CreateAccessibleContext()
{
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXEdit::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maTextListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void VCLXEdit::addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetTextListeners().addInterface( l );
}

void VCLXEdit::removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetTextListeners().removeInterface( l );
}

void VCLXEdit::setText( const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Edit* pEdit = (Edit*)GetWindow();
    if ( pEdit )
    {
        pEdit->SetText( aText );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pEdit->SetModifyFlag();
        pEdit->Modify();
        SetSynthesizingVCLEvent( sal_False );
    }
}

void VCLXEdit::insertText( const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Edit* pEdit = (Edit*)GetWindow();
    if ( pEdit )
    {
        pEdit->SetSelection( Selection( rSel.Min, rSel.Max ) );
        pEdit->ReplaceSelected( aText );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pEdit->SetModifyFlag();
        pEdit->Modify();
        SetSynthesizingVCLEvent( sal_False );
    }
}

::rtl::OUString VCLXEdit::getText() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aText;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

::rtl::OUString VCLXEdit::getSelectedText() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aText;
    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit)
        aText = pEdit->GetSelected();
    return aText;

}

void VCLXEdit::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        pEdit->SetSelection( Selection( aSelection.Min, aSelection.Max ) );
}

::com::sun::star::awt::Selection VCLXEdit::getSelection() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Selection aSel;
    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        aSel = pEdit->GetSelection();
    return ::com::sun::star::awt::Selection( aSel.Min(), aSel.Max() );
}

sal_Bool VCLXEdit::isEditable() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Edit* pEdit = (Edit*) GetWindow();
    return ( pEdit && !pEdit->IsReadOnly() && pEdit->IsEnabled() ) ? sal_True : sal_False;
}

void VCLXEdit::setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        pEdit->SetReadOnly( !bEditable );
}


void VCLXEdit::setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        pEdit->SetMaxTextLen( nLen );
}

sal_Int16 VCLXEdit::getMaxTextLen() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Edit* pEdit = (Edit*) GetWindow();
    return pEdit ? pEdit->GetMaxTextLen() : 0;
}

void VCLXEdit::setEchoChar( sal_Unicode cEcho ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        pEdit->SetEchoChar( cEcho );
}

void VCLXEdit::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Edit* pEdit = (Edit*)GetWindow();
    if ( pEdit )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_HIDEINACTIVESELECTION:
                ::toolkit::adjustBooleanWindowStyle( Value, pEdit, WB_NOHIDESELECTION, sal_True );
                if ( pEdit->GetSubEdit() )
                    ::toolkit::adjustBooleanWindowStyle( Value, pEdit->GetSubEdit(), WB_NOHIDESELECTION, sal_True );
                break;

            case BASEPROPERTY_READONLY:
            {
                sal_Bool b = sal_Bool();
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

::com::sun::star::uno::Any VCLXEdit::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    Edit* pEdit = (Edit*)GetWindow();
    if ( pEdit )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_HIDEINACTIVESELECTION:
                aProp <<= (sal_Bool)( ( pEdit->GetStyle() & WB_NOHIDESELECTION ) == 0 );
                break;
            case BASEPROPERTY_READONLY:
                 aProp <<= (sal_Bool) pEdit->IsReadOnly();
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

::com::sun::star::awt::Size VCLXEdit::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        aSz = pEdit->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXEdit::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
    {
        aSz = pEdit->CalcMinimumSize();
        aSz.Height() += 4;
    }
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXEdit::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::Size aSz = rNewSize;
    ::com::sun::star::awt::Size aMinSz = getMinimumSize();
    if ( aSz.Height != aMinSz.Height )
        aSz.Height = aMinSz.Height;

    return aSz;
}

::com::sun::star::awt::Size VCLXEdit::getMinimumSize( sal_Int16 nCols, sal_Int16 ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
    {
        if ( nCols )
            aSz = pEdit->CalcSize( nCols );
        else
            aSz = pEdit->CalcMinimumSize();
    }
    return AWTSize(aSz);
}

void VCLXEdit::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    nLines = 1;
    nCols = 0;
    Edit* pEdit = (Edit*) GetWindow();
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
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTextListeners().textChanged( aEvent );
            }
        }
        break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

//  ----------------------------------------------------
//  class VCLXComboBox
//  ----------------------------------------------------

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
    // no, don't call VCLXEdit here - it has properties which we do *not* want to have at at combo box
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
#ifndef __SUNPRO_CC
    OSL_TRACE ("%s", __FUNCTION__);
#endif
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXComboBox::CreateAccessibleContext()
{
    SolarMutexGuard aGuard;

    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXComboBox::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aObj );
    maActionListeners.disposeAndClear( aObj );
    VCLXEdit::dispose();
}


void VCLXComboBox::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maItemListeners.addInterface( l );
}

void VCLXComboBox::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maItemListeners.removeInterface( l );
}

void VCLXComboBox::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maActionListeners.addInterface( l );
}

void VCLXComboBox::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    maActionListeners.removeInterface( l );
}

void VCLXComboBox::addItem( const ::rtl::OUString& aItem, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
        pBox->InsertEntry( aItem, nPos );
}

void VCLXComboBox::addItems( const ::com::sun::star::uno::Sequence< ::rtl::OUString>& aItems, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
    {
        sal_uInt16 nP = nPos;
        for ( sal_uInt16 n = 0; n < aItems.getLength(); n++ )
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

void VCLXComboBox::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
    {
        for ( sal_uInt16 n = nCount; n; )
            pBox->RemoveEntry( nPos + (--n) );
    }
}

sal_Int16 VCLXComboBox::getItemCount() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ComboBox* pBox = (ComboBox*) GetWindow();
    return pBox ? pBox->GetEntryCount() : 0;
}

::rtl::OUString VCLXComboBox::getItem( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aItem;
    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
        aItem = pBox->GetEntry( nPos );
    return aItem;
}

::com::sun::star::uno::Sequence< ::rtl::OUString> VCLXComboBox::getItems() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
    {
        sal_uInt16 nEntries = pBox->GetEntryCount();
        aSeq = ::com::sun::star::uno::Sequence< ::rtl::OUString>( nEntries );
        for ( sal_uInt16 n = nEntries; n; )
        {
            --n;
            aSeq.getArray()[n] = pBox->GetEntry( n );
        }
    }
    return aSeq;
}

void VCLXComboBox::setDropDownLineCount( sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
        pBox->SetDropDownLineCount( nLines );
}

sal_Int16 VCLXComboBox::getDropDownLineCount() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int16 nLines = 0;
    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
        nLines = pBox->GetDropDownLineCount();
    return nLines;
}

void VCLXComboBox::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ComboBox* pComboBox = (ComboBox*)GetWindow();
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
            }
            break;
            case BASEPROPERTY_STRINGITEMLIST:
            {
                ::com::sun::star::uno::Sequence< ::rtl::OUString> aItems;
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
                        pComboBox->SetBorderStyle( nBorder );
                }
            }
        }
    }
}

::com::sun::star::uno::Any VCLXComboBox::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    ComboBox* pComboBox = (ComboBox*)GetWindow();
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
                 aProp <<= (sal_Bool) pComboBox->IsAutocompleteEnabled();
            }
            break;
            case BASEPROPERTY_STRINGITEMLIST:
            {
                sal_uInt16 nItems = pComboBox->GetEntryCount();
                ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq( nItems );
                ::rtl::OUString* pStrings = aSeq.getArray();
                for ( sal_uInt16 n = 0; n < nItems; n++ )
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
                ComboBox* pComboBox = (ComboBox*)GetWindow();
                if( pComboBox )
                {
                    if ( !pComboBox->IsTravelSelect() )
                    {
                        ::com::sun::star::awt::ItemEvent aEvent;
                        aEvent.Source = (::cppu::OWeakObject*)this;
                        aEvent.Highlighted = sal_False;

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
                aEvent.Source = (::cppu::OWeakObject*)this;
//              aEvent.ActionCommand = ...;
                maActionListeners.actionPerformed( aEvent );
            }
            break;

        default:
            VCLXEdit::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

::com::sun::star::awt::Size VCLXComboBox::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    ComboBox* pComboBox = (ComboBox*) GetWindow();
    if ( pComboBox )
        aSz = pComboBox->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXComboBox::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    ComboBox* pComboBox = (ComboBox*) GetWindow();
    if ( pComboBox )
    {
        aSz = pComboBox->CalcMinimumSize();
        if ( pComboBox->GetStyle() & WB_DROPDOWN )
            aSz.Height() += 4;
    }
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXComboBox::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz = VCLSize(rNewSize);
    ComboBox* pComboBox = (ComboBox*) GetWindow();
    if ( pComboBox )
        aSz = pComboBox->CalcAdjustedSize( aSz );
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXComboBox::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    ComboBox* pComboBox = (ComboBox*) GetWindow();
    if ( pComboBox )
        aSz = pComboBox->CalcSize( nCols, nLines );
    return AWTSize(aSz);
}

void VCLXComboBox::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    nCols = nLines = 0;
    ComboBox* pComboBox = (ComboBox*) GetWindow();
    if ( pComboBox )
    {
        sal_uInt16 nC, nL;
        pComboBox->GetMaxVisColumnsAndLines( nC, nL );
        nCols = nC;
        nLines = nL;
    }
}
void SAL_CALL VCLXComboBox::listItemInserted( const ItemListEvent& i_rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    ComboBox* pComboBox = dynamic_cast< ComboBox* >( GetWindow() );

    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemInserted: no ComboBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition <= sal_Int32( pComboBox->GetEntryCount() ) ),
        "VCLXComboBox::listItemInserted: illegal (inconsistent) item position!" );
    pComboBox->InsertEntry(
        i_rEvent.ItemText.IsPresent ? i_rEvent.ItemText.Value : ::rtl::OUString(),
        i_rEvent.ItemImageURL.IsPresent ? lcl_getImageFromURL( i_rEvent.ItemImageURL.Value ) : Image(),
        i_rEvent.ItemPosition );
}

void SAL_CALL VCLXComboBox::listItemRemoved( const ItemListEvent& i_rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    ComboBox* pComboBox = dynamic_cast< ComboBox* >( GetWindow() );

    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemRemoved: no ComboBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition < sal_Int32( pComboBox->GetEntryCount() ) ),
        "VCLXComboBox::listItemRemoved: illegal (inconsistent) item position!" );

    pComboBox->RemoveEntry( i_rEvent.ItemPosition );
}

void SAL_CALL VCLXComboBox::listItemModified( const ItemListEvent& i_rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    ComboBox* pComboBox = dynamic_cast< ComboBox* >( GetWindow() );

    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemModified: no ComboBox?!" );
    ENSURE_OR_RETURN_VOID( ( i_rEvent.ItemPosition >= 0 ) && ( i_rEvent.ItemPosition < sal_Int32( pComboBox->GetEntryCount() ) ),
        "VCLXComboBox::listItemModified: illegal (inconsistent) item position!" );

    // VCL's ComboBox does not support changing an entry's text or image, so remove and re-insert

    const ::rtl::OUString sNewText = i_rEvent.ItemText.IsPresent ? i_rEvent.ItemText.Value : ::rtl::OUString( pComboBox->GetEntry( i_rEvent.ItemPosition ) );
    const Image aNewImage( i_rEvent.ItemImageURL.IsPresent ? lcl_getImageFromURL( i_rEvent.ItemImageURL.Value ) : pComboBox->GetEntryImage( i_rEvent.ItemPosition  ) );

    pComboBox->RemoveEntry( i_rEvent.ItemPosition );
    pComboBox->InsertEntry( sNewText, aNewImage, i_rEvent.ItemPosition );
}

void SAL_CALL VCLXComboBox::allItemsRemoved( const EventObject& i_rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    ComboBox* pComboBox = dynamic_cast< ComboBox* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemModified: no ComboBox?!" );

    pComboBox->Clear();

    (void)i_rEvent;
}

void SAL_CALL VCLXComboBox::itemListChanged( const EventObject& i_rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    ComboBox* pComboBox = dynamic_cast< ComboBox* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pComboBox, "VCLXComboBox::listItemModified: no ComboBox?!" );

    pComboBox->Clear();

    uno::Reference< beans::XPropertySet > xPropSet( i_rEvent.Source, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySetInfo > xPSI( xPropSet->getPropertySetInfo(), uno::UNO_QUERY_THROW );
    // bool localize = xPSI->hasPropertyByName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ResourceResolver" ) ) );
    uno::Reference< resource::XStringResourceResolver > xStringResourceResolver;
    if ( xPSI->hasPropertyByName( ::rtl::OUString( "ResourceResolver" ) ) )
    {
        xStringResourceResolver.set(
            xPropSet->getPropertyValue( ::rtl::OUString( "ResourceResolver" ) ),
            uno::UNO_QUERY
        );
    }


    Reference< XItemList > xItemList( i_rEvent.Source, uno::UNO_QUERY_THROW );
    uno::Sequence< beans::Pair< ::rtl::OUString, ::rtl::OUString > > aItems = xItemList->getAllItems();
    for ( sal_Int32 i=0; i<aItems.getLength(); ++i )
    {
        ::rtl::OUString aLocalizationKey( aItems[i].First );
        if ( xStringResourceResolver.is() && !aLocalizationKey.isEmpty() && aLocalizationKey[0] == '&' )
        {
            aLocalizationKey = xStringResourceResolver->resolveString(aLocalizationKey.copy( 1 ));
        }
        pComboBox->InsertEntry( aLocalizationKey, lcl_getImageFromURL( aItems[i].Second ) );
    }
}
void SAL_CALL VCLXComboBox::disposing( const EventObject& i_rEvent ) throw (RuntimeException)
{
    // just disambiguate
    VCLXEdit::disposing( i_rEvent );
}

//  ----------------------------------------------------
//  class VCLXFormattedSpinField
//  ----------------------------------------------------
void VCLXFormattedSpinField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    // Interestingly in the UnoControl API this is
    // - not derived from XEdit ultimately, (correct ?) - so cut this here ...
//    VCLXSpinField::ImplGetPropertyIds( rIds );
    VCLXWindow::ImplGetPropertyIds( rIds );
}

VCLXFormattedSpinField::VCLXFormattedSpinField()
{
}

VCLXFormattedSpinField::~VCLXFormattedSpinField()
{
}

void VCLXFormattedSpinField::setStrictFormat( sal_Bool bStrict )
{
    SolarMutexGuard aGuard;

    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
        pFormatter->SetStrictFormat( bStrict );
}

sal_Bool VCLXFormattedSpinField::isStrictFormat()
{
    FormatterBase* pFormatter = GetFormatter();
    return pFormatter ? pFormatter->IsStrictFormat() : sal_False;
}


void VCLXFormattedSpinField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
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
                sal_Bool b = sal_Bool();
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
                sal_Bool b = sal_Bool();
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

::com::sun::star::uno::Any VCLXFormattedSpinField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
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
                aProp <<= (sal_Bool) ( ( GetWindow()->GetStyle() & WB_SPIN ) ? sal_True : sal_False );
            }
            break;
            case BASEPROPERTY_STRICTFORMAT:
            {
                aProp <<= (sal_Bool) pFormatter->IsStrictFormat();
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


//  ----------------------------------------------------
//  class VCLXDateField
//  ----------------------------------------------------

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

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXDateField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XDateField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXDateField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDateField>* ) NULL ),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXDateField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_DATE:
            {
                if ( bVoid )
                {
                    ((DateField*)GetWindow())->EnableEmptyFieldValue( sal_True );
                    ((DateField*)GetWindow())->SetEmptyFieldValue();
                }
                else
                {
                    sal_Int32 n = 0;
                    if ( Value >>= n )
                         setDate( n );
                }
            }
            break;
            case BASEPROPERTY_DATEMIN:
            {
                sal_Int32 n = 0;
                if ( Value >>= n )
                     setMin( n );
            }
            break;
            case BASEPROPERTY_DATEMAX:
            {
                sal_Int32 n = 0;
                if ( Value >>= n )
                     setMax( n );
            }
            break;
            case BASEPROPERTY_EXTDATEFORMAT:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                    ((DateField*)GetWindow())->SetExtDateFormat( (ExtDateFieldFormat) n );
            }
            break;
            case BASEPROPERTY_DATESHOWCENTURY:
            {
                sal_Bool b = sal_Bool();
                if ( Value >>= b )
                     ((DateField*)GetWindow())->SetShowDateCentury( b );
            }
            break;
            case BASEPROPERTY_ENFORCE_FORMAT:
            {
                sal_Bool bEnforce( sal_True );
                OSL_VERIFY( Value >>= bEnforce );
                static_cast< DateField* >( GetWindow() )->EnforceValidValue( bEnforce );
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXDateField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
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
                aProp <<= (sal_Int32) getDate();
            }
            break;
            case BASEPROPERTY_DATEMIN:
            {
                aProp <<= (sal_Int32) getMin();
            }
            break;
            case BASEPROPERTY_DATEMAX:
            {
                aProp <<= (sal_Int32) getMax();
            }
            break;
            case BASEPROPERTY_DATESHOWCENTURY:
            {
                aProp <<= ((DateField*)GetWindow())->IsShowDateCentury();
            }
            break;
            case BASEPROPERTY_ENFORCE_FORMAT:
            {
                aProp <<= (sal_Bool)static_cast< DateField* >( GetWindow() )->IsEnforceValidValue( );
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


void VCLXDateField::setDate( sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
    {
        pDateField->SetDate( nDate );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pDateField->SetModifyFlag();
        pDateField->Modify();
        SetSynthesizingVCLEvent( sal_False );
    }
}

sal_Int32 VCLXDateField::getDate() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nDate = 0;
    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        nDate = pDateField->GetDate().GetDate();

    return nDate;
}

void VCLXDateField::setMin( sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        pDateField->SetMin( nDate );
}

sal_Int32 VCLXDateField::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nDate = 0;
    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        nDate = pDateField->GetMin().GetDate();

    return nDate;
}

void VCLXDateField::setMax( sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        pDateField->SetMax( nDate );
}

sal_Int32 VCLXDateField::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nDate = 0;
    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        nDate = pDateField->GetMax().GetDate();

    return nDate;
}

void VCLXDateField::setFirst( sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        pDateField->SetFirst( nDate );
}

sal_Int32 VCLXDateField::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nDate = 0;
    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        nDate = pDateField->GetFirst().GetDate();

    return nDate;
}

void VCLXDateField::setLast( sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        pDateField->SetLast( nDate );
}

sal_Int32 VCLXDateField::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nDate = 0;
    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        nDate = pDateField->GetLast().GetDate();

    return nDate;
}

void VCLXDateField::setLongFormat( sal_Bool bLong ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        pDateField->SetLongFormat( bLong );
}

sal_Bool VCLXDateField::isLongFormat() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    DateField* pDateField = (DateField*) GetWindow();
    return pDateField ? pDateField->IsLongFormat() : sal_False;
}

void VCLXDateField::setEmpty() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
    {
        pDateField->SetEmptyDate();

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pDateField->SetModifyFlag();
        pDateField->Modify();
        SetSynthesizingVCLEvent( sal_False );
    }
}

sal_Bool VCLXDateField::isEmpty() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    DateField* pDateField = (DateField*) GetWindow();
    return pDateField ? pDateField->IsEmptyDate() : sal_False;
}

void VCLXDateField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXDateField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXFormattedSpinField::isStrictFormat();
}


//  ----------------------------------------------------
//  class VCLXTimeField
//  ----------------------------------------------------

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

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXTimeField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XTimeField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXTimeField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTimeField>* ) NULL ),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXTimeField::setTime( sal_Int32 nTime ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
    {
        pTimeField->SetTime( nTime );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pTimeField->SetModifyFlag();
        pTimeField->Modify();
        SetSynthesizingVCLEvent( sal_False );
    }
}

sal_Int32 VCLXTimeField::getTime() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nTime = 0;
    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        nTime = pTimeField->GetTime().GetTime();

    return nTime;
}

void VCLXTimeField::setMin( sal_Int32 nTime ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        pTimeField->SetMin( nTime );
}

sal_Int32 VCLXTimeField::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nTime = 0;
    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        nTime = pTimeField->GetMin().GetTime();

    return nTime;
}

void VCLXTimeField::setMax( sal_Int32 nTime ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        pTimeField->SetMax( nTime );
}

sal_Int32 VCLXTimeField::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nTime = 0;
    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        nTime = pTimeField->GetMax().GetTime();

    return nTime;
}

void VCLXTimeField::setFirst( sal_Int32 nTime ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        pTimeField->SetFirst( nTime );
}

sal_Int32 VCLXTimeField::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nTime = 0;
    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        nTime = pTimeField->GetFirst().GetTime();

    return nTime;
}

void VCLXTimeField::setLast( sal_Int32 nTime ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        pTimeField->SetLast( nTime );
}

sal_Int32 VCLXTimeField::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nTime = 0;
    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        nTime = pTimeField->GetLast().GetTime();

    return nTime;
}

void VCLXTimeField::setEmpty() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        pTimeField->SetEmptyTime();
}

sal_Bool VCLXTimeField::isEmpty() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    TimeField* pTimeField = (TimeField*) GetWindow();
    return pTimeField ? pTimeField->IsEmptyTime() : sal_False;
}

void VCLXTimeField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXTimeField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXFormattedSpinField::isStrictFormat();
}


void VCLXTimeField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_TIME:
            {
                if ( bVoid )
                {
                    ((TimeField*)GetWindow())->EnableEmptyFieldValue( sal_True );
                    ((TimeField*)GetWindow())->SetEmptyFieldValue();
                }
                else
                {
                    sal_Int32 n = 0;
                    if ( Value >>= n )
                         setTime( n );
                }
            }
            break;
            case BASEPROPERTY_TIMEMIN:
            {
                sal_Int32 n = 0;
                if ( Value >>= n )
                     setMin( n );
            }
            break;
            case BASEPROPERTY_TIMEMAX:
            {
                sal_Int32 n = 0;
                if ( Value >>= n )
                     setMax( n );
            }
            break;
            case BASEPROPERTY_EXTTIMEFORMAT:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                    ((TimeField*)GetWindow())->SetExtFormat( (ExtTimeFieldFormat) n );
            }
            break;
            case BASEPROPERTY_ENFORCE_FORMAT:
            {
                sal_Bool bEnforce( sal_True );
                OSL_VERIFY( Value >>= bEnforce );
                static_cast< TimeField* >( GetWindow() )->EnforceValidValue( bEnforce );
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXTimeField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
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
                aProp <<= (sal_Int32) getTime();
            }
            break;
            case BASEPROPERTY_TIMEMIN:
            {
                aProp <<= (sal_Int32) getMin();
            }
            break;
            case BASEPROPERTY_TIMEMAX:
            {
                aProp <<= (sal_Int32) getMax();
            }
            break;
            case BASEPROPERTY_ENFORCE_FORMAT:
            {
                aProp <<= (sal_Bool)static_cast< TimeField* >( GetWindow() )->IsEnforceValidValue( );
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

//  ----------------------------------------------------
//  class VCLXNumericField
//  ----------------------------------------------------

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
::com::sun::star::uno::Any VCLXNumericField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XNumericField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXNumericField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XNumericField>* ) NULL ),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXNumericField::setValue( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    if ( pNumericFormatter )
    {
        // shift long value using decimal digits
        // (e.g., input 105 using 2 digits returns 1,05)
        // Thus, to set a value of 1,05, insert 105 and 2 digits
        pNumericFormatter->SetValue(
            (long)ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() ) );

        // #107218# Call same listeners like VCL would do after user interaction
        Edit* pEdit = (Edit*)GetWindow();
        if ( pEdit )
        {
            SetSynthesizingVCLEvent( sal_True );
            pEdit->SetModifyFlag();
            pEdit->Modify();
            SetSynthesizingVCLEvent( sal_False );
        }
    }
}

double VCLXNumericField::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    return pNumericFormatter
        ? ImplCalcDoubleValue( (double)pNumericFormatter->GetValue(), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setMin( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    if ( pNumericFormatter )
        pNumericFormatter->SetMin(
            (long)ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() ) );
}

double VCLXNumericField::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    return pNumericFormatter
        ? ImplCalcDoubleValue( (double)pNumericFormatter->GetMin(), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setMax( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    if ( pNumericFormatter )
        pNumericFormatter->SetMax(
            (long)ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() ) );
}

double VCLXNumericField::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    return pNumericFormatter
        ? ImplCalcDoubleValue( (double)pNumericFormatter->GetMax(), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericField* pNumericField = (NumericField*) GetWindow();
    if ( pNumericField )
        pNumericField->SetFirst(
            (long)ImplCalcLongValue( Value, pNumericField->GetDecimalDigits() ) );
}

double VCLXNumericField::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericField* pNumericField = (NumericField*) GetWindow();
    return pNumericField
        ? ImplCalcDoubleValue( (double)pNumericField->GetFirst(), pNumericField->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setLast( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericField* pNumericField = (NumericField*) GetWindow();
    if ( pNumericField )
        pNumericField->SetLast(
            (long)ImplCalcLongValue( Value, pNumericField->GetDecimalDigits() ) );
}

double VCLXNumericField::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericField* pNumericField = (NumericField*) GetWindow();
    return pNumericField
        ? ImplCalcDoubleValue( (double)pNumericField->GetLast(), pNumericField->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXNumericField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXFormattedSpinField::isStrictFormat();
}


void VCLXNumericField::setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericField* pNumericField = (NumericField*) GetWindow();
    if ( pNumericField )
        pNumericField->SetSpinSize(
            (long)ImplCalcLongValue( Value, pNumericField->GetDecimalDigits() ) );
}

double VCLXNumericField::getSpinSize() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericField* pNumericField = (NumericField*) GetWindow();
    return pNumericField
        ? ImplCalcDoubleValue( (double)pNumericField->GetSpinSize(), pNumericField->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setDecimalDigits( sal_Int16 Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    if ( pNumericFormatter )
    {
        double n = getValue();
        pNumericFormatter->SetDecimalDigits( Value );
        setValue( n );
       }
}

sal_Int16 VCLXNumericField::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    return pNumericFormatter ? pNumericFormatter->GetDecimalDigits() : 0;
}

void VCLXNumericField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                if ( bVoid )
                {
                    ((NumericField*)GetWindow())->EnableEmptyFieldValue( sal_True );
                    ((NumericField*)GetWindow())->SetEmptyFieldValue();
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
                sal_Bool b = sal_Bool();
                if ( Value >>= b )
                     ((NumericField*)GetWindow())->SetUseThousandSep( b );
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXNumericField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
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
                aProp <<= (sal_Bool) ((NumericField*)GetWindow())->IsUseThousandSep();
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
    MetricFormatter *pFormatter = (MetricFormatter *) GetFormatter();
    if (!pFormatter)
        throw ::com::sun::star::uno::RuntimeException();
    return pFormatter;
}

MetricField *VCLXMetricField::GetMetricField() throw(::com::sun::star::uno::RuntimeException)
{
    MetricField *pField = (MetricField *) GetWindow();
    if (!pField)
        throw ::com::sun::star::uno::RuntimeException();
    return pField;
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXMetricField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                                              (static_cast< ::com::sun::star::awt::XMetricField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXMetricField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMetricField>* ) NULL ),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

// FIXME: later ...
#define MetricUnitUnoToVcl(a) ((FieldUnit)(a))

#define METRIC_MAP_PAIR(method,parent) \
    sal_Int64 VCLXMetricField::get##method( sal_Int16 nUnit ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        SolarMutexGuard aGuard; \
        return GetMetric##parent()->Get##method( MetricUnitUnoToVcl( nUnit ) ); \
    } \
    void VCLXMetricField::set##method( sal_Int64 nValue, sal_Int16 nUnit ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        SolarMutexGuard aGuard; \
        GetMetric##parent()->Set##method( nValue, MetricUnitUnoToVcl( nUnit ) ); \
    }

METRIC_MAP_PAIR(Min, Formatter)
METRIC_MAP_PAIR(Max, Formatter)
METRIC_MAP_PAIR(First, Field)
METRIC_MAP_PAIR(Last,  Field)

#undef METRIC_MAP_PAIR

::sal_Int64 VCLXMetricField::getValue( ::sal_Int16 nUnit ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GetMetricFormatter()->GetValue( MetricUnitUnoToVcl( nUnit ) );
}

::sal_Int64 VCLXMetricField::getCorrectedValue( ::sal_Int16 nUnit ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GetMetricFormatter()->GetCorrectedValue( MetricUnitUnoToVcl( nUnit ) );
}

// FIXME: acute cut/paste evilness - move this to the parent Edit class ?
void VCLXMetricField::CallListeners()
{
    // #107218# Call same listeners like VCL would do after user interaction
    Edit* pEdit = (Edit*)GetWindow();
    if ( pEdit )
    {
        SetSynthesizingVCLEvent( sal_True );
        pEdit->SetModifyFlag();
        pEdit->Modify();
        SetSynthesizingVCLEvent( sal_False );
    }
}

void VCLXMetricField::setValue( ::sal_Int64 Value, ::sal_Int16 Unit ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetMetricFormatter()->SetValue( Value, MetricUnitUnoToVcl( Unit ) );
    CallListeners();
}

void VCLXMetricField::setUserValue( ::sal_Int64 Value, ::sal_Int16 Unit ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetMetricFormatter()->SetUserValue( Value, MetricUnitUnoToVcl( Unit ) );
    CallListeners();
}

void VCLXMetricField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXMetricField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXFormattedSpinField::isStrictFormat();
}

void VCLXMetricField::setSpinSize( sal_Int64 Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetMetricField()->SetSpinSize( Value );
}

sal_Int64 VCLXMetricField::getSpinSize() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return GetMetricField()->GetSpinSize();
}

void VCLXMetricField::setDecimalDigits( sal_Int16 Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    GetMetricFormatter()->SetDecimalDigits( Value );
}

sal_Int16 VCLXMetricField::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    return pNumericFormatter ? pNumericFormatter->GetDecimalDigits() : 0;
}

void VCLXMetricField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
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
                sal_Bool b = sal_False;
                if ( Value >>= b )
                     ((NumericField*)GetWindow())->SetUseThousandSep( b );
            }
            break;
            case BASEPROPERTY_UNIT:
            {
                sal_uInt16 nVal = 0;
                if ( Value >>= nVal )
                    ((MetricField*)GetWindow())->SetUnit( (FieldUnit) nVal );
                break;
            }
            case BASEPROPERTY_CUSTOMUNITTEXT:
            {
                rtl::OUString aStr;
                if ( Value >>= aStr )
                    ((MetricField*)GetWindow())->SetCustomUnitText( aStr );
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

::com::sun::star::uno::Any VCLXMetricField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
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
                aProp <<= (sal_Bool) ((NumericField*)GetWindow())->IsUseThousandSep();
                break;
            case BASEPROPERTY_UNIT:
                aProp <<= (sal_uInt16) ((MetricField*)GetWindow())->GetUnit();
                break;
            case BASEPROPERTY_CUSTOMUNITTEXT:
                aProp <<= rtl::OUString (((MetricField*)GetWindow())->GetCustomUnitText());
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


//  ----------------------------------------------------
//  class VCLXCurrencyField
//  ----------------------------------------------------

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
::com::sun::star::uno::Any VCLXCurrencyField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XCurrencyField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXCurrencyField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCurrencyField>* ) NULL ),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXCurrencyField::setValue( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    if ( pCurrencyFormatter )
    {
        // shift long value using decimal digits
        // (e.g., input 105 using 2 digits returns 1,05)
        // Thus, to set a value of 1,05, insert 105 and 2 digits
        pCurrencyFormatter->SetValue(
            ImplCalcLongValue( Value, pCurrencyFormatter->GetDecimalDigits() ) );

        // #107218# Call same listeners like VCL would do after user interaction
        Edit* pEdit = (Edit*)GetWindow();
        if ( pEdit )
        {
            SetSynthesizingVCLEvent( sal_True );
            pEdit->SetModifyFlag();
            pEdit->Modify();
            SetSynthesizingVCLEvent( sal_False );
        }
    }
}

double VCLXCurrencyField::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    return pCurrencyFormatter
        ? ImplCalcDoubleValue( (double)pCurrencyFormatter->GetValue(), pCurrencyFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setMin( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    if ( pCurrencyFormatter )
        pCurrencyFormatter->SetMin(
            ImplCalcLongValue( Value, pCurrencyFormatter->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    return pCurrencyFormatter
        ? ImplCalcDoubleValue( (double)pCurrencyFormatter->GetMin(), pCurrencyFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setMax( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    if ( pCurrencyFormatter )
        pCurrencyFormatter->SetMax(
            ImplCalcLongValue( Value, pCurrencyFormatter->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    return pCurrencyFormatter
        ? ImplCalcDoubleValue( (double)pCurrencyFormatter->GetMax(), pCurrencyFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyField* pCurrencyField = (LongCurrencyField*) GetWindow();
    if ( pCurrencyField )
        pCurrencyField->SetFirst(
            ImplCalcLongValue( Value, pCurrencyField->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyField* pCurrencyField = (LongCurrencyField*) GetWindow();
    return pCurrencyField
        ? ImplCalcDoubleValue( (double)pCurrencyField->GetFirst(), pCurrencyField->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setLast( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyField* pCurrencyField = (LongCurrencyField*) GetWindow();
    if ( pCurrencyField )
        pCurrencyField->SetLast(
            ImplCalcLongValue( Value, pCurrencyField->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyField* pCurrencyField = (LongCurrencyField*) GetWindow();
    return pCurrencyField
        ? ImplCalcDoubleValue( (double)pCurrencyField->GetLast(), pCurrencyField->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyField* pCurrencyField = (LongCurrencyField*) GetWindow();
    if ( pCurrencyField )
        pCurrencyField->SetSpinSize(
            ImplCalcLongValue( Value, pCurrencyField->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getSpinSize() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyField* pCurrencyField = (LongCurrencyField*) GetWindow();
    return pCurrencyField
        ? ImplCalcDoubleValue( (double)pCurrencyField->GetSpinSize(), pCurrencyField->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXCurrencyField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXFormattedSpinField::isStrictFormat();
}


void VCLXCurrencyField::setDecimalDigits( sal_Int16 Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    if ( pCurrencyFormatter )
    {
        double n = getValue();
        pCurrencyFormatter->SetDecimalDigits( Value );
        setValue( n );
       }
}

sal_Int16 VCLXCurrencyField::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    return pCurrencyFormatter ? pCurrencyFormatter->GetDecimalDigits() : 0;
}

void VCLXCurrencyField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                if ( bVoid )
                {
                    ((LongCurrencyField*)GetWindow())->EnableEmptyFieldValue( sal_True );
                    ((LongCurrencyField*)GetWindow())->SetEmptyFieldValue();
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
                ::rtl::OUString aString;
                if ( Value >>= aString )
                     ((LongCurrencyField*)GetWindow())->SetCurrencySymbol( aString );
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                sal_Bool b = sal_Bool();
                if ( Value >>= b )
                     ((LongCurrencyField*)GetWindow())->SetUseThousandSep( b );
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXCurrencyField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
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
                aProp <<= ::rtl::OUString( ((LongCurrencyField*)GetWindow())->GetCurrencySymbol() );
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                aProp <<= (sal_Bool) ((LongCurrencyField*)GetWindow())->IsUseThousandSep();
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

//  ----------------------------------------------------
//  class VCLXPatternField
//  ----------------------------------------------------

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
::com::sun::star::uno::Any VCLXPatternField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XPatternField* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXPatternField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPatternField>* ) NULL ),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXPatternField::setMasks( const ::rtl::OUString& EditMask, const ::rtl::OUString& LiteralMask ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    PatternField* pPatternField = (PatternField*) GetWindow();
    if ( pPatternField )
    {
        pPatternField->SetMask( rtl::OUStringToOString(EditMask, RTL_TEXTENCODING_ASCII_US), LiteralMask );
    }
}

void VCLXPatternField::getMasks( ::rtl::OUString& EditMask, ::rtl::OUString& LiteralMask ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    PatternField* pPatternField = (PatternField*) GetWindow();
    if ( pPatternField )
    {
        EditMask = rtl::OStringToOUString(pPatternField->GetEditMask(), RTL_TEXTENCODING_ASCII_US);
        LiteralMask = pPatternField->GetLiteralMask();
    }
}

void VCLXPatternField::setString( const ::rtl::OUString& Str ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    PatternField* pPatternField = (PatternField*) GetWindow();
    if ( pPatternField )
    {
        pPatternField->SetString( Str );
    }
}

::rtl::OUString VCLXPatternField::getString() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aString;
    PatternField* pPatternField = (PatternField*) GetWindow();
    if ( pPatternField )
        aString = pPatternField->GetString();
    return aString;
}

void VCLXPatternField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXPatternField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXFormattedSpinField::isStrictFormat();
}

void VCLXPatternField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
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
                ::rtl::OUString aString;
                if ( Value >>= aString )
                {
                    ::rtl::OUString aEditMask, aLiteralMask;
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

::com::sun::star::uno::Any VCLXPatternField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
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
                ::rtl::OUString aEditMask, aLiteralMask;
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

//  ----------------------------------------------------
//  class VCLXToolBox
//  ----------------------------------------------------
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

//  ----------------------------------------------------
//  class VCLXFrame
//  ----------------------------------------------------
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
throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXContainer::queryInterface( rType );
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXFrame )
    VCLXContainer::getTypes()
IMPL_XTYPEPROVIDER_END

// ::com::sun::star::awt::XView
void SAL_CALL VCLXFrame::draw( sal_Int32 nX, sal_Int32 nY )
throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Window* pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Size aSize = pDev->PixelToLogic( pWindow->GetSizePixel() );
        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );

        pWindow->Draw( pDev, aPos, aSize, WINDOW_DRAW_NOCONTROLS );
    }
}

// ::com::sun::star::awt::XDevice,
::com::sun::star::awt::DeviceInfo SAL_CALL VCLXFrame::getInfo()
throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::DeviceInfo aInfo = VCLXDevice::getInfo();
    return aInfo;
}

void SAL_CALL VCLXFrame::setProperty(
    const ::rtl::OUString& PropertyName,
    const ::com::sun::star::uno::Any& Value )
throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

#if OSL_DEBUG_LEVEL > 0
    sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;
    (void)bVoid;
#endif

    VCLXContainer::setProperty( PropertyName, Value );
}

void VCLXFrame::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
    VCLXContainer::ProcessWindowEvent( rVclWindowEvent );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
