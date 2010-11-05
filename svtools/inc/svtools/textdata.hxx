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

#define TEXTUNDO_START              100
#define TEXTUNDO_REMOVECHARS        100
#define TEXTUNDO_CONNECTPARAS       101
#define TEXTUNDO_SPLITPARA          102
#define TEXTUNDO_INSERTCHARS        103
#define TEXTUNDO_DELCONTENT         104
#define TEXTUNDO_DELETE             105
#define TEXTUNDO_CUT                106
#define TEXTUNDO_PASTE              107
#define TEXTUNDO_INSERT             108
#define TEXTUNDO_ATTRIBS            109
#define TEXTUNDO_DRAGANDDROP        110
#define TEXTUNDO_READ               111
#define TEXTUNDO_END                149

#define XTEXTUNDO_START             150
#define XTEXTUNDO_END               199

#define TEXTUNDO_USER               200

// Fuer Notify, wenn alle Absaetze geloescht wurden...
#define TEXT_PARA_ALL               0xFFFFFFFF

class TextPaM
{
private:
    sal_uIntPtr         mnPara;
    sal_uInt16          mnIndex;

public:
                    TextPaM()                               { mnPara = 0, mnIndex = 0; }
                    TextPaM( sal_uIntPtr nPara, sal_uInt16 nIndex )     { mnPara = nPara, mnIndex = nIndex; }

    sal_uIntPtr         GetPara() const     { return mnPara; }
    sal_uIntPtr&            GetPara()           { return mnPara; }

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
    sal_uIntPtr     mnValue;

public:
            TYPEINFO();
            TextHint( sal_uIntPtr nId );
            TextHint( sal_uIntPtr nId, sal_uIntPtr nValue );

    sal_uIntPtr     GetValue() const        { return mnValue; }
    void    SetValue( sal_uIntPtr n )   { mnValue = n; }
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
    sal_uIntPtr         Count() const { return static_cast<sal_uIntPtr>(::std::vector< T >::size()); }
    sal_uIntPtr         GetPos( T pObject ) const { return ( ::std::find( this->begin(), this->end(), pObject ) ) - this->begin(); }
    T               GetObject( sal_uIntPtr nIndex ) const { return (*this)[nIndex]; }
    void            Insert( T pObject, sal_uIntPtr nPos ) { ::std::vector< T >::insert( this->begin()+nPos, pObject ); }
    void            Remove( sal_uIntPtr nPos ) { ::std::vector< T >::erase( this->begin()+nPos ); }
};

#endif // _TEXTDATA_HXX
