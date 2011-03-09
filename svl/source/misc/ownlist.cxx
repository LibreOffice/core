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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"
#include <ctype.h>
#include <stdio.h>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <svl/ownlist.hxx>

using namespace com::sun::star;

//=========================================================================
//============== SvCommandList ============================================
//=========================================================================
PRV_SV_IMPL_OWNER_LIST(SvCommandList,SvCommand)


static String parseString(const String & rCmd, sal_uInt16 * pIndex)
{
    String result;

    if(rCmd.GetChar( *pIndex ) == '\"') {
        (*pIndex) ++;

        sal_uInt16 begin = *pIndex;

        while(*pIndex < rCmd.Len() && rCmd.GetChar((*pIndex) ++) != '\"') ;

        result = String(rCmd.Copy(begin, *pIndex - begin - 1));
    }

    return result;
}

static String parseWord(const String & rCmd, sal_uInt16 * pIndex)
{
    sal_uInt16 begin = *pIndex;

    while(*pIndex < rCmd.Len() && !isspace(rCmd.GetChar(*pIndex)) && rCmd.GetChar(*pIndex) != '=')
        (*pIndex) ++;

    return String(rCmd.Copy(begin, *pIndex - begin));
}

static void eatSpace(const String & rCmd, sal_uInt16 * pIndex)
{
    while(*pIndex < rCmd.Len() && isspace(rCmd.GetChar(*pIndex)))
        (*pIndex) ++;
}


//=========================================================================
sal_Bool SvCommandList::AppendCommands
(
    const String & rCmd,    /* Dieser Text wird in Kommandos umgesetzt */
    sal_uInt16 * pEaten         /* Anzahl der Zeichen, die gelesen wurden */
)
/*  [Beschreibung]

    Es wird eine Text geparsed und die einzelnen Kommandos werden an
    die Liste angeh"angt.

    [R"uckgabewert]

    sal_Bool        sal_True, der Text wurde korrekt geparsed.
                sal_False, der Text wurde nicht korrekt geparsed.
*/
{
    sal_uInt16 index = 0;
    while(index < rCmd.Len())
    {

        eatSpace(rCmd, &index);
        String name = (rCmd.GetChar(index) == '\"') ? parseString(rCmd, &index) : parseWord(rCmd, &index);

        eatSpace(rCmd, &index);
        String value;
        if(index < rCmd.Len() && rCmd.GetChar(index) == '=')
        {
            index ++;

            eatSpace(rCmd, &index);
            value = (rCmd.GetChar(index) == '\"') ? parseString(rCmd, &index) : parseWord(rCmd, &index);
        }

        SvCommand * pCmd = new SvCommand(name, value);
        aTypes.Insert(pCmd, LIST_APPEND);
    }

    *pEaten = index;

//      sal_uInt16 nPos = 0;
//      while( nPos < rCmd.Len() )
//      {
//          // ein Zeichen ? Dann faengt hier eine Option an
//          if( isalpha( rCmd[nPos] ) )
//          {
//              String aValue;
//              sal_uInt16 nStt = nPos;
//              register char c;

//              while( nPos < rCmd.Len() &&
//                      ( isalnum(c=rCmd[nPos]) || '-'==c || '.'==c ) )
//                  nPos++;

//              String aToken( rCmd.Copy( nStt, nPos-nStt ) );

//              while( nPos < rCmd.Len() &&
//                      ( !String::IsPrintable( (c=rCmd[nPos]),
//                      RTL_TEXTENCODING_MS_1252 ) || isspace(c) ) )
//                  nPos++;

//              // hat die Option auch einen Wert?
//              if( nPos!=rCmd.Len() && '='==c )
//              {
//                  nPos++;

//                  while( nPos < rCmd.Len() &&
//                          ( !String::IsPrintable( (c=rCmd[nPos]),
//                          RTL_TEXTENCODING_MS_1252 ) || isspace(c) ) )
//                      nPos++;

//                  if( nPos != rCmd.Len() )
//                  {
//                      sal_uInt16 nLen = 0;
//                      nStt = nPos;
//                      if( '"' == c )
//                      {
//                          nPos++; nStt++;
//                          while( nPos < rCmd.Len() &&
//                                  '"' != rCmd[nPos] )
//                              nPos++, nLen++;
//                          if( nPos!=rCmd.Len() )
//                              nPos++;
//                      }
//                      else
//                          // hier sind wir etwas laxer als der
//                          // Standard und erlauben alles druckbare
//                          while( nPos < rCmd.Len() &&
//                                  String::IsPrintable( (c=rCmd[nPos]),
//                                  RTL_TEXTENCODING_MS_1252 ) &&
//                                  !isspace( c ) )
//                              nPos++, nLen++;

//                      if( nLen )
//                          aValue = rCmd( nStt, nLen );
//                  }
//              }

//              SvCommand * pCmd = new SvCommand( aToken, aValue );
//              aTypes.Insert( pCmd, LIST_APPEND );
//          }
//          else
//              // white space un unerwartete Zeichen ignorieren wie
//              nPos++;
//      }
//      *pEaten = nPos;
    return sal_True;
}

