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
#if 1

#include <vcl/seleng.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cursor.hxx>

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
    sal_uInt16      nLen;
    long        nWidth;
    sal_uInt8       nKind;
    sal_uInt8        nRightToLeft;

                TETextPortion()             { nLen = 0; nKind = PORTIONKIND_TEXT; nWidth = -1; nRightToLeft = 0;}

public:
                TETextPortion( sal_uInt16 nL )  {
                                                nLen = nL;
                                                nKind = PORTIONKIND_TEXT;
                                                nWidth= -1;
                                                nRightToLeft = 0;
                                            }

    sal_uInt16      GetLen() const              { return nLen; }
    sal_uInt16&     GetLen()                    { return nLen; }

    long        GetWidth()const             { return nWidth; }
    long&       GetWidth()                  { return nWidth; }

    sal_uInt8       GetKind() const             { return nKind; }
    sal_uInt8&      GetKind()                   { return nKind; }

    sal_uInt8       GetRightToLeft() const      { return nRightToLeft; }
    sal_uInt8&      GetRightToLeft()            { return nRightToLeft; }
    sal_Bool        IsRightToLeft() const       { return (nRightToLeft&1); }

    sal_Bool        HasValidSize() const        { return nWidth != (-1); }
};



typedef std::vector<TETextPortion*> TextPortionArray;

class TETextPortionList : public TextPortionArray
{
public:
    TETextPortionList();
    ~TETextPortionList();

    void    Reset();
    sal_uInt16  FindPortion( sal_uInt16 nCharPos, sal_uInt16& rPortionStart, sal_Bool bPreferStartingPortion = sal_False );
    sal_uInt16  GetPortionStartIndex( sal_uInt16 nPortion );
    void    DeleteFromPortion( sal_uInt16 nDelFrom );
};

struct TEWritingDirectionInfo
{
    sal_uInt8    nType;
    sal_uInt16  nStartPos;
    sal_uInt16  nEndPos;
    TEWritingDirectionInfo( sal_uInt8 _Type, sal_uInt16 _Start, sal_uInt16 _End )
    {
        nType = _Type;
        nStartPos = _Start;
        nEndPos = _End;
    }
};

class TextLine
{
private:
    sal_uInt16          mnStart;
    sal_uInt16          mnEnd;
    sal_uInt16          mnStartPortion;
    sal_uInt16          mnEndPortion;

    short           mnStartX;

    sal_Bool            mbInvalid;  // fuer geschickte Formatierung/Ausgabe

public:
                    TextLine()  {
                                    mnStart = mnEnd = 0;
                                    mnStartPortion = mnEndPortion = 0;
                                    mnStartX = 0;
                                    mbInvalid = sal_True;
                                }

    sal_Bool            IsIn( sal_uInt16 nIndex ) const
                        { return ( (nIndex >= mnStart ) && ( nIndex < mnEnd ) ); }

    sal_Bool            IsIn( sal_uInt16 nIndex, sal_Bool bInclEnd ) const
                        { return ( ( nIndex >= mnStart ) && ( bInclEnd ? ( nIndex <= mnEnd ) : ( nIndex < mnEnd ) ) ); }

    void            SetStart( sal_uInt16 n )            { mnStart = n; }
    sal_uInt16          GetStart() const                { return mnStart; }
    sal_uInt16&         GetStart()                      { return mnStart; }

    void            SetEnd( sal_uInt16 n )              { mnEnd = n; }
    sal_uInt16          GetEnd() const                  { return mnEnd; }
    sal_uInt16&         GetEnd()                        { return mnEnd; }

    void            SetStartPortion( sal_uInt16 n )     { mnStartPortion = n; }
    sal_uInt16          GetStartPortion() const         { return mnStartPortion; }
    sal_uInt16&         GetStartPortion()               { return mnStartPortion; }

    void            SetEndPortion( sal_uInt16 n )       { mnEndPortion = n; }
    sal_uInt16          GetEndPortion() const           { return mnEndPortion; }
    sal_uInt16&         GetEndPortion()                 { return mnEndPortion; }

    sal_uInt16          GetLen() const                  { return mnEnd - mnStart; }

    sal_Bool            IsInvalid() const               { return mbInvalid; }
    sal_Bool            IsValid() const                 { return !mbInvalid; }
    void            SetInvalid()                    { mbInvalid = sal_True; }
    void            SetValid()                      { mbInvalid = sal_False; }

