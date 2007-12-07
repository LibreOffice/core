/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbapane.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:58:05 $
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
#include<com/sun/star/table/CellRangeAddress.hpp>
#include<vbapane.hxx>

using namespace com::sun::star;
using namespace org::openoffice;

/*
ScVbaPane::ScVbaPane( uno::Reference< uno::XComponentContext > xContext, uno::Refrence< sheet::XViewPane > xViewPane )
        : m_xContext( xContext ), m_xViewPane( xViewPane )
{
}
*/

sal_Int32 SAL_CALL
ScVbaPane::getScrollColumn() throw (uno::RuntimeException)
{
    return ( m_xViewPane->getFirstVisibleColumn() + 1 );
}

void SAL_CALL
ScVbaPane::setScrollColumn( sal_Int32 _scrollcolumn ) throw (uno::RuntimeException)
{
    if( _scrollcolumn < 1 )
    {
        throw uno::RuntimeException( rtl::OUString::createFromAscii( "Column number should not less than 1" ),
                uno::Reference< uno::XInterface >() );
    }
    m_xViewPane->setFirstVisibleColumn( _scrollcolumn - 1 );
}

sal_Int32 SAL_CALL
ScVbaPane::getScrollRow() throw (uno::RuntimeException)
{
    return ( m_xViewPane->getFirstVisibleRow() + 1 );
}

void SAL_CALL
ScVbaPane::setScrollRow( sal_Int32 _scrollrow ) throw (uno::RuntimeException)
{
    if( _scrollrow < 1 )
    {
        throw uno::RuntimeException( rtl::OUString::createFromAscii( "Row number should not less than 1" ),
                uno::Reference< uno::XInterface >() );
    }
    m_xViewPane->setFirstVisibleRow( _scrollrow - 1 );
}

//Method
void SAL_CALL
ScVbaPane::SmallScroll( const uno::Any& Down, const uno::Any& Up, const uno::Any& ToRight, const uno::Any& ToLeft ) throw (uno::RuntimeException)
{
    rtl::OUString messageBuffer;
    sal_Int32 downRows = 0;
    sal_Int32 rightCols = 0;
    table::CellRangeAddress visibleRange = m_xViewPane->getVisibleRange();

    if( Down.hasValue() )
    {
        sal_Int32 down = 0;
        try
        {
            Down >>= down;
            downRows += down;
        }
        catch ( uno::Exception )
        {
            messageBuffer += rtl::OUString::createFromAscii( "Error getting parameter: Down\n" );
        }
    }
    if( Up.hasValue() )
    {
        sal_Int32 up = 0;
        try
        {
            Up >>= up;
            downRows -= up;
        }
        catch ( uno::Exception )
        {
            messageBuffer += rtl::OUString::createFromAscii( "Error getting parameter: Up\n" );
        }
    }
    if( ToRight.hasValue() )
    {
        sal_Int32 right = 0;
        try
        {
            ToRight >>= right;
            rightCols += right;
        }
        catch ( uno::Exception )
        {
            messageBuffer += rtl::OUString::createFromAscii( "Error getting parameter: ToRight\n" );
        }
    }
    if( ToLeft.hasValue() )
    {
        sal_Int32 left = 0;
        try
        {
            ToLeft >>= left;
            rightCols -= left;
        }
        catch ( uno::Exception )
        {
            messageBuffer += rtl::OUString::createFromAscii( "Error getting parameter: ToLeft\n" );
        }
    }
    if( messageBuffer.getLength() > 0 )
        throw(uno::RuntimeException( messageBuffer, uno::Reference< uno::XInterface >() ) );

    sal_Int32 newStartRow = visibleRange.StartRow + downRows;
    if( newStartRow < 0 )
        newStartRow = 0;
    sal_Int32 newStartCol = visibleRange.StartColumn + rightCols;
    if( newStartCol < 0 )
        newStartCol = 0;
    m_xViewPane->setFirstVisibleRow( newStartRow );
    m_xViewPane->setFirstVisibleColumn( newStartCol );
}

void SAL_CALL
ScVbaPane::LargeScroll( const uno::Any& Down, const uno::Any& Up, const uno::Any& ToRight, const uno::Any& ToLeft ) throw (uno::RuntimeException)
{
    rtl::OUString messageBuffer;
    table::CellRangeAddress visibleRange = m_xViewPane->getVisibleRange();

    sal_Int32 vertPageSize = 1 + visibleRange.EndRow - visibleRange.StartRow;
    sal_Int32 horizPageSize = 1 + visibleRange.EndColumn - visibleRange.StartColumn;
    sal_Int32 downPages = 0;
    sal_Int32 acrossPages = 0;
    if( Down.hasValue() )
    {
        sal_Int32 down = 0;
        try
        {
            Down >>= down;
            downPages += down;
        }
        catch ( uno::Exception )
        {
            messageBuffer += rtl::OUString::createFromAscii( "Error getting parameter: Down\n" );
        }
    }
    if( Up.hasValue() )
    {
        sal_Int32 up = 0;
        try
        {
            Up >>= up;
            downPages -= up;
        }
        catch ( uno::Exception )
        {
            messageBuffer += rtl::OUString::createFromAscii( "Error getting parameter: Up\n" );
        }
    }
    if( ToRight.hasValue() )
    {
        sal_Int32 right = 0;
        try
        {
            ToRight >>= right;
            acrossPages += right;
        }
        catch ( uno::Exception )
        {
            messageBuffer += rtl::OUString::createFromAscii( "Error getting parameter: ToRight\n" );
        }
    }
    if( ToLeft.hasValue() )
    {
        sal_Int32 left = 0;
        try
        {
            ToLeft >>= left;
            acrossPages -= left;
        }
        catch ( uno::Exception )
        {
            messageBuffer += rtl::OUString::createFromAscii( "Error getting parameter: ToLeft\n" );
        }
    }

    if( messageBuffer.getLength() > 0 )
        throw(uno::RuntimeException( messageBuffer, uno::Reference< uno::XInterface >() ) );

    sal_Int32 newStartRow = visibleRange.StartRow + (downPages * vertPageSize );
    if( newStartRow < 0 )
        newStartRow = 0;
    sal_Int32 newStartCol = visibleRange.StartColumn + (acrossPages * horizPageSize );
    if( newStartCol < 0 )
        newStartCol = 0;
    m_xViewPane->setFirstVisibleRow( newStartRow );
    m_xViewPane->setFirstVisibleColumn( newStartCol );
}
