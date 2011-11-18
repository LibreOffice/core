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
#ifndef _STARMATHIMPORT_HXX
#define _STARMATHIMPORT_HXX

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <oox/helper/attributelist.hxx>
#include <vector>

#include <oox/dllapi.h>

namespace ooxmlformulaimport
{

const int TAG_OPENING = 1 << 29;
const int TAG_CLOSING = 1 << 30;

// used to differentiate between tags that open or close
// TODO
//inline int OPENING( int token ) { return TAG_OPENING | token; }
//inline int CLOSING( int token ) { return TAG_CLOSING | token; }
#define OPENING( token ) ( TAG_OPENING | token )
#define CLOSING( token ) ( TAG_CLOSING | token )

class OOX_DLLPUBLIC XmlStream
{
public:
    XmlStream();
    bool nextIsEnd() const;
    int peekNextToken() const;
    int getNextToken();
    oox::AttributeList getAttributes();
    rtl::OUString getCharacters();
protected:
    // TODO one list containing all 3?
    std::vector< int > tokens;
    std::vector< oox::AttributeList > attributes;
    std::vector< rtl::OUString > characters;
    int pos;
};

// use this to create the data and then cast to the base class for reading
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

class OOX_DLLPUBLIC OoxmlFormulaImportHelper
{
public:
    OoxmlFormulaImportHelper();
    virtual void addFormula( com::sun::star::uno::Reference< com::sun::star::embed::XEmbeddedObject > ) = 0;
};

class OOX_DLLPUBLIC OoxmlFormulaImportBase
{
public:
    OoxmlFormulaImportBase();
    virtual void readFormulaOoxml( ooxmlformulaimport::XmlStream& stream ) = 0;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
