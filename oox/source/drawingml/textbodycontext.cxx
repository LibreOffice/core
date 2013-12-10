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

#include "oox/drawingml/textbodycontext.hxx"
#include "oox/drawingml/textbodypropertiescontext.hxx"
#include "oox/drawingml/textparagraph.hxx"
#include "oox/drawingml/textparagraphpropertiescontext.hxx"
#include "oox/drawingml/textcharacterpropertiescontext.hxx"
#include "oox/drawingml/textliststylecontext.hxx"
#include "oox/drawingml/textfield.hxx"
#include "oox/drawingml/textfieldcontext.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// --------------------------------------------------------------------

// CT_TextParagraph
class TextParagraphContext : public ContextHandler
{
public:
    TextParagraphContext( ContextHandler& rParent, TextParagraph& rPara );

    virtual void SAL_CALL endFastElement( sal_Int32 aElementToken ) throw (SAXException, RuntimeException);
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);

protected:
    TextParagraph& mrParagraph;
};

// --------------------------------------------------------------------
TextParagraphContext::TextParagraphContext( ContextHandler& rParent, TextParagraph& rPara )
: ContextHandler( rParent )
, mrParagraph( rPara )
{
    mbEnableTrimSpace = false;
}

// --------------------------------------------------------------------
void TextParagraphContext::endFastElement( sal_Int32 aElementToken ) throw (SAXException, RuntimeException)
{
    if( aElementToken == (A_TOKEN( p )) )
    {
    }
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > TextParagraphContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    // EG_TextRun
    switch( aElementToken )
    {
    case A_TOKEN( r ):      // "CT_RegularTextRun" Regular Text Run.
    {
        TextRunPtr pRun( new TextRun );
        mrParagraph.addRun( pRun );
        xRet.set( new RegularTextRunContext( *this, pRun ) );
        break;
    }
    case A_TOKEN( br ): // "CT_TextLineBreak" Soft return line break (vertical tab).
    {
        TextRunPtr pRun( new TextRun );
        pRun->setLineBreak();
        mrParagraph.addRun( pRun );
        xRet.set( new RegularTextRunContext( *this, pRun ) );
        break;
    }
    case A_TOKEN( fld ):    // "CT_TextField" Text Field.
    {
        TextFieldPtr pField( new TextField );
        mrParagraph.addRun( pField );
        xRet.set( new TextFieldContext( *this, xAttribs, *pField ) );
        break;
    }
    case A_TOKEN( pPr ):
        xRet.set( new TextParagraphPropertiesContext( *this, xAttribs, mrParagraph.getProperties() ) );
        break;
    case A_TOKEN( endParaRPr ):
        xRet.set( new TextCharacterPropertiesContext( *this, xAttribs, mrParagraph.getEndProperties() ) );
        break;
    }

    return xRet;
}
// --------------------------------------------------------------------

RegularTextRunContext::RegularTextRunContext( ContextHandler& rParent, TextRunPtr pRunPtr )
: ContextHandler( rParent )
, mpRunPtr( pRunPtr )
, mbIsInText( false )
{
}

// --------------------------------------------------------------------

void RegularTextRunContext::endFastElement( sal_Int32 aElementToken ) throw (SAXException, RuntimeException)
{
    switch( aElementToken )
    {
    case A_TOKEN( t ):
    {
        mbIsInText = false;
        break;
    }
    case A_TOKEN( r ):
    {
        break;
    }

    }
}

// --------------------------------------------------------------------

void RegularTextRunContext::characters( const OUString& aChars ) throw (SAXException, RuntimeException)
{
    if( mbIsInText )
    {
        mpRunPtr->getText() += aChars;
    }
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > RegularTextRunContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet( this );

    switch( aElementToken )
    {
    case A_TOKEN( rPr ):    // "CT_TextCharPropertyBag" The text char properties of this text run.
        xRet.set( new TextCharacterPropertiesContext( *this, xAttribs, mpRunPtr->getTextCharacterProperties() ) );
        break;
    case A_TOKEN( t ):      // "xsd:string" minOccurs="1" The actual text string.
        mbIsInText = true;
        break;
    }

    return xRet;
}

// --------------------------------------------------------------------

TextBodyContext::TextBodyContext( ContextHandler& rParent, TextBody& rTextBody )
: ContextHandler( rParent )
, mrTextBody( rTextBody )
{
}

// --------------------------------------------------------------------

void TextBodyContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > TextBodyContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case A_TOKEN( bodyPr ):     // CT_TextBodyPropertyBag
        xRet.set( new TextBodyPropertiesContext( *this, xAttribs, mrTextBody.getTextProperties() ) );
        break;
    case A_TOKEN( lstStyle ):   // CT_TextListStyle
        xRet.set( new TextListStyleContext( *this, mrTextBody.getTextListStyle() ) );
        break;
    case A_TOKEN( p ):          // CT_TextParagraph
        xRet.set( new TextParagraphContext( *this, mrTextBody.addParagraph() ) );
        break;
    }

    return xRet;
}

// --------------------------------------------------------------------

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
