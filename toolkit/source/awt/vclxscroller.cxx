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



#include "vclxscroller.hxx"

#include <assert.h>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <sal/macros.h>
#include <toolkit/helper/property.hxx>
#include <tools/debug.hxx>
#include <vcl/scrbar.hxx>

#include "forward.hxx"

namespace layoutimpl
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;

DBG_NAME( VCLXScroller )

VCLXScroller::VCLXScroller()
  : VCLXWindow()
  , Bin()
{
    DBG_CTOR( VCLXScroller, NULL );
    mpHorScrollBar = mpVerScrollBar = 0;
}

VCLXScroller::~VCLXScroller()
{
    DBG_DTOR( VCLXScroller, NULL );
}

IMPLEMENT_2_FORWARD_XINTERFACE1( VCLXScroller, VCLXWindow, Container );

IMPLEMENT_FORWARD_XTYPEPROVIDER1( VCLXScroller, VCLXWindow );

void SAL_CALL VCLXScroller::dispose() throw(RuntimeException)
{
    {
        ::vos::OGuard aGuard( GetMutex() );

        EventObject aDisposeEvent;
        aDisposeEvent.Source = W3K_EXPLICIT_CAST (*this);
//            maTabListeners.disposeAndClear( aDisposeEvent );
    }

    VCLXWindow::dispose();
}

void VCLXScroller::ensureScrollBars()
{

    if ( !mpVerScrollBar )
    {
        mpVerScrollBar = new ScrollBar( GetWindow() , WB_VERT );
        mpVerScrollBar->SetLineSize( 4 );
        mpVerScrollBar->SetPageSize( 15 );
        mpVerScrollBar->Show();
        mpVerScrollBar->SetScrollHdl( LINK( this, VCLXScroller, ScrollHdl ) );
    }
    if ( !mpHorScrollBar )
    {
        mpHorScrollBar = new ScrollBar( GetWindow() , WB_HORZ );
        mpHorScrollBar->SetLineSize( 4 );
        mpHorScrollBar->SetPageSize( 15 );
        mpHorScrollBar->Show();
        mpHorScrollBar->SetScrollHdl( LINK( this, VCLXScroller, ScrollHdl ) );
    } //        mpContent = new FixedImage( this, ImplGetWinBits( WindowAttributes, 0 ) );

}

void SAL_CALL VCLXScroller::allocateArea(
    const ::com::sun::star::awt::Rectangle &rArea )
    throw (::com::sun::star::uno::RuntimeException)
{
    ensureScrollBars();        // shouldn't be needed

    maAllocation = rArea;
    setPosSize( rArea.X, rArea.Y, rArea.Width, rArea.Height, PosSize::POSSIZE );

    mpHorScrollBar->SetRangeMin( 0 );
    mpHorScrollBar->SetRangeMax( SAL_MAX( maChildRequisition.Width - rArea.Width, 0 ) );
    mpVerScrollBar->SetRangeMin( 0 );
    mpVerScrollBar->SetRangeMax( SAL_MAX( maChildRequisition.Height - rArea.Height, 0 ) );

    int thumbX = mpHorScrollBar->GetThumbPos();
    int thumbY = mpVerScrollBar->GetThumbPos();
    int thumbWidth = mpVerScrollBar->GetSizePixel().getWidth();
    int thumbHeight = mpHorScrollBar->GetSizePixel().getHeight();

    mpHorScrollBar->SetPosSizePixel( rArea.X, rArea.Y + rArea.Height - thumbHeight - 2,
                                     rArea.Width - thumbWidth, thumbHeight );
    mpVerScrollBar->SetPosSizePixel( rArea.X + rArea.Width - thumbWidth - 2, rArea.Y-2,
                                     thumbWidth, rArea.Height - thumbHeight );

    awt::Rectangle childRect( rArea.X - thumbX, rArea.Y - thumbY,
                              SAL_MAX( maChildRequisition.Width, rArea.Width ) - thumbWidth - 4,
                              SAL_MAX( maChildRequisition.Height, rArea.Height ) - thumbHeight - 4 );
    if ( mxChild.is() )
        allocateChildAt( mxChild, childRect );
}

#define MAX_CHILD_REQ 40
::com::sun::star::awt::Size SAL_CALL VCLXScroller::getMinimumSize()
    throw(::com::sun::star::uno::RuntimeException)
{
    ensureScrollBars();
    assert( mpHorScrollBar && mpVerScrollBar );
    awt::Size childSize = Bin::getMinimumSize();
    int thumbWidth = mpVerScrollBar->GetSizePixel().getWidth();
    int thumbHeight = mpHorScrollBar->GetSizePixel().getHeight();
    maRequisition = awt::Size(
        SAL_MIN( MAX_CHILD_REQ, childSize.Width ) + thumbWidth,
        SAL_MIN( MAX_CHILD_REQ, childSize.Height ) + thumbHeight );
    return maRequisition;
}

void VCLXScroller::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
{
/*
    ::vos::OClearableGuard aGuard( GetMutex() );

    switch ( _rVclWindowEvent.GetId() )
    {
        default:
            aGuard.clear();
*/
            VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
/*
        break;
    }
*/
}

void SAL_CALL VCLXScroller::setProperty( const ::rtl::OUString& PropertyName, const Any &Value ) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
/*
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
*/
                VCLXWindow::setProperty( PropertyName, Value );
/*
        }
*/
    }
}

Any SAL_CALL VCLXScroller::getProperty( const ::rtl::OUString& PropertyName ) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Any aReturn;
    if ( GetWindow() )
    {
/*
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
*/
                aReturn = VCLXWindow::getProperty( PropertyName );

//        }
    }
    return aReturn;
}

IMPL_LINK( VCLXScroller, ScrollHdl, ScrollBar *, pScrollBar )
{
    (void) pScrollBar;
    forceRecalc();
    return 0;
}

} // namespace layoutimpl
