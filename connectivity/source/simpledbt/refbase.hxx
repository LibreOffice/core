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

#ifndef CONNECTIVITY_DBTOOLS_REFBASE_HXX
#define CONNECTIVITY_DBTOOLS_REFBASE_HXX

#include <rtl/ref.hxx>


namespace connectivity
{



    //= ORefBase

    /// base class for all objects implementing the ::rtl::IReference interface
    class ORefBase : public ::rtl::IReference
    {
    protected:
        oslInterlockedCount     m_refCount;

    protected:
        ORefBase() : m_refCount(0) { }
        virtual ~ORefBase();

        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();
    };


}   // namespace connectivity


#endif // CONNECTIVITY_DBTOOLS_REFBASE_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
