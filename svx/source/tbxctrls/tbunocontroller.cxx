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


#include "tbunocontroller.hxx"

#include <com/sun/star/frame/status/FontHeight.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/ctrlbox.hxx>
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>

#include <memory>

#define LOGICAL_EDIT_HEIGHT         12

using namespace ::com::sun::star;

namespace svx
{

class FontHeightToolBoxControl;
class SvxFontSizeBox_Impl : public FontSizeBox
{
public:
                        SvxFontSizeBox_Impl( Window* pParent,
                                             const uno::Reference< frame::XDispatchProvider >& rDispatchProvider,
                                             const uno::Reference< frame::XFrame >& _xFrame,
                                             FontHeightToolBoxControl& rCtrl );

    void                statusChanged_Impl( long nHeight, bool bErase = false );
    void                UpdateFont( const ::com::sun::star::awt::FontDescriptor& rCurrentFont );

    virtual long        Notify( NotifyEvent& rNEvt );

protected:
    virtual void        Select();
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

private:
    FontHeightToolBoxControl*                  m_pCtrl;
    OUString                                   m_aCurText;
    Size                                       m_aLogicalSize;
    bool                                       m_bRelease;
    uno::Reference< frame::XDispatchProvider > m_xDispatchProvider;
    uno::Reference< frame::XFrame >            m_xFrame;
    uno::Reference< awt::XWindow >             m_xOldFocusWindow;

    void                ReleaseFocus_Impl();
};

//========================================================================
// class SvxFontSizeBox_Impl --------------------------------------------------
//========================================================================

SvxFontSizeBox_Impl::SvxFontSizeBox_Impl(
    Window*                                           _pParent,
    const uno::Reference< frame::XDispatchProvider >& _rDispatchProvider,
    const uno::Reference< frame::XFrame >&            _xFrame,
    FontHeightToolBoxControl&                         _rCtrl ) :

    FontSizeBox( _pParent, WinBits( WB_DROPDOWN ) ),

    m_pCtrl             ( &_rCtrl ),
    m_aLogicalSize      ( 30,100 ),
    m_bRelease          ( true ),
    m_xDispatchProvider ( _rDispatchProvider ),
    m_xFrame            ( _xFrame )
{
    SetSizePixel( LogicToPixel( m_aLogicalSize, MAP_APPFONT ));
    SetValue( 0 );
    SetText( "" );
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void SvxFontSizeBox_Impl::Select()
{
    FontSizeBox::Select();

    if ( !IsTravelSelect() )
    {
        sal_Int64 nSelVal = GetValue();
        float fSelVal     = float( nSelVal ) / 10;

        uno::Sequence< beans::PropertyValue > aArgs( 1 );
        aArgs[0].Name  = OUString( "FontHeight.Height" );
        aArgs[0].Value = uno::makeAny( fSelVal );

        /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
            This instance may be deleted in the meantime (i.e. when a dialog is opened
            while in Dispatch()), accessing members will crash in this case. */
        ReleaseFocus_Impl();

        m_pCtrl->dispatchCommand( aArgs );
    }
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void SvxFontSizeBox_Impl::UpdateFont( const ::com::sun::star::awt::FontDescriptor& rCurrentFont )
{
    // Sizes-Liste auff"ullen
    sal_Int64 nOldVal = GetValue(); // alten Wert merken
    const FontList* _pFontList = NULL;
    ::std::auto_ptr<FontList> aHold( new FontList( this ));
    _pFontList = aHold.get();

    if ( !rCurrentFont.Name.isEmpty() )
    {
        FontInfo _aFontInfo;
        _aFontInfo.SetName( rCurrentFont.Name );
        _aFontInfo.SetStyleName( rCurrentFont.StyleName );
        _aFontInfo.SetHeight( rCurrentFont.Height );
        Fill( &_aFontInfo, _pFontList );
    }
    else
    {
        Fill( NULL, _pFontList );
    }
    SetValue( nOldVal ); // alten Wert wiederherstellen
    m_aCurText = GetText(); // zum R"ucksetzen bei ESC merken
}

// -----------------------------------------------------------------------

long SvxFontSizeBox_Impl::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
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
                    nHandled = 1;
                Select();
                break;
            }

            case KEY_ESCAPE:
                SetText( m_aCurText );
                ReleaseFocus_Impl();
                nHandled = 1;
                break;
        }
    }
    else if( EVENT_LOSEFOCUS == rNEvt.GetType() )
    {
        Window* pFocusWin = Application::GetFocusWindow();
        if(!HasFocus() && GetSubEdit() != pFocusWin)
            SetText(GetSavedValue());
    }

    return nHandled ? nHandled : FontSizeBox::Notify( rNEvt );
}

