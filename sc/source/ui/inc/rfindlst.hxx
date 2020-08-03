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

#include <tools/color.hxx>
#include <address.hxx>
#include <tools/solar.h>

#include <vector>

struct ScRangeFindData
{
    ScRange    aRef;
    ScRefFlags nFlags;
    sal_Int32  nSelStart;
    sal_Int32  nSelEnd;
    Color      nColor;

    ScRangeFindData( const ScRange& rR, ScRefFlags nF, sal_Int32 nS, sal_Int32 nE ) :
        aRef(rR), nFlags(nF), nSelStart(nS), nSelEnd(nE) {}
};

class ScRangeFindList
{
    std::vector<ScRangeFindData> maEntries;
    OUString    aDocName;
    bool        bHidden;
    sal_uInt16  nIndexColor;

public:
                     ScRangeFindList(const OUString& rName);

    sal_uLong        Count() const                       { return maEntries.size(); }
    Color            Insert( const ScRangeFindData &rNew );

    ScRangeFindData& GetObject( sal_uLong nIndex ) { return maEntries[nIndex]; }

    void             SetHidden( bool bSet ) { bHidden = bSet; }

    const OUString&  GetDocName() const { return aDocName; }
    bool             IsHidden() const   { return bHidden; }

    static Color     GetColorName(const size_t nIndex);
    Color            FindColor(const ScRange& rRef, const size_t nIndex);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
