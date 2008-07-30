/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wrapper.cxx,v $
 *
 * $Revision: 1.3 $
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

#include "wrapper.hxx"

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/awt/XProgressBar.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/window.hxx>
#include <toolkit/awt/vclxwindow.hxx>

#include <layout/layoutcore.hxx>
#include <layout/factory.hxx>
#include <layout/root.hxx>
#include <vcl/image.hxx>

using namespace ::com::sun::star;

namespace layout
{

// Context bits ...
class ContextImpl
{
    uno::Reference< awt::XLayoutRoot > mxRoot;
    uno::Reference< container::XNameAccess > mxNameAccess;
    PeerHandle mxTopLevel;

public:
    ContextImpl( char const *pPath )
    {
        uno::Sequence< uno::Any > aParams( 1 );
        aParams[0] <<= rtl::OUString( pPath, strlen( pPath ), RTL_TEXTENCODING_UTF8 );

        uno::Reference< lang::XSingleServiceFactory > xFactory(
            comphelper::createProcessComponent(
                rtl::OUString::createFromAscii( "com.sun.star.awt.Layout" ) ),
            uno::UNO_QUERY );
        if ( !xFactory.is() )
        {
            throw uno::RuntimeException(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Layout engine not installed" ) ),
                uno::Reference< uno::XInterface >() );
        }
        mxRoot = uno::Reference< awt::XLayoutRoot >(
            xFactory->createInstanceWithArguments( aParams ),
            uno::UNO_QUERY );

        mxNameAccess = uno::Reference< container::XNameAccess >( mxRoot, uno::UNO_QUERY );
    }

    ~ContextImpl()
    {
    }

    PeerHandle getByName( const rtl::OUString &rName )
    {
        uno::Any val = mxNameAccess->getByName( rName );
        PeerHandle xRet;
        val >>= xRet;
        return xRet;
    }
    PeerHandle getTopLevel()                 { return mxTopLevel; }
    void setTopLevel( PeerHandle xToplevel ) { mxTopLevel = xToplevel; }
    PeerHandle getRoot()                     { return mxRoot; }
};

Context::Context( const char *pPath )
    : pImpl( new ContextImpl( pPath ) )
{
}
Context::~Context()
{
    delete pImpl;
    pImpl = NULL;
}

void Context::setToplevel( PeerHandle xToplevel )
{
    pImpl->setTopLevel( xToplevel );
}

PeerHandle Context::getToplevel()
{
    return pImpl->getTopLevel();
}
PeerHandle Context::getRoot()
{
     return pImpl->getRoot();
}

PeerHandle Context::GetPeerHandle( const char *pId, sal_uInt32 nId ) const
{
    PeerHandle xHandle;
    xHandle = pImpl->getByName( rtl::OUString( pId, strlen( pId ), RTL_TEXTENCODING_UTF8 ) );
    if ( !xHandle.is() )
    {
        DBG_ERROR1( "Failed to fetch widget '%s'", pId );
    }

    if ( nId != 0 )
    {
        rtl::OString aStr = rtl::OString::valueOf( (sal_Int32) nId );
        xHandle = GetPeerHandle( aStr, 0 );
    }
    return xHandle;
}

// Window/Dialog
class DialogImpl : public WindowImpl
{
public:
    uno::Reference< awt::XDialog2 > mxDialog;
    DialogImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : WindowImpl( pCtx, xPeer, pWindow )
        , mxDialog( xPeer, uno::UNO_QUERY )
    {
    }
};

// Accessors
DECL_GET_IMPL_IMPL( Control )
DECL_GET_IMPL_IMPL( Dialog )

Window::Window( WindowImpl *pImpl )
: mpImpl( pImpl )
{
}

Window::~Window()
{
    /* likely to be an UNO object - with floating references */
    mpImpl->wrapperGone();
    mpImpl = NULL;
}

Context *Window::getContext()
{
    return this && mpImpl ? mpImpl->mpCtx : NULL;
}

PeerHandle Window::GetPeer()
{
    if (!mpImpl)
        return PeerHandle();
    return mpImpl->mxWindow;
}

struct ToolkitVclPropsMap
{
    WinBits vclStyle;
    long initAttr;
    const char *propName;

