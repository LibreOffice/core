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
#ifndef _SVX_HLNKITEM_HXX
#define _SVX_HLNKITEM_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <bf_svtools/poolitem.hxx>
#endif
#ifndef _SFXSIDS_HRC
#include <bf_sfx2/sfxsids.hrc>
#endif

#ifndef _SFXMACITEM_HXX
#include <bf_svtools/macitem.hxx>
#endif
namespace binfilter {

#define HYPERDLG_EVENT_MOUSEOVER_OBJECT		0x0001
#define HYPERDLG_EVENT_MOUSECLICK_OBJECT	0x0002
#define HYPERDLG_EVENT_MOUSEOUT_OBJECT		0x0004

enum SvxLinkInsertMode
{
    HLINK_DEFAULT,
    HLINK_FIELD,
    HLINK_BUTTON,
    HLINK_HTMLMODE = 0x0080
};

class SvxHyperlinkItem : public SfxPoolItem
{
public:
    TYPEINFO();
    virtual int 			 operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;
};

}//end of namespace binfilter
#endif

