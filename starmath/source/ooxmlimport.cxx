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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_starmath.hxx"

#include "ooxmlimport.hxx"

#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>

using namespace oox;
using namespace oox::formulaimport;
using rtl::OUString;

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
    while( !stream.atEnd())
    {
        XmlStream::Tag tag = stream.currentTag();
        if( tag.token == CLOSING( M_TOKEN( oMath )))
            break;
        switch( tag.token )
        {
            case OPENING( M_TOKEN( acc )):
                ret += STR( " " ) + handleAcc();
                break;
            case OPENING( M_TOKEN( bar )):
                ret += STR( " " ) + handleBar();
                break;
            case OPENING( M_TOKEN( borderBox )):
                ret += STR( " " ) + handleBorderBox();
                break;
            case OPENING( M_TOKEN( f )):
                ret += STR( " " ) + handleF();
                break;
            default:
                stream.handleUnexpectedTag();
                break;
        }
    }
    stream.ensureClosingTag( M_TOKEN( oMath ));
    return ret;
}

OUString SmOoxmlImport::handleAcc()
{
    stream.ensureOpeningTag( M_TOKEN( acc ));
    OUString acc;
    if( XmlStream::Tag accPr = stream.checkOpeningTag( M_TOKEN( accPr )))
    {
        if( XmlStream::Tag chr = stream.checkOpeningTag( M_TOKEN( chr )))
        {
            acc = chr.attributes.attribute( M_TOKEN( val ));
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
            if( pos.attributes.attribute( M_TOKEN( val )) == STR( "top" ))
                topbot = top;
            else if( pos.attributes.attribute( M_TOKEN( val )) == STR( "bot" ))
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
            if( strikeH.attributes.attribute( M_TOKEN( val ), false ))
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

OUString SmOoxmlImport::handleE()
{
    stream.ensureOpeningTag( M_TOKEN( e ));
    OUString ret = readOMathArg( M_TOKEN( e ));
    stream.ensureClosingTag( M_TOKEN( e ));
    return ret;
}

OUString SmOoxmlImport::readOMathArg( int endtoken )
{
    OUString ret;
    while( !stream.atEnd())
    { // TODO can there really be more or just one sub-elements?
        XmlStream::Tag tag = stream.currentTag();
        if( tag.token == CLOSING( endtoken ))
            break;
        switch( tag.token )
        {
            case OPENING( M_TOKEN( acc )):
                ret += STR( " " ) + handleAcc();
                break;
            case OPENING( M_TOKEN( f )):
                ret += STR( " " ) + handleF();
                break;
            case OPENING( M_TOKEN( r )):
                ret += STR( " " ) + handleR();
                break;
            default:
                stream.handleUnexpectedTag();
                break;
        }
    }
    return ret;
}

// NOT complete
OUString SmOoxmlImport::handleF()
{
    stream.ensureOpeningTag( M_TOKEN( f ));
    enum operation_t { bar, lin, noBar } operation = bar;
    OUString oper = STR( "over" );
    if( stream.checkOpeningTag( M_TOKEN( fPr )))
    {
        if( XmlStream::Tag type = stream.checkOpeningTag( M_TOKEN( type )))
        {
            if( type.attributes.attribute( M_TOKEN( val )) == STR( "bar" ))
                operation = bar;
            else if( type.attributes.attribute( M_TOKEN( val )) == STR( "lin" ))
                operation = lin;
            else if( type.attributes.attribute( M_TOKEN( val )) == STR( "noBar" ))
                operation = noBar;
            stream.ensureClosingTag( M_TOKEN( type ));
        }
        stream.ensureClosingTag( M_TOKEN( fPr ));
    }
    stream.ensureOpeningTag( M_TOKEN( num ));
    OUString num = readOMathArg( M_TOKEN( num ));
    stream.ensureClosingTag( M_TOKEN( num ));
    stream.ensureOpeningTag( M_TOKEN( den ));
    OUString den = readOMathArg( M_TOKEN( den ));
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
    if( rtag.attributes.attribute( OOX_TOKEN( xml, space )) != STR( "preserve" ))
        text = text.trim();
    stream.ensureClosingTag( M_TOKEN( t ));
    stream.ensureClosingTag( M_TOKEN( r ));
    return text;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
