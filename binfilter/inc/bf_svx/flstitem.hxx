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
#ifndef _SVX_FLSTITEM_HXX
#define _SVX_FLSTITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXPOOLITEM_HXX //autogen
#include <bf_svtools/poolitem.hxx>
#endif
#ifndef   ITEMID_FONTLIST
#define   ITEMID_FONTLIST 0
#endif
namespace binfilter {
class FontList;

// class SvxFontListItem -------------------------------------------------


/*
    [Beschreibung]

    Dieses Item dient als Transport-Medium fuer eine FontListe.
    Die Fontliste wird nicht kopiert und nicht geloescht!
*/

class SvxFontListItem : public SfxPoolItem
{
private:
    const FontList*         pFontList;

public:
    TYPEINFO();

    SvxFontListItem( const FontList* pFontLst,
                     const USHORT nId = ITEMID_FONTLIST );
    SvxFontListItem( const SvxFontListItem& rItem );

    virtual int				operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;


    const FontList*			GetFontList() const { return pFontList; }
};

}//end of namespace binfilter
#endif

