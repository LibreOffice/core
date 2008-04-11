/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: serialization.hxx,v $
 * $Revision: 1.5 $
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

#ifndef __SERIALIZATION_HXX
#define __SERIALIZATION_HXX

#include <map>

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>

namespace CSS = com::sun::star;

/**
Serialize an XObject
*/

typedef std::map<rtl::OUString, rtl::OUString> PropMap;

class CSerialization
{
protected:
    CSS::uno::Reference< CSS::xml::dom::XDocumentFragment > m_aFragment;
    PropMap m_properties;

public:
    virtual ~CSerialization() {}

    /**
    sets the XObject that is to serialized
    */
    virtual void setSource(const CSS::uno::Reference< CSS::xml::dom::XDocumentFragment >& aFragment)
    {
        m_aFragment = aFragment;
    }

    /**
    set the properties from the submission element
    that control aspects of the serialization
    eachs serialization may support individual properties
    */
    void setProperties(const CSS::uno::Sequence< CSS::beans::NamedValue >& props)
    {
        m_properties.clear();
        rtl::OUString aValue;
        for (sal_Int32 i=0; i<props.getLength(); i++)
        {
            if (props[i].Value >>= aValue)
                m_properties.insert(PropMap::value_type(props[i].Name, aValue));
        }
    }

    /**
    start the serialization process
    */
    virtual void serialize()=0;

    /**
    get the serialized bytes.
    reads up to buffer->getLength() bytes and returns the number of
    bytes read.
    returns -1 on error
    */
    virtual CSS::uno::Reference< CSS::io::XInputStream > getInputStream() = 0;

};

#endif
