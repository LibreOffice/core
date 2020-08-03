/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <rtl/ref.hxx>

#include "node.hxx"
#include "nodemap.hxx"

namespace configmgr {

class RootNode: public Node {
public:
    RootNode();

private:
    virtual ~RootNode() override;

    virtual Kind kind() const override;

    virtual rtl::Reference< Node > clone(bool keepTemplateName) const override;

    virtual NodeMap & getMembers() override;

    NodeMap members_;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
