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


#ifndef VBA_WORKBOOKS_HXX
#define VBA_WORKBOOKS_HXX


#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/XDocumentsBase.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

typedef CollTestImplHelper< ooo::vba::XDocumentsBase > VbaDocumentsBase_BASE;

class VBAHELPER_DLLPUBLIC VbaDocumentsBase : public VbaDocumentsBase_BASE
{
public:
    enum DOCUMENT_TYPE
    {
        WORD_DOCUMENT = 1,
        EXCEL_DOCUMENT
    };

private:
    DOCUMENT_TYPE meDocType;

public:
    VbaDocumentsBase( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, DOCUMENT_TYPE eDocType ) throw (css::uno::RuntimeException);
    virtual ~VbaDocumentsBase() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) = 0;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) = 0;

    // VbaDocumentsBase_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) = 0;

protected:
    css::uno::Any createDocument() throw (css::uno::RuntimeException);
    void closeDocuments() throw (css::uno::RuntimeException);
    css::uno::Any openDocument( const ::rtl::OUString& Filename, const css::uno::Any& ReadOnly, const css::uno::Sequence< css::beans::PropertyValue >& rProps ) throw (css::uno::RuntimeException);
};

#endif /* SC_VBA_WORKBOOKS_HXX */
