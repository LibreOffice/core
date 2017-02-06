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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_WRONG_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_WRONG_HXX

#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XStringKeyMap.hpp>

#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <vector>

#include <tools/color.hxx>
#include <swtypes.hxx>
#include <viewopt.hxx>

class SwWrongList;

enum WrongAreaLineType
{
    WRONGAREA_DASHED,
    WRONGAREA_WAVE,
    WRONGAREA_NONE
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

    SwWrongArea( const OUString& rType,
                 WrongListType listType,
                 css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag,
                 sal_Int32 nPos,
                 sal_Int32 nLen);

    SwWrongArea( const OUString& rType,
                 css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag,
                 sal_Int32 nPos,
                 sal_Int32 nLen,
                 SwWrongList* pSubList);
private:

    static Color getSmartColor ( css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag)
    {
        try
        {
            if (xPropertyBag.is())
            {
                const OUString colorKey("LineColor");
                css::uno::Any aLineColor = xPropertyBag->getValue(colorKey);
                css::util::Color lineColor = 0;

                if (aLineColor >>= lineColor)
                {
                    return Color( lineColor );
                }
            }
        }
        catch(const css::container::NoSuchElementException&)
        {
        }
        catch(const css::uno::RuntimeException&)
        {
        }

        return SwViewOption::GetSmarttagColor( );
    }

    static WrongAreaLineType getSmartLineType( css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag )
    {
        try
        {
            if (xPropertyBag.is())
            {
                const OUString typeKey("LineType");
                css::uno::Any aLineType = xPropertyBag->getValue(typeKey);
                ::sal_Int16 lineType = 0;

                if (!(aLineType >>= lineType))
                {
                    return WRONGAREA_DASHED;
                }
                if (css::awt::FontUnderline::WAVE == lineType)
                {
                    return WRONGAREA_WAVE;
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
            return SwViewOption::GetSpellColor();
        }
        else if (WRONGLIST_GRAMMAR == listType)
        {
            return Color( COL_LIGHTBLUE );
        }
        else if (WRONGLIST_SMARTTAG == listType)
        {
            return  getSmartColor(xPropertyBag);
        }

        return SwViewOption::GetSpellColor();
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
            return WRONGAREA_WAVE;
        }
        else if (WRONGLIST_SMARTTAG == listType)
        {
            return getSmartLineType(xPropertyBag);
        }

        return WRONGAREA_WAVE;
    }

};

class SwWrongList
{
    std::vector<SwWrongArea> maList;
    WrongListType            meType;

    sal_Int32 nBeginInvalid;   // Start of the invalid range
    sal_Int32 nEndInvalid;     // End of the invalid range

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

    inline WrongListType GetWrongListType() const { return meType; }
    inline sal_Int32 GetBeginInv() const { return nBeginInvalid; }
    inline sal_Int32 GetEndInv() const { return nEndInvalid; }
    void SetInvalid( sal_Int32 nBegin, sal_Int32 nEnd );
    inline void Validate(){ nBeginInvalid = nEndInvalid = COMPLETE_STRING; }
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
    SwWrongList* SplitList( sal_Int32 nSplitPos );
    // Join the next SwWrongList, nInsertPos is my own text length, where
    // the other wrong list has to be inserted.
    void JoinList( SwWrongList* pNext, sal_Int32 nInsertPos );

    inline sal_Int32 Len( sal_uInt16 nIdx ) const
    {
        return nIdx < maList.size() ? maList[nIdx].mnLen : 0;
    }

    inline sal_Int32 Pos( sal_uInt16 nIdx ) const
    {
        return nIdx < maList.size() ? maList[nIdx].mnPos : 0;
    }

    inline sal_uInt16 Count() const { return (sal_uInt16)maList.size(); }

    inline void Insert( const OUString& rType,
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

    inline SwWrongList* SubList( sal_uInt16 nIdx ) const
    {
        return nIdx < maList.size() ? maList[nIdx].mpSubList : nullptr;
    }

    void InsertSubList( sal_Int32 nNewPos, sal_Int32 nNewLen, sal_uInt16 nWhere, SwWrongList* pSubList );

    inline const SwWrongArea* GetElement( sal_uInt16 nIdx ) const
    {
        return nIdx < maList.size() ? &maList[nIdx] : nullptr;
    }
    void RemoveEntry( sal_Int32 nBegin, sal_Int32 nEnd );
    bool LookForEntry( sal_Int32 nBegin, sal_Int32 nEnd );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
