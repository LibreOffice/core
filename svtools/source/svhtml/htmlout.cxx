/*************************************************************************
 *
 *  $RCSfile: htmlout.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-28 17:11:54 $
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

#include <tools/urlobj.hxx>

#ifndef _ZFORMAT_HXX //autogen
#include <zformat.hxx>
#endif

#ifndef _SFXMACITEM_HXX
#include <macitem.hxx>
#endif
#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif
#ifndef _SV_SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif

#ifndef _ZFORLIST_HXX //autogen
#include <zforlist.hxx>
#endif

#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif

#include "htmlout.hxx"
#include "htmlkywd.hxx"
#include "imap.hxx"
#include "imaprect.hxx"
#include "imapcirc.hxx"
#include "imappoly.hxx"

#ifndef RTL_CONSTASCII_STRINGPARAM
#define RTL_CONSTASCII_STRINGPARAM( c ) c, sizeof(c)-1
#endif

#if defined(MAC)
const sal_Char HTMLOutFuncs::sNewLine = '\015';
#elif defined(UNX)
const sal_Char HTMLOutFuncs::sNewLine = '\012';
#else
const sal_Char __FAR_DATA HTMLOutFuncs::sNewLine[] = "\015\012";
#endif

const sal_Char *lcl_svhtml_GetEntityForANSIChar( sal_Char c )
{
    const sal_Char* pStr = 0;
    switch( (unsigned char)c )
    {
//      case '\x0a':   return HTMLOutFuncs::Out_Tag( rStream, sHTML_linebreak );

    case '<':       pStr = sHTML_C_lt;      break;
    case '>':       pStr = sHTML_C_gt;      break;
    case '&':       pStr = sHTML_C_amp;     break;
    case '"':       pStr = sHTML_C_quot;    break;

    case 161:       pStr = sHTML_S_iexcl;   break;
    case 162:       pStr = sHTML_S_cent;        break;
    case 163:       pStr = sHTML_S_pound;   break;
    case 164:       pStr = sHTML_S_curren;  break;
    case 165:       pStr = sHTML_S_yen;     break;
    case 166:       pStr = sHTML_S_brvbar;  break;
    case 167:       pStr = sHTML_S_sect;    break;
    case 168:       pStr = sHTML_S_uml;     break;
    case 169:       pStr = sHTML_S_copy;    break;
    case 170:       pStr = sHTML_S_ordf;    break;
    case 171:       pStr = sHTML_S_laquo;   break;
    case 172:       pStr = sHTML_S_not;     break;
    case 174:       pStr = sHTML_S_reg;     break;
    case 175:       pStr = sHTML_S_macr;    break;
    case 176:       pStr = sHTML_S_deg;     break;
    case 177:       pStr = sHTML_S_plusmn;  break;
    case 178:       pStr = sHTML_S_sup2;    break;
    case 179:       pStr = sHTML_S_sup3;    break;
    case 180:       pStr = sHTML_S_acute;   break;
    case 181:       pStr = sHTML_S_micro;   break;
    case 182:       pStr = sHTML_S_para;    break;
    case 183:       pStr = sHTML_S_middot;  break;
    case 184:       pStr = sHTML_S_cedil;   break;
    case 185:       pStr = sHTML_S_sup1;    break;
    case 186:       pStr = sHTML_S_ordm;    break;
    case 187:       pStr = sHTML_S_raquo;   break;
    case 188:       pStr = sHTML_S_frac14;  break;
    case 189:       pStr = sHTML_S_frac12;  break;
    case 190:       pStr = sHTML_S_frac34;  break;
    case 191:       pStr = sHTML_S_iquest;  break;

    case 192:       pStr = sHTML_C_Agrave;  break;
    case 193:       pStr = sHTML_C_Aacute;  break;
    case 194:       pStr = sHTML_C_Acirc;   break;
    case 195:       pStr = sHTML_C_Atilde;  break;
    case 196:       pStr = sHTML_C_Auml;    break;
    case 197:       pStr = sHTML_C_Aring;   break;
    case 198:       pStr = sHTML_C_AElig;   break;
    case 199:       pStr = sHTML_C_Ccedil;  break;
    case 200:       pStr = sHTML_C_Egrave;  break;
    case 201:       pStr = sHTML_C_Eacute;  break;
    case 202:       pStr = sHTML_C_Ecirc;   break;
    case 203:       pStr = sHTML_C_Euml;    break;
    case 204:       pStr = sHTML_C_Igrave;  break;
    case 205:       pStr = sHTML_C_Iacute;  break;
    case 206:       pStr = sHTML_C_Icirc;   break;
    case 207:       pStr = sHTML_C_Iuml;    break;
    case 208:       pStr = sHTML_C_ETH;     break;
    case 209:       pStr = sHTML_C_Ntilde;  break;
    case 210:       pStr = sHTML_C_Ograve;  break;
    case 211:       pStr = sHTML_C_Oacute;  break;
    case 212:       pStr = sHTML_C_Ocirc;   break;
    case 213:       pStr = sHTML_C_Otilde;  break;
    case 214:       pStr = sHTML_C_Ouml;    break;
    case 215:       pStr = sHTML_S_times;   break;
    case 216:       pStr = sHTML_C_Oslash;  break;
    case 217:       pStr = sHTML_C_Ugrave;  break;
    case 218:       pStr = sHTML_C_Uacute;  break;
    case 219:       pStr = sHTML_C_Ucirc;   break;
    case 220:       pStr = sHTML_C_Uuml;    break;
    case 221:       pStr = sHTML_C_Yacute;  break;

    case 222:       pStr = sHTML_C_THORN;   break;
    case 223:       pStr = sHTML_C_szlig;   break;

    case 224:       pStr = sHTML_S_agrave;  break;
    case 225:       pStr = sHTML_S_aacute;  break;
    case 226:       pStr = sHTML_S_acirc;   break;
    case 227:       pStr = sHTML_S_atilde;  break;
    case 228:       pStr = sHTML_S_auml;    break;
    case 229:       pStr = sHTML_S_aring;   break;
    case 230:       pStr = sHTML_S_aelig;   break;
    case 231:       pStr = sHTML_S_ccedil;  break;
    case 232:       pStr = sHTML_S_egrave;  break;
    case 233:       pStr = sHTML_S_eacute;  break;
    case 234:       pStr = sHTML_S_ecirc;   break;
    case 235:       pStr = sHTML_S_euml;    break;
    case 236:       pStr = sHTML_S_igrave;  break;
    case 237:       pStr = sHTML_S_iacute;  break;
    case 238:       pStr = sHTML_S_icirc;   break;
    case 239:       pStr = sHTML_S_iuml;    break;
    case 240:       pStr = sHTML_S_eth;     break;
    case 241:       pStr = sHTML_S_ntilde;  break;
    case 242:       pStr = sHTML_S_ograve;  break;
    case 243:       pStr = sHTML_S_oacute;  break;
    case 244:       pStr = sHTML_S_ocirc;   break;
    case 245:       pStr = sHTML_S_otilde;  break;
    case 246:       pStr = sHTML_S_ouml;    break;
    case 247:       pStr = sHTML_S_divide;  break;
    case 248:       pStr = sHTML_S_oslash;  break;
    case 249:       pStr = sHTML_S_ugrave;  break;
    case 250:       pStr = sHTML_S_uacute;  break;
    case 251:       pStr = sHTML_S_ucirc;   break;
    case 252:       pStr = sHTML_S_uuml;    break;
    case 253:       pStr = sHTML_S_yacute;  break;
    case 254:       pStr = sHTML_S_thorn;   break;
    case 255:       pStr = sHTML_S_yuml;    break;

    }

    return pStr;
}

void lcl_ConvertCharToHTML( sal_Unicode c, ByteString& rDest,
                            rtl_TextEncoding eDestEnc )
{
    DBG_ASSERT( RTL_TEXTENCODING_DONTKNOW != eDestEnc,
                    "wrong destination encoding" );
    const sal_Char *pStr = 0;
    sal_Char cANSI = 0;
    switch( c )
    {
    case 0xA0:      // is a hard blank
//!! the TextConverter has a problem with this character - so change it to
// a hard space - that's the same as our 5.2
    case 0x2011:    // is a hard hyphen
        pStr = sHTML_S_nbsp;
        break;
    case 0xAD:      // is a soft hyphen
        pStr = sHTML_S_shy;
        break;

    default:
        // Convert character to ISO8859-1 temporarily
        cANSI = ByteString::ConvertFromUnicode(
                                    c, RTL_TEXTENCODING_ISO_8859_1, FALSE );
        // If it could be converted, there may be an entity for the character.
        if( cANSI )
            pStr = lcl_svhtml_GetEntityForANSIChar( cANSI );
    }

    if( pStr )
    {
        ((rDest += '&') += pStr) += ';';
    }
    else
    {
        switch( eDestEnc )
        {
        case RTL_TEXTENCODING_ISO_8859_1:
            rDest += (cANSI ? cANSI : (sal_Char)c);
            break;
        case RTL_TEXTENCODING_UTF8:
            {
                sal_Char cBuffer[3];
                size_t nLen = ByteString::ConvertFromUnicode( c, cBuffer, 3,
                                                        RTL_TEXTENCODING_UTF8 );
                if( nLen )
                {
                    sal_Char *pBuffer = cBuffer;
                    while( nLen-- )
                        rDest += *pBuffer++;
                }
                else
                {
                    rDest += (sal_Char)c;
                }
            }
            break;
        default:
            {
                // We allow 8 bit encoding here only
                sal_Char cOut = ByteString::ConvertFromUnicode( c,
                                                        eDestEnc, FALSE );
                // If the character could not be converted to the destination
                // character set, the original character code is truncated to
                // an 8-bit-character code.
                if( !cOut )
                    cOut = (sal_Char)c;
                if( (cOut>=' ' && cOut<='~') || cOut=='\t' )
                    rDest += cOut;
                else
                    (((rDest += '&') += '#') +=
                        ByteString::CreateFromInt32( (sal_Int32)cOut )) += ';';
            }
        }
    }
}

void HTMLOutFuncs::ConvertStringToHTML( const String& rSrc,
                                        ByteString& rDest,
                                        rtl_TextEncoding eDestEnc )
{
    if( RTL_TEXTENCODING_DONTKNOW == eDestEnc )
        eDestEnc = gsl_getSystemTextEncoding();

    for( sal_uInt32 i=0UL, nLen = rSrc.Len(); i < nLen; i++ )
        lcl_ConvertCharToHTML( rSrc.GetChar( (xub_StrLen)i ), rDest,  eDestEnc );
}

SvStream& HTMLOutFuncs::Out_AsciiTag( SvStream& rStream, const sal_Char *pStr,
                                      BOOL bOn, rtl_TextEncoding )
{
    sal_Char sStt[3] = "</";
    if( bOn )
        sStt[1] = 0;
    return (rStream << sStt << pStr << '>');
}

SvStream& HTMLOutFuncs::Out_Char( SvStream& rStream, sal_Unicode c,
                                  rtl_TextEncoding eDestEnc )
{
    if( RTL_TEXTENCODING_DONTKNOW == eDestEnc )
        eDestEnc = gsl_getSystemTextEncoding();

    ByteString sOut;
    lcl_ConvertCharToHTML( c, sOut,  eDestEnc );
    rStream << sOut.GetBuffer();
    return rStream;
}

SvStream& HTMLOutFuncs::Out_String( SvStream& rStream, const String& rStr,
                                    rtl_TextEncoding eDestEnc )
{
    for( sal_uInt32 n = 0UL; n < rStr.Len(); n++ )
        HTMLOutFuncs::Out_Char( rStream, rStr.GetChar( (xub_StrLen)n ), eDestEnc );
    return rStream;
}

SvStream& HTMLOutFuncs::Out_Hex( SvStream& rStream, ULONG nHex, BYTE nLen,
                                   rtl_TextEncoding )
{                                                  // in einen Stream aus
    sal_Char aNToABuf[] = "0000000000000000";

    DBG_ASSERT( nLen < sizeof(aNToABuf), "zu viele Stellen" );
    if( nLen>=sizeof(aNToABuf) )
        nLen = (sizeof(aNToABuf)-1);

    // Pointer an das Bufferende setzen
    sal_Char *pStr = aNToABuf + (sizeof(aNToABuf)-1);
    for( BYTE n = 0; n < nLen; ++n )
    {
        *(--pStr) = (sal_Char)(nHex & 0xf ) + 48;
        if( *pStr > '9' )
            *pStr += 39;
        nHex >>= 4;
    }
    return rStream << pStr;
}


SvStream& HTMLOutFuncs::Out_Color( SvStream& rStream, const Color& rColor,
                                   rtl_TextEncoding )
{
    rStream << "\"#";
    Out_Hex( rStream, rColor.GetRed(), 2 );
    Out_Hex( rStream, rColor.GetGreen(), 2 );
    Out_Hex( rStream, rColor.GetBlue(), 2 );
    rStream << '\"';

    return rStream;
}

SvStream& HTMLOutFuncs::Out_ImageMap( SvStream& rStream,
                                      const ImageMap& rIMap,
                                      const String& rName,
                                      const HTMLOutEvent *pEventTable,
                                      BOOL bOutStarBasic,
                                      const sal_Char *pDelim,
                                      const sal_Char *pIndentArea,
                                      const sal_Char *pIndentMap,
                                      rtl_TextEncoding eDestEnc )
{
    if( RTL_TEXTENCODING_DONTKNOW == eDestEnc )
        eDestEnc = gsl_getSystemTextEncoding();

    const String& rOutName = rName.Len() ? rName : rIMap.GetName();
    DBG_ASSERT( rOutName.Len(), "Kein ImageMap-Name" )
    if( !rOutName.Len() )
        return rStream;

    ByteString sOut( '<' );
    sOut.Append( RTL_CONSTASCII_STRINGPARAM(sHTML_map ) );
    sOut.Append( ' ' );
    sOut.Append( RTL_CONSTASCII_STRINGPARAM(sHTML_O_name) );
    sOut.Append( RTL_CONSTASCII_STRINGPARAM("=\"") );
    rStream << sOut.GetBuffer();
    sOut.Erase();
    Out_String( rStream, rOutName, eDestEnc );
    rStream << "\">";

    for( USHORT i=0U; i<rIMap.GetIMapObjectCount(); i++ )
    {
        const IMapObject* pObj = rIMap.GetIMapObject( i );
        DBG_ASSERT( pObj, "Wo ist das ImageMap-Object?" );

        if( pObj )
        {
            const sal_Char *pShape = 0;
            ByteString aCoords;
            switch( pObj->GetType() )
            {
            case( IMAP_OBJ_RECTANGLE ):
                {
                    const IMapRectangleObject* pRectObj =
                        (const IMapRectangleObject *)pObj;
                    pShape = sHTML_SH_rect;
                    Rectangle aRect( pRectObj->GetRectangle() );
                    ((((((aCoords =
                        ByteString::CreateFromInt32(aRect.Left())) += ',')
                        += ByteString::CreateFromInt32(aRect.Top())) += ',')
                        += ByteString::CreateFromInt32(aRect.Right())) += ',')
                        += ByteString::CreateFromInt32(aRect.Bottom());
                }
                break;
            case( IMAP_OBJ_CIRCLE ):
                {
                    const IMapCircleObject* pCirc =
                        (const IMapCircleObject *)pObj;
                    pShape= sHTML_SH_circ;
                    Point aCenter( pCirc->GetCenter() );
                    long nOff = pCirc->GetRadius();
                    ((((aCoords =
                        ByteString::CreateFromInt32(aCenter.X())) += ',')
                        += ByteString::CreateFromInt32(aCenter.Y())) += ',')
                        += ByteString::CreateFromInt32(nOff);
                }
                break;
            case( IMAP_OBJ_POLYGON ):
                {
                    const IMapPolygonObject* pPolyObj =
                        (const IMapPolygonObject *)pObj;
                    pShape= sHTML_SH_poly;
                    Polygon aPoly( pPolyObj->GetPolygon() );
                    USHORT nCount = aPoly.GetSize();
                    if( nCount>0 )
                    {
                        const Point& rPoint = aPoly[0];
                        ((aCoords =
                            ByteString::CreateFromInt32(rPoint.X())) += ',')
                            += ByteString::CreateFromInt32(rPoint.Y());
                    }
                    for( USHORT j=1; j<nCount; j++ )
                    {
                        const Point& rPoint = aPoly[j];
                        (((aCoords += ',')
                              += ByteString::CreateFromInt32(rPoint.X())) += ',')
                            += ByteString::CreateFromInt32(rPoint.Y());
                    }
                }
                break;
            default:
                DBG_ASSERT( pShape, "unbekanntes IMapObject" );
                break;
            }

            if( pShape )
            {
                if( pDelim )
                    rStream << pDelim;
                if( pIndentArea )
                    rStream << pIndentArea;

                ((((((((((sOut = '<') += sHTML_area) += ' ')
                    += sHTML_O_shape) += '=') += pShape) += ' ')
                    += sHTML_O_coords) += "=\"") += aCoords) += "\" ";
                rStream << sOut.GetBuffer();

                String aURL( pObj->GetURL() );
                if( aURL.Len() && pObj->IsActive() )
                {
                    aURL = INetURLObject::AbsToRel( aURL );
                    (sOut = sHTML_O_href) += "=\"";
                    rStream << sOut.GetBuffer();
                    Out_String( rStream, aURL, eDestEnc ) << '\"';
                }
                else
                    rStream << sHTML_O_nohref;

                const String& rName = pObj->GetName();
                if( rName.Len() )
                {
                    ((sOut = ' ') += sHTML_O_name) += "=\"";
                    rStream << sOut.GetBuffer();
                    Out_String( rStream, rName, eDestEnc ) << '\"';
                }

                const String& rTarget = pObj->GetTarget();
                if( rTarget.Len() && pObj->IsActive() )
                {
                    ((sOut = ' ') += sHTML_O_target) += "=\"";
                    rStream << sOut.GetBuffer();
                    Out_String( rStream, rTarget, eDestEnc ) << '\"';
                }

                const String& rDesc = pObj->GetDescription();
                if( rDesc.Len() )
                {
                    ((sOut = ' ') += sHTML_O_alt) += "=\"";
                    rStream << sOut.GetBuffer();
                    Out_String( rStream, rDesc, eDestEnc ) << '\"';
                }

                const SvxMacroTableDtor& rMacroTab = pObj->GetMacroTable();
                if( pEventTable && rMacroTab.Count() )
                    Out_Events( rStream, rMacroTab, pEventTable,
                                bOutStarBasic, eDestEnc );

                rStream << '>';
            }
        }

    }

    if( pDelim )
        rStream << pDelim;
    if( pIndentMap )
        rStream << pIndentMap;
    Out_AsciiTag( rStream, sHTML_map, FALSE );

    return rStream;
}

SvStream& HTMLOutFuncs::OutScript( SvStream& rStrm, const String& rSource,
                                   const String& rLanguage,
                                   ScriptType eScriptType,
                                   const String& rSrc,
                                   const String *pSBLibrary,
                                   const String *pSBModule,
                                   rtl_TextEncoding eDestEnc )
{
    if( RTL_TEXTENCODING_DONTKNOW == eDestEnc )
        eDestEnc = gsl_getSystemTextEncoding();

    // Script wird komplett nicht eingerueckt!
    ByteString sOut( '<' );
    sOut.Append( RTL_CONSTASCII_STRINGPARAM(sHTML_script) );

    if( rLanguage.Len() )
    {
        sOut.Append( ' ' );
        sOut.Append( RTL_CONSTASCII_STRINGPARAM(sHTML_O_language) );
        sOut.Append( RTL_CONSTASCII_STRINGPARAM("=\"") );
        rStrm << sOut.GetBuffer();
        Out_String( rStrm, rLanguage, eDestEnc );
        sOut = '\"';
    }

    if( rSrc.Len() )
    {
        ((sOut += ' ') += sHTML_O_src) += "=\"";
        rStrm << sOut.GetBuffer();
        Out_String( rStrm, INetURLObject::AbsToRel(rSrc), eDestEnc );
        sOut = '\"';
    }

    if( STARBASIC != eScriptType && pSBLibrary )
    {
        ((sOut += ' ') += sHTML_O_sdlibrary) += "=\"";
        rStrm << sOut.GetBuffer();
        Out_String( rStrm, *pSBLibrary, eDestEnc );
        sOut = '\"';
    }

    if( STARBASIC != eScriptType && pSBModule )
    {
        ((sOut += ' ') += sHTML_O_sdmodule) += "=\"";
        rStrm << sOut.GetBuffer();
        Out_String( rStrm, *pSBModule, eDestEnc );
        sOut = '\"';
    }

    sOut += '>';

    rStrm << sOut.GetBuffer();

    if( rSource.Len() || pSBLibrary || pSBModule )
    {
        rStrm << sNewLine;

        if( JAVASCRIPT != eScriptType )
        {
            rStrm << "<!--"
                  << sNewLine;
        }

        if( STARBASIC == eScriptType )
        {
            if( pSBLibrary )
            {
                sOut.Assign( RTL_CONSTASCII_STRINGPARAM("' ") );
                sOut.Append( RTL_CONSTASCII_STRINGPARAM(sHTML_SB_library) );
                sOut.Append( ' ' );
                ByteString sTmp( *pSBLibrary, eDestEnc );
                sOut.Append( sTmp );
                rStrm << sOut.GetBuffer() << sNewLine;
            }

            if( pSBModule )
            {
                sOut.Assign( RTL_CONSTASCII_STRINGPARAM("' ") );
                sOut.Append( RTL_CONSTASCII_STRINGPARAM(sHTML_SB_module) );
                sOut.Append( ' ' );
                ByteString sTmp( *pSBModule, eDestEnc );
                sOut.Append( sTmp );
                rStrm << sOut.GetBuffer() << sNewLine;
            }
        }

        if( rSource.Len() )
        {
            // Wir schreiben das Modul mm ANSI-Zeichensatz, aber mit
            // System-Zeilenumbruechen raus.
            ByteString sSource( rSource, eDestEnc );
            sSource.ConvertLineEnd( GetSystemLineEnd() );
            rStrm << sSource.GetBuffer();
        }
        rStrm << sNewLine;

        if( JAVASCRIPT != eScriptType )
        {
            // MIB/MM: Wenn es kein StarBasic ist, kann ein // natuerlich
            // falsch sein. Da der Kommentar aber beim Einlesen wider
            // entfernt wird, schickt uns das nicht weiter ...
            rStrm << (STARBASIC == eScriptType ? "' -->" : "// -->")
                  << sNewLine;
        }
    }

    HTMLOutFuncs::Out_AsciiTag( rStrm, sHTML_script, FALSE );

    return rStrm;
}


SvStream& HTMLOutFuncs::Out_Events( SvStream& rStrm,
                                    const SvxMacroTableDtor& rMacroTable,
                                    const HTMLOutEvent *pEventTable,
                                    BOOL bOutStarBasic,
                                    rtl_TextEncoding eDestEnc )
{
    USHORT i=0;
    while( pEventTable[i].pBasicName || pEventTable[i].pJavaName )
    {
        const SvxMacro *pMacro =
            rMacroTable.Get( pEventTable[i].nEvent );

        if( pMacro && pMacro->GetMacName().Len() &&
            ( JAVASCRIPT == pMacro->GetScriptType() || bOutStarBasic ))
        {
            const sal_Char *pStr = STARBASIC == pMacro->GetScriptType()
                ? pEventTable[i].pBasicName
                : pEventTable[i].pJavaName;

            if( pStr )
            {
                ByteString sOut( ' ' );
                (sOut += pStr) += "=\"";
                rStrm << sOut.GetBuffer();

                Out_String( rStrm, pMacro->GetMacName(), eDestEnc ) << '\"';
            }
        }
        i++;
    }

    return rStrm;
}

ByteString& HTMLOutFuncs::CreateTableDataOptionsValNum( ByteString& aStrTD,
            BOOL bValue,
            double fVal, ULONG nFormat, SvNumberFormatter& rFormatter,
            rtl_TextEncoding eDestEnc )
{
    if ( bValue )
    {
        // printf / scanf ist zu ungenau
        String aValStr;
        rFormatter.GetInputLineString( fVal, 0, aValStr );
        ByteString sTmp( aValStr, eDestEnc );
        ((((aStrTD += ' ') += sHTML_O_SDval) += "=\"") += sTmp) += '\"';
    }
    if ( bValue || nFormat )
    {
        ((aStrTD += ' ') += sHTML_O_SDnum) += "=\"";
        (aStrTD += ByteString::CreateFromInt32(System::GetLanguage()))
            += ';'; // Language fuer Format 0
        if ( nFormat )
        {
            ByteString aNumStr;
            LanguageType nLang;
            const SvNumberformat* pFormatEntry = rFormatter.GetEntry( nFormat );
            if ( pFormatEntry )
            {
                ByteString sTmp( pFormatEntry->GetFormatstring(), eDestEnc );
                aNumStr = sTmp;
                nLang = pFormatEntry->GetLanguage();
            }
            else
                nLang = LANGUAGE_SYSTEM;
            ((aStrTD += ByteString::CreateFromInt32(nLang)) += ';') += aNumStr;
        }
        aStrTD += '\"';
    }
    return aStrTD;
}

BOOL HTMLOutFuncs::PrivateURLToInternalImg( String& rURL )
{
    if( rURL.Len() > 14UL &&
        rURL.CompareToAscii( sHTML_private_image, 14UL ) == COMPARE_EQUAL )
    {
        rURL.Erase( 0UL, 14UL );
        return TRUE;
    }

    return FALSE;
}


