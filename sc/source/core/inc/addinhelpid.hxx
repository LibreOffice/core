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
#include <rtl/string.hxx>

struct ScUnoAddInHelpId;

/** Generates help IDs for standard Calc AddIns. */
class ScUnoAddInHelpIdGenerator
{
private:
    const ScUnoAddInHelpId*     pCurrHelpIds;       /// Array of function names and help IDs.
    sal_uInt32                  nArrayCount;        /// Count of array entries.

public:
                                ScUnoAddInHelpIdGenerator() = delete;
                                ScUnoAddInHelpIdGenerator( std::u16string_view rServiceName );

    /** Sets service name of the AddIn. Has to be done before requesting help IDs. */
    void                        SetServiceName( std::u16string_view rServiceName );

    /** @return  The help ID of the function with given built-in name or 0 if not found. */
    OString                GetHelpId( const OUString& rFuncName ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
