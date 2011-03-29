/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlscript.hxx"
#include <xmlscript/xml_helper.hxx>


using namespace com::sun::star;
using namespace com::sun::star::uno;

using ::rtl::OUString;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
