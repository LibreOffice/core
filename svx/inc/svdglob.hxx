/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdglob.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 12:47:18 $
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

#ifndef _SVDGLOB_HXX
#define _SVDGLOB_HXX

#ifndef _CONTNR_HXX //autogen
#include <tools/contnr.hxx>
#endif

#ifndef _RESID_HXX //autogen
#include <tools/resid.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

//************************************************************
//   Vorausdeklarationen
//************************************************************

class Link;
class ResMgr;
class SdrEngineDefaults;
class ImpSdrStrCache;

//************************************************************
//   Defines
//************************************************************

// Den zur App passenden Resourcemanager holen
ResMgr* ImpGetResMgr();

class SdrResId: ResId {
public:
    SdrResId(USHORT nId): ResId(nId,ImpGetResMgr()) {}
};

// ResourceCache fuer haeuffig benoetigte Strings
// gecachet werden globale String-Resourcen mit den
// IDs von SDR_StringCacheBegin (256) bis
// SDR_StringCacheEnd (weiss ich grad' nicht).
// Hierzu siehe auch SvdStr.Hrc
String ImpGetResStr(sal_uInt16 nResID);

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDGLOB_HXX