    // the value to give the prop to enable/disable it -- not the most brilliant
    // type declaration and storage, but does the work... properties are
    // either a boolean or a short since they are either a directly wrappers for
    // a WinBit, or aggregates related (like Align for WB_LEFT, _RIGHT and _CENTER).
    bool isBoolean;
    short enableProp, disableProp;
};

#define TYPE_BOOL  true
#define TYPE_SHORT false
#define NOTYPE     0
static const ToolkitVclPropsMap toolkitVclPropsMap[] =
{
    { WB_BORDER,    awt::WindowAttribute::BORDER,    "Border", TYPE_SHORT, 1, 0 },
    { WB_NOBORDER,    awt::VclWindowPeerAttribute::NOBORDER,    "Border", TYPE_SHORT, 0, 1 },
    { WB_SIZEABLE,    awt::WindowAttribute::SIZEABLE,    NULL, NOTYPE, 0, 0 },
    { WB_MOVEABLE,    awt::WindowAttribute::MOVEABLE,    NULL, NOTYPE, 0, 0 },
    { WB_CLOSEABLE,    awt::WindowAttribute::CLOSEABLE,    NULL, NOTYPE, 0, 0 },

    { WB_HSCROLL,    awt::VclWindowPeerAttribute::HSCROLL,    NULL, NOTYPE, 0, 0 },
    { WB_VSCROLL,    awt::VclWindowPeerAttribute::VSCROLL,    NULL, NOTYPE, 0, 0 },
    { WB_LEFT,    awt::VclWindowPeerAttribute::LEFT,    "Align", TYPE_SHORT, 0, 0 },
    { WB_CENTER,    awt::VclWindowPeerAttribute::CENTER,    "Align", TYPE_SHORT, 1, 0 },
    { WB_RIGHT,    awt::VclWindowPeerAttribute::RIGHT,    "Align", TYPE_SHORT, 2, 0 },
    { WB_SPIN,    awt::VclWindowPeerAttribute::SPIN,    NULL, NOTYPE, 0, 0 },
    { WB_SORT,    awt::VclWindowPeerAttribute::SORT,    NULL, NOTYPE, 0, 0 },
    { WB_DROPDOWN,    awt::VclWindowPeerAttribute::DROPDOWN,    "Dropdown",    TYPE_BOOL, 1, 0 },
    { WB_DEFBUTTON,    awt::VclWindowPeerAttribute::DEFBUTTON,    "DefaultButton", TYPE_BOOL, 1, 0 },
    { WB_READONLY,    awt::VclWindowPeerAttribute::READONLY,    NULL, NOTYPE, 0, 0 },
    { WB_CLIPCHILDREN,    awt::VclWindowPeerAttribute::CLIPCHILDREN,    NULL, NOTYPE, 0, 0 },
    { WB_GROUP,    awt::VclWindowPeerAttribute::GROUP,    NULL, NOTYPE, 0, 0 },

    { WB_OK,    awt::VclWindowPeerAttribute::OK,    NULL, NOTYPE, 0, 0 },
    { WB_OK_CANCEL,    awt::VclWindowPeerAttribute::OK_CANCEL,    NULL, NOTYPE, 0, 0 },
    { WB_YES_NO,    awt::VclWindowPeerAttribute::YES_NO,    NULL, NOTYPE, 0, 0 },
    { WB_YES_NO_CANCEL,    awt::VclWindowPeerAttribute::YES_NO_CANCEL,    NULL, NOTYPE, 1, 0 },
    { WB_RETRY_CANCEL,    awt::VclWindowPeerAttribute::RETRY_CANCEL,    NULL, NOTYPE, 1, 0 },
    { WB_DEF_OK,    awt::VclWindowPeerAttribute::DEF_OK,    NULL, NOTYPE, 0, 0 },
    { WB_DEF_CANCEL,    awt::VclWindowPeerAttribute::DEF_CANCEL,    NULL, NOTYPE, 1, 0 },
    { WB_DEF_RETRY,    awt::VclWindowPeerAttribute::DEF_RETRY,    NULL, NOTYPE, 0, 0 },
    { WB_DEF_YES,    awt::VclWindowPeerAttribute::DEF_YES,    NULL, NOTYPE, 0, 0 },
    { WB_DEF_NO,    awt::VclWindowPeerAttribute::DEF_NO,    NULL, NOTYPE, 0, 0 },

    { WB_AUTOHSCROLL, awt::VclWindowPeerAttribute::AUTOHSCROLL, "AutoHScroll", TYPE_BOOL, 1, 0 },
    { WB_AUTOVSCROLL, awt::VclWindowPeerAttribute::AUTOVSCROLL, "AutoVScroll",    TYPE_BOOL, 1, 0 },

    { WB_WORDBREAK,    0,    "MultiLine", TYPE_BOOL, 1, 0 },
    { WB_NOPOINTERFOCUS,    0,    "FocusOnClick", TYPE_BOOL, 1, 0 },
    { WB_TOGGLE,    0,    "Toggle", TYPE_BOOL, 1, 0 },
    { WB_REPEAT,    0,    "Repeat", TYPE_BOOL, 1, 0 },
    { WB_NOHIDESELECTION,    0,    "HideInactiveSelection", TYPE_BOOL, 1, 0 },
};
#undef TYPE_BOOL
#undef TYPE_SHORT
#undef NOTYPE

static const int toolkitVclPropsMapLen =
    sizeof( toolkitVclPropsMap ) / sizeof( ToolkitVclPropsMap );

void Window::SetStyle( WinBits nStyle )
{
    uno::Reference< awt::XVclWindowPeer > xPeer = mpImpl->mxVclPeer;
    for ( int i = 0; i < toolkitVclPropsMapLen; i++ )
    {
        if ( toolkitVclPropsMap[ i ].propName )
        {
            short nValue;
            if ( nStyle & toolkitVclPropsMap[ i ].vclStyle )
                nValue = toolkitVclPropsMap[ i ].enableProp;
            else
                nValue = toolkitVclPropsMap[ i ].disableProp;
            uno::Any aValue;
            if ( toolkitVclPropsMap[ i ].isBoolean )
                aValue = uno::makeAny( (bool) nValue );
            else
                aValue = uno::makeAny( (short) nValue );
            mpImpl->setProperty( toolkitVclPropsMap[ i ].propName, aValue );
        }
    }
}

WinBits Window::GetStyle()
{
    uno::Reference< awt::XVclWindowPeer > xPeer = mpImpl->mxVclPeer;
    WinBits ret = 0;
    for ( int i = 0; i < toolkitVclPropsMapLen; i++ )
    {
        if ( toolkitVclPropsMap[ i ].propName )
        {
            short nValue = 0;
            if ( toolkitVclPropsMap[ i ].isBoolean )
            {
                bool bValue = false;
                mpImpl->getProperty( toolkitVclPropsMap[ i ].propName ) >>= bValue;
                nValue = bValue ? 1 : 0;
            }
            else
                mpImpl->getProperty( toolkitVclPropsMap[ i ].propName ) >>= nValue;
            if ( nValue == toolkitVclPropsMap[ i ].enableProp )
                ret |= toolkitVclPropsMap[i].vclStyle;
        }
    }
    return ret;
}

/* Unpleasant way to get an xToolkit pointer ... */
uno::Reference< awt::XToolkit > getToolkit()
{
    static uno::Reference< awt::XToolkit > xToolkit;
    if (!xToolkit.is())
    {
        // Urgh ...
        xToolkit = uno::Reference< awt::XToolkit >(
            ::comphelper::getProcessServiceFactory()->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.Toolkit" ) ) ),
            uno::UNO_QUERY );
        if ( !xToolkit.is() )
            throw uno::RuntimeException(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "failed to create toolkit!") ),
                uno::Reference< uno::XInterface >() );
    }
    return xToolkit;
}

