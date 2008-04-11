#include "vclxtabcontrol.hxx"
#include "toolkit/helper/property.hxx"
#include <com/sun/star/awt/ScrollBarOrientation.hpp>


#include <tools/debug.hxx>
#include <vcl/tabctrl.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <vcl/tabpage.hxx>
#include <com/sun/star/awt/PosSize.hpp>
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

class TabChildProps : public PropHelper
{
public:
    TabChildProps( VCLXTabControl::ChildData *pData )
    {
        addProp( RTL_CONSTASCII_USTRINGPARAM( "Title" ),
                 ::getCppuType( static_cast< const rtl::OUString* >( NULL ) ),
                 &(pData->aTitle) );
    }
    PROPHELPER_SET_INFO
};

//====================================================================
//= VCLXTabControl
//====================================================================
DBG_NAME( VCLXTabControl )
//--------------------------------------------------------------------
VCLXTabControl::VCLXTabControl()
: VCLXWindow()
    , VCLXTabControl_Base()
    , Container()
{
    DBG_CTOR( VCLXTabControl, NULL );
    mnNextTabId = 1;
    mnChildrenNb = 0;
}

//--------------------------------------------------------------------
VCLXTabControl::~VCLXTabControl()
{
    DBG_DTOR( VCLXTabControl, NULL );
}

//--------------------------------------------------------------------
IMPLEMENT_2_FORWARD_XINTERFACE2( VCLXTabControl, VCLXWindow, Container, VCLXTabControl_Base )

//--------------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXTabControl, VCLXWindow, VCLXTabControl_Base )

//--------------------------------------------------------------------
void SAL_CALL VCLXTabControl::dispose( ) throw(RuntimeException)
{
    {
        ::vos::OGuard aGuard( GetMutex() );

        EventObject aDisposeEvent;
        aDisposeEvent.Source = *this;
//            maTabListeners.disposeAndClear( aDisposeEvent );
    }

    VCLXWindow::dispose();
}
/*
//--------------------------------------------------------------------
void SAL_CALL VCLXTabControl::addTabListener( const Reference< XTabListener >& listener ) throw (RuntimeException)
{
if ( listener.is() )
maTabListeners.addInterface( listener );
}

//--------------------------------------------------------------------
void SAL_CALL VCLXTabControl::removeTabListener( const Reference< XTabListener >& listener ) throw (RuntimeException)
{
if ( listener.is() )
maTabListeners.removeInterface( listener );
}
*/

//--------------------------------------------------------------------
TabControl *VCLXTabControl::getTabControl() const throw (RuntimeException)
{
    TabControl *pTabControl = static_cast< TabControl* >( GetWindow() );
    if ( pTabControl )
        return pTabControl;
    throw RuntimeException();
}

//--------------------------------------------------------------------
sal_Int32 SAL_CALL VCLXTabControl::insertTab() throw (RuntimeException)
{
    TabControl *pTabControl = getTabControl();
    int id = mnNextTabId++;
    rtl::OUString title (RTL_CONSTASCII_USTRINGPARAM( "" ) );
    pTabControl->InsertPage( id, title.getStr(), TAB_APPEND );
    pTabControl->SetTabPage( id, new TabPage( pTabControl ) );
    return id;
}

//--------------------------------------------------------------------
void SAL_CALL VCLXTabControl::removeTab( sal_Int32 ID ) throw (RuntimeException, IndexOutOfBoundsException)
{
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( ID ) == NULL )
        throw IndexOutOfBoundsException();
    pTabControl->RemovePage( ID );
}


//--------------------------------------------------------------------
void SAL_CALL VCLXTabControl::activateTab( sal_Int32 ID ) throw (RuntimeException, IndexOutOfBoundsException)
{
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( ID ) == NULL )
        throw IndexOutOfBoundsException();
    pTabControl->SelectTabPage( ID );
}

//--------------------------------------------------------------------
sal_Int32 SAL_CALL VCLXTabControl::getActiveTabID() throw (RuntimeException)
{
    return getTabControl()->GetCurPageId( );
}

