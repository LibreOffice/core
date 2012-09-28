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

#ifndef __SERIALIZATION_APP_XML_HXX
#define __SERIALIZATION_APP_XML_HXX

#include <com/sun/star/io/XPipe.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "serialization.hxx"


class CSerializationAppXML : public CSerialization
{
private:
    CSS::uno::Reference< CSS::lang::XMultiServiceFactory > m_aFactory;
    CSS::uno::Reference< CSS::io::XPipe > m_aPipe;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
