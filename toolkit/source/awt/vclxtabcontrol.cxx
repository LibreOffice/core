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

#include "vclxtabcontrol.hxx"

#include <com/sun/star/awt/PosSize.hpp>
#include <sal/macros.h>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>

#include "forward.hxx"

namespace layoutimpl
{

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;

VCLXTabControl::ChildProps::ChildProps( VCLXTabControl::ChildData *pData )
{
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Title" ),
             ::getCppuType( static_cast< const rtl::OUString* >( NULL ) ),
             &(pData->maTitle) );
}

VCLXTabControl::ChildData::ChildData( uno::Reference< awt::XLayoutConstrains > const& xChild )
    : Box_Base::ChildData( xChild )
    , maTitle()
{
}

VCLXTabControl::ChildData*
VCLXTabControl::createChild( uno::Reference< awt::XLayoutConstrains > const& xChild )
{
    return new ChildData( xChild );
}

VCLXTabControl::ChildProps*
VCLXTabControl::createChildProps( Box_Base::ChildData *pData )
{
    return new ChildProps( static_cast<VCLXTabControl::ChildData*> ( pData ) );
}

DBG_NAME( VCLXTabControl );

#if !defined (__GNUC__)
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif /* !__GNUC__ */

VCLXTabControl::VCLXTabControl()
  : VCLXWindow()
  , VCLXTabControl_Base()
  , Box_Base()
  , mTabId (1)
  , bRealized (false)
{
#ifndef __SUNPRO_CC
    OSL_TRACE ("\n********%s:%x", __PRETTY_FUNCTION__, this);
#endif
    DBG_CTOR( VCLXTabControl, NULL );
}

VCLXTabControl::~VCLXTabControl()
{
    DBG_DTOR( VCLXTabControl, NULL );
}

IMPLEMENT_2_FORWARD_XINTERFACE2( VCLXTabControl, VCLXWindow, Container, VCLXTabControl_Base );

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXTabControl, VCLXWindow, VCLXTabControl_Base );

void SAL_CALL VCLXTabControl::dispose( ) throw(uno::RuntimeException)
{
    {
        ::vos::OGuard aGuard( GetMutex() );

        EventObject aDisposeEvent;
        aDisposeEvent.Source = W3K_EXPLICIT_CAST (*this);
//            maTabListeners.disposeAndClear( aDisposeEvent );
    }

    VCLXWindow::dispose();
}

#if 0
void SAL_CALL VCLXTabControl::addTabListener( const Reference< XTabListener >& listener ) throw (uno::RuntimeException)
{
    if ( listener.is() )
        maTabListeners.addInterface( listener );
}

void SAL_CALL VCLXTabControl::removeTabListener( const Reference< XTabListener >& listener ) throw (uno::RuntimeException)
{
    if ( listener.is() )
        maTabListeners.removeInterface( listener );
}
#endif

TabControl *VCLXTabControl::getTabControl() const throw (uno::RuntimeException)
{
    TabControl *pTabControl = static_cast< TabControl* >( GetWindow() );
    if ( pTabControl )
        return pTabControl;
    throw uno::RuntimeException();
}

sal_Int32 SAL_CALL VCLXTabControl::insertTab() throw (uno::RuntimeException)
{
    TabControl *pTabControl = getTabControl();
    sal_uInt16 id = sal::static_int_cast< sal_uInt16 >( mTabId++ );
    rtl::OUString title (RTL_CONSTASCII_USTRINGPARAM( "" ) );
    pTabControl->InsertPage( id, title.getStr(), TAB_APPEND );
    pTabControl->SetTabPage( id, new TabPage( pTabControl ) );
    return id;
}

void SAL_CALL VCLXTabControl::removeTab( sal_Int32 ID ) throw (uno::RuntimeException, IndexOutOfBoundsException)
{
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == NULL )
        throw IndexOutOfBoundsException();
    pTabControl->RemovePage( sal::static_int_cast< sal_uInt16 >( ID ) );
}

void SAL_CALL VCLXTabControl::activateTab( sal_Int32 ID ) throw (uno::RuntimeException, IndexOutOfBoundsException)
{
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == NULL )
        throw IndexOutOfBoundsException();
    pTabControl->SelectTabPage( sal::static_int_cast< sal_uInt16 >( ID ) );
}

