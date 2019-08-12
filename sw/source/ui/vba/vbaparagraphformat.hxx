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
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XParagraphFormat > SwVbaParagraphFormat_BASE;

class SwVbaParagraphFormat : public SwVbaParagraphFormat_BASE
{
private:
    css::uno::Reference< css::beans::XPropertySet > mxParaProps;

private:
    static css::style::LineSpacing getOOoLineSpacing( float _lineSpace, sal_Int16 mode );
    css::style::LineSpacing getOOoLineSpacingFromRule( sal_Int32 _linespacingrule );
    static float getMSWordLineSpacing( css::style::LineSpacing const & rLineSpacing );
    static sal_Int32 getMSWordLineSpacingRule( css::style::LineSpacing const & rLineSpacing );
    /// @throws css::uno::RuntimeException
    sal_Int16 getCharHeight();
    static css::style::ParagraphAdjust getOOoAlignment( sal_Int32 _alignment );
    static sal_Int32 getMSWordAlignment( css::style::ParagraphAdjust _alignment );

public:
    SwVbaParagraphFormat( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::beans::XPropertySet >& rParaProps );
    virtual ~SwVbaParagraphFormat() override;

    // Attributes
    virtual ::sal_Int32 SAL_CALL getAlignment() override;
    virtual void SAL_CALL setAlignment( ::sal_Int32 _alignment ) override;
    virtual float SAL_CALL getFirstLineIndent() override;
    virtual void SAL_CALL setFirstLineIndent( float _firstlineindent ) override;
    virtual css::uno::Any SAL_CALL getKeepTogether() override;
    virtual void SAL_CALL setKeepTogether( const css::uno::Any& _keeptogether ) override;
    virtual css::uno::Any SAL_CALL getKeepWithNext() override;
    virtual void SAL_CALL setKeepWithNext( const css::uno::Any& _keepwithnext ) override;
    virtual css::uno::Any SAL_CALL getHyphenation() override;
    virtual void SAL_CALL setHyphenation( const css::uno::Any& _hyphenation ) override;
    virtual float SAL_CALL getLineSpacing() override;
    virtual void SAL_CALL setLineSpacing( float _linespacing ) override;
    virtual ::sal_Int32 SAL_CALL getLineSpacingRule() override;
    virtual void SAL_CALL setLineSpacingRule( ::sal_Int32 _linespacingrule ) override;
    virtual css::uno::Any SAL_CALL getNoLineNumber() override;
    virtual void SAL_CALL setNoLineNumber( const css::uno::Any& _nolinenumber ) override;
    virtual ::sal_Int32 SAL_CALL getOutlineLevel() override;
    virtual void SAL_CALL setOutlineLevel( ::sal_Int32 _outlinelevel ) override;
    virtual css::uno::Any SAL_CALL getPageBreakBefore() override;
    virtual void SAL_CALL setPageBreakBefore( const css::uno::Any& _pagebreakbefore ) override;
    virtual float SAL_CALL getSpaceBefore() override;
    virtual void SAL_CALL setSpaceBefore( float _spacebefore ) override;
    virtual float SAL_CALL getSpaceAfter() override;
    virtual void SAL_CALL setSpaceAfter( float _spaceafter ) override;
    virtual float SAL_CALL getLeftIndent() override;
    virtual void SAL_CALL setLeftIndent( float _leftindent ) override;
    virtual float SAL_CALL getRightIndent() override;
    virtual void SAL_CALL setRightIndent( float _rightindent ) override;
    virtual css::uno::Any SAL_CALL getTabStops() override;
    virtual void SAL_CALL setTabStops( const css::uno::Any& _tabstops ) override;
    virtual css::uno::Any SAL_CALL getWidowControl() override;
    virtual void SAL_CALL setWidowControl( const css::uno::Any& _widowcontrol ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAPARAGRAPHFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
