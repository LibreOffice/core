/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
