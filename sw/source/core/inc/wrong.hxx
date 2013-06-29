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

#ifndef _WRONG_HXX
#define _WRONG_HXX

#include <com/sun/star/container/XStringKeyMap.hpp>

#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <vector>

#include <tools/string.hxx>
#include <tools/color.hxx>
#include <viewopt.hxx>

class SwWrongList;

enum WrongAreaLineType
{
    WRONGAREA_DASHED,
    WRONGAREA_WAVE,
    WRONGAREA_WAVE_NORMAL,
    WRONGAREA_WAVE_SMALL,
    WRONGAREA_WAVE_FLAT,
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
    com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > mxPropertyBag;
    xub_StrLen mnPos;
    xub_StrLen mnLen;
    SwWrongList* mpSubList;

    Color mColor;
    WrongAreaLineType mLineType;

    SwWrongArea( const OUString& rType,
                 WrongListType listType,
                 com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xPropertyBag,
                 xub_StrLen nPos,
                 xub_StrLen nLen);

    SwWrongArea( const OUString& rType,
                 com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xPropertyBag,
                 xub_StrLen nPos,
                 xub_StrLen nLen,
                 SwWrongList* pSubList);
private:

    SwWrongArea() : mnPos(0), mnLen(0), mpSubList(NULL), mColor(0,0,0), mLineType(WRONGAREA_WAVE) {}

    Color getSmartColor ( com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xPropertyBag)
    {
        try
        {
            if (xPropertyBag.is())
            {
                const ::rtl::OUString colorKey("LineColor");
                com::sun::star::uno::Any aLineColor = xPropertyBag->getValue(colorKey).get< com::sun::star::uno::Any>();
                com::sun::star::util::Color lineColor = 0;

                if (aLineColor >>= lineColor)
                {
                    return Color( lineColor );
                }
            }
        }
        catch(::com::sun::star::container::NoSuchElementException&)
        {
        }
        catch(::com::sun::star::uno::RuntimeException&)
        {
        }

        return SwViewOption::GetSmarttagColor( );
    }

    WrongAreaLineType getSmartLineType( com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xPropertyBag )
    {
        try
        {
            if (xPropertyBag.is())
            {
                const ::rtl::OUString typeKey("LineType");
                com::sun::star::uno::Any aLineType = xPropertyBag->getValue(typeKey).get< com::sun::star::uno::Any>();
                ::sal_Int16 lineType = 0;

                if (!(aLineType >>= lineType))
                {
                    return WRONGAREA_DASHED;
                }
                if (::com::sun::star::awt::FontUnderline::WAVE == lineType)
                {
                    return WRONGAREA_WAVE_NORMAL;
                }
                if (::com::sun::star::awt::FontUnderline::SMALLWAVE == lineType)
                {
                    return WRONGAREA_WAVE_SMALL;
                }
            }
        }
        catch(::com::sun::star::container::NoSuchElementException&)
        {
        }
        catch(::com::sun::star::uno::RuntimeException&)
        {
        }

        return WRONGAREA_DASHED;
    }

    Color getWrongAreaColor(WrongListType listType,
                            com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xPropertyBag )
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

    WrongAreaLineType getWrongAreaLineType(WrongListType listType,
                                           com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xPropertyBag )
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

    xub_StrLen nBeginInvalid;   // Start of the invalid range
    xub_StrLen nEndInvalid;     // End of the invalid range

    void ShiftLeft( xub_StrLen &rPos, xub_StrLen nStart, xub_StrLen nEnd )
    { if( rPos > nStart ) rPos = rPos > nEnd ? rPos - nEnd + nStart : nStart; }
    void ShiftRight( xub_StrLen &rPos, xub_StrLen nStart, xub_StrLen nEnd )
    { if( rPos >= nStart ) rPos += nStart - nEnd; }
    void _Invalidate( xub_StrLen nBegin, xub_StrLen nEnd );

