/*************************************************************************
 *
 *  $RCSfile: XMLElement.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dfoster $ $Date: 2003-07-17 08:36:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "XMLElement.hxx"
#include <osl/diagnose.h>

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace scripting_impl
{

//*************************************************************************
void XMLElement::addAttribute( OUString const & rAttrName, OUString const & rValue )
SAL_THROW( () )
{
    OSL_TRACE( "XMLElement::addAttribute\n" );

    _attrNames.push_back( rAttrName );
    _attrValues.push_back( rValue );
}

//*************************************************************************
void XMLElement::addSubElement( Reference< xml::sax::XAttributeList > const & xElem )
SAL_THROW( () )
{
    OSL_TRACE( "XMLElement::addSubElement\n" );

    _subElems.push_back( xElem );
}

//*************************************************************************
Reference< xml::sax::XAttributeList > XMLElement::getSubElement( sal_Int32 nIndex )
SAL_THROW( () )
{
    OSL_TRACE( "XMLElement::getSubElement\n" );

    return _subElems[ ( size_t )nIndex ];
}

//*************************************************************************
void XMLElement::dumpSubElements( Reference< xml::sax::XExtendedDocumentHandler > const & xOut )
{
    OSL_TRACE( "+++++ XMLElement::dumpSubElement\n" );

    for ( size_t nPos = 0; nPos < _subElems.size(); ++nPos )
    {
        XMLElement * pElem = static_cast< XMLElement * >( _subElems[ nPos ].get() );
        pElem->dump( xOut );
    }
}

//*************************************************************************
void XMLElement::dump( Reference< xml::sax::XExtendedDocumentHandler > const & xOut )
{
    OSL_TRACE( "XMLElement::dump" );

    xOut->ignorableWhitespace( OUString() );
    OSL_TRACE( "XMLElement::dump starting %s",::rtl::OUStringToOString(
                _name, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    xOut->startElement( _name, static_cast< xml::sax::XAttributeList * >( this ) );
    // Write out CDATA
    if( _chars.getLength() > 0 )
    {
        xOut->ignorableWhitespace( OUString() );
        xOut->characters( _chars );
    }
    // write sub elements
    dumpSubElements( xOut );
    xOut->ignorableWhitespace( OUString() );
    xOut->endElement( _name );
    OSL_TRACE( "XMLElement::dump ending %s",::rtl::OUStringToOString(
                _name, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
}

//*************************************************************************
// XAttributeList
sal_Int16 XMLElement::getLength()
throw ( RuntimeException )
{
    OSL_TRACE( "XMLElement::getLength\n" );

    return _attrNames.size();
}

//*************************************************************************
OUString XMLElement::getNameByIndex( sal_Int16 nPos )
throw ( RuntimeException )
{
    OSL_TRACE( "XMLElement::getNameByIndex\n" );
    OSL_ASSERT( ( size_t )nPos < _attrNames.size() );

    return _attrNames[ nPos ];
}

//*************************************************************************
OUString XMLElement::getTypeByIndex( sal_Int16 nPos )
throw ( RuntimeException )
{
    OSL_TRACE( "XMLElement::getTypeByIndex\n" );
    OSL_ASSERT( (size_t)nPos < _attrNames.size() );

    // xxx todo
    return OUString();
}

//*************************************************************************
OUString XMLElement::getTypeByName( OUString const & rName )
throw ( RuntimeException )
{
    OSL_TRACE( "XMLElement::getTypeByName\n" );
    // xxx todo
    return OUString();
}

//*************************************************************************
OUString XMLElement::getValueByIndex( sal_Int16 nPos )
throw ( RuntimeException )
{
    OSL_TRACE( "XMLElement::getValueByIndex\n" );
    OSL_ASSERT( ( size_t )nPos < _attrNames.size() );

    return _attrValues[ nPos ];
}

//*************************************************************************
OUString XMLElement::getValueByName( OUString const & rName )
throw ( RuntimeException )
{
    OSL_TRACE( "XMLElement::getValueByName\n" );

    for ( size_t nPos = 0; nPos < _attrNames.size(); ++nPos )
    {
        if (_attrNames[ nPos ] == rName)
        {
            return _attrValues[ nPos ];
        }
    }
    return OUString();
}

}
