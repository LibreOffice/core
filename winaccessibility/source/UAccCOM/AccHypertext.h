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

#ifndef INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_ACCHYPERTEXT_H
#define INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_ACCHYPERTEXT_H

#include "resource.h"       // main symbols

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>
#include "AccTextBase.h"

/**
 * CAccHypertext implements IAccessibleHypertext interface.
 */
class ATL_NO_VTABLE CAccHypertext :
            public CComObjectRoot,
            public CComCoClass<CAccHypertext,&CLSID_AccHypertext>,
            public IAccessibleHypertext,
            public CAccTextBase
{
public:
    CAccHypertext()
    {
            }
    ~CAccHypertext()
    {
            }

    BEGIN_COM_MAP(CAccHypertext)
    COM_INTERFACE_ENTRY(IAccessibleText)
    COM_INTERFACE_ENTRY(IAccessibleHypertext)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,_SmartQI)
    END_COM_MAP()

    static HRESULT WINAPI _SmartQI(void* pv,
                                   REFIID iid, void** ppvObject, DWORD_PTR)
    {
        return ((CAccHypertext*)pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    DECLARE_NO_REGISTRY()

public:
    // IAccessibleText

    // Adds a text selection.
    STDMETHOD(addSelection)(long startOffset, long endOffset);//, unsigned char * success)

    // Gets text attributes.
    STDMETHOD(get_attributes)(long offset, long * startOffset, long * endOffset, BSTR * textAttributes);

    // Gets caret offset.
    STDMETHOD(get_caretOffset)(long * offset);

    // Gets total number of characters.
    STDMETHOD(get_characterCount)(long * nCharacters);

    // Gets bounding rect containing the glyph(s) representing the character
    // at the specified text offset
    STDMETHOD(get_characterExtents)(long offset, IA2CoordinateType coordType, long * x, long * y, long * width, long * height);

    // Gets number of active non-contiguous selections.
    STDMETHOD(get_nSelections)(long * nSelections);

    // Gets bounding rect for the glyph at a certain point.
    STDMETHOD(get_offsetAtPoint)(long x, long y, IA2CoordinateType coordType, long * offset);

    // Gets character offsets of N-th active text selection.
    STDMETHOD(get_selection)(long selection, long * startOffset, long * endOffset);

    // Gets a range of text by offset NOTE: returned string may be longer
    // than endOffset-startOffset bytes if text contains multi-byte characters.
    STDMETHOD(get_text)(long startOffset, long endOffset, BSTR * text);

    // Gets a specified amount of text that ends before a specified offset.
    STDMETHOD(get_textBeforeOffset)(long offset, IA2TextBoundaryType boundaryType, long * startOffset, long * endOffset, BSTR * text);

    // Gets a specified amount of text that spans the specified offset.
    STDMETHOD(get_textAfterOffset)(long offset, IA2TextBoundaryType boundaryType, long * startOffset, long * endOffset, BSTR * text);

    // Gets a specified amount of text that starts after a specified offset.
    STDMETHOD(get_textAtOffset)(long offset, IA2TextBoundaryType boundaryType, long * startOffset, long * endOffset, BSTR * text);

    // Unselects a range of text.
    STDMETHOD(removeSelection)(long selectionIndex);

    // Moves text caret.
    STDMETHOD(setCaretOffset)(long offset);

    // Changes the bounds of an existing selection.
    STDMETHOD(setSelection)(long selectionIndex, long startOffset, long endOffset);

    // Gets total number of characters.
    // NOTE: this may be different than the total number of bytes required
    // to store the text, if the text contains multi-byte characters.
    STDMETHOD(get_nCharacters)(long * nCharacters);

    // Makes specific part of string visible on screen.
    STDMETHOD(scrollSubstringTo)(long startIndex, long endIndex,enum IA2ScrollType scrollType);

    STDMETHOD(scrollSubstringToPoint)(long startIndex, long endIndex,enum IA2CoordinateType coordinateType, long x, long y );

    STDMETHOD(get_newText)( IA2TextSegment *newText);

    STDMETHOD(get_oldText)( IA2TextSegment *oldText);

    //IAccessibleHypertext

    // Gets the number of hyperlink.
    STDMETHOD(get_nHyperlinks)(long *hyperlinkCount);

    // Gets the hyperlink object via specified index.
    STDMETHOD(get_hyperlink)(long index,IAccessibleHyperlink **hyperlink);

    // Returns the index of the hyperlink that is associated with this
    // character index.
    STDMETHOD(get_hyperlinkIndex)(long charIndex, long *hyperlinkIndex);

    // Override of IUNOXWrapper.
    STDMETHOD(put_XInterface)(hyper pXInterface);

private:

    css::uno::Reference<css::accessibility::XAccessibleHypertext> pHyperText;

};

#endif // INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_ACCHYPERTEXT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
