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
 * Copyright (C) 2011 Tor Lillqvist <tlillqvist@suse.com> (initial developer)
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

#include "oox/mathml/importutils.hxx"

#include <assert.h>
#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>

using namespace com::sun::star;

namespace oox
{

namespace formulaimport
{

namespace
{
// a class that inherits from AttributeList, builds the internal data and then will be sliced off
// during conversion to the base class
class AttributeListBuilder
    : public XmlStream::AttributeList
{
public:
    AttributeListBuilder( const uno::Reference< xml::sax::XFastAttributeList >& a );
};

AttributeListBuilder::AttributeListBuilder( const uno::Reference< xml::sax::XFastAttributeList >& a )
{
    if( a.get() == NULL )
        return;
    uno::Sequence< xml::FastAttribute > aFastAttrSeq = a->getFastAttributes();
    const xml::FastAttribute* pFastAttr = aFastAttrSeq.getConstArray();
    sal_Int32 nFastAttrLength = aFastAttrSeq.getLength();
    for( int i = 0;
         i < nFastAttrLength;
         ++i )
    {
        attrs[ pFastAttr[ i ].Token ] = pFastAttr[ i ].Value;
    }
}
} // namespace

bool XmlStream::AttributeList::hasAttribute( int token ) const
{
    return attrs.find( token ) != attrs.end();
}

rtl::OUString XmlStream::AttributeList::attribute( int token, const rtl::OUString& def ) const
{
    std::map< int, rtl::OUString >::const_iterator find = attrs.find( token );
    if( find != attrs.end())
        return find->second;
    return def;
}

bool XmlStream::AttributeList::attribute( int token, bool def ) const
{
    std::map< int, rtl::OUString >::const_iterator find = attrs.find( token );
    if( find != attrs.end())
    {
        if( find->second.equalsIgnoreAsciiCaseAscii( "true" ) || find->second.equalsIgnoreAsciiCaseAscii( "on" )
            || find->second.equalsIgnoreAsciiCaseAscii( "t" ) || find->second.equalsIgnoreAsciiCaseAscii( "1" ))
            return true;
        if( find->second.equalsIgnoreAsciiCaseAscii( "false" ) || find->second.equalsIgnoreAsciiCaseAscii( "off" )
            || find->second.equalsIgnoreAsciiCaseAscii( "f" ) || find->second.equalsIgnoreAsciiCaseAscii( "0" ))
            return false;
        fprintf( stderr, "Cannot convert \'%s\' to bool.\n",
            rtl::OUStringToOString( find->second, RTL_TEXTENCODING_UTF8 ).getStr());
    }
    return def;
}

XmlStream::Tag::Tag( int t, const uno::Reference< xml::sax::XFastAttributeList >& a, const rtl::OUString& txt )
: token( t )
, attributes( AttributeListBuilder( a ))
, text( txt )
{
}


XmlStream::XmlStream::Tag::operator bool() const
{
    return token != XML_TOKEN_INVALID;
}

XmlStream::XmlStream()
: pos( 0 )
{
    // make sure our extra bit does not conflict with values used by oox
    assert( TAG_OPENING > ( 1024 << NMSP_SHIFT ));
}

bool XmlStream::atEnd() const
{
    return pos >= tags.size();
}

XmlStream::Tag XmlStream::currentTag() const
{
    if( pos >= tags.size())
        return Tag();
    return tags[ pos ];
}

int XmlStream::currentToken() const
{
    if( pos >= tags.size())
        return XML_TOKEN_INVALID;
    return tags[ pos ].token;
}

void XmlStream::moveToNextTag()
{
    if( pos < tags.size())
        ++pos;
}

XmlStream::Tag XmlStream::ensureOpeningTag( int token )
{
    return checkTag( OPENING( token ), false, "opening" );
}

XmlStream::Tag XmlStream::checkOpeningTag( int token )
{
    return checkTag( OPENING( token ), true, "opening" );
}

void XmlStream::ensureClosingTag( int token )
{
    checkTag( CLOSING( token ), false, "closing" );
}

XmlStream::Tag XmlStream::checkTag( int token, bool optional, const char* txt )
{
    // either it's the following tag, or find it
    int savedPos = pos;
    if( currentToken() == token || recoverAndFindTag( token ))
    {
        Tag ret = currentTag();
        moveToNextTag();
        return ret; // ok
    }
    if( optional )
    { // not a problem, just rewind
        pos = savedPos;
        return Tag();
    }
    fprintf( stderr, "Expected %s tag %d not found.\n", txt, token );
    return Tag();
}

bool XmlStream::recoverAndFindTag( int token )
{
    int depth = 0;
    for(;
         !atEnd();
         moveToNextTag())
    {
        if( depth > 0 ) // we're inside a nested element, skip those
        {
            if( currentToken() == OPENING( currentToken()))
            {
                fprintf( stderr, "Skipping opening tag %d\n", currentToken());
                ++depth;
            }
            else if( currentToken() == CLOSING( currentToken()))
            { // TODO debug output without the OPENING/CLOSING bits set
                fprintf( stderr, "Skipping closing tag %d\n", currentToken());
                --depth;
            }
            else
            {
                fprintf( stderr, "Malformed token %d\n", currentToken());
                abort();
            }
            continue;
        }
        if( currentToken() == token )
            return true; // ok, found
        if( currentToken() == CLOSING( currentToken()))
            return false; // that would be leaving current element, so not found
        if( currentToken() == OPENING( currentToken()))
        {
            fprintf( stderr, "Skipping opening tag %d\n", currentToken());
            ++depth;
        }
        else
            abort();
    }
    fprintf( stderr, "Unexpected end of stream reached.\n" );
    return false;
}

void XmlStream::skipElement( int token )
{
    int closing = ( token & ~TAG_OPENING ) | TAG_CLOSING; // make it a closing tag
    assert( currentToken() == OPENING( token ));
    // just find the matching closing tag
    if( recoverAndFindTag( closing ))
    {
        moveToNextTag(); // and skip it too
        return;
    }
    fprintf( stderr, "Expected end of element %d not found.\n", token );
}

void XmlStream::handleUnexpectedTag()
{
    if( atEnd())
        return;
    if( currentToken() == CLOSING( currentToken()))
    {
        moveToNextTag(); // just skip it
        return;
    }
    skipElement( currentToken()); // otherwise skip the entire element
}


void XmlStreamBuilder::appendOpeningTag( int token, const uno::Reference< xml::sax::XFastAttributeList >& attrs )
{
    tags.push_back( Tag( OPENING( token ), attrs ));
}

void XmlStreamBuilder::appendClosingTag( int token )
{
    tags.push_back( Tag( CLOSING( token )));
}

void XmlStreamBuilder::appendCharacters( const rtl::OUString& chars )
{
    assert( !tags.empty());
    tags.back().text = chars;
}

} // namespace
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
