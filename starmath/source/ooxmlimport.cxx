/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Lubos Lunak <l.lunak@suse.cz> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */


#include "ooxmlimport.hxx"

#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>

using namespace oox;
using namespace oox::formulaimport;
using rtl::OUString;
using rtl::OUStringBuffer;

/*
The primary internal data structure for the formula is the text representation
(the SmNode tree is built from it), so read data must be converted into this format.
*/

#define M_TOKEN( token ) OOX_TOKEN( officeMath, token )
#define OPENING_TAG( token ) OPENING( token )
#define CLOSING_TAG( token ) CLOSING( token )

// *sigh*
#define STR( str ) OUString( RTL_CONSTASCII_USTRINGPARAM( str ))

// TODO create IS_OPENING(), IS_CLOSING() instead of doing 'next == OPENING( next )' ?

SmOoxmlImport::SmOoxmlImport( oox::formulaimport::XmlStream& s )
: stream( s )
{
}

OUString SmOoxmlImport::ConvertToStarMath()
{
    return handleStream();
}

// "toplevel" of reading, there will be oMath (if there was oMathPara, that was
// up to the parent component to handle)

// NOT complete
OUString SmOoxmlImport::handleStream()
{
    stream.ensureOpeningTag( M_TOKEN( oMath ));
    OUString ret;
    while( !stream.atEnd() && stream.currentToken() != CLOSING( M_TOKEN( oMath )))
    {
        // strictly speaking, it is not OMathArg here, but currently supported
        // functionality is the same like OMathArg, in the future this may need improving
        OUString item = readOMathArg();
        if( item.isEmpty())
            continue;
        if( !ret.isEmpty())
            ret += STR( " " );
        ret += item;
    }
    stream.ensureClosingTag( M_TOKEN( oMath ));
    fprintf(stderr, "FORMULA: %s\n", rtl::OUStringToOString( ret, RTL_TEXTENCODING_UTF8 ).getStr());
    return ret;
}


OUString SmOoxmlImport::readOMathArg()
{
    while( stream.currentToken() != CLOSING( stream.currentToken()))
    {
        switch( stream.currentToken())
        {
            case OPENING( M_TOKEN( acc )):
                return handleAcc();
            case OPENING( M_TOKEN( bar )):
                return handleBar();
            case OPENING( M_TOKEN( borderBox )):
                return handleBorderBox();
            case OPENING( M_TOKEN( d )):
                return handleD();
            case OPENING( M_TOKEN( f )):
                return handleF();
            case OPENING( M_TOKEN( func )):
                return handleFunc();
            case OPENING( M_TOKEN( limLow )):
                return handleLimLowUpp( LimLow );
            case OPENING( M_TOKEN( limUpp )):
                return handleLimLowUpp( LimUpp );
            case OPENING( M_TOKEN( groupChr )):
                return handleGroupChr();
            case OPENING( M_TOKEN( r )):
                return handleR();
            default:
                stream.handleUnexpectedTag();
                break;
        }
    }
    return OUString();
}

OUString SmOoxmlImport::handleAcc()
{
    stream.ensureOpeningTag( M_TOKEN( acc ));
    OUString acc;
    if( XmlStream::Tag accPr = stream.checkOpeningTag( M_TOKEN( accPr )))
    {
        if( XmlStream::Tag chr = stream.checkOpeningTag( M_TOKEN( chr )))
        {
            acc = chr.attribute( M_TOKEN( val ));
            stream.ensureClosingTag( M_TOKEN( chr ));
        }
        stream.ensureClosingTag( M_TOKEN( accPr ));
    }
    // see aTokenTable in parse.cxx
    switch( acc.isEmpty() ? sal_Unicode( MS_ACUTE ) : acc[ 0 ] )
    {
        case MS_CHECK:
            acc = STR( "check" );
            break;
        case MS_ACUTE:
            acc = STR( "acute" );
            break;
        case MS_GRAVE:
            acc = STR( "grave" );
            break;
        case MS_BREVE:
            acc = STR( "breve" );
            break;
        case MS_CIRCLE:
            acc = STR( "circle" );
            break;
        case MS_VEC:
            // prefer wide variants for these 3, .docx can't seem to differentiate
            // between e.g. 'vec' and 'widevec', if whatever the accent is above is short, this
            // shouldn't matter, but short above a longer expression doesn't look right
            acc = STR( "widevec" );
            break;
        case MS_TILDE:
            acc = STR( "widetilde" );
            break;
        case MS_HAT:
            acc = STR( "widehat" );
            break;
        case MS_DOT:
            acc = STR( "dot" );
            break;
        case MS_DDOT:
            acc = STR( "ddot" );
            break;
        case MS_DDDOT:
            acc = STR( "dddot" );
            break;
        default:
            acc = STR( "acute" );
            break;
    }
    OUString e = handleE();
    stream.ensureClosingTag( M_TOKEN( acc ));
    return acc + STR( " { " ) + e + STR( " }" );
}

