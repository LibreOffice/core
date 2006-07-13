/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pdfdialog.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 11:13:33 $
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

#ifndef PDFDIALOG_HXX
#define PDFDIALOG_HXX

#include "pdffilter.hxx"
#include <svtools/genericunodialog.hxx>

// -------------
// - PDFDialog -
// -------------

class Window;
class ResMgr;

class PDFDialog : public ::svt::OGenericUnoDialog,
                  public ::comphelper::OPropertyArrayUsageHelper< PDFDialog >,
                  public XPropertyAccess,
                  public XExporter
{
private:

    ResMgr*                     mpResMgr;
    Sequence< PropertyValue >   maMediaDescriptor;
    Sequence< PropertyValue >   maFilterData;
    Reference< XComponent >     mxSrcDoc;

protected:

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& aType ) throw (RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // OGenericUnoDialog
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(RuntimeException);
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);
    virtual Dialog* createDialog( Window* pParent );
    virtual void executedDialog( sal_Int16 nExecutionResult );
    virtual Reference< XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // XPropertyAccess
    using OPropertySetHelper::getPropertyValues;
    virtual Sequence< PropertyValue > SAL_CALL getPropertyValues(  ) throw (RuntimeException);
    using OPropertySetHelper::setPropertyValues;
    virtual void SAL_CALL setPropertyValues( const Sequence< PropertyValue >& aProps ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException);

       // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException);

public:

                PDFDialog( const Reference< XMultiServiceFactory >& rxMSF );
    virtual     ~PDFDialog();
};

// -----------------------------------------------------------------------------

OUString PDFDialog_getImplementationName () throw (RuntimeException);
Sequence< sal_Int8 > SAL_CALL PDFDialog_getImplementationId() throw(RuntimeException);
Sequence< OUString > SAL_CALL PDFDialog_getSupportedServiceNames() throw (RuntimeException);
Reference< XInterface > SAL_CALL PDFDialog_createInstance( const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );

#endif // PDFDIALOG_HXX
