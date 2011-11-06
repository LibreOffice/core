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


#ifndef SW_VBA_PARAGRAPH_HXX
#define SW_VBA_PARAGRAPH_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XParagraphs.hpp>
#include <ooo/vba/word/XParagraph.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextDocument.hpp>


typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XParagraph > SwVbaParagraph_BASE;

class SwVbaParagraph : public SwVbaParagraph_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::text::XTextRange > mxTextRange;

public:
    SwVbaParagraph( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& xDocument, const css::uno::Reference< css::text::XTextRange >& xTextRange ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaParagraph();

    // XParagraph
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL getRange() throw ( css::uno::RuntimeException );

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};


/* class SwVbaParagraphs */
typedef CollTestImplHelper< ooo::vba::word::XParagraphs > SwVbaParagraphs_BASE;

class SwVbaParagraphs : public SwVbaParagraphs_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
public:
    SwVbaParagraphs( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::text::XTextDocument >& xDocument ) throw (css::uno::RuntimeException);
    virtual ~SwVbaParagraphs() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // SwVbaParagraphs_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif /* SW_VBA_PARAGRAPH_HXX */
