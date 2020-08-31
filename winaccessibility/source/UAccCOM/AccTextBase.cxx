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


// Construction/Destruction


static OUString ReplaceFourChar(OUString const & oldOUString);

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

    ENTER_PROTECTED_BLOCK

    // #CHECK XInterface#
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
        GetXInterface()->setSelection(startOffset, endOffset);
        return S_OK;
    }

    LEAVE_PROTECTED_BLOCK
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

    ENTER_PROTECTED_BLOCK

    if (startOffset == nullptr || endOffset == nullptr || textAttributes == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXText.is())
    {
        return E_FAIL;
    }

    if( offset < 0 || offset > GetXInterface()->getCharacterCount() )
        return E_FAIL;

    OUStringBuffer strAttrs("Version:1;");

    Sequence< css::beans::PropertyValue > pValues = GetXInterface()->getCharacterAttributes(offset, Sequence< OUString >());
    int nCount = pValues.getLength();

    sal_Int16 numberingLevel = 0;
    OUString numberingPrefix;
    Any anyNumRule;
    bool bHaveNumberingPrefixAttr = false;
    bool bHaveNumberingLevel = false;
    bool bHaveNumberingRules = false;
    for(int i =0; i<nCount; i++)
    {

        css::beans::PropertyValue &pValue = pValues[i];
        if(pValue.Name == "NumberingLevel")
        {
            if (pValue.Value != Any())
                pValue.Value >>= numberingLevel;
            else
                numberingLevel = -1;
            bHaveNumberingLevel = true;
            continue;
        }
        if(pValue.Name == "NumberingPrefix")
        {
            pValue.Value >>=numberingPrefix;
            bHaveNumberingPrefixAttr = true;
            continue;
        }
        if(pValue.Name == "NumberingRules")
        {
            bHaveNumberingRules = true;
            anyNumRule = pValue.Value;
            continue;
        }
        if (bHaveNumberingLevel && bHaveNumberingRules && bHaveNumberingPrefixAttr)
        {
            strAttrs.append(';');
            numberingPrefix = ReplaceFourChar(numberingPrefix);
            strAttrs.append(CMAccessible::get_String4Numbering(anyNumRule,numberingLevel,numberingPrefix));
            bHaveNumberingLevel = false;
            bHaveNumberingRules = false;
        }
        if( (bHaveNumberingPrefixAttr && i > 1 ) ||
            (!bHaveNumberingPrefixAttr && i > 0 ) ) //element 0 is NumberingPrefix, not write alone
        {
            strAttrs.append(';');
        }
        strAttrs.append(pValue.Name);
        strAttrs.append(':');

        if (pValue.Name == "CharBackColor" ||
                pValue.Name == "CharColor" ||
                pValue.Name == "CharUnderlineColor" )
        {
            unsigned long nColor;
            pValue.Value >>= nColor;
            strAttrs.append('#');
            OUString const hex = OUString::number(nColor, 16).toAsciiUpperCase();
            for (sal_Int32 j = hex.getLength(); j < 8; ++j) {
                strAttrs.append('0');
            }
            strAttrs.append(hex);
        }
        else
        {
            strAttrs.append(CMAccessible::get_StringFromAny(pValue.Value));
        }
    }
    strAttrs.append(';');
    // #CHECK#
    if(*textAttributes)
        SysFreeString(*textAttributes);
    *textAttributes = SysAllocString(o3tl::toW(strAttrs.makeStringAndClear().getStr()));

    if( offset < GetXInterface()->getCharacterCount() )
    {
        TextSegment textSeg = GetXInterface()->getTextAtIndex(offset, AccessibleTextType::ATTRIBUTE_RUN);
        *startOffset = textSeg.SegmentStart;
        *endOffset = textSeg.SegmentEnd;
    }
    else
    {
        *startOffset = offset;
        *endOffset = offset;
    }

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Get caret position.
   * @param offset     Variant to accept caret offset.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_caretOffset(long * offset)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if (offset == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXText.is())
    {
        *offset = 0;
        return S_OK;
    }

    *offset = GetXInterface()->getCaretPosition();
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Get character count.
   * @param nCharacters  Variant to accept character count.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_characterCount(long * nCharacters)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if (nCharacters == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXText.is())
    {
        *nCharacters = 0;
        return S_OK;
    }

    *nCharacters = GetXInterface()->getCharacterCount();
    return S_OK;

    LEAVE_PROTECTED_BLOCK
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

    ENTER_PROTECTED_BLOCK

    if (x == nullptr || height == nullptr || y == nullptr || width == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXText.is())
        return E_FAIL;

    if(offset < 0 || offset > GetXInterface()->getCharacterCount() )
        return E_FAIL;

    css::awt::Rectangle rectangle;
    rectangle = GetXInterface()->getCharacterBounds(offset);

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

    // GetXInterface()->getCharacterBounds() have different implement in different acc component
    // But we need return the width/height == 1 for every component when offset == text length.
    // So we ignore the return result of GetXInterface()->getCharacterBounds() when offset == text length.
    if( offset == GetXInterface()->getCharacterCount() )
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

    LEAVE_PROTECTED_BLOCK
}