sal_Int32 SAL_CALL VCLXTabControl::getActiveTabID() throw (uno::RuntimeException)
{
    return getTabControl()->GetCurPageId( );
}

void SAL_CALL VCLXTabControl::addTabListener( const uno::Reference< awt::XTabListener >& xListener ) throw (uno::RuntimeException)
{
    for ( std::list< uno::Reference
              < awt::XTabListener > >::const_iterator it
              = mxTabListeners.begin(); it != mxTabListeners.end(); it++ )
    {
        if ( *it == xListener )
            // already added
            return;
    }
    mxTabListeners.push_back( xListener );
}

void SAL_CALL VCLXTabControl::removeTabListener( const uno::Reference< awt::XTabListener >& xListener ) throw (uno::RuntimeException)
{
    for ( std::list< uno::Reference
              < awt::XTabListener > >::iterator it
              = mxTabListeners.begin(); it != mxTabListeners.end(); it++ )
    {
        if ( *it == xListener )
        {
            mxTabListeners.erase( it );
            break;
        }
    }
}

void SAL_CALL VCLXTabControl::setTabProps( sal_Int32 ID, const uno::Sequence< NamedValue >& Properties ) throw (uno::RuntimeException, IndexOutOfBoundsException)
{
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == NULL )
        throw IndexOutOfBoundsException();

    for ( int i = 0; i < Properties.getLength(); i++ )
    {
        const rtl::OUString &name = Properties[i].Name;
        const uno::Any &value = Properties[i].Value;

        if ( name  == rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ) )
        {
            rtl::OUString title = value.get<rtl::OUString>();
            pTabControl->SetPageText( sal::static_int_cast< sal_uInt16 >( ID ), title.getStr() );
        }
    }
}

uno::Sequence< NamedValue > SAL_CALL VCLXTabControl::getTabProps( sal_Int32 ID )
    throw (IndexOutOfBoundsException, uno::RuntimeException)
{
    TabControl *pTabControl = getTabControl();
    if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( ID ) ) == NULL )
        throw IndexOutOfBoundsException();

#define ADD_PROP( seq, i, name, val ) {                                \
        NamedValue value;                                                  \
        value.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( name ) ); \
        value.Value = uno::makeAny( val );                                      \
        seq[i] = value;                                                    \
    }

    uno::Sequence< NamedValue > props( 2 );
    ADD_PROP( props, 0, "Title", rtl::OUString( pTabControl->GetPageText( sal::static_int_cast< sal_uInt16 >( ID ) ) ) );
    ADD_PROP( props, 1, "Position", pTabControl->GetPagePos( sal::static_int_cast< sal_uInt16 >( ID ) ) );
#undef ADD_PROP
    return props;
}

// TODO: draw tab border here
void SAL_CALL VCLXTabControl::draw( sal_Int32 nX, sal_Int32 nY ) throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TabControl *pTabControl = getTabControl();
    TabPage *pTabPage = pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >(  getActiveTabID() ) );
    if ( pTabPage )
    {
        ::Point aPos( nX, nY );
        ::Size  aSize = pTabPage->GetSizePixel();

        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        aPos  = pDev->PixelToLogic( aPos );
        aSize = pDev->PixelToLogic( aSize );

        pTabPage->Draw( pDev, aPos, aSize, 0 );
    }

    VCLXWindow::draw( nX, nY );
}

void VCLXTabControl::AddChild (uno::Reference< awt::XLayoutConstrains > const& xChild)

{
#ifndef __SUNPRO_CC
    OSL_TRACE ("%s: children: %d", __PRETTY_FUNCTION__, maChildren.size ());
#endif
    mIdMap[ xChild ] = mTabId++;
    Box_Base::AddChild( xChild );
#ifndef __SUNPRO_CC
    OSL_TRACE ("%s: children: %d", __PRETTY_FUNCTION__, maChildren.size ());
#endif
}

void SAL_CALL VCLXTabControl::addChild(
    const uno::Reference< awt::XLayoutConstrains > &xChild )
    throw (uno::RuntimeException, awt::MaxChildrenException)
{
    mIdMap[ xChild ] = insertTab();
    Box_Base::addChild( xChild );
}

