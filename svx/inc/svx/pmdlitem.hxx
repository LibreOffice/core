/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pmdlitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:05:19 $
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
#ifndef _SVX_PMDLITEM_HXX
#define _SVX_PMDLITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class SvxPageModelItem ------------------------------------------------

/*
[Beschreibung]
Dieses Item enthaelt einen Namen einer Seitenvorlage.
*/

class SVX_DLLPUBLIC SvxPageModelItem : public SfxStringItem
{
private:
    BOOL bAuto;

public:
    TYPEINFO();

    inline SvxPageModelItem( USHORT nWh = ITEMID_PAGEMODEL );
    inline SvxPageModelItem( const String& rModel, BOOL bA = FALSE,
                             USHORT nWh = ITEMID_PAGEMODEL );
    inline SvxPageModelItem& operator=( const SvxPageModelItem& rModel );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    BOOL IsAuto() const { return bAuto; }
};

inline SvxPageModelItem::SvxPageModelItem( USHORT nWh )
    : bAuto( FALSE )
{
    SetWhich( nWh );
}

inline SvxPageModelItem::SvxPageModelItem( const String& rModel, BOOL bA,
                                           USHORT nWh ) :
    SfxStringItem( nWh, rModel ),
    bAuto( bA )
{}

inline SvxPageModelItem& SvxPageModelItem::operator=( const SvxPageModelItem& rModel )
{
    SetValue( rModel.GetValue() );
    return *this;
}

#endif

