/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textfield.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:28:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#include "oox/drawingml/textfield.hxx"

#include <list>

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextField.hpp>

#include "oox/helper/helper.hxx"
#include "oox/drawingml/textparagraphproperties.hxx"

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
        : mpTextParagraphPropertiesPtr( new TextParagraphProperties())
    {
    }


    /** intsanciate the textfields. Because of semantics difference between
     * OpenXML and OpenOffice, some OpenXML field might cause two fields to be created.
     * @param aFields the created fields. The list is empty if no field has been created.
     * @param xModel the model
     * @param sType the OpenXML field type.
     */
    static void createTextFields( std::list< Reference< XTextField > > & aFields,
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
                    createTextFields( aFields, xModel, CREATE_OUSTRING( "datetime12" ) );
                    break;
                case 9: // DateTime dd/mm/yy H:MM:SS PM
                    createTextFields( aFields, xModel, CREATE_OUSTRING( "datetime13" ) );
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

    void TextField::insertAt(
            const ::oox::core::XmlFilterBase& rFilterBase,
            const Reference < XText > & xText,
            const Reference < XTextCursor > &xAt,
            const TextCharacterPropertiesPtr& rTextCharacterStyle )
    {
        try {

            PropertyMap aioBulletList;
            Reference< XTextRange > xStart( xAt, UNO_QUERY );
            Reference< XPropertySet > xProps( xStart, UNO_QUERY);
            mpTextParagraphPropertiesPtr->pushToPropSet( rFilterBase, xProps, aioBulletList, sal_True );

            if ( rTextCharacterStyle.get() )
                rTextCharacterStyle->pushToPropSet( rFilterBase, xProps );

            maTextCharacterPropertiesPtr->pushToPropSet( rFilterBase, xProps );

            std::list< Reference< XTextField > > fields;
            createTextFields( fields, rFilterBase.getModel(), msType );
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
                xText->insertString( xStart, text(), sal_False );
            }
        }
        catch( const Exception&  )
        {
            OSL_TRACE("OOX:  TextField::insertAt() exception");
        }
    }


} }
