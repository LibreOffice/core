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

/**
 * AccEditableText.cpp : Implementation of CUAccCOMApp and DLL registration.
 */
#include "stdafx.h"
#include "AccEditableText.h"

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include  <UAccCOM.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

#include <vcl/svapp.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <vector>

using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::beans;

/**
 * Copy a range of text to the clipboard.
 *
 * @param    startOffset    the start offset of copying.
 * @param    endOffset      the end offset of copying.
 * @param    success        the boolean result to be returned.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccEditableText::copyText(long startOffset, long endOffset)
{
    SolarMutexGuard g;

    try {

    // #CHECK XInterface#
    if(!pRXEdtTxt.is())
    {
        return E_FAIL;
    }

    if ( GetXInterface()->copyText( startOffset, endOffset ) )
        return S_OK;

    return E_FAIL;

    } catch(...) { return E_FAIL; }
}

/**
 * Deletes a range of text.
 *
 * @param    startOffset    the start offset of deleting.
 * @param    endOffset      the end offset of deleting.
 * @param    success        the boolean result to be returned.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccEditableText::deleteText(long startOffset, long endOffset)
{
    SolarMutexGuard g;

    try {

    if( !pRXEdtTxt.is() )
        return E_FAIL;

    if( GetXInterface()->deleteText( startOffset, endOffset ) )
        return S_OK;

    return E_FAIL;

    } catch(...) { return E_FAIL; }
}

/**
 * Inserts text at a specified offset.
 *
 * @param    offset    the offset of inserting.
 * @param    text      the text to be inserted.
 * @param    success   the boolean result to be returned.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccEditableText::insertText(long offset, BSTR * text)
{
    SolarMutexGuard g;

    try {

    if (text == nullptr)
        return E_INVALIDARG;

    if( !pRXEdtTxt.is() )
        return E_FAIL;

    OUString ouStr(o3tl::toU(*text));

    if( GetXInterface()->insertText( ouStr, offset ) )
        return S_OK;

    return E_FAIL;

    } catch(...) { return E_FAIL; }
}

/**
 * Cuts a range of text to the clipboard.
 *
 * @param    startOffset    the start offset of cutting.
 * @param    endOffset      the end offset of cutting.
 * @param    success        the boolean result to be returned.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccEditableText::cutText(long startOffset, long endOffset)
{
    SolarMutexGuard g;

    try {

    if( !pRXEdtTxt.is() )
        return E_FAIL;

    if( GetXInterface()->cutText( startOffset, endOffset ) )
        return S_OK;

    return E_FAIL;

    } catch(...) { return E_FAIL; }
}

/**
 * Pastes text from clipboard at specified offset.
 *
 * @param    offset    the offset of pasting.
 * @param    success   the boolean result to be returned.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccEditableText::pasteText(long offset)
{
    SolarMutexGuard g;

    try {

    if( !pRXEdtTxt.is() )
        return E_FAIL;

    if( GetXInterface()->pasteText( offset ) )
        return S_OK;

    return E_FAIL;

    } catch(...) { return E_FAIL; }
}

/**
 * Replaces range of text with new text.
 *
 * @param    startOffset    the start offset of replacing.
 * @param    endOffset      the end offset of replacing.
 * @param    text           the replacing text.
 * @param    success        the boolean result to be returned.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccEditableText::replaceText(long startOffset, long endOffset, BSTR * text)
{
    SolarMutexGuard g;

    try {

    // #CHECK#
    if (text == nullptr)
        return E_INVALIDARG;
    if( !pRXEdtTxt.is() )
        return E_FAIL;

    OUString ouStr(o3tl::toU(*text));

    if( GetXInterface()->replaceText( startOffset,endOffset, ouStr) )
        return S_OK;
    return E_FAIL;

    } catch(...) { return E_FAIL; }
}

/**
 * Sets attributes of range of text.
 *
 * @param    startOffset    the start offset.
 * @param    endOffset      the end offset.
 * @param    attributes     the attribute text.
 * @param    success        the boolean result to be returned.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccEditableText::setAttributes(long startOffset, long endOffset, BSTR * attributes)
{
    SolarMutexGuard g;

    try {

    // #CHECK#
    if (attributes == nullptr)
        return E_INVALIDARG;
    if( !pRXEdtTxt.is() )
        return E_FAIL;

    OUString ouStr(o3tl::toU(*attributes));

    std::vector< OUString > vecAttr;
    for (sal_Int32 nIndex {0}; nIndex >= 0; )
        vecAttr.push_back(ouStr.getToken(0, ';', nIndex));

    Sequence< PropertyValue > beanSeq(vecAttr.size());
    auto beanSeqRange = asNonConstRange(beanSeq);
    for(std::vector<OUString>::size_type i = 0; i < vecAttr.size(); i ++)
    {
        OUString attr = vecAttr[i];
        sal_Int32 nPos = attr.indexOf(':');
        if(nPos > -1)
        {
            OUString attrName = attr.copy(0, nPos);
            OUString attrValue = attr.copy(nPos + 1);
            beanSeqRange[i].Name = attrName;
            get_AnyFromOLECHAR(attrName, attrValue, beanSeqRange[i].Value);
        }
    }

    if( GetXInterface()->setAttributes( startOffset,endOffset, beanSeq) )
        return S_OK;

    return E_FAIL;

    } catch(...) { return E_FAIL; }
}

/**
 * Convert attributes string to Any type.
 *
 * @param   ouName      the string of attribute name.
 * @param   ouValue     the string of attribute value.
 * @param   rAny        the Any object to be returned.
 */
