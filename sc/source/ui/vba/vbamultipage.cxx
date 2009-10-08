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
#include "vbamultipage.hxx"
#include <ooo/vba/XCollection.hpp>
#include "vbapages.hxx"
#include <vector>

using namespace com::sun::star;
using namespace ooo::vba;

// uno servicename com.sun.star.awt.UnoControlProgressBarMode
const rtl::OUString SVALUE( RTL_CONSTASCII_USTRINGPARAM("ProgressValue") );
const rtl::OUString SVALUEMAX( RTL_CONSTASCII_USTRINGPARAM("ProgressValueMax") );
const rtl::OUString SSTEP( RTL_CONSTASCII_USTRINGPARAM("Step") );

typedef cppu::WeakImplHelper1< container::XIndexAccess > PagesImpl_Base;
class PagesImpl : public PagesImpl_Base
{
    sal_Int32 mnPages;
public:
    PagesImpl( sal_Int32 nPages ) : mnPages( nPages ) {}
    virtual ::sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException) { return mnPages; }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, ::uno::RuntimeException)
    {
        if ( Index < 0 || Index > mnPages )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( uno::Reference< uno::XInterface >() );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException)
    {
        // no Pages object yet #FIXME
        //return msforms::XPage::static_type(0);
        return uno::XInterface::static_type(0);
    }
    virtual ::sal_Bool SAL_CALL hasElements( ) throw (uno::RuntimeException)
    {
        return ( mnPages > 0 );
    }
};
uno::Reference< container::XIndexAccess >
ScVbaMultiPage::getPages( sal_Int32 nPages )
{
    return new PagesImpl( nPages );
}

ScVbaMultiPage::ScVbaMultiPage( const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper, const uno::Reference< uno::XInterface >& xDialog ) : MultiPageImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
    mxDialogProps.set( xDialog, uno::UNO_QUERY_THROW );
    // set dialog step to value of multipage pseudo model
    setValue(getValue());
}

// Attributes
sal_Int32 SAL_CALL
ScVbaMultiPage::getValue() throw (css::uno::RuntimeException)
{
    sal_Int32 nValue = 0;
    m_xProps->getPropertyValue( SVALUE ) >>= nValue;
    return nValue;
}

void SAL_CALL
ScVbaMultiPage::setValue( const sal_Int32 _value ) throw (::com::sun::star::uno::RuntimeException)
{
    // track change in dialog ( dialog value is 1 based, 0 is a special value )
    m_xProps->setPropertyValue( SVALUE, uno::makeAny( _value ) );
    mxDialogProps->setPropertyValue( SSTEP, uno::makeAny( _value + 1) );
}


rtl::OUString&
ScVbaMultiPage::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaMultiPage") );
    return sImplName;
}

uno::Any SAL_CALL
ScVbaMultiPage::Pages( const uno::Any& index ) throw (uno::RuntimeException)
{
    sal_Int32 nValue = 0;
    m_xProps->getPropertyValue( SVALUEMAX ) >>= nValue;
    uno::Reference< XCollection > xColl( new ScVbaPages( this, mxContext, getPages( nValue ) ) );
    if ( !index.hasValue() )
        return uno::makeAny( xColl );
    return xColl->Item( uno::makeAny( index ), uno::Any() );
}

uno::Sequence< rtl::OUString >
ScVbaMultiPage::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msforms.MultiPage" ) );
    }
    return aServiceNames;
}
