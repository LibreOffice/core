/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <ooo/vba/word/XListFormat.hpp>
#include "wordvbahelper.hxx"

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
    void GetStyleInfo(rtl::OUString& aStyleName, rtl::OUString& aStyleType ) throw ( css::uno::RuntimeException );
public:
    SwVbaRange( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rTextDocument, const css::uno::Reference< css::text::XTextRange >& rStart, sal_Bool _bMaySpanEndOfDocument = sal_False ) throw (css::uno::RuntimeException);
    SwVbaRange( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rTextDocument, const css::uno::Reference< css::text::XTextRange >& rStart, const css::uno::Reference< css::text::XTextRange >& rEnd, sal_Bool _bMaySpanEndOfDocument = sal_False ) throw (css::uno::RuntimeException);
    SwVbaRange( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rTextDocument, const css::uno::Reference< css::text::XTextRange >& rStart, const css::uno::Reference< css::text::XTextRange >& rEnd, const css::uno::Reference< css::text::XText >& rText, sal_Bool _bMaySpanEndOfDocument = sal_False ) throw (css::uno::RuntimeException);
    virtual ~SwVbaRange();
   css::uno::Reference< css::text::XTextDocument > getDocument() const { return mxTextDocument; }

    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getXTextRange() throw (css::uno::RuntimeException);
    css::uno::Reference< css::text::XText > getXText() const { return mxText; }
    void setXTextCursor( const css::uno::Reference< css::text::XTextCursor >& xTextCursor ) { mxTextCursor = xTextCursor; }
    void Move( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend, ooo::vba::word::E_DIRECTION eDirection ) throw (css::uno::RuntimeException);

    // Attribute
    virtual rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setText( const rtl::OUString& rText ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XParagraphFormat > SAL_CALL getParagraphFormat() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setParagraphFormat( const css::uno::Reference< ooo::vba::word::XParagraphFormat >& rParagraphFormat ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStyle( const css::uno::Any& _xStyle ) throw (css::uno::RuntimeException);

    virtual css::uno::Reference< ooo::vba::word::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XListFormat > SAL_CALL getListFormat() throw (css::uno::RuntimeException);
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
    virtual ::sal_Bool SAL_CALL InRange( const css::uno::Reference< ::ooo::vba::word::XRange >& Range ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Revisions( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Sections( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Fields( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_RANGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
