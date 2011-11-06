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
#include "precompiled_xmlscript.hxx"
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
