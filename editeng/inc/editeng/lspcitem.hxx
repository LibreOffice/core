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
#ifndef _SVX_LSPCITEM_HXX
#define _SVX_LSPCITEM_HXX

// include ---------------------------------------------------------------

#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxLineSpacingItem ----------------------------------------------

/*  [Description]
    This item describes the distance between the lines.
*/

#define LINE_SPACE_DEFAULT_HEIGHT 200
class EDITENG_DLLPUBLIC SvxLineSpacingItem : public SfxEnumItemInterface
{
    friend SvStream& operator<<( SvStream&, SvxLineSpacingItem& ); //$ ostream

    short nInterLineSpace;
    USHORT nLineHeight;
    BYTE nPropLineSpace;
    SvxLineSpace eLineSpace;
    SvxInterLineSpace eInterLineSpace;

public:
    TYPEINFO();

    // The writer relies on a default height of 200! Actually, I would
    // initialize all values to 0, but who can ignore the consequences in
    // writer? => Rather have a crooked vales as the default, but the
    // programmer sees that there's something special happening.

    SvxLineSpacingItem( USHORT nHeight /*= LINE_SPACE_DEFAULT_HEIGHT*/, const USHORT nId  );

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT) const;
    virtual SvStream&        Store(SvStream &, USHORT nItemVersion ) const;

    // Methods to query and edit. InterlineSpace is added to the height.
    inline short GetInterLineSpace() const { return nInterLineSpace; }
    inline void SetInterLineSpace( const short nSpace )
    {
        nInterLineSpace = nSpace;
        eInterLineSpace = SVX_INTER_LINE_SPACE_FIX;
    }

    // Determines the absolute or minimum row height.
    inline USHORT GetLineHeight() const { return nLineHeight; }
    inline void SetLineHeight( const USHORT nHeight )
    {
        nLineHeight = nHeight;
        eLineSpace = SVX_LINE_SPACE_MIN;
    }

    // To increase or decrease the row height.
    BYTE GetPropLineSpace() const { return nPropLineSpace; }
    inline void SetPropLineSpace( const BYTE nProp )
    {
        nPropLineSpace = nProp;
        eInterLineSpace = SVX_INTER_LINE_SPACE_PROP;
    }

    inline SvxLineSpace &GetLineSpaceRule() { return eLineSpace; }
    inline SvxLineSpace GetLineSpaceRule() const { return eLineSpace; }

    inline SvxInterLineSpace &GetInterLineSpaceRule() { return eInterLineSpace; }
    inline SvxInterLineSpace GetInterLineSpaceRule() const { return eInterLineSpace; }

    virtual USHORT          GetValueCount() const;
    virtual String          GetValueTextByPos( USHORT nPos ) const;
    virtual USHORT          GetEnumValue() const;
    virtual void            SetEnumValue( USHORT nNewVal );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
