/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <drawingml/textfieldcontext.hxx>
#include <drawingml/textparagraphpropertiescontext.hxx>
#include <drawingml/textcharacterpropertiescontext.hxx>
#include <drawingml/textfield.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;

namespace oox::drawingml {

TextFieldContext::TextFieldContext( ContextHandler2Helper const & rParent,
            const AttributeList& rAttributes,
            TextField& rTextField)
    : ContextHandler2( rParent )
        , mrTextField( rTextField )
        , mbIsInText( false )
{
    mrTextField.setUuid( rAttributes.getStringDefaulted( XML_id ) );
    mrTextField.setType( rAttributes.getStringDefaulted( XML_type ) );
}

void TextFieldContext::onEndElement( )
{
    if( getCurrentElement() == (A_TOKEN( t )) )
    {
        mbIsInText = false;
    }
}

void TextFieldContext::onCharacters( const OUString& aChars )
{
    if( mbIsInText )
    {
        mrTextField.getText() += aChars;
    }
}

ContextHandlerRef TextFieldContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    case A_TOKEN( rPr ):
        return new TextCharacterPropertiesContext( *this, rAttribs, mrTextField.getTextCharacterProperties() );
    case A_TOKEN( pPr ):
        return new TextParagraphPropertiesContext( *this, rAttribs, mrTextField.getTextParagraphProperties() );
    case A_TOKEN( t ):
        mbIsInText = true;
        break;
    }
    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
