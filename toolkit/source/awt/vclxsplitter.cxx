#include "vclxsplitter.hxx"
#include "toolkit/helper/property.hxx"
#include <tools/debug.hxx>
#include <vcl/split.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/awt/PosSize.hpp>

#include <assert.h>
#include <sal/macros.h>

using namespace toolkit;
//........................................................................
namespace layoutimpl
{
//........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;

class SplitterChildProps : public PropHelper
{
public:
    SplitterChildProps( VCLXSplitter::ChildData *pData )
    {
        addProp( RTL_CONSTASCII_USTRINGPARAM( "Shrink" ),
                 ::getCppuType( static_cast< const rtl::OUString* >( NULL ) ),
                 &(pData->bShrink) );
    }
    PROPHELPER_SET_INFO
};

//====================================================================
//= VCLXSplitter
//====================================================================
DBG_NAME( VCLXSplitter )
//--------------------------------------------------------------------
VCLXSplitter::VCLXSplitter( bool bHorizontal )
: VCLXWindow()
    , Container()
{
    DBG_CTOR( VCLXSplitter, NULL );
    mnHandleRatio = 0.5;
    mbHandlePressed = false;
    mbHorizontal = bHorizontal;
    mpSplitter = NULL;
}

//--------------------------------------------------------------------
VCLXSplitter::~VCLXSplitter()
{
    DBG_DTOR( VCLXSplitter, NULL );
}

//--------------------------------------------------------------------
IMPLEMENT_2_FORWARD_XINTERFACE1( VCLXSplitter, VCLXWindow, Container )

//--------------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER1( VCLXSplitter, VCLXWindow )

//--------------------------------------------------------------------
void SAL_CALL VCLXSplitter::dispose( ) throw(RuntimeException)
{
    {
        ::vos::OGuard aGuard( GetMutex() );

        EventObject aDisposeEvent;
        aDisposeEvent.Source = *this;
//            maTabListeners.disposeAndClear( aDisposeEvent );
    }

    VCLXWindow::dispose();
}

//--------------------------------------------------------------------
void VCLXSplitter::ensureSplitter()
{
    if ( !mpSplitter )
    {
        mpSplitter = new Splitter( GetWindow() , mbHorizontal ? WB_HORZ : WB_VERT );
        mpSplitter->Show();
        mpSplitter->SetEndSplitHdl( LINK( this, VCLXSplitter, HandleMovedHdl ) );
    }
}

//--------------------------------------------------------------------
void SAL_CALL VCLXSplitter::addChild(
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains > &xChild )
    throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::awt::MaxChildrenException)
{
    ChildData *pData;
    if ( ! maChildren[ 0 ].xChild.is() )
        pData = &maChildren[ 0 ];
    else if ( ! maChildren[ 1 ].xChild.is() )
        pData = &maChildren[ 1 ];
    else
        throw css::awt::MaxChildrenException();

    if ( xChild.is() )
    {
        pData->xChild = xChild;
        setChildParent( xChild );
        queueResize();
    }
}

//--------------------------------------------------------------------
void SAL_CALL VCLXSplitter::removeChild( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains > &xChild )
    throw (::com::sun::star::uno::RuntimeException)
{
    ChildData *pData = 0;
    if ( maChildren[ 0 ].xChild == xChild )
        pData = &maChildren[ 0 ];
    else if ( maChildren[ 1 ].xChild == xChild )
        pData = &maChildren[ 1 ];

    if ( pData )
    {
        pData->xChild = uno::Reference< awt::XLayoutConstrains >();
        unsetChildParent( xChild );
        queueResize();
    }
}

//--------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference
                                 < ::com::sun::star::awt::XLayoutConstrains > > SAL_CALL VCLXSplitter::getChildren()
    throw (::com::sun::star::uno::RuntimeException)
{
    int childLen = (maChildren[ 0 ].xChild.is() ? 1: 0) +
        (maChildren[ 1 ].xChild.is() ? 1: 0);
    uno::Sequence< uno::Reference< awt::XLayoutConstrains > > childrenSeq( childLen );
    int i = 0;
    if ( maChildren[ 0 ].xChild.is() )
        childrenSeq[ i++ ] = maChildren[ 0 ].xChild;
    if ( maChildren[ 1 ].xChild.is() )
        childrenSeq[ i++ ] = maChildren[ 1 ].xChild;
    return childrenSeq;
}