void SAL_CALL VCLXTabControl::removeChild( const uno::Reference< awt::XLayoutConstrains > &xChild )
    throw (uno::RuntimeException)
{
    removeTab( mIdMap[xChild] );
    mIdMap[ xChild ] = -1;
    Box_Base::removeChild( xChild );
}

static void setChildrenVisible( uno::Reference < awt::XLayoutConstrains > xChild, bool visible )
{
    uno::Reference< awt::XWindow > xWin( xChild, uno::UNO_QUERY);
    if ( xWin.is() )
    {
        xWin->setVisible( visible );
    }

    uno::Reference < awt::XLayoutContainer > xCont( xChild, uno::UNO_QUERY );
    if ( xCont.is())
    {
        uno::Sequence< uno::Reference < awt::XLayoutConstrains > > children = xCont->getChildren();
        for ( int i = 0; i < children.getLength(); i++ )
        {
            setChildrenVisible( children[i], visible );
        }
    }
}

void SAL_CALL VCLXTabControl::allocateArea (awt::Rectangle const &area)
    throw (uno::RuntimeException)
{
#ifndef __SUNPRO_CC
    OSL_TRACE ("\n%s", __PRETTY_FUNCTION__);
#endif
    maAllocation = area;

    TabControl *pTabControl = getTabControl();

// FIXME: this is wrong. We just want to set tab controls pos/size for
// the tabs menu, otherwise, it gets events that should go to children
// (I guess we could solve this by making the tabcontrol as the actual
// XWindow parent of its children, when importing...)  Not sure about
// TabPage drawing... That doesn't work on gtk+; just ignoring that.
// LATER: Nah, the proper fix is to get the XWindow hierarchy
// straight.

#if 0
    setPosSize( area.X, area.Y, area.Width, area.Height, awt::PosSize::POSSIZE );
#else
    awt::Size currentSize = getSize();
    awt::Size requestedSize (area.Width, area.Height);
//    requestedSize.Height = getHeightForWidth( area.Width );

    awt::Size minimumSize = getMinimumSize();
    if (requestedSize.Width < minimumSize.Width)
        requestedSize.Width = minimumSize.Width;
    if (requestedSize.Height < minimumSize.Height)
        requestedSize.Height = minimumSize.Height;

    Size pageSize = static_cast<TabControl*> (GetWindow ())->GetTabPageSizePixel ();
    awt::Size pageBasedSize (0, 0);
    pageBasedSize.Width = pageSize.Width ();
    pageBasedSize.Height = pageSize.Height ();

    const int wc = 0;
    const int hc = 20;
    static int pwc = 0;
    static int phc = 40;

    if (requestedSize.Width < pageBasedSize.Width)
        requestedSize.Width = pageBasedSize.Width + wc;
    if (requestedSize.Height < pageBasedSize.Height)
        requestedSize.Height = pageBasedSize.Height + hc;

    Size windowSize = GetWindow()->GetSizePixel();
    Window *parent = GetWindow()->GetParent();
    Size parentSize = parent->GetSizePixel();

#ifndef __SUNPRO_CC
#ifdef GCC_MAJOR
    OSL_TRACE ("\n%s", __PRETTY_FUNCTION__);
#endif /* GCC_MAJOR */
    OSL_TRACE ("%s: cursize: %d ,%d", __FUNCTION__, currentSize.Width, currentSize.Height );
    OSL_TRACE ("%s: area: %d, %d", __FUNCTION__, area.Width, area.Height );
    OSL_TRACE ("%s: minimum: %d, %d", __FUNCTION__, minimumSize.Width, minimumSize.Height );
    OSL_TRACE ("%s: requestedSize: %d, %d", __FUNCTION__, requestedSize.Width, requestedSize.Height );
    OSL_TRACE ("%s: pageBasedSize: %d, %d", __FUNCTION__, pageBasedSize.Width, pageBasedSize.Height );

    //OSL_TRACE ("%s: parent: %d, %d", __FUNCTION__, parentSize.Width(), parentSize.Height() );
    //OSL_TRACE ("%s: window: %d, %d", __FUNCTION__, windowSize.Width(), windowSize.Height() );
#endif

    //bRealized = false;
    if (!bRealized)
    {
        setPosSize( area.X, area.Y, requestedSize.Width, requestedSize.Height, awt::PosSize::POSSIZE );
        bRealized = true;
    }
    else
    {
        if ( requestedSize.Width > currentSize.Width + 10)
            setPosSize( 0, 0, requestedSize.Width, 0, awt::PosSize::WIDTH );
        if ( requestedSize.Height > currentSize.Height + 10)
            setPosSize( 0, 0, 0, requestedSize.Height, awt::PosSize::HEIGHT );
    }
#endif

    if (pageBasedSize.Width > parentSize.Width ()
        || pageBasedSize.Height > parentSize.Height ())
        //parent->SetSizePixel ( Size (pageBasedSize.Width, pageBasedSize.Height));
        //parent->SetSizePixel ( Size (pageBasedSize.Width + pwc, pageBasedSize.Height + phc));
        parent->SetSizePixel ( Size (requestedSize.Width + pwc, requestedSize.Height + phc));

    // FIXME: we can save cycles by setting visibility more sensibly. Having
    // it here does makes it easier when changing tabs (just needs a recalc())
    unsigned i = 0;
    for ( std::list<Box_Base::ChildData *>::const_iterator it
              = maChildren.begin(); it != maChildren.end(); it++, i++ )
    {
        ChildData *child = static_cast<VCLXTabControl::ChildData*> ( *it );
        uno::Reference
              < awt::XLayoutConstrains > xChild( child->mxChild );
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
                ::Rectangle label_rect = pTabControl->GetTabBounds( sal::static_int_cast< sal_uInt16 >( i+1 ) );
                ::Rectangle page_rect = pTabControl->GetTabPageBounds( sal::static_int_cast< sal_uInt16 >( i+1 ) );

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

awt::Size SAL_CALL VCLXTabControl::getMinimumSize()
    throw(uno::RuntimeException)
{
    awt::Size requestedSize = VCLXWindow::getMinimumSize();
    awt::Size childrenSize( 0, 0 );

    TabControl* pTabControl = static_cast< TabControl* >( GetWindow() );
    if ( !pTabControl )
        return requestedSize;

    // calculate size to accomodate all children
    unsigned i = 0;
    for ( std::list<Box_Base::ChildData *>::const_iterator it
              = maChildren.begin(); it != maChildren.end(); it++, i++ )
    {
        ChildData *child = static_cast<VCLXTabControl::ChildData*> ( *it );
        if ( child->mxChild.is() )
        {
            // set the title prop here...
            pTabControl->SetPageText( sal::static_int_cast< sal_uInt16 >( i+1 ), child->maTitle.getStr() );

            awt::Size childSize( child->mxChild->getMinimumSize() );
            childrenSize.Width = SAL_MAX( childSize.Width, childrenSize.Width );
            childrenSize.Height = SAL_MAX( childSize.Height, childrenSize.Height );
        }
    }

#ifndef __SUNPRO_CC
#ifdef GCC_MAJOR
    OSL_TRACE ("\n%s", __PRETTY_FUNCTION__);
#endif /* GCC_MAJOR */
    OSL_TRACE ("%s: children: %d", __FUNCTION__, i);
    OSL_TRACE ("%s: childrenSize: %d, %d", __FUNCTION__, childrenSize.Width, childrenSize.Height );
#endif

    requestedSize.Width += childrenSize.Width;
    requestedSize.Height += childrenSize.Height + 20;

    maRequisition = requestedSize;
    return requestedSize;
}

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
            sal_uLong page = (sal_uLong) _rVclWindowEvent.GetData();
            for ( std::list< uno::Reference
                      < awt::XTabListener > >::iterator it
                      = mxTabListeners.begin(); it != mxTabListeners.end(); it++)
            {
                uno::Reference
                    < awt::XTabListener > listener = *it;

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
                        for ( int i = 1; i < mTabId; i++)
                        {
                            if ( pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >( i ) ) )
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

void SAL_CALL VCLXTabControl::setProperty( const ::rtl::OUString& PropertyName, const uno::Any &Value ) throw(uno::RuntimeException)
{
    VCLXWindow::setProperty( PropertyName, Value );
}

uno::Any SAL_CALL VCLXTabControl::getProperty( const ::rtl::OUString& PropertyName ) throw(uno::RuntimeException)
{
    return VCLXWindow::getProperty( PropertyName );
}

} // namespace layoutimpl
