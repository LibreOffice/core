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

#include "userdat.hxx"
#include "drwlayer.hxx"
#include "rechead.hxx"

// -----------------------------------------------------------------------

ScDrawObjFactory::ScDrawObjFactory()
{
    SdrObjFactory::InsertMakeUserDataHdl( LINK ( this, ScDrawObjFactory, MakeUserData ) );
}

ScDrawObjFactory::~ScDrawObjFactory()
{
    SdrObjFactory::RemoveMakeUserDataHdl( LINK ( this, ScDrawObjFactory, MakeUserData ) );
}

IMPL_LINK_INLINE_START( ScDrawObjFactory, MakeUserData, SdrObjFactory *, pObjFactory )
{
    if ( pObjFactory->nInventor == SC_DRAWLAYER )
    {
        if ( pObjFactory->nIdentifier == SC_UD_OBJDATA )
            pObjFactory->pNewData = new ScDrawObjData;
        else if ( pObjFactory->nIdentifier == SC_UD_IMAPDATA )
            pObjFactory->pNewData = new ScIMapInfo;
        else if ( pObjFactory->nIdentifier == SC_UD_MACRODATA )
            pObjFactory->pNewData = new ScMacroInfo;
        else
        {
            OSL_FAIL("MakeUserData: wrong ID");
        }
    }
    return 0;
}
IMPL_LINK_INLINE_END( ScDrawObjFactory, MakeUserData, SdrObjFactory *, pObjFactory )

//------------------------------------------------------------------------

ScDrawObjData::ScDrawObjData() :
    SdrObjUserData( SC_DRAWLAYER, SC_UD_OBJDATA, 0 ),
    maStart( ScAddress::INITIALIZE_INVALID ),
    maEnd( ScAddress::INITIALIZE_INVALID ),
    meType( DrawingObject )
{
}

ScDrawObjData* ScDrawObjData::Clone( SdrObject* ) const
{
    return new ScDrawObjData( *this );
}

//------------------------------------------------------------------------

ScIMapInfo::ScIMapInfo() :
    SdrObjUserData( SC_DRAWLAYER, SC_UD_IMAPDATA, 0 )
{
}

ScIMapInfo::ScIMapInfo( const ImageMap& rImageMap ) :
    SdrObjUserData( SC_DRAWLAYER, SC_UD_IMAPDATA, 0 ),
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

//------------------------------------------------------------------------

ScMacroInfo::ScMacroInfo() :
    SdrObjUserData( SC_DRAWLAYER, SC_UD_MACRODATA, 0 )
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
