/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pageitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:04:10 $
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
#ifndef _SVX_PAGEITEM_HXX
#define _SVX_PAGEITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif


/*--------------------------------------------------------------------
    Beschreibung:   SvxNumType
 --------------------------------------------------------------------*/

enum SvxNumType
{
    SVX_CHARS_UPPER_LETTER,
    SVX_CHARS_LOWER_LETTER,
    SVX_ROMAN_UPPER,
    SVX_ROMAN_LOWER,
    SVX_ARABIC,
    SVX_NUMBER_NONE,
    SVX_CHAR_SPECIAL,
    SVX_PAGEDESC
};

/*--------------------------------------------------------------------
    Beschreibung:   Benutzung der Seite
 --------------------------------------------------------------------*/

enum SvxPageUsage
{
    SVX_PAGE_LEFT           = 0x0001,
    SVX_PAGE_RIGHT          = 0x0002,
    SVX_PAGE_ALL            = 0x0003,
    SVX_PAGE_MIRROR         = 0x0007,
    SVX_PAGE_HEADERSHARE    = 0x0040,
    SVX_PAGE_FOOTERSHARE    = 0x0080
};

/*--------------------------------------------------------------------
    Beschreibung:   Teile der Seitenbeschreibung
 --------------------------------------------------------------------*/

#ifdef ITEMID_PAGE

/*
[Beschreibung]
Dieses Item beschreibt ein Seiten-Attribut (Name der Vorlage, Numerierung,
Portrait oder Landscape, Layout).
*/

class SVX_DLLPUBLIC SvxPageItem: public SfxPoolItem
{
private:
    String          aDescName;          // Name der Vorlage
    SvxNumType      eNumType;           // Numerierung
    BOOL            bLandscape;         // Portrait / Landscape
    USHORT          eUse;               // Layout

public:

    TYPEINFO();
    SvxPageItem( const USHORT nId = ITEMID_PAGE);
    SvxPageItem( const SvxPageItem& rItem );

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual SfxPoolItem*     Create( SvStream&, USHORT ) const;
    virtual SvStream&        Store( SvStream& , USHORT nItemVersion ) const;

    // Ausrichtung
    USHORT          GetPageUsage() const                { return eUse;       }
    void            SetPageUsage(USHORT eU)             { eUse= eU;          }

    BOOL            IsLandscape() const                 { return bLandscape; }
    void            SetLandscape(BOOL bL)               { bLandscape = bL;   }

    // Numerierung
    SvxNumType      GetNumType() const                  { return eNumType;   }
    void            SetNumType(SvxNumType eNum)         { eNumType = eNum;   }

    // Name des Descriptors
    const String&   GetDescName() const                 { return aDescName;  }
    void            SetDescName(const String& rStr)     { aDescName = rStr;  }
};
#endif


/*--------------------------------------------------------------------
    Beschreibung:   Container fuer Header/Footer-Attribute
 --------------------------------------------------------------------*/

#ifdef ITEMID_SETITEM

/*
[Beschreibung]
Dieses Item dient als Container fuer Header- und Footer-Attribute.
*/

class SVX_DLLPUBLIC SvxSetItem: public SfxSetItem
{
public:
    SvxSetItem( const USHORT nId, const SfxItemSet& rSet );
    SvxSetItem( const SvxSetItem& rItem );
    SvxSetItem( const USHORT nId, SfxItemSet* pSet );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*    Create( SvStream&, USHORT nVersion ) const;
    virtual SvStream&       Store( SvStream&, USHORT nItemVersion ) const;
};
#endif



#endif

