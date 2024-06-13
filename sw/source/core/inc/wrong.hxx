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

#pragma once

#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XStringKeyMap.hpp>

#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <vector>
#include <memory>
#include <optional>

#include <tools/color.hxx>
#include <swtypes.hxx>
#include <viewopt.hxx>
#include "TextFrameIndex.hxx"

#if defined _MSC_VER
// For MSVC (without /vmg) SwTextNode must consistently be defined for
// WrongListIterator::m_pGetWrongList of pointer-to-SwTextNode-member type to consistently have the
// same size in all translation units that include this file:
#include <ndtxt.hxx>
#endif

class SwWrongList;

enum WrongAreaLineType
{
    WRONGAREA_NONE,
    WRONGAREA_WAVE,
    WRONGAREA_BOLDWAVE,
    WRONGAREA_BOLD,
    WRONGAREA_DASHED
};

enum WrongListType
{
    WRONGLIST_SPELL,
    WRONGLIST_GRAMMAR,
    WRONGLIST_SMARTTAG,
    WRONGLIST_CHANGETRACKING
};

// ST2
class SwWrongArea
{
public:
    OUString maType;
    css::uno::Reference< css::container::XStringKeyMap > mxPropertyBag;
    sal_Int32 mnPos;
    sal_Int32 mnLen;
    SwWrongList* mpSubList;

    Color mColor;
    WrongAreaLineType mLineType;

    SwWrongArea( OUString aType,
                 WrongListType listType,
                 css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag,
                 sal_Int32 nPos,
                 sal_Int32 nLen);

    SwWrongArea( OUString aType,
                 css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag,
                 sal_Int32 nPos,
                 sal_Int32 nLen,
                 SwWrongList* pSubList);
private:

    static Color getGrammarColor ( css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag)
    {
        try
        {
            if (xPropertyBag.is())
            {
                css::uno::Any aLineColor = xPropertyBag->getValue(u"LineColor"_ustr);
                ::Color lineColor;

                if (aLineColor >>= lineColor)
                {
                    return lineColor;
                }
            }
        }
        catch(const css::container::NoSuchElementException&)
        {
        }
        catch(const css::uno::RuntimeException&)
        {
        }

        return SwViewOption::GetCurrentViewOptions().GetGrammarColor();
    }

    static WrongAreaLineType getGrammarLineType( css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag )
    {
        try
        {
            if (xPropertyBag.is())
            {
                css::uno::Any aLineType = xPropertyBag->getValue(u"LineType"_ustr);
                ::sal_Int16 lineType = 0;

                if (!(aLineType >>= lineType))
                {
                    return WRONGAREA_WAVE;
                }
                if (css::awt::FontUnderline::BOLDWAVE == lineType)
                {
                    return WRONGAREA_BOLDWAVE;
                }
                if (css::awt::FontUnderline::BOLD == lineType)
                {
                    return WRONGAREA_BOLD;
                }
                if (css::awt::FontUnderline::DASH == lineType)
                {
                    return WRONGAREA_DASHED;
                }
                if (css::awt::FontUnderline::SMALLWAVE == lineType)
                {
                    return WRONGAREA_WAVE; //Code draws wave height based on space that fits.
                }
            }
        }
        catch(const css::container::NoSuchElementException&)
        {
        }
        catch(const css::uno::RuntimeException&)
        {
        }

        return WRONGAREA_WAVE;
    }

    static Color getSmartColor ( css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag)
    {
        try
        {
            if (xPropertyBag.is())
            {
                css::uno::Any aLineColor = xPropertyBag->getValue(u"LineColor"_ustr);
                ::Color lineColor;

                if (aLineColor >>= lineColor)
                {
                    return lineColor;
                }
            }
        }
        catch(const css::container::NoSuchElementException&)
        {
        }
        catch(const css::uno::RuntimeException&)
        {
        }

        return SwViewOption::GetCurrentViewOptions().GetSmarttagColor();
    }

