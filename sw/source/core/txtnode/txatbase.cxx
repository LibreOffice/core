/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txatbase.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 13:48:46 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif

#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _DOCUFLD_HXX //autogen
#include <docufld.hxx>
#endif


#include "doc.hxx"
#include "docsh.hxx"
#include "../../../inc/PostItMgr.hxx"
#include "../../ui/inc/view.hxx"


SwTxtAttr::SwTxtAttr( const SfxPoolItem& rAttr, xub_StrLen nStt )
    : pAttr( &rAttr ), nStart( nStt )
{
    bDontExpand = bLockExpandFlag = bDontMergeAttr = bDontMoveAttr =
        bCharFmtAttr = bOverlapAllowedAttr = bPriorityAttr =
        bDontExpandStart = FALSE;
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
    /*
    // delete in SwPostItMgr
    if (Which()==RES_TXTATR_FIELD)
    {
        if ( ((SwFmtFld&)(GetAttr())).GetFld()->GetTyp()->Which()==RES_POSTITFLD)
        {
            SwDocShell* aShell = static_cast<SwPostItFieldType*>(((SwFmtFld&)GetAttr()).GetFld()->GetTyp())->GetDoc()->GetDocShell();
            if (aShell)
            {
                aShell->GetView()->GetPostItMgr()->Delete(&(SwFmtFld&)GetAttr());
            }

        }
    }
    */
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
