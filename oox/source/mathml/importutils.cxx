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
