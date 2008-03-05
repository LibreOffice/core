/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textrun.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:30:52 $
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

#include "oox/drawingml/textrun.hxx"

#include <rtl/ustring.hxx>

#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextField.hpp>

#include "oox/helper/helper.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

namespace oox { namespace drawingml {

    TextRun::TextRun()
        : mbIsLineBreak( false )
        , maTextCharacterPropertiesPtr( new TextCharacterProperties() )
    {
    }


    TextRun::~TextRun()
    {
    }

    void TextRun::insertAt(
            const ::oox::core::XmlFilterBase& rFilterBase,
            const Reference < XText > & xText,
            const Reference < XTextCursor > &xAt,
            const TextCharacterPropertiesPtr& rTextCharacterStyle )
    {
        try {
            Reference< XTextRange > xStart( xAt, UNO_QUERY );

            Reference< XPropertySet > xProps( xStart, UNO_QUERY);
            if ( rTextCharacterStyle.get() )
                rTextCharacterStyle->pushToPropSet( rFilterBase, xProps );

            maTextCharacterPropertiesPtr->pushToPropSet( rFilterBase, xProps );

            if( maTextCharacterPropertiesPtr->getHyperlinkPropertyMap().empty() )
            {
                 if( mbIsLineBreak )
                {
                    OSL_TRACE( "OOX: TextRun::insertAt() insert line break" );
                    xText->insertControlCharacter( xStart, ControlCharacter::LINE_BREAK, sal_False );
                }
                else
                {
                    xText->insertString( xStart, text(), sal_False );
                }
            }
            else
            {
                OSL_TRACE( "OOX: URL field" );
                Reference< XMultiServiceFactory > xFactory( rFilterBase.getModel(), UNO_QUERY );
                Reference< XTextField > xField( xFactory->createInstance( CREATE_OUSTRING( "com.sun.star.text.TextField.URL" ) ), UNO_QUERY );
                if( xField.is() )
                {
                    const rtl::OUString sRepresentation( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "Representation" ) ) );
                    maTextCharacterPropertiesPtr->getHyperlinkPropertyMap()[ sRepresentation ] <<= text();

                    Reference< XPropertySet > xFieldProps( xField, UNO_QUERY);
                    maTextCharacterPropertiesPtr->pushToUrlFieldPropSet( xFieldProps );
                    Reference< XTextContent > xContent( xField, UNO_QUERY);
                    xText->insertTextContent( xStart, xContent, sal_False );
                }
                else
                {
                    OSL_TRACE( "OOX: URL field couldn't be created" );
                    xText->insertString( xStart, text(), sal_False );
                }
            }
        }
        catch( const Exception&  )
        {
            OSL_TRACE("OOX:  TextRun::insertAt() exception");
        }
    }


} }
