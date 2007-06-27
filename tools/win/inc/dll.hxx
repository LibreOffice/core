/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dll.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 22:20:57 $
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

#ifndef _DLL_HXX
#define _DLL_HXX

#ifdef WIN

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

// ----------------------
// - Zugriffsfunktionen -
// ----------------------

struct SVDATA;

extern "C"
{
// IN APPDATA.ASM
SVDATA* FAR PASCAL GetSVData();
}

// IN TOOLSDLL.CXX
void SetSVData( SVDATA* pSVData );

#endif

// -------------------------------
// - Sonstige Funktionen fuer SV -
// -------------------------------

// Um Resourcen wieder freizugeben
inline void ImpDeInitWinTools() {}

#endif /* _DLL_HXX */
