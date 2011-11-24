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
#ifndef _STARMATHIMPORTUTILS_HXX
#define _STARMATHIMPORTUTILS_HXX

#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <oox/token/tokens.hxx>
#include <map>
#include <vector>

#include <oox/dllapi.h>

namespace oox
{

namespace formulaimport
{

const int TAG_OPENING = 1 << 29;
const int TAG_CLOSING = 1 << 30;

// used to differentiate between tags that open or close
// TODO
//inline int OPENING( int token ) { return TAG_OPENING | token; }
//inline int CLOSING( int token ) { return TAG_CLOSING | token; }
#define OPENING( token ) ( TAG_OPENING | token )
#define CLOSING( token ) ( TAG_CLOSING | token )

/**
 Class for storing a stream of xml tokens.

 A part of an XML file can be parsed and stored in this stream, from which it can be read
 as if parsed linearly. The purpose of this class is to allow simpler handling of XML
 files, unlike the usual LO way of using callbacks, context handlers and similar needlesly
 complicated stuff (YMMV).

 @since 3.5.0
*/
class OOX_DLLPUBLIC XmlStream
{
public:
    XmlStream();
    /**
     Structure representing a list of attributes.
    */
    // One could theoretically use oox::AttributeList, but that complains if the passed reference is empty,
    // which would be complicated to avoid here. Also, parsers apparently reuse the same instance of XFastAttributeList,
    // which means using oox::AttributeList would make them all point to the one instance.
    struct AttributeList
    {
        bool hasAttribute( int token ) const;
        rtl::OUString attribute( int token, const rtl::OUString& def = rtl::OUString()) const;
        bool attribute( int token, bool def ) const;
    protected:
        std::map< int, rtl::OUString > attrs;
    };
    /**
     Structure representing a tag, including its attributes and content text immediatelly following it.
    */
    struct Tag
    {
        Tag( int token = XML_TOKEN_INVALID,
            const com::sun::star::uno::Reference< com::sun::star::xml::sax::XFastAttributeList >& attributes = com::sun::star::uno::Reference< com::sun::star::xml::sax::XFastAttributeList >(),
            const rtl::OUString& text = rtl::OUString());
        int token; ///< tag type, or XML_TOKEN_INVALID
        AttributeList attributes;
        rtl::OUString text;
        /**
         Converts to true if the tag has a valid token, false otherwise. Allows simple
         usage in if(), for example 'if( XmlStream::Tag foo = stream.checkOpeningTag( footoken ))'.
        */
        operator bool() const;
    };
    /**
     @return true if current position is at the end of the XML stream
    */
    bool atEnd() const;
    /**
     @return data about the current tag
    */
    Tag currentTag() const;
    /**
     @return the token for the current tag
    */
    int currentToken() const;
    /**
     Moves position to the next tag.
    */
    void moveToNextTag();
    /**
     Ensures that an opening tag with the given token is read. If the current tag does not match,
     writes out a warning and tries to recover by skipping tags until found (or until the current element would end).
     If found, the position in the stream is afterwards moved to the next tag.
     @return the matching found opening tag, or empty tag if not found
    */
    Tag ensureOpeningTag( int token );
    /**
     Tries to find an opening tag with the given token. Works similarly like ensureOpeningTag(),
     but if a matching tag is not found, the position in the stream is not altered. The primary
     use of this function is to check for optional elements.
     @return the matching found opening tag, or empty tag if not found
    */
    Tag checkOpeningTag( int token );
    /**
     Ensures that a closing tag with the given token is read. Like ensureOpeningTag(),
     if not, writes out a warning and tries to recover by skiping tags until found (or until the current element would end).
     If found, the position in the stream is afterwards moved to the next tag.
    */
    void ensureClosingTag( int token );
    /**
     Tries to find the given token, until either found (returns true) or end of current element.
     Position in the stream is set to make the tag current.
    */
    bool recoverAndFindTag( int token );
    /**
     Skips the given element (i.e. reads up to and including the matching closing tag).
    */
    void skipElement( int token );
    /**
     Handle the current (unexpected) tag.
    */
    void handleUnexpectedTag();
protected:
    Tag checkTag( int token, bool optional, const char* txt );
    std::vector< Tag > tags;
    unsigned int pos;
};

/**
 This class is used for creating XmlStream.

 Simply use this class and then pass it as XmlStream to the consumer.

 @since 3.5.0
*/
class OOX_DLLPUBLIC XmlStreamBuilder
: public XmlStream
{
public:
    void appendOpeningTag( int token,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XFastAttributeList >& attributes );
    void appendClosingTag( int token );
    // appends the characters after the last appended token
    void appendCharacters( const rtl::OUString& characters );
};

} // namespace
} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
