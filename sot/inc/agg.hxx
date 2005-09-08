/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: agg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:33:55 $
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

#ifndef _SOT_AGG_HXX
#define _SOT_AGG_HXX

#ifndef _TOOLS_OWNLIST_HXX
#include <tools/ownlist.hxx>
#endif

/************** class SvAggregate ***************************************/
/************************************************************************/
class SotFactory;
class SotObject;
struct SvAggregate
{
    union
    {
        SotFactory * pFact;
        SotObject * pObj;
    };
    BOOL    bFactory;
    BOOL    bMainObj; // TRUE, das Objekt, welches das casting steuert

    SvAggregate()
        : bFactory( FALSE )
        , bMainObj( FALSE )
        , pFact( NULL ) {}
    SvAggregate( SotObject * pObjP, BOOL bMainP )
        : bFactory( FALSE )
        , bMainObj( bMainP )
        , pObj( pObjP ) {}
    SvAggregate( SotFactory * pFactP )
        : bFactory( TRUE )
        , bMainObj( FALSE )
        , pFact( pFactP ) {}
};

/************** class SvAggregateMemberList *****************************/
/************************************************************************/
class SvAggregateMemberList
{
     PRV_SV_DECL_OWNER_LIST(SvAggregateMemberList,SvAggregate)
};

#endif // _AGG_HXX
