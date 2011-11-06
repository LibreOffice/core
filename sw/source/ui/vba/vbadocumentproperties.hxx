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


#ifndef SW_VBA_DOCUMENTPROPERTIES_HXX
#define SW_VBA_DOCUMENTPROPERTIES_HXX

#include <ooo/vba/XDocumentProperties.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <vbahelper/vbacollectionimpl.hxx>
#include <hash_map>

typedef CollTestImplHelper< ov::XDocumentProperties > SwVbaDocumentproperties_BASE;

class SwVbaBuiltinDocumentProperties : public SwVbaDocumentproperties_BASE
{
protected:
    css::uno::Reference< css::frame::XModel > m_xModel;
public:
    SwVbaBuiltinDocumentProperties( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::frame::XModel >& xDocument );

    // XDocumentProperties
    virtual css::uno::Reference< ::ooo::vba::XDocumentProperty > SAL_CALL Add( const ::rtl::OUString& Name, ::sal_Bool LinkToContent, ::sal_Int8 Type, const css::uno::Any& Value, const css::uno::Any& LinkSource ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

class SwVbaCustomDocumentProperties : public SwVbaBuiltinDocumentProperties
{
public:
    SwVbaCustomDocumentProperties( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::frame::XModel >& xDocument );
// XDocumentProperties
    virtual css::uno::Reference< ::ooo::vba::XDocumentProperty > SAL_CALL Add( const ::rtl::OUString& Name, ::sal_Bool LinkToContent, ::sal_Int8 Type, const css::uno::Any& Value, const css::uno::Any& LinkSource ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
};

#endif /* SW_VBA_DOCUMENTPROPERTY_HXX */
