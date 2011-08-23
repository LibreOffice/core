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
#ifndef _SVX_PAGEITEM_HXX
#define _SVX_PAGEITEM_HXX

// include ---------------------------------------------------------------

namespace binfilter {

/*--------------------------------------------------------------------
    Beschreibung:	SvxNumType
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
    Beschreibung:	Benutzung der Seite
 --------------------------------------------------------------------*/

enum SvxPageUsage
{
    SVX_PAGE_LEFT			= 0x0001,
    SVX_PAGE_RIGHT		  	= 0x0002,
    SVX_PAGE_ALL			= 0x0003,
    SVX_PAGE_MIRROR		  	= 0x0007,
    SVX_PAGE_HEADERSHARE    = 0x0040,
    SVX_PAGE_FOOTERSHARE    = 0x0080
};

/*--------------------------------------------------------------------
    Beschreibung:	Teile der Seitenbeschreibung
 --------------------------------------------------------------------*/

#ifdef ITEMID_PAGE

/*
[Beschreibung]
Dieses Item beschreibt ein Seiten-Attribut (Name der Vorlage, Numerierung,
Portrait oder Landscape, Layout).
*/

class SvxPageItem: public SfxPoolItem
{
private:
    String			aDescName;			// Name der Vorlage
    SvxNumType		eNumType;			// Numerierung
    BOOL			bLandscape;     	// Portrait / Landscape
    USHORT			eUse;				// Layout

public:

    TYPEINFO();
    SvxPageItem( const USHORT nId = ITEMID_PAGE);
    SvxPageItem( const SvxPageItem& rItem );

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual int              operator==( const SfxPoolItem& ) const;


    virtual	sal_Bool        	 QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool			 PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual SfxPoolItem*     Create( SvStream&, USHORT ) const;
    virtual SvStream&		 Store( SvStream& , USHORT nItemVersion ) const;

    // Ausrichtung
    USHORT 			GetPageUsage() const 				{ return eUse; 		 }
    void			SetPageUsage(USHORT eU)				{ eUse= eU; 		 }

    BOOL			IsLandscape() const 				{ return bLandscape; }
    void			SetLandscape(BOOL bL)				{ bLandscape = bL; 	 }

    // Numerierung
    SvxNumType		GetNumType() const 					{ return eNumType;	 }
    void			SetNumType(SvxNumType eNum)			{ eNumType = eNum;   }

    // Name des Descriptors
    const String& 	GetDescName() const 				{ return aDescName;  }
    void			SetDescName(const String& rStr)		{ aDescName = rStr;  }
};
#endif

/*--------------------------------------------------------------------
    Beschreibung:	Container fuer Header/Footer-Attribute
 --------------------------------------------------------------------*/

#ifdef ITEMID_SETITEM

/*
[Beschreibung]
Dieses Item dient als Container fuer Header- und Footer-Attribute.
*/

class SvxSetItem: public SfxSetItem
{
public:
    SvxSetItem( const USHORT nId, const SfxItemSet& rSet );
    SvxSetItem( const SvxSetItem& rItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;


    virtual SfxPoolItem*	Create( SvStream&, USHORT nVersion ) const;
    virtual SvStream&		Store( SvStream&, USHORT nItemVersion ) const;
};
#endif

}//end of namespace binfilter
#endif

