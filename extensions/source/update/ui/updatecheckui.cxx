/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updatecheckui.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-09-07 14:01:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <list>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implementationentry.hxx>

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XJOB_HPP_
#include <com/sun/star/task/XJob.hpp>
#endif

#include <vos/mutex.hxx>

#include <vcl/window.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/timer.hxx>
#include <vcl/menu.hxx>
#include <vcl/outdev.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/imagebtn.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/sfx.hrc>

#include "updatecheckui.hrc"

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

#define MSG_ERR_NO_WEBBROWSER_FOUND  (RID_SFX_APP_START + 7)
#define DEFAULT_MENUBAR_HEIGHT 24

namespace css = com::sun::star ;
namespace beans = css::beans ;
namespace lang = css::lang ;
namespace task = css::task ;
namespace uno = css::uno ;
namespace document = css::document ;

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
ResId UpdResId( sal_uInt32 nID )
{
    static ResMgr* pResMgr = ResMgr::CreateResMgr( "updchk" MAKE_NUMSTR(SUPD) );
    return ResId( nID, pResMgr );
}

//------------------------------------------------------------------------------
ResId SfxResId( sal_uInt32 nID )
{
    static ResMgr* pResMgr = ResMgr::CreateResMgr( "sfx" MAKE_NUMSTR(SUPD) );
    return ResId( nID, pResMgr );
}


//------------------------------------------------------------------------------
class BubbleWindow : public FloatingWindow
{
    Point           maTipPos;
    Region          maBounds;
    Polygon         maRectPoly;
    Polygon         maTriPoly;
    XubString       maBubbleTitle;
    XubString       maBubbleText;

public:
                    BubbleWindow( Window* pParent, const XubString& rTitle,
                                  const XubString& rText );
                   ~BubbleWindow();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Paint( const Rectangle& rRect );
    void            Resize();
    void            Show( BOOL bVisible = TRUE, USHORT nFlags = 0 );
    void            SetTipPosPixel( const Point& rTipPos ) { maTipPos = rTipPos; }
};

struct IconListData {
    SystemWindow   *pWindow;
    //BubbleWindow   *pBubbleWin;
    MenuBar        *pMBar;
    USHORT          nID;
};

