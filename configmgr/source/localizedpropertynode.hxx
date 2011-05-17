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

#ifndef INCLUDED_CONFIGMGR_SOURCE_LOCALIZEDPROPERTYNODE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_LOCALIZEDPROPERTYNODE_HXX

#include "sal/config.h"

#include "rtl/ref.hxx"

#include "node.hxx"
#include "nodemap.hxx"
#include "type.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }
namespace rtl { class OUString; }

namespace configmgr {

class LocalizedPropertyNode: public Node {
public:
    LocalizedPropertyNode(int layer, Type staticType, bool nillable);

    virtual rtl::Reference< Node > clone(bool keepTemplateName) const;

    virtual NodeMap & getMembers();

    Type getStaticType() const;

    bool isNillable() const;

private:
    LocalizedPropertyNode(LocalizedPropertyNode const & other);

    virtual ~LocalizedPropertyNode();

    virtual Kind kind() const;

    virtual void clear();

    Type staticType_;
        // as specified in the component-schema (TYPE_ANY, ...,
        // TYPE_HEXBINARY_LIST; not TYPE_ERROR or TYPE_NIL)
    bool nillable_;
    NodeMap members_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
