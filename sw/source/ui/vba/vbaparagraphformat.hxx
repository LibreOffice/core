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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAPARAGRAPHFORMAT_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAPARAGRAPHFORMAT_HXX

#include <ooo/vba/word/XParagraphFormat.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/LineSpacing.hpp>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XParagraphFormat > SwVbaParagraphFormat_BASE;

class SwVbaParagraphFormat : public SwVbaParagraphFormat_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::beans::XPropertySet > mxParaProps;

private:
    static css::style::LineSpacing getOOoLineSpacing( float _lineSpace, sal_Int16 mode );
    css::style::LineSpacing getOOoLineSpacingFromRule( sal_Int32 _linespacingrule );
    static float getMSWordLineSpacing( css::style::LineSpacing& rLineSpacing );
    static sal_Int32 getMSWordLineSpacingRule( css::style::LineSpacing& rLineSpacing );
    sal_Int16 getCharHeight() throw (css::uno::RuntimeException);
    static sal_Int32 getOOoAlignment( sal_Int32 _alignment );
    static sal_Int32 getMSWordAlignment( sal_Int32 _alignment );

public:
    SwVbaParagraphFormat( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rTextDocument, const css::uno::Reference< css::beans::XPropertySet >& rParaProps );
    virtual ~SwVbaParagraphFormat();

    // Attributes
    virtual ::sal_Int32 SAL_CALL getAlignment() throw (css::uno::RuntimeException, css::script::BasicErrorException, std::exception) override;
    virtual void SAL_CALL setAlignment( ::sal_Int32 _alignment ) throw (css::uno::RuntimeException, std::exception) override;
    virtual float SAL_CALL getFirstLineIndent() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFirstLineIndent( float _firstlineindent ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getKeepTogether() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setKeepTogether( const css::uno::Any& _keeptogether ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getKeepWithNext() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setKeepWithNext( const css::uno::Any& _keepwithnext ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getHyphenation() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHyphenation( const css::uno::Any& _hyphenation ) throw (css::uno::RuntimeException, std::exception) override;
    virtual float SAL_CALL getLineSpacing() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLineSpacing( float _linespacing ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getLineSpacingRule() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLineSpacingRule( ::sal_Int32 _linespacingrule ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getNoLineNumber() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setNoLineNumber( const css::uno::Any& _nolinenumber ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getOutlineLevel() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setOutlineLevel( ::sal_Int32 _outlinelevel ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPageBreakBefore() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPageBreakBefore( const css::uno::Any& _pagebreakbefore ) throw (css::uno::RuntimeException, std::exception) override;
    virtual float SAL_CALL getSpaceBefore() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSpaceBefore( float _spacebefore ) throw (css::uno::RuntimeException, std::exception) override;
    virtual float SAL_CALL getSpaceAfter() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSpaceAfter( float _spaceafter ) throw (css::uno::RuntimeException, std::exception) override;
    virtual float SAL_CALL getLeftIndent() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLeftIndent( float _leftindent ) throw (css::uno::RuntimeException, std::exception) override;
    virtual float SAL_CALL getRightIndent() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRightIndent( float _rightindent ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getTabStops() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTabStops( const css::uno::Any& _tabstops ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getWidowControl() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setWidowControl( const css::uno::Any& _widowcontrol ) throw (css::uno::RuntimeException, std::exception) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAPARAGRAPHFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
