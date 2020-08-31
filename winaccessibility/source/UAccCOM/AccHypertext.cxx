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

#include "stdafx.h"
#include "AccHypertext.h"
#include "AccHyperLink.h"
#include "acccommon.h"

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include  <UAccCOM.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

#include <vcl/svapp.hxx>


using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;


/**
   * Get special selection.
   * @param startOffset Start selection offset.
   * @param endOffset   End selection offset.
   * @param success     Variant to accept the result of if the method call is successful.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::addSelection(long startOffset, long endOffset)
{

    return CAccTextBase::get_addSelection(startOffset, endOffset);
}


/**
   * Get special attributes.
   * @param offset Offset.
   * @param startOffset Variant to accept start offset.
   * @param endOffset   Variant to accept end offset.
   * @param textAttributes     Variant to accept attributes.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_attributes(long offset, long * startOffset, long * endOffset, BSTR * textAttributes)
{

    return CAccTextBase::get_attributes(offset, startOffset, endOffset, textAttributes);
}

/**
   * Get caret position.
   * @param offset     Variant to accept caret offset.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_caretOffset(long * offset)
{

    return CAccTextBase::get_caretOffset(offset);
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
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_characterExtents(long offset, IA2CoordinateType coordType, long * x, long * y, long * width, long * height)
{

    return CAccTextBase::get_characterExtents(offset, coordType, x, y, width, height);
}

/**
   * Get selections count.
   * @param nSelections Variant to accept selections count.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_nSelections(long * nSelections)
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
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_offsetAtPoint(long x, long y, IA2CoordinateType coordType, long * offset)
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
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_selection(long selection, long * startOffset, long * endOffset)
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
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_text(long startOffset, long endOffset, BSTR * text)
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
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_textBeforeOffset(long offset, IA2TextBoundaryType boundaryType, long * startOffset, long * endOffset, BSTR * text)
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
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_textAfterOffset(long offset, IA2TextBoundaryType boundaryType, long * startOffset, long * endOffset, BSTR * text)
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
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_textAtOffset(long offset, IA2TextBoundaryType boundaryType, long * startOffset, long * endOffset, BSTR * text)
{

    return CAccTextBase::get_textAtOffset(offset, boundaryType,
                                          startOffset, endOffset, text);
}

/**
   * Remove selection.
   * @param selectionIndex Special selection index
   * @param success Variant to accept the method called result.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::removeSelection(long selectionIndex)
{

    return CAccTextBase::removeSelection(selectionIndex);
}

/**
   * Set caret position.
   * @param offset Special position.
   * @param success Variant to accept the method called result.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::setCaretOffset(long offset)
{

    return CAccTextBase::setCaretOffset(offset);
}

/**
   * Set special selection.
   * @param selectionIndex Special selection index.
   * @param startOffset start position.
   * @param endOffset end position.
   * @param success Variant to accept the method called result.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::setSelection(long selectionIndex, long startOffset, long endOffset)
{

    return CAccTextBase::setSelection(selectionIndex, startOffset,
                                      endOffset);
}

/**
   * Get characters count.
   * @param nCharacters Variant to accept the characters count.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_nCharacters(long * nCharacters)
{

    return CAccTextBase::get_nCharacters(nCharacters);
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_newText( IA2TextSegment *newText)
{
    return CAccTextBase::get_newText(newText);
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_oldText( IA2TextSegment *oldText)
{
    return CAccTextBase::get_oldText(oldText);
}

/**
   * Scroll to special sub-string .
   * @param startIndex Start index of sub string.
   * @param endIndex   End index of sub string.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::scrollSubstringToPoint(long startIndex, long endIndex,enum IA2CoordinateType coordinateType, long x, long y )
{

    return CAccTextBase::scrollSubstringToPoint(startIndex, endIndex, coordinateType, x, y);
}
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::scrollSubstringTo(long startIndex, long endIndex,enum IA2ScrollType scrollType)
{

    return CAccTextBase::scrollSubstringTo(startIndex, endIndex,scrollType);
}

/**
   * Get hyperlink count.
   * @param hyperlinkCount Variant to accept hyperlink count.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_nHyperlinks(long *hyperlinkCount)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(hyperlinkCount == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pHyperText.is())
    {
        return E_FAIL;
    }

    *hyperlinkCount = pHyperText->getHyperLinkCount();
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Get special hyperlink.
   * @param index Special hyperlink index.
   * @param hyperlink Variant to accept special hyperlink via index.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_hyperlink(long index,IAccessibleHyperlink **hyperlink)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(hyperlink == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pHyperText.is())
    {
        return E_FAIL;
    }

    Reference<XAccessibleHyperlink> pRLink = pHyperText->getHyperLink(index);
    if(!pRLink.is())
    {
        *hyperlink = nullptr;
        return E_FAIL;
    }

    IAccessibleHyperlink* plink = nullptr;
    HRESULT hr = createInstance<CAccHyperLink>(IID_IAccessibleHyperlink, &plink);
    if( SUCCEEDED(hr) )
    {
        IUNOXWrapper* wrapper = nullptr;
        plink->QueryInterface(IID_IUNOXWrapper, reinterpret_cast<void**>(&wrapper));
        if(wrapper)
        {
            wrapper->put_XSubInterface(reinterpret_cast<hyper>(pRLink.get()));
            wrapper->Release();
        }
        *hyperlink = plink;
        return S_OK;
    }

    return E_FAIL;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Returns the index of the hyperlink that is associated with this character index.
   * @param charIndex Special char index.
   * @param hyperlinkIndex Variant to accept special hyperlink index.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::get_hyperlinkIndex(long charIndex, long *hyperlinkIndex)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(hyperlinkIndex == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pHyperText.is())
    {
        return E_FAIL;
    }

    *hyperlinkIndex = pHyperText->getHyperLinkIndex(charIndex);
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Put UNO interface.
   * @param pXInterface UNO interface.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccHypertext::put_XInterface(hyper pXInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    ENTER_PROTECTED_BLOCK

    CAccTextBase::put_XInterface(pXInterface);
    //special query.
    if(pUNOInterface == nullptr)
        return E_FAIL;
    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();
    if( !pRContext.is() )
    {
        return E_FAIL;
    }
    Reference<XAccessibleHypertext> pRXI(pRContext,UNO_QUERY);
    if( !pRXI.is() )
        pHyperText = nullptr;
    else
        pHyperText = pRXI.get();
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
