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
#include <ooo/vba/word/XFind.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <ooo/vba/word/XListFormat.hpp>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XRange > SwVbaRange_BASE;

class SwVbaRange : public SwVbaRange_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::text::XTextCursor >   mxTextCursor;
    css::uno::Reference< css::text::XText >         mxText;

private:
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    void initialize( const css::uno::Reference< css::text::XTextRange >& rStart, const css::uno::Reference< css::text::XTextRange >& rEnd );
    /// @throws css::uno::RuntimeException
    void GetStyleInfo(OUString& aStyleName, OUString& aStyleType );
public:
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    SwVbaRange( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, css::uno::Reference< css::text::XTextDocument > xTextDocument, const css::uno::Reference< css::text::XTextRange >& rStart);
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    SwVbaRange( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, css::uno::Reference< css::text::XTextDocument > xTextDocument, const css::uno::Reference< css::text::XTextRange >& rStart, const css::uno::Reference< css::text::XTextRange >& rEnd );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    SwVbaRange( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, css::uno::Reference< css::text::XTextDocument > xTextDocument, const css::uno::Reference< css::text::XTextRange >& rStart, const css::uno::Reference< css::text::XTextRange >& rEnd, css::uno::Reference< css::text::XText > xText);
    virtual ~SwVbaRange() override;
    const css::uno::Reference< css::text::XTextDocument >& getDocument() const { return mxTextDocument; }

    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getXTextRange() override;
    const css::uno::Reference< css::text::XText >& getXText() const { return mxText; }
    void setXTextCursor( const css::uno::Reference< css::text::XTextCursor >& xTextCursor ) { mxTextCursor = xTextCursor; }

    // Attribute
    virtual OUString SAL_CALL getText() override;
    virtual void SAL_CALL setText( const OUString& rText ) override;
    virtual css::uno::Reference< ooo::vba::word::XParagraphFormat > SAL_CALL getParagraphFormat() override;
    virtual void SAL_CALL setParagraphFormat( const css::uno::Reference< ooo::vba::word::XParagraphFormat >& rParagraphFormat ) override;
    virtual css::uno::Any SAL_CALL getStyle() override;
    virtual void SAL_CALL setStyle( const css::uno::Any& _xStyle ) override;
    virtual css::uno::Reference< ooo::vba::word::XFont > SAL_CALL getFont() override;
    virtual css::uno::Reference< ooo::vba::word::XFind > SAL_CALL getFind() override;
    virtual css::uno::Reference< ooo::vba::word::XListFormat > SAL_CALL getListFormat() override;

    //XDefaultProperty
    virtual OUString SAL_CALL getDefaultPropertyName() override { return u"Text"_ustr; }

    // Methods
    virtual void SAL_CALL InsertBreak(const css::uno::Any& _breakType) override;
    virtual void SAL_CALL Select() override;
    virtual void SAL_CALL InsertParagraph() override;
    virtual void SAL_CALL InsertParagraphBefore() override;
    virtual void SAL_CALL InsertParagraphAfter() override;
    virtual ::sal_Int32 SAL_CALL getLanguageID() override;
    virtual void SAL_CALL setLanguageID( ::sal_Int32 _languageid ) override;
    virtual css::uno::Any SAL_CALL PageSetup() override;
    virtual ::sal_Int32 SAL_CALL getStart() override;
    virtual void SAL_CALL setStart( ::sal_Int32 _start ) override;
    virtual ::sal_Int32 SAL_CALL getEnd() override;
    virtual void SAL_CALL setEnd( ::sal_Int32 _end ) override;
    virtual sal_Bool SAL_CALL InRange( const css::uno::Reference< ::ooo::vba::word::XRange >& Range ) override;
    virtual css::uno::Any SAL_CALL Revisions( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Sections( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Fields( const css::uno::Any& aIndex ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBARANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
