/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#ifndef SW_VBA_SELECTION_HXX
#define SW_VBA_SELECTION_HXX

#include <ooo/vba/word/XSelection.hpp>
#include <ooo/vba/word/XRange.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <ooo/vba/word/XParagraphFormat.hpp>
#include <ooo/vba/word/XFind.hpp>
#include <ooo/vba/word/XStyle.hpp>
#include <ooo/vba/word/XFont.hpp>
#include <ooo/vba/word/XHeaderFooter.hpp>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XSelection > SwVbaSelection_BASE;

class SwVbaSelection : public SwVbaSelection_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::text::XTextViewCursor > mxTextViewCursor;
    css::uno::Reference< ooo::vba::word::XRange > mxRange;

    enum E_DIRECTION
    {
        MOVE_LEFT = 1,
        MOVE_RIGHT,
        MOVE_UP,
        MOVE_DOWN
    };

private:
    void NextCell( sal_Int32 nCount, E_DIRECTION eDirection ) throw ( css::uno::RuntimeException );
    css::uno::Reference< css::text::XTextRange > GetSelectedRange() throw ( css::uno::RuntimeException );

public:
    SwVbaSelection( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& rModel ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaSelection();

    // Attribute
    virtual rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setText( const rtl::OUString& rText ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL getRange() throw (css::uno::RuntimeException);
    virtual void SAL_CALL HomeKey( const css::uno::Any& _unit, const css::uno::Any& _extend ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL EndKey( const css::uno::Any& _unit, const css::uno::Any& _extend ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL TypeText( const rtl::OUString& rText ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Delete( const css::uno::Any& _unit, const css::uno::Any& _count ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL MoveRight( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL MoveLeft( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL MoveDown( const css::uno::Any& _unit, const css::uno::Any& _count, const css::uno::Any& _extend ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL TypeParagraph() throw (css::uno::RuntimeException);
    virtual void SAL_CALL InsertParagraph() throw (css::uno::RuntimeException);
    virtual void SAL_CALL InsertParagraphBefore() throw (css::uno::RuntimeException);
    virtual void SAL_CALL InsertParagraphAfter() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XParagraphFormat > SAL_CALL getParagraphFormat() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setParagraphFormat( const css::uno::Reference< ooo::vba::word::XParagraphFormat >& rParagraphFormat ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XFind > SAL_CALL getFind() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XStyle > SAL_CALL getStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStyle( const css::uno::Reference< ooo::vba::word::XStyle >& _xStyle ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException);
    virtual void SAL_CALL TypeBackspace() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL GoTo( const css::uno::Any& _what, const css::uno::Any& _which, const css::uno::Any& _count, const css::uno::Any& _name ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getLanguageID( ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLanguageID( ::sal_Int32 _languageid ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Information( sal_Int32 _type ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL InsertBreak( const css::uno::Any& _breakType ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Tables( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Fields( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XHeaderFooter > SAL_CALL getHeaderFooter() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL ShapeRange( ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getStart() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStart( ::sal_Int32 _start ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getEnd() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEnd( ::sal_Int32 _end ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_SELECTION_HXX */
