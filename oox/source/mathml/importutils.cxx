/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/mathml/importutils.hxx>

#include <assert.h>

#include <com/sun/star/xml/FastAttribute.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <oox/token/tokenmap.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#define OPENING( token ) XML_STREAM_OPENING( token )
#define CLOSING( token ) XML_STREAM_CLOSING( token )

using namespace com::sun::star;

namespace oox::formulaimport
{

namespace
{
// a class that inherits from AttributeList, builds the internal data and then will be sliced off
// during conversion to the base class
class AttributeListBuilder
    : public XmlStream::AttributeList
{
public:
    explicit AttributeListBuilder( const uno::Reference< xml::sax::XFastAttributeList >& a );
};

AttributeListBuilder::AttributeListBuilder( const uno::Reference< xml::sax::XFastAttributeList >& a )
{
    if( !a )
        return;
    const uno::Sequence< xml::FastAttribute > aFastAttrSeq = a->getFastAttributes();
    for( const xml::FastAttribute& rFastAttr : aFastAttrSeq )
    {
        attrs[ rFastAttr.Token ] = rFastAttr.Value;
    }
}

OString tokenToString( int token )
{
    uno::Sequence< sal_Int8 > const & aTokenNameSeq = StaticTokenMap::get().getUtf8TokenName( token & TOKEN_MASK );
    OString tokenname( reinterpret_cast< const char* >( aTokenNameSeq.getConstArray() ), aTokenNameSeq.getLength() );
    if( tokenname.isEmpty())
        tokenname = "???";
    int nmsp = ( token & NMSP_MASK & ~( TAG_OPENING | TAG_CLOSING ));
#if 0 // this is awfully long
    OString namespacename = StaticNamespaceMap::get().count( nmsp ) != 0
        ? StaticNamespaceMap::get()[ nmsp ] : OString( "???" );
#else
    OString namespacename;
    // only few are needed actually
    switch( nmsp )
    {
        case NMSP_officeMath:
            namespacename = "m";
            break;
        case NMSP_doc:
            namespacename = "w";
            break;
        default:
            namespacename = "?";
            break;
    }
#endif
    if( token == OPENING( token ))
        return "<" + namespacename + ":" + tokenname + ">";
    if( token == CLOSING( token ))
        return "</" + namespacename + ":" + tokenname + ">";
    // just the name itself, not specified whether opening or closing
    return namespacename + ":" + tokenname;
}

} // namespace

OUString& XmlStream::AttributeList::operator[] (int token)
{
    return attrs[token];
}

OUString XmlStream::AttributeList::attribute( int token, const OUString& def ) const
{
    std::map< int, OUString >::const_iterator find = attrs.find( token );
    if( find != attrs.end())
        return find->second;
    return def;
}

bool XmlStream::AttributeList::attribute( int token, bool def ) const
{
    std::map< int, OUString >::const_iterator find = attrs.find( token );
    if( find != attrs.end())
    {
        const OUString sValue = find->second;
        if( sValue.equalsIgnoreAsciiCase("true") ||
            sValue.equalsIgnoreAsciiCase("on") ||
            sValue.equalsIgnoreAsciiCase("t") ||
            sValue.equalsIgnoreAsciiCase("1") )
            return true;
        if( sValue.equalsIgnoreAsciiCase("false") ||
            sValue.equalsIgnoreAsciiCase("off") ||
            sValue.equalsIgnoreAsciiCase("f") ||
            sValue.equalsIgnoreAsciiCase("0") )
            return false;
        SAL_WARN( "oox.xmlstream", "Cannot convert \'" << sValue << "\' to bool." );
    }
    return def;
}

sal_Unicode XmlStream::AttributeList::attribute( int token, sal_Unicode def ) const
{
    std::map< int, OUString >::const_iterator find = attrs.find( token );
    if( find != attrs.end())
    {
        if( !find->second.isEmpty() )
        {
            if( find->second.getLength() != 1 )
                SAL_WARN( "oox.xmlstream", "Cannot convert \'" << find->second << "\' to sal_Unicode, stripping." );
            return find->second[ 0 ];
        }
    }
    return def;
}

XmlStream::Tag::Tag( int t, const uno::Reference< xml::sax::XFastAttributeList >& a )
: token( t )
, attributes( AttributeListBuilder( a ))
{
}

XmlStream::Tag::Tag( int t, const AttributeList& a )
: token( t )
, attributes( a )
{
}

XmlStream::Tag::operator bool() const
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
    return checkTag( OPENING( token ), false );
}

