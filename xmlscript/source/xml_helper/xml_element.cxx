/*************************************************************************
 *
 *  $RCSfile: xml_element.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-15 14:44:15 $
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
#include <xmlscript/xml_helper.hxx>


using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;


namespace xmlscript
{

//__________________________________________________________________________________________________
void XMLElement::addAttribute( OUString const & rAttrName, OUString const & rValue )
    SAL_THROW( () )
{
    _attrNames.push_back( rAttrName );
    _attrValues.push_back( rValue );
}
//__________________________________________________________________________________________________
void XMLElement::addSubElement( Reference< xml::sax::XAttributeList > const & xElem )
    SAL_THROW( () )
{
    _subElems.push_back( xElem );
}
//__________________________________________________________________________________________________
Reference< xml::sax::XAttributeList > XMLElement::getSubElement( sal_Int32 nIndex )
    SAL_THROW( () )
{
    return _subElems[ (size_t)nIndex ];
}
//__________________________________________________________________________________________________
void XMLElement::dump( Reference< xml::sax::XExtendedDocumentHandler > const & xOut )
{
    xOut->ignorableWhitespace( OUString() );
    xOut->startElement( _name, static_cast< xml::sax::XAttributeList * >( this ) );
    // write sub elements
    for ( size_t nPos = 0; nPos < _subElems.size(); ++nPos )
    {
        XMLElement * pElem = static_cast< XMLElement * >( _subElems[ nPos ].get() );
        pElem->dump( xOut );
    }
    //
    xOut->ignorableWhitespace( OUString() );
    xOut->endElement( _name );
}

// XAttributeList
//__________________________________________________________________________________________________
sal_Int16 XMLElement::getLength()
    throw (RuntimeException)
{
    return _attrNames.size();
}
//__________________________________________________________________________________________________
OUString XMLElement::getNameByIndex( sal_Int16 nPos )
    throw (RuntimeException)
{
    OSL_ASSERT( (size_t)nPos < _attrNames.size() );
    return _attrNames[ nPos ];
}
//__________________________________________________________________________________________________
OUString XMLElement::getTypeByIndex( sal_Int16 nPos )
    throw (RuntimeException)
{
    OSL_ASSERT( (size_t)nPos < _attrNames.size() );
    // xxx todo
    return OUString();
}
//__________________________________________________________________________________________________
OUString XMLElement::getTypeByName( OUString const & rName )
    throw (RuntimeException)
{
    // xxx todo
    return OUString();
}
//__________________________________________________________________________________________________
OUString XMLElement::getValueByIndex( sal_Int16 nPos )
    throw (RuntimeException)
{
    OSL_ASSERT( (size_t)nPos < _attrNames.size() );
    return _attrValues[ nPos ];
}
//__________________________________________________________________________________________________
OUString XMLElement::getValueByName( OUString const & rName )
    throw (RuntimeException)
{
    for ( size_t nPos = 0; nPos < _attrNames.size(); ++nPos )
    {
        if (_attrNames[ nPos ] == rName)
        {
            return _attrValues[ nPos ];
        }
    }
    return OUString();
}

};
