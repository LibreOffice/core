/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svgdialog.hxx,v $
 *
 *  $Revision: 1.1.2.1 $
 *
 *  last change: $Author: ka $ $Date: 2006/03/08 08:18:10 $
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

#ifndef SVGDIALOG_HXX
#define SVGDIALOG_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif

#include <svtools/genericunodialog.hxx>
#include <memory>

// -------------
// - SVGDialog -
// -------------

class ResMgr;

class SVGDialog : public ::svt::OGenericUnoDialog,
                  public ::comphelper::OPropertyArrayUsageHelper< SVGDialog >,
                  public ::com::sun::star::beans::XPropertyAccess,
                  public ::com::sun::star::document::XExporter
{
private:

    ::std::auto_ptr< ResMgr >                                               mapResMgr;
    com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > maMediaDescriptor;
    com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > maFilterData;
    com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >    mxSrcDoc;

protected:

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL queryInterface( const com::sun::star::uno::Type& aType ) throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // OGenericUnoDialog
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getImplementationName() throw (com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);
    virtual Dialog* createDialog( Window* pParent );
    virtual void executedDialog( sal_Int16 nExecutionResult );
    virtual com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // XPropertyAccess
    using cppu::OPropertySetHelper::getPropertyValues;
    virtual com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPropertyValues(  ) throw (com::sun::star::uno::RuntimeException);
    using cppu::OPropertySetHelper::setPropertyValues;
    virtual void SAL_CALL setPropertyValues( const com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps ) throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::beans::PropertyVetoException, com::sun::star::lang::IllegalArgumentException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException);

       // XExporter
    virtual void SAL_CALL setSourceDocument( const com::sun::star::uno::Reference< com::sun::star::lang::XComponent >& xDoc ) throw(com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);

public:

                SVGDialog( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    virtual     ~SVGDialog();
};

// -----------------------------------------------------------------------------

::rtl::OUString SVGDialog_getImplementationName ()
    throw ( ::com::sun::star::uno::RuntimeException );

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL SVGDialog_supportsService( const ::rtl::OUString& ServiceName )
    throw ( ::com::sun::star::uno::RuntimeException );

// -----------------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL SVGDialog_getSupportedServiceNames(  )
    throw ( ::com::sun::star::uno::RuntimeException );

// -----------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    SAL_CALL SVGDialog_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr)
        throw ( ::com::sun::star::uno::Exception );

#endif // SVGDialog_HXX
