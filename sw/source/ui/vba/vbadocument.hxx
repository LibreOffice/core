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


#ifndef SW_VBA_DOCUMENT_HXX
#define SW_VBA_DOCUMENT_HXX

#include <ooo/vba/word/XDocument.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbadocumentbase.hxx>
#include <com/sun/star/text/XTextDocument.hpp>

typedef cppu::ImplInheritanceHelper1< VbaDocumentBase, ooo::vba::word::XDocument > SwVbaDocument_BASE;

class SwVbaDocument : public SwVbaDocument_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;

    void Initialize();
    css::uno::Any getControlShape( const rtl::OUString& sName );
    css::uno::Reference< css::container::XNameAccess > getFormControls();

public:
    SwVbaDocument( const css::uno::Reference< ooo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& m_xContext, css::uno::Reference< css::frame::XModel > xModel );
    SwVbaDocument(  css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );
    virtual ~SwVbaDocument();

    // XDocument
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL getContent() throw ( css::uno::RuntimeException );
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL Range( const css::uno::Any& rStart, const css::uno::Any& rEnd ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL BuiltInDocumentProperties( const css::uno::Any& index ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL CustomDocumentProperties( const css::uno::Any& index ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Bookmarks( const css::uno::Any& rIndex ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL Variables( const css::uno::Any& rIndex ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL getAttachedTemplate() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAttachedTemplate( const css::uno::Any& _attachedtemplate ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Paragraphs( const css::uno::Any& rIndex ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL Styles( const css::uno::Any& rIndex ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL Tables( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Fields( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Shapes( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Sections( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Activate() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL PageSetup() throw (css::uno::RuntimeException);
    // XInvocation
    virtual css::uno::Reference< css::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL invoke( const ::rtl::OUString& aFunctionName, const css::uno::Sequence< css::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< css::uno::Any >& aOutParam ) throw (css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL setValue( const ::rtl::OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getValue( const ::rtl::OUString& aPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasMethod( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasProperty( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_DOCUMENT_HXX */
