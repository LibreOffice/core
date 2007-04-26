/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basrdll.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 08:30:19 $
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

#ifndef _BASRDLL_HXX
#define _BASRDLL_HXX

class ResMgr;

#ifndef _SV_ACCEL_HXX //autogen
#include <vcl/accel.hxx>
#endif

class BasicDLL
{
private:
    ResMgr*     pSttResMgr;
    ResMgr*     pBasResMgr;

    BOOL        bDebugMode;
    BOOL        bBreakEnabled;

public:
                BasicDLL();
                ~BasicDLL();

    ResMgr*     GetSttResMgr() const { return pSttResMgr; }
    ResMgr*     GetBasResMgr() const { return pBasResMgr; }

    static void BasicBreak();

    static void EnableBreak( BOOL bEnable );
    static void SetDebugMode( BOOL bDebugMode );
};

#define BASIC_DLL() (*(BasicDLL**)GetAppData( SHL_BASIC ) )

#endif //_BASRDLL_HXX
