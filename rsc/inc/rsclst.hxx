/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rsclst.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:32:39 $
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

#ifndef _RSCLST_HXX
#define _RSCLST_HXX

#ifndef _TOOLS_LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _RSCALL_H
#include <rscall.h>
#endif

class REResourceList;

DECLARE_LIST( RESubResourceList, REResourceList * )

class REResourceList : public RESubResourceList
{
protected:
    REResourceList*  pParent;
    RscId            aRscId;    //Id und Name des Resourceobjektes
    ByteString       aClassName;
    BOOL             bVisible;

public:
                     REResourceList();
                     REResourceList( REResourceList * pParentList,
                                     ByteString& rClassName,
                                     const RscId & rResourceID,
                                     BOOL   bVisible = FALSE );
                     ~REResourceList();

    REResourceList*  GetParent()     { return pParent; }
    ByteString       GetObjName()    { return aRscId.GetName(); }
    ByteString       GetClassName()  { return aClassName; }
    RscId            GetRscId()      { return aRscId; }
    void             SetRscId( const RscId & rId ){ aRscId = rId; }

    void             SetVisible( BOOL bVis )
                                     { bVisible = bVis; }
    BOOL             IsVisible()     { return bVisible; }
};

#endif // _RSCLST_HXX
