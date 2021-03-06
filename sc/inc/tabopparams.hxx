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

#include "address.hxx"

#include <vector>

class ScFormulaCell;

struct ScInterpreterTableOpParams
{
    ScAddress   aOld1;
    ScAddress   aNew1;
    ScAddress   aOld2;
    ScAddress   aNew2;
    ScAddress   aFormulaPos;
    ::std::vector< ScFormulaCell* > aNotifiedFormulaCells;
    ::std::vector< ScAddress >      aNotifiedFormulaPos;
    bool        bValid;
    bool        bRefresh;
    bool        bCollectNotifications;

    ScInterpreterTableOpParams()
            : bValid( false )
            , bRefresh( false )
            , bCollectNotifications( true )
    {
    }

    ScInterpreterTableOpParams( const ScInterpreterTableOpParams& r )
            : aOld1(                 r.aOld1 )
            , aNew1(                 r.aNew1 )
            , aOld2(                 r.aOld2 )
            , aNew2(                 r.aNew2 )
            , aFormulaPos(           r.aFormulaPos )
            , aNotifiedFormulaPos(   r.aNotifiedFormulaPos )
            , bValid(                r.bValid )
            , bRefresh(              r.bRefresh )
            , bCollectNotifications( r.bCollectNotifications )
    {
    }

    ScInterpreterTableOpParams& operator =( const ScInterpreterTableOpParams& r )
    {
        aOld1                 = r.aOld1;
        aNew1                 = r.aNew1;
        aOld2                 = r.aOld2;
        aNew2                 = r.aNew2;
        aFormulaPos           = r.aFormulaPos;
        ::std::vector< ScFormulaCell* >().swap( aNotifiedFormulaCells );
        aNotifiedFormulaPos   = r.aNotifiedFormulaPos;
        bValid                = r.bValid;
        bRefresh              = r.bRefresh;
        bCollectNotifications = r.bCollectNotifications;
        return *this;
    }

    bool operator ==( const ScInterpreterTableOpParams& r )
    {
        return
            bValid && r.bValid &&
            aOld1       == r.aOld1 &&
            aOld2       == r.aOld2 &&
            aFormulaPos == r.aFormulaPos ;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