PeerHandle Window::CreatePeer( Window *pParent, WinBits nStyle, const char *pName)
{

    long nWinAttrbs = 0;
    for ( int i = 0; i < toolkitVclPropsMapLen; i++ )
        if ( nStyle & toolkitVclPropsMap[ i ].vclStyle )
            nWinAttrbs |= toolkitVclPropsMap[ i ].initAttr;

    return layoutimpl::createWidget( getToolkit(), pParent->GetPeer(),
                                     rtl::OUString::createFromAscii( pName ), nWinAttrbs );
}

void Window::Enable( bool bEnable )
{
    if ( !getImpl().mxWindow.is() )
        return;
    getImpl().mxWindow->setEnable( bEnable );
}

void Window::Show( BOOL bVisible )
{
    if ( !getImpl().mxWindow.is() )
        return;
    getImpl().mxWindow->setVisible( bVisible );
}

void Window::GrabFocus()
{
    if ( !getImpl().mxWindow.is() )
        return;
    getImpl().mxWindow->setFocus();
}

Dialog::Dialog( Window *pParent, const char *pXMLPath, const char *pId, sal_uInt32 nId )
    : Context( pXMLPath )
    , Window( new DialogImpl( this, Context::GetPeerHandle( pId, nId ), this ) )
{
    if ( pParent )
        SetParent( pParent );
}

