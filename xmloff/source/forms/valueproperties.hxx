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

#include "controlelement.hxx"
#include <rtl/ustring.hxx>
#include <optional>

namespace xmloff
{

    //= OValuePropertiesMetaData
    class OValuePropertiesMetaData
    {
    protected:
        OValuePropertiesMetaData() { }

    public:
        /** calculate the property names for the <em>current-value</em> and the <em>value</em> attribute.

            <p>If controls of the given FormComponentType do not have any of the properties requested,
            the respective out parameter will be set to NULL.</p>
        */
        static void getValuePropertyNames(
            OControlElement::ElementType _eType,
            sal_Int16 _nFormComponentType,
            std::optional<OUString> & _rpCurrentValuePropertyName,
            std::optional<OUString> & _rpValuePropertyName);

        /** calculate the property names for the <em>min-value</em> and the <em>max-value</em> attribute.

            <p>If controls of the given FormComponentType do not have any of the properties requested,
            the respective out parameter will be set to NULL.</p>
        */
        static void getValueLimitPropertyNames(
            sal_Int16 _nFormComponentType,
            std::optional<OUString> & _rpMinValuePropertyName,
            std::optional<OUString> & _rpMaxValuePropertyName);

        /** calculate the names of the properties which, at runtime, are used for <em>value</em> and
            <em>default value</em>.
        */
        static void getRuntimeValuePropertyNames(
            OControlElement::ElementType _eType,
            sal_Int16 _nFormComponentType,
            std::optional<OUString> & _rpValuePropertyName,
            std::optional<OUString> & _rpDefaultValuePropertyName);
    };

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
