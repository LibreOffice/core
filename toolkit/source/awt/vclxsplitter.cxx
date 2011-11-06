/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "vclxsplitter.hxx"

#include <assert.h>
#include <com/sun/star/awt/PosSize.hpp>
#include <sal/macros.h>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/split.hxx>

#include "forward.hxx"

namespace layoutimpl
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;

VCLXSplitter::ChildProps::ChildProps( VCLXSplitter::ChildData *pData )
{
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Shrink" ),
             ::getCppuType( static_cast< const rtl::OUString* >( NULL ) ),
             &(pData->mbShrink) );
}

VCLXSplitter::ChildData::ChildData( uno::Reference< awt::XLayoutConstrains > const& xChild )
    : Box_Base::ChildData( xChild )
    , mbShrink( false )
{
}

VCLXSplitter::ChildData*
VCLXSplitter::createChild( uno::Reference< awt::XLayoutConstrains > const& xChild )
{
    return new ChildData( xChild );
}

VCLXSplitter::ChildProps*
VCLXSplitter::createChildProps( Box_Base::ChildData *pData )
{
    return new ChildProps( static_cast<VCLXSplitter::ChildData*> ( pData ) );
}


DBG_NAME( VCLXSplitter );

VCLXSplitter::VCLXSplitter( bool bHorizontal )
    : VCLXWindow()
    , Box_Base()
{
    DBG_CTOR( VCLXSplitter, NULL );
    mnHandleRatio = 0.5;
    mbHandlePressed = false;
    mbHorizontal = bHorizontal;
    mpSplitter = NULL;
}

VCLXSplitter::~VCLXSplitter()
{
    DBG_DTOR( VCLXSplitter, NULL );
}

IMPLEMENT_2_FORWARD_XINTERFACE1( VCLXSplitter, VCLXWindow, Container );

IMPLEMENT_FORWARD_XTYPEPROVIDER1( VCLXSplitter, VCLXWindow );

VCLXSplitter::ChildData*
VCLXSplitter::getChild( int i )
{
    if ( maChildren.size() && i == 0 )
        return static_cast<VCLXSplitter::ChildData*>( maChildren.front() );
    else if ( maChildren.size() > 1 && i == 1 )
        return static_cast<VCLXSplitter::ChildData*>( maChildren.back() );
    return 0;
}

void SAL_CALL VCLXSplitter::dispose() throw(RuntimeException)
{
    {
        ::vos::OGuard aGuard( GetMutex() );

        EventObject aDisposeEvent;
        aDisposeEvent.Source = W3K_EXPLICIT_CAST (*this);
//            maTabListeners.disposeAndClear( aDisposeEvent );
    }

    VCLXWindow::dispose();
}

void VCLXSplitter::ensureSplitter()
{
    if ( !mpSplitter )
    {
        mpSplitter = new Splitter( GetWindow() , mbHorizontal ? WB_HORZ : WB_VERT );
        mpSplitter->Show();
        mpSplitter->SetEndSplitHdl( LINK( this, VCLXSplitter, HandleMovedHdl ) );
    }
}

void SAL_CALL VCLXSplitter::addChild(
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains > &xChild )
    throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::awt::MaxChildrenException)
{
    if ( maChildren.size() == 2 )
        throw css::awt::MaxChildrenException();
    Box_Base::addChild( xChild );
}

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

    assert( mpSplitter );
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

    if ( getChild( 0 ) && getChild( 0 )->mxChild.is() )
    {
        awt::Rectangle childRect( 0, 0, rArea.Width, rArea.Height );

        if ( mbHorizontal )
            childRect.Width = leftWidth - 2;
        else
            childRect.Height = leftWidth - 2;
        allocateChildAt( getChild( 0 )->mxChild, childRect );
    }
    if ( getChild( 0 ) && getChild( 0 )->mxChild.is() )
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
        allocateChildAt( getChild( 1 )->mxChild, childRect );
    }
}

::com::sun::star::awt::Size SAL_CALL VCLXSplitter::getMinimumSize()
    throw(::com::sun::star::uno::RuntimeException)
{
    ensureSplitter();

    awt::Size size( mbHorizontal ? 2 : 0, mbHorizontal ? 0 : 2 );
    for ( unsigned int i = 0; i < 2; i++ )
    {
        if ( getChild( i ) && getChild( i )->mxChild.is() )
        {
            awt::Size childSize = getChild( i )->mxChild->getMinimumSize();
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

void VCLXSplitter::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
{
    VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
}

void SAL_CALL VCLXSplitter::setProperty( const ::rtl::OUString& PropertyName, const Any &Value ) throw(RuntimeException)
{
    VCLXWindow::setProperty( PropertyName, Value );
}

Any SAL_CALL VCLXSplitter::getProperty( const ::rtl::OUString& PropertyName ) throw(RuntimeException)
{
    return VCLXWindow::getProperty( PropertyName );
}

IMPL_LINK( VCLXSplitter, HandleMovedHdl, Splitter *, pSplitter )
{
    (void) pSplitter;
    forceRecalc();
    return 0;
}

} // namespace layoutimpl