Dialog::Dialog( ::Window *pParent, const char *pXMLPath, const char *pId, sal_uInt32 nId )
    : Context( pXMLPath )
    , Window( new DialogImpl( this, Context::GetPeerHandle( pId, nId ), this ) )
{
    if ( pParent )
        SetParent( pParent );
}

void Dialog::SetParent( ::Window *pParent )
{
    uno::Reference <awt::XWindow> ref( GetPeer(), uno::UNO_QUERY );
    ::Window *window = VCLXWindow::GetImplementation( ref )->GetWindow();
    window->SetParent( pParent );
}

void Dialog::SetParent( Window *pParent )
{
    uno::Reference <awt::XWindow> parentRef( pParent->GetPeer(), uno::UNO_QUERY );
    ::Window *parentWindow = VCLXWindow::GetImplementation( parentRef )->GetWindow();
    SetParent( parentWindow );
}

short Dialog::Execute()
{
    if ( !getImpl().mxDialog.is() )
        return -1;
    return getImpl().mxDialog->execute();
}

void Dialog::EndDialog( long nResult )
{
    if ( !getImpl().mxDialog.is() )
        return;
    getImpl().mxDialog->endDialog( nResult );
}

void Dialog::SetText( const String& rStr )
{
    if ( !getImpl().mxDialog.is() )
        return;
    getImpl().mxDialog->setTitle( rStr );
}

class FixedLineImpl : public ControlImpl
{
public:
    FixedLineImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ControlImpl( pCtx, xPeer, pWindow )
    {
    }
};

DECL_CONSTRUCTOR_IMPLS( FixedLine, Control, "hfixedline" );
DECL_GET_IMPL_IMPL( FixedLine )

bool FixedLine::IsEnabled()
{
    //FIXME
    return true;
}

class FixedTextImpl : public ControlImpl
{
public:
    uno::Reference< awt::XFixedText > mxFixedText;
    FixedTextImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ControlImpl( pCtx, xPeer, pWindow )
        , mxFixedText( xPeer, uno::UNO_QUERY )
    {
    }

    virtual void SAL_CALL disposing( const css::lang::EventObject& /* Source */ )
        throw (css::uno::RuntimeException)
    {
        mxFixedText.clear();
    }
};

DECL_CONSTRUCTOR_IMPLS( FixedText, Control, "fixedtext" );
DECL_GET_IMPL_IMPL( FixedText )

