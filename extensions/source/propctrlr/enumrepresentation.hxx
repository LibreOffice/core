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

#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <vector>


namespace pcr
{


    //= IPropertyEnumRepresentation

    class SAL_NO_VTABLE IPropertyEnumRepresentation : public salhelper::SimpleReferenceObject
    {
    public:
        /** retrieves all descriptions of all possible values of the enumeration property
        */
        virtual std::vector< OUString > getDescriptions(
            ) const = 0;

        /** converts a given description into a property value
        */
        virtual void getValueFromDescription(
                const OUString& _rDescription,
                css::uno::Any& _out_rValue
            ) const = 0;

        /** converts a given property value into a description
        */
        virtual OUString getDescriptionForValue(
                const css::uno::Any& _rEnumValue
            ) const = 0;

    };


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
