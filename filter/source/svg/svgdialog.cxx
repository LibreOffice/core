 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svgdialog.cxx,v $
 *
 *  $Revision: 1.1.2.3 $
 *
 *  last change: $Author: ka $ $Date: 2008/05/19 10:12:43 $
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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "svgdialog.hxx"
#include "impsvgdialog.hxx"

#include <vcl/svapp.hxx>
#include <vcl/dialog.hxx>

#ifndef _COM_SUN_STAR_VIEW_XRENDERABLE_HPP_
#include <com/sun/star/view/XRenderable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif

#define SVG_DIALOG_SERVICE_NAME         "com.sun.star.comp.Draw.SVGFilterDialog"
#define SVG_DIALOG_IMPLEMENTATION_NAME  SVG_DIALOG_SERVICE_NAME
#define SVG_FILTER_DATA_NAME            "FilterData"

using namespace ::rtl;
using namespace ::vcl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::document;

// -----------------------
// - SVGDialog functions -
// -----------------------

OUString SVGDialog_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( SVG_DIALOG_IMPLEMENTATION_NAME ) );
}

// -----------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL SVGDialog_getImplementationId()
    throw(RuntimeException)
{
    static const ::cppu::OImplementationId aId;

    return( aId.getImplementationId() );
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL SVGDialog_getSupportedServiceNames()
    throw (RuntimeException)
{
    Sequence< OUString > aRet( 1 );

    aRet.getArray()[ 0 ] = OUString ( RTL_CONSTASCII_USTRINGPARAM ( SVG_DIALOG_SERVICE_NAME ) );

    return aRet;
}

// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL SVGDialog_createInstance( const Reference< XMultiServiceFactory > & rSMgr )
    throw( Exception )
{
    return( static_cast< cppu::OWeakObject* >( new SVGDialog( rSMgr ) ) );
}

// -------------
// - SVGDialog -
// -------------

SVGDialog::SVGDialog( const Reference< XMultiServiceFactory > &rxMSF ) :
    OGenericUnoDialog( rxMSF )
{
    ByteString aResMgrName( "svgfilter" );

    aResMgrName.Append( ByteString::CreateFromInt32( 0/*SOLARUPD*/ ) );

    // !!! KA: enable ResMgr
    // mapResMgr.reset( ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() ) );
}

// -----------------------------------------------------------------------------

SVGDialog::~SVGDialog()
{
}

// -----------------------------------------------------------------------------

Any SAL_CALL SVGDialog::queryInterface( const Type& rType )
    throw (RuntimeException)
{
    Any aReturn( OGenericUnoDialog::queryInterface( rType ) );

    if( !aReturn.hasValue() )
    {
        aReturn = ::cppu::queryInterface( rType, static_cast< XPropertyAccess* >( this ),
                                                 static_cast< XExporter* >( this ) );
    }

    return( aReturn );
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGDialog::acquire()
    throw ()
{
    OWeakObject::acquire();
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGDialog::release()
    throw ()
{
    OWeakObject::release();
}

// -----------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL SVGDialog::getImplementationId()
    throw(RuntimeException)
{
    return SVGDialog_getImplementationId();
}

// -----------------------------------------------------------------------------

OUString SAL_CALL SVGDialog::getImplementationName()
    throw (RuntimeException)
{
    return SVGDialog_getImplementationName();
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL SVGDialog::getSupportedServiceNames()
    throw (RuntimeException)
{
    return SVGDialog_getSupportedServiceNames();
}

// -----------------------------------------------------------------------------

Dialog* SVGDialog::createDialog( Window* pParent )
{
    return( ( /*KA: *mapResMgr.get() &&*/ mxSrcDoc.is() ) ?
            new ImpSVGDialog( pParent/*KA: , *mapResMgr*/, maFilterData ) :
            NULL );
}

// -----------------------------------------------------------------------------

void SVGDialog::executedDialog( sal_Int16 nExecutionResult )
{
    if( nExecutionResult && m_pDialog )
        maFilterData = static_cast< ImpSVGDialog* >( m_pDialog )->GetFilterData();

    destroyDialog();
}

// -----------------------------------------------------------------------------

Reference< XPropertySetInfo > SAL_CALL SVGDialog::getPropertySetInfo()
    throw(RuntimeException)
{
    return( Reference< XPropertySetInfo >( createPropertySetInfo( getInfoHelper() ) ) );
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& SVGDialog::getInfoHelper()
{
    return( *const_cast< SVGDialog *>(this)->getArrayHelper() );
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* SVGDialog::createArrayHelper() const
{
    Sequence< Property > aProps;

    describeProperties(aProps);

    return new ::cppu::OPropertyArrayHelper( aProps );
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > SAL_CALL SVGDialog::getPropertyValues()
    throw ( RuntimeException )
{
    sal_Int32 i, nCount;

    for( i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; ++i )
    {
        if( maMediaDescriptor[ i ].Name.equalsAscii( SVG_FILTER_DATA_NAME ) )
            break;
    }

    if( i == nCount )
    {
        maMediaDescriptor.realloc( ++nCount );
        maMediaDescriptor[ i ].Name = String( RTL_CONSTASCII_USTRINGPARAM( SVG_FILTER_DATA_NAME ) );
    }

    maMediaDescriptor[ i ].Value <<= maFilterData;

    return( maMediaDescriptor );
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGDialog::setPropertyValues( const Sequence< PropertyValue >& rProps )
    throw ( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    maMediaDescriptor = rProps;

    for( sal_Int32 i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if( maMediaDescriptor[ i ].Name.equalsAscii( SVG_FILTER_DATA_NAME ) )
        {
            maMediaDescriptor[ i ].Value >>= maFilterData;
            break;
        }
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGDialog::setSourceDocument( const Reference< XComponent >& xDoc )
    throw(IllegalArgumentException, RuntimeException)
{
    mxSrcDoc = xDoc;
}
