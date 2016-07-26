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

#ifndef INCLUDED_VCL_SOURCE_EDIT_TEXTDAT2_HXX
#define INCLUDED_VCL_SOURCE_EDIT_TEXTDAT2_HXX

#include <vcl/seleng.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cursor.hxx>
#include <vcl/idle.hxx>

#include <vector>

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
    sal_Int32       nLen;
    long            nWidth;
    sal_uInt8       nKind;
    bool            bRightToLeft;

public:
                TETextPortion( sal_Int32 nL )
                    : nLen {nL}
                    , nWidth {-1}
                    , nKind {PORTIONKIND_TEXT}
                    , bRightToLeft {false}
                {}

    sal_Int32&  GetLen()                        { return nLen; }
    long&           GetWidth()                  { return nWidth; }
    sal_uInt8&      GetKind()                   { return nKind; }
    void            SetRightToLeft(bool b)      { bRightToLeft = b; }
    bool            IsRightToLeft() const       { return bRightToLeft; }
};

class TETextPortionList
{
private:
    std::vector<TETextPortion*> maPortions;

public:
    TETextPortionList();
    ~TETextPortionList();

    TETextPortion* operator[]( size_t nPos );
    std::vector<TETextPortion*>::const_iterator begin() const;
    std::vector<TETextPortion*>::const_iterator end() const;
    bool empty() const;
    size_t size() const;
    std::vector<TETextPortion*>::iterator erase( std::vector<TETextPortion*>::iterator aIter );
    std::vector<TETextPortion*>::iterator insert( std::vector<TETextPortion*>::iterator aIter,
                                                  TETextPortion* pTP );
    void push_back( TETextPortion* pTP );

    void    Reset();
    sal_uInt16  FindPortion( sal_Int32 nCharPos, sal_Int32& rPortionStart, bool bPreferStartingPortion = false );
    void    DeleteFromPortion( sal_uInt16 nDelFrom );
};

struct TEWritingDirectionInfo
{
    sal_uInt8    nType;
    sal_Int32    nStartPos;
    sal_Int32    nEndPos;
    TEWritingDirectionInfo( sal_uInt8 Type, sal_Int32 Start, sal_Int32 End )
        : nType {Type}
        , nStartPos {Start}
        , nEndPos {End}
    {}
};

class TextLine
{
private:
    sal_Int32           mnStart;
    sal_Int32           mnEnd;
    sal_uInt16          mnStartPortion;
    sal_uInt16          mnEndPortion;

    short               mnStartX;

    bool                mbInvalid;  // fuer geschickte Formatierung/Ausgabe

public:
                    TextLine()
                        : mnStart {0}
                        , mnEnd {0}
                        , mnStartPortion {0}
                        , mnEndPortion {0}
                        , mnStartX {0}
                        , mbInvalid {true}
                    {}

    bool            IsIn( sal_Int32 nIndex, bool bInclEnd ) const
                        { return nIndex >= mnStart && ( bInclEnd ? nIndex <= mnEnd : nIndex < mnEnd ); }

    void            SetStart( sal_Int32 n )         { mnStart = n; }
    sal_Int32       GetStart() const                { return mnStart; }
    sal_Int32&      GetStart()                      { return mnStart; }

    void            SetEnd( sal_Int32 n )           { mnEnd = n; }
    sal_Int32       GetEnd() const                  { return mnEnd; }
    sal_Int32&      GetEnd()                        { return mnEnd; }

    void            SetStartPortion( sal_uInt16 n )     { mnStartPortion = n; }
    sal_uInt16      GetStartPortion() const         { return mnStartPortion; }
    sal_uInt16&     GetStartPortion()               { return mnStartPortion; }

    void            SetEndPortion( sal_uInt16 n )       { mnEndPortion = n; }
    sal_uInt16      GetEndPortion() const           { return mnEndPortion; }
    sal_uInt16&     GetEndPortion()                 { return mnEndPortion; }

    sal_Int32       GetLen() const                  { return mnEnd - mnStart; }

    bool            IsInvalid() const               { return mbInvalid; }
    bool            IsValid() const                 { return !mbInvalid; }
    void            SetInvalid()                    { mbInvalid = true; }
    void            SetValid()                      { mbInvalid = false; }

    short           GetStartX() const               { return mnStartX; }
    void            SetStartX( short n )            { mnStartX = n; }

    inline bool operator == ( const TextLine& rLine ) const;
};

inline bool TextLine::operator == ( const TextLine& rLine ) const
{
    return mnStart == rLine.mnStart &&
           mnEnd == rLine.mnEnd &&
           mnStartPortion == rLine.mnStartPortion &&
           mnEndPortion == rLine.mnEndPortion;
}

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
    void                MarkSelectionInvalid( sal_Int32 nStart, sal_Int32 nEnd );

    sal_Int32           GetInvalidPosStart() const  { return mnInvalidPosStart; }
    sal_Int32           GetInvalidDiff() const      { return mnInvalidDiff; }

    TextNode*           GetNode() const             { return mpNode; }
    std::vector<TextLine>& GetLines()               { return maLines; }
    TETextPortionList&  GetTextPortions()           { return maTextPortions; }
    std::vector<TEWritingDirectionInfo>& GetWritingDirectionInfos() { return maWritingDirectionInfos; }

    sal_uInt16          GetLineNumber( sal_Int32 nIndex, bool bInclEnd );
    void                CorrectValuesBehindLastFormattedLine( sal_uInt16 nLastFormattedLine );
};

class TEParaPortions
{
private:
    std::vector<TEParaPortion*> mvData;

public:
                    TEParaPortions() : mvData() {}
                    ~TEParaPortions();

    sal_uInt32      Count() const { return static_cast<sal_uInt32>(mvData.size()); }
    TEParaPortion*  GetObject( sal_uInt32 nIndex ) { return mvData[nIndex]; }
    void            Insert( TEParaPortion* pObject, sal_uInt32 nPos ) { mvData.insert( mvData.begin()+nPos, pObject ); }
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

    virtual bool    SetCursorAtPoint( const Point& rPointPixel, bool bDontSelectAtCursor = false ) override;

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
                virtual ~IdleFormatter();

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

#endif // INCLUDED_VCL_SOURCE_EDIT_TEXTDAT2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
