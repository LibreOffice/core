/*************************************************************************
 *
 *  $RCSfile: navicfg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include <tools/stream.hxx>

#include "navicfg.hxx"
#include "cfgids.hxx"
#include "scresid.hxx"
#include "sc.hrc"

#define SC_NAVICFG_VERSION  0

//------------------------------------------------------------------

ScNavipiCfg::ScNavipiCfg() :
    SfxConfigItem( SCCFG_NAVIPI ),
    nListMode(0),
    nDragMode(0),
    nRootType(0)
{
}

//------------------------------------------------------------------------

int __EXPORT ScNavipiCfg::Load( SvStream& rStream )
{
    SetDefault(FALSE);

    USHORT nVer;
    rStream >> nVer;
    if ( nVer != SC_NAVICFG_VERSION )
        return SfxConfigItem::WARNING_VERSION;

    rStream >> nListMode;
    rStream >> nDragMode;
    rStream >> nRootType;

    return SfxConfigItem::ERR_OK;
}

//------------------------------------------------------------------------

BOOL __EXPORT ScNavipiCfg::Store( SvStream& rStream)
{
    rStream << (USHORT) SC_NAVICFG_VERSION;

    rStream << nListMode;
    rStream << nDragMode;
    rStream << nRootType;

//? SetDefault( FALSE );
    return SfxConfigItem::ERR_OK;
}

//------------------------------------------------------------------------

void __EXPORT ScNavipiCfg::UseDefault()
{
    nListMode = nDragMode = nRootType = 0;

    SetDefault( TRUE );
}


//------------------------------------------------------------------------

void ScNavipiCfg::SetListMode(USHORT nNew)
{
    if ( nListMode != nNew )
    {
        nListMode = nNew;
        SetDefault(FALSE);
    }
}

void ScNavipiCfg::SetDragMode(USHORT nNew)
{
    if ( nDragMode != nNew )
    {
        nDragMode = nNew;
        SetDefault(FALSE);
    }
}

void ScNavipiCfg::SetRootType(USHORT nNew)
{
    if ( nRootType != nNew )
    {
        nRootType = nNew;
        SetDefault(FALSE);
    }
}

//------------------------------------------------------------------------

String __EXPORT ScNavipiCfg::GetName() const
{
    return String( ScResId( SCSTR_CFG_NAVIPI ) );
}




