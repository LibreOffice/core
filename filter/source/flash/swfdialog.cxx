 /*************************************************************************
 *
 *  $RCSfile: swfdialog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 11:16:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "swfdialog.hxx"
#include "impswfdialog.hxx"
#include <vcl/svapp.hxx>
#include <vcl/dialog.hxx>
#include <svtools/solar.hrc>

#ifndef _COM_SUN_STAR_VIEW_XRENDERABLE_HPP_
#include <com/sun/star/view/XRenderable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif

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
// - PDFDialog functions -
// -----------------------

#define SERVICE_NAME "com.sun.star.Impress.FlashExportDialog"

// -----------------------------------------------------------------------------

OUString SWFDialog_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
}

// -----------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL SWFDialog_getImplementationId()
    throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL SWFDialog_getSupportedServiceNames()
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}

// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL SWFDialog_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new SWFDialog( rSMgr );
}

// -----------------------------------------------------------------------------

#undef SERVICE_NAME

// -------------
// - SWFDialog -
// -------------

SWFDialog::SWFDialog( const Reference< XMultiServiceFactory > &rxMSF ) :
    OGenericUnoDialog( rxMSF )
{
    ByteString aResMgrName( "flash" );

    aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
    mpResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );
}

// -----------------------------------------------------------------------------

SWFDialog::~SWFDialog()
{
    delete mpResMgr;
}

// -----------------------------------------------------------------------------

Any SAL_CALL SWFDialog::queryInterface( const Type& rType )
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

void SAL_CALL SWFDialog::acquire()
    throw ()
{
    OWeakObject::acquire();
}

// -----------------------------------------------------------------------------

void SAL_CALL SWFDialog::release()
    throw ()
{
    OWeakObject::release();
}

// -----------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL SWFDialog::getImplementationId()
    throw(RuntimeException)
{
    return SWFDialog_getImplementationId();
}

// -----------------------------------------------------------------------------

OUString SAL_CALL SWFDialog::getImplementationName()
    throw (RuntimeException)
{
    return SWFDialog_getImplementationName();
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL SWFDialog::getSupportedServiceNames()
    throw (RuntimeException)
{
    return SWFDialog_getSupportedServiceNames();
}

// -----------------------------------------------------------------------------

Dialog* SWFDialog::createDialog( Window* pParent )
{
    Dialog* pRet = NULL;

    if( mpResMgr && mxSrcDoc.is() )
    {
/*      TODO: From the controler we may get information what page is visible and what shapes
              are selected, if we optionaly want to limit output to that
        Any aSelection;

        try
        {
            Reference< XController > xController( Reference< XModel >( mxSrcDoc, UNO_QUERY )->getCurrentController() );

            if( xController.is() )
            {
                Reference< XSelectionSupplier > xView( xController, UNO_QUERY );

                if( xView.is() )
                    xView->getSelection() >>= aSelection;
            }
        }
        catch( RuntimeException )
        {
        }
*/
        ImpSWFDialog* pDlg = new ImpSWFDialog( pParent, *mpResMgr, maFilterData );
        pRet = pDlg;
    }

    return pRet;
}

// -----------------------------------------------------------------------------

void SWFDialog::executedDialog( sal_Int16 nExecutionResult )
{
    if( nExecutionResult && m_pDialog )
        maFilterData = static_cast< ImpSWFDialog* >( m_pDialog )->GetFilterData();

    destroyDialog();
}

// -----------------------------------------------------------------------------

Reference< XPropertySetInfo > SAL_CALL SWFDialog::getPropertySetInfo()
    throw(RuntimeException)
{
    Reference< XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& SWFDialog::getInfoHelper()
{
    return *const_cast<SWFDialog*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* SWFDialog::createArrayHelper() const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper( aProps );
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > SAL_CALL SWFDialog::getPropertyValues()
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

void SAL_CALL SWFDialog::setPropertyValues( const Sequence< PropertyValue >& rProps )
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

void SAL_CALL SWFDialog::setSourceDocument( const Reference< XComponent >& xDoc )
    throw(IllegalArgumentException, RuntimeException)
{
    mxSrcDoc = xDoc;
}