/**
   * Get selections count.
   * @param nSelections Variant to accept selections count.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_nSelections(long * nSelections)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if (nSelections == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
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

    long iLength = GetXInterface()->getSelectedText().getLength();
    if( iLength> 0)
    {
        *nSelections = 1;
        return S_OK;
    }

    *nSelections = 0;
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Get offset of some special point.
   * @param x X position of one point.
   * @param x Y position of one point.
   * @param coordType Type.
   * @param offset Variant to accept offset.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_offsetAtPoint(long x, long y, IA2CoordinateType, long * offset)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if (offset == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXText.is())
        return E_FAIL;

    css::awt::Point point;
    point.X = x;
    point.Y = y;
    *offset = GetXInterface()->getIndexAtPoint(point);
    return S_OK;

    LEAVE_PROTECTED_BLOCK
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

    ENTER_PROTECTED_BLOCK

    if (startOffset == nullptr || endOffset == nullptr )
        return E_INVALIDARG;
    // #CHECK XInterface#
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
    else if(GetXInterface()->getSelectionEnd() > -1)
    {
        *startOffset = GetXInterface()->getSelectionStart();
        *endOffset = GetXInterface()->getSelectionEnd();
        return S_OK;
    }

    *startOffset = 0;
    *endOffset = 0;
    return E_FAIL;

    LEAVE_PROTECTED_BLOCK
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

    ENTER_PROTECTED_BLOCK

    if (text == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXText.is())
        return E_FAIL;

    if (endOffset < -1 || endOffset < startOffset )
    {
        return E_FAIL;
    }

    OUString ouStr;
    if (endOffset == -1 )
    {
        long nLen=0;
        if(SUCCEEDED(get_characterCount(&nLen)))
        {
            ouStr = GetXInterface()->getTextRange( 0, nLen );
        }
    }
    else
    {
        ouStr = GetXInterface()->getTextRange( startOffset, endOffset );
    }

    SysFreeString(*text);
    *text = SysAllocString(o3tl::toW(ouStr.getStr()));
    return S_OK;

    LEAVE_PROTECTED_BLOCK
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

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if (startOffset == nullptr || endOffset == nullptr || text == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXText.is())
        return E_FAIL;

    // In New UNO IAccessibleText.idl these constant values are defined as follows:

    //  const long TEXT_BOUNDARY_CHAR = -1;
    //  const long TEXT_BOUNDARY_TO_CURSOR_POS = -2;
    //  const long TEXT_BOUNDARY_START_OF_WORD = -3;
    //  const long TEXT_BOUNDARY_END_OF_WORD = -4;
    //  const long TEXT_BOUNDARY_START_OF_SENTENCE = -5;
    //  const long TEXT_BOUNDARY_END_OF_SENTENCE = -6;
    //  const long TEXT_BOUNDARY_START_OF_LINE = -7;
    //  const long TEXT_BOUNDARY_END_OF_LINE = -8;

    // In UNO, the corresponding values are as follows:

    //  const short CHARACTER = 1;
    //  const short WORD = 2;
    //  const short SENTENCE = 3;
    //  const short PARAGRAPH = 4;
    //  const short LINE = 5;
    //  const short GLYPH = 6;
    //  const short ATTRIBUTE_RUN = 7;


    long            lUnoBoundaryType;

    switch(boundaryType)
    {
    case IA2_TEXT_BOUNDARY_CHAR:
        lUnoBoundaryType = 1; // CHARACTER;
        break;
    case IA2_TEXT_BOUNDARY_WORD:
        lUnoBoundaryType = 2; // WORD;
        break;
    case IA2_TEXT_BOUNDARY_SENTENCE:
        lUnoBoundaryType = 3; // SENTENCE;
        break;
    case IA2_TEXT_BOUNDARY_LINE:
        lUnoBoundaryType = 5; // LINE;
        break;
    case IA2_TEXT_BOUNDARY_PARAGRAPH:
        lUnoBoundaryType = 4;
        break;
    case IA2_TEXT_BOUNDARY_ALL:
        {
            long nChar;
            get_nCharacters( &nChar );
            *startOffset = 0;
            *endOffset = nChar;
            return get_text(0, nChar, text);
        }
        break;
    default:
        return E_FAIL;
    }

    TextSegment segment = GetXInterface()->getTextBeforeIndex( offset, sal_Int16(lUnoBoundaryType));
    OUString ouStr = segment.SegmentText;
    SysFreeString(*text);
    *text = SysAllocString(o3tl::toW(ouStr.getStr()));
    *startOffset = segment.SegmentStart;
    *endOffset = segment.SegmentEnd;

    return S_OK;

    LEAVE_PROTECTED_BLOCK
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

    ENTER_PROTECTED_BLOCK

    if (startOffset == nullptr || endOffset == nullptr || text == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXText.is())
        return E_FAIL;

    // In New UNO IAccessibleText.idl these constant values are defined as follows:

    //  const long TEXT_BOUNDARY_CHAR = -1;
    //  const long TEXT_BOUNDARY_TO_CURSOR_POS = -2;
    //  const long TEXT_BOUNDARY_START_OF_WORD = -3;
    //  const long TEXT_BOUNDARY_END_OF_WORD = -4;
    //  const long TEXT_BOUNDARY_START_OF_SENTENCE = -5;
    //  const long TEXT_BOUNDARY_END_OF_SENTENCE = -6;
    //  const long TEXT_BOUNDARY_START_OF_LINE = -7;
    //  const long TEXT_BOUNDARY_END_OF_LINE = -8;

    // In UNO, the corresponding values are as follows:

    //  const short CHARACTER = 1;
    //  const short WORD = 2;
    //  const short SENTENCE = 3;
    //  const short PARAGRAPH = 4;
    //  const short LINE = 5;
    //  const short GLYPH = 6;
    //  const short ATTRIBUTE_RUN = 7;


    long            lUnoBoundaryType;
    switch(boundaryType)
    {
    case IA2_TEXT_BOUNDARY_CHAR:
        lUnoBoundaryType = 1; // CHARACTER;
        break;
    case IA2_TEXT_BOUNDARY_WORD:
        lUnoBoundaryType = 2; // WORD;
        break;
    case IA2_TEXT_BOUNDARY_SENTENCE:
        lUnoBoundaryType = 3; // SENTENCE;
        break;
    case IA2_TEXT_BOUNDARY_LINE:
        lUnoBoundaryType = 5; // LINE;
        break;
    case IA2_TEXT_BOUNDARY_PARAGRAPH:
        lUnoBoundaryType = 4;
        break;
    case IA2_TEXT_BOUNDARY_ALL:
        {
            long nChar;
            get_nCharacters( &nChar );
            *startOffset = 0;
            *endOffset = nChar;
            return get_text(0, nChar, text);
        }
        break;
    default:
        return E_FAIL;
    }

    TextSegment segment = GetXInterface()->getTextBehindIndex( offset, sal_Int16(lUnoBoundaryType));
    OUString ouStr = segment.SegmentText;
    SysFreeString(*text);
    *text = SysAllocString(o3tl::toW(ouStr.getStr()));
    *startOffset = segment.SegmentStart;
    *endOffset = segment.SegmentEnd;

    return S_OK;

    LEAVE_PROTECTED_BLOCK
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

    ENTER_PROTECTED_BLOCK

    if (startOffset == nullptr || text == nullptr ||endOffset == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXText.is())
        return E_FAIL;

    // In New UNO IAccessibleText.idl these constant values are defined as follows:

    //  const long TEXT_BOUNDARY_CHAR = -1;
    //  const long TEXT_BOUNDARY_TO_CURSOR_POS = -2;
    //  const long TEXT_BOUNDARY_START_OF_WORD = -3;
    //  const long TEXT_BOUNDARY_END_OF_WORD = -4;
    //  const long TEXT_BOUNDARY_START_OF_SENTENCE = -5;
    //  const long TEXT_BOUNDARY_END_OF_SENTENCE = -6;
    //  const long TEXT_BOUNDARY_START_OF_LINE = -7;
    //  const long TEXT_BOUNDARY_END_OF_LINE = -8;

    // In UNO, the corresponding values are as follows:

    //  const short CHARACTER = 1;
    //  const short WORD = 2;
    //  const short SENTENCE = 3;
    //  const short PARAGRAPH = 4;
    //  const short LINE = 5;
    //  const short GLYPH = 6;
    //  const short ATTRIBUTE_RUN = 7;


    long            lUnoBoundaryType;

    switch(boundaryType)
    {
    case IA2_TEXT_BOUNDARY_CHAR:
        lUnoBoundaryType = 1; // CHARACTER;
        break;
    case IA2_TEXT_BOUNDARY_WORD:
        lUnoBoundaryType = 2; // WORD;
        break;
    case IA2_TEXT_BOUNDARY_SENTENCE:
        lUnoBoundaryType = 3; // SENTENCE;
        break;
    case IA2_TEXT_BOUNDARY_LINE:
        lUnoBoundaryType = 5; // LINE;
        break;
    case IA2_TEXT_BOUNDARY_PARAGRAPH:
        lUnoBoundaryType = 4;
        break;
    case IA2_TEXT_BOUNDARY_ALL:
        {
            long nChar;
            get_nCharacters( &nChar );
            *startOffset = 0;
            *endOffset = nChar;
            return get_text(0, nChar, text);
        }
        break;
    default:
        return E_FAIL;
    }

    TextSegment segment = GetXInterface()->getTextAtIndex( offset, sal_Int16(lUnoBoundaryType));
    OUString ouStr = segment.SegmentText;
    SysFreeString(*text);
    *text = SysAllocString(o3tl::toW(ouStr.getStr()));
    *startOffset = segment.SegmentStart;
    *endOffset = segment.SegmentEnd;

    return S_OK;

    LEAVE_PROTECTED_BLOCK
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

    ENTER_PROTECTED_BLOCK

    // #CHECK XInterface#
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
        GetXInterface()->setSelection(0, 0);
        return S_OK;
    }

    LEAVE_PROTECTED_BLOCK
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

    ENTER_PROTECTED_BLOCK

    // #CHECK XInterface#
    if(!pRXText.is())
        return E_FAIL;

    GetXInterface()->setCaretPosition( offset);

    return S_OK;

    LEAVE_PROTECTED_BLOCK
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

    ENTER_PROTECTED_BLOCK

    // #CHECK XInterface#
    if(!pRXText.is())
    {
        return E_FAIL;
    }

    GetXInterface()->setSelection( startOffset, endOffset );

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Get characters count.
   * @param nCharacters Variant to accept the characters count.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::get_nCharacters(long * nCharacters)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if (nCharacters == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXText.is())
    {
        *nCharacters = 0;
        return S_OK;
    }

    *nCharacters = GetXInterface()->getCharacterCount();

    return S_OK;

    LEAVE_PROTECTED_BLOCK
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

    ENTER_PROTECTED_BLOCK

    // #CHECK XInterface#
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

    if( GetXInterface()->scrollSubstringTo(startIndex, endIndex, lUnoType) )
        return S_OK;

    return E_NOTIMPL;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Put UNO interface.
   * @param pXInterface UNO interface.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTextBase::put_XInterface(hyper pXInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    ENTER_PROTECTED_BLOCK

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
    if( !pRXI.is() )
        pRXText = nullptr;
    else
        pRXText = pRXI;
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

static OUString ReplaceOneChar(OUString const & oldOUString, OUString const & replacedChar, OUString const & replaceStr)
{
    auto s = oldOUString;
    int iReplace = s.lastIndexOf(replacedChar);
    if (iReplace > -1)
    {
        for(;iReplace>-1;)
        {
            s = s.replaceAt(iReplace,1, replaceStr);
            iReplace=s.lastIndexOf(replacedChar,iReplace);
        }
    }
    return s;
}

static OUString ReplaceFourChar(OUString const & oldOUString)
{
    auto s = oldOUString;
    s = ReplaceOneChar(s, "\\", "\\\\");
    s = ReplaceOneChar(s, ";", "\\;");
    s = ReplaceOneChar(s, "=", "\\=");
    s = ReplaceOneChar(s, ",", "\\,");
    s = ReplaceOneChar(s, ":", "\\:");
    return s;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
