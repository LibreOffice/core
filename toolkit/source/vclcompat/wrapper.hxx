#ifndef WRAPPER_HXX
#define WRAPPER_HXX

#include <layout/layout.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <cstring>

namespace css = com::sun::star;

namespace layout
{
// Window
class WindowImpl
{
  public:
    Window  *mpWindow;
    Context *mpCtx;
    css::uno::Reference< css::awt::XWindow >        mxWindow;
    css::uno::Reference< css::awt::XVclWindowPeer > mxVclPeer;

    WindowImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : mpWindow ( pWindow )
        , mpCtx ( pCtx )
        , mxWindow ( xPeer, css::uno::UNO_QUERY )
        , mxVclPeer( xPeer, css::uno::UNO_QUERY )
    {
    }

    virtual ~WindowImpl()
    {
    }

    void wrapperGone()
    {
        mpWindow = NULL;
        mpCtx = NULL;
        if ( mxWindow.is() )
        {
            css::uno::Reference< css::lang::XComponent > xComp( mxWindow, css::uno::UNO_QUERY );
            mxWindow.clear();
            if ( xComp.is() )
                xComp->dispose();
        }
    }

    css::uno::Any getProperty( const char *pName )
    {
        if ( !this || !mxVclPeer.is() )
            return css::uno::Any();
        return mxVclPeer->getProperty
            ( rtl::OUString( pName, strlen( pName ), RTL_TEXTENCODING_ASCII_US ) );
    }
    void setProperty( const char *pName, css::uno::Any aAny )
    {
        if ( !this || !mxVclPeer.is() )
            return;
        mxVclPeer->setProperty
            ( rtl::OUString( pName, strlen( pName ), RTL_TEXTENCODING_ASCII_US ), aAny );
    }
};

// Window/Control
class ControlImpl : public WindowImpl
{
  public:
    ControlImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : WindowImpl( pCtx, xPeer, pWindow ) {}
};

class ImageImpl
{
  public:
    css::uno::Reference< css::graphic::XGraphic > mxGraphic;
    ImageImpl( const char *pName );
};

// Window/Control/Edit
inline WindowImpl &Window::getImpl() const
{ return *(static_cast< WindowImpl * >( mpImpl )); }

// Helpers for defining boiler-plate constructors ...
// Could in-line in top-level but not with safe static_casts.
#define DECL_GET_IMPL_IMPL(t) \
    inline t##Impl &t::getImpl() const \
    { \
        return *(static_cast<t##Impl *>(mpImpl)); \
    }
#define DECL_CONSTRUCTOR_IMPLS(t,par,unoName) \
    t::t( Context *pCtx, const char *pId, sal_uInt32 nId ) \
        : par( new t##Impl( pCtx, pCtx->GetPeerHandle( pId, nId ), this ) ) \
    { \
    } \
    t::t( Window *pParent, WinBits nBits) \
        : par( new t##Impl( pParent->getContext(), Window::CreatePeer( pParent, nBits, unoName ), this ) ) \
    { \
    }
#define DECL_CONSTRUCTOR_IMPLS_2(t,win_par,other_par,unoName) \
    t::t( Context *pCtx, const char *pId, sal_uInt32 nId ) \
        : win_par( new t##Impl( pCtx, pCtx->GetPeerHandle( pId, nId ), this ) ) \
        , other_par( new other_par##Impl( Window::GetPeer() ) ) \
    { \
    } \
    t::t( Window *pParent, WinBits nBits) \
        : win_par( new t##Impl( pParent->getContext(), Window::CreatePeer( pParent, nBits, unoName ), this ) ) \
        , other_par( new other_par##Impl( Window::GetPeer() ) ) \
    { \
    }

}; // end namespace layout

#endif // WRAPPER_HXX
