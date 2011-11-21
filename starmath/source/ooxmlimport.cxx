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
    checkOpeningTag( M_TOKEN( oMath ));
    OUString ret;
    while( !stream.atEnd())
    {
        XmlStream::Tag tag = stream.currentTag();
        if( tag.token == CLOSING( M_TOKEN( oMath )))
            break;
        switch( tag.token )
        {
            case OPENING( M_TOKEN( f )):
                ret += STR( " " ) + handleF();
                break;
            default:
                handleUnexpectedTag();
                break;
        }
    }
    checkClosingTag( M_TOKEN( oMath ));
    return ret;
}

// NOT complete
OUString SmOoxmlImport::handleF()
{
    checkOpeningTag( M_TOKEN( f ));
    if( stream.currentToken() == OPENING_TAG( M_TOKEN( fPr )))
    {
        // TODO
    }
    checkOpeningTag( M_TOKEN( num ));
    OUString num = readR();
    checkClosingTag( M_TOKEN( num ));
    checkOpeningTag( M_TOKEN( den ));
    OUString den = readR();
    checkClosingTag( M_TOKEN( den ));
    checkClosingTag( M_TOKEN( f ));
    return STR( "{" ) + num + STR( "} over {" ) + den + STR( "}" );
}

// NOT complete
OUString SmOoxmlImport::readR()
{
    checkOpeningTag( M_TOKEN( r ));

//    checkOpeningTag( OOX_TOKEN( doc, rPr ));
//    checkOpeningTag( OOX_TOKEN( doc, rFonts ));
//    checkClosingTag( OOX_TOKEN( doc, rFonts ));
//    checkClosingTag( OOX_TOKEN( doc, rPr ));

    // TODO can there be more t's ?
    XmlStream::Tag rtag = checkOpeningTag( M_TOKEN( t ));
    OUString text = rtag.text;
    if( !rtag.attributes.getBool( OOX_TOKEN( xml, space ), false ))
        text = text.trim();
    checkClosingTag( M_TOKEN( t ));
    checkClosingTag( M_TOKEN( r ));
    return text;
}

XmlStream::Tag SmOoxmlImport::checkOpeningTag( int token )
{
    return checkTag( OPENING( token ), "opening" );
}

void SmOoxmlImport::checkClosingTag( int token )
{
    checkTag( CLOSING( token ), "closing" );
}

XmlStream::Tag SmOoxmlImport::checkTag( int token, const char* txt )
{
    // either it's the following tag, or find it
    if( stream.currentToken() == token || recoverAndFindTag( token ))
    {
        XmlStream::Tag ret = stream.currentTag();
        stream.moveToNextTag();
        return ret; // ok
    }
    fprintf( stderr, "Expected %s tag %d not found.\n", txt, token );
    return XmlStream::Tag();
}

bool SmOoxmlImport::recoverAndFindTag( int token )
{
    int depth = 0;
    for(;
         !stream.atEnd();
         stream.moveToNextTag())
    {
        if( depth > 0 ) // we're inside a nested element, skip those
        {
            if( stream.currentToken() == OPENING( stream.currentToken()))
            {
                fprintf( stderr, "Skipping opening tag %d\n", stream.currentToken());
                ++depth;
            }
            else if( stream.currentToken() == CLOSING( stream.currentToken()))
            { // TODO debug output without the OPENING/CLOSING bits set
                fprintf( stderr, "Skipping closing tag %d\n", stream.currentToken());
                --depth;
            }
            else
            {
                fprintf( stderr, "Malformed token %d\n", stream.currentToken());
                abort();
            }
            continue;
        }
        if( stream.currentToken() == CLOSING( stream.currentToken()))
            return false; // that would be leaving current element, so not found
        if( stream.currentToken() == token )
            return true; // ok, found
        if( stream.currentToken() == OPENING( stream.currentToken()))
        {
            fprintf( stderr, "Skipping opening tag %d\n", stream.currentToken());
            ++depth;
        }
        else
            abort();
    }
    fprintf( stderr, "Unexpected end of stream reached.\n" );
    return false;
}

void SmOoxmlImport::skipElement( int token )
{
    int closing = ( token & ~TAG_OPENING ) | TAG_CLOSING; // make it a closing tag
    assert( stream.currentToken() == OPENING( token ));
    // just find the matching closing tag
    if( recoverAndFindTag( closing ))
    {
        stream.moveToNextTag(); // and skip it too
        return;
    }
    fprintf( stderr, "Expected end of element %d not found.\n", token );
}

void SmOoxmlImport::handleUnexpectedTag()
{
    if( stream.atEnd())
        return;
    if( stream.currentToken() == CLOSING( stream.currentToken()))
    {
        stream.moveToNextTag(); // just skip it
        return;
    }
    skipElement( stream.currentToken()); // otherwise skip the entire element
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
