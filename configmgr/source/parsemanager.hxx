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

#include <set>

#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <xmlreader/span.hxx>
#include <xmlreader/xmlreader.hxx>


namespace configmgr {

class Parser;

class ParseManager: public salhelper::SimpleReferenceObject {
public:
    ParseManager(
        OUString const & url, rtl::Reference< Parser > const & parser);

    bool parse(std::set< OUString > const * existingDependencies);

    enum { NAMESPACE_OOR = 1, NAMESPACE_XS = 2, NAMESPACE_XSI = 3 };

private:
    virtual ~ParseManager() override;

    xmlreader::XmlReader reader_;
    rtl::Reference< Parser > parser_;
    xmlreader::Span itemData_;
    int itemNamespaceId_;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
