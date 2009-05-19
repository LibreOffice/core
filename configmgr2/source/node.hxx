/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
************************************************************************/

#ifndef INCLUDED_CONFIGMGR_NODE_HXX
#define INCLUDED_CONFIGMGR_NODE_HXX

#include "sal/config.h"

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "salhelper/simplereferenceobject.hxx"

namespace configmgr {

class Node: public salhelper::SimpleReferenceObject {
public:
    virtual rtl::Reference< Node > clone(
        Node * parent, rtl::OUString const & name) const = 0;

    virtual rtl::Reference< Node > getMember(rtl::OUString const & name) = 0;

    Node * getParent() const;

    rtl::OUString getName() const;

    void unbind() throw ();

protected:
    Node(Node * parent, rtl::OUString const & name);

    virtual ~Node();

    // template or free node: parent_ and name_ both empty
    // component node: parent_ empty, name_ non-empty
    // member node: parent_ and name_ both non-empty
    Node * parent_;
    rtl::OUString name_;
};

}

#endif
