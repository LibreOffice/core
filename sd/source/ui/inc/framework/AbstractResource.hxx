/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <comphelper/compbase.hxx>
#include <sddllapi.h>

namespace sd::framework
{
/** Base interface that provides functionality shared by all resource types
    of the drawing framework.
*/
class SD_DLLPUBLIC AbstractResource : public comphelper::WeakComponentImplHelper<>
{
public:
    virtual ~AbstractResource() override;

    /** Return an XResourceId object for the called resource.
        The returned id unambiguously identifies the resource.
    */
    virtual css::uno::Reference<css::drawing::framework::XResourceId> getResourceId() = 0;

    /** Some resources must not be leafs, i.e. have to be anchor to at least
        one other resource.  Most panes are examples for this.  Views on the
        other hand are in most cases no anchors.  So the typical pane will
        return `TRUE` and the typical view will return `FALSE`.

        The return value is used to determine whether a resource has to be
        deactivated when it has no children, either because none is
        requested or because none can be created.
    */
    virtual bool isAnchorOnly() = 0;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
