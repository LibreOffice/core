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

#include "oox/drawingml/textfield.hxx"

#include <list>

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextField.hpp>

#include "oox/helper/helper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/textparagraphproperties.hxx"
#include "oox/drawingml/textcharacterproperties.hxx"

using ::rtl::OString;
using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

namespace oox { namespace drawingml {

TextField::TextField()
{
}

namespace {

/** intsanciate the textfields. Because of semantics difference between
 * OpenXML and OpenOffice, some OpenXML field might cause two fields to be created.
 * @param aFields the created fields. The list is empty if no field has been created.
 * @param xModel the model
 * @param sType the OpenXML field type.
 */
void lclCreateTextFields( std::list< Reference< XTextField > > & aFields,
                                                            const Reference< XModel > & xModel, const OUString & sType )
{
    Reference< XInterface > xIface;
    Reference< XMultiServiceFactory > xFactory( xModel, UNO_QUERY_THROW );

    if( sType.compareToAscii( "datetime", 8 ) == 0)
    {
        OString s = ::rtl::OUStringToOString( sType, RTL_TEXTENCODING_UTF8);
        OString p( s.pData->buffer + 8 );
        try
        {
            bool bIsDate = true;
            int idx = p.toInt32();
//              OSL_TRACE( "OOX: p = %s, %d", p.pData->buffer, idx );
            xIface = xFactory->createInstance( CREATE_OUSTRING( "com.sun.star.text.TextField.DateTime" ) );
            aFields.push_back( Reference< XTextField > ( xIface, UNO_QUERY ) );
            Reference< XPropertySet > xProps( xIface, UNO_QUERY_THROW );

            // here we should format the field properly. waiting after #i81091.
            switch( idx )
            {
            case 1: // Date dd/mm/yyyy
                // this is the default format...
                break;
            case 2: // Date Day, Month dd, yyyy
                break;
            case 3: // Date dd Month yyyy
                break;
            case 4: // Date Month dd, yyyy
                break;
            case 5: // Date dd-Mon-yy
                break;
            case 6: // Date Month yy
                break;
            case 7: // Date Mon-yy
                break;
            case 8: // DateTime dd/mm/yyyy H:MM PM
                lclCreateTextFields( aFields, xModel, CREATE_OUSTRING( "datetime12" ) );
                break;
            case 9: // DateTime dd/mm/yy H:MM:SS PM
                lclCreateTextFields( aFields, xModel, CREATE_OUSTRING( "datetime13" ) );
                break;
            case 10: // Time H:MM
                bIsDate = false;
                break;
            case 11: // Time H:MM:SS
                bIsDate = false;
                // this is the default format
                break;
            case 12: // Time H:MM PM
                bIsDate = false;
                break;
            case 13: // Time H:MM:SS PM
                bIsDate = false;
                break;
            }
            xProps->setPropertyValue( CREATE_OUSTRING( "IsDate" ), makeAny( bIsDate ) );
            xProps->setPropertyValue( CREATE_OUSTRING( "IsFixed" ), makeAny( false ) );
        }
        catch(Exception & e)
        {
            OSL_TRACE( "Exception %s",  OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        }
    }
    else if ( sType.compareToAscii( "slidenum" ) == 0 )
    {
        xIface = xFactory->createInstance( CREATE_OUSTRING( "com.sun.star.text.TextField.PageNumber" ) );
        aFields.push_back( Reference< XTextField > ( xIface, UNO_QUERY ) );
    }
}

} // namespace

void TextField::insertAt(
        const ::oox::core::XmlFilterBase& rFilterBase,
        const Reference < XText > & xText,
        const Reference < XTextCursor > &xAt,
        const TextCharacterProperties& rTextCharacterStyle ) const
{
    try
    {
        PropertyMap aioBulletList;
        Reference< XTextRange > xStart( xAt, UNO_QUERY );
        Reference< XPropertySet > xProps( xStart, UNO_QUERY);
        PropertySet aPropSet( xProps );

        maTextParagraphProperties.pushToPropSet( rFilterBase, xProps, aioBulletList, NULL, sal_True, 18 );

        TextCharacterProperties aTextCharacterProps( rTextCharacterStyle );
        aTextCharacterProps.assignUsed( maTextParagraphProperties.getTextCharacterProperties() );
        aTextCharacterProps.assignUsed( getTextCharacterProperties() );
        aTextCharacterProps.pushToPropSet( aPropSet, rFilterBase );

        std::list< Reference< XTextField > > fields;
        lclCreateTextFields( fields, rFilterBase.getModel(), msType );
        if( !fields.empty() )
        {
            bool bFirst = true;
            for( std::list< Reference< XTextField > >::iterator iter = fields.begin();
                     iter != fields.end(); ++iter )
            {
                if( iter->is() )
                {
                    Reference< XTextContent > xContent( *iter, UNO_QUERY);
                    if( bFirst)
                    {
                        bFirst = false;
                    }
                    else
                    {
                        xText->insertString( xStart, CREATE_OUSTRING( " " ), sal_False );
                    }
                    xText->insertTextContent( xStart, xContent, sal_False );
                }
            }
        }
        else
        {
            xText->insertString( xStart, getText(), sal_False );
        }
    }
    catch( const Exception&  )
    {
        OSL_TRACE("OOX:  TextField::insertAt() exception");
    }
}

} }
