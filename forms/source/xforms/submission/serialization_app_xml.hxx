/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: serialization_app_xml.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:24:17 $
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

#ifndef __SERIALIZATION_APP_XML_HXX
#define __SERIALIZATION_APP_XML_HXX

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "serialization.hxx"


class CSerializationAppXML : public CSerialization
{
private:
    CSS::uno::Reference< CSS::lang::XMultiServiceFactory > m_aFactory;
    CSS::uno::Reference< CSS::io::XOutputStream > m_aPipe;

    void serialize_node(const CSS::uno::Reference< CSS::xml::dom::XNode >& aNode);
    void serialize_nodeset();

public:
    CSerializationAppXML();

    virtual void serialize();
    //    virtual void setSource(const CSS::uno::Reference< CSS::xml::xpath::XXPathObject >& object);
    //    virtual void setProperties(const CSS::uno::Sequence< CSS::beans::NamedValue >& props);
    virtual CSS::uno::Reference< CSS::io::XInputStream > getInputStream();
};

#endif
