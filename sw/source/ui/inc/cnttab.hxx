/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cnttab.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-12 16:31:56 $
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
#ifndef _CNTTAB_HXX
#define _CNTTAB_HXX

#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#include "tox.hxx"
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _TOXMGR_HXX
#include <toxmgr.hxx>
#endif
#ifndef _SVX_CHECKLBX_HXX //autogen wg. SvxCheckListBox
#include <svx/checklbx.hxx>
#endif
#ifndef _TOOLS_RESARY_HXX
#include <tools/resary.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SV_MENUBTN_HXX
#include <vcl/menubtn.hxx>
#endif
#ifndef _SVX_LANGBOX_HXX
#include <svx/langbox.hxx>
#endif
#define TOX_PAGE_SELECT 1
#define TOX_PAGE_ENTRY  2
#define TOX_PAGE_STYLES 3

struct CurTOXType
{
    TOXTypes    eType;
    USHORT      nIndex; //for TOX_USER only

    BOOL operator==(const CurTOXType aCmp)
       {
        return eType == aCmp.eType && nIndex == aCmp.nIndex;
       }
    USHORT GetFlatIndex() const;

    CurTOXType () : eType (TOX_INDEX), nIndex (0) {};

    CurTOXType (TOXTypes t, USHORT i) : eType (t), nIndex (i) {};
};

class SwOLENames : public Resource
{
    ResStringArray      aNamesAry;
public:
    SwOLENames(const ResId& rResId) :
        Resource(rResId),
        aNamesAry(ResId(1,*rResId.GetResMgr())){FreeResource();}

    ResStringArray&     GetNames() { return aNamesAry;}

};

#endif
