/*************************************************************************
 *
 *  $RCSfile: atrref.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-15 15:51:57 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _TXTRFMRK_HXX //autogen
#include <txtrfmrk.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen
#include <fmtrfmrk.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif


/****************************************************************************
 *
 *  class SwFmtRefMark
 *
 ****************************************************************************/

SwFmtRefMark::~SwFmtRefMark( )
{
}

SwFmtRefMark::SwFmtRefMark( const XubString& rName )
    : SfxPoolItem( RES_TXTATR_REFMARK ),
    aRefName( rName ),
    pTxtAttr( 0 )
{
}

SwFmtRefMark::SwFmtRefMark( const SwFmtRefMark& rAttr )
    : SfxPoolItem( RES_TXTATR_REFMARK ),
    aRefName( rAttr.aRefName ),
    pTxtAttr( 0 )
{
}

int SwFmtRefMark::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return aRefName == ((SwFmtRefMark&)rAttr).aRefName;
}

SfxPoolItem* SwFmtRefMark::Clone( SfxItemPool* ) const
{
    return new SwFmtRefMark( *this );
}

/*************************************************************************
 *                      class SwTxtRefMark
 *************************************************************************/

// Attribut fuer Inhalts-/Positions-Referenzen im Text

SwTxtRefMark::SwTxtRefMark( const SwFmtRefMark& rAttr,
                    xub_StrLen nStart, xub_StrLen* pEnde )
    : SwTxtAttrEnd( rAttr, nStart, nStart ),
    pEnd( 0 ),
    pMyTxtNd( 0 )
{
    ((SwFmtRefMark&)rAttr).pTxtAttr = this;
    if( pEnde )
    {
        nEnd = *pEnde;
        pEnd = &nEnd;
    }
    SetDontMergeAttr( TRUE );
    SetDontMoveAttr( TRUE );
    SetOverlapAllowedAttr( TRUE );
}

xub_StrLen* SwTxtRefMark::GetEnd()
{
    return pEnd;
}

