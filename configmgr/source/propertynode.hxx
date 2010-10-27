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

#ifndef INCLUDED_CONFIGMGR_SOURCE_PROPERTYNODE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_PROPERTYNODE_HXX

#include "sal/config.h"

#include "com/sun/star/uno/Any.hxx"
#include "rtl/ref.hxx"

#include "node.hxx"
#include "type.hxx"

namespace rtl { class OUString; }

namespace configmgr {

class Components;

class PropertyNode: public Node {
public:
    PropertyNode(
        int layer, Type staticType, bool nillable,
        com::sun::star::uno::Any const & value, bool extension);

    virtual rtl::Reference< Node > clone(bool keepTemplateName) const;

    Type getStaticType() const;

    bool isNillable() const;

    com::sun::star::uno::Any getValue(Components & components);

    void setValue(int layer, com::sun::star::uno::Any const & value);

    void setExternal(int layer, rtl::OUString const & descriptor);

    bool isExtension() const;

private:
    PropertyNode(PropertyNode const & other);

    virtual ~PropertyNode();

    virtual Kind kind() const;

    Type staticType_;
        // as specified in the component-schema (TYPE_ANY, ...,
        // TYPE_HEXBINARY_LIST; not TYPE_ERROR or TYPE_NIL)
    bool nillable_;
    bool extension_;
    rtl::OUString externalDescriptor_;
    com::sun::star::uno::Any value_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
