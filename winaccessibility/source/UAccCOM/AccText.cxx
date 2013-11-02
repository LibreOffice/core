/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "stdafx.h"
#include "UAccCOM2.h"
#include "AccText.h"

using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;

/**
   * Get special selection.
   * @param startOffset Start selection offset.
   * @param endOffset   End selection offset.
   * @param success     Variant to accept the result of if the method call is successful.
   * @return Result.
*/
STDMETHODIMP CAccText::addSelection(long startOffset, long endOffset)//, unsigned char * success)
{

    return CAccTextBase::get_addSelection(startOffset, endOffset);//, success);
}

/**
   * Get special attributes.
   * @param offset Offset.
   * @param startOffset Variant to accept start offset.
   * @param endOffset   Variant to accept end offset.
   * @param textAttributes     Variant to accept attributes.
   * @return Result.
*/
STDMETHODIMP CAccText::get_attributes(long offset, long * startOffset, long * endOffset, BSTR * textAttributes)
{

    return CAccTextBase::get_attributes(offset, startOffset, endOffset, textAttributes);
}

/**
   * Get caret position.
   * @param offset     Variant to accept caret offset.
   * @return Result.
*/
STDMETHODIMP CAccText::get_caretOffset(long * offset)
{

    return CAccTextBase::get_caretOffset(offset);
}

/**
   * Get character count.
   * @param nCharacters  Variant to accept character count.
   * @return Result.
*/
STDMETHODIMP CAccText::get_characterCount(long * nCharacters)
{

    return CAccTextBase::get_characterCount(nCharacters);
}

/**
   * Get character extents.
   * @param offset  Offset.
   * @param x Variant to accpet x position.
   * @param y Variant to accpet y position.
   * @param width Variant to accpet width.
   * @param Height Variant to accpet height.
   * @return Result.
*/
STDMETHODIMP CAccText::get_characterExtents(long offset, IA2CoordinateType coordType, long * x, long * y, long * width, long * height)
{

    return CAccTextBase::get_characterExtents(offset, coordType, x, y, width, height);
}

/**
   * Get slections count.
   * @param nSelections Variant to accpet slections count.
   * @return Result.
*/
STDMETHODIMP CAccText::get_nSelections(long * nSelections)
{

    return CAccTextBase::get_nSelections(nSelections);
}

/**
   * Get offset of some special point.
   * @param x X position of one point.
   * @param x Y position of one point.
   * @param coordType Type.
   * @param offset Variant to accept offset.
   * @return Result.
*/

STDMETHODIMP CAccText::get_offsetAtPoint(long x, long y, IA2CoordinateType coordType, long * offset)
{

    return CAccTextBase::get_offsetAtPoint(x, y, coordType, offset);
}

/**
   * Get selection range.
   * @param selection selection count.
   * @param startOffset Variant to accept the start offset of special selection.
   * @param endOffset Variant to accept the end offset of special selection.
   * @return Result.
*/
STDMETHODIMP CAccText::get_selection(long selection, long * startOffset, long * endOffset)
{

    return CAccTextBase::get_selection(selection, startOffset, endOffset);
}

/**
   * Get special text.
   * @param startOffset Start position of special range.
   * @param endOffset   End position of special range.
   * @param text        Variant to accept the text of special range.
   * @return Result.
*/
STDMETHODIMP CAccText::get_text(long startOffset, long endOffset, BSTR * text)
{

    return CAccTextBase::get_text(startOffset, endOffset, text);
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
STDMETHODIMP CAccText::get_textBeforeOffset(long offset, IA2TextBoundaryType boundaryType, long * startOffset, long * endOffset, BSTR * text)
{

    return CAccTextBase::get_textBeforeOffset(offset, boundaryType,
            startOffset, endOffset, text);
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
STDMETHODIMP CAccText::get_textAfterOffset(long offset, IA2TextBoundaryType boundaryType, long * startOffset, long * endOffset, BSTR * text)
{

    return CAccTextBase::get_textAfterOffset(offset, boundaryType,
            startOffset, endOffset, text);
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
STDMETHODIMP CAccText::get_textAtOffset(long offset, IA2TextBoundaryType boundaryType, long * startOffset, long * endOffset, BSTR * text)
{

    return CAccTextBase::get_textAtOffset(offset, boundaryType,
                                          startOffset, endOffset, text);
}

/**
   * Remove selection.
   * @param selectionIndex Special selection index
   * @param success Variant to accept the memthod called result.
   * @return Result.
*/
STDMETHODIMP CAccText::removeSelection(long selectionIndex)//, unsigned char * success)
{

    return CAccTextBase::removeSelection(selectionIndex);//, success);
}

/**
   * Set caret position.
   * @param offset Special position.
   * @param success Variant to accept the memthod called result.
   * @return Result.
*/
STDMETHODIMP CAccText::setCaretOffset(long offset)
{

    return CAccTextBase::setCaretOffset(offset);
}

/**
   * Set special selection.
   * @param selectionIndex Special selection index.
   * @param startOffset start position.
   * @param endOffset end position.
   * @param success Variant to accept the memthod called result.
   * @return Result.
*/

STDMETHODIMP CAccText::setSelection(long selectionIndex, long startOffset, long endOffset)
{

    return CAccTextBase::setSelection(selectionIndex, startOffset,
                                      endOffset);
}

/**
   * Get characters count.
   * @param nCharacters Variant to accept the characters count.
   * @return Result.
*/
STDMETHODIMP CAccText::get_nCharacters(long * nCharacters)
{

    return CAccTextBase::get_nCharacters(nCharacters);
}

STDMETHODIMP CAccText::get_newText( IA2TextSegment *newText)
{
    return CAccTextBase::get_newText(newText);
}

STDMETHODIMP CAccText::get_oldText( IA2TextSegment *oldText)
{
    return CAccTextBase::get_oldText(oldText);
}

/**
   * Scroll to special sub-string .
   * @param startIndex Start index of sub string.
   * @param endIndex   End index of sub string.
   * @return Result.
*/
STDMETHODIMP CAccText::scrollSubstringToPoint(long startIndex, long endIndex,enum IA2CoordinateType coordinateType, long x, long y )
{

    return CAccTextBase::scrollSubstringToPoint(startIndex, endIndex, coordinateType, x, y);
}

STDMETHODIMP CAccText::scrollSubstringTo(long startIndex, long endIndex,enum IA2ScrollType scrollType)
{

    return CAccTextBase::scrollSubstringTo(startIndex, endIndex,scrollType);
}

/**
   * Put UNO interface.
   * @param pXInterface UNO interface.
   * @return Result.
*/
STDMETHODIMP CAccText::put_XInterface(long pXInterface)
{

    return CAccTextBase::put_XInterface(pXInterface);
}

