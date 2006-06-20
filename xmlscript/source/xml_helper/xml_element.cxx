/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xml_element.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:11:17 $
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
void XMLElement::dumpSubElements( Reference< xml::sax::XDocumentHandler > const & xOut )
{
    for ( size_t nPos = 0; nPos < _subElems.size(); ++nPos )
    {
        XMLElement * pElem = static_cast< XMLElement * >( _subElems[ nPos ].get() );
        pElem->dump( xOut );
    }
}
//__________________________________________________________________________________________________
void XMLElement::dump( Reference< xml::sax::XDocumentHandler > const & xOut )
{
    xOut->ignorableWhitespace( OUString() );
    xOut->startElement( _name, static_cast< xml::sax::XAttributeList * >( this ) );
    // write sub elements
    dumpSubElements( xOut );
    //
    xOut->ignorableWhitespace( OUString() );
    xOut->endElement( _name );
}

// XAttributeList
//__________________________________________________________________________________________________
sal_Int16 XMLElement::getLength()
    throw (RuntimeException)
{
    return static_cast<sal_Int16>(_attrNames.size());
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
    static_cast<void>(nPos);
    // xxx todo
    return OUString();
}
//__________________________________________________________________________________________________
OUString XMLElement::getTypeByName( OUString const & /*rName*/ )
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

}