XmlStream::Tag XmlStream::checkOpeningTag( int token )
{
    return checkTag( OPENING( token ), true );
}

void XmlStream::ensureClosingTag( int token )
{
    checkTag( CLOSING( token ), false );
}

XmlStream::Tag XmlStream::checkTag( int token, bool optional )
{
    // either it's the following tag, or find it
    int savedPos = pos;
    if( optional )
    { // avoid printing debug messages about skipping tags if the optional one
      // will not be found and the position will be reset back
        if( currentToken() != token && !findTagInternal( token, true ))
        {
            pos = savedPos;
            return Tag();
        }
    }
    if( currentToken() == token || findTag( token ))
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
    SAL_WARN( "oox.xmlstream", "Expected tag " << tokenToString( token ) << " not found." );
    return Tag();
}

bool XmlStream::findTag( int token )
{
    return findTagInternal( token, false );
}

bool XmlStream::findTagInternal( int token, bool silent )
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
                if( !silent )
                    SAL_INFO( "oox.xmlstream", "Skipping tag " << tokenToString( currentToken()));
                ++depth;
            }
            else if( currentToken() == CLOSING( currentToken()))
            {
                if( !silent )
                    SAL_INFO( "oox.xmlstream", "Skipping tag " << tokenToString( currentToken()));
                --depth;
            }
            else
            {
                if( !silent )
                    SAL_WARN( "oox.xmlstream", "Malformed token " << currentToken() << " ("
                        << tokenToString( currentToken()) << ")" );
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
            if( !silent )
                SAL_INFO( "oox.xmlstream", "Skipping tag " << tokenToString( currentToken()));
            ++depth;
        }
        else
            abort();
    }
    if( !silent )
        SAL_WARN( "oox.xmlstream", "Unexpected end of stream reached." );
    return false;
}

void XmlStream::skipElementInternal( int token, bool silent )
{
    int closing = ( token & ~TAG_OPENING ) | TAG_CLOSING; // make it a closing tag
    assert( currentToken() == OPENING( token ));
    if( !silent )
        SAL_INFO( "oox.xmlstream", "Skipping unexpected element " << tokenToString( currentToken()));
    moveToNextTag();
    // and just find the matching closing tag
    if( findTag( closing ))
    {
        if( !silent )
            SAL_INFO( "oox.xmlstream", "Skipped unexpected element " << tokenToString( token ));
        moveToNextTag(); // and skip it too
        return;
    }
    // this one is an unexpected problem, do not silent it
    SAL_WARN( "oox.xmlstream", "Expected end of element " << tokenToString( token ) << " not found." );
}

void XmlStream::handleUnexpectedTag()
{
    if( atEnd())
        return;
    if( currentToken() == CLOSING( currentToken()))
    {
        SAL_INFO( "oox.xmlstream", "Skipping unexpected tag " << tokenToString( currentToken()));
        moveToNextTag(); // just skip it
        return;
    }
    skipElementInternal( currentToken(), false ); // otherwise skip the entire element
}

void XmlStreamBuilder::appendOpeningTag( int token, const uno::Reference< xml::sax::XFastAttributeList >& attrs )
{
    tags.emplace_back( OPENING( token ), attrs );
}

void XmlStreamBuilder::appendOpeningTag( int token, const AttributeList& attrs )
{
    tags.emplace_back( OPENING( token ), attrs );
}

void XmlStreamBuilder::appendClosingTag( int token )
{
    tags.emplace_back( CLOSING( token ));
}

void XmlStreamBuilder::appendCharacters( std::u16string_view chars )
{
    assert( !tags.empty());
    tags.back().text += chars;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
