/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: serialization_urlencoded.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 13:06:06 $
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

#ifndef __SERIALIZATION_URLENCODED_HXX
#define __SERIALIZATION_URLENCODED_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <rtl/strbuf.hxx>

#include "serialization.hxx"

class CSerializationURLEncoded : public CSerialization
{
private:
    CSS::uno::Reference< CSS::lang::XMultiServiceFactory > m_aFactory;
    CSS::uno::Reference< CSS::io::XOutputStream > m_aPipe;

    sal_Bool is_unreserved(sal_Char);
    void encode_and_append(const rtl::OUString& aString, rtl::OStringBuffer& aBuffer);
    void serialize_node(const CSS::uno::Reference< CSS::xml::dom::XNode >& aNode);
    void serialize_nodeset();

public:
    CSerializationURLEncoded();
    virtual void serialize();
    virtual CSS::uno::Reference< CSS::io::XInputStream > getInputStream();
};

#endif
