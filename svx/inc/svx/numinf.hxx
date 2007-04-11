/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numinf.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:02:15 $
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
#ifndef _SVX_NUMINF_HXX
#define _SVX_NUMINF_HXX

// include ---------------------------------------------------------------

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SVX_NUMFMTSH_HXX //autogen
#include <svx/numfmtsh.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class SvxNumberInfoItem -----------------------------------------------

#ifdef ITEMID_NUMBERINFO

/*
[Beschreibung]
Dieses Item dient als Transport-Medium fuer einen Number-Formatter.
*/

class SVX_DLLPUBLIC SvxNumberInfoItem : public SfxPoolItem
{
public:
    TYPEINFO();

    SvxNumberInfoItem( const USHORT nId = ITEMID_NUMBERINFO );
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                       const USHORT nId = ITEMID_NUMBERINFO );
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter, const String& rVal,
                       const USHORT nId = ITEMID_NUMBERINFO );
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter, const double& rVal,
                       const USHORT nId = ITEMID_NUMBERINFO );
    // if both double and String are supplied, String is used for text formats
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter, const double& rVal,
                       const String& rValueStr, const USHORT nId );
    SvxNumberInfoItem( const SvxNumberInfoItem& );
    ~SvxNumberInfoItem();

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create( SvStream& rStream, USHORT nVer ) const;
    virtual SvStream&        Store( SvStream& , USHORT nItemVersion ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    SvNumberFormatter*      GetNumberFormatter() const { return pFormatter; }
    void                    SetNumberFormatter( SvNumberFormatter* pNumFrmt );
    const String&           GetValueString() const { return aStringVal; }
    void                    SetStringValue( const String& rNewVal );
    double                  GetValueDouble() const  { return nDoubleVal; }
    void                    SetDoubleValue( const double& rNewVal );

    const sal_uInt32*       GetDelArray() const { return pDelFormatArr; }
    void                    SetDelFormatArray( const sal_uInt32* pData,
                                               const sal_uInt32  nCount );

    SvxNumberValueType      GetValueType() const { return eValueType; }
    sal_uInt32              GetDelCount() const  { return nDelCount; }

private:
    SvNumberFormatter*  pFormatter;
    SvxNumberValueType  eValueType;
    String              aStringVal;
    double              nDoubleVal;

    sal_uInt32*         pDelFormatArr;
    sal_uInt32          nDelCount;
};

#endif

#endif

