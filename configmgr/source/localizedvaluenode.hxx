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

#ifndef INCLUDED_CONFIGMGR_SOURCE_LOCALIZEDVALUENODE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_LOCALIZEDVALUENODE_HXX

#include "sal/config.h"

#include "com/sun/star/uno/Any.hxx"
#include "rtl/ref.hxx"

#include "node.hxx"

namespace rtl { class OUString; }

namespace configmgr {

class LocalizedValueNode: public Node {
public:
    LocalizedValueNode(int layer, com::sun::star::uno::Any const & value);

    virtual rtl::Reference< Node > clone(bool keepTemplateName) const;

    virtual rtl::OUString getTemplateName() const;

    com::sun::star::uno::Any getValue() const;

    void setValue(int layer, com::sun::star::uno::Any const & value);

private:
    LocalizedValueNode(LocalizedValueNode const & other);

    virtual ~LocalizedValueNode();

    virtual Kind kind() const;

    com::sun::star::uno::Any value_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
