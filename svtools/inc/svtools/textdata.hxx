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
    sal_uLong           mnPara;
    sal_uInt16          mnIndex;

public:
                    TextPaM()                               { mnPara = 0, mnIndex = 0; }
                    TextPaM( sal_uLong nPara, sal_uInt16 nIndex )   { mnPara = nPara, mnIndex = nIndex; }

    sal_uLong           GetPara() const     { return mnPara; }
    sal_uLong&          GetPara()           { return mnPara; }

    sal_uInt16          GetIndex() const    { return mnIndex; }
    sal_uInt16&         GetIndex()          { return mnIndex; }

    inline sal_Bool     operator == ( const TextPaM& rPaM ) const;
    inline sal_Bool     operator != ( const TextPaM& rPaM ) const;
    inline sal_Bool     operator < ( const TextPaM& rPaM ) const;
    inline sal_Bool     operator > ( const TextPaM& rPaM ) const;
};

inline sal_Bool TextPaM::operator == ( const TextPaM& rPaM ) const
{
    return ( ( mnPara == rPaM.mnPara ) && ( mnIndex == rPaM.mnIndex ) ) ? sal_True : sal_False;
}

inline sal_Bool TextPaM::operator != ( const TextPaM& rPaM ) const
{
    return !( *this == rPaM );
}

inline sal_Bool TextPaM::operator < ( const TextPaM& rPaM ) const
{
    return ( ( mnPara < rPaM.mnPara ) ||
             ( ( mnPara == rPaM.mnPara ) && mnIndex < rPaM.mnIndex ) ) ? sal_True : sal_False;
}

inline sal_Bool TextPaM::operator > ( const TextPaM& rPaM ) const
{
    return ( ( mnPara > rPaM.mnPara ) ||
             ( ( mnPara == rPaM.mnPara ) && mnIndex > rPaM.mnIndex ) ) ? sal_True : sal_False;
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

    sal_Bool            HasRange() const    { return maStartPaM != maEndPaM; }

    inline sal_Bool     operator == ( const TextSelection& rSel ) const;
    inline sal_Bool     operator != ( const TextSelection& rSel ) const;
};

inline sal_Bool TextSelection::operator == ( const TextSelection& rSel ) const
{
    return ( ( maStartPaM == rSel.maStartPaM ) && ( maEndPaM == rSel.maEndPaM ) );
}

inline sal_Bool TextSelection::operator != ( const TextSelection& rSel ) const
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
    sal_uLong   mnValue;

public:
            TYPEINFO();
            TextHint( sal_uLong nId );
            TextHint( sal_uLong nId, sal_uLong nValue );

    sal_uLong   GetValue() const        { return mnValue; }
    void    SetValue( sal_uLong n )     { mnValue = n; }
};

struct TEIMEInfos
{
    String  aOldTextAfterStartPos;
    sal_uInt16* pAttribs;
    TextPaM aPos;
    sal_uInt16  nLen;
    sal_Bool    bCursor;
    sal_Bool    bWasCursorOverwrite;

            TEIMEInfos( const TextPaM& rPos, const String& rOldTextAfterStartPos );
            ~TEIMEInfos();

    void    CopyAttribs( const sal_uInt16* pA, sal_uInt16 nL );
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
    sal_uLong           Count() const { return static_cast<sal_uLong>(::std::vector< T >::size()); }
    sal_uLong           GetPos( T pObject ) const { return ( ::std::find( this->begin(), this->end(), pObject ) ) - this->begin(); }
    T               GetObject( sal_uLong nIndex ) const { return (*this)[nIndex]; }
    void            Insert( T pObject, sal_uLong nPos ) { ::std::vector< T >::insert( this->begin()+nPos, pObject ); }
    void            Remove( sal_uLong nPos ) { ::std::vector< T >::erase( this->begin()+nPos ); }
};

#endif // _TEXTDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
