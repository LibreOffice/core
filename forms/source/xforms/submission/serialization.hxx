/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: serialization.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 11:15:04 $
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
