/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: msgpool.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:37:35 $
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

#ifndef SC_MSGPOOL_HXX
#define SC_MSGPOOL_HXX

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif


#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

//#include <dbitems.hxx>
#include "uiitems.hxx"


//------------------------------------------------------------------------

class ScDocumentPool;

//------------------------------------------------------------------------

class ScMessagePool: public SfxItemPool
{
    SfxStringItem       aGlobalStringItem;
    SvxSearchItem       aGlobalSearchItem;
    ScSortItem          aGlobalSortItem;
    ScQueryItem         aGlobalQueryItem;
    ScSubTotalItem      aGlobalSubTotalItem;
    ScConsolidateItem   aGlobalConsolidateItem;
    ScPivotItem         aGlobalPivotItem;
    ScSolveItem         aGlobalSolveItem;
    ScUserListItem      aGlobalUserListItem;

    SfxBoolItem         aPrintWarnItem;

    SfxPoolItem**   ppPoolDefaults;
    ScDocumentPool* pDocPool;

public:
    ScMessagePool();
    ~ScMessagePool();

    virtual SfxMapUnit              GetMetric( USHORT nWhich ) const;
};



#endif

