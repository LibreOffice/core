/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: twolinesitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:42:52 $
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
#ifndef _SVX_TWOLINESITEM_HXX
#define _SVX_TWOLINESITEM_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

class SVX_DLLPUBLIC SvxTwoLinesItem : public SfxPoolItem
{
    sal_Unicode cStartBracket, cEndBracket;
    sal_Bool bOn;
public:
    TYPEINFO();
    SvxTwoLinesItem( sal_Bool bOn = TRUE,
                     sal_Unicode nStartBracket = 0,
                     sal_Unicode nEndBracket = 0,
                     sal_uInt16 nId = ITEMID_TWOLINES );
    SvxTwoLinesItem( const SvxTwoLinesItem& rAttr );
    virtual ~SvxTwoLinesItem();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVer) const;
    virtual SvStream&       Store(SvStream &, USHORT nIVer) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 );

    virtual USHORT          GetVersion( USHORT nFFVer ) const;

    SvxTwoLinesItem&        operator=( const SvxTwoLinesItem& rCpy )
    {
        SetValue( rCpy.GetValue() );
        SetStartBracket( rCpy.GetStartBracket() );
        SetEndBracket( rCpy.GetEndBracket() );
        return *this;
    }

    sal_Bool GetValue() const                   { return bOn; }
    void SetValue( sal_Bool bFlag )             { bOn = bFlag; }

    sal_Unicode GetStartBracket() const         { return cStartBracket; }
    void SetStartBracket( sal_Unicode c )       { cStartBracket = c; }

    sal_Unicode GetEndBracket() const           { return cEndBracket; }
    void SetEndBracket( sal_Unicode c )         { cEndBracket = c; }
};

#endif

