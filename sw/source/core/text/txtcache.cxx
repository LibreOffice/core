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

#include <txtfrm.hxx>
#include "porlay.hxx"

#include <sfx2/viewsh.hxx>
#include <osl/diagnose.h>
#include <view.hxx>

SwParaPortion *SwTextFrame::EnsurePara()
{
    if (!m_xParaPortion)
    {
        m_xParaPortion = std::make_shared<SwParaPortion>();
    }
    return m_xParaPortion.get();
}

void SwTextFrame::ClearPara()
{
    OSL_ENSURE( !IsLocked(), "+SwTextFrame::ClearPara: this is locked." );
    if ( !IsLocked() && m_xParaPortion )
    {
        m_xParaPortion.reset();
    }
}

void SwTextFrame::RemoveFromCache()
{
    m_xParaPortion.reset();
}

std::shared_ptr<SwParaPortion> SwTextFrame::SetPara(std::shared_ptr<SwParaPortion> xNew)
{
    std::shared_ptr<SwParaPortion> xOld;

    if ( m_xParaPortion )
        xOld = std::move(m_xParaPortion);

    if (xNew)
        m_xParaPortion = std::move(xNew);

    return xOld;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
