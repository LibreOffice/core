/*************************************************************************
 *
 *  $RCSfile: dbtreemodel.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2001-08-16 14:22:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    sal_uInt16 DBTreeListModel::getImageResId(SbaTableQueryBrowser::EntryType _eType)
    {
        switch (_eType)
        {
            case SbaTableQueryBrowser::etTable:
                return TABLE_TREE_ICON;
            case SbaTableQueryBrowser::etView:
                return VIEW_TREE_ICON;
            case SbaTableQueryBrowser::etQuery:
                return QUERY_TREE_ICON;
            case SbaTableQueryBrowser::etBookmark:
                return BOOKMARK_TREE_ICON;
        }
        OSL_ENSURE(sal_False, "DBTreeListModel::getImageResId: invalid entry type!");
        return 0;
    }

};

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.9  2001/08/15 13:14:59  oj
 *  #88644# correct getImplementationId and insert some DBG's
 *
 *  Revision 1.8  2001/05/14 11:58:35  oj
 *  #86744# some changes for entries and views
 *
 *  Revision 1.7  2001/04/26 11:37:51  fs
 *  file is alive, again - added support for data source associated bookmarks
 *
 *
 *  Revision 1.0 25.04.01 14:00:45  fs
 ************************************************************************/

