/*************************************************************************
 *
 *  $RCSfile: zoomitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:21 $
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

#ifndef _ARGS_HXX //autogen
#include <svtools/args.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef __SBX_SBXVARIABLE_HXX
#include <svtools/sbxvar.hxx>
#endif
#pragma hdrstop

#include "zoomitem.hxx"

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxZoomItem,SfxUInt16Item);

// -----------------------------------------------------------------------

SvxZoomItem::SvxZoomItem
(
    SvxZoomType eZoomType,
    sal_uInt16      nVal,
    sal_uInt16      nWhich
)
:   SfxUInt16Item( nWhich, nVal ),
    nValueSet( SVX_ZOOM_ENABLE_ALL ),
    eType( eZoomType )
{
}

// -----------------------------------------------------------------------

SvxZoomItem::SvxZoomItem( const SvxZoomItem& rOrig )
:   SfxUInt16Item( rOrig.Which(), rOrig.GetValue() ),
    nValueSet( rOrig.GetValueSet() ),
    eType( rOrig.GetType() )
{
}

// -----------------------------------------------------------------------

SvxZoomItem::~SvxZoomItem()
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxZoomItem::Clone( SfxItemPool *pPool ) const
{
    return new SvxZoomItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxZoomItem::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
{
    sal_uInt16 nValue;
    sal_uInt16 nValSet;
    sal_Int8 nType;
    rStrm >> nValue >> nValSet >> nType;
    SvxZoomItem* pNew = new SvxZoomItem( (SvxZoomType)nType, nValue, Which() );
    pNew->SetValueSet( nValSet );
    return pNew;
}

// -----------------------------------------------------------------------

SvStream& SvxZoomItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_uInt16)GetValue()
          << nValueSet
          << (sal_Int8)eType;
    return rStrm;
}

// -----------------------------------------------------------------------

int SvxZoomItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    SvxZoomItem& rItem = (SvxZoomItem&)rAttr;

    return ( GetValue() == rItem.GetValue()     &&
             nValueSet  == rItem.GetValueSet()  &&
             eType      == rItem.GetType()          );
}