OUString SmOoxmlImport::handleBar()
{
    stream.ensureOpeningTag( M_TOKEN( bar ));
    enum pos_t { top, bot } topbot = bot;
    if( stream.checkOpeningTag( M_TOKEN( barPr )))
    {
        if( XmlStream::Tag pos = stream.checkOpeningTag( M_TOKEN( pos )))
        {
            if( pos.attribute( M_TOKEN( val )) == STR( "top" ))
                topbot = top;
            else if( pos.attribute( M_TOKEN( val )) == STR( "bot" ))
                topbot = bot;
            stream.ensureClosingTag( M_TOKEN( pos ));
        }
        stream.ensureClosingTag( M_TOKEN( barPr ));
    }
    OUString e = handleE();
    stream.ensureClosingTag( M_TOKEN( bar ));
    if( topbot == top )
        return STR( "bar { " ) + e + STR( " }" );
    else
        return STR( "underline { " ) + e + STR( " }" );
}

OUString SmOoxmlImport::handleBorderBox()
{
    stream.ensureOpeningTag( M_TOKEN( borderBox ));
    bool isStrikeH = false;
    if( stream.checkOpeningTag( M_TOKEN( borderBoxPr )))
    {
        if( XmlStream::Tag strikeH = stream.checkOpeningTag( M_TOKEN( strikeH )))
        {
            if( strikeH.attribute( M_TOKEN( val ), false ))
                isStrikeH = true;
            stream.ensureClosingTag( M_TOKEN( strikeH ));
        }
        stream.ensureClosingTag( M_TOKEN( borderBoxPr ));
    }
    OUString e = handleE();
    stream.ensureClosingTag( M_TOKEN( borderBox ));
    if( isStrikeH )
        return STR( "overstrike { " ) + e + STR( " }" );
    // LO does not seem to implement anything for handling the other cases
    return e;
}

OUString SmOoxmlImport::handleD()
{
    stream.ensureOpeningTag( M_TOKEN( d ));
    sal_Unicode opening = '(';
    sal_Unicode closing = ')';
    sal_Unicode separator = '|';
    if( XmlStream::Tag dPr = stream.checkOpeningTag( M_TOKEN( dPr )))
    {
        if( XmlStream::Tag begChr = stream.checkOpeningTag( M_TOKEN( begChr )))
        {
            opening = begChr.attribute( M_TOKEN( val ), opening );
            stream.ensureClosingTag( M_TOKEN( begChr ));
        }
        if( XmlStream::Tag sepChr = stream.checkOpeningTag( M_TOKEN( sepChr )))
        {
            separator = sepChr.attribute( M_TOKEN( val ), separator );
            stream.ensureClosingTag( M_TOKEN( sepChr ));
        }
        if( XmlStream::Tag endChr = stream.checkOpeningTag( M_TOKEN( endChr )))
        {
            closing = endChr.attribute( M_TOKEN( val ), closing );
            stream.ensureClosingTag( M_TOKEN( endChr ));
        }
        stream.ensureClosingTag( M_TOKEN( dPr ));
    }
    OUStringBuffer ret;
    ret.append( opening );
    bool first = true;
    while( stream.currentToken() == OPENING( M_TOKEN( e )))
    {
        if( !first )
        { // plain "|" would be actually "V" (logical or)
            ret.append( separator == '|' ? STR( " mline " ) : STR( "|" ));
        }
        first = false;
        ret.append( handleE());
    }
    ret.append( closing );
    stream.ensureClosingTag( M_TOKEN( d ));
    return ret.makeStringAndClear();
}

OUString SmOoxmlImport::handleE()
{
    stream.ensureOpeningTag( M_TOKEN( e ));
    OUString ret = readOMathArg();
    stream.ensureClosingTag( M_TOKEN( e ));
    return ret;
}

