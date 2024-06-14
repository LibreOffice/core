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


// AccTextBase.cpp: implementation of the CAccTextBase class.

#include "stdafx.h"

#include "AccTextBase.h"

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <vcl/accessibility/AccessibleTextAttributeHelper.hxx>
#include <vcl/svapp.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <com/sun/star/accessibility/AccessibleScrollType.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleTextSelection.hpp>
#include "MAccessible.h"

using namespace css::accessibility;
using namespace css::uno;

namespace
{
sal_Int16 lcl_matchIA2TextBoundaryType(IA2TextBoundaryType boundaryType)
{
    switch (boundaryType)
    {
    case IA2_TEXT_BOUNDARY_CHAR:
        return com::sun::star::accessibility::AccessibleTextType::CHARACTER;
    case IA2_TEXT_BOUNDARY_WORD:
       return com::sun::star::accessibility::AccessibleTextType::WORD;
    case IA2_TEXT_BOUNDARY_SENTENCE:
        return com::sun::star::accessibility::AccessibleTextType::SENTENCE;
    case IA2_TEXT_BOUNDARY_PARAGRAPH:
        return com::sun::star::accessibility::AccessibleTextType::PARAGRAPH;
    case IA2_TEXT_BOUNDARY_LINE:
        return com::sun::star::accessibility::AccessibleTextType::LINE;
    case IA2_TEXT_BOUNDARY_ALL:
        // assert here, better handle it directly at call site
        assert(false
               && "No match for IA2_TEXT_BOUNDARY_ALL, handle at call site.");
        break;
    default:
        break;
    }

    SAL_WARN("iacc2", "Unmatched text boundary type: " << boundaryType);
    return -1;
}
}


// Construction/Destruction


CAccTextBase::CAccTextBase()
{}

CAccTextBase::~CAccTextBase()
{}


