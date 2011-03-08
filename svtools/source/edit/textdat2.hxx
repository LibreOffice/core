/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifndef _TEXTDAT2_HXX
#define _TEXTDAT2_HXX

#include <svl/svarray.hxx>
#include <vcl/seleng.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cursor.hxx>

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
    USHORT      nLen;
    long        nWidth;
    BYTE        nKind;
    BYTE        nRightToLeft;

                TETextPortion()             { nLen = 0; nKind = PORTIONKIND_TEXT; nWidth = -1; nRightToLeft = 0;}

public:
                TETextPortion( USHORT nL )  {
                                                nLen = nL;
                                                nKind = PORTIONKIND_TEXT;
                                                nWidth= -1;
                                                nRightToLeft = 0;
                                            }

    USHORT      GetLen() const              { return nLen; }
    USHORT&     GetLen()                    { return nLen; }

    long        GetWidth()const             { return nWidth; }
    long&       GetWidth()                  { return nWidth; }

    BYTE        GetKind() const             { return nKind; }
    BYTE&       GetKind()                   { return nKind; }

    BYTE        GetRightToLeft() const      { return nRightToLeft; }
    BYTE&       GetRightToLeft()            { return nRightToLeft; }
    BOOL        IsRightToLeft() const       { return (nRightToLeft&1); }

    BOOL        HasValidSize() const        { return nWidth != (-1); }
};



typedef TETextPortion* TextPortionPtr;
SV_DECL_PTRARR( TextPortionArray, TextPortionPtr, 0, 8 )

class TETextPortionList : public TextPortionArray
{
public:
            TETextPortionList();
            ~TETextPortionList();

    void    Reset();
    USHORT  FindPortion( USHORT nCharPos, USHORT& rPortionStart, BOOL bPreferStartingPortion = FALSE );
    USHORT  GetPortionStartIndex( USHORT nPortion );
    void    DeleteFromPortion( USHORT nDelFrom );
};

struct TEWritingDirectionInfo
{
    BYTE    nType;
    USHORT  nStartPos;
    USHORT  nEndPos;
    TEWritingDirectionInfo( BYTE _Type, USHORT _Start, USHORT _End )
    {
        nType = _Type;
        nStartPos = _Start;
        nEndPos = _End;
    }
};

SV_DECL_VARARR( TEWritingDirectionInfos, TEWritingDirectionInfo, 0, 4 )

class TextLine
{
private:
    USHORT          mnStart;
    USHORT          mnEnd;
    USHORT          mnStartPortion;
    USHORT          mnEndPortion;

    short           mnStartX;

    BOOL            mbInvalid;  // fuer geschickte Formatierung/Ausgabe

public:
                    TextLine()  {
                                    mnStart = mnEnd = 0;
                                    mnStartPortion = mnEndPortion = 0;
                                    mnStartX = 0;
                                    mbInvalid = TRUE;
                                }

    BOOL            IsIn( USHORT nIndex ) const
                        { return ( (nIndex >= mnStart ) && ( nIndex < mnEnd ) ); }

    BOOL            IsIn( USHORT nIndex, BOOL bInclEnd ) const
                        { return ( ( nIndex >= mnStart ) && ( bInclEnd ? ( nIndex <= mnEnd ) : ( nIndex < mnEnd ) ) ); }

    void            SetStart( USHORT n )            { mnStart = n; }
    USHORT          GetStart() const                { return mnStart; }
    USHORT&         GetStart()                      { return mnStart; }

    void            SetEnd( USHORT n )              { mnEnd = n; }
    USHORT          GetEnd() const                  { return mnEnd; }
    USHORT&         GetEnd()                        { return mnEnd; }

    void            SetStartPortion( USHORT n )     { mnStartPortion = n; }
    USHORT          GetStartPortion() const         { return mnStartPortion; }
    USHORT&         GetStartPortion()               { return mnStartPortion; }

    void            SetEndPortion( USHORT n )       { mnEndPortion = n; }
    USHORT          GetEndPortion() const           { return mnEndPortion; }
    USHORT&         GetEndPortion()                 { return mnEndPortion; }

