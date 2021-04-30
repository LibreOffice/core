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
#ifndef INCLUDED_SVX_DIAGRAMDATAINTERFACE_HXX
#define INCLUDED_SVX_DIAGRAMDATAINTERFACE_HXX

#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <utility>
#include <vector>

/// Interface class to access diagram data for UI
class SAL_NO_VTABLE SAL_DLLPUBLIC_RTTI DiagramDataInterface
{
public:
    // get text representation of data tree
    virtual OUString getString() const = 0;

    // get children of provided data node
    // use empty string for top-level nodes
    // returns vector of (id, text)
    virtual std::vector<std::pair<OUString, OUString>>
    getChildren(const OUString& rParentId) const = 0;

    // add new top-level node to data model, returns its id
    virtual OUString addNode(const OUString& rText) = 0;

    virtual bool removeNode(const OUString& rNodeId) = 0;

protected:
    ~DiagramDataInterface() noexcept {}
};

#endif // INCLUDED_SVX_DIAGRAMDATAINTERFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
