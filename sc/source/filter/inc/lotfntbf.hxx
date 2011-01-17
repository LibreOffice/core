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

#ifndef SC_LOTFNTBF_HXX
#define SC_LOTFNTBF_HXX

#include <tools/solar.h>

#include "scitems.hxx"
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>

// ---------------------------------------------------- class LotusFontBuffer -

// Code in fontbuff.cxx (excel)

class LotusFontBuffer
{
private:
    struct ENTRY
    {
        String*             pTmpName;
        SvxFontItem*        pFont;
        SvxFontHeightItem*  pHeight;
        SvxColorItem*       pColor;
        sal_Int32               nType;      // < 0 -> undefiniert
        inline              ENTRY( void )
                            {
                                pTmpName = NULL;
                                pFont = NULL;
                                pHeight = NULL;
                                pColor = NULL;
                                nType = -1;
                            }
        inline              ~ENTRY()
                            {
                                if( pTmpName )
                                    delete pTmpName;
                                if( pFont )
                                    delete pFont;
                                if( pHeight )
                                    delete pHeight;
                                if( pColor )
                                    delete pColor;
                            }
        inline void         TmpName( const String &rNew )
                            {
                                if( pTmpName )
                                    *pTmpName = rNew;
                                else
                                    pTmpName = new String( rNew );
                            }
        inline void         Font( SvxFontItem& rNew )
                            {
                                if( pFont )
                                    delete pFont;
                                pFont = &rNew;
                            }
        inline void         Height( SvxFontHeightItem& rNew )
                            {
                                if( pHeight )
                                    delete pHeight;
                                pHeight = &rNew;
                            }
        inline void         Color( SvxColorItem& rNew )
                            {
                                if( pColor )
                                    delete pColor;
                                pColor = &rNew;
                            }
        inline void         Type( const sal_uInt16 nNew )       { nType = nNew; }
    };

    ENTRY                   pData[ 8 ];
    const static sal_uInt16     nSize;
    void                    MakeFont( ENTRY* pEntry );
public:
    void                    Fill( const sal_uInt8 nIndex, SfxItemSet& rItemSet );
    void                    SetName( const sal_uInt16 nIndex, const String& rName );
    void                    SetHeight( const sal_uInt16 nIndex, const sal_uInt16 nHeight );
    void                    SetType( const sal_uInt16 nIndex, const sal_uInt16 nType );
};



#endif
