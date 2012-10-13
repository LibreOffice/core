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


#include <list>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/GlobalEventBroadcaster.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/task/XJob.hpp>

#include <comphelper/processfactory.hxx>

#include <osl/mutex.hxx>

#include <vcl/window.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/timer.hxx>
#include <vcl/menu.hxx>
#include <vcl/outdev.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/button.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/sfx.hrc>
#include "rtl/ustrbuf.hxx"

#include "updatecheckui.hrc"

#define UNISTRING(s) rtl::OUString(s)

#define MSG_ERR_NO_WEBBROWSER_FOUND  (RID_SFX_APP_START + 7)

#define PROPERTY_TITLE          RTL_CONSTASCII_STRINGPARAM("BubbleHeading")
#define PROPERTY_TEXT           RTL_CONSTASCII_STRINGPARAM("BubbleText")
#define PROPERTY_IMAGE          RTL_CONSTASCII_STRINGPARAM("BubbleImageURL")
#define PROPERTY_SHOW_BUBBLE    RTL_CONSTASCII_STRINGPARAM("BubbleVisible")
#define PROPERTY_CLICK_HDL      RTL_CONSTASCII_STRINGPARAM("MenuClickHDL")
#define PROPERTY_SHOW_MENUICON  RTL_CONSTASCII_STRINGPARAM("MenuIconVisible")

using namespace ::com::sun::star;

//------------------------------------------------------------------------------

static uno::Sequence< rtl::OUString > getServiceNames()
{
    uno::Sequence< rtl::OUString > aServiceList(1);
    aServiceList[0] = UNISTRING( "com.sun.star.setup.UpdateCheckUI");
    return aServiceList;
}

//------------------------------------------------------------------------------

static rtl::OUString getImplementationName()
{
    return UNISTRING( "vnd.sun.UpdateCheckUI");
}

//------------------------------------------------------------------------------

