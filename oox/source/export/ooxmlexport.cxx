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

#include <oox/export/ooxmlexport.hxx>
#include <oox/export/starmathimport.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>

using namespace oox;
using namespace oox::core;
using namespace com::sun::star;

OoxmlFormulaExportBase::OoxmlFormulaExportBase()
{
}

OoxmlFormulaImportBase::OoxmlFormulaImportBase()
{
}

OoxmlFormulaImportHelper::OoxmlFormulaImportHelper()
{
}


namespace ooxmlformulaimport
{

XmlStream::XmlStream()
: pos( -1 )
{
    // make sure our extra bit does not conflict with values used by oox
    assert( TAG_OPENING > ( 1024 << NMSP_SHIFT ));
}

bool XmlStream::nextIsEnd() const
{
    return pos + 1 >= int( tokens.size());
}

int XmlStream::getNextToken()
{
    ++pos;
    if( pos < int( tokens.size()))
        return tokens[ pos ];
    return XML_TOKEN_INVALID;
}

int XmlStream::peekNextToken() const
{
    if( pos - 1 < int( tokens.size()))
        return tokens[ pos + 1 ];
    return XML_TOKEN_INVALID;
}

AttributeList XmlStream::getAttributes()
{
    assert( pos < int( attributes.size()));
    return attributes[ pos ];
}

rtl::OUString XmlStream::getCharacters()
{
    assert( pos < int( characters.size()));
    return characters[ pos ];
}

void XmlStreamBuilder::appendOpeningTag( int token, const uno::Reference< xml::sax::XFastAttributeList >& attrs )
{
    tokens.push_back( OPENING( token ));
    attributes.push_back( AttributeList( attrs ));
    characters.push_back( rtl::OUString());
}

void XmlStreamBuilder::appendClosingTag( int token )
{
    tokens.push_back( CLOSING( token ));
    attributes.push_back( AttributeList( uno::Reference< xml::sax::XFastAttributeList >()));
    characters.push_back( rtl::OUString());
}

void XmlStreamBuilder::appendCharacters( const rtl::OUString& chars )
{
    assert( !characters.empty());
    characters.back() = chars;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
