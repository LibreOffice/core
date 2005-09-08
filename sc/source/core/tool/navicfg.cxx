/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: navicfg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:46:34 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "navicfg.hxx"

//------------------------------------------------------------------

//! #define CFGPATH_NAVIPI          "Office.Calc/Navigator"

//------------------------------------------------------------------

ScNavipiCfg::ScNavipiCfg() :
//! ConfigItem( OUString::createFromAscii( CFGPATH_NAVIPI ) ),
    nListMode(0),
    nDragMode(0),
    nRootType(0)
{
}

//------------------------------------------------------------------------

void ScNavipiCfg::SetListMode(USHORT nNew)
{
    if ( nListMode != nNew )
    {
        nListMode = nNew;
//!     SetModified();
    }
}

void ScNavipiCfg::SetDragMode(USHORT nNew)
{
    if ( nDragMode != nNew )
    {
        nDragMode = nNew;
//!     SetModified();
    }
}

void ScNavipiCfg::SetRootType(USHORT nNew)
{
    if ( nRootType != nNew )
    {
        nRootType = nNew;
//!     SetModified();
    }
}