    void Insert(sal_uInt16 nWhere, std::vector<SwWrongArea>::iterator startPos, std::vector<SwWrongArea>::iterator endPos);
    void Remove( sal_uInt16 nIdx, sal_uInt16 nLen );

    // forbidden and not implemented
    SwWrongList& operator= (const SwWrongList &);
    SwWrongList( const SwWrongList& rCpy );

public:
    SwWrongList( WrongListType eType );

    virtual ~SwWrongList();
    virtual SwWrongList* Clone();
    virtual void CopyFrom( const SwWrongList& rCopy );

    inline WrongListType GetWrongListType() const { return meType; }
    inline xub_StrLen GetBeginInv() const { return nBeginInvalid; }
    inline xub_StrLen GetEndInv() const { return nEndInvalid; }
    inline sal_Bool InsideInvalid( xub_StrLen nChk ) const
        { return nChk >= nBeginInvalid && nChk <= nEndInvalid; }
    void SetInvalid( xub_StrLen nBegin, xub_StrLen nEnd );
    inline void Validate(){ nBeginInvalid = STRING_LEN; }
    void Invalidate( xub_StrLen nBegin, xub_StrLen nEnd );
    sal_Bool InvalidateWrong();
    sal_Bool Fresh( xub_StrLen &rStart, xub_StrLen &rEnd, xub_StrLen nPos,
            xub_StrLen nLen, sal_uInt16 nIndex, xub_StrLen nCursorPos );
    sal_uInt16 GetWrongPos( xub_StrLen nValue ) const;

    sal_Bool Check( xub_StrLen &rChk, xub_StrLen &rLn ) const;
    sal_Bool InWrongWord( xub_StrLen &rChk, xub_StrLen &rLn ) const;
    xub_StrLen NextWrong( xub_StrLen nChk ) const;

    void Move( xub_StrLen nPos, long nDiff );
    void ClearList();

    // Divide the list into two part, the wrong words until nSplitPos will be
    // removed and transferred to a new SwWrongList.
    SwWrongList* SplitList( xub_StrLen nSplitPos );
    // Join the next SwWrongList, nInsertPos is my own text length, where
    // the other wrong list has to be inserted.
    void JoinList( SwWrongList* pNext, xub_StrLen nInsertPos );

    inline xub_StrLen Len( sal_uInt16 nIdx ) const
    {
        return nIdx < maList.size() ? maList[nIdx].mnLen : 0;
    }

    inline xub_StrLen Pos( sal_uInt16 nIdx ) const
    {
        return nIdx < maList.size() ? maList[nIdx].mnPos : 0;
    }

    inline sal_uInt16 Count() const { return (sal_uInt16)maList.size(); }

    inline void Insert( const OUString& rType,
                        com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xPropertyBag,
                        xub_StrLen nNewPos, xub_StrLen nNewLen, sal_uInt16 nWhere )
    {
        std::vector<SwWrongArea>::iterator i = maList.begin();
        if ( nWhere >= maList.size() )
            i = maList.end(); // robust
        else
            i += nWhere;

        maList.insert(i, SwWrongArea( rType, meType, xPropertyBag, nNewPos, nNewLen) );
    }

    void Insert( const OUString& rType,
                 com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xPropertyBag,
                 xub_StrLen nNewPos, xub_StrLen nNewLen );

    inline SwWrongList* SubList( sal_uInt16 nIdx ) const
    {
        return nIdx < maList.size() ? maList[nIdx].mpSubList : 0;
    }

    void InsertSubList( xub_StrLen nNewPos, xub_StrLen nNewLen, sal_uInt16 nWhere, SwWrongList* pSubList );

    inline const SwWrongArea* GetElement( sal_uInt16 nIdx ) const
    {
        return nIdx < maList.size() ? &maList[nIdx] : 0;
    }
    void RemoveEntry( xub_StrLen nBegin, xub_StrLen nEnd );
    bool LookForEntry( xub_StrLen nBegin, xub_StrLen nEnd );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