class UpdateCheckUI : public ::cppu::WeakImplHelper3< lang::XServiceInfo, lang::XInitialization, document::XEventListener >
//class UpdateCheckUI : public ::cppu::WeakImplHelper2< lang::XServiceInfo, lang::XInitialization >
{
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference< task::XJob > mrJob;
    std::list <IconListData> maWinList;
    rtl::OUString       maBubbleTitle;
    rtl::OUString       maBubbleText;
    BubbleWindow       *mpBubbleWin;
    MenuBar            *mpCurrMBar;
    Timer               maWaitTimer;
    Timer               maTimeoutTimer;
    AutoTimer           maRetryTimer;
    sal_Bool            mbShowBubble;
    Link                maWindowEventHdl;

private:
                    DECL_LINK( ClickHdl, USHORT* );
                    DECL_LINK( HighlightHdl, MenuBar::MenuBarButtonCallbackArg* );
                    DECL_LINK( WaitTimeOutHdl, Timer* );
                    DECL_LINK( TimeOutHdl, Timer* );
                    DECL_LINK( RetryHdl, Timer* );
                    DECL_LINK( WindowEventHdl, VclWindowEvent* );

    BubbleWindow*   GetBubbleWindow( MenuBar *pMBar );
    void            RemoveBubbleWindow();
    Image           GetMenuBarIcon( MenuBar* pMBar );
    void            AddMenuBarIcons();

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

    // XInitialization
    virtual void SAL_CALL initialize(const uno::Sequence< uno::Any >&)
        throw (uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL notifyEvent(const document::EventObject& Event)
        throw (uno::RuntimeException);
    virtual void SAL_CALL disposing(const lang::EventObject& Event)
        throw (uno::RuntimeException);
};

//------------------------------------------------------------------------------
UpdateCheckUI::UpdateCheckUI(const uno::Reference<uno::XComponentContext>& xContext) :
    m_xContext(xContext)
{
    mpBubbleWin = NULL;
    mpCurrMBar = NULL;
    maWaitTimer.SetTimeout( 400 );
    maWaitTimer.SetTimeoutHdl( LINK( this, UpdateCheckUI, WaitTimeOutHdl ) );
    maTimeoutTimer.SetTimeout( 10000 );
    maTimeoutTimer.SetTimeoutHdl( LINK( this, UpdateCheckUI, TimeOutHdl ) );
    maRetryTimer.SetTimeout( 5000 );
    maRetryTimer.SetTimeoutHdl( LINK( this, UpdateCheckUI, RetryHdl ) );
    maWindowEventHdl = LINK( this, UpdateCheckUI, WindowEventHdl );

    OSL_TRACE( "in ctor of UpdateCheckUI" );
}

//------------------------------------------------------------------------------
UpdateCheckUI::~UpdateCheckUI()
{
    OSL_TRACE( "in dtor of UpdateCheckUI" );
    maWaitTimer.Stop();
    maRetryTimer.Stop();
    maTimeoutTimer.Stop();
    delete mpBubbleWin;
}

//------------------------------------------------------------------------------
uno::Reference<document::XEventBroadcaster>
UpdateCheckUI::getGlobalEventBroadcaster() const throw (uno::RuntimeException)
{
    uno::Reference<uno::XComponentContext> xContext(m_xContext);

    if( !xContext.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckUI: empty component context" ),
                uno::Reference< uno::XInterface >() );

    uno::Reference< lang::XMultiComponentFactory > xServiceManager(xContext->getServiceManager());

    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckUI: unable to obtain service manager from component context" ),
                uno::Reference< uno::XInterface >() );

    return uno::Reference<document::XEventBroadcaster> (
        xServiceManager->createInstanceWithContext(
            UNISTRING( "com.sun.star.frame.GlobalEventBroadcaster" ),
            xContext),
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
void SAL_CALL
UpdateCheckUI::initialize(const uno::Sequence< uno::Any >& rArgumentList) throw (uno::RuntimeException)
{
    sal_Int32 nParamCount = rArgumentList.getLength();

    OSL_TRACE( "UpdateCheckUI::initialize()" );

    if ( nParamCount != 4 ) return;

    sal_Bool bWrongParameter = sal_False;

    if ( ! (rArgumentList[0] >>= mrJob) ) bWrongParameter = sal_True;
    if ( ! (rArgumentList[1] >>= maBubbleTitle) ) bWrongParameter = sal_True;
    if ( ! (rArgumentList[2] >>= maBubbleText) ) bWrongParameter = sal_True;
    if ( ! (rArgumentList[3] >>= mbShowBubble) ) bWrongParameter = sal_True;

    if ( bWrongParameter ) return;

    uno::Reference< document::XEventBroadcaster > xBroadcaster( getGlobalEventBroadcaster() );
    xBroadcaster->addEventListener( this );

    if ( ! mbShowBubble )
        AddMenuBarIcons();

    maRetryTimer.Start();
}

//------------------------------------------------------------------------------
Image UpdateCheckUI::GetMenuBarIcon( MenuBar* pMBar )
{
    sal_uInt32 nResID;
    Window *pMBarWin = pMBar->GetWindow();
    sal_uInt32 nMBarHeight = 20;

    if ( pMBarWin )
        nMBarHeight = pMBarWin->GetOutputSizePixel().getHeight();

    if ( Application::GetSettings().GetStyleSettings().GetHighContrastMode() ) {
        if ( nMBarHeight >= 35 )
            nResID = RID_UPDATE_AVAILABLE_26_HC;
        else
            nResID = RID_UPDATE_AVAILABLE_16_HC;
    } else {
        if ( nMBarHeight >= 35 )
            nResID = RID_UPDATE_AVAILABLE_26;
        else
            nResID = RID_UPDATE_AVAILABLE_16;
    }

    return Image( UpdResId( nResID ) );
}
//------------------------------------------------------------------------------
void UpdateCheckUI::AddMenuBarIcons()
{
    // Get MenuBar Image
    vos::OGuard aGuard( Application::GetSolarMutex() );

    Window *pTopWin = Application::GetFirstTopLevelWindow();
    Window *pActiveWin = Application::GetActiveTopWindow();
    SystemWindow *pActiveSysWin = NULL;

    if ( pActiveWin )
        pActiveSysWin = pActiveWin->GetSystemWindow();

    while ( pTopWin || pActiveWin )
    {
        SystemWindow *pSysWin = NULL;
        MenuBar *pWinMenu = NULL;
        BOOL bInList = FALSE;

        if ( pActiveWin )
            pSysWin = pActiveWin->GetSystemWindow();
        else
            pSysWin = pTopWin->GetSystemWindow();

        if ( pSysWin )
        {
            for( std::list <IconListData>::iterator it = maWinList.begin();
                 it != maWinList.end(); ++it )
            {
                if ( it->pWindow == pSysWin )
                {
                    MenuBar *pTmpMenu = pSysWin->GetMenuBar();
                    if ( pTmpMenu == it->pMBar )
                        bInList = TRUE;
                    else
                        // We found the window, but the MenuBar has changed so
                        // remove entry from list
                        maWinList.erase( it );
                    break;
                }
            }
            if ( ! bInList )
                pWinMenu = pSysWin->GetMenuBar();
        }
        if ( pWinMenu )
        {
            Image aImage = GetMenuBarIcon( pWinMenu );
            IconListData *pData = new IconListData;
            pData->nID = pWinMenu->AddMenuBarButton( aImage,
                                    LINK( this, UpdateCheckUI, ClickHdl ) );
            pData->pWindow = pSysWin;
            pData->pMBar = pWinMenu;
            //pData->pBubbleWin = NULL;
            maWinList.push_back( *pData );
            pWinMenu->SetMenuBarButtonHighlightHdl( pData->nID,
                                    LINK( this, UpdateCheckUI, HighlightHdl ) );

            if ( ( pSysWin == pActiveSysWin ) && mbShowBubble )
            {
                mpBubbleWin = GetBubbleWindow( pWinMenu );
                mpBubbleWin->Show();
                mbShowBubble = FALSE;
                maTimeoutTimer.Start();
            }
        }
        if ( pActiveWin )
            pActiveWin = NULL;
        else
            pTopWin = Application::GetNextTopLevelWindow( pTopWin );
    }
}

//------------------------------------------------------------------------------
void SAL_CALL UpdateCheckUI::notifyEvent(const document::EventObject& rEvent)
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    OSL_TRACE( "notifyEvent: %s", rtl::OUStringToOString(rEvent.EventName, RTL_TEXTENCODING_UTF8).getStr() );
    if( ( rEvent.EventName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("OnPrepareViewClosing") ) == 0 ) ||
        ( rEvent.EventName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("OnCloseApp") ) == 0 ) )
    {
        RemoveBubbleWindow();
    }
}

