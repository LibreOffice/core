/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// -----------------------------------------------------------------------

#include "userdat.hxx"
#include <tools/debug.hxx>
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
            OSL_FAIL("MakeUserData: falsche ID");
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
    mbNote( false )
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