OUString SmOoxmlImport::handleF()
{
    stream.ensureOpeningTag( M_TOKEN( f ));
    enum operation_t { bar, lin, noBar } operation = bar;
    OUString oper = STR( "over" );
    if( stream.checkOpeningTag( M_TOKEN( fPr )))
    {
        if( XmlStream::Tag type = stream.checkOpeningTag( M_TOKEN( type )))
        {
            if( type.attribute( M_TOKEN( val )) == STR( "bar" ))
                operation = bar;
            else if( type.attribute( M_TOKEN( val )) == STR( "lin" ))
                operation = lin;
            else if( type.attribute( M_TOKEN( val )) == STR( "noBar" ))
                operation = noBar;
            stream.ensureClosingTag( M_TOKEN( type ));
        }
        stream.ensureClosingTag( M_TOKEN( fPr ));
    }
    stream.ensureOpeningTag( M_TOKEN( num ));
    OUString num = readOMathArg();
    stream.ensureClosingTag( M_TOKEN( num ));
    stream.ensureOpeningTag( M_TOKEN( den ));
    OUString den = readOMathArg();
    stream.ensureClosingTag( M_TOKEN( den ));
    stream.ensureClosingTag( M_TOKEN( f ));
    if( operation == bar )
        return STR( "{" ) + num + STR( "} over {" ) + den + STR( "}" );
    else if( operation == lin )
        return STR( "{" ) + num + STR( "} / {" ) + den + STR( "}" );
    else // noBar
    { // TODO we write out stack of 3 items as recursive m:f, so merge here back
      // to 'stack { x # y # z }'
        return STR( "binom { " ) + num + STR( " } { " ) + den + STR( " }" );
    }
}

OUString SmOoxmlImport::handleFunc()
{
//lim from{x rightarrow 1} x
    stream.ensureOpeningTag( M_TOKEN( func ));
    stream.ensureOpeningTag( M_TOKEN( fName ));
    OUString fname = readOMathArg();
    stream.ensureClosingTag( M_TOKEN( fName ));
    // fix the various functions
    if( fname.match( STR( "lim {" ), 0 )) // startsWith()
        fname = STR( "lim from {" ) + fname.copy( 5 );
    OUString ret = fname + STR( " {" ) + handleE() + STR( "}" );
    stream.ensureClosingTag( M_TOKEN( func ));
    return ret;
}

OUString SmOoxmlImport::handleLimLowUpp( LimLowUpp_t limlowupp )
{
    int token = limlowupp == LimLow ? M_TOKEN( limLow ) : M_TOKEN( limUpp );
    stream.ensureOpeningTag( token );
    OUString e = handleE();
    stream.ensureOpeningTag( M_TOKEN( lim ));
    OUString lim = readOMathArg();
    stream.ensureClosingTag( M_TOKEN( lim ));
    stream.ensureClosingTag( token );
    return e + STR( " {" ) + lim + STR( "}" );
}

OUString SmOoxmlImport::handleGroupChr()
{
    stream.ensureOpeningTag( M_TOKEN( groupChr ));
    enum pos_t { top, bot } pos = bot;
    if( stream.checkOpeningTag( M_TOKEN( groupChrPr )))
    {
        if( XmlStream::Tag posTag = stream.checkOpeningTag( M_TOKEN( pos )))
        {
            if( posTag.attribute( M_TOKEN( val ), STR( "bot" )) == STR( "top" ))
                pos = top;
            stream.ensureClosingTag( M_TOKEN( pos ));
        }
        stream.ensureClosingTag( M_TOKEN( groupChrPr ));
    }
    OUString ret = STR( "{ " ) + handleE() + ( pos == top ? STR( "} overbrace" ) : STR( "} underbrace" ));
    stream.ensureClosingTag( M_TOKEN( groupChr ));
    return ret;
}

// NOT complete
OUString SmOoxmlImport::handleR()
{
    stream.ensureOpeningTag( M_TOKEN( r ));
    if( XmlStream::Tag rPr = stream.checkOpeningTag( OOX_TOKEN( doc, rPr )))
    { // TODO
//        stream.checkOpeningTag( OOX_TOKEN( doc, rFonts ));
//        stream.ensureClosingTag( OOX_TOKEN( doc, rFonts ));
        stream.ensureClosingTag( OOX_TOKEN( doc, rPr ));
    }
    // TODO can there be more t's ?
    XmlStream::Tag rtag = stream.ensureOpeningTag( M_TOKEN( t ));
    // TODO bail out if failure?
    OUString text = rtag.text;
    if( rtag.attribute( OOX_TOKEN( xml, space )) != STR( "preserve" ))
        text = text.trim();
    stream.ensureClosingTag( M_TOKEN( t ));
    stream.ensureClosingTag( M_TOKEN( r ));
    return text;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
