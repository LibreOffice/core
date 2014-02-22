/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "oox/mathml/importutils.hxx"

#include <assert.h>
#include <stdio.h>

#include <oox/token/namespacemap.hxx>
#include <oox/token/tokenmap.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>
#include <rtl/ustring.hxx>

#define OPENING( token ) XML_STREAM_OPENING( token )
#define CLOSING( token ) XML_STREAM_CLOSING( token )

using namespace com::sun::star;

namespace oox
{

namespace formulaimport
{

namespace
{


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

static OUString tokenToString( int token )
{
    OUString tokenname = StaticTokenMap::get().getUnicodeTokenName( token & TOKEN_MASK );
    if( tokenname.isEmpty())
        tokenname = "???";
    int nmsp = ( token & NMSP_MASK & ~( TAG_OPENING | TAG_CLOSING ));
#if 0 
    OUString namespacename = StaticNamespaceMap::get().count( nmsp ) != 0
        ? StaticNamespaceMap::get()[ nmsp ] : OUString( "???" );
#else
    OUString namespacename;
    
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
    
    return namespacename + ":" + tokenname;
}

} 

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

XmlStream::Tag::Tag( int t, const uno::Reference< xml::sax::XFastAttributeList >& a, const OUString& txt )
: token( t )
, attributes( AttributeListBuilder( a ))
, text( txt )
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
    
    int savedPos = pos;
    if( optional )
    { 
      
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
        return ret; 
    }
    if( optional )
    { 
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
        if( depth > 0 ) 
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
            return true; 
        if( currentToken() == CLOSING( currentToken()))
            return false; 
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
    int closing = ( token & ~TAG_OPENING ) | TAG_CLOSING; 
    assert( currentToken() == OPENING( token ));
    if( !silent )
        SAL_INFO( "oox.xmlstream", "Skipping unexpected element " << tokenToString( currentToken()));
    moveToNextTag();
    
    if( findTag( closing ))
    {
        if( !silent )
            SAL_INFO( "oox.xmlstream", "Skipped unexpected element " << tokenToString( token ));
        moveToNextTag(); 
        return;
    }
    
    SAL_WARN( "oox.xmlstream", "Expected end of element " << tokenToString( token ) << " not found." );
}

void XmlStream::handleUnexpectedTag()
{
    if( atEnd())
        return;
    if( currentToken() == CLOSING( currentToken()))
    {
        SAL_INFO( "oox.xmlstream", "Skipping unexpected tag " << tokenToString( currentToken()));
        moveToNextTag(); 
        return;
    }
    skipElementInternal( currentToken(), false ); 
}


void XmlStreamBuilder::appendOpeningTag( int token, const uno::Reference< xml::sax::XFastAttributeList >& attrs )
{
    tags.push_back( Tag( OPENING( token ), attrs ));
}

void XmlStreamBuilder::appendOpeningTag( int token, const AttributeList& attrs )
{
    tags.push_back( Tag( OPENING( token ), attrs ));
}

void XmlStreamBuilder::appendClosingTag( int token )
{
    tags.push_back( Tag( CLOSING( token )));
}

void XmlStreamBuilder::appendCharacters( const OUString& chars )
{
    assert( !tags.empty());
    tags.back().text += chars;
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
