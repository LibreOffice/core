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

#include <vcl/seleng.hxx>
#include <vcl/cursor.hxx>
#include <vcl/idle.hxx>
#include <vcl/textdata.hxx>

#include <cstddef>
#include <limits>
#include <vector>

class TextLine;
class TextNode;
class TextView;

#define PORTIONKIND_TEXT        0
#define PORTIONKIND_TAB         1

#define DELMODE_SIMPLE          0
#define DELMODE_RESTOFWORD      1
#define DELMODE_RESTOFCONTENT   2

#define DEL_LEFT    1
#define DEL_RIGHT   2
#define TRAVEL_X_DONTKNOW   0xFFFF
#define MAXCHARSINPARA      0x3FFF-CHARPOSGROW

#define LINE_SEP    0x0A

class TETextPortion
{
private:
    tools::Long     nWidth;
    sal_Int32       nLen;
    sal_uInt8       nKind;
    bool            bRightToLeft;

public:
                TETextPortion( sal_Int32 nL )
                    : nWidth {-1}
                    , nLen {nL}
                    , nKind {PORTIONKIND_TEXT}
                    , bRightToLeft {false}
                {}

    sal_Int32&  GetLen()                        { return nLen; }
    sal_Int32  GetLen() const                   { return nLen; }
    tools::Long&           GetWidth()                  { return nWidth; }
    sal_uInt8&      GetKind()                   { return nKind; }
    void            SetRightToLeft(bool b)      { bRightToLeft = b; }
    bool            IsRightToLeft() const       { return bRightToLeft; }
};

class TETextPortionList
{
private:
    std::vector<TETextPortion> maPortions;

public:
    static constexpr auto npos = std::numeric_limits<std::size_t>::max();

    TETextPortionList();
    ~TETextPortionList();

    TETextPortion& operator[]( std::size_t nPos );
    std::vector<TETextPortion>::iterator begin();
    std::vector<TETextPortion>::const_iterator begin() const;
    std::vector<TETextPortion>::iterator end();
    std::vector<TETextPortion>::const_iterator end() const;
    bool empty() const;
    std::size_t size() const;
    std::vector<TETextPortion>::iterator erase( const std::vector<TETextPortion>::iterator& aIter );
    std::vector<TETextPortion>::iterator insert( const std::vector<TETextPortion>::iterator& aIter,
                                                  const TETextPortion& rTP );
    void push_back( const TETextPortion & aTP );

    void    Reset();
    std::size_t FindPortion( sal_Int32 nCharPos, sal_Int32& rPortionStart, bool bPreferStartingPortion = false );
    void    DeleteFromPortion( std::size_t nDelFrom );
};

struct TEWritingDirectionInfo
{
    bool         bLeftToRight;
    sal_Int32    nStartPos;
    sal_Int32    nEndPos;
    TEWritingDirectionInfo( bool LeftToRight, sal_Int32 Start, sal_Int32 End )
        : bLeftToRight {LeftToRight}
        , nStartPos {Start}
        , nEndPos {End}
    {}
};

class TEParaPortion
{
private:
    TextNode*               mpNode;

    std::vector<TextLine>   maLines;
    TETextPortionList       maTextPortions;
    std::vector<TEWritingDirectionInfo> maWritingDirectionInfos;

    sal_Int32               mnInvalidPosStart;
    sal_Int32               mnInvalidDiff;

    bool                    mbInvalid;
    bool                    mbSimple;   // only type linearly

public:
                        TEParaPortion( TextNode* pNode );
                        ~TEParaPortion();

    TEParaPortion( const TEParaPortion& ) = delete;
    void operator=( const TEParaPortion& ) = delete;

    bool                IsInvalid() const           { return mbInvalid; }
    bool                IsSimpleInvalid() const     { return mbSimple; }
    void                SetNotSimpleInvalid()       { mbSimple = false; }
    void                SetValid()                  { mbInvalid = false; mbSimple = true;}

    void                MarkInvalid( sal_Int32 nStart, sal_Int32 nDiff );
    void                MarkSelectionInvalid( sal_Int32 nStart );

    sal_Int32           GetInvalidPosStart() const  { return mnInvalidPosStart; }
    sal_Int32           GetInvalidDiff() const      { return mnInvalidDiff; }

    TextNode*           GetNode() const             { return mpNode; }
    std::vector<TextLine>& GetLines()               { return maLines; }
    TETextPortionList&  GetTextPortions()           { return maTextPortions; }
    std::vector<TEWritingDirectionInfo>& GetWritingDirectionInfos() { return maWritingDirectionInfos; }

    std::vector<TextLine>::size_type GetLineNumber( sal_Int32 nIndex, bool bInclEnd );
    void                CorrectValuesBehindLastFormattedLine( sal_uInt16 nLastFormattedLine );
};

class TEParaPortions
{
private:
    std::vector<std::unique_ptr<TEParaPortion>> mvData;

public:
                    TEParaPortions() : mvData() {}
                    ~TEParaPortions();

    sal_uInt32      Count() const { return static_cast<sal_uInt32>(mvData.size()); }
    TEParaPortion*  GetObject( sal_uInt32 nIndex ) { return mvData[nIndex].get(); }
    void            Insert( TEParaPortion* pObject, sal_uInt32 nPos ) { mvData.emplace( mvData.begin()+nPos, pObject ); }
    void            Remove( sal_uInt32 nPos ) { mvData.erase( mvData.begin()+nPos ); }
};

class TextSelFunctionSet: public FunctionSet
{
private:
    TextView*       mpView;

public:
                    TextSelFunctionSet( TextView* pView );

    virtual void    BeginDrag() override;

    virtual void    CreateAnchor() override;

    virtual void    SetCursorAtPoint( const Point& rPointPixel, bool bDontSelectAtCursor = false ) override;

    virtual bool    IsSelectionAtPoint( const Point& rPointPixel ) override;
    virtual void    DeselectAll() override;

    virtual void    DeselectAtPoint( const Point& ) override;
    virtual void    DestroyAnchor() override;
};

class IdleFormatter : public Idle
{
private:
    TextView*   mpView;
    sal_uInt16      mnRestarts;

public:
                IdleFormatter();
                virtual ~IdleFormatter() override;

    void        DoIdleFormat( TextView* pV, sal_uInt16 nMaxRestarts );
    void        ForceTimeout();
    TextView*   GetView()       { return mpView; }
};

struct TextDDInfo
{
    vcl::Cursor     maCursor;
    TextPaM         maDropPos;

    bool            mbStarterOfDD;
    bool            mbVisCursor;

    TextDDInfo()
        : maCursor()
        , maDropPos()
        , mbStarterOfDD {false}
        , mbVisCursor {false}
    {
        maCursor.SetStyle( CURSOR_SHADOW );
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
