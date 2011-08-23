/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef PDFDIALOG_HXX
#define PDFDIALOG_HXX

#include "pdffilter.hxx"
#include <svtools/genericunodialog.hxx>

#include <cppuhelper/implbase2.hxx>

// -------------
// - PDFDialog -
// -------------

class Window;
class ResMgr;

typedef ::svt::OGenericUnoDialog             PDFDialog_DialogBase;
typedef ::cppu::ImplInheritanceHelper2  <
                                             PDFDialog_DialogBase,
                                             XPropertyAccess,
                                             XExporter
                                        >    PDFDialog_Base;

class PDFDialog : public PDFDialog_Base,
                  public ::comphelper::OPropertyArrayUsageHelper< PDFDialog >
{
private:
    Sequence< PropertyValue >   maMediaDescriptor;
    Sequence< PropertyValue >   maFilterData;
    Reference< XComponent >     mxSrcDoc;

protected:
    // OGenericUnoDialog
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(RuntimeException);
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);
    virtual Dialog*	createDialog( Window* pParent );
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
    virtual		~PDFDialog();
};

// -----------------------------------------------------------------------------

OUString PDFDialog_getImplementationName () throw (RuntimeException);
Sequence< sal_Int8 > SAL_CALL PDFDialog_getImplementationId() throw(RuntimeException);
Sequence< OUString > SAL_CALL PDFDialog_getSupportedServiceNames() throw (RuntimeException);
Reference< XInterface > SAL_CALL PDFDialog_createInstance( const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );

#endif // PDFDIALOG_HXX
