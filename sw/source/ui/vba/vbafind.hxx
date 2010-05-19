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
#ifndef SW_VBA_FIND_HXX
#define SW_VBA_FIND_HXX

#include <ooo/vba/word/XFind.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XPropertyReplace.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XFind > SwVbaFind_BASE;

class SwVbaFind : public SwVbaFind_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::text::XTextRange > mxTextRange;
    css::uno::Reference< css::util::XReplaceable > mxReplaceable;
    css::uno::Reference< css::util::XPropertyReplace> mxPropertyReplace;
    css::uno::Reference< css::text::XTextViewCursor> mxTVC;
    css::uno::Reference< css::view::XSelectionSupplier> mxSelSupp;
    sal_Bool mbReplace;
    sal_Int32 mnReplaceType;
    sal_Int32 mnWrap;

private:
    sal_Bool InRange( const css::uno::Reference< css::text::XTextRange >& xCurrentRange ) throw ( css::uno::RuntimeException );
    sal_Bool InEqualRange( const css::uno::Reference< css::text::XTextRange >& xCurrentRange ) throw ( css::uno::RuntimeException );
    void SetReplace( sal_Int32 type );
    void SetReplaceWith( const rtl::OUString& rText ) throw ( css::uno::RuntimeException );
    rtl::OUString GetReplaceWith() throw ( css::uno::RuntimeException );
#ifdef TOMORROW
    rtl::OUString ReplaceWildcards( const rtl::OUString& rText ) throw ( css::uno::RuntimeException );
#endif
    css::uno::Reference< css::text::XTextRange > FindOneElement() throw ( css::uno::RuntimeException );
    sal_Bool SearchReplace() throw ( css::uno::RuntimeException );

public:
    SwVbaFind( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XTextRange >& xTextRange ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaFind();

    // Attributes
    virtual ::rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setText( const ::rtl::OUString& _text ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getReplacement() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setReplacement( const css::uno::Any& _replacement ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getForward() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setForward( ::sal_Bool _forward ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getWrap() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWrap( ::sal_Int32 _wrap ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getFormat() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFormat( ::sal_Bool _format ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getMatchCase() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMatchCase( ::sal_Bool _matchcase ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getMatchWholeWord() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMatchWholeWord( ::sal_Bool _matchwholeword ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getMatchWildcards() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMatchWildcards( ::sal_Bool _matchwildcards ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getMatchSoundsLike() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMatchSoundsLike( ::sal_Bool _matchsoundslike ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getMatchAllWordForms() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMatchAllWordForms( ::sal_Bool _matchallwordforms ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStyle( const css::uno::Any& _style ) throw (css::uno::RuntimeException);

    // Methods
    virtual ::sal_Bool SAL_CALL Execute( const css::uno::Any& FindText, const css::uno::Any& MatchCase, const css::uno::Any& MatchWholeWord, const css::uno::Any& MatchWildcards, const css::uno::Any& MatchSoundsLike, const css::uno::Any& MatchAllWordForms, const css::uno::Any& Forward, const css::uno::Any& Wrap, const css::uno::Any& Format, const css::uno::Any& ReplaceWith, const css::uno::Any& Replace, const css::uno::Any& MatchKashida, const css::uno::Any& MatchDiacritics, const css::uno::Any& MatchAlefHamza, const css::uno::Any& MatchControl, const css::uno::Any& MatchPrefix, const css::uno::Any& MatchSuffix, const css::uno::Any& MatchPhrase, const css::uno::Any& IgnoreSpace, const css::uno::Any& IgnorePunct ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL ClearFormatting(  ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_FIND_HXX */
