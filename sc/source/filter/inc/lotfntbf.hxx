/*************************************************************************
 *
 *  $RCSfile: lotfntbf.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