    USHORT          GetLen() const                  { return mnEnd - mnStart; }

    BOOL            IsInvalid() const               { return mbInvalid; }
    BOOL            IsValid() const                 { return !mbInvalid; }
    void            SetInvalid()                    { mbInvalid = TRUE; }
    void            SetValid()                      { mbInvalid = FALSE; }

    BOOL            IsEmpty() const                 { return (mnEnd > mnStart) ? FALSE : TRUE; }

    short           GetStartX() const               { return mnStartX; }
    void            SetStartX( short n )            { mnStartX = n; }

    inline BOOL operator == ( const TextLine& rLine ) const;
    inline BOOL operator != ( const TextLine& rLine ) const;
};

typedef TextLine* TextLinePtr;
 SV_DECL_PTRARR_DEL( TextLines, TextLinePtr, 1, 4 )

inline BOOL TextLine::operator == ( const TextLine& rLine ) const
{
    return (    ( mnStart == rLine.mnStart ) &&
                ( mnEnd == rLine.mnEnd ) &&
                ( mnStartPortion == rLine.mnStartPortion ) &&
                ( mnEndPortion == rLine.mnEndPortion ) );
}

inline BOOL TextLine::operator != ( const TextLine& rLine ) const
{
    return !( *this == rLine );
}



class TEParaPortion
{
private:
    TextNode*               mpNode;

    TextLines               maLines;
    TETextPortionList       maTextPortions;
    TEWritingDirectionInfos maWritingDirectionInfos;


    USHORT              mnInvalidPosStart;
    short               mnInvalidDiff;

    BOOL                mbInvalid;
    BOOL                mbSimple;   // nur lineares Tippen


                        TEParaPortion( const TEParaPortion& ) {;}

public:
                        TEParaPortion( TextNode* pNode );
                        ~TEParaPortion();


    BOOL                IsInvalid() const           { return mbInvalid; }
    BOOL                IsSimpleInvalid() const     { return mbSimple; }
    void                SetNotSimpleInvalid()       { mbSimple = FALSE; }
    void                SetValid()                  { mbInvalid = FALSE; mbSimple = TRUE;}

    void                MarkInvalid( USHORT nStart, short nDiff);
    void                MarkSelectionInvalid( USHORT nStart, USHORT nEnd );

    USHORT              GetInvalidPosStart() const  { return mnInvalidPosStart; }
    short               GetInvalidDiff() const      { return mnInvalidDiff; }

    TextNode*           GetNode() const             { return mpNode; }
    TextLines&          GetLines()                  { return maLines; }
    TETextPortionList&  GetTextPortions()           { return maTextPortions; }
    TEWritingDirectionInfos& GetWritingDirectionInfos() { return maWritingDirectionInfos; }


    USHORT              GetLineNumber( USHORT nIndex, BOOL bInclEnd );
    void                CorrectValuesBehindLastFormattedLine( USHORT nLastFormattedLine );
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

    virtual BOOL    SetCursorAtPoint( const Point& rPointPixel, BOOL bDontSelectAtCursor = FALSE );

    virtual BOOL    IsSelectionAtPoint( const Point& rPointPixel );
    virtual void    DeselectAll();

    virtual void    DeselectAtPoint( const Point& );
    virtual void    DestroyAnchor();
};


class IdleFormatter : public Timer
{
private:
    TextView*   mpView;
    USHORT      mnRestarts;

public:
                IdleFormatter();
                ~IdleFormatter();

    void        DoIdleFormat( TextView* pV, USHORT nMaxRestarts );
    void        ForceTimeout();
    TextView*   GetView()       { return mpView; }
};

struct TextDDInfo
{
    Cursor          maCursor;
    TextPaM         maDropPos;

    BOOL            mbStarterOfDD;
    BOOL            mbVisCursor;

    TextDDInfo()
    {
        maCursor.SetStyle( CURSOR_SHADOW );
        mbStarterOfDD = FALSE;
        mbVisCursor = FALSE;
    }
};

#endif // _TEXTDAT2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
