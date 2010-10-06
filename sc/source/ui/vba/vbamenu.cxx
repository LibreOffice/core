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
#include "vbamenu.hxx"
#include "vbamenuitems.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaMenu::ScVbaMenu( const uno::Reference< ov::XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< XCommandBarControl >& xCommandBarControl ) throw( uno::RuntimeException ) : Menu_BASE( xParent, xContext ), m_xCommandBarControl( xCommandBarControl )
{
}

::rtl::OUString SAL_CALL
ScVbaMenu::getCaption() throw ( uno::RuntimeException )
{
    return m_xCommandBarControl->getCaption();
}

void SAL_CALL
ScVbaMenu::setCaption( const ::rtl::OUString& _caption ) throw (uno::RuntimeException)
{
    m_xCommandBarControl->setCaption( _caption );
}

void SAL_CALL
ScVbaMenu::Delete( ) throw (script::BasicErrorException, uno::RuntimeException)
{
    m_xCommandBarControl->Delete();
}

uno::Any SAL_CALL
ScVbaMenu::MenuItems( const uno::Any& aIndex ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< XCommandBarControls > xCommandBarControls( m_xCommandBarControl->Controls( uno::Any() ), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XMenuItems > xMenuItems( new ScVbaMenuItems( this, mxContext, xCommandBarControls ) );
    if( aIndex.hasValue() )
    {
        return xMenuItems->Item( aIndex, uno::Any() );
    }
    return uno::makeAny( xMenuItems );
}

rtl::OUString&
ScVbaMenu::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaMenu") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaMenu::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Menu" ) );
    }
    return aServiceNames;
}
