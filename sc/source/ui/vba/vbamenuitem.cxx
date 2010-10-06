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
#include "vbamenuitem.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaMenuItem::ScVbaMenuItem( const uno::Reference< ov::XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< XCommandBarControl >& xCommandBarControl ) throw( uno::RuntimeException ) : MenuItem_BASE( xParent, xContext ), m_xCommandBarControl( xCommandBarControl )
{
}

::rtl::OUString SAL_CALL
ScVbaMenuItem::getCaption() throw ( uno::RuntimeException )
{
    return m_xCommandBarControl->getCaption();
}

void SAL_CALL
ScVbaMenuItem::setCaption( const ::rtl::OUString& _caption ) throw (uno::RuntimeException)
{
    m_xCommandBarControl->setCaption( _caption );
}

::rtl::OUString SAL_CALL
ScVbaMenuItem::getOnAction() throw ( uno::RuntimeException )
{
    return m_xCommandBarControl->getOnAction();
}

void SAL_CALL
ScVbaMenuItem::setOnAction( const ::rtl::OUString& _onaction ) throw (uno::RuntimeException)
{
    m_xCommandBarControl->setOnAction( _onaction );
}

void SAL_CALL
ScVbaMenuItem::Delete( ) throw (script::BasicErrorException, uno::RuntimeException)
{
    m_xCommandBarControl->Delete();
}

rtl::OUString&
ScVbaMenuItem::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaMenuItem") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaMenuItem::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.MenuItem" ) );
    }
    return aServiceNames;
}