/**
   * Get special selection.
   * @param startOffset Start selection offset.
   * @param endOffset   End selection offset.
   * @param success     Variant to accept the result of if the method call is successful.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_addSelection(long startOffset, long endOffset)
{
    SolarMutexGuard g;

    try {

    if(pUNOInterface == nullptr)
        return E_FAIL;

    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();

    Reference< XAccessibleTextSelection > pRExtension(pRContext,UNO_QUERY);

    if( pRExtension.is() )
    {
        pRExtension->addSelection(0, startOffset, endOffset);
        return S_OK;
    }
    else
    {
        pRXText->setSelection(startOffset, endOffset);
        return S_OK;
    }

    } catch(...) { return E_FAIL; }
}

/**
   * Get special attributes.
   * @param offset Offset.
   * @param startOffset Variant to accept start offset.
   * @param endOffset   Variant to accept end offset.
   * @param textAttributes     Variant to accept attributes.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_attributes(long offset, long * startOffset, long * endOffset, BSTR * textAttributes)
{
    SolarMutexGuard g;

    try {

    if (startOffset == nullptr || endOffset == nullptr || textAttributes == nullptr)
        return E_INVALIDARG;

    if(!pRXText.is())
    {
        return E_FAIL;
    }

    if (offset < 0 || offset > pRXText->getCharacterCount() )
        return E_FAIL;


    const OUString sAttrs = AccessibleTextAttributeHelper::GetIAccessible2TextAttributes(pRXText,
                                                                                         IA2AttributeType::TextAttributes,
                                                                                         offset, *startOffset, *endOffset);

    if(*textAttributes)
        SysFreeString(*textAttributes);
    *textAttributes = SysAllocString(o3tl::toW(sAttrs.getStr()));

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
   * Get caret position.
   * @param offset     Variant to accept caret offset.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_caretOffset(long * offset)
{
    SolarMutexGuard g;

    try {

    if (offset == nullptr)
        return E_INVALIDARG;

    if(!pRXText.is())
    {
        *offset = 0;
        return S_OK;
    }

    *offset = pRXText->getCaretPosition();
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
   * Get character count.
   * @param nCharacters  Variant to accept character count.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_characterCount(long * nCharacters)
{
    SolarMutexGuard g;

    try {

    if (nCharacters == nullptr)
        return E_INVALIDARG;

    if(!pRXText.is())
    {
        *nCharacters = 0;
        return S_OK;
    }

    *nCharacters = pRXText->getCharacterCount();
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
   * Get character extents.
   * @param offset  Offset.
   * @param x Variant to accept x position.
   * @param y Variant to accept y position.
   * @param width Variant to accept width.
   * @param Height Variant to accept height.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_characterExtents(long offset, IA2CoordinateType coordType, long * x, long * y, long * width, long * height)
{
    SolarMutexGuard g;

    try {

    if (x == nullptr || height == nullptr || y == nullptr || width == nullptr)
        return E_INVALIDARG;

    if(!pRXText.is())
        return E_FAIL;

    if (offset < 0 || offset > pRXText->getCharacterCount())
        return E_FAIL;

    css::awt::Rectangle rectangle;
    rectangle = pRXText->getCharacterBounds(offset);

    //IA2Point aPoint;
    css::awt::Point aPoint;

    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();
    if( !pRContext.is() )
    {
        return E_FAIL;
    }
    Reference<XAccessibleComponent> pRComp(pRContext,UNO_QUERY);
    if( pRComp.is() )
    {
        if(coordType == IA2_COORDTYPE_SCREEN_RELATIVE)
        {
            css::awt::Point pt = pRComp->getLocationOnScreen();
            aPoint.X = pt.X;
            aPoint.Y = pt.Y;
        }
        else if(coordType == IA2_COORDTYPE_PARENT_RELATIVE)
        {
            css::awt::Point pt = pRComp->getLocation();
            aPoint.X = pt.X;
            aPoint.Y = pt.Y;
        }
    }
    rectangle.X = rectangle.X + aPoint.X;
    rectangle.Y = rectangle.Y + aPoint.Y;

    *x = rectangle.X;
    *y = rectangle.Y;

    // pRXText->getCharacterBounds() have different implement in different acc component
    // But we need return the width/height == 1 for every component when offset == text length.
    // So we ignore the return result of pRXText->getCharacterBounds() when offset == text length.
    if (offset == pRXText->getCharacterCount())
    {
        *width = 1;
        *height = 1;
    }
    else
    {
        *width = rectangle.Width;
        *height = rectangle.Height;
    }

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
   * Get selections count.
   * @param nSelections Variant to accept selections count.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_nSelections(long * nSelections)
{
    SolarMutexGuard g;

    try {

    if (nSelections == nullptr)
        return E_INVALIDARG;

    if(pUNOInterface == nullptr)
    {
        *nSelections = 0;
        return S_OK;
    }

    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();

    Reference< XAccessibleTextSelection > pRExtension(pRContext,UNO_QUERY);

    if( pRExtension.is() )
    {
        *nSelections = pRExtension->getSelectedPortionCount();
        return S_OK;
    }

    long iLength = pRXText->getSelectedText().getLength();
    if( iLength> 0)
    {
        *nSelections = 1;
        return S_OK;
    }

    *nSelections = 0;
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
   * Get offset of some special point.
   * @param x X position of one point.
   * @param x Y position of one point.
   * @param coordType Type.
   * @param offset Variant to accept offset.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_offsetAtPoint(long x, long y, IA2CoordinateType coordType, long * offset)
{
    SolarMutexGuard g;

    try {

    if (offset == nullptr)
        return E_INVALIDARG;

    if(!pRXText.is())
        return E_FAIL;

    css::awt::Point point;
    point.X = x;
    point.Y = y;

    if (coordType == IA2_COORDTYPE_SCREEN_RELATIVE)
    {
        // convert from screen to local coordinates
        Reference<XAccessibleContext> xContext = pUNOInterface->getAccessibleContext();
        Reference<XAccessibleComponent> xComponent(xContext, UNO_QUERY);
        if (!xComponent.is())
            return S_FALSE;

        css::awt::Point aObjectPos = xComponent->getLocationOnScreen();
        point.X -= aObjectPos.X;
        point.Y -= aObjectPos.Y;
    }

    *offset = pRXText->getIndexAtPoint(point);
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
   * Get selection range.
   * @param selection selection count.
   * @param startOffset Variant to accept the start offset of special selection.
   * @param endOffset Variant to accept the end offset of special selection.
   * @return Result.
*/

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_selection(long selectionIndex, long * startOffset, long * endOffset)
{
    SolarMutexGuard g;

    try {

    if (startOffset == nullptr || endOffset == nullptr )
        return E_INVALIDARG;

    if(pUNOInterface == nullptr )
        return E_FAIL;

    long nSelection = 0;
    get_nSelections(&nSelection);

    if(selectionIndex >= nSelection || selectionIndex < 0 )
        return E_FAIL;

    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();

    Reference< XAccessibleTextSelection > pRExtension(pRContext,UNO_QUERY);

    if( pRExtension.is() )
    {
        *startOffset = pRExtension->getSeletedPositionStart(selectionIndex);
        *endOffset = pRExtension->getSeletedPositionEnd(selectionIndex);
        return S_OK;
    }
    else if (pRXText->getSelectionEnd() > -1)
    {
        *startOffset = pRXText->getSelectionStart();
        *endOffset = pRXText->getSelectionEnd();
        return S_OK;
    }

    *startOffset = 0;
    *endOffset = 0;
    return E_FAIL;

    } catch(...) { return E_FAIL; }
}