//--------------------------------------------------------------------
void SAL_CALL VCLXTabControl::addTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    std::list< ::com::sun::star::uno::Reference
        < ::com::sun::star::awt::XTabListener > >::const_iterator it;
    for( it = mxTabListeners.begin(); it != mxTabListeners.end(); it++)
    {
        if ( *it == xListener )
            // already added
            return;
    }
    mxTabListeners.push_back( xListener );
}

//--------------------------------------------------------------------
void SAL_CALL VCLXTabControl::removeTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    std::list< ::com::sun::star::uno::Reference
        < ::com::sun::star::awt::XTabListener > >::iterator it;
    for( it = mxTabListeners.begin(); it != mxTabListeners.end(); it++)
    {
        if ( *it == xListener )
        {
            mxTabListeners.erase( it );
            break;
        }
    }
}

//--------------------------------------------------------------------
void SAL_CALL VCLXTabControl::setTabProps( sal_Int32 ID, const Sequence< NamedValue >& Properties ) throw (RuntimeException, IndexOutOfBoundsException)
{
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( ID ) == NULL )
        throw IndexOutOfBoundsException();

    for( int i = 0; i < Properties.getLength(); i++ )
    {
        const rtl::OUString &name = Properties[i].Name;
        const Any &value = Properties[i].Value;

        if ( name  == rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ) )
        {
            rtl::OUString title = value.get<rtl::OUString>();
            pTabControl->SetPageText( ID, title.getStr() );
        }
    }
}

//--------------------------------------------------------------------
Sequence< NamedValue > SAL_CALL VCLXTabControl::getTabProps( sal_Int32 ID )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( ID ) == NULL )
        throw IndexOutOfBoundsException();

#define ADD_PROP( seq, i, name, val ) {                                \
        NamedValue value;                                                  \
        value.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( name ) ); \
        value.Value = makeAny( val );                                      \
        seq[i] = value;                                                    \
    }

    Sequence< NamedValue > props( 2 );
    ADD_PROP( props, 0, "Title", rtl::OUString( pTabControl->GetPageText( ID ) ) );
    ADD_PROP( props, 1, "Position", pTabControl->GetPagePos( ID ) );
#undef ADD_PROP
    return props;
}

//--------------------------------------------------------------------

// TODO: draw tab border here
void SAL_CALL VCLXTabControl::draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TabControl *pTabControl = getTabControl();
    TabPage *pTabPage = pTabControl->GetTabPage( getActiveTabID() );
    if ( pTabPage )
    {
        ::Point aPos( nX, nY );
        ::Size  aSize = pTabPage->GetSizePixel();

        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( GetViewGraphics() );
        aPos  = pDev->PixelToLogic( aPos );
        aSize = pDev->PixelToLogic( aSize );

        pTabPage->Draw( pDev, aPos, aSize, 0 );
    }

    VCLXWindow::draw( nX, nY );
}

//--------------------------------------------------------------------
void SAL_CALL VCLXTabControl::addChild(
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains > &xChild )
    throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::awt::MaxChildrenException)
{
    unsigned int id = insertTab();
    if ( maChildren.size() < id )
        maChildren.resize( id, 0 );
    mnChildrenNb++;

    if ( xChild.is() )
    {
        ChildData *pData = new ChildData();
        pData->xChild = xChild;
        maChildren[ id-1 ] = pData;

        setChildParent( xChild );
        queueResize();
    }
}


//--------------------------------------------------------------------
void SAL_CALL VCLXTabControl::removeChild( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains > &xChild )
    throw (::com::sun::star::uno::RuntimeException)
{
    for( unsigned i = 0; i < maChildren.size(); i++)
    {
        if ( maChildren[ i ] && maChildren[ i ]->xChild == xChild )
        {
            removeTab( i );
            delete maChildren[ i ];
            maChildren[ i ] = NULL;
            mnChildrenNb--;

            unsetChildParent( xChild );
            queueResize();
            break;
        }
    }
}

