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

#include <uno/any2.h>

namespace com::sun::star::uno { class TypeDescription; }

namespace binaryurp {

class BinaryAny {
public:
    BinaryAny() noexcept;

    BinaryAny(com::sun::star::uno::TypeDescription const & type, void * value)
        noexcept;

    explicit BinaryAny(uno_Any const & raw) noexcept;
        // takes over raw.pData (but copies raw.pType); raw must not be passed
        // to uno_any_destruct

    BinaryAny(BinaryAny const & other) noexcept;

    BinaryAny(BinaryAny && other) noexcept;

    ~BinaryAny() noexcept;

    BinaryAny & operator =(BinaryAny const & other) noexcept;

    BinaryAny & operator =(BinaryAny && other) noexcept;

    uno_Any& get() noexcept { return data_; }

    com::sun::star::uno::TypeDescription getType() const noexcept;

    void * getValue(com::sun::star::uno::TypeDescription const & type) const
        noexcept;

private:
    mutable uno_Any data_;
        // mutable so that getValue() can return a non-const void *, as in turn
        // required at various places in binary UNO
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
