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

#ifndef _TEXTDATA_HXX
#define _TEXTDATA_HXX

#include "svtools/svtdllapi.h"
#include <svl/brdcst.hxx>
#include <svl/smplhint.hxx>
#include <tools/string.hxx>

// Fuer Notify, wenn alle Absaetze geloescht wurden...
#define TEXT_PARA_ALL               0xFFFFFFFF

class TextPaM
{
private:
    ULONG           mnPara;
    USHORT          mnIndex;

public:
                    TextPaM()                               { mnPara = 0, mnIndex = 0; }
                    TextPaM( ULONG nPara, USHORT nIndex )   { mnPara = nPara, mnIndex = nIndex; }

    ULONG           GetPara() const     { return mnPara; }
    ULONG&          GetPara()           { return mnPara; }

    USHORT          GetIndex() const    { return mnIndex; }
    USHORT&         GetIndex()          { return mnIndex; }

    inline BOOL     operator == ( const TextPaM& rPaM ) const;
    inline BOOL     operator != ( const TextPaM& rPaM ) const;
    inline BOOL     operator < ( const TextPaM& rPaM ) const;
    inline BOOL     operator > ( const TextPaM& rPaM ) const;
};

inline BOOL TextPaM::operator == ( const TextPaM& rPaM ) const
{
    return ( ( mnPara == rPaM.mnPara ) && ( mnIndex == rPaM.mnIndex ) ) ? TRUE : FALSE;
}

inline BOOL TextPaM::operator != ( const TextPaM& rPaM ) const
{
    return !( *this == rPaM );
}

inline BOOL TextPaM::operator < ( const TextPaM& rPaM ) const
{
    return ( ( mnPara < rPaM.mnPara ) ||
             ( ( mnPara == rPaM.mnPara ) && mnIndex < rPaM.mnIndex ) ) ? TRUE : FALSE;
}

inline BOOL TextPaM::operator > ( const TextPaM& rPaM ) const
{
    return ( ( mnPara > rPaM.mnPara ) ||
             ( ( mnPara == rPaM.mnPara ) && mnIndex > rPaM.mnIndex ) ) ? TRUE : FALSE;
}

class SVT_DLLPUBLIC TextSelection
{
private:
    TextPaM         maStartPaM;
    TextPaM         maEndPaM;

public:
                    TextSelection();
                    TextSelection( const TextPaM& rPaM );
                    TextSelection( const TextPaM& rStart, const TextPaM& rEnd );

    const TextPaM&  GetStart() const    { return maStartPaM; }
    TextPaM&        GetStart()          { return maStartPaM; }

    const TextPaM&  GetEnd() const      { return maEndPaM; }
    TextPaM&        GetEnd()            { return maEndPaM; }

    void            Justify();

    BOOL            HasRange() const    { return maStartPaM != maEndPaM; }

    inline BOOL     operator == ( const TextSelection& rSel ) const;
    inline BOOL     operator != ( const TextSelection& rSel ) const;
};

inline BOOL TextSelection::operator == ( const TextSelection& rSel ) const
{
    return ( ( maStartPaM == rSel.maStartPaM ) && ( maEndPaM == rSel.maEndPaM ) );
}

inline BOOL TextSelection::operator != ( const TextSelection& rSel ) const
{
    return !( *this == rSel );
}

#define TEXT_HINT_PARAINSERTED              1
#define TEXT_HINT_PARAREMOVED               2
#define TEXT_HINT_PARACONTENTCHANGED        3
#define TEXT_HINT_TEXTHEIGHTCHANGED         4
#define TEXT_HINT_FORMATPARA                5
#define TEXT_HINT_TEXTFORMATTED             6
#define TEXT_HINT_MODIFIED                  7
#define TEXT_HINT_BLOCKNOTIFICATION_START   8
#define TEXT_HINT_BLOCKNOTIFICATION_END     9
#define TEXT_HINT_INPUT_START               10
#define TEXT_HINT_INPUT_END                 11

#define TEXT_HINT_VIEWSCROLLED          100
#define TEXT_HINT_VIEWSELECTIONCHANGED  101

class SVT_DLLPUBLIC TextHint : public SfxSimpleHint
{
private:
    ULONG   mnValue;

public:
            TYPEINFO();
            TextHint( ULONG nId );
            TextHint( ULONG nId, ULONG nValue );

    ULONG   GetValue() const        { return mnValue; }
    void    SetValue( ULONG n )     { mnValue = n; }
};

struct TEIMEInfos
{
    String  aOldTextAfterStartPos;
    USHORT* pAttribs;
    TextPaM aPos;
    USHORT  nLen;
    BOOL    bCursor;
    BOOL    bWasCursorOverwrite;

            TEIMEInfos( const TextPaM& rPos, const String& rOldTextAfterStartPos );
            ~TEIMEInfos();

    void    CopyAttribs( const USHORT* pA, USHORT nL );
    void    DestroyAttribs();
};

// -----------------  Wrapper for old Tools List -------------------

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif

template <class T> class ToolsList : public ::std::vector< T >
{
public:
    ULONG           Count() const { return static_cast<ULONG>(::std::vector< T >::size()); }
    ULONG           GetPos( T pObject ) const { return ( ::std::find( this->begin(), this->end(), pObject ) ) - this->begin(); }
    T               GetObject( ULONG nIndex ) const { return (*this)[nIndex]; }
    void            Insert( T pObject, ULONG nPos ) { ::std::vector< T >::insert( this->begin()+nPos, pObject ); }
    void            Remove( ULONG nPos ) { ::std::vector< T >::erase( this->begin()+nPos ); }
};

#endif // _TEXTDATA_HXX