void FixedText::SetText( const String& rStr )
{
    if ( !getImpl().mxFixedText.is() )
        return;
    getImpl().mxFixedText->setText( rStr );
}

class FixedInfoImpl : public FixedTextImpl
{
public:
    FixedInfoImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : FixedTextImpl( pCtx, xPeer, pWindow )
    {
    }
};

DECL_CONSTRUCTOR_IMPLS( FixedInfo, FixedText, "fixedinfo" );
DECL_GET_IMPL_IMPL( FixedInfo );

class ProgressBarImpl : public ControlImpl
{
public:
    uno::Reference< awt::XProgressBar > mxProgressBar;
    ProgressBarImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ControlImpl( pCtx, xPeer, pWindow )
        , mxProgressBar( xPeer, uno::UNO_QUERY )
    {
    }

    virtual void SAL_CALL disposing( const css::lang::EventObject& /* Source */ )
        throw (css::uno::RuntimeException)
    {
        mxProgressBar.clear();
    }
};


class FixedImageImpl: public ControlImpl
{
public:
    uno::Reference< graphic::XGraphic > mxGraphic;
    FixedImageImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow)
//                    const char *pName )
        : ControlImpl( pCtx, xPeer, pWindow )
          //, mxGraphic( layoutimpl::loadGraphic( pName ) )
        , mxGraphic( xPeer, uno::UNO_QUERY )
    {
        if ( !mxGraphic.is() )
        {
            DBG_ERROR( "ERROR: failed to load image: `%s'" /*, pName*/ );
        }
#if 0
        else
            getImpl().mxGraphic->...();
#endif
    }
};

DECL_CONSTRUCTOR_IMPLS( FixedImage, Control, "fixedimage" );
DECL_GET_IMPL_IMPL( FixedImage )

void FixedImage::setImage( ::Image const& i )
{
    (void) i;
    if ( !getImpl().mxGraphic.is() )
        return;
    //FIXME: hack moved to proplist
    //getImpl().mxGraphic =
}

#if 0

    FixedImage::FixedImage( const char *pName )
    : pImpl( new FixedImageImpl( pName ) )
{
}

FixedImage::~FixedImage()
{
    delete pImpl;
}

#endif


DECL_CONSTRUCTOR_IMPLS( ProgressBar, Control, "ProgressBar" );
#if 0
ProgressBar::ProgressBar( Context *pCtx, const char *pId, sal_uInt32 nId )
    : Control( new ProgressBarImpl( pCtx, pCtx->GetPeerHandle( pId, nId ), this ) )
{
}
ProgressBar::ProgressBar( Window *pParent, WinBits nBits)
//    : Control( new ProgressBarImpl( pParent->getContext(), Window::CreatePeer( pParent, nBits, "ProgressBar" ), this ) )
    : Control( new ProgressBarImpl( pParent->getContext(), Window::CreatePeer( pParent, nBits, "progressbar" ), this ) )
{
}
#endif

DECL_GET_IMPL_IMPL( ProgressBar )


void ProgressBar::SetForegroundColor( css::util::Color color )
{
    if ( !getImpl().mxProgressBar.is() )
        return;
    getImpl().mxProgressBar->setForegroundColor( color );
}

void ProgressBar::SetBackgroundColor( css::util::Color color )
{
    if ( !getImpl().mxProgressBar.is() )
        return;
    getImpl().mxProgressBar->setBackgroundColor( color );
}

void ProgressBar::SetValue( sal_Int32 i )
{
    if ( !getImpl().mxProgressBar.is() )
        return;
    getImpl().mxProgressBar->setValue( i );
}

void ProgressBar::SetRange( sal_Int32 min, sal_Int32 max )
{
    if ( !getImpl().mxProgressBar.is() )
        return;
    getImpl().mxProgressBar->setRange( min, max );
}

sal_Int32 ProgressBar::GetValue()
{
    if ( !getImpl().mxProgressBar.is() )
        return 0;
    return getImpl().mxProgressBar->getValue();
}

} // namespace layout