//--------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference
                                 < ::com::sun::star::awt::XLayoutConstrains > > SAL_CALL VCLXTabControl::getChildren()
    throw (::com::sun::star::uno::RuntimeException)
{
    uno::Sequence< uno::Reference< awt::XLayoutConstrains > > childrenSeq( mnChildrenNb );
    for( unsigned si = 0, ci = 0; ci < maChildren.size(); ci++)
    {
        if ( maChildren[ ci ] && maChildren[ ci ]->xChild.is() )
            childrenSeq[si++] = maChildren[ ci ]->xChild;
    }
    return childrenSeq;
}

//--------------------------------------------------------------------
uno::Reference< beans::XPropertySet > SAL_CALL
VCLXTabControl::getChildProperties( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw (uno::RuntimeException)
{
    std::vector< ChildData * >::iterator iter;
    for( iter = maChildren.begin(); iter != maChildren.end(); iter++)
    {
        if ( (*iter)->xChild == xChild )
        {
            if ( !(*iter)->xProps.is() )
            {
                // FIXME: make me safe !
                PropHelper *pProps = new TabChildProps( *iter );
                pProps->setChangeListener( this );
                (*iter)->xProps = pProps;
            }
            return (*iter)->xProps;
        }
    }
    return uno::Reference< beans::XPropertySet >();
}

// TEMP:
static void setChildrenVisible( uno::Reference < awt::XLayoutConstrains > xChild, bool visible )
{
    uno::Reference< awt::XWindow > xWin( xChild, uno::UNO_QUERY);
    if ( xWin.is() )
    {
        xWin->setVisible( visible );
//xWin->setPosSize( 0, 0, 5, 5, PosSize::POSSIZE );
    }

    uno::Reference < awt::XLayoutContainer > xCont( xChild, uno::UNO_QUERY );
    if ( xCont.is())
    {
        uno::Sequence< uno::Reference < awt::XLayoutConstrains > > children = xCont->getChildren();
        for( int i = 0; i < children.getLength(); i++ )
        {
            setChildrenVisible( children[i], visible );
        }
    }
}

//--------------------------------------------------------------------
void SAL_CALL VCLXTabControl::allocateArea(
    const ::com::sun::star::awt::Rectangle &rArea )
    throw (::com::sun::star::uno::RuntimeException)
{
    maAllocation = rArea;

    TabControl *pTabControl = getTabControl();

// FIXME: this is wrong. We just want to set tab controls pos/size for the tabs menu,
// otherwise, it gets events that should go to children (I guess we could solve this
// by making the tabcontrol as the actual XWindow parent of its children, when importing...)
// Not sure about TabPage drawing... That doesn't work on gtk+; just ignoring that.
// LATER: Nah, the proper fix is to get the XWindow hierarchy straight.

    setPosSize( rArea.X, rArea.Y, rArea.Width, rArea.Height, PosSize::POSSIZE );

    // FIXME: we can save cycles by setting visibility more sensibly. Having
    // it here does makes it easier when changing tabs (just needs a recalc())
    for( unsigned int i = 0; i < maChildren.size(); i++ )
    {
        if ( !maChildren[ i ] )
            continue;
        ::com::sun::star::uno::Reference
              < ::com::sun::star::awt::XLayoutConstrains > xChild( maChildren[ i ]->xChild );
        if ( xChild.is() )
        {
            uno::Reference< awt::XWindow > xWin( xChild, uno::UNO_QUERY );
            bool active = (i+1 == (unsigned) getActiveTabID());

            // HACK: since our layout:: container don't implement XWindow, we have no easy
            // way to set them invisible; lets just set all their children as such :P
#if 0
            if ( xWin.is() )
                xWin->setVisible( active );
#else
            setChildrenVisible( xChild, active );
#endif

            if ( active )
            {
                ::Rectangle label_rect = pTabControl->GetTabBounds( i+1 );
                ::Rectangle page_rect = pTabControl->GetTabPageBounds( i+1 );

                awt::Rectangle childRect;
                childRect.X = page_rect.Left();
                childRect.Y = SAL_MAX( label_rect.Bottom(), page_rect.Top() );
                childRect.Width = page_rect.Right() - page_rect.Left();
                childRect.Height = page_rect.Bottom() - childRect.Y;

                allocateChildAt( xChild, childRect );
            }
        }
    }
}

//--------------------------------------------------------------------
::com::sun::star::awt::Size SAL_CALL VCLXTabControl::getMinimumSize()
    throw(::com::sun::star::uno::RuntimeException)
{
    awt::Size size = VCLXWindow::getMinimumSize();
    awt::Size childrenSize( 0, 0 );

    TabControl* pTabControl = static_cast< TabControl* >( GetWindow() );
    if ( !pTabControl )
        return size;

    // calculate size to accomodate all children
    for( unsigned int i = 0; i < maChildren.size(); i++ )
    {
        ChildData *pChild = maChildren[ i ];
        if ( pChild && pChild->xChild.is() )
        {
            // set the title prop here...
            pTabControl->SetPageText( i+1, pChild->aTitle.getStr() );

            awt::Size childSize( pChild->xChild->getMinimumSize() );
            childrenSize.Width = SAL_MAX( childSize.Width, childrenSize.Width );
            childrenSize.Height = SAL_MAX( childSize.Height, childrenSize.Height );
        }
    }

    size.Width += childrenSize.Width;
    size.Height += childrenSize.Height + 20;
    maRequisition = size;
    return size;
}

//--------------------------------------------------------------------
void VCLXTabControl::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
{
    ::vos::OClearableGuard aGuard( GetMutex() );
    TabControl* pTabControl = static_cast< TabControl* >( GetWindow() );
    if ( !pTabControl )
        return;

    switch ( _rVclWindowEvent.GetId() )
    {
        case VCLEVENT_TABPAGE_ACTIVATE:
            forceRecalc();
        case VCLEVENT_TABPAGE_DEACTIVATE:
        case VCLEVENT_TABPAGE_INSERTED:
        case VCLEVENT_TABPAGE_REMOVED:
        case VCLEVENT_TABPAGE_REMOVEDALL:
        case VCLEVENT_TABPAGE_PAGETEXTCHANGED:
        {
            ULONG page = (ULONG) _rVclWindowEvent.GetData();
            std::list< ::com::sun::star::uno::Reference
                < ::com::sun::star::awt::XTabListener > >::iterator it;
            for( it = mxTabListeners.begin(); it != mxTabListeners.end(); it++)
            {
                ::com::sun::star::uno::Reference
                    < ::com::sun::star::awt::XTabListener > listener = *it;

                switch ( _rVclWindowEvent.GetId() )
                {

                    case VCLEVENT_TABPAGE_ACTIVATE:
                        listener->activated( page );
                        break;
                    case VCLEVENT_TABPAGE_DEACTIVATE:
                        listener->deactivated( page );
                        break;
                    case VCLEVENT_TABPAGE_INSERTED:
                        listener->inserted( page );
                        break;
                    case VCLEVENT_TABPAGE_REMOVED:
                        listener->removed( page );
                        break;
                    case VCLEVENT_TABPAGE_REMOVEDALL:
                        for( int i = 1; i < mnNextTabId; i++)
                        {
                            if ( pTabControl->GetTabPage( i ) )
                                listener->removed( i );
                        }
                        break;
                    case VCLEVENT_TABPAGE_PAGETEXTCHANGED:
                        listener->changed( page, getTabProps( page ) );
                        break;
                }
            }
            break;
        }
        default:
            aGuard.clear();
            VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
            break;
    }
}

//--------------------------------------------------------------------
void SAL_CALL VCLXTabControl::setProperty( const ::rtl::OUString& PropertyName, const Any &Value ) throw(RuntimeException)
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
Any SAL_CALL VCLXTabControl::getProperty( const ::rtl::OUString& PropertyName ) throw(RuntimeException)
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

//........................................................................
} // namespace toolkit
//........................................................................