namespace
{

//------------------------------------------------------------------------------
class BubbleWindow : public FloatingWindow
{
    Point           maTipPos;
    Region          maBounds;
    Polygon         maRectPoly;
    Polygon         maTriPoly;
    XubString       maBubbleTitle;
    XubString       maBubbleText;
    Image           maBubbleImage;
    Size            maMaxTextSize;
    Rectangle       maTitleRect;
    Rectangle       maTextRect;
    long            mnTipOffset;

private:
    void            RecalcTextRects();

public:
                    BubbleWindow( Window* pParent, const XubString& rTitle,
                                  const XubString& rText, const Image& rImage );
                   ~BubbleWindow();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Paint( const Rectangle& rRect );
    void            Resize();
    void            Show( sal_Bool bVisible = sal_True, sal_uInt16 nFlags = SHOW_NOACTIVATE );
    void            SetTipPosPixel( const Point& rTipPos ) { maTipPos = rTipPos; }
    void            SetTitleAndText( const XubString& rTitle, const XubString& rText,
                                     const Image& rImage );
};

//------------------------------------------------------------------------------
class UpdateCheckUI : public ::cppu::WeakImplHelper3
                        < lang::XServiceInfo, document::XEventListener, beans::XPropertySet >
{
    uno::Reference< uno::XComponentContext > m_xContext;
    uno::Reference< task::XJob > mrJob;
    rtl::OUString       maBubbleTitle;
    rtl::OUString       maBubbleText;
    rtl::OUString       maBubbleImageURL;
    Image               maBubbleImage;
    BubbleWindow*       mpBubbleWin;
    SystemWindow*       mpIconSysWin;
    MenuBar*            mpIconMBar;
    ResMgr*             mpUpdResMgr;
    ResMgr*             mpSfxResMgr;
    Timer               maWaitTimer;
    Timer               maTimeoutTimer;
    Link                maWindowEventHdl;
    Link                maApplicationEventHdl;
    bool                mbShowBubble;
    bool                mbShowMenuIcon;
    bool                mbBubbleChanged;
    sal_uInt16              mnIconID;

private:
                    DECL_LINK(ClickHdl, void *);
                    DECL_LINK( HighlightHdl, MenuBar::MenuBarButtonCallbackArg* );
                    DECL_LINK(WaitTimeOutHdl, void *);
                    DECL_LINK(TimeOutHdl, void *);
                    DECL_LINK(UserEventHdl, void *);
                    DECL_LINK( WindowEventHdl, VclWindowEvent* );
                    DECL_LINK( ApplicationEventHdl, VclSimpleEvent* );

    BubbleWindow*   GetBubbleWindow();
    void            RemoveBubbleWindow( bool bRemoveIcon );
    Image           GetMenuBarIcon( MenuBar* pMBar );
    void            AddMenuBarIcon( SystemWindow* pSysWin, bool bAddEventHdl );
    Image           GetBubbleImage( ::rtl::OUString &rURL );

    uno::Reference< document::XEventBroadcaster > getGlobalEventBroadcaster() const
         throw (uno::RuntimeException);

public:
                    UpdateCheckUI(const uno::Reference<uno::XComponentContext>&);
    virtual        ~UpdateCheckUI();

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (uno::RuntimeException);
    virtual uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL notifyEvent(const document::EventObject& Event)
        throw (uno::RuntimeException);
    virtual void SAL_CALL disposing(const lang::EventObject& Event)
        throw (uno::RuntimeException);

    //XPropertySet
    virtual uno::Reference< beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(void)
        throw ( uno::RuntimeException );
    virtual void SAL_CALL setPropertyValue(const rtl::OUString& PropertyName, const uno::Any& aValue)
        throw( beans::UnknownPropertyException, beans::PropertyVetoException,
               lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException );
    virtual uno::Any SAL_CALL getPropertyValue(const rtl::OUString& PropertyName)
        throw ( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException );
    virtual void SAL_CALL addPropertyChangeListener(const rtl::OUString& PropertyName,
                                                    const uno::Reference< beans::XPropertyChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException );
    virtual void SAL_CALL removePropertyChangeListener(const rtl::OUString& PropertyName,
                                                       const uno::Reference< beans::XPropertyChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException );
    virtual void SAL_CALL addVetoableChangeListener(const rtl::OUString& PropertyName,
                                                    const uno::Reference< beans::XVetoableChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException );
    virtual void SAL_CALL removeVetoableChangeListener(const rtl::OUString& PropertyName,
                                                       const uno::Reference< beans::XVetoableChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException );
};

//------------------------------------------------------------------------------
UpdateCheckUI::UpdateCheckUI(const uno::Reference<uno::XComponentContext>& xContext) :
      m_xContext(xContext)
    , mpBubbleWin( NULL )
    , mpIconSysWin( NULL )
    , mpIconMBar( NULL )
    , mbShowBubble( false )
    , mbShowMenuIcon( false )
    , mbBubbleChanged( false )
    , mnIconID( 0 )
{
    mpUpdResMgr = ResMgr::CreateResMgr( "updchk" );
    mpSfxResMgr = ResMgr::CreateResMgr( "sfx" );

    maBubbleImage = GetBubbleImage( maBubbleImageURL );

    maWaitTimer.SetTimeout( 400 );
    maWaitTimer.SetTimeoutHdl( LINK( this, UpdateCheckUI, WaitTimeOutHdl ) );

    maTimeoutTimer.SetTimeout( 10000 );
    maTimeoutTimer.SetTimeoutHdl( LINK( this, UpdateCheckUI, TimeOutHdl ) );

    uno::Reference< document::XEventBroadcaster > xBroadcaster( getGlobalEventBroadcaster() );
    xBroadcaster->addEventListener( this );

    maWindowEventHdl = LINK( this, UpdateCheckUI, WindowEventHdl );
    maApplicationEventHdl = LINK( this, UpdateCheckUI, ApplicationEventHdl );
    Application::AddEventListener( maApplicationEventHdl );
}

//------------------------------------------------------------------------------
UpdateCheckUI::~UpdateCheckUI()
{
    Application::RemoveEventListener( maApplicationEventHdl );
    RemoveBubbleWindow( true );
    delete mpUpdResMgr;
    delete mpSfxResMgr;
}

//------------------------------------------------------------------------------
uno::Reference<document::XEventBroadcaster>
UpdateCheckUI::getGlobalEventBroadcaster() const throw (uno::RuntimeException)
{
    if( !m_xContext.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckUI: empty component context" ),
                uno::Reference< uno::XInterface >() );

    return uno::Reference<document::XEventBroadcaster> (
        frame::GlobalEventBroadcaster::create(m_xContext),
        uno::UNO_QUERY_THROW);
}

//------------------------------------------------------------------------------
rtl::OUString SAL_CALL
UpdateCheckUI::getImplementationName() throw (uno::RuntimeException)
{
    return ::getImplementationName();
}

//------------------------------------------------------------------------------
uno::Sequence< rtl::OUString > SAL_CALL
UpdateCheckUI::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return ::getServiceNames();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL
UpdateCheckUI::supportsService( rtl::OUString const & serviceName ) throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > aServiceNameList = ::getServiceNames();