/**
   * Get special text.
   * @param startOffset Start position of special range.
   * @param endOffset   End position of special range.
   * @param text        Variant to accept the text of special range.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_text(long startOffset, long endOffset, BSTR * text)
{
    SolarMutexGuard g;

    try {

    if (text == nullptr)
        return E_INVALIDARG;

    if(!pRXText.is())
        return E_FAIL;

    if (endOffset == -1)
        endOffset = pRXText->getCharacterCount();

    if (endOffset < 0 || endOffset < startOffset)
        return E_FAIL;

    const OUString ouStr = pRXText->getTextRange(startOffset, endOffset);
    SysFreeString(*text);
    *text = SysAllocString(o3tl::toW(ouStr.getStr()));
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
   * Get special text before some position.
   * @param offset Special position.
   * @param boundaryType Boundary type.
   * @param startOffset Variant to accept the start offset.
   * @param endOffset   Variant to accept the end offset.
   * @param text        Variant to accept the special text.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_textBeforeOffset(long offset, IA2TextBoundaryType boundaryType, long * startOffset, long * endOffset, BSTR * text)
{
    SolarMutexGuard g;

    try {

    if (startOffset == nullptr || endOffset == nullptr || text == nullptr)
        return E_INVALIDARG;

    if(!pRXText.is())
        return E_FAIL;

    if (boundaryType == IA2_TEXT_BOUNDARY_ALL)
    {
        long nChar;
        get_nCharacters( &nChar );
        *startOffset = 0;
        *endOffset = nChar;
        return get_text(0, nChar, text);
    }

    const sal_Int16 nUnoBoundaryType = lcl_matchIA2TextBoundaryType(boundaryType);
    if (nUnoBoundaryType < 0)
        return E_FAIL;

    TextSegment segment = pRXText->getTextBeforeIndex(offset, nUnoBoundaryType);
    OUString ouStr = segment.SegmentText;
    SysFreeString(*text);
    *text = SysAllocString(o3tl::toW(ouStr.getStr()));
    *startOffset = segment.SegmentStart;
    *endOffset = segment.SegmentEnd;

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
   * Get special text after some position.
   * @param offset Special position.
   * @param boundaryType Boundary type.
   * @param startOffset Variant to accept the start offset.
   * @param endOffset   Variant to accept the end offset.
   * @param text        Variant to accept the special text.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_textAfterOffset(long offset, IA2TextBoundaryType boundaryType, long * startOffset, long * endOffset, BSTR * text)
{
    SolarMutexGuard g;

    try {

    if (startOffset == nullptr || endOffset == nullptr || text == nullptr)
        return E_INVALIDARG;

    if(!pRXText.is())
        return E_FAIL;

    if (boundaryType == IA2_TEXT_BOUNDARY_ALL)
    {
        long nChar;
        get_nCharacters( &nChar );
        *startOffset = 0;
        *endOffset = nChar;
        return get_text(0, nChar, text);
    }

    const sal_Int16 nUnoBoundaryType = lcl_matchIA2TextBoundaryType(boundaryType);
    if (nUnoBoundaryType < 0)
        return E_FAIL;

    TextSegment segment = pRXText->getTextBehindIndex(offset, nUnoBoundaryType);
    OUString ouStr = segment.SegmentText;
    SysFreeString(*text);
    *text = SysAllocString(o3tl::toW(ouStr.getStr()));
    *startOffset = segment.SegmentStart;
    *endOffset = segment.SegmentEnd;

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
   * Get special text at some position.
   * @param offset Special position.
   * @param boundaryType Boundary type.
   * @param startOffset Variant to accept the start offset.
   * @param endOffset   Variant to accept the end offset.
   * @param text        Variant to accept the special text.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_textAtOffset(long offset, IA2TextBoundaryType boundaryType, long * startOffset, long * endOffset, BSTR * text)
{
    SolarMutexGuard g;

    try {

    if (startOffset == nullptr || text == nullptr ||endOffset == nullptr)
        return E_INVALIDARG;

    if(!pRXText.is())
        return E_FAIL;

    if (boundaryType == IA2_TEXT_BOUNDARY_ALL)
    {
        long nChar;
        get_nCharacters( &nChar );
        *startOffset = 0;
        *endOffset = nChar;
        return get_text(0, nChar, text);
    }

    const sal_Int16 nUnoBoundaryType = lcl_matchIA2TextBoundaryType(boundaryType);
    if (nUnoBoundaryType < 0)
        return E_FAIL;

    TextSegment segment = pRXText->getTextAtIndex(offset, nUnoBoundaryType);
    OUString ouStr = segment.SegmentText;
    SysFreeString(*text);
    *text = SysAllocString(o3tl::toW(ouStr.getStr()));
    *startOffset = segment.SegmentStart;
    *endOffset = segment.SegmentEnd;

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
   * Remove selection.
   * @param selectionIndex Special selection index
   * @param success Variant to accept the method called result.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::removeSelection(long selectionIndex)
{
    SolarMutexGuard g;

    try {

    if(pUNOInterface == nullptr)
    {
        return E_FAIL;
    }

    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();

    Reference< XAccessibleTextSelection > pRExtension(pRContext,UNO_QUERY);

    if( pRExtension.is() )
    {
        pRExtension->removeSelection(selectionIndex);
        return S_OK;
    }
    else
    {
        pRXText->setSelection(0, 0);
        return S_OK;
    }

    } catch(...) { return E_FAIL; }
}

/**
   * Set caret position.
   * @param offset Special position.
   * @param success Variant to accept the method called result.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::setCaretOffset(long offset)
{
    SolarMutexGuard g;

    try {

    if(!pRXText.is())
        return E_FAIL;

    pRXText->setCaretPosition(offset);

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
   * Set special selection.
   * @param selectionIndex Special selection index.
   * @param startOffset start position.
   * @param endOffset end position.
   * @param success Variant to accept the method called result.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::setSelection(long, long startOffset, long endOffset)
{
    SolarMutexGuard g;

    try {

    if(!pRXText.is())
    {
        return E_FAIL;
    }

    pRXText->setSelection(startOffset, endOffset);

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
   * Get characters count.
   * @param nCharacters Variant to accept the characters count.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_nCharacters(long * nCharacters)
{
    SolarMutexGuard g;

    try {

    if (nCharacters == nullptr)
        return E_INVALIDARG;

    if(!pRXText.is())
    {
        *nCharacters = 0;
        return S_OK;
    }

    *nCharacters = pRXText->getCharacterCount();

    return S_OK;

    } catch(...) { return E_FAIL; }
}

// added by qiuhd, 2006/07/03, for direver 07/11
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_newText( IA2TextSegment *)
{
    return E_NOTIMPL;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_oldText( IA2TextSegment *)
{
    return E_NOTIMPL;
}

/**
   * Scroll to special sub-string .
   * @param startIndex Start index of sub string.
   * @param endIndex   End index of sub string.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::scrollSubstringToPoint(long, long, IA2CoordinateType, long, long )
{
    return E_NOTIMPL;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::scrollSubstringTo(long startIndex, long endIndex, IA2ScrollType type)
{
    SolarMutexGuard g;

    try {

    if(!pRXText.is())
        return E_FAIL;

    AccessibleScrollType lUnoType;

    switch(type)
    {
    case IA2_SCROLL_TYPE_TOP_LEFT:
        lUnoType = AccessibleScrollType_SCROLL_TOP_LEFT;
        break;
    case IA2_SCROLL_TYPE_BOTTOM_RIGHT:
        lUnoType = AccessibleScrollType_SCROLL_BOTTOM_RIGHT;
        break;
    case IA2_SCROLL_TYPE_TOP_EDGE:
        lUnoType = AccessibleScrollType_SCROLL_TOP_EDGE;
        break;
    case IA2_SCROLL_TYPE_BOTTOM_EDGE:
        lUnoType = AccessibleScrollType_SCROLL_BOTTOM_EDGE;
        break;
    case IA2_SCROLL_TYPE_LEFT_EDGE:
        lUnoType = AccessibleScrollType_SCROLL_LEFT_EDGE;
        break;
    case IA2_SCROLL_TYPE_RIGHT_EDGE:
        lUnoType = AccessibleScrollType_SCROLL_RIGHT_EDGE;
        break;
    case IA2_SCROLL_TYPE_ANYWHERE:
        lUnoType = AccessibleScrollType_SCROLL_ANYWHERE;
        break;
    default:
        return E_NOTIMPL;
    }

    if (pRXText->scrollSubstringTo(startIndex, endIndex, lUnoType))
        return S_OK;

    return E_NOTIMPL;

    } catch(...) { return E_FAIL; }
}

/**
   * Put UNO interface.
   * @param pXInterface UNO interface.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::put_XInterface(hyper pXInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    try {

    CUNOXWrapper::put_XInterface(pXInterface);
    //special query.
    if(pUNOInterface == nullptr)
        return E_FAIL;
    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();
    if( !pRContext.is() )
    {
        return E_FAIL;
    }
    Reference<XAccessibleText> pRXI(pRContext,UNO_QUERY);
    pRXText = pRXI;
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