//=========================================================================
String SvCommandList::GetCommands() const
/*  [Beschreibung]

    Die Kommandos in der Liste werden als Text hintereinander, durch ein
    Leerzeichen getrennt geschrieben. Der Text muss nicht genauso
    aussehen wie der in <SvCommandList::AppendCommands()> "ubergebene.

    [R"uckgabewert]

    String      Die Kommandos werden zur"uckgegeben.
*/
{
    String aRet;
    for( sal_uLong i = 0; i < aTypes.Count(); i++ )
    {
        if( i != 0 )
            aRet += ' ';
        SvCommand * pCmd = (SvCommand *)aTypes.GetObject( i );
        aRet += pCmd->GetCommand();
        if( pCmd->GetArgument().Len() )
        {
            aRet.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "=\"" ) );
            aRet += pCmd->GetArgument();
            aRet.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "\"" ) );
        }
    }
    return aRet;
}

//=========================================================================
SvCommand & SvCommandList::Append
(
    const String & rCommand,    /* das Kommando */
    const String & rArg         /* dasArgument des Kommandos */
)
/*  [Beschreibung]

    Es wird eine Objekt vom Typ SvCommand erzeugt und an die Liste
    angeh"angt.

    [R"uckgabewert]

    SvCommand &     Das erteugte Objekt wird zur"uckgegeben.
*/
{
    SvCommand * pCmd = new SvCommand( rCommand, rArg );
    aTypes.Insert( pCmd, LIST_APPEND );
    return *pCmd;
}

//=========================================================================
SvStream & operator >>
(
    SvStream & rStm,        /* Stream aus dem gelesen wird */
    SvCommandList & rThis   /* Die zu f"ullende Liste */
)
/*  [Beschreibung]

    Die Liste mit ihren Elementen wird gelesen. Das Format ist:
    1. Anzahl der Elemente
    2. Alle Elemente

    [R"uckgabewert]

    SvStream &      Der "ubergebene Stream.
*/
{
    sal_uInt32 nCount = 0;
    rStm >> nCount;
    if( !rStm.GetError() )
    {
        while( nCount-- )
        {
            SvCommand * pCmd = new SvCommand();
            rStm >> *pCmd;
            rThis.aTypes.Insert( pCmd, LIST_APPEND );
        }
    }
    return rStm;
}

//=========================================================================
SvStream & operator <<
(
    SvStream & rStm,            /* Stream in den geschrieben wird */
    const SvCommandList & rThis /* Die zu schreibende Liste */
)
/*  [Beschreibung]

    Die Liste mit ihren Elementen wir geschrieben. Das Format ist:
    1. Anzahl der Elemente
    2. Alle Elemente

    [R"uckgabewert]

    SvStream &      Der "ubergebene Stream.
*/
{
    sal_uInt32 nCount = rThis.aTypes.Count();
    rStm << nCount;

    for( sal_uInt32 i = 0; i < nCount; i++ )
    {
        SvCommand * pCmd = (SvCommand *)rThis.aTypes.GetObject( i );
        rStm << *pCmd;
    }
    return rStm;
}

sal_Bool SvCommandList::FillFromSequence( const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& aCommandSequence )
{
    const sal_Int32 nCount = aCommandSequence.getLength();
    String aCommand, aArg;
    ::rtl::OUString aApiArg;
    for( sal_Int32 nIndex=0; nIndex<nCount; nIndex++ )
    {
        aCommand = aCommandSequence[nIndex].Name;
        if( !( aCommandSequence[nIndex].Value >>= aApiArg ) )
            return sal_False;
        aArg = aApiArg;
        Append( aCommand, aArg );
    }

    return sal_True;
}

void SvCommandList::FillSequence( com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& aCommandSequence )
{
    const sal_Int32 nCount = Count();
    aCommandSequence.realloc( nCount );
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const SvCommand& rCommand = (*this)[ nIndex ];
        aCommandSequence[nIndex].Name = rCommand.GetCommand();
        aCommandSequence[nIndex].Handle = -1;
        aCommandSequence[nIndex].Value = uno::makeAny( ::rtl::OUString( rCommand.GetArgument() ) );
        aCommandSequence[nIndex].State = beans::PropertyState_DIRECT_VALUE;
    }
}

