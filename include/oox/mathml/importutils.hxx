/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_OOX_MATHML_IMPORTUTILS_HXX
#define INCLUDED_OOX_MATHML_IMPORTUTILS_HXX

#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <oox/token/tokens.hxx>
#include <map>
#include <vector>

#include <oox/dllapi.h>

namespace oox
{

namespace formulaimport
{

// used to differentiate between tags that opening or closing
const int TAG_OPENING = 1 << 29;
const int TAG_CLOSING = 1 << 30;

// you probably want to #define these to something shorter in the .cxx file,
// but they must be done as macros, otherwise they wouldn't be usable for case values,
// and macros cannot be namespaced
#define XML_STREAM_OPENING( token ) ( TAG_OPENING | token )
#define XML_STREAM_CLOSING( token ) ( TAG_CLOSING | token )

/**
 Class for storing a stream of xml tokens.

 A part of an XML file can be parsed and stored in this stream, from which it can be read
 as if parsed linearly. The purpose of this class is to allow simpler handling of XML
 files, unlike the usual LO way of using callbacks, context handlers and similar needlessly
 complicated stuff (YMMV).

 The advantages of this approach is easy to read and debug code (as it is just functions
 reading tokens one by one and calling other functions, compared to having to use callbacks
 and temporary storage). The disadvantage is that the XML structure needs to be handled
 manually by the code.

 Note that tag identifiers are simply int values and the API does not care besides matching
 their values to XML stream contents and requiring that the values are not as high as TAG_OPENING.
 Be prepared for the fact that some of the functions may throw exceptions if the input
 stream does not match the required token (TBD).

 The API tries to make the common idioms as simple as possible, see the following examples.

 Parse <tagone attr="value"><tagtwo>text</tagtwo></tagone> , where tagtwo is optional:
 @code
XmlStream::Tag tagoneTag = stream.ensureOpeningTag( tagone );
if( attributeTag.hasAttribute( attr ))
    ... = attributeTag.attribute( attr, defaultValueOfTheRightType );
if( XmlStream::Tag tagtwoTag = stream.checkOpeningTag( tagtwo ))
{
    ... = tagtwoTag.text;
    stream.ensureClosingTag( tagtwo );
}
stream.ensureClosingTag( tagone );
 @endcode

 Parse an element that may contain several sub-elements of different types in random order:
 @code
stream.ensureOpeningTag( element );
while( !stream.atEnd() && stream.currentToken() != CLOSING( element ))
    {
    switch( stream.currentToken())
    {
        case OPENING( subelement1 ):
            handleSubElement1();
            break;
        case OPENING( subelement2 ):
            ... process subelement2;
            break;
        default:
            stream.handleUnexpectedTag();
            break;
    }
stream.ensureClosingTag( element );
 @endcode

 If there may not be a zero number of sub-elements, use a helper bool variable or use a do-while loop.

 Parse an element that may contain an unknown number of sub-elements of the same type:
 @code
stream.ensureOpeningTag( element );
while( !stream.atEnd() && stream.findTag( OPENING( subelement )))
    {
    handleSubelement();
    }
stream.ensureClosingTag( element );
 @endcode

 If there may not be a zero number of sub-elements, use a helper bool variable or use a do-while loop.

 @since 3.5
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
    struct OOX_DLLPUBLIC AttributeList
    {
        OUString& operator[] (int token);
        OUString attribute( int token, const OUString& def = OUString()) const;
        bool attribute( int token, bool def ) const;
        sal_Unicode attribute( int token, sal_Unicode def ) const;
        // when adding more attribute() overloads, add also to XmlStream itself
    protected:
        std::map< int, OUString > attrs;
    };
    /**
     Structure representing a tag, including its attributes and content text immediately following it.
    */
    struct OOX_DLLPUBLIC Tag
    {
        Tag( int token = XML_TOKEN_INVALID,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& attributes = css::uno::Reference< css::xml::sax::XFastAttributeList >(),
            const OUString& text = OUString());
        Tag( int token,
            const AttributeList& attribs);
        int token; ///< tag type, or XML_TOKEN_INVALID
        AttributeList attributes;
        OUString text;
        /**
         This function returns value of the given attribute, or the passed default value if not found.
         The type of the default value selects the return type (OUString here).
        */
        OUString attribute( int token, const OUString& def = OUString()) const;
        /**
         @overload
        */
        bool attribute( int token, bool def ) const;
        /**
         @overload
        */
        sal_Unicode attribute( int token, sal_Unicode def ) const;
        // when adding more attribute() overloads, add also to XmlStream::AttributeList and inline below
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
     Position in the stream is set to make the tag current (i.e. it will be the next one read).
    */
    bool findTag( int token );
    /**
     Handle the current (unexpected) tag.
    */
    void handleUnexpectedTag();
protected:
    Tag checkTag( int token, bool optional );
    bool findTagInternal( int token, bool silent );
    void skipElementInternal( int token, bool silent );
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
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& attributes = css::uno::Reference< css::xml::sax::XFastAttributeList >());
    void appendOpeningTag( int token,
        const AttributeList& attribs );
    void appendClosingTag( int token );
    // appends the characters after the last appended token
    void appendCharacters( const OUString& characters );
};

inline
OUString XmlStream::Tag::attribute( int t, const OUString& def ) const
{
    return attributes.attribute( t, def );
}

inline
bool XmlStream::Tag::attribute( int t, bool def ) const
{
    return attributes.attribute( t, def );
}

inline
sal_Unicode XmlStream::Tag::attribute( int t, sal_Unicode def ) const
{
    return attributes.attribute( t, def );
}

} // namespace
} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
