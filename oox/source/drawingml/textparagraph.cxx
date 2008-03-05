/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textparagraph.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:29:58 $
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


#include <algorithm>
#include <boost/bind.hpp>

#include <rtl/ustring.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <comphelper/processfactory.hxx>

#include "oox/drawingml/textparagraph.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

namespace oox { namespace drawingml {

    TextParagraph::TextParagraph()
        : mpProperties( new TextParagraphProperties( ) )
        , mpEndProperties( new TextParagraphProperties( ) )
    {
    }


    TextParagraph::~TextParagraph()
    {
    }


    void TextParagraph::insertAt(
            const ::oox::core::XmlFilterBase& rFilterBase,
            const Reference < XText > &xText,
            const Reference < XTextCursor > &xAt,
            const TextListStylePtr& rTextStyleList, bool bFirst)
    {
        try {
            sal_Int32 nParagraphSize = 0;
            Reference< XTextRange > xStart( xAt, UNO_QUERY );

            sal_Int16 nLevel = mpProperties->getLevel();
            std::vector< TextParagraphPropertiesPtr >& rListStyle = rTextStyleList->getListStyle();
            if ( nLevel >= static_cast< sal_Int16 >( rListStyle.size() ) )
                nLevel = 0;
            TextParagraphPropertiesPtr pTextParagraphStyle;
            TextCharacterPropertiesPtr pTextCharacterStyle;
            if ( rListStyle.size() )
                pTextParagraphStyle = rListStyle[ nLevel ];
            if ( pTextParagraphStyle.get() )
                pTextCharacterStyle = pTextParagraphStyle->getTextCharacterProperties();

            if( !bFirst )
            {
                xText->insertControlCharacter( xStart, ControlCharacter::APPEND_PARAGRAPH, sal_False );
                xAt->gotoEnd(true);
            }

            std::vector< TextRunPtr >::iterator begin( maRuns.begin() );
            while( begin != maRuns.end() )
            {
                (*begin)->insertAt( rFilterBase, xText, xAt, pTextCharacterStyle );
                nParagraphSize += (*begin++)->text().getLength();
            }
            xAt->gotoEnd(true);

            PropertyMap aioBulletList;
            Reference< XPropertySet > xProps( xStart, UNO_QUERY);
            if ( pTextParagraphStyle.get() )
                pTextParagraphStyle->pushToPropSet( rFilterBase, xProps, aioBulletList, sal_False );

            mpProperties->pushToPropSet( rFilterBase, xProps, aioBulletList, sal_True );

            // empty paragraphs do not have bullets in ppt
            if ( !nParagraphSize )
            {
                const rtl::OUString sIsNumbering( CREATE_OUSTRING( "IsNumbering" ) );
                xProps->setPropertyValue( sIsNumbering, Any( sal_False ) );
            }

// FIXME this is causing a lot of dispruption (ie does not work). I wonder what to do -- Hub
//          Reference< XTextRange > xEnd( xAt, UNO_QUERY );
//      Reference< XPropertySet > xProps2( xEnd, UNO_QUERY );
//          mpEndProperties->pushToPropSet( xProps2 );
        }
        catch( Exception & )
        {
            OSL_TRACE("OOX: exception in TextParagraph::insertAt");
        }
    }


} }

