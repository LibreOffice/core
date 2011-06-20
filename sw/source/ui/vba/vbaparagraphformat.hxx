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
#ifndef SW_VBA_PARAGRAPHFORMAT_HXX
#define SW_VBA_PARAGRAPHFORMAT_HXX

#include <ooo/vba/word/XParagraphFormat.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/LineSpacing.hpp>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XParagraphFormat > SwVbaParagraphFormat_BASE;

class SwVbaParagraphFormat : public SwVbaParagraphFormat_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::beans::XPropertySet > mxParaProps;

private:
    css::style::LineSpacing getOOoLineSpacing( float _lineSpace, sal_Int16 mode );
    css::style::LineSpacing getOOoLineSpacingFromRule( sal_Int32 _linespacingrule );
    float getMSWordLineSpacing( css::style::LineSpacing& rLineSpacing );
    sal_Int32 getMSWordLineSpacingRule( css::style::LineSpacing& rLineSpacing );
    sal_Int16 getCharHeight() throw (css::uno::RuntimeException);
    sal_Int32 getOOoAlignment( sal_Int32 _alignment );
    sal_Int32 getMSWordAlignment( sal_Int32 _alignment );

public:
    SwVbaParagraphFormat( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rTextDocument, const css::uno::Reference< css::beans::XPropertySet >& rParaProps );
    virtual ~SwVbaParagraphFormat();

    // Attributes
    virtual ::sal_Int32 SAL_CALL getAlignment() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAlignment( ::sal_Int32 _alignment ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getFirstLineIndent() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFirstLineIndent( float _firstlineindent ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getKeepTogether() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setKeepTogether( const css::uno::Any& _keeptogether ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getKeepWithNext() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setKeepWithNext( const css::uno::Any& _keepwithnext ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getHyphenation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHyphenation( const css::uno::Any& _hyphenation ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getLineSpacing() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLineSpacing( float _linespacing ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getLineSpacingRule() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLineSpacingRule( ::sal_Int32 _linespacingrule ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getNoLineNumber() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setNoLineNumber( const css::uno::Any& _nolinenumber ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getOutlineLevel() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setOutlineLevel( ::sal_Int32 _outlinelevel ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getPageBreakBefore() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setPageBreakBefore( const css::uno::Any& _pagebreakbefore ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getSpaceBefore() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSpaceBefore( float _spacebefore ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getSpaceAfter() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSpaceAfter( float _spaceafter ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getLeftIndent() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLeftIndent( float _leftindent ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getRightIndent() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRightIndent( float _rightindent ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getTabStops() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTabStops( const css::uno::Any& _tabstops ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getWidowControl() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWidowControl( const css::uno::Any& _widowcontrol ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_PARAGRAPHFORMAT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
