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

#ifndef INCLUDED_CONFIGMGR_SETNODE_HXX
#define INCLUDED_CONFIGMGR_SETNODE_HXX

#include "sal/config.h"

#include <vector>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "node.hxx"
#include "nodemap.hxx"

namespace configmgr {

class SetNode: public Node {
public:
    SetNode(
        rtl::OUString const & defaultTemplateName,
        std::vector< rtl::OUString > const & additionalTemplateNames,
        rtl::OUString const & templateName);

    virtual rtl::Reference< Node > clone() const;

    virtual rtl::Reference< Node > getMember(rtl::OUString const & name);

    rtl::OUString const & getDefaultTemplateName() const;

    bool isValidTemplate(rtl::OUString const & templateName) const;

    rtl::OUString getTemplateName() const;

    NodeMap & getMembers();

private:
    virtual ~SetNode();

    rtl::OUString defaultTemplateName_;
    std::vector< rtl::OUString > additionalTemplateNames_;
    NodeMap members_;
    rtl::OUString templateName_;
        // non-empty iff this node is a template, free node, or set member
};

}

#endif
