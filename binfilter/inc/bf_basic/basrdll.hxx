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

#ifndef _BASRDLL_HXX
#define _BASRDLL_HXX

class ResMgr;

#ifndef _SV_ACCEL_HXX //autogen
#include <vcl/accel.hxx>
#endif

namespace binfilter {

class BasicDLL
{
private:
    ResMgr*		pResMgr;
    BOOL		bDebugMode;
    BOOL		bBreakEnabled;

public:
                BasicDLL();
                ~BasicDLL();

    ResMgr*		GetResMgr() const { return pResMgr; }

    static void BasicBreak();

    static void	EnableBreak( BOOL bEnable );
    static void	SetDebugMode( BOOL bDebugMode );
};

/*?*/ // #define BASIC_DLL() (*(BasicDLL**)GetAppData( SHL_BASIC ) )

}

#endif //_BASRDLL_HXX
