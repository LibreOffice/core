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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAFIND_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAFIND_HXX

#include <ooo/vba/word/XFind.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XPropertyReplace.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XFind > SwVbaFind_BASE;

class SwVbaFind : public SwVbaFind_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::text::XTextRange > mxTextRange;
    css::uno::Reference< css::util::XReplaceable > mxReplaceable;
    css::uno::Reference< css::util::XPropertyReplace> mxPropertyReplace;
    css::uno::Reference< css::text::XTextViewCursor> mxTVC;
    css::uno::Reference< css::view::XSelectionSupplier> mxSelSupp;
    bool mbReplace;
    sal_Int32 mnReplaceType;
    sal_Int32 mnWrap;

private:
    /// @throws css::uno::RuntimeException
    bool InRange( const css::uno::Reference< css::text::XTextRange >& xCurrentRange );
    /// @throws css::uno::RuntimeException
    bool InEqualRange( const css::uno::Reference< css::text::XTextRange >& xCurrentRange );
    void SetReplace( sal_Int32 type );
    /// @throws css::uno::RuntimeException
    void SetReplaceWith( const OUString& rText );
    /// @throws css::uno::RuntimeException
    OUString GetReplaceWith();
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::text::XTextRange > FindOneElement();
    /// @throws css::uno::RuntimeException
    bool SearchReplace();

public:
    /// @throws css::uno::RuntimeException
    SwVbaFind( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XTextRange >& xTextRange );
    virtual ~SwVbaFind() override;

    // Attributes
    virtual OUString SAL_CALL getText() override;
    virtual void SAL_CALL setText( const OUString& _text ) override;
    virtual css::uno::Any SAL_CALL getReplacement() override;
    virtual void SAL_CALL setReplacement( const css::uno::Any& _replacement ) override;
    virtual sal_Bool SAL_CALL getForward() override;
    virtual void SAL_CALL setForward( sal_Bool _forward ) override;
    virtual ::sal_Int32 SAL_CALL getWrap() override;
    virtual void SAL_CALL setWrap( ::sal_Int32 _wrap ) override;
    virtual sal_Bool SAL_CALL getFormat() override;
    virtual void SAL_CALL setFormat( sal_Bool _format ) override;
    virtual sal_Bool SAL_CALL getMatchCase() override;
    virtual void SAL_CALL setMatchCase( sal_Bool _matchcase ) override;
    virtual sal_Bool SAL_CALL getMatchWholeWord() override;
    virtual void SAL_CALL setMatchWholeWord( sal_Bool _matchwholeword ) override;
    virtual sal_Bool SAL_CALL getMatchWildcards() override;
    virtual void SAL_CALL setMatchWildcards( sal_Bool _matchwildcards ) override;
    virtual sal_Bool SAL_CALL getMatchSoundsLike() override;
    virtual void SAL_CALL setMatchSoundsLike( sal_Bool _matchsoundslike ) override;
    virtual sal_Bool SAL_CALL getMatchAllWordForms() override;
    virtual void SAL_CALL setMatchAllWordForms( sal_Bool _matchallwordforms ) override;
    virtual css::uno::Any SAL_CALL getStyle() override;
    virtual void SAL_CALL setStyle( const css::uno::Any& _style ) override;

    // Methods
    virtual sal_Bool SAL_CALL Execute( const css::uno::Any& FindText, const css::uno::Any& MatchCase, const css::uno::Any& MatchWholeWord, const css::uno::Any& MatchWildcards, const css::uno::Any& MatchSoundsLike, const css::uno::Any& MatchAllWordForms, const css::uno::Any& Forward, const css::uno::Any& Wrap, const css::uno::Any& Format, const css::uno::Any& ReplaceWith, const css::uno::Any& Replace, const css::uno::Any& MatchKashida, const css::uno::Any& MatchDiacritics, const css::uno::Any& MatchAlefHamza, const css::uno::Any& MatchControl, const css::uno::Any& MatchPrefix, const css::uno::Any& MatchSuffix, const css::uno::Any& MatchPhrase, const css::uno::Any& IgnoreSpace, const css::uno::Any& IgnorePunct ) override;
    virtual void SAL_CALL ClearFormatting(  ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAFIND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
