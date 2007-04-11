/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: writingmodeitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:48:17 $
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
#ifndef _SVX_WRITINGMODEITEM_HXX
#define _SVX_WRITINGMODEITEM_HXX

// include ---------------------------------------------------------------

#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE_HPP_
#include <com/sun/star/text/WritingMode.hpp>
#endif

#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

#ifndef _SVDDEF_HXX
#include <svx/svddef.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class SvxWritingModeItem ----------------------------------------------

class SVX_DLLPUBLIC SvxWritingModeItem : public SfxUInt16Item
{
public:
    TYPEINFO();

    SvxWritingModeItem( ::com::sun::star::text::WritingMode eValue = com::sun::star::text::WritingMode_LR_TB, USHORT nWhich = SDRATTR_TEXTDIRECTION );
    virtual ~SvxWritingModeItem();

    SvxWritingModeItem& operator=( const SvxWritingModeItem& rItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT) const;
    virtual SvStream&       Store(SvStream & rStrm, USHORT nIVer) const;
    virtual USHORT          GetVersion( USHORT nFileVersion ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper * = 0 ) const;

    virtual sal_Bool PutValue( const com::sun::star::uno::Any& rVal,
                                    BYTE nMemberId );
    virtual sal_Bool QueryValue( com::sun::star::uno::Any& rVal,
                                BYTE nMemberId ) const;
};

#endif // #ifndef _SVX_WRITINGMODEITEM_HXX

