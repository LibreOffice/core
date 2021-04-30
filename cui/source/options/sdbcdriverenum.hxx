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

#include <rtl/ustring.hxx>

#include <vector>
#include <memory>


namespace offapp
{

    class ODriverEnumerationImpl;
    /** simple class for accessing SDBC drivers registered within the office
        <p>Rather small, introduced to not contaminate other instances with the
        exception handling (code-size-bloating) implementations here.
        </p>
    */
    class ODriverEnumeration
    {
    private:
        std::unique_ptr<ODriverEnumerationImpl> m_pImpl;

    public:
        ODriverEnumeration() noexcept;
        ~ODriverEnumeration() noexcept;
        typedef std::vector< OUString >::const_iterator const_iterator;

        const_iterator  begin() const noexcept;
        const_iterator  end() const noexcept;
    };


}   // namespace offapp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
