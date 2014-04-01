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
    bool        IsRightToLeft() const       { return (nRightToLeft&1); }

    bool        HasValidSize() const        { return nWidth != (-1); }
};



typedef std::vector<TETextPortion*> TextPortionArray;

class TETextPortionList : public TextPortionArray
{
public:
    TETextPortionList();
    ~TETextPortionList();

    void    Reset();
    sal_uInt16  FindPortion( sal_uInt16 nCharPos, sal_uInt16& rPortionStart, bool bPreferStartingPortion = false );
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

    bool            mbInvalid;  // fuer geschickte Formatierung/Ausgabe

public:
                    TextLine()  {
                                    mnStart = mnEnd = 0;
                                    mnStartPortion = mnEndPortion = 0;
                                    mnStartX = 0;
                                    mbInvalid = true;
                                }

    bool            IsIn( sal_uInt16 nIndex ) const
                        { return ( (nIndex >= mnStart ) && ( nIndex < mnEnd ) ); }

    bool            IsIn( sal_uInt16 nIndex, bool bInclEnd ) const
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

    bool            IsInvalid() const               { return mbInvalid; }
    bool            IsValid() const                 { return !mbInvalid; }
    void            SetInvalid()                    { mbInvalid = true; }
    void            SetValid()                      { mbInvalid = false; }

    bool            IsEmpty() const                 { return (mnEnd > mnStart) ? false : true; }

    short           GetStartX() const               { return mnStartX; }
    void            SetStartX( short n )            { mnStartX = n; }

    inline bool operator == ( const TextLine& rLine ) const;
    inline bool operator != ( const TextLine& rLine ) const;
};

class TextLines : public std::vector<TextLine*> {
public:
    ~TextLines()
    {
        for( iterator it = begin(); it != end(); ++it )
            delete *it;
    }
};

inline bool TextLine::operator == ( const TextLine& rLine ) const
{
    return (    ( mnStart == rLine.mnStart ) &&
                ( mnEnd == rLine.mnEnd ) &&
                ( mnStartPortion == rLine.mnStartPortion ) &&
                ( mnEndPortion == rLine.mnEndPortion ) );
}

inline bool TextLine::operator != ( const TextLine& rLine ) const
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

    bool                mbInvalid;
    bool                mbSimple;   // nur lineares Tippen


                        TEParaPortion( const TEParaPortion& ) {;}

public:
                        TEParaPortion( TextNode* pNode );
                        ~TEParaPortion();


    bool                IsInvalid() const           { return mbInvalid; }
    bool                IsSimpleInvalid() const     { return mbSimple; }
    void                SetNotSimpleInvalid()       { mbSimple = false; }
    void                SetValid()                  { mbInvalid = false; mbSimple = true;}

    void                MarkInvalid( sal_uInt16 nStart, short nDiff);
    void                MarkSelectionInvalid( sal_uInt16 nStart, sal_uInt16 nEnd );

    sal_uInt16              GetInvalidPosStart() const  { return mnInvalidPosStart; }
    short               GetInvalidDiff() const      { return mnInvalidDiff; }

    TextNode*           GetNode() const             { return mpNode; }
    TextLines&          GetLines()                  { return maLines; }
    TETextPortionList&  GetTextPortions()           { return maTextPortions; }
    std::vector<TEWritingDirectionInfo>& GetWritingDirectionInfos() { return maWritingDirectionInfos; }


    sal_uInt16              GetLineNumber( sal_uInt16 nIndex, bool bInclEnd );
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

    virtual void    BeginDrag() SAL_OVERRIDE;

    virtual void    CreateAnchor() SAL_OVERRIDE;

    virtual bool    SetCursorAtPoint( const Point& rPointPixel, bool bDontSelectAtCursor = false ) SAL_OVERRIDE;

    virtual bool    IsSelectionAtPoint( const Point& rPointPixel ) SAL_OVERRIDE;
    virtual void    DeselectAll() SAL_OVERRIDE;

    virtual void    DeselectAtPoint( const Point& ) SAL_OVERRIDE;
    virtual void    DestroyAnchor() SAL_OVERRIDE;
};


class IdleFormatter : public Timer
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
    Cursor          maCursor;
    TextPaM         maDropPos;

    bool            mbStarterOfDD;
    bool            mbVisCursor;

    TextDDInfo()
    {
        maCursor.SetStyle( CURSOR_SHADOW );
        mbStarterOfDD = false;
        mbVisCursor = false;
    }
};

#endif // INCLUDED_VCL_SOURCE_EDIT_TEXTDAT2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
