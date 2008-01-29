/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: userdat.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:21:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
            DBG_ERROR("MakeUserData: falsche ID");
        }
    }
    return 0;
}
IMPL_LINK_INLINE_END( ScDrawObjFactory, MakeUserData, SdrObjFactory *, pObjFactory )

//------------------------------------------------------------------------

ScDrawObjData::ScDrawObjData() : SdrObjUserData( SC_DRAWLAYER, SC_UD_OBJDATA, 0 )
{
    bValidEnd = FALSE;
}

ScDrawObjData::ScDrawObjData( const ScDrawObjData& r )
             : SdrObjUserData( r ), aStt( r.aStt ), aEnd( r.aEnd ),
               bValidStart( r.bValidStart ), bValidEnd( r.bValidEnd )
{}

ScDrawObjData::~ScDrawObjData()
{}

SdrObjUserData* ScDrawObjData::Clone(SdrObject*) const
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

