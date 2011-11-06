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


#ifndef SW_VBA_RANGE_HXX
#define SW_VBA_RANGE_HXX

#include <ooo/vba/word/XRange.hpp>
#include <ooo/vba/word/XParagraphFormat.hpp>
#include <ooo/vba/word/XFont.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <ooo/vba/word/XStyle.hpp>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XRange > SwVbaRange_BASE;

class SwVbaRange : public SwVbaRange_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::text::XTextCursor > mxTextCursor;
    css::uno::Reference< css::text::XText > mxText;
    sal_Bool mbMaySpanEndOfDocument;

private:
    void initialize( const css::uno::Reference< css::text::XTextRange >& rStart, const css::uno::Reference< css::text::XTextRange >& rEnd ) throw (css::uno::RuntimeException);
public:
    SwVbaRange( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rTextDocument, const css::uno::Reference< css::text::XTextRange >& rStart, sal_Bool _bMaySpanEndOfDocument = sal_False ) throw (css::uno::RuntimeException);
    SwVbaRange( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rTextDocument, const css::uno::Reference< css::text::XTextRange >& rStart, const css::uno::Reference< css::text::XTextRange >& rEnd, sal_Bool _bMaySpanEndOfDocument = sal_False ) throw (css::uno::RuntimeException);
    SwVbaRange( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rTextDocument, const css::uno::Reference< css::text::XTextRange >& rStart, const css::uno::Reference< css::text::XTextRange >& rEnd, const css::uno::Reference< css::text::XText >& rText, sal_Bool _bMaySpanEndOfDocument = sal_False ) throw (css::uno::RuntimeException);
    virtual ~SwVbaRange();
   css::uno::Reference< css::text::XTextDocument > getDocument() { return mxTextDocument; }

    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getXTextRange() throw (css::uno::RuntimeException);
#ifdef TOMORROW
    void setXTextRange( const css::uno::Reference< css::text::XTextRange >& xRange ) throw (css::uno::RuntimeException);
#endif
    css::uno::Reference< css::text::XText > getXText() { return mxText; }
    void setXTextCursor( const css::uno::Reference< css::text::XTextCursor >& xTextCursor ) { mxTextCursor = xTextCursor; }

    // Attribute
    virtual rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setText( const rtl::OUString& rText ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XParagraphFormat > SAL_CALL getParagraphFormat() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setParagraphFormat( const css::uno::Reference< ooo::vba::word::XParagraphFormat >& rParagraphFormat ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XStyle > SAL_CALL getStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStyle( const css::uno::Reference< ooo::vba::word::XStyle >& _xStyle ) throw (css::uno::RuntimeException);

    virtual css::uno::Reference< ooo::vba::word::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException);
    // Methods
    virtual void SAL_CALL InsertBreak( const css::uno::Any& _breakType ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Select() throw (css::uno::RuntimeException);
    virtual void SAL_CALL InsertParagraph() throw (css::uno::RuntimeException);
    virtual void SAL_CALL InsertParagraphBefore() throw (css::uno::RuntimeException);
    virtual void SAL_CALL InsertParagraphAfter() throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getLanguageID() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLanguageID( ::sal_Int32 _languageid ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL PageSetup() throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getStart() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStart( ::sal_Int32 _start ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getEnd() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEnd( ::sal_Int32 _end ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_RANGE_HXX */
