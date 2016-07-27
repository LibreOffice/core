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

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/frame/status/FontHeight.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <rtl/ref.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <memory>

using namespace ::com::sun::star;

namespace {

class SvxFontSizeBox_Impl;
class FontHeightToolBoxControl : public svt::ToolboxController,
                                 public lang::XServiceInfo
{
    public:
        explicit FontHeightToolBoxControl(
            const css::uno::Reference< css::uno::XComponentContext >& rServiceManager );
        virtual ~FontHeightToolBoxControl();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

        // XComponent
        virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL click() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL doubleClick() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw (css::uno::RuntimeException, std::exception) override;

        void dispatchCommand( const css::uno::Sequence< css::beans::PropertyValue >& rArgs );
        using svt::ToolboxController::dispatchCommand;

    private:
        VclPtr<SvxFontSizeBox_Impl>           m_pBox;
        css::awt::FontDescriptor m_aCurrentFont;
};

class SvxFontSizeBox_Impl : public FontSizeBox
{
public:
                        SvxFontSizeBox_Impl( vcl::Window* pParent,
                                             const uno::Reference< frame::XFrame >& _xFrame,
                                             FontHeightToolBoxControl& rCtrl );

    void                statusChanged_Impl( long nHeight, bool bErase );
    void                UpdateFont( const css::awt::FontDescriptor& rCurrentFont );
    void                SetOptimalSize();

    virtual bool        Notify( NotifyEvent& rNEvt ) override;

protected:
    virtual void        Select() override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

private:
    FontHeightToolBoxControl*                  m_pCtrl;
    OUString                                   m_aCurText;
    Size                                       m_aLogicalSize;
    bool                                       m_bRelease;
    uno::Reference< frame::XFrame >            m_xFrame;

    void                ReleaseFocus_Impl();
};

SvxFontSizeBox_Impl::SvxFontSizeBox_Impl(
    vcl::Window*                                      _pParent,
    const uno::Reference< frame::XFrame >&            _xFrame,
    FontHeightToolBoxControl&                         _rCtrl ) :

    FontSizeBox( _pParent, WinBits( WB_DROPDOWN ) ),

    m_pCtrl             ( &_rCtrl ),
    m_aLogicalSize      ( 0,100 ),
    m_bRelease          ( true ),
    m_xFrame            ( _xFrame )
{
    SetValue( 0 );
    SetText( "" );
}

void SvxFontSizeBox_Impl::ReleaseFocus_Impl()
{
    if ( !m_bRelease )
    {
        m_bRelease = true;
        return;
    }

    if ( m_xFrame.is() && m_xFrame->getContainerWindow().is() )
        m_xFrame->getContainerWindow()->setFocus();
}


void SvxFontSizeBox_Impl::Select()
{
    FontSizeBox::Select();

    if ( !IsTravelSelect() )
    {
        sal_Int64 nSelVal = GetValue();
        float fSelVal     = float( nSelVal ) / 10;

        uno::Sequence< beans::PropertyValue > aArgs( 1 );
        aArgs[0].Name  = "FontHeight.Height";
        aArgs[0].Value = uno::makeAny( fSelVal );

        /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
            This instance may be deleted in the meantime (i.e. when a dialog is opened
            while in Dispatch()), accessing members will crash in this case. */
        ReleaseFocus_Impl();

        m_pCtrl->dispatchCommand( aArgs );
    }
}


void SvxFontSizeBox_Impl::statusChanged_Impl( long nPoint, bool bErase )
{
    if ( !bErase )
    {
        // Metric Umrechnen
        long nVal = nPoint;

        // ge"andert => neuen Wert setzen
        if ( GetValue() != nVal )
            SetValue( nVal );
    }
    else
    {
        // Wert in der Anzeige l"oschen
        SetValue( -1L );
        SetText( "" );
    }
    SaveValue();
}


void SvxFontSizeBox_Impl::UpdateFont( const css::awt::FontDescriptor& rCurrentFont )
{
    // Sizes-Liste auff"ullen
    sal_Int64 nOldVal = GetValue(); // alten Wert merken
    const FontList* _pFontList = nullptr;
    std::unique_ptr<FontList> aHold( new FontList( this ));
    _pFontList = aHold.get();

    if ( !rCurrentFont.Name.isEmpty() )
    {
        FontMetric _aFontMetric;
        _aFontMetric.SetFamilyName( rCurrentFont.Name );
        _aFontMetric.SetStyleName( rCurrentFont.StyleName );
        _aFontMetric.SetFontHeight( rCurrentFont.Height );
        Fill( &_aFontMetric, _pFontList );
    }
    else
    {
        Fill( nullptr, _pFontList );
    }
    SetValue( nOldVal ); // alten Wert wiederherstellen
    m_aCurText = GetText(); // zum R"ucksetzen bei ESC merken
}


bool SvxFontSizeBox_Impl::Notify( NotifyEvent& rNEvt )
{
    bool bHandled = false;

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        sal_uInt16 nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

        switch ( nCode )
        {
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( KEY_TAB == nCode )
                    m_bRelease = false;
                else
                    bHandled = true;
                Select();
                break;
            }

            case KEY_ESCAPE:
                SetText( m_aCurText );
                ReleaseFocus_Impl();
                bHandled = true;
                break;
        }
    }
    else if( MouseNotifyEvent::LOSEFOCUS == rNEvt.GetType() )
    {
        vcl::Window* pFocusWin = Application::GetFocusWindow();
        if(!HasFocus() && GetSubEdit() != pFocusWin)
            SetText(GetSavedValue());
    }

    return bHandled || FontSizeBox::Notify( rNEvt );
}

