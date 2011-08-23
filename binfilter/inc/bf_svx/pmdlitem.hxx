/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SVX_PMDLITEM_HXX
#define _SVX_PMDLITEM_HXX

// include ---------------------------------------------------------------

#include <bf_svtools/stritem.hxx>
#include <bf_svx/svxids.hrc>
namespace binfilter {

// class SvxPageModelItem ------------------------------------------------

/*
[Beschreibung]
Dieses Item enthaelt einen Namen einer Seitenvorlage.
*/

class SvxPageModelItem : public SfxStringItem
{
private:
    BOOL bAuto;

public:
    TYPEINFO();

    inline SvxPageModelItem( USHORT nWhich = ITEMID_PAGEMODEL );
    inline SvxPageModelItem( const String& rModel, BOOL bA = FALSE,
                             USHORT nWhich = ITEMID_PAGEMODEL );
    inline SvxPageModelItem& operator=( const SvxPageModelItem& rModel );

};

inline SvxPageModelItem::SvxPageModelItem( USHORT nWhich )
    : bAuto( FALSE )
{
    SetWhich( nWhich );
}

inline SvxPageModelItem::SvxPageModelItem( const String& rModel, BOOL bA,
                                           USHORT nWhich ) :
    SfxStringItem( nWhich, rModel ),
    bAuto( bA )
{}

inline SvxPageModelItem& SvxPageModelItem::operator=( const SvxPageModelItem& rModel )
{
    SetValue( rModel.GetValue() );
    return *this;
}

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
