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

#include <string_view>

#include <com/sun/star/io/XPipe.hpp>

#include <rtl/strbuf.hxx>

#include "serialization.hxx"

class CSerializationURLEncoded : public CSerialization
{
private:
    css::uno::Reference<css::io::XPipe> m_aPipe;

    static bool is_unreserved(char);
    static void encode_and_append(std::u16string_view aString, OStringBuffer& aBuffer);
    void serialize_node(const css::uno::Reference<css::xml::dom::XNode>& aNode);

public:
    CSerializationURLEncoded();
    virtual void serialize() override;
    virtual css::uno::Reference<css::io::XInputStream> getInputStream() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
