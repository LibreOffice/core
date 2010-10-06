/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
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
#include "vbamenubar.hxx"
#include "vbamenus.hxx"
#include <ooo/vba/XCommandBarControls.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaMenuBar::ScVbaMenuBar( const uno::Reference< ov::XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< XCommandBar >& xCommandBar ) throw( uno::RuntimeException ) : MenuBar_BASE( xParent, xContext ), m_xCommandBar( xCommandBar )
{
}

uno::Any SAL_CALL
ScVbaMenuBar::Menus( const uno::Any& aIndex ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< XCommandBarControls > xCommandBarControls( m_xCommandBar->Controls( uno::Any() ), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XMenus > xMenus( new ScVbaMenus( this, mxContext, xCommandBarControls ) );
    if( aIndex.hasValue() )
    {
        return xMenus->Item( aIndex, uno::Any() );
    }
    return uno::makeAny( xMenus );
}

rtl::OUString&
ScVbaMenuBar::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaMenuBar") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaMenuBar::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.MenuBar" ) );
    }
    return aServiceNames;
}
