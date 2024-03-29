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

#include <basic/modsizeexceeded.hxx>

#include <comphelper/interaction.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/script/ModuleSizeExceededRequest.hpp>

using namespace com::sun::star;
using namespace cppu;

ModuleSizeExceeded::ModuleSizeExceeded(const std::vector<OUString>& sModules)
{
    script::ModuleSizeExceededRequest aReq;
    aReq.Names = comphelper::containerToSequence(sModules);

    m_aRequest <<= aReq;

    m_xAbort = new comphelper::OInteractionAbort;
    m_xApprove = new comphelper::OInteractionApprove;
    m_lContinuations = { m_xApprove, m_xAbort };
}

bool ModuleSizeExceeded::isAbort() const { return m_xAbort->wasSelected(); }

bool ModuleSizeExceeded::isApprove() const { return m_xApprove->wasSelected(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
