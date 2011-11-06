/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"
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