    for( sal_Int32 n=0; n < aServiceNameList.getLength(); n++ )
        if( aServiceNameList[n].equals(serviceName) )
            return sal_True;

    return sal_False;
}

//------------------------------------------------------------------------------
Image UpdateCheckUI::GetMenuBarIcon( MenuBar* pMBar )
{
    sal_uInt32 nResID;
    Window *pMBarWin = pMBar->GetWindow();
    sal_uInt32 nMBarHeight = 20;

    if ( pMBarWin )
        nMBarHeight = pMBarWin->GetOutputSizePixel().getHeight();

    if ( nMBarHeight >= 35 )
        nResID = RID_UPDATE_AVAILABLE_26;
    else
        nResID = RID_UPDATE_AVAILABLE_16;

    return Image( ResId( nResID, *mpUpdResMgr ) );
}

//------------------------------------------------------------------------------
Image UpdateCheckUI::GetBubbleImage( ::rtl::OUString &rURL )
{
    Image aImage;

    if ( !maBubbleImageURL.isEmpty() )
    {
        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

        if( !xContext.is() )
            throw uno::RuntimeException(
                UNISTRING( "UpdateCheckUI: unable to obtain service manager from component context" ),
                    uno::Reference< uno::XInterface >() );

        try
        {
            uno::Reference< graphic::XGraphicProvider > xGraphProvider(graphic::GraphicProvider::create(xContext));
            uno::Sequence< beans::PropertyValue > aMediaProps( 1 );
            aMediaProps[0].Name = ::rtl::OUString("URL");
            aMediaProps[0].Value <<= rURL;

            uno::Reference< graphic::XGraphic > xGraphic = xGraphProvider->queryGraphic( aMediaProps );
            if ( xGraphic.is() )
            {
                aImage = Image( xGraphic );
            }
        }
        catch( const uno::Exception& )
        {
        }
    }

    if ( aImage.GetSizePixel().Width() == 0 )
        aImage = InfoBox::GetStandardImage();

    return aImage;
}

//------------------------------------------------------------------------------
void UpdateCheckUI::AddMenuBarIcon( SystemWindow *pSysWin, bool bAddEventHdl )
{
    if ( ! mbShowMenuIcon )
        return;

    SolarMutexGuard aGuard;

    MenuBar *pActiveMBar = pSysWin->GetMenuBar();
    if ( ( pSysWin != mpIconSysWin ) || ( pActiveMBar != mpIconMBar ) )
    {
        if ( bAddEventHdl && mpIconSysWin )
            mpIconSysWin->RemoveEventListener( maWindowEventHdl );

        RemoveBubbleWindow( true );

        if ( pActiveMBar )
        {
            rtl::OUStringBuffer aBuf;
            if( !maBubbleTitle.isEmpty() )
                aBuf.append( maBubbleTitle );
            if( !maBubbleText.isEmpty() )
            {
                if( !maBubbleTitle.isEmpty() )
                    aBuf.appendAscii( "\n\n" );
                aBuf.append( maBubbleText );
            }

            Image aImage = GetMenuBarIcon( pActiveMBar );
            mnIconID = pActiveMBar->AddMenuBarButton( aImage,
                                    LINK( this, UpdateCheckUI, ClickHdl ),
                                    aBuf.makeStringAndClear()
                                    );
            pActiveMBar->SetMenuBarButtonHighlightHdl( mnIconID,
                                    LINK( this, UpdateCheckUI, HighlightHdl ) );
        }
        mpIconMBar = pActiveMBar;
        mpIconSysWin = pSysWin;
        if ( bAddEventHdl && mpIconSysWin )
            mpIconSysWin->AddEventListener( maWindowEventHdl );
    }

    if ( mbShowBubble && pActiveMBar )
    {
        mpBubbleWin = GetBubbleWindow();
        if ( mpBubbleWin )
        {
            mpBubbleWin->Show( sal_True );
            maTimeoutTimer.Start();
        }
        mbShowBubble = false;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL UpdateCheckUI::notifyEvent(const document::EventObject& rEvent)
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( rEvent.EventName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("OnPrepareViewClosing") ) == 0 )
    {
        RemoveBubbleWindow( true );
    }
}