//------------------------------------------------------------------------------
void SAL_CALL UpdateCheckUI::disposing(const lang::EventObject&)
    throw (uno::RuntimeException)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BubbleWindow * UpdateCheckUI::GetBubbleWindow( MenuBar *pMBar )
{
    for( std::list <IconListData>::iterator it = maWinList.begin();
         it != maWinList.end(); ++it )
    {
        if ( it->pMBar == pMBar )
        {
            BubbleWindow *pBubble;

            pBubble = new BubbleWindow( it->pWindow,
                                 XubString( maBubbleTitle ),
                                 XubString( maBubbleText ) );

            it->pWindow->AddEventListener( maWindowEventHdl );
            Rectangle aIconRect = pMBar->GetMenuBarButtonRectPixel( it->nID );
            Point aWinPos = aIconRect.BottomCenter();
            pBubble->SetTipPosPixel( aWinPos );
            return pBubble;
        }
    }
    return NULL;
}

//------------------------------------------------------------------------------
void UpdateCheckUI::RemoveBubbleWindow()
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    maWaitTimer.Stop();
    maTimeoutTimer.Stop();

    if ( mpBubbleWin )
    {
        Window *pParent = mpBubbleWin->GetParent();

        if ( pParent )
            pParent->RemoveEventListener( maWindowEventHdl );

        delete mpBubbleWin;
        mpBubbleWin = NULL;

        OSL_TRACE( "UpdateCheckUI::RemoveBubbleWindow" );
    }
    /*for( std::list <IconListData>::iterator it = maWinList.begin();
         it != maWinList.end(); ++it )
    {
        if ( it->pBubbleWin )
        {
            delete it->pBubbleWin;
            it->pBubbleWin = NULL;
        }
    }*/
}

