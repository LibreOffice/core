/*************************************************************************
 *
 *  $RCSfile: resourcemanager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: gt $ $Date: 2004-07-14 11:36:06 $
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

#include "resourcemanager.hxx"

#include <vcl/svapp.hxx>
#include <svtools/solar.hrc>


namespace XmlSec
{
    static ResMgr*          pResMgr = 0;
    static International*   pInternational = 0;

    ResMgr* GetResMgr( void )
    {
        if( !pResMgr )
        {
            ByteString  aName( "xmlsec" );
            aName += ByteString::CreateFromInt32( SOLARUPD );
//          pResMgr = ResMgr::CreateResMgr( aName.GetBuffer(), Application::GetSettings().GetUILanguage() );
            LanguageType    aLang( LANGUAGE_ENGLISH_US );
//          pResMgr = ResMgr::CreateResMgr( aName.GetBuffer(), aLang );
// MT: Change to Locale
            pResMgr = ResMgr::CreateResMgr( aName.GetBuffer());
        }

    return pResMgr;
    }

    International* GetInternational( void )
    {
        if( !pInternational )
        {
            LanguageType    aLang( LANGUAGE_ENGLISH_US );
            pInternational = new International( aLang );
        }
        return pInternational;
    }

    DateTime GetDateTime( const ::com::sun::star::util::DateTime& _rDT )
    {
        return DateTime(
            Date( _rDT.Day, _rDT.Month, _rDT.Year ),
            Time( _rDT.Hours, _rDT.Minutes, _rDT.Seconds, _rDT.HundredthSeconds ) );
    }

    String GetDateTimeString( const ::com::sun::star::util::DateTime& _rDT )
    {
        return GetInternational()->GetDate( GetDateTime( _rDT ) );
    }

    String GetDateString( const ::com::sun::star::util::DateTime& _rDT )
    {
        return GetInternational()->GetDate( GetDateTime( _rDT ) );
    }

    String GetPureContent( const String& _rRawString, const char* _pCommaReplacement, bool _bPreserveId )
    {
        enum STATE { PRE_ID, ID, EQUALSIGN, PRE_CONT, CONT };
        String      s;
        STATE       e = _bPreserveId? PRE_ID : ID;

        const sal_Unicode*  p = _rRawString.GetBuffer();
        sal_Unicode         c;
        const sal_Unicode   cComma = ',';
        const sal_Unicode   cEqualSign = '=';
        const sal_Unicode   cSpace = ' ';
        String              aCommaReplacement;
        if( _pCommaReplacement )
            aCommaReplacement = String::CreateFromAscii( _pCommaReplacement );

        while( *p )
        {
            c = *p;
            switch( e )
            {
                case PRE_ID:
                    if( c != cSpace )
                    {
                        s += c;
                        e = ID;
                    }
                    break;
                case ID:
                    if( _bPreserveId )
                        s += c;

                    if( c == cEqualSign )
                        e = _bPreserveId? PRE_CONT : CONT;
                    break;
//              case EQUALSIGN:
//                  break;
                case PRE_CONT:
                    if( c != cSpace )
                    {
                        s += c;
                        e = CONT;
                    }
                    break;
                case CONT:
                    if( c == cComma )
                    {
                        s += aCommaReplacement;
                        e = _bPreserveId? PRE_ID : ID;
                    }
                    else
                        s += c;
                    break;
            }

            ++p;
        }

//      xub_StrLen  nEquPos = _rRawString.SearchAscii( "=" );
//      if( nEquPos == STRING_NOTFOUND )
//          s = _rRawString;
//      else
//      {
//          ++nEquPos;
//          s = String( _rRawString, nEquPos, STRING_MAXLEN );
//          s.EraseLeadingAndTrailingChars();
//      }

        return s;
    }

    String GetContentPart( const String& _rRawString, const String& _rPartId )
    {
        String      s;

        xub_StrLen  nContStart = _rRawString.Search( _rPartId );
        if( nContStart != STRING_NOTFOUND )
        {
            nContStart += _rPartId.Len();
            ++nContStart;                   // now it's start of content, directly after Id

            xub_StrLen  nContEnd = _rRawString.Search( sal_Unicode( ',' ), nContStart );

            s = String( _rRawString, nContStart, nContEnd - nContStart );
        }

        return s;
    }
}


