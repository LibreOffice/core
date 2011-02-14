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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "tbunocontroller.hxx"

#include <com/sun/star/frame/status/FontHeight.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/ctrlbox.hxx>
#include <vos/mutex.hxx>

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
    String                                     m_aCurText;
    Size                                       m_aLogicalSize;
    sal_Bool                                       m_bRelease;
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
    m_bRelease          ( sal_True ),
    m_xDispatchProvider ( _rDispatchProvider ),
    m_xFrame            ( _xFrame )
{
    SetSizePixel( LogicToPixel( m_aLogicalSize, MAP_APPFONT ));
    SetValue( 0 );
    SetText( String() );
}

// -----------------------------------------------------------------------

void SvxFontSizeBox_Impl::ReleaseFocus_Impl()
{
    if ( !m_bRelease )
    {
        m_bRelease = sal_True;
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
        aArgs[0].Name  = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontHeight.Height" ));
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
        SetText( String() );
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

    if ( rCurrentFont.Name.getLength() > 0 )
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
                    m_bRelease = sal_False;
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

FontHeightToolBoxControl::FontHeightToolBoxControl(
    const uno::Reference< lang::XMultiServiceFactory >& rServiceManager ) :
    svt::ToolboxController( rServiceManager,
                            uno::Reference< frame::XFrame >(),
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontHeight" )) ),
    m_pBox( NULL )
{
    addStatusListener( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CharFontName" )));
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
sal_Bool SAL_CALL FontHeightToolBoxControl::supportsService( const ::rtl::OUString& ServiceName )
throw(uno::RuntimeException)
{
    const uno::Sequence< ::rtl::OUString > aSNL( getSupportedServiceNames() );
    const ::rtl::OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return true;

    return false;
}

::rtl::OUString SAL_CALL FontHeightToolBoxControl::getImplementationName()
throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

uno::Sequence< ::rtl::OUString > SAL_CALL FontHeightToolBoxControl::getSupportedServiceNames(  )
throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< ::rtl::OUString > FontHeightToolBoxControl::getSupportedServiceNames_Static()
throw()
{
    uno::Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ));
    return aSNS;
}

// XComponent
void SAL_CALL FontHeightToolBoxControl::dispose()
throw (uno::RuntimeException)
{
    svt::ToolboxController::dispose();

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
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
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
        if ( rEvent.FeatureURL.Path.equalsAscii( "FontHeight" ))
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
        else if ( rEvent.FeatureURL.Path.equalsAscii( "CharFontName" ))
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
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
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

        aURL.Complete = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontHeight" ));
        xURLTransformer->parseStrict( aURL );
        xDispatch = xDispatchProvider->queryDispatch( aURL, ::rtl::OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aURL, rArgs );
    }
}

uno::Reference< uno::XInterface > SAL_CALL FontHeightToolBoxControl_createInstance(
    const uno::Reference< lang::XMultiServiceFactory >& rSMgr )
{
    return *new FontHeightToolBoxControl( rSMgr );
}

}
