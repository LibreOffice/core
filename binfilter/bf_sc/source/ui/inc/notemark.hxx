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

#ifndef SC_NOTEMARK_HXX
#define SC_NOTEMARK_HXX

#ifndef _MAPMOD_HXX //autogen
#include <vcl/mapmod.hxx>
#endif
#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
namespace binfilter {

class ScNoteMarker
{
public:
    ScNoteMarker( Window* pWin,							//STRIP001 				ScNoteMarker( Window* pWin,
        Window* pRight, Window* pBottom, Window* pDiagonal,//STRIP001 								Window* pRight, Window* pBottom, Window* pDiagonal,
        ScDocument* pD, ScAddress aPos,						//STRIP001 								ScDocument* pD, ScAddress aPos,
        const String& rUser, const MapMode& rMap,			//STRIP001 								const String& rUser, const MapMode& rMap,
        BOOL bLeftEdge, BOOL bForce, BOOL bKeyboard ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 								BOOL bLeftEdge, BOOL bForce, BOOL bKeyboard );
};



} //namespace binfilter
#endif

