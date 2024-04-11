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

#include <tools/long.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/sheet/MemberResult.hpp>

using namespace css;

struct ScDPOutLevelData
{
    tools::Long mnDim;
    tools::Long mnHier;
    tools::Long mnLevel;
    tools::Long mnDimPos;
    sal_uInt32 mnSrcNumFmt; /// Prevailing number format used in the source data.
    uno::Sequence<sheet::MemberResult> maResult;
    OUString maName; /// Name is the internal field name.
    OUString maCaption; /// Caption is the name visible in the output table.
    bool mbHasHiddenMember : 1;
    bool mbDataLayout : 1;
    bool mbPageDim : 1;

    ScDPOutLevelData(tools::Long nDim, tools::Long nHier, tools::Long nLevel, tools::Long nDimPos,
                     sal_uInt32 nSrcNumFmt, const uno::Sequence<sheet::MemberResult>& aResult,
                     OUString aName, OUString aCaption, bool bHasHiddenMember, bool bDataLayout,
                     bool bPageDim)
        : mnDim(nDim)
        , mnHier(nHier)
        , mnLevel(nLevel)
        , mnDimPos(nDimPos)
        , mnSrcNumFmt(nSrcNumFmt)
        , maResult(aResult)
        , maName(std::move(aName))
        , maCaption(std::move(aCaption))
        , mbHasHiddenMember(bHasHiddenMember)
        , mbDataLayout(bDataLayout)
        , mbPageDim(bPageDim)
    {
    }

    // bug (73840) in uno::Sequence - copy and then assign doesn't work!
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