    sal_Bool            IsEmpty() const                 { return (mnEnd > mnStart) ? sal_False : sal_True; }

    short           GetStartX() const               { return mnStartX; }
    void            SetStartX( short n )            { mnStartX = n; }

    inline sal_Bool operator == ( const TextLine& rLine ) const;
    inline sal_Bool operator != ( const TextLine& rLine ) const;
};

class TextLines : public std::vector<TextLine*> {
public:
    ~TextLines()
    {
        for( iterator it = begin(); it != end(); ++it )
            delete *it;
    }
};

inline sal_Bool TextLine::operator == ( const TextLine& rLine ) const
{
    return (    ( mnStart == rLine.mnStart ) &&
                ( mnEnd == rLine.mnEnd ) &&
                ( mnStartPortion == rLine.mnStartPortion ) &&
                ( mnEndPortion == rLine.mnEndPortion ) );
}

inline sal_Bool TextLine::operator != ( const TextLine& rLine ) const
{
    return !( *this == rLine );
}



class TEParaPortion
{
private:
    TextNode*               mpNode;

    TextLines               maLines;
    TETextPortionList       maTextPortions;
    std::vector<TEWritingDirectionInfo> maWritingDirectionInfos;


    sal_uInt16              mnInvalidPosStart;
    short               mnInvalidDiff;

    sal_Bool                mbInvalid;
    sal_Bool                mbSimple;   // nur lineares Tippen


                        TEParaPortion( const TEParaPortion& ) {;}

public:
                        TEParaPortion( TextNode* pNode );
                        ~TEParaPortion();


    sal_Bool                IsInvalid() const           { return mbInvalid; }
    sal_Bool                IsSimpleInvalid() const     { return mbSimple; }
    void                SetNotSimpleInvalid()       { mbSimple = sal_False; }
    void                SetValid()                  { mbInvalid = sal_False; mbSimple = sal_True;}

    void                MarkInvalid( sal_uInt16 nStart, short nDiff);
    void                MarkSelectionInvalid( sal_uInt16 nStart, sal_uInt16 nEnd );

    sal_uInt16              GetInvalidPosStart() const  { return mnInvalidPosStart; }
    short               GetInvalidDiff() const      { return mnInvalidDiff; }

    TextNode*           GetNode() const             { return mpNode; }
    TextLines&          GetLines()                  { return maLines; }
    TETextPortionList&  GetTextPortions()           { return maTextPortions; }
    std::vector<TEWritingDirectionInfo>& GetWritingDirectionInfos() { return maWritingDirectionInfos; }


    sal_uInt16              GetLineNumber( sal_uInt16 nIndex, sal_Bool bInclEnd );
    void                CorrectValuesBehindLastFormattedLine( sal_uInt16 nLastFormattedLine );
};


class TEParaPortions : public ToolsList<TEParaPortion*>
{
public:
                    TEParaPortions();
                    ~TEParaPortions();
    void            Reset();
};


class TextSelFunctionSet: public FunctionSet
{
private:
    TextView*       mpView;

public:
                    TextSelFunctionSet( TextView* pView );

    virtual void    BeginDrag();

    virtual void    CreateAnchor();

    virtual sal_Bool    SetCursorAtPoint( const Point& rPointPixel, sal_Bool bDontSelectAtCursor = sal_False );

    virtual sal_Bool    IsSelectionAtPoint( const Point& rPointPixel );
    virtual void    DeselectAll();

    virtual void    DeselectAtPoint( const Point& );
    virtual void    DestroyAnchor();
};


class IdleFormatter : public Timer
{
private:
    TextView*   mpView;
    sal_uInt16      mnRestarts;

public:
                IdleFormatter();
                ~IdleFormatter();

    void        DoIdleFormat( TextView* pV, sal_uInt16 nMaxRestarts );
    void        ForceTimeout();
    TextView*   GetView()       { return mpView; }
};

struct TextDDInfo
{
    Cursor          maCursor;
    TextPaM         maDropPos;

    sal_Bool            mbStarterOfDD;
    sal_Bool            mbVisCursor;

    TextDDInfo()
    {
        maCursor.SetStyle( CURSOR_SHADOW );
        mbStarterOfDD = sal_False;
        mbVisCursor = sal_False;
    }
};

#endif // _TEXTDAT2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
