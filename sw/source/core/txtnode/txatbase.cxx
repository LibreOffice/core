/*************************************************************************
 *
 *  $RCSfile: txatbase.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-15 15:51:39 $
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

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif

SwTxtAttr::SwTxtAttr( const SfxPoolItem& rAttr, xub_StrLen nStt )
    : pAttr( &rAttr ), nStart( nStt )
{
    bDontExpand = bLockExpandFlag = bDontMergeAttr = bDontMoveAttr =
        bCharFmtAttr = bOverlapAllowedAttr = bPriorityAttr = FALSE;
}

SwTxtAttr::~SwTxtAttr( )
{
}

xub_StrLen* SwTxtAttr::GetEnd()
{
    return 0;
}

    // RemoveFromPool muss immer vorm DTOR Aufruf erfolgen!!
    // Meldet sein Attribut beim Pool ab
void SwTxtAttr::RemoveFromPool( SfxItemPool& rPool )
{
    rPool.Remove( GetAttr() );
    pAttr = 0;
}

int SwTxtAttr::operator==( const SwTxtAttr& rAttr ) const
{
    return GetAttr() == rAttr.GetAttr();
}

SwTxtAttrEnd::SwTxtAttrEnd( const SfxPoolItem& rAttr, xub_StrLen nS,
                            xub_StrLen nE )
    : SwTxtAttr( rAttr, nS ), nEnd( nE )
{
}

xub_StrLen* SwTxtAttrEnd::GetEnd()
{
    return &nEnd;
}