void CAccEditableText::get_AnyFromOLECHAR(std::u16string_view ouName, const OUString &ouValue, Any &rAny)
{
    if(ouName == u"CharBackColor" ||
            ouName == u"CharColor" ||
            ouName == u"ParaAdjust" ||
            ouName == u"ParaFirstLineIndent" ||
            ouName == u"ParaLeftMargin" ||
            ouName == u"ParaRightMargin" ||
            ouName == u"ParaTopMargin" ||
            ouName == u"ParaBottomMargin" ||
            ouName == u"CharFontPitch" )
    {
        // Convert to int.
        // NOTE: CharFontPitch is not implemented in java file.
        sal_Int32 nValue = ouValue.toInt32();
        rAny.setValue(&nValue, cppu::UnoType<sal_Int32>::get());
    }
    else if(ouName == u"CharShadowed" ||
            ouName == u"CharContoured" )
    {
        // Convert to boolean.
        rAny <<= ouValue.toBoolean();
    }
    else if(ouName == u"CharEscapement" ||
            ouName == u"CharStrikeout"  ||
            ouName == u"CharUnderline" ||
            ouName == u"CharFontPitch" )
    {
        // Convert to short.
        short nValue = static_cast<short>(ouValue.toInt32());
        rAny.setValue(&nValue, cppu::UnoType<short>::get());
    }
    else if(ouName == u"CharHeight" ||
            ouName == u"CharWeight" )
    {
        // Convert to float.
        float fValue = ouValue.toFloat();
        rAny.setValue(&fValue, cppu::UnoType<float>::get());
    }
    else if(ouName == u"CharFontName" )
    {
        // Convert to string.
        rAny.setValue(&ouValue, cppu::UnoType<OUString>::get());
    }
    else if(ouName == u"CharPosture" )
    {
        // Convert to FontSlant.
        css::awt::FontSlant fontSlant = static_cast<css::awt::FontSlant>(ouValue.toInt32());
        rAny.setValue(&fontSlant, cppu::UnoType<css::awt::FontSlant>::get());
    }
    else if(ouName == u"ParaTabStops" )
    {

        // Convert to the Sequence with TabStop element.
        std::vector< css::style::TabStop > vecTabStop;
        css::style::TabStop tabStop;
        OUString ouSubValue;
        sal_Int32 pos = 0, posComma = 0;

        do
        {
            // Position.
            pos = ouValue.indexOf("Position=", pos);
            if(pos != -1)
            {
                posComma = ouValue.indexOf(',', pos + 9); // 9 = length of "Position=".
                if(posComma != -1)
                {
                    ouSubValue = ouValue.copy(pos + 9, posComma - pos - 9);
                    tabStop.Position = ouSubValue.toInt32();
                    pos = posComma + 1;

                    // TabAlign.
                    pos = ouValue.indexOf("TabAlign=", pos);
                    if(pos != -1)
                    {
                        posComma = ouValue.indexOf(',', pos + 9); // 9 = length of "TabAlign=".
                        if(posComma != -1)
                        {
                            ouSubValue = ouValue.copy(pos + 9, posComma - pos - 9);
                            tabStop.Alignment = static_cast<css::style::TabAlign>(ouSubValue.toInt32());
                            pos = posComma + 1;

                            // DecimalChar.
                            pos = ouValue.indexOf("DecimalChar=", pos);
                            if(pos != -1)
                            {
                                posComma = ouValue.indexOf(',', pos + 11); // 11 = length of "TabAlign=".
                                if(posComma != -1)
                                {
                                    ouSubValue = ouValue.copy(pos + 11, posComma - pos - 11);
                                    tabStop.DecimalChar = ouSubValue.toChar();
                                    pos = posComma + 1;

                                    // FillChar.
                                    pos = ouValue.indexOf("FillChar=", pos);
                                    if(pos != -1)
                                    {
                                        posComma = ouValue.indexOf(',', pos + 9); // 9 = length of "TabAlign=".
                                        if(posComma != -1)
                                        {
                                            ouSubValue = ouValue.copy(pos + 9, posComma - pos - 9);
                                            tabStop.DecimalChar = ouSubValue.toChar();
                                            pos = posComma + 1;

                                            // Complete TabStop element.
                                            vecTabStop.push_back(tabStop);
                                        }
                                        else
                                            break;  // No match comma.
                                    }
                                    else
                                        break;  // No match FillChar.
                                }
                                else
                                    break;  // No match comma.
                            }
                            else
                                break;  // No match DecimalChar.
                        }
                        else
                            break;  // No match comma.
                    }
                    else
                        break;  // No match TabAlign.
                }
                else
                    break;  // No match comma.
            }
            else
                break;  // No match Position.
        }
        while(pos < ouValue.getLength());


        // Dump into Sequence.
        int iSeqLen = vecTabStop.empty() ? 1 : vecTabStop.size();
        Sequence< css::style::TabStop > seqTabStop(iSeqLen);
        auto pseqTabStop = seqTabStop.getArray();

        if (!vecTabStop.empty())
        {
            // Dump every element.
            for(int i = 0; i < iSeqLen; i ++)
            {
                pseqTabStop[i] = vecTabStop[i];
            }
        }
        else
        {
            // Create default value.
            pseqTabStop[0].Position = 0;
            pseqTabStop[0].Alignment = css::style::TabAlign_DEFAULT;
            pseqTabStop[0].DecimalChar = '.';
            pseqTabStop[0].FillChar = ' ';
        }

        // Assign to Any object.
        rAny.setValue(&seqTabStop, cppu::UnoType<Sequence< css::style::TabStop >>::get());
    }
    else if(ouName == u"ParaLineSpacing" )
    {
        // Parse value string.
        css::style::LineSpacing lineSpacing;
        OUString ouSubValue;
        sal_Int32 pos = 0, posComma = 0;

        pos = ouValue.indexOf("Mode=", pos);
        if(pos != -1)
        {
            posComma = ouValue.indexOf(',', pos + 5); // 5 = length of "Mode=".
            if(posComma != -1)
            {
                ouSubValue = ouValue.copy(pos + 5, posComma - pos - 5);
                lineSpacing.Mode = static_cast<sal_Int16>(ouSubValue.toInt32());
                pos = posComma + 1;

                pos = ouValue.indexOf("Height=", pos);
                if(pos != -1)
                {
                    ouSubValue = ouValue.copy(pos + 7);
                    lineSpacing.Height = static_cast<sal_Int16>(ouSubValue.toInt32());
                }
                else
                {
                    lineSpacing.Height = sal_Int16(100);    // Default height.
                }
            }
            else
            {
                lineSpacing.Height = sal_Int16(100);    // Default height.
            }
        }
        else
        {
            // Default Mode and Height.
            lineSpacing.Mode = sal_Int16(0);
            lineSpacing.Height = sal_Int16(100);    // Default height.
        }

        // Convert to Any object.
        rAny.setValue(&lineSpacing, cppu::UnoType<css::style::LineSpacing>::get());
    }
    else
    {
        // Do nothing.
        sal_Int32 nDefault = 0;
        rAny.setValue(&nDefault, cppu::UnoType<sal_Int32>::get());
    }
}

/**
 * Override of IUNOXWrapper.
 *
 * @param    pXInterface    the pointer of UNO interface.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccEditableText::put_XInterface(hyper pXInterface)
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
    Reference<XAccessibleEditableText> pRXI(pRContext,UNO_QUERY);
    if( !pRXI.is() )
        pRXEdtTxt = nullptr;
    else
        pRXEdtTxt = pRXI.get();
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
