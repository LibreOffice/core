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

#ifndef SC_MSGPOOL_HXX
#define SC_MSGPOOL_HXX

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif

#ifndef _SFXITEMPOOL_HXX //autogen
#include <bf_svtools/itempool.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <bf_svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
#ifndef _SFX_SRCHITEM_HXX
#include <bf_sfx2/srchitem.hxx>
#endif

#include "uiitems.hxx"
namespace binfilter {


//------------------------------------------------------------------------

class ScDocumentPool;

//------------------------------------------------------------------------

class ScMessagePool: public SfxItemPool
{
    SfxStringItem		aGlobalStringItem;
    SvxSearchItem		aGlobalSearchItem;
    ScSortItem			aGlobalSortItem;
    ScQueryItem			aGlobalQueryItem;
    ScSubTotalItem		aGlobalSubTotalItem;
    ScConsolidateItem	aGlobalConsolidateItem;
    ScPivotItem			aGlobalPivotItem;
    ScSolveItem			aGlobalSolveItem;
    ScUserListItem		aGlobalUserListItem;

    SfxBoolItem			aPrintWarnItem;

    SfxPoolItem**	ppPoolDefaults;
    ScDocumentPool*	pDocPool;

public:
    ScMessagePool();
    ~ScMessagePool();

    virtual SfxMapUnit				GetMetric( USHORT nWhich ) const;
};



} //namespace binfilter
#endif

