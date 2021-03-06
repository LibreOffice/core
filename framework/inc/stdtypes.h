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

#include <com/sun/star/awt/KeyEvent.hpp>

#include <cppuhelper/interfacecontainer.hxx>
#include <rtl/ustring.hxx>

namespace framework{

/**
    Own hash functions used for stl-structures ... e.g. hash tables/maps ...
*/

struct KeyEventHashCode
{
    size_t operator()( const css::awt::KeyEvent& aEvent ) const
    {
        return static_cast<size_t>(aEvent.KeyCode  +
                        //aEvent.KeyChar  +
                        //aEvent.KeyFunc  +
                        aEvent.Modifiers);
    }
};

struct KeyEventEqualsFunc
{
    bool operator()(const css::awt::KeyEvent& rKey1,
                    const css::awt::KeyEvent& rKey2) const
    {
        return (
                (rKey1.KeyCode   == rKey2.KeyCode  ) &&
                //(rKey1.KeyChar   == rKey2.KeyChar  ) &&
                //(rKey1.KeyFunc   == rKey2.KeyFunc  ) &&
                (rKey1.Modifiers == rKey2.Modifiers)
               );
    }
};

/**
    It can be used to map names (e.g. of properties) to her corresponding handles.
    Our helper class OPropertySetHelper works optimized with handles but sometimes we have only a property name.
    Mapping between these two parts of a property should be done in the fastest way :-)
*/

typedef cppu::OMultiTypeInterfaceContainerHelperVar<OUString> ListenerHash;

}       // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
