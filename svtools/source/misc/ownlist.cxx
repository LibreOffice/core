/*************************************************************************
 *
 *  $RCSfile: ownlist.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:02 $
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
#include <ctype.h>
#include <stdio.h>

#include <ownlist.hxx>

//=========================================================================
//============== SvCommandList ============================================
//=========================================================================
PRV_SV_IMPL_OWNER_LIST(SvCommandList,SvCommand)


static String parseString(const String & rCmd, USHORT * pIndex)
{
    String result;

    if(rCmd.GetChar( *pIndex ) == '\"') {
        (*pIndex) ++;

        USHORT begin = *pIndex;

        while(*pIndex < rCmd.Len() && rCmd.GetChar((*pIndex) ++) != '\"');

        result = String(rCmd.Copy(begin, *pIndex - begin - 1));
    }

    return result;
}

static String parseWord(const String & rCmd, USHORT * pIndex)
{
    USHORT begin = *pIndex;

    while(*pIndex < rCmd.Len() && !isspace(rCmd.GetChar(*pIndex)) && rCmd.GetChar(*pIndex) != '=')
        (*pIndex) ++;

    return String(rCmd.Copy(begin, *pIndex - begin));
}

static void eatSpace(const String & rCmd, USHORT * pIndex)
{
    while(*pIndex < rCmd.Len() && isspace(rCmd.GetChar(*pIndex)))
        (*pIndex) ++;
}


//=========================================================================
BOOL SvCommandList::AppendCommands
(
    const String & rCmd,    /* Dieser Text wird in Kommandos umgesetzt */
    USHORT * pEaten         /* Anzahl der Zeichen, die gelesen wurden */
)
/*  [Beschreibung]

    Es wird eine Text geparsed und die einzelnen Kommandos werden an
    die Liste angeh"angt.

    [R"uckgabewert]

    BOOL        TRUE, der Text wurde korrekt geparsed.
                FALSE, der Text wurde nicht korrekt geparsed.
*/
{
    USHORT index = 0;
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

//      USHORT nPos = 0;
//      while( nPos < rCmd.Len() )
//      {
//          // ein Zeichen ? Dann faengt hier eine Option an
//          if( isalpha( rCmd[nPos] ) )
//          {
//              String aValue;
//              USHORT nStt = nPos;
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
//                      USHORT nLen = 0;
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
    return TRUE;
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
    for( ULONG i = 0; i < aTypes.Count(); i++ )
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
    UINT32 nCount = 0;
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
    UINT32 nCount = rThis.aTypes.Count();
    rStm << nCount;

    for( UINT32 i = 0; i < nCount; i++ )
    {
        SvCommand * pCmd = (SvCommand *)rThis.aTypes.GetObject( i );
        rStm << *pCmd;
    }
    return rStm;
}


