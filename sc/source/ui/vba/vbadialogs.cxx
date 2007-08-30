/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbadialogs.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-30 10:04:20 $
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
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <org/openoffice/excel/XApplication.hpp>
#include <org/openoffice/excel/XDialog.hpp>

#include <tools/string.hxx>

#include "vbahelper.hxx"
#include "vbadialogs.hxx"
#include "vbaglobals.hxx"
#include "vbadialog.hxx"

using namespace ::org::openoffice;
using namespace ::com::sun::star;

void
ScVbaDialogs::Dummy() throw (uno::RuntimeException)
{
}

uno::Any
ScVbaDialogs::getParent() throw (uno::RuntimeException)
{
    uno::Reference< vba::XGlobals > xGlobals = ScVbaGlobals::getGlobalsImpl( m_xContext );
    uno::Reference< excel::XApplication > xApplication = xGlobals->getApplication();
    if ( !xApplication.is() )
    {
        throw uno::RuntimeException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ScVbaWorkbooks::getParent: Couldn't access Application               object") ),uno::Reference< XInterface >() );
    }
    return uno::Any(xApplication);
}

::sal_Int32
ScVbaDialogs::getCreator() throw (uno::RuntimeException)
{
    // #FIXME #TODO
    return 0;
}
uno::Reference< excel::XApplication >
ScVbaDialogs::getApplication() throw (uno::RuntimeException)
{
    uno::Reference< excel::XApplication > xApplication =  ScVbaGlobals::getGlobalsImpl( m_xContext )->getApplication();
    if ( !xApplication.is() )
    {
        throw uno::RuntimeException(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ScVbaWorkbooks::getParent: Couldn't access Application object" ) ),
        uno::Reference< XInterface >() );
    }
    return xApplication;
}

::sal_Int32
ScVbaDialogs::getCount() throw (uno::RuntimeException)
{
    //#TODO #FIXEME
    //SC_VBA_STUB();
    return 0;
}

uno::Any
ScVbaDialogs::Item( const uno::Any &aItem ) throw (uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    aItem >>= nIndex;
    uno::Reference< excel::XDialog > aDialog( new ScVbaDialog( nIndex, m_xContext ) );
    return uno::Any( aDialog );
}
