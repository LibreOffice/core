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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBASELECTION_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBASELECTION_HXX

#include <ooo/vba/word/XSelection.hpp>
#include <ooo/vba/word/XRange.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <ooo/vba/word/XParagraphFormat.hpp>
#include <ooo/vba/word/XFind.hpp>
#include <ooo/vba/word/XFont.hpp>
#include <ooo/vba/word/XHeaderFooter.hpp>
#include "wordvbahelper.hxx"

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XSelection > SwVbaSelection_BASE;

class SwVbaSelection : public SwVbaSelection_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::text::XTextViewCursor > mxTextViewCursor;

private:
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    void Move( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend, ooo::vba::word::E_DIRECTION eDirection );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    void NextCell( sal_Int32 nCount, ooo::vba::word::E_DIRECTION eDirection );
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::text::XTextRange > GetSelectedRange();
    /// @throws css::uno::RuntimeException
    void GetSelectedCellRange( OUString& sTLName, OUString& sBRName );
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::text::XTextTable > GetXTextTable();
    /// @throws css::uno::RuntimeException
    bool IsInTable();
    /// @throws css::uno::RuntimeException
    bool HasSelection();

public:
    /// @throws css::uno::RuntimeException
    SwVbaSelection( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& rModel );
    virtual ~SwVbaSelection() override;

    // Attribute
    virtual OUString SAL_CALL getText() override;
    virtual void SAL_CALL setText( const OUString& rText ) override;
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL getRange() override;
    virtual void SAL_CALL HomeKey( const css::uno::Any& _unit, const css::uno::Any& _extend ) override;
    virtual void SAL_CALL EndKey( const css::uno::Any& _unit, const css::uno::Any& _extend ) override;
    virtual void SAL_CALL TypeText( const OUString& rText ) override;
    virtual void SAL_CALL Delete( const css::uno::Any& _unit, const css::uno::Any& _count ) override;
    virtual void SAL_CALL MoveRight( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend ) override;
    virtual void SAL_CALL MoveLeft( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend ) override;
    virtual void SAL_CALL MoveDown( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend ) override;
    virtual void SAL_CALL MoveUp( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend ) override;
    virtual void SAL_CALL TypeParagraph() override;
    virtual void SAL_CALL InsertParagraph() override;
    virtual void SAL_CALL InsertParagraphBefore() override;
    virtual void SAL_CALL InsertParagraphAfter() override;
    virtual css::uno::Reference< ooo::vba::word::XParagraphFormat > SAL_CALL getParagraphFormat() override;
    virtual void SAL_CALL setParagraphFormat( const css::uno::Reference< ooo::vba::word::XParagraphFormat >& rParagraphFormat ) override;
    virtual css::uno::Reference< ooo::vba::word::XFind > SAL_CALL getFind() override;
    virtual css::uno::Any SAL_CALL getStyle() override;
    virtual void SAL_CALL setStyle( const css::uno::Any& _xStyle ) override;
    virtual css::uno::Reference< ooo::vba::word::XFont > SAL_CALL getFont() override;
    virtual void SAL_CALL TypeBackspace() override;
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL GoTo( const css::uno::Any& _what, const css::uno::Any& _which, const css::uno::Any& _count, const css::uno::Any& _name ) override;
    virtual ::sal_Int32 SAL_CALL getLanguageID( ) override;
    virtual void SAL_CALL setLanguageID( ::sal_Int32 _languageid ) override;
    virtual css::uno::Any SAL_CALL Information( sal_Int32 _type ) override;
    virtual void SAL_CALL InsertBreak( const css::uno::Any& _breakType ) override;
    virtual css::uno::Any SAL_CALL Tables( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Fields( const css::uno::Any& aIndex ) override;
    virtual css::uno::Reference< ooo::vba::word::XHeaderFooter > SAL_CALL getHeaderFooter() override;
    virtual css::uno::Any SAL_CALL ShapeRange( ) override;
    virtual ::sal_Int32 SAL_CALL getStart() override;
    virtual void SAL_CALL setStart( ::sal_Int32 _start ) override;
    virtual ::sal_Int32 SAL_CALL getEnd() override;
    virtual void SAL_CALL setEnd( ::sal_Int32 _end ) override;
    virtual void SAL_CALL SelectRow() override;
    virtual void SAL_CALL SelectColumn() override;
    virtual css::uno::Any SAL_CALL Rows( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Columns( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Cells( const css::uno::Any& aIndex ) override;
    virtual void SAL_CALL Copy(  ) override;
    virtual void SAL_CALL CopyAsPicture(  ) override;
    virtual void SAL_CALL Paste(  ) override;
    virtual void SAL_CALL Collapse( const css::uno::Any& Direction ) override;
    virtual void SAL_CALL WholeStory(  ) override;
    virtual sal_Bool SAL_CALL InRange( const css::uno::Reference< ::ooo::vba::word::XRange >& Range ) override;
    virtual void SAL_CALL SplitTable() override;
    virtual css::uno::Any SAL_CALL Paragraphs( const css::uno::Any& aIndex ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBASELECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