//--------------------------------------------------------------------
uno::Reference< beans::XPropertySet > SAL_CALL
VCLXSplitter::getChildProperties( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw (uno::RuntimeException)
{
    ChildData *pData = 0;
    if ( maChildren[ 0 ].xChild == xChild )
        pData = &maChildren[ 0 ];
    else if ( maChildren[ 1 ].xChild == xChild )
        pData = &maChildren[ 1 ];

    if ( pData )
    {
        if ( !pData->xProps.is() )
        {
            PropHelper *pProps = new SplitterChildProps( pData );
            pProps->setChangeListener( this );
            pData->xProps = pProps;
        }
        return pData->xProps;
    }
    return uno::Reference< beans::XPropertySet >();
}

//--------------------------------------------------------------------
void SAL_CALL VCLXSplitter::allocateArea(
    const ::com::sun::star::awt::Rectangle &rArea )
    throw (::com::sun::star::uno::RuntimeException)
{
    ensureSplitter();  // shouldn't be needed...
    getMinimumSize();
    int splitDiff;
    if ( mbHorizontal )
        splitDiff = rArea.Width - maAllocation.Width;
    else
        splitDiff = rArea.Height - maAllocation.Height;

    assert(mpSplitter);
    if ( splitDiff )
        mpSplitter->SetSplitPosPixel( mpSplitter->GetSplitPosPixel() + splitDiff/2 );

    maAllocation = rArea;
    int width = mbHorizontal ? rArea.Width : rArea.Height;
    int splitLen = 2;
    int splitPos = mpSplitter->GetSplitPosPixel();
    setPosSize( rArea.X, rArea.Y, rArea.Width, rArea.Height, PosSize::POSSIZE );
    if ( mbHorizontal )
        mpSplitter->SetPosSizePixel( splitPos, 0, splitLen, rArea.Height, PosSize::POSSIZE );
    else
        mpSplitter->SetPosSizePixel( 0, splitPos, rArea.Width, splitLen, PosSize::POSSIZE );
    mpSplitter->SetDragRectPixel( ::Rectangle( 0, 0, rArea.Width, rArea.Height ) );
    int leftWidth = splitPos;
    int rightWidth = width - splitPos;

    if ( maChildren[ 0 ].xChild.is() )
    {
        awt::Rectangle childRect( 0, 0, rArea.Width, rArea.Height );

        if ( mbHorizontal )
            childRect.Width = leftWidth - 2;
        else
            childRect.Height = leftWidth - 2;
        allocateChildAt( maChildren[ 0 ].xChild, childRect );
    }
    if ( maChildren[ 1 ].xChild.is() )
    {
        awt::Rectangle childRect( 0, 0, rArea.Width, rArea.Height );

        if ( mbHorizontal )
        {
            childRect.X += leftWidth + splitLen + 2;
            childRect.Width = rightWidth;
        }
        else
        {
            childRect.Y += leftWidth + splitLen + 2;
            childRect.Height = rightWidth;
        }
        allocateChildAt( maChildren[ 1 ].xChild, childRect );
    }
}

//--------------------------------------------------------------------
::com::sun::star::awt::Size SAL_CALL VCLXSplitter::getMinimumSize()
    throw(::com::sun::star::uno::RuntimeException)
{
    ensureSplitter();

    awt::Size size( mbHorizontal ? 2 : 0, mbHorizontal ? 0 : 2 );
    for( unsigned int i = 0; i < 2; i++ )
    {
        if ( maChildren[ i ].xChild.is() )
        {
            awt::Size childSize = maChildren[ i ].xChild->getMinimumSize();
            if ( mbHorizontal )
            {
                size.Width += childSize.Width;
                size.Height = SAL_MAX( size.Height, childSize.Height );
            }
            else
            {
                size.Width = SAL_MAX( size.Width, childSize.Width );
                size.Height += childSize.Height;
            }
        }
    }

    maRequisition = size;
    return size;
}

//--------------------------------------------------------------------
void VCLXSplitter::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
{
    ::vos::OClearableGuard aGuard( GetMutex() );
    switch ( _rVclWindowEvent.GetId() )
    {
        default:
            aGuard.clear();
            VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
            break;
    }
}

//--------------------------------------------------------------------
void SAL_CALL VCLXSplitter::setProperty( const ::rtl::OUString& PropertyName, const Any &Value ) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
                VCLXWindow::setProperty( PropertyName, Value );
        }
    }
}

//--------------------------------------------------------------------
Any SAL_CALL VCLXSplitter::getProperty( const ::rtl::OUString& PropertyName ) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Any aReturn;
    if ( GetWindow() )
    {
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
                aReturn = VCLXWindow::getProperty( PropertyName );
        }
    }
    return aReturn;
}

IMPL_LINK( VCLXSplitter, HandleMovedHdl, Splitter *, pSplitter )
{
    (void) pSplitter;
    forceRecalc();
    return 0;
}

//........................................................................
} // namespace toolkit
//........................................................................
