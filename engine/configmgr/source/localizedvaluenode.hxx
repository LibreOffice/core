/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <sal/config.h>

#include <cpo/uno/Any.hxx>

#include "node.hxx"

namespace configmgr
{
class LocalizedValueNode : public Node
{
public:
    explicit LocalizedValueNode(int layer);
    LocalizedValueNode(int layer, cpo::uno::Any value);

    virtual rtl::Reference<Node> clone(bool keepTemplateName) const override;

    virtual OUString getTemplateName() const override;

    const cpo::uno::Any& getValue() const { return value_; }
    cpo::uno::Any* getValuePtr(int layer, bool bIsUserModification)
    {
        setLayer(layer);
        modified_ = bIsUserModification;
        return &value_;
    }

    void setValue(int layer, cpo::uno::Any const& value, bool bIsUserModification);

    bool isModified() { return modified_; }

private:
    LocalizedValueNode(LocalizedValueNode const&) = default;

    virtual ~LocalizedValueNode() override;

    virtual Kind kind() const override;

    cpo::uno::Any value_;
    bool modified_;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
