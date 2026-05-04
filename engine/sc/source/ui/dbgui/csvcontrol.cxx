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

#include <csvcontrol.hxx>
#include <osl/diagnose.h>

ScCsvLayoutData::ScCsvLayoutData() :
    mnPosCount( 1 ),
    mnNoRepaint( 0 )
{
}

ScCsvControl::ScCsvControl(const ScCsvLayoutData& rData)
    : mrData(rData)
{
}

ScCsvControl::~ScCsvControl()
{
}

// repaint helpers ------------------------------------------------------------

void ScCsvControl::Repaint()
{
    if( !IsNoRepaint() )
        Execute( CSVCMD_REPAINT );
}

void ScCsvControl::DisableRepaint()
{
    ++mrData.mnNoRepaint;
}

void ScCsvControl::EnableRepaint()
{
    OSL_ENSURE( IsNoRepaint(), "ScCsvControl::EnableRepaint - invalid call" );
    --mrData.mnNoRepaint;
    Repaint();
}

// command handling -----------------------------------------------------------

void ScCsvControl::Execute( ScCsvCmdType eType, sal_Int32 nParam1, sal_Int32 nParam2 )
{
    maCmd.Set( eType, nParam1, nParam2 );
    maCmdHdl.Call( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
