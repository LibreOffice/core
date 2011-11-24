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

// NOT complete
OUString SmOoxmlImport::handleF()
{
    stream.ensureOpeningTag( M_TOKEN( f ));
    if( stream.currentToken() == OPENING_TAG( M_TOKEN( fPr )))
    {
        // TODO
    }
    stream.ensureOpeningTag( M_TOKEN( num ));
    OUString num = readR();
    stream.ensureClosingTag( M_TOKEN( num ));
    stream.ensureOpeningTag( M_TOKEN( den ));
    OUString den = readR();
    stream.ensureClosingTag( M_TOKEN( den ));
    stream.ensureClosingTag( M_TOKEN( f ));
    return STR( "{" ) + num + STR( "} over {" ) + den + STR( "}" );
}

// NOT complete
OUString SmOoxmlImport::readR()
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
    if( !rtag.attributes.getBool( OOX_TOKEN( xml, space ), false ))
        text = text.trim();
    stream.ensureClosingTag( M_TOKEN( t ));
    stream.ensureClosingTag( M_TOKEN( r ));
    return text;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