//------------------------------------------------------------------------------
void SAL_CALL UpdateCheckUI::disposing(const lang::EventObject&)
    throw (uno::RuntimeException)
{
}

//------------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > UpdateCheckUI::getPropertySetInfo(void)
    throw ( uno::RuntimeException )
{
    return NULL;
}

//------------------------------------------------------------------------------
void UpdateCheckUI::setPropertyValue(const rtl::OUString& rPropertyName,
                                     const uno::Any& rValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException,
           lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    rtl::OUString aString;

    if( rPropertyName.compareToAscii( PROPERTY_TITLE ) == 0 ) {
        rValue >>= aString;
        if ( aString != maBubbleTitle ) {
            maBubbleTitle = aString;
            mbBubbleChanged = true;
        }
    }
    else if( rPropertyName.compareToAscii( PROPERTY_TEXT ) == 0 ) {
        rValue >>= aString;
        if ( aString != maBubbleText ) {
            maBubbleText = aString;
            mbBubbleChanged = true;
        }
    }
    else if( rPropertyName.compareToAscii( PROPERTY_IMAGE ) == 0 ) {
        rValue >>= aString;
        if ( aString != maBubbleImageURL ) {
            maBubbleImageURL = aString;
            maBubbleImage = GetBubbleImage( maBubbleImageURL );
            mbBubbleChanged = true;
        }
    }
    else if( rPropertyName.compareToAscii( PROPERTY_SHOW_BUBBLE ) == 0 ) {
        rValue >>= mbShowBubble;
        if ( mbShowBubble )
            Application::PostUserEvent( LINK( this, UpdateCheckUI, UserEventHdl ) );
        else if ( mpBubbleWin )
            mpBubbleWin->Show( sal_False );
    }
    else if( rPropertyName.compareToAscii( PROPERTY_CLICK_HDL ) == 0 ) {
        uno::Reference< task::XJob > aJob;
        rValue >>= aJob;
        if ( aJob.is() )
            mrJob = aJob;
        else
            throw lang::IllegalArgumentException();
    }
    else if (rPropertyName.compareToAscii( PROPERTY_SHOW_MENUICON ) == 0) {
        bool bShowMenuIcon = sal_False;
        rValue >>= bShowMenuIcon;
        if ( bShowMenuIcon != mbShowMenuIcon )
        {
            mbShowMenuIcon = bShowMenuIcon;
            if ( bShowMenuIcon )
                Application::PostUserEvent( LINK( this, UpdateCheckUI, UserEventHdl ) );
            else
                RemoveBubbleWindow( true );
        }
    }
    else
        throw beans::UnknownPropertyException();

    if ( mbBubbleChanged && mpBubbleWin )
        mpBubbleWin->Show( sal_False );
}

//------------------------------------------------------------------------------
uno::Any UpdateCheckUI::getPropertyValue(const rtl::OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    uno::Any aRet;

    if( rPropertyName.compareToAscii( PROPERTY_TITLE ) == 0 )
        aRet = uno::makeAny( maBubbleTitle );
    else if( rPropertyName.compareToAscii( PROPERTY_TEXT ) == 0 )
        aRet = uno::makeAny( maBubbleText );
    else if( rPropertyName.compareToAscii( PROPERTY_SHOW_BUBBLE ) == 0 )
        aRet = uno::makeAny( mbShowBubble );
    else if( rPropertyName.compareToAscii( PROPERTY_IMAGE ) == 0 )
        aRet = uno::makeAny( maBubbleImageURL );
    else if( rPropertyName.compareToAscii( PROPERTY_CLICK_HDL ) == 0 )
        aRet = uno::makeAny( mrJob );
    else if( rPropertyName.compareToAscii( PROPERTY_SHOW_MENUICON ) == 0 )
        aRet = uno::makeAny( mbShowMenuIcon );
    else
        throw beans::UnknownPropertyException();

    return aRet;
}

