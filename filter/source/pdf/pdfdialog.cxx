 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pdfdialog.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 16:31:21 $
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

#include "pdfdialog.hxx"
#include "impdialog.hxx"
#include <vcl/svapp.hxx>
#include <vcl/dialog.hxx>
#include <svtools/solar.hrc>

#ifndef _COM_SUN_STAR_VIEW_XRENDERABLE_HPP_
#include <com/sun/star/view/XRenderable.hpp>
#endif

using namespace ::rtl;
using namespace ::vcl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

//uncomment this to use the Tabbed PDF dialog (under development !)

// -----------------------
// - PDFDialog functions -
// -----------------------

#define SERVICE_NAME "com.sun.star.document.PDFDialog"

// -----------------------------------------------------------------------------

OUString PDFDialog_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.PDF.PDFDialog" ) );
}

// -----------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL PDFDialog_getImplementationId()
    throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL PDFDialog_getSupportedServiceNames()
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}

// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL PDFDialog_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new PDFDialog( rSMgr );
}

// -----------------------------------------------------------------------------

#undef SERVICE_NAME

// -------------
// - PDFDialog -
// -------------

PDFDialog::PDFDialog( const Reference< XMultiServiceFactory > &rxMSF ) :
    OGenericUnoDialog( rxMSF )
{
    ByteString aResMgrName( "pdffilter" );

    mpResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );
}

// -----------------------------------------------------------------------------

PDFDialog::~PDFDialog()
{
    delete mpResMgr;
}

// -----------------------------------------------------------------------------

Any SAL_CALL PDFDialog::queryInterface( const Type& rType )
    throw (RuntimeException)
{
    Any aReturn = OGenericUnoDialog::queryInterface( rType );

    if( !aReturn.hasValue() )
        aReturn = ::cppu::queryInterface( rType,
                                          static_cast< XPropertyAccess* >( this ),
                                          static_cast< XExporter* >( this ) );

    return aReturn;
}

// -----------------------------------------------------------------------------

void SAL_CALL PDFDialog::acquire()
    throw ()
{
    OWeakObject::acquire();
}

// -----------------------------------------------------------------------------

void SAL_CALL PDFDialog::release()
    throw ()
{
    OWeakObject::release();
}

// -----------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL PDFDialog::getImplementationId()
    throw(RuntimeException)
{
    return PDFDialog_getImplementationId();
}

// -----------------------------------------------------------------------------

OUString SAL_CALL PDFDialog::getImplementationName()
    throw (RuntimeException)
{
    return PDFDialog_getImplementationName();
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL PDFDialog::getSupportedServiceNames()
    throw (RuntimeException)
{
    return PDFDialog_getSupportedServiceNames();
}

// -----------------------------------------------------------------------------

Dialog* PDFDialog::createDialog( Window* pParent )
{
    Dialog* pRet = NULL;

    if( mpResMgr && mxSrcDoc.is() )
    {
        ImpPDFTabDialog* pDlg = new ImpPDFTabDialog( pParent, *mpResMgr, maFilterData, mxSrcDoc, m_xORB );
        pRet = pDlg;
    }

    return pRet;
}

// -----------------------------------------------------------------------------

void PDFDialog::executedDialog( sal_Int16 nExecutionResult )
{
    if( nExecutionResult && m_pDialog )
        maFilterData = static_cast< ImpPDFTabDialog* >( m_pDialog )->GetFilterData();
    destroyDialog();
}

// -----------------------------------------------------------------------------

Reference< XPropertySetInfo > SAL_CALL PDFDialog::getPropertySetInfo()
    throw(RuntimeException)
{
    Reference< XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& PDFDialog::getInfoHelper()
{
    return *const_cast<PDFDialog*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* PDFDialog::createArrayHelper() const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper( aProps );
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > SAL_CALL PDFDialog::getPropertyValues()
    throw ( RuntimeException )
{
    sal_Int32 i, nCount;

    for( i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if( maMediaDescriptor[ i ].Name.equalsAscii( "FilterData" ) )
            break;
    }

    if( i == nCount )
        maMediaDescriptor.realloc( ++nCount );

    maMediaDescriptor[ i ].Name = String( RTL_CONSTASCII_USTRINGPARAM( "FilterData" ) );
    maMediaDescriptor[ i ].Value <<= maFilterData;

    return maMediaDescriptor;
}

// -----------------------------------------------------------------------------

void SAL_CALL PDFDialog::setPropertyValues( const Sequence< PropertyValue >& rProps )
    throw ( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    maMediaDescriptor = rProps;

    for( sal_Int32 i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if( maMediaDescriptor[ i ].Name.equalsAscii( "FilterData" ) )
        {
            maMediaDescriptor[ i ].Value >>= maFilterData;
            break;
        }
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL PDFDialog::setSourceDocument( const Reference< XComponent >& xDoc )
    throw(IllegalArgumentException, RuntimeException)
{
    mxSrcDoc = xDoc;
}
