/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: charfmt.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 15:31:37 $
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
#ifndef _CHARFMT_HXX
#define _CHARFMT_HXX

#ifndef _FORMAT_HXX
#include <format.hxx>
#endif

class SwCharFmt : public SwFmt
{
    friend class SwDoc;
    friend class SwTxtFmtColl;

    SwCharFmt( SwAttrPool& rPool, const sal_Char* pFmtName,
                SwCharFmt *pDerivedFrom )
          : SwFmt( rPool, pFmtName, aCharFmtSetRange, pDerivedFrom, RES_CHRFMT )
    {}
    SwCharFmt( SwAttrPool& rPool, const String &rFmtName,
                SwCharFmt *pDerivedFrom )
          : SwFmt( rPool, rFmtName, aCharFmtSetRange, pDerivedFrom, RES_CHRFMT )
    {}


public:
    TYPEINFO();    //Bereits in Basisklasse Client drin.
};

namespace CharFmt
{
    extern const SfxItemSet* GetItemSet( const SfxPoolItem& rAttr );
    extern const SfxPoolItem* GetItem( const SwTxtAttr& rAttr, USHORT nWhich );
    extern BOOL IsItemIncluded( const USHORT nWhich, const SwTxtAttr *pAttr );
}

#endif



