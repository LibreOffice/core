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

#include <sal/config.h>

#include <climits>
#include "config_map.hxx"
#include <vector>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <salhelper/simplereferenceobject.hxx>

#include "additions.hxx"
#include "modifications.hxx"
#include "nodemap.hxx"

namespace configmgr {

class Node;

struct Data {
    enum { NO_LAYER = INT_MAX };

    struct ExtensionXcu: public salhelper::SimpleReferenceObject {
        int layer;
        Additions additions;
    };

    NodeMap templates;

    Modifications modifications;

    static OUString createSegment(
        OUString const & templateName, OUString const & name);

    static sal_Int32 parseSegment(
        OUString const & path, sal_Int32 index, OUString * name,
        bool * setElement, OUString * templateName);

    static OUString fullTemplateName(
        OUString const & component, OUString const & name);

    //TODO: better rules under which circumstances a short template name matches
    static bool equalTemplateNames(
        OUString const & shortName, OUString const & longName);

    Data();

    rtl::Reference< Node > resolvePathRepresentation(
        OUString const & pathRepresentation,
        OUString * canonicRepresentation, std::vector<OUString> * path, int * finalizedLayer)
        const;

    rtl::Reference< Node > getTemplate(
        int layer, OUString const & fullName) const;

    NodeMap & getComponents() const;

    Additions * addExtensionXcuAdditions(
        OUString const & url, int layer);

    rtl::Reference< ExtensionXcu > removeExtensionXcuAdditions(
        OUString const & url);

private:
    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;

    typedef config_map< rtl::Reference< ExtensionXcu > >
        ExtensionXcuAdditions;

    rtl::Reference< Node > root_;

    ExtensionXcuAdditions extensionXcuAdditions_;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
