/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbtree.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 15:17:40 $
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
#ifndef _DBTREE_HXX
#define _DBTREE_HXX

#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

namespace com{namespace sun{namespace star{namespace container{
    class XNameAccess;
}}}}

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWUNODEF_HXX
#include <swunodef.hxx>
#endif

class SwDBTreeList_Impl;
class SwWrtShell;

class SW_DLLPUBLIC SwDBTreeList : public SvTreeListBox
{
    ImageList       aImageList;
    ImageList       aImageListHC;
    Image           aDBBMP;
    Image           aTableBMP;
    Image           aQueryBMP;

    String          sDefDBName;
    BOOL            bInitialized;
    BOOL            bShowColumns;

    SwDBTreeList_Impl* pImpl;

    SW_DLLPRIVATE DECL_LINK( DBCompare, SvSortData* );

    SW_DLLPRIVATE void          InitTreeList();
    SW_DLLPRIVATE virtual void  RequestingChilds( SvLBoxEntry* pParent );

    SW_DLLPRIVATE virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );
    SW_DLLPRIVATE virtual void  StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    using SvTreeListBox::Select;

public:
     SwDBTreeList( Window* pParent, const ResId& rResId,
                    SwWrtShell* pSh,
                    const String& rDefDBName = aEmptyStr,
                    const BOOL bShowCol = FALSE );
    virtual ~SwDBTreeList();

    String  GetDBName( String& rTableName, String& rColumnName, BOOL* pbIsTable = 0);

    void    Select( const String& rDBName, const String& rTableName,
                    const String& rColumnName );

    void    ShowColumns(BOOL bShowCol);
    void    SetWrtShell(SwWrtShell& rSh);

    void    AddDataSource(const String& rSource);
};

#endif

