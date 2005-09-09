/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numberingtypelistbox.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:52:52 $
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
#ifndef _NUMBERINGTYPELISTBOX_HXX
#define _NUMBERINGTYPELISTBOX_HXX

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

#define INSERT_NUM_TYPE_NO_NUMBERING                0x01
#define INSERT_NUM_TYPE_PAGE_STYLE_NUMBERING        0x02
#define INSERT_NUM_TYPE_BITMAP                      0x04
#define INSERT_NUM_TYPE_BULLET                      0x08
#define INSERT_NUM_EXTENDED_TYPES                   0x10

struct SwNumberingTypeListBox_Impl;

class SW_DLLPUBLIC SwNumberingTypeListBox : public ListBox
{
    SwNumberingTypeListBox_Impl* pImpl;

public:
    SwNumberingTypeListBox( Window* pWin, const ResId& rResId,
        USHORT nTypeFlags = INSERT_NUM_TYPE_PAGE_STYLE_NUMBERING|INSERT_NUM_TYPE_NO_NUMBERING|INSERT_NUM_EXTENDED_TYPES );
    ~SwNumberingTypeListBox();

    void        Reload(USHORT nTypeFlags);

    sal_Int16   GetSelectedNumberingType();
    sal_Bool    SelectNumberingType(sal_Int16 nType);
};


#endif