void SvxFontSizeBox_Impl::SetOptimalSize()
{
    Size aPrefSize(LogicToPixel(m_aLogicalSize, MAP_APPFONT));
    aPrefSize.Width() = get_preferred_size().Width();
    SetSizePixel(aPrefSize);
}

void SvxFontSizeBox_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        SetOptimalSize();
    }

    FontSizeBox::DataChanged( rDCEvt );
}

FontHeightToolBoxControl::FontHeightToolBoxControl( const uno::Reference< uno::XComponentContext >& rxContext )
 : svt::ToolboxController( rxContext,
                           uno::Reference< frame::XFrame >(),
                           OUString( ".uno:FontHeight" ) ),
   m_pBox( nullptr )
{
    addStatusListener( ".uno:CharFontName");
}

FontHeightToolBoxControl::~FontHeightToolBoxControl()
{
}

// XInterface
css::uno::Any SAL_CALL FontHeightToolBoxControl::queryInterface( const css::uno::Type& aType )
throw (css::uno::RuntimeException, std::exception)
{
    uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< lang::XServiceInfo* >( this ));
}

void SAL_CALL FontHeightToolBoxControl::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL FontHeightToolBoxControl::release() throw ()
{
    ToolboxController::release();
}

// XServiceInfo
sal_Bool SAL_CALL FontHeightToolBoxControl::supportsService( const OUString& ServiceName )
throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

OUString SAL_CALL FontHeightToolBoxControl::getImplementationName()
throw( uno::RuntimeException, std::exception )
{
    return OUString("com.sun.star.svx.FontHeightToolBoxController");
}

uno::Sequence< OUString > SAL_CALL FontHeightToolBoxControl::getSupportedServiceNames(  )
throw( uno::RuntimeException, std::exception )
{
    uno::Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}

// XComponent
void SAL_CALL FontHeightToolBoxControl::dispose()
throw (uno::RuntimeException, std::exception)
{
    svt::ToolboxController::dispose();

    SolarMutexGuard aSolarMutexGuard;
    m_pBox.disposeAndClear();
}

// XStatusListener
void SAL_CALL FontHeightToolBoxControl::statusChanged(
    const frame::FeatureStateEvent& rEvent )
throw ( uno::RuntimeException, std::exception )
{
    if ( m_pBox )
    {
        SolarMutexGuard aSolarMutexGuard;
        if ( rEvent.FeatureURL.Path == "FontHeight" )
        {
            if ( rEvent.IsEnabled )
            {
                m_pBox->Enable();
                frame::status::FontHeight aFontHeight;
                if ( rEvent.State >>= aFontHeight )
                    m_pBox->statusChanged_Impl( long( 10. * aFontHeight.Height ), false );
                else
                    m_pBox->statusChanged_Impl( long( -1 ), true );
            }
            else
                m_pBox->Disable();
        }
        else if ( rEvent.FeatureURL.Path == "CharFontName" )
        {
            if ( rEvent.State >>= m_aCurrentFont )
                m_pBox->UpdateFont( m_aCurrentFont );
        }
    }
}

// XToolbarController
void SAL_CALL FontHeightToolBoxControl::execute( sal_Int16 /*KeyModifier*/ )
throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL FontHeightToolBoxControl::click()
throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL FontHeightToolBoxControl::doubleClick()
throw (css::uno::RuntimeException, std::exception)
{
}

uno::Reference< awt::XWindow > SAL_CALL FontHeightToolBoxControl::createPopupWindow()
throw (css::uno::RuntimeException, std::exception)
{
    return uno::Reference< awt::XWindow >();
}

uno::Reference< awt::XWindow > SAL_CALL FontHeightToolBoxControl::createItemWindow(
    const uno::Reference< awt::XWindow >& xParent )
    throw (css::uno::RuntimeException, std::exception)
{
    uno::Reference< awt::XWindow > xItemWindow;

    vcl::Window* pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        SolarMutexGuard aSolarMutexGuard;
        m_pBox = VclPtr<SvxFontSizeBox_Impl>::Create( pParent, m_xFrame, *this );
        //Get the box to fill itself with all its sizes
        m_pBox->UpdateFont(m_aCurrentFont);
        //Make it size itself to its optimal size re above sizes
        m_pBox->SetOptimalSize();
        xItemWindow = VCLUnoHelper::GetInterface( m_pBox );
    }

    return xItemWindow;
}

void FontHeightToolBoxControl::dispatchCommand(
    const uno::Sequence< beans::PropertyValue >& rArgs )
{
    uno::Reference< frame::XDispatchProvider > xDispatchProvider( m_xFrame, uno::UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        util::URL                               aURL;
        uno::Reference< frame::XDispatch >      xDispatch;
        uno::Reference< util::XURLTransformer > xURLTransformer = getURLTransformer();

        aURL.Complete = ".uno:FontHeight";
        xURLTransformer->parseStrict( aURL );
        xDispatch = xDispatchProvider->queryDispatch( aURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aURL, rArgs );
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_svx_FontHeightToolBoxController_get_implementation(
    css::uno::XComponentContext *rxContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FontHeightToolBoxControl(rxContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
