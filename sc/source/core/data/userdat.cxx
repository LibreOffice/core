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

#include <userdat.hxx>
#include <drwlayer.hxx>
#include <rechead.hxx>


ScDrawObjData::ScDrawObjData() :
    SdrObjUserData( SdrInventor::ScOrSwDraw, SC_UD_OBJDATA ),
    maStart( ScAddress::INITIALIZE_INVALID ),
    maEnd( ScAddress::INITIALIZE_INVALID ),
    meType( DrawingObject ),
    mbResizeWithCell( false )
{
}

ScDrawObjData* ScDrawObjData::Clone( SdrObject* ) const
{
    return new ScDrawObjData( *this );
}

ScIMapInfo::ScIMapInfo( const ImageMap& rImageMap ) :
    SdrObjUserData( SdrInventor::ScOrSwDraw, SC_UD_IMAPDATA ),
    aImageMap( rImageMap )
{
}

ScIMapInfo::ScIMapInfo( const ScIMapInfo& rIMapInfo ) :
    SdrObjUserData( rIMapInfo ),
    aImageMap( rIMapInfo.aImageMap )
{
}

ScIMapInfo::~ScIMapInfo()
{
}

SdrObjUserData* ScIMapInfo::Clone( SdrObject* ) const
{
    return new ScIMapInfo( *this );
}

ScMacroInfo::ScMacroInfo() :
    SdrObjUserData( SdrInventor::ScOrSwDraw, SC_UD_MACRODATA )
{
}

ScMacroInfo::~ScMacroInfo()
{
}

SdrObjUserData* ScMacroInfo::Clone( SdrObject* /*pObj*/ ) const
{
   return new ScMacroInfo( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
