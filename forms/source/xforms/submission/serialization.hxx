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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>

/**
Serialize an XObject
*/

class CSerialization
{
protected:
    css::uno::Reference<css::xml::dom::XDocumentFragment> m_aFragment;

public:
    virtual ~CSerialization() {}

    /**
    sets the XObject that is to serialized
    */
    void setSource(const css::uno::Reference<css::xml::dom::XDocumentFragment>& aFragment)
    {
        m_aFragment = aFragment;
    }

    /**
    start the serialization process
    */
    virtual void serialize() = 0;

    /**
    get the serialized bytes.
    reads up to buffer->getLength() bytes and returns the number of
    bytes read.
    returns -1 on error
    */
    virtual css::uno::Reference<css::io::XInputStream> getInputStream() = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
