/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/drawingml/textbodycontext.hxx"
#include "oox/drawingml/textbodypropertiescontext.hxx"
#include "oox/drawingml/textparagraph.hxx"
#include "oox/drawingml/textparagraphpropertiescontext.hxx"
#include "oox/drawingml/textcharacterpropertiescontext.hxx"
#include "oox/drawingml/textliststylecontext.hxx"
#include "oox/drawingml/textfield.hxx"
#include "oox/drawingml/textfieldcontext.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

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
}

// --------------------------------------------------------------------
void TextParagraphContext::endFastElement( sal_Int32 aElementToken ) throw (SAXException, RuntimeException)
{
    if( aElementToken == (NMSP_DRAWINGML|XML_p) )
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
    case NMSP_DRAWINGML|XML_r:		// "CT_RegularTextRun" Regular Text Run.
    {
        TextRunPtr pRun( new TextRun );
        mrParagraph.addRun( pRun );
        xRet.set( new RegularTextRunContext( *this, pRun ) );
        break;
    }
    case NMSP_DRAWINGML|XML_br:	// "CT_TextLineBreak" Soft return line break (vertical tab).
    {
        TextRunPtr pRun( new TextRun );
        pRun->setLineBreak();
        mrParagraph.addRun( pRun );
        xRet.set( new RegularTextRunContext( *this, pRun ) );
        break;
    }
    case NMSP_DRAWINGML|XML_fld:	// "CT_TextField" Text Field.
    {
        TextFieldPtr pField( new TextField );
        mrParagraph.addRun( pField );
        xRet.set( new TextFieldContext( *this, xAttribs, *pField ) );
        break;
    }
    case NMSP_DRAWINGML|XML_pPr:
        xRet.set( new TextParagraphPropertiesContext( *this, xAttribs, mrParagraph.getProperties() ) );
        break;
    case NMSP_DRAWINGML|XML_endParaRPr:
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
    case NMSP_DRAWINGML|XML_t:
    {
        mbIsInText = false;
        break;
    }
    case NMSP_DRAWINGML|XML_r:
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
    case NMSP_DRAWINGML|XML_rPr:	// "CT_TextCharPropertyBag" The text char properties of this text run.
        xRet.set( new TextCharacterPropertiesContext( *this, xAttribs, mpRunPtr->getTextCharacterProperties() ) );
        break;
    case NMSP_DRAWINGML|XML_t:		// "xsd:string" minOccurs="1" The actual text string.
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
    case NMSP_DRAWINGML|XML_bodyPr:		// CT_TextBodyPropertyBag
        xRet.set( new TextBodyPropertiesContext( *this, xAttribs, mrTextBody.getTextProperties() ) );
        break;
    case NMSP_DRAWINGML|XML_lstStyle:	// CT_TextListStyle
        xRet.set( new TextListStyleContext( *this, mrTextBody.getTextListStyle() ) );
        break;
    case NMSP_DRAWINGML|XML_p:			// CT_TextParagraph
        xRet.set( new TextParagraphContext( *this, mrTextBody.addParagraph() ) );
        break;
    }

    return xRet;
}

// --------------------------------------------------------------------

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