// -----------------------------------------------------------------------
IMPL_LINK( UpdateCheckUI, ClickHdl, USHORT*, EMPTYARG )
{
    RemoveBubbleWindow();

    if ( mrJob.is() )
    {
        try {
            uno::Sequence<beans::NamedValue> aEmpty;
            mrJob->execute( aEmpty );
        }
        catch(const uno::Exception&) {
            ErrorBox( NULL, SfxResId( MSG_ERR_NO_WEBBROWSER_FOUND )).Execute();
        }
    }

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK( UpdateCheckUI, HighlightHdl, MenuBar::MenuBarButtonCallbackArg*, pData )
{
    if ( pData->bHighlight )
    {
        // Set Tip Pos
        maWaitTimer.Start();
        mpCurrMBar = pData->pMenuBar;
    }
    else
    {
        RemoveBubbleWindow();
    }

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK( UpdateCheckUI, WaitTimeOutHdl, Timer*, EMPTYARG )
{
    if ( mpCurrMBar )
        mpBubbleWin = GetBubbleWindow( mpCurrMBar );
    else
        mpBubbleWin = NULL;

    if ( mpBubbleWin )
        mpBubbleWin->Show();

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK( UpdateCheckUI, TimeOutHdl, Timer*, EMPTYARG )
{
    RemoveBubbleWindow();

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK( UpdateCheckUI, RetryHdl, Timer*, EMPTYARG )
{
    AddMenuBarIcons();
    maRetryTimer.SetTimeout( 15000 );

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK( UpdateCheckUI, WindowEventHdl, VclWindowEvent*, pEvent )
{
    if ( ( VCLEVENT_OBJECT_DYING == pEvent->GetId() ) && mpBubbleWin )
    {
        OSL_TRACE( "UpdateCheckUI::WindowEventHdl" );
        Window * pParent = mpBubbleWin->GetParent();
        if ( pParent == pEvent->GetWindow() )
            RemoveBubbleWindow();
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
#define TEXT_MAX_HEIGHT       300
#define INITIAL_SHOW_TIME   10000

//------------------------------------------------------------------------------
BubbleWindow::BubbleWindow( Window* pParent, const XubString& rTitle,
                            const XubString& rText )
    : FloatingWindow( pParent, 0 )
{
    maBubbleTitle = rTitle;
    maBubbleText = rText;

    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetHelpColor() ) );
}

//------------------------------------------------------------------------------
BubbleWindow::~BubbleWindow()
{
}

//------------------------------------------------------------------------------
void BubbleWindow::Resize()
{
    FloatingWindow::Resize();

    Size aSize = GetSizePixel();

    if ( ( aSize.Height() < 20 ) || ( aSize.Width() < 60 ) )
        return;

    Rectangle aRect( 0, TIP_HEIGHT, aSize.Width(), aSize.Height() - TIP_HEIGHT );
    maRectPoly = Polygon( aRect, 6, 6 );
    Region aRegion( maRectPoly );
    long nTipOffset = aSize.Width() - TIP_RIGHT_OFFSET;

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
void BubbleWindow::Paint( const Rectangle& rRect )
{
    (void) rRect;

    LineInfo aThickLine( LINE_SOLID, 2 );

    DrawPolyLine( maRectPoly, aThickLine );
    DrawPolyLine( maTriPoly );

    Color aOldLine = GetLineColor();
    Size aSize = GetSizePixel();
    long nTipOffset = aSize.Width() - TIP_RIGHT_OFFSET;

    SetLineColor( GetSettings().GetStyleSettings().GetHelpColor() );
    DrawLine( Point( nTipOffset+2, TIP_HEIGHT ),
              Point( nTipOffset + TIP_WIDTH -1 , TIP_HEIGHT ),
              aThickLine );
    SetLineColor( aOldLine );

    Image aImage = InfoBox::GetStandardImage();
    Size aImgSize = aImage.GetSizePixel();

    DrawImage( Point( BUBBLE_BORDER, BUBBLE_BORDER + TIP_HEIGHT ), aImage );

    Font aOldFont = GetFont();
    Font aBoldFont = aOldFont;
    aBoldFont.SetWeight( WEIGHT_BOLD );
    SetFont( aBoldFont );

    Rectangle aTitleRect = GetTextRect( Rectangle( 0, 0, TEXT_MAX_WIDTH, TEXT_MAX_HEIGHT ),
                                        maBubbleTitle,
                                        TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    aTitleRect.Move( 2*BUBBLE_BORDER + aImgSize.Width(), BUBBLE_BORDER + TIP_HEIGHT );

    DrawText( aTitleRect, maBubbleTitle, TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );

    SetFont( aOldFont );
    Rectangle aTextRect = GetTextRect( Rectangle( 0, 0, TEXT_MAX_WIDTH, TEXT_MAX_HEIGHT ),
                                       maBubbleText,
                                       TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    aTextRect.Move( 2*BUBBLE_BORDER + aImgSize.Width(), BUBBLE_BORDER + TIP_HEIGHT );
    aTextRect.Move( 0, aBoldFont.GetHeight() * 7 / 4 );

    DrawText( aTextRect, maBubbleText, TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
}

//------------------------------------------------------------------------------
void BubbleWindow::MouseButtonDown( const MouseEvent& )
{
    Show( FALSE );
}

//------------------------------------------------------------------------------
void BubbleWindow::Show( BOOL bVisible, USHORT nFlags )
{

    if ( !bVisible )
    {
        FloatingWindow::Show( bVisible, nFlags );
        return;
    }

    Size aWindowSize = GetSizePixel();

    Image aImage = InfoBox::GetStandardImage();
    Size aImgSize = aImage.GetSizePixel();

    Font aOldFont = GetFont();
    Font aBoldFont = aOldFont;
    aBoldFont.SetWeight( WEIGHT_BOLD );
    SetFont( aBoldFont );

    Rectangle aTitleRect = GetTextRect( Rectangle( 0, 0, TEXT_MAX_WIDTH, TEXT_MAX_HEIGHT ),
                                        maBubbleTitle,
                                        TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );

    SetFont( aOldFont );
    Rectangle aTextRect = GetTextRect( Rectangle( 0, 0, TEXT_MAX_WIDTH, TEXT_MAX_HEIGHT ),
                                       maBubbleText,
                                       TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );

    aWindowSize.setHeight( aTitleRect.GetHeight() * 7 / 4+ aTextRect.GetHeight() +
                           3 * BUBBLE_BORDER + TIP_HEIGHT );

    if ( aTitleRect.GetWidth() > aTextRect.GetWidth() )
        aWindowSize.setWidth( aTitleRect.GetWidth() );
    else
        aWindowSize.setWidth( aTextRect.GetWidth() );

    aWindowSize.setWidth( aWindowSize.Width() + 3 * BUBBLE_BORDER + aImgSize.Width() );

    if ( aWindowSize.Height() < aImgSize.Height() + TIP_HEIGHT + 2 * BUBBLE_BORDER )
        aWindowSize.setHeight( aImgSize.Height() + TIP_HEIGHT + 2 * BUBBLE_BORDER );

    Point aPos;
    aPos.X() = maTipPos.X() - aWindowSize.Width() + TIP_RIGHT_OFFSET;
    aPos.Y() = maTipPos.Y();
    SetPosSizePixel( aPos, aWindowSize );

    FloatingWindow::Show( bVisible, nFlags );
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

extern "C" void SAL_CALL
component_getImplementationEnvironment( const sal_Char **aEnvTypeName, uno_Environment **)
{
    *aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

//------------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL
component_writeInfo(void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_writeInfoHelper(
        pServiceManager,
        pRegistryKey,
        kImplementations_entries
    );
}

//------------------------------------------------------------------------------

extern "C" void *
component_getFactory(const sal_Char *pszImplementationName, void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pszImplementationName,
        pServiceManager,
        pRegistryKey,
        kImplementations_entries) ;
}

