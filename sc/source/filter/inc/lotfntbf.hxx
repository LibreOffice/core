/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lotfntbf.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:21:28 $
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

#ifndef _LOTFNTBF_HXX
#define _LOTFNTBF_HXX

#include <tools/solar.h>

#include "scitems.hxx"

#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif

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
        INT32               nType;      // < 0 -> undefiniert
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
        inline void         Type( const UINT16 nNew )       { nType = nNew; }
    };

    ENTRY                   pData[ 8 ];
    const static UINT16     nSize;
    void                    MakeFont( ENTRY* pEntry );
public:
    void                    Fill( const UINT8 nIndex, SfxItemSet& rItemSet );
    void                    SetName( const UINT16 nIndex, const String& rName );
    void                    SetHeight( const UINT16 nIndex, const UINT16 nHeight );
    void                    SetType( const UINT16 nIndex, const UINT16 nType );
};



#endif
