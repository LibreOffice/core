/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// -----------------------------------------------------------------------

#include "userdat.hxx"
#include <tools/debug.hxx>
#include "drwlayer.hxx"
#include "rechead.hxx"
#include <svx/sdrobjectfactory.hxx>

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
    if ( SC_DRAWLAYER == pObjFactory->getSdrObjectCreationInfo().getInvent() )
    {
        if ( pObjFactory->getSdrObjectCreationInfo().getIdent() == SC_UD_OBJDATA )
        {
            pObjFactory->setNewSdrObjUserData(new ScDrawObjData);
        }
        else if ( pObjFactory->getSdrObjectCreationInfo().getIdent() == SC_UD_IMAPDATA )
        {
            pObjFactory->setNewSdrObjUserData(new ScIMapInfo);
        }
        else if ( pObjFactory->getSdrObjectCreationInfo().getIdent() == SC_UD_MACRODATA )
        {
            pObjFactory->setNewSdrObjUserData(new ScMacroInfo);
        }
        else
        {
            DBG_ERROR("MakeUserData: falsche ID");
        }
    }
    return 0;
}
IMPL_LINK_INLINE_END( ScDrawObjFactory, MakeUserData, SdrObjFactory *, pObjFactory )

//------------------------------------------------------------------------

ScDrawObjData::ScDrawObjData() :
    SdrObjUserData( SC_DRAWLAYER, SC_UD_OBJDATA ),
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
    SdrObjUserData( SC_DRAWLAYER, SC_UD_IMAPDATA )
{
}

ScIMapInfo::ScIMapInfo( const ImageMap& rImageMap ) :
    SdrObjUserData( SC_DRAWLAYER, SC_UD_IMAPDATA ),
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
    SdrObjUserData( SC_DRAWLAYER, SC_UD_MACRODATA )
{
}

ScMacroInfo::~ScMacroInfo()
{
}

SdrObjUserData* ScMacroInfo::Clone( SdrObject* /*pObj*/ ) const
{
   return new ScMacroInfo( *this );
}