    static WrongAreaLineType getSmartLineType( css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag )
    {
        try
        {
            if (xPropertyBag.is())
            {
                css::uno::Any aLineType = xPropertyBag->getValue(u"LineType"_ustr);
                ::sal_Int16 lineType = 0;

                if (!(aLineType >>= lineType))
                {
                    return WRONGAREA_DASHED;
                }
                if (css::awt::FontUnderline::WAVE == lineType)
                {
                    return WRONGAREA_WAVE;
                }
                if (css::awt::FontUnderline::BOLDWAVE == lineType)
                {
                    return WRONGAREA_BOLDWAVE;
                }
                if (css::awt::FontUnderline::BOLD == lineType)
                {
                    return WRONGAREA_BOLD;
                }
                if (css::awt::FontUnderline::SMALLWAVE == lineType)
                {
                    return WRONGAREA_WAVE; //Code draws wave height based on space that fits.
                }
            }
        }
        catch(const css::container::NoSuchElementException&)
        {
        }
        catch(const css::uno::RuntimeException&)
        {
        }

        return WRONGAREA_DASHED;
    }

    static Color getWrongAreaColor(WrongListType listType,
                            css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag )
    {
        if (WRONGLIST_SPELL == listType)
        {
            return SwViewOption::GetCurrentViewOptions().GetSpellColor();
        }
        else if (WRONGLIST_GRAMMAR == listType)
        {
            return getGrammarColor(xPropertyBag);
        }
        else if (WRONGLIST_SMARTTAG == listType)
        {
            return  getSmartColor(xPropertyBag);
        }

        return SwViewOption::GetCurrentViewOptions().GetSpellColor();
    }

    static WrongAreaLineType getWrongAreaLineType(WrongListType listType,
                                           css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag )
    {
        if (WRONGLIST_SPELL == listType)
        {
            return WRONGAREA_WAVE;
        }
        else if (WRONGLIST_GRAMMAR == listType)
        {
            return getGrammarLineType(xPropertyBag);
        }
        else if (WRONGLIST_SMARTTAG == listType)
        {
            return getSmartLineType(xPropertyBag);
        }

        return WRONGAREA_WAVE;
    }

};

class SAL_DLLPUBLIC_RTTI SwWrongList
{
    std::vector<SwWrongArea> maList;
    WrongListType            meType;

    sal_Int32 mnBeginInvalid;   // Start of the invalid range
    sal_Int32 mnEndInvalid;     // End of the invalid range

    static void ShiftLeft( sal_Int32 &rPos, sal_Int32 nStart, sal_Int32 nEnd )
    { if( rPos > nStart ) rPos = rPos > nEnd ? rPos - nEnd + nStart : nStart; }
    void Invalidate_( sal_Int32 nBegin, sal_Int32 nEnd );

    void Insert(sal_uInt16 nWhere, std::vector<SwWrongArea>::iterator startPos, std::vector<SwWrongArea>::iterator const & endPos);
    void Remove( sal_uInt16 nIdx, sal_uInt16 nLen );

    SwWrongList& operator= (const SwWrongList &) = delete;
    SwWrongList( const SwWrongList& rCpy ) = delete;

public:
    SwWrongList( WrongListType eType );

    virtual ~SwWrongList();
    virtual SwWrongList* Clone();
    virtual void CopyFrom( const SwWrongList& rCopy );

    WrongListType GetWrongListType() const { return meType; }
    sal_Int32 GetBeginInv() const { return mnBeginInvalid; }
    sal_Int32 GetEndInv() const { return mnEndInvalid; }
    void SetInvalid( sal_Int32 nBegin, sal_Int32 nEnd );
    void Validate(){ mnBeginInvalid = mnEndInvalid = COMPLETE_STRING; }
    void Invalidate( sal_Int32 nBegin, sal_Int32 nEnd );
    bool InvalidateWrong();
    enum class FreshState { FRESH, CURSOR, NOTHING };
    FreshState Fresh( sal_Int32 &rStart, sal_Int32 &rEnd, sal_Int32 nPos,
            sal_Int32 nLen, sal_uInt16 nIndex, sal_Int32 nCursorPos );
    sal_uInt16 GetWrongPos( sal_Int32 nValue ) const;

