/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBARANGE_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBARANGE_HXX

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
    void GetStyleInfo(OUString& aStyleName, OUString& aStyleType ) throw ( css::uno::RuntimeException );
public:
    SwVbaRange( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rTextDocument, const css::uno::Reference< css::text::XTextRange >& rStart, sal_Bool _bMaySpanEndOfDocument = sal_False ) throw (css::uno::RuntimeException);
    SwVbaRange( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rTextDocument, const css::uno::Reference< css::text::XTextRange >& rStart, const css::uno::Reference< css::text::XTextRange >& rEnd, sal_Bool _bMaySpanEndOfDocument = sal_False ) throw (css::uno::RuntimeException);
    SwVbaRange( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rTextDocument, const css::uno::Reference< css::text::XTextRange >& rStart, const css::uno::Reference< css::text::XTextRange >& rEnd, const css::uno::Reference< css::text::XText >& rText, sal_Bool _bMaySpanEndOfDocument = sal_False ) throw (css::uno::RuntimeException);
    virtual ~SwVbaRange();
   css::uno::Reference< css::text::XTextDocument > getDocument() const { return mxTextDocument; }

    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getXTextRange() throw (css::uno::RuntimeException, std::exception);
    css::uno::Reference< css::text::XText > getXText() const { return mxText; }
    void setXTextCursor( const css::uno::Reference< css::text::XTextCursor >& xTextCursor ) { mxTextCursor = xTextCursor; }
    void Move( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend, ooo::vba::word::E_DIRECTION eDirection ) throw (css::uno::RuntimeException);

    // Attribute
    virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setText( const OUString& rText ) throw (css::uno::RuntimeException, std::exception);
    virtual css::uno::Reference< ooo::vba::word::XParagraphFormat > SAL_CALL getParagraphFormat() throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setParagraphFormat( const css::uno::Reference< ooo::vba::word::XParagraphFormat >& rParagraphFormat ) throw (css::uno::RuntimeException, std::exception);
    virtual css::uno::Any SAL_CALL getStyle() throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setStyle( const css::uno::Any& _xStyle ) throw (css::uno::RuntimeException, std::exception);

    virtual css::uno::Reference< ooo::vba::word::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException, std::exception);
    virtual css::uno::Reference< ooo::vba::word::XListFormat > SAL_CALL getListFormat() throw (css::uno::RuntimeException, std::exception);
    // Methods
    virtual void SAL_CALL InsertBreak( const css::uno::Any& _breakType ) throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL Select() throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL InsertParagraph() throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL InsertParagraphBefore() throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL InsertParagraphAfter() throw (css::uno::RuntimeException, std::exception);
    virtual ::sal_Int32 SAL_CALL getLanguageID() throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setLanguageID( ::sal_Int32 _languageid ) throw (css::uno::RuntimeException, std::exception);
    virtual css::uno::Any SAL_CALL PageSetup() throw (css::uno::RuntimeException, std::exception);
    virtual ::sal_Int32 SAL_CALL getStart() throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setStart( ::sal_Int32 _start ) throw (css::uno::RuntimeException, std::exception);
    virtual ::sal_Int32 SAL_CALL getEnd() throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setEnd( ::sal_Int32 _end ) throw (css::uno::RuntimeException, std::exception);
    virtual ::sal_Bool SAL_CALL InRange( const css::uno::Reference< ::ooo::vba::word::XRange >& Range ) throw (css::uno::RuntimeException, std::exception);
    virtual css::uno::Any SAL_CALL Revisions( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception);
    virtual css::uno::Any SAL_CALL Sections( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception);
    virtual css::uno::Any SAL_CALL Fields( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception);

    // XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBARANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
