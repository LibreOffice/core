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

#ifndef INCLUDED_CONFIGMGR_SOURCE_SETNODE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_SETNODE_HXX

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
        int layer, rtl::OUString const & defaultTemplateName,
        rtl::OUString const & templateName);

    virtual rtl::Reference< Node > clone() const;

    virtual NodeMap & getMembers();

    virtual rtl::OUString getTemplateName() const;

    virtual void setMandatory(int layer);

    virtual int getMandatory() const;

    rtl::OUString const & getDefaultTemplateName() const;

    std::vector< rtl::OUString > & getAdditionalTemplateNames();

    bool isValidTemplate(rtl::OUString const & templateName) const;

private:
    SetNode(SetNode const & other);

    virtual ~SetNode();

    virtual Kind kind() const;

    virtual void clear();

    rtl::OUString defaultTemplateName_;
    std::vector< rtl::OUString > additionalTemplateNames_;
    NodeMap members_;
    rtl::OUString templateName_;
        // non-empty iff this node is a template, free node, or set member
    int mandatory_;
};

}

#endif
