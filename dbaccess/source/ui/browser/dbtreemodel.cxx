/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbtreemodel.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:56:55 $
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

#ifndef DBAUI_DBTREEMODEL_HXX
#include "dbtreemodel.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace dbaui
{
    //========================================================================
    //= DBTreeListModel
    //========================================================================
    DBG_NAME(DBTreeListUserData)
    //------------------------------------------------------------------------
    DBTreeListModel::DBTreeListUserData::DBTreeListUserData()
        :eType(SbaTableQueryBrowser::etQuery)
    {
        DBG_CTOR(DBTreeListUserData,NULL);
    }
    //------------------------------------------------------------------------
    DBTreeListModel::DBTreeListUserData::~DBTreeListUserData()
    {
        DBG_DTOR(DBTreeListUserData,NULL);
    }

    //------------------------------------------------------------------------
    sal_uInt16 DBTreeListModel::getImageResId(SbaTableQueryBrowser::EntryType _eType,
                                                sal_Bool _bHiContrast)
    {
        switch (_eType)
        {
            case SbaTableQueryBrowser::etTable:
                return _bHiContrast ? TABLE_TREE_ICON_SCH : TABLE_TREE_ICON;
            case SbaTableQueryBrowser::etView:
                return _bHiContrast ? VIEW_TREE_ICON_SCH : VIEW_TREE_ICON;
            case SbaTableQueryBrowser::etQuery:
                return _bHiContrast ? QUERY_TREE_ICON_SCH : QUERY_TREE_ICON;
            case SbaTableQueryBrowser::etDatasource:
                return _bHiContrast ? IMG_DATABASE_SCH : IMG_DATABASE;
            case SbaTableQueryBrowser::etQueryContainer:
                return _bHiContrast ? QUERYFOLDER_TREE_ICON_SCH : QUERYFOLDER_TREE_ICON;
            case SbaTableQueryBrowser::etTableContainer:
                return _bHiContrast ? TABLEFOLDER_TREE_ICON_SCH : TABLEFOLDER_TREE_ICON;
            case SbaTableQueryBrowser::etUnknown:
                break;
        }
        OSL_ENSURE(sal_False, "DBTreeListModel::getImageResId: invalid entry type!");
        return 0;
    }

}