//------------------------------------------------------------------------------
void UpdateCheckUI::addPropertyChangeListener( const rtl::OUString& /*aPropertyName*/,
                                               const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    //no bound properties
}

//------------------------------------------------------------------------------
void UpdateCheckUI::removePropertyChangeListener( const rtl::OUString& /*aPropertyName*/,
                                                  const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    //no bound properties
}

//------------------------------------------------------------------------------
void UpdateCheckUI::addVetoableChangeListener( const rtl::OUString& /*aPropertyName*/,
                                               const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    //no vetoable properties
}

//------------------------------------------------------------------------------
void UpdateCheckUI::removeVetoableChangeListener( const rtl::OUString& /*aPropertyName*/,
                                                  const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    //no vetoable properties
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BubbleWindow * UpdateCheckUI::GetBubbleWindow()
{
    if ( !mpIconSysWin )
        return NULL;

    Rectangle aIconRect = mpIconMBar->GetMenuBarButtonRectPixel( mnIconID );
    if( aIconRect.IsEmpty() )
        return NULL;

    BubbleWindow* pBubbleWin = mpBubbleWin;

    if ( !pBubbleWin ) {
        pBubbleWin = new BubbleWindow( mpIconSysWin,
                                         XubString( maBubbleTitle ),
                                       XubString( maBubbleText ),
                                       maBubbleImage );
        mbBubbleChanged = false;
    }
    else if ( mbBubbleChanged ) {
        pBubbleWin->SetTitleAndText( XubString( maBubbleTitle ),
                                     XubString( maBubbleText ),
                                     maBubbleImage );
        mbBubbleChanged = false;
    }

    Point aWinPos = aIconRect.BottomCenter();

    pBubbleWin->SetTipPosPixel( aWinPos );

    return pBubbleWin;
}

//------------------------------------------------------------------------------
void UpdateCheckUI::RemoveBubbleWindow( bool bRemoveIcon )
{
    SolarMutexGuard aGuard;

    maWaitTimer.Stop();
    maTimeoutTimer.Stop();

    if ( mpBubbleWin )
    {
        delete mpBubbleWin;
        mpBubbleWin = NULL;
    }

    if ( bRemoveIcon )
    {
        try {
            if ( mpIconMBar && ( mnIconID != 0 ) )
            {
                mpIconMBar->RemoveMenuBarButton( mnIconID );
                mpIconMBar = NULL;
                mnIconID = 0;
            }
        }
        catch ( ... ) {
            mpIconMBar = NULL;
            mnIconID = 0;
        }

        mpIconSysWin = NULL;
    }
}

// -----------------------------------------------------------------------
IMPL_LINK_NOARG(UpdateCheckUI, ClickHdl)
{
    SolarMutexGuard aGuard;

    maWaitTimer.Stop();
    if ( mpBubbleWin )
        mpBubbleWin->Show( sal_False );

    if ( mrJob.is() )
    {
        try {
            uno::Sequence<beans::NamedValue> aEmpty;
            mrJob->execute( aEmpty );
        }
        catch(const uno::Exception&) {
            ErrorBox( NULL, ResId( MSG_ERR_NO_WEBBROWSER_FOUND, *mpSfxResMgr )).Execute();
        }
    }

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK( UpdateCheckUI, HighlightHdl, MenuBar::MenuBarButtonCallbackArg*, pData )
{
    if ( pData->bHighlight )
        maWaitTimer.Start();
    else
        RemoveBubbleWindow( false );

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK_NOARG(UpdateCheckUI, WaitTimeOutHdl)
{
    SolarMutexGuard aGuard;

    mpBubbleWin = GetBubbleWindow();

    if ( mpBubbleWin )
    {
        mpBubbleWin->Show();
    }

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK_NOARG(UpdateCheckUI, TimeOutHdl)
{
    RemoveBubbleWindow( false );

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK_NOARG(UpdateCheckUI, UserEventHdl)
{
    SolarMutexGuard aGuard;

    Window *pTopWin = Application::GetFirstTopLevelWindow();
    Window *pActiveWin = Application::GetActiveTopWindow();
    SystemWindow *pActiveSysWin = NULL;

    Window *pBubbleWin = NULL;
    if ( mpBubbleWin )
        pBubbleWin = mpBubbleWin;

    if ( pActiveWin && ( pActiveWin != pBubbleWin ) && pActiveWin->IsTopWindow() )
        pActiveSysWin = pActiveWin->GetSystemWindow();

    if ( pActiveWin == pBubbleWin )
        pActiveSysWin = NULL;

    while ( !pActiveSysWin && pTopWin )
    {
        if ( ( pTopWin != pBubbleWin ) && pTopWin->IsTopWindow() )
            pActiveSysWin = pTopWin->GetSystemWindow();
        if ( !pActiveSysWin )
            pTopWin = Application::GetNextTopLevelWindow( pTopWin );
    }

    if ( pActiveSysWin )
        AddMenuBarIcon( pActiveSysWin, true );

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK( UpdateCheckUI, WindowEventHdl, VclWindowEvent*, pEvent )
{
    sal_uLong nEventID = pEvent->GetId();

    if ( VCLEVENT_OBJECT_DYING == nEventID )
    {
        SolarMutexGuard aGuard;
        if ( mpIconSysWin == pEvent->GetWindow() )
        {
            mpIconSysWin->RemoveEventListener( maWindowEventHdl );
            RemoveBubbleWindow( true );
        }
    }
    else if ( VCLEVENT_WINDOW_MENUBARADDED == nEventID )
    {
        SolarMutexGuard aGuard;
        Window *pWindow = pEvent->GetWindow();
        if ( pWindow )
        {
            SystemWindow *pSysWin = pWindow->GetSystemWindow();
            if ( pSysWin )
            {
                AddMenuBarIcon( pSysWin, false );
            }
        }
    }
    else if ( VCLEVENT_WINDOW_MENUBARREMOVED == nEventID )
    {
        SolarMutexGuard aGuard;
        MenuBar *pMBar = (MenuBar*) pEvent->GetData();
        if ( pMBar && ( pMBar == mpIconMBar ) )
            RemoveBubbleWindow( true );
    }
    else if ( ( nEventID == VCLEVENT_WINDOW_MOVE ) ||
              ( nEventID == VCLEVENT_WINDOW_RESIZE ) )
    {
        SolarMutexGuard aGuard;
        if ( ( mpIconSysWin == pEvent->GetWindow() ) &&
             ( mpBubbleWin != NULL ) && ( mpIconMBar != NULL ) )
        {
            Rectangle aIconRect = mpIconMBar->GetMenuBarButtonRectPixel( mnIconID );
            Point aWinPos = aIconRect.BottomCenter();
            mpBubbleWin->SetTipPosPixel( aWinPos );
            if ( mpBubbleWin->IsVisible() )
                mpBubbleWin->Show();    // This will recalc the screen positon of the bubble
        }
    }

    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK( UpdateCheckUI, ApplicationEventHdl, VclSimpleEvent *, pEvent)
{
    switch (pEvent->GetId())
    {
        case VCLEVENT_WINDOW_SHOW:
        case VCLEVENT_WINDOW_ACTIVATE:
        case VCLEVENT_WINDOW_GETFOCUS: {
            SolarMutexGuard aGuard;

            Window *pWindow = static_cast< VclWindowEvent * >(pEvent)->GetWindow();
            if ( pWindow && pWindow->IsTopWindow() )
            {
                SystemWindow *pSysWin = pWindow->GetSystemWindow();
                MenuBar      *pMBar   = pSysWin->GetMenuBar();
                if ( pSysWin && pMBar )
                {
                    AddMenuBarIcon( pSysWin, true );
                }
            }
            break;
        }
    }
    return 0;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define TIP_HEIGHT             15
#define TIP_WIDTH               7
#define TIP_RIGHT_OFFSET       18
#define BUBBLE_BORDER          10
#define TEXT_MAX_WIDTH        300
#define TEXT_MAX_HEIGHT       200

//------------------------------------------------------------------------------
BubbleWindow::BubbleWindow( Window* pParent, const XubString& rTitle,
                            const XubString& rText, const Image& rImage )
    : FloatingWindow( pParent, WB_SYSTEMWINDOW
                               | WB_OWNERDRAWDECORATION
                               | WB_NOBORDER
                    )
    , maBubbleTitle( rTitle )
    , maBubbleText( rText )
    , maBubbleImage( rImage )
    , maMaxTextSize( TEXT_MAX_WIDTH, TEXT_MAX_HEIGHT )
    , mnTipOffset( 0 )
{
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetHelpColor() ) );
}

//------------------------------------------------------------------------------
BubbleWindow::~BubbleWindow()
{
}

//------------------------------------------------------------------------------
void BubbleWindow::Resize()
{
    SolarMutexGuard aGuard;

    FloatingWindow::Resize();

    Size aSize = GetSizePixel();

    if ( ( aSize.Height() < 20 ) || ( aSize.Width() < 60 ) )
        return;

    Rectangle aRect( 0, TIP_HEIGHT, aSize.Width(), aSize.Height() - TIP_HEIGHT );
    maRectPoly = Polygon( aRect, 6, 6 );
    Region aRegion( maRectPoly );
    long nTipOffset = aSize.Width() - TIP_RIGHT_OFFSET + mnTipOffset;

    Point aPointArr[4];
    aPointArr[0] = Point( nTipOffset, TIP_HEIGHT );
    aPointArr[1] = Point( nTipOffset, 0 );
    aPointArr[2] = Point( nTipOffset + TIP_WIDTH , TIP_HEIGHT );
    aPointArr[3] = Point( nTipOffset, TIP_HEIGHT );
     maTriPoly = Polygon( 4, aPointArr );
    Region aTriRegion( maTriPoly );

    aRegion.Union( aTriRegion);
    maBounds = aRegion;

    SetWindowRegionPixel( maBounds );
}

//------------------------------------------------------------------------------
void BubbleWindow::SetTitleAndText( const XubString& rTitle,
                                    const XubString& rText,
                                    const Image& rImage )
{
    maBubbleTitle = rTitle;
    maBubbleText = rText;
    maBubbleImage = rImage;

    Resize();
}

//------------------------------------------------------------------------------
void BubbleWindow::Paint( const Rectangle& )
{
    SolarMutexGuard aGuard;

    LineInfo aThickLine( LINE_SOLID, 2 );

    DrawPolyLine( maRectPoly, aThickLine );
    DrawPolyLine( maTriPoly );

    Color aOldLine = GetLineColor();
    Size aSize = GetSizePixel();
    long nTipOffset = aSize.Width() - TIP_RIGHT_OFFSET + mnTipOffset;

    SetLineColor( GetSettings().GetStyleSettings().GetHelpColor() );
    DrawLine( Point( nTipOffset+2, TIP_HEIGHT ),
              Point( nTipOffset + TIP_WIDTH -1 , TIP_HEIGHT ),
              aThickLine );
    SetLineColor( aOldLine );

    //Image aImage = InfoBox::GetStandardImage();
    Size aImgSize = maBubbleImage.GetSizePixel();

    DrawImage( Point( BUBBLE_BORDER, BUBBLE_BORDER + TIP_HEIGHT ), maBubbleImage );

    Font aOldFont = GetFont();
    Font aBoldFont = aOldFont;
    aBoldFont.SetWeight( WEIGHT_BOLD );

    SetFont( aBoldFont );
    Rectangle aTitleRect = maTitleRect;
    aTitleRect.Move( aImgSize.Width(), 0 );
    DrawText( aTitleRect, maBubbleTitle, TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );

    SetFont( aOldFont );
    Rectangle aTextRect = maTextRect;
    aTextRect.Move( aImgSize.Width(), 0 );
    DrawText( aTextRect, maBubbleText, TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
}

//------------------------------------------------------------------------------
void BubbleWindow::MouseButtonDown( const MouseEvent& )
{
    Show( sal_False );
}

//------------------------------------------------------------------------------
void BubbleWindow::Show( sal_Bool bVisible, sal_uInt16 nFlags )
{
    SolarMutexGuard aGuard;

    if ( !bVisible )
    {
        FloatingWindow::Show( bVisible );
        return;
    }

    // don't show bubbles without a text
    if ( ( maBubbleTitle.Len() == 0 ) && ( maBubbleText.Len() == 0 ) )
        return;

    Size aWindowSize = GetSizePixel();

    // Image aImage = InfoBox::GetStandardImage();
    Size aImgSize = maBubbleImage.GetSizePixel();

    RecalcTextRects();

    aWindowSize.setHeight( maTitleRect.GetHeight() * 7 / 4+ maTextRect.GetHeight() +
                           3 * BUBBLE_BORDER + TIP_HEIGHT );

    if ( maTitleRect.GetWidth() > maTextRect.GetWidth() )
        aWindowSize.setWidth( maTitleRect.GetWidth() );
    else
        aWindowSize.setWidth( maTextRect.GetWidth() );

    aWindowSize.setWidth( aWindowSize.Width() + 3 * BUBBLE_BORDER + aImgSize.Width() );

    if ( aWindowSize.Height() < aImgSize.Height() + TIP_HEIGHT + 2 * BUBBLE_BORDER )
        aWindowSize.setHeight( aImgSize.Height() + TIP_HEIGHT + 2 * BUBBLE_BORDER );

    Point aPos;
    aPos.X() = maTipPos.X() - aWindowSize.Width() + TIP_RIGHT_OFFSET;
    aPos.Y() = maTipPos.Y();
    Point aScreenPos = GetParent()->OutputToAbsoluteScreenPixel( aPos );
    if ( aScreenPos.X() < 0 )
    {
        mnTipOffset = aScreenPos.X();
        aPos.X() -= mnTipOffset;
    }
    SetPosSizePixel( aPos, aWindowSize );

    FloatingWindow::Show( bVisible, nFlags );
}

//------------------------------------------------------------------------------
void BubbleWindow::RecalcTextRects()
{
    Size aTotalSize;
    sal_Bool bFinished = sal_False;
    Font aOldFont = GetFont();
    Font aBoldFont = aOldFont;

    aBoldFont.SetWeight( WEIGHT_BOLD );

    while ( !bFinished )
    {
        SetFont( aBoldFont );

        maTitleRect = GetTextRect( Rectangle( Point( 0, 0 ), maMaxTextSize ),
                                   maBubbleTitle,
                                   TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );

        SetFont( aOldFont );
        maTextRect = GetTextRect( Rectangle( Point( 0, 0 ), maMaxTextSize ),
                                  maBubbleText,
                                  TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );

        if ( maTextRect.GetHeight() < 10 )
            maTextRect.setHeight( 10 );

        aTotalSize.setHeight( maTitleRect.GetHeight() +
                              aBoldFont.GetHeight() * 3 / 4 +
                              maTextRect.GetHeight() +
                              3 * BUBBLE_BORDER + TIP_HEIGHT );
        if ( aTotalSize.Height() > maMaxTextSize.Height() )
        {
            maMaxTextSize.Width() = maMaxTextSize.Width() * 3 / 2;
            maMaxTextSize.Height() = maMaxTextSize.Height() * 3 / 2;
        }
        else
            bFinished = sal_True;
    }
    maTitleRect.Move( 2*BUBBLE_BORDER, BUBBLE_BORDER + TIP_HEIGHT );
    maTextRect.Move( 2*BUBBLE_BORDER, BUBBLE_BORDER + TIP_HEIGHT + maTitleRect.GetHeight() + aBoldFont.GetHeight() * 3 / 4 );
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

} // anonymous namespace

//------------------------------------------------------------------------------

static uno::Reference<uno::XInterface> SAL_CALL
createInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    return  *new UpdateCheckUI(xContext);
}

//------------------------------------------------------------------------------

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createInstance,
        getImplementationName,
        getServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL, NULL, NULL, NULL, NULL, 0 }
} ;

//------------------------------------------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL updchkui_component_getFactory(const sal_Char *pszImplementationName, void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pszImplementationName,
        pServiceManager,
        pRegistryKey,
        kImplementations_entries) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