// ---------------------------------------------------------------------------

void SvxFontSizeBox_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetSizePixel( LogicToPixel( m_aLogicalSize, MAP_APPFONT ));
        Size aDropSize( m_aLogicalSize.Width(), LOGICAL_EDIT_HEIGHT );
        SetDropDownSizePixel( LogicToPixel( aDropSize, MAP_APPFONT ));
    }

    FontSizeBox::DataChanged( rDCEvt );
}

//========================================================================
// class FontHeightToolBoxControl
//========================================================================

FontHeightToolBoxControl::FontHeightToolBoxControl( const uno::Reference< uno::XComponentContext >& rxContext )
 : svt::ToolboxController( rxContext,
                           uno::Reference< frame::XFrame >(),
                           OUString( ".uno:FontHeight" ) ),
   m_pBox( NULL )
{
    addStatusListener( OUString( ".uno:CharFontName" ));
}

FontHeightToolBoxControl::~FontHeightToolBoxControl()
{
}

// XInterface
::com::sun::star::uno::Any SAL_CALL FontHeightToolBoxControl::queryInterface( const ::com::sun::star::uno::Type& aType )
throw (::com::sun::star::uno::RuntimeException)
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
throw(uno::RuntimeException)
{
    const uno::Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return true;

    return false;
}

OUString SAL_CALL FontHeightToolBoxControl::getImplementationName()
throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

uno::Sequence< OUString > SAL_CALL FontHeightToolBoxControl::getSupportedServiceNames(  )
throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > FontHeightToolBoxControl::getSupportedServiceNames_Static()
throw()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( "com.sun.star.frame.ToolbarController" );
    return aSNS;
}

// XComponent
void SAL_CALL FontHeightToolBoxControl::dispose()
throw (uno::RuntimeException)
{
    svt::ToolboxController::dispose();

    SolarMutexGuard aSolarMutexGuard;
    delete m_pBox;
    m_pBox = 0;
}

// XStatusListener
void SAL_CALL FontHeightToolBoxControl::statusChanged(
    const frame::FeatureStateEvent& rEvent )
throw ( uno::RuntimeException )
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
throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL FontHeightToolBoxControl::click()
throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL FontHeightToolBoxControl::doubleClick()
throw (::com::sun::star::uno::RuntimeException)
{
}

uno::Reference< awt::XWindow > SAL_CALL FontHeightToolBoxControl::createPopupWindow()
throw (::com::sun::star::uno::RuntimeException)
{
    return uno::Reference< awt::XWindow >();
}

uno::Reference< awt::XWindow > SAL_CALL FontHeightToolBoxControl::createItemWindow(
    const uno::Reference< awt::XWindow >& Parent )
    throw (::com::sun::star::uno::RuntimeException)
{
    uno::Reference< awt::XWindow > xItemWindow;
    uno::Reference< awt::XWindow > xParent( Parent );

    Window* pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        SolarMutexGuard aSolarMutexGuard;
        m_pBox = new SvxFontSizeBox_Impl(
                        pParent,
                        uno::Reference< frame::XDispatchProvider >( m_xFrame, uno::UNO_QUERY ),
                        m_xFrame,
                        *this );
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

        aURL.Complete = OUString( ".uno:FontHeight" );
        xURLTransformer->parseStrict( aURL );
        xDispatch = xDispatchProvider->queryDispatch( aURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aURL, rArgs );
    }
}

uno::Reference< uno::XInterface > SAL_CALL FontHeightToolBoxControl_createInstance(
    const uno::Reference< lang::XMultiServiceFactory >& rSMgr )
{
    return *new FontHeightToolBoxControl( comphelper::getComponentContext(rSMgr) );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
