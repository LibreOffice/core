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
#include <ooo/vba/word/XStyle.hpp>
#include <ooo/vba/word/XFont.hpp>
#include <ooo/vba/word/XHeaderFooter.hpp>
#include "wordvbahelper.hxx"

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XSelection > SwVbaSelection_BASE;

class SwVbaSelection : public SwVbaSelection_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::text::XTextViewCursor > mxTextViewCursor;
    css::uno::Reference< ooo::vba::word::XRange > mxRange;

private:
    void Move( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend, ooo::vba::word::E_DIRECTION eDirection ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    void NextCell( sal_Int32 nCount, ooo::vba::word::E_DIRECTION eDirection )
        throw (css::script::BasicErrorException, css::uno::RuntimeException);
    css::uno::Reference< css::text::XTextRange > GetSelectedRange() throw ( css::uno::RuntimeException );
    void GetSelectedCellRange( OUString& sTLName, OUString& sBRName ) throw ( css::uno::RuntimeException );
    css::uno::Reference< css::text::XTextTable > GetXTextTable() throw ( css::uno::RuntimeException );
    bool IsInTable() throw ( css::uno::RuntimeException );
    bool HasSelection() throw ( css::uno::RuntimeException );

public:
    SwVbaSelection( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& rModel ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaSelection();

    // Attribute
    virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setText( const OUString& rText ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL getRange() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL HomeKey( const css::uno::Any& _unit, const css::uno::Any& _extend ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL EndKey( const css::uno::Any& _unit, const css::uno::Any& _extend ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL TypeText( const OUString& rText ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Delete( const css::uno::Any& _unit, const css::uno::Any& _count ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL MoveRight( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL MoveLeft( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL MoveDown( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL MoveUp( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL TypeParagraph() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL InsertParagraph() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL InsertParagraphBefore() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL InsertParagraphAfter() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ooo::vba::word::XParagraphFormat > SAL_CALL getParagraphFormat() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setParagraphFormat( const css::uno::Reference< ooo::vba::word::XParagraphFormat >& rParagraphFormat ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ooo::vba::word::XFind > SAL_CALL getFind() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getStyle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStyle( const css::uno::Any& _xStyle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ooo::vba::word::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL TypeBackspace() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL GoTo( const css::uno::Any& _what, const css::uno::Any& _which, const css::uno::Any& _count, const css::uno::Any& _name )
        throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getLanguageID( ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLanguageID( ::sal_Int32 _languageid ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Information( sal_Int32 _type ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL InsertBreak( const css::uno::Any& _breakType ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Tables( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Fields( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ooo::vba::word::XHeaderFooter > SAL_CALL getHeaderFooter() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL ShapeRange( ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getStart() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStart( ::sal_Int32 _start ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getEnd() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setEnd( ::sal_Int32 _end ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL SelectRow() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL SelectColumn() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Rows( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Columns( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Cells( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Copy(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL CopyAsPicture(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Paste(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Collapse( const css::uno::Any& Direction ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL WholeStory(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL InRange( const css::uno::Reference< ::ooo::vba::word::XRange >& Range ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL SplitTable()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual css::uno::Any SAL_CALL Paragraphs( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBASELECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
