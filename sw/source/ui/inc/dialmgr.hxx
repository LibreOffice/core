/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dialmgr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:09:50 $
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
#ifndef _SW_DIALMGR_HXX
#define _SW_DIALMGR_HXX

// include ---------------------------------------------------------------

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif

// forward ---------------------------------------------------------------

class ResMgr;
struct SwDialogsResMgr
{
    static ResMgr*      GetResMgr();
};

#define SWDIALOG_MGR()  SwDialogsResMgr::GetResMgr()
#define SW_RES(i)       ResId(i,SWDIALOG_MGR())
#define SW_RESSTR(i)    UniString(ResId(i,SWDIALOG_MGR()))
#define SW_RESSSTR(i)   String(ResId(i,SWDIALOG_MGR()))

#endif