    bool Check( sal_Int32 &rChk, sal_Int32 &rLn ) const;
    bool InWrongWord( sal_Int32 &rChk, sal_Int32 &rLn ) const;
    sal_Int32 NextWrong( sal_Int32 nChk ) const;

    void Move( sal_Int32 nPos, sal_Int32 nDiff );
    void ClearList();

    // Divide the list into two part, the wrong words until nSplitPos will be
    // removed and transferred to a new SwWrongList.
    std::unique_ptr<SwWrongList> SplitList( sal_Int32 nSplitPos );
    // Join the next SwWrongList, nInsertPos is my own text length, where
    // the other wrong list has to be inserted.
    void JoinList( SwWrongList* pNext, sal_Int32 nInsertPos );

    sal_Int32 Len( sal_uInt16 nIdx ) const
    {
        return nIdx < maList.size() ? maList[nIdx].mnLen : 0;
    }

    sal_Int32 Pos( sal_uInt16 nIdx ) const
    {
        return nIdx < maList.size() ? maList[nIdx].mnPos : 0;
    }

    sal_uInt16 Count() const { return o3tl::narrowing<sal_uInt16>(maList.size()); }

    void Insert( const OUString& rType,
                        css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag,
                        sal_Int32 nNewPos, sal_Int32 nNewLen, sal_uInt16 nWhere )
    {
        std::vector<SwWrongArea>::iterator i = maList.begin();
        if ( nWhere >= maList.size() )
            i = maList.end(); // robust
        else
            i += nWhere;

        maList.insert(i, SwWrongArea( rType, meType, xPropertyBag, nNewPos, nNewLen) );
    }

    void Insert( const OUString& rType,
                 css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag,
                 sal_Int32 nNewPos, sal_Int32 nNewLen );

    SwWrongList* SubList( sal_uInt16 nIdx ) const
    {
        return nIdx < maList.size() ? maList[nIdx].mpSubList : nullptr;
    }

    void InsertSubList( sal_Int32 nNewPos, sal_Int32 nNewLen, sal_uInt16 nWhere, SwWrongList* pSubList );

    const SwWrongArea* GetElement( sal_uInt16 nIdx ) const
    {
        return nIdx < maList.size() ? &maList[nIdx] : nullptr;
    }
    void RemoveEntry( sal_Int32 nBegin, sal_Int32 nEnd );
    bool LookForEntry( sal_Int32 nBegin, sal_Int32 nEnd );
};

class SwTextNode;
class SwTextFrame;

namespace sw {

struct MergedPara;

class WrongListIteratorBase
{
protected:
    SwWrongList const* (SwTextNode::*const m_pGetWrongList)() const;
    sw::MergedPara const*const m_pMergedPara;
    size_t m_CurrentExtent;
    TextFrameIndex m_CurrentIndex;
    SwWrongList const*const m_pWrongList;

public:
    /// for the text frame
    WrongListIteratorBase(SwTextFrame const& rFrame,
        SwWrongList const* (SwTextNode::*pGetWrongList)() const);
    /// for SwTextSlot
    WrongListIteratorBase(SwWrongList const& rWrongList);
};

class WrongListIterator
    : public WrongListIteratorBase
{
public:
    /// for the text frame
    WrongListIterator(SwTextFrame const& rFrame,
        SwWrongList const* (SwTextNode::*pGetWrongList)() const);
    /// for SwTextSlot
    WrongListIterator(SwWrongList const& rWrongList);

    bool Check(TextFrameIndex &rStart, TextFrameIndex &rLen);
    const SwWrongArea* GetWrongElement(TextFrameIndex nStart);

    bool LooksUseful() { return m_pMergedPara || m_pWrongList; }
};

class WrongListIteratorCounter
    : public WrongListIteratorBase
{
public:
    WrongListIteratorCounter(SwTextFrame const& rFrame,
        SwWrongList const* (SwTextNode::*pGetWrongList)() const);
    WrongListIteratorCounter(SwWrongList const& rWrongList);

    sal_uInt16 GetElementCount();
    std::optional<std::pair<TextFrameIndex, TextFrameIndex>> GetElementAt(sal_uInt16 nIndex);
};

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
