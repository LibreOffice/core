/*************************************************************************
 *
 *  $RCSfile: textdat2.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _TEXTDAT2_HXX
#define _TEXTDAT2_HXX

#ifndef _SVARRAY_HXX
#include <svarray.hxx>
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif

#ifndef _SV_SELENG_HXX //autogen
#include <vcl/seleng.hxx>
#endif

#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _SV_CURSOR_HXX
#include <vcl/cursor.hxx>
#endif

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


class TextPortion
{
private:
    USHORT      nLen;
    long        nWidth;
    BYTE        nKind;

                TextPortion()               { nLen = 0; nKind = PORTIONKIND_TEXT; nWidth = -1;}

public:
                TextPortion( USHORT nL )    {
                                                nLen = nL;
                                                nKind = PORTIONKIND_TEXT;
                                                nWidth= -1;
                                            }

    USHORT      GetLen() const              { return nLen; }
    USHORT&     GetLen()                    { return nLen; }

    long        GetWidth()const             { return nWidth; }
    long&       GetWidth()                  { return nWidth; }

    BYTE        GetKind() const             { return nKind; }
    BYTE&       GetKind()                   { return nKind; }

    BOOL        HasValidSize() const        { return nWidth != (-1); }
};



typedef TextPortion* TextPortionPtr;
SV_DECL_PTRARR( TextPortionArray, TextPortionPtr, 0, 8 );

class TETextPortionList : public TextPortionArray
{
public:
            TETextPortionList();
            ~TETextPortionList();

    void    Reset();
    USHORT  FindPortion( USHORT nCharPos, USHORT& rPortionStart );
    void    DeleteFromPortion( USHORT nDelFrom );
};



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
SV_DECL_PTRARR_DEL( TextLines, TextLinePtr, 1, 4 );

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
    TextNode*           mpNode;

    TextLines           maLines;
    TETextPortionList       maTextPortions;

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

    USHORT              GetLineNumber( USHORT nIndex, BOOL bInclEnd );
    void                CorrectValuesBehindLastFormattedLine( USHORT nLastFormattedLine );
};




DECLARE_LIST( DummyTEParaPortionList, TEParaPortion* );

class TEParaPortions : public DummyTEParaPortionList
{
public:
                    TEParaPortions();
                    ~TEParaPortions();

    void            Reset();
//  long            GetYOffset( TEParaPortion* pPPortion );
//  USHORT          FindParagraph( long nYOffset );
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

    void        DoIdleFormat( TextView* pV );
    void        ForceTimeout();
    void        ResetRestarts() { mnRestarts = 0; }
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
