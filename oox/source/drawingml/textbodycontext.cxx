/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textbodycontext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/drawingml/textbodypropertiescontext.hxx"
#include "oox/drawingml/textbodycontext.hxx"
#include "oox/drawingml/textparagraphpropertiescontext.hxx"
#include "oox/drawingml/textcharacterpropertiescontext.hxx"
#include "oox/drawingml/textliststylecontext.hxx"
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
class TextParagraphContext : public Context
{
public:
    TextParagraphContext( const FragmentHandlerRef& xHandler, TextParagraph& rPara );

    virtual void SAL_CALL endFastElement( sal_Int32 aElementToken ) throw (SAXException, RuntimeException);
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException);

protected:
    TextParagraph& mrParagraph;
};

// --------------------------------------------------------------------
TextParagraphContext::TextParagraphContext( const FragmentHandlerRef& xHandler, TextParagraph& rPara )
: Context( xHandler )
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
    case NMSP_DRAWINGML|XML_r:      // "CT_RegularTextRun" Regular Text Run.
    {
        TextRunPtr pRun( new TextRun() );
        mrParagraph.addRun( pRun );
        xRet.set( new RegularTextRunContext( getHandler(), pRun ) );
        break;
    }
    case NMSP_DRAWINGML|XML_br: // "CT_TextLineBreak" Soft return line break (vertical tab).
    {
        TextRunPtr pRun( new TextRun() );
        pRun->setLineBreak();
        mrParagraph.addRun( pRun );
        xRet.set( new RegularTextRunContext( getHandler(), pRun ) );
        break;
    }
    case NMSP_DRAWINGML|XML_fld:    // "CT_TextField" Text Field.
    {
        TextFieldPtr pField( new TextField() );
        mrParagraph.addRun( pField );
        xRet.set( new TextFieldContext( this, xAttribs, pField ) );
        break;
    }
    case NMSP_DRAWINGML|XML_pPr:
        xRet.set( new TextParagraphPropertiesContext( this, xAttribs, *(mrParagraph.getProperties().get()) ) );
        break;
    case NMSP_DRAWINGML|XML_endParaRPr:
        xRet.set( new TextParagraphPropertiesContext( this, xAttribs, *(mrParagraph.getEndProperties().get()) ) );
        break;
    }

    return xRet;
}
// --------------------------------------------------------------------

RegularTextRunContext::RegularTextRunContext( const FragmentHandlerRef& xHandler, oox::drawingml::TextRunPtr pRunPtr )
: Context( xHandler )
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
        mpRunPtr->text() += aChars;
    }
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > RegularTextRunContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet( this );

    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_rPr:    // "CT_TextCharPropertyBag" The text char properties of this text run.
        xRet.set( new TextCharacterPropertiesContext( this, xAttribs, *(mpRunPtr->getTextCharacterProperties().get()) ) );
        break;
    case NMSP_DRAWINGML|XML_t:      // "xsd:string" minOccurs="1" The actual text string.
        mbIsInText = true;
        break;
    }

    return xRet;
}

// --------------------------------------------------------------------

TextBodyContext::TextBodyContext( const ::oox::core::FragmentHandlerRef& xHandler, oox::drawingml::Shape& rShape )
: Context( xHandler )
, mrShape( rShape )
, mpBodyPtr( new TextBody() )
{
    rShape.setTextBody( mpBodyPtr );
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
    case NMSP_DRAWINGML|XML_bodyPr:     // CT_TextBodyPropertyBag
        xRet.set( new TextBodyPropertiesContext( this, xAttribs, mrShape ) );
        break;
    case NMSP_DRAWINGML|XML_lstStyle:   // CT_TextListStyle
        xRet.set( new TextListStyleContext( getHandler(), *(mpBodyPtr->getTextListStyle().get()) ) );
        break;
    case NMSP_DRAWINGML|XML_p:          // CT_TextParagraph
        TextParagraphPtr pPara( new TextParagraph() );
        mpBodyPtr->addParagraph( pPara );
        xRet.set( new TextParagraphContext( getHandler(), *(pPara.get()) ) );
        break;
    }

    return xRet;
}

// --------------------------------------------------------------------

} }

