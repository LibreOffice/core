/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <rtl/ref.hxx>

class SwXTextDocument;

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XFind > SwVbaFind_BASE;

class SwVbaFind : public SwVbaFind_BASE
{
private:
    rtl::Reference< SwXTextDocument > mxModel;
    css::uno::Reference< css::text::XTextRange > mxTextRange;
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

    /// @throws css::uno::RuntimeException
    SwVbaFind( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent,
               const css::uno::Reference< css::uno::XComponentContext >& rContext,
               rtl::Reference< SwXTextDocument > xModel );
public:
    static css::uno::Reference< ooo::vba::word::XFind > GetOrCreateFind(
                const css::uno::Reference< ooo::vba::XHelperInterface >& rParent,
                const css::uno::Reference< css::uno::XComponentContext >& rContext,
                const rtl::Reference< SwXTextDocument >& xModel,
                const css::uno::Reference< css::text::XTextRange >& xTextRange);
    virtual ~SwVbaFind() override;

    // Attributes
    virtual OUString SAL_CALL getText() override;
    virtual void SAL_CALL setText( const OUString& _text ) override;
    virtual cpo::uno::Any SAL_CALL getReplacement() override;
    virtual void SAL_CALL setReplacement( const cpo::uno::Any& _replacement ) override;
    virtual bool SAL_CALL getForward() override;
    virtual void SAL_CALL setForward( bool _forward ) override;
    virtual ::sal_Int32 SAL_CALL getWrap() override;
    virtual void SAL_CALL setWrap( ::sal_Int32 _wrap ) override;
    virtual bool SAL_CALL getFormat() override;
    virtual void SAL_CALL setFormat( bool _format ) override;
    virtual bool SAL_CALL getMatchCase() override;
    virtual void SAL_CALL setMatchCase( bool _matchcase ) override;
    virtual bool SAL_CALL getMatchWholeWord() override;
    virtual void SAL_CALL setMatchWholeWord( bool _matchwholeword ) override;
    virtual bool SAL_CALL getMatchWildcards() override;
    virtual void SAL_CALL setMatchWildcards( bool _matchwildcards ) override;
    virtual bool SAL_CALL getMatchSoundsLike() override;
    virtual void SAL_CALL setMatchSoundsLike( bool _matchsoundslike ) override;
    virtual bool SAL_CALL getMatchAllWordForms() override;
    virtual void SAL_CALL setMatchAllWordForms( bool _matchallwordforms ) override;
    virtual cpo::uno::Any SAL_CALL getStyle() override;
    virtual void SAL_CALL setStyle( const cpo::uno::Any& _style ) override;

    // Methods
    virtual bool SAL_CALL Execute( const cpo::uno::Any& FindText, const cpo::uno::Any& MatchCase, const cpo::uno::Any& MatchWholeWord, const cpo::uno::Any& MatchWildcards, const cpo::uno::Any& MatchSoundsLike, const cpo::uno::Any& MatchAllWordForms, const cpo::uno::Any& Forward, const cpo::uno::Any& Wrap, const cpo::uno::Any& Format, const cpo::uno::Any& ReplaceWith, const cpo::uno::Any& Replace, const cpo::uno::Any& MatchKashida, const cpo::uno::Any& MatchDiacritics, const cpo::uno::Any& MatchAlefHamza, const cpo::uno::Any& MatchControl, const cpo::uno::Any& MatchPrefix, const cpo::uno::Any& MatchSuffix, const cpo::uno::Any& MatchPhrase, const cpo::uno::Any& IgnoreSpace, const cpo::uno::Any& IgnorePunct ) override;
    virtual void SAL_CALL ClearFormatting(  ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAFIND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
