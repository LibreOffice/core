/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <xmlscript/xml_helper.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace xmlscript
{

void XMLElement::addAttribute( OUString const & rAttrName, OUString const & rValue )
    SAL_THROW(())
{
    _attrNames.push_back( rAttrName );
    _attrValues.push_back( rValue );
}

void XMLElement::addSubElement( Reference< xml::sax::XAttributeList > const & xElem )
    SAL_THROW(())
{
    _subElems.push_back( xElem );
}

Reference< xml::sax::XAttributeList > XMLElement::getSubElement( sal_Int32 nIndex )
    SAL_THROW(())
{
    return _subElems[ (size_t)nIndex ];
}

void XMLElement::dumpSubElements( Reference< xml::sax::XDocumentHandler > const & xOut )
{
    for ( size_t nPos = 0; nPos < _subElems.size(); ++nPos )
    {
        XMLElement * pElem = static_cast< XMLElement * >( _subElems[ nPos ].get() );
        pElem->dump( xOut );
    }
}

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
sal_Int16 XMLElement::getLength()
    throw (RuntimeException)
{
    return static_cast<sal_Int16>(_attrNames.size());
}

OUString XMLElement::getNameByIndex( sal_Int16 nPos )
    throw (RuntimeException)
{
    OSL_ASSERT( (size_t)nPos < _attrNames.size() );
    return _attrNames[ nPos ];
}

OUString XMLElement::getTypeByIndex( sal_Int16 nPos )
    throw (RuntimeException)
{
    OSL_ASSERT( (size_t)nPos < _attrNames.size() );
    static_cast<void>(nPos);
    // xxx todo
    return OUString();
}

OUString XMLElement::getTypeByName( OUString const & /*rName*/ )
    throw (RuntimeException)
{
    // xxx todo
    return OUString();
}

OUString XMLElement::getValueByIndex( sal_Int16 nPos )
    throw (RuntimeException)
{
    OSL_ASSERT( (size_t)nPos < _attrNames.size() );
    return _attrValues[ nPos ];
}

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
