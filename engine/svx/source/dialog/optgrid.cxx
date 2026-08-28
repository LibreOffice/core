/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <svl/intitem.hxx>
#include <svtools/unitconv.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <officecfg/Office/Writer.hxx>
#include <officecfg/Office/Calc.hxx>
#include <officecfg/Office/WriterWeb.hxx>
#include <officecfg/Office/Impress.hxx>
#include <officecfg/Office/Draw.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <rtl/ustrbuf.hxx>

#include <svx/svxids.hrc>
#include <svx/optgrid.hxx>
#include <svx/dlgutil.hxx>

SvxOptionsGrid::SvxOptionsGrid() :
    m_nFldDrawX       ( 100 ),
    m_nFldDivisionX   ( 0 ),
    m_nFldDrawY       ( 100 ),
    m_nFldDivisionY   ( 0 ),
    m_bUseGridsnap    ( false ),
    m_bSynchronize    ( true ),
    m_bGridVisible    ( false )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
