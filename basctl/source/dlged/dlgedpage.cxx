/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedpage.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:15:26 $
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


#ifndef _BASCTL_DLGEDPAGE_HXX
#include "dlgedpage.hxx"
#endif

#ifndef _BASCTL_DLGED_HXX
#include "dlged.hxx"
#endif
#ifndef _BASCTL_DLGEDMOD_HXX
#include "dlgedmod.hxx"
#endif
#ifndef _BASCTL_DLGEDOBJ_HXX
#include "dlgedobj.hxx"
#endif


TYPEINIT1( DlgEdPage, SdrPage );

//----------------------------------------------------------------------------

DlgEdPage::DlgEdPage( DlgEdModel& rModel, FASTBOOL bMasterPage )
    :SdrPage( rModel, bMasterPage )
{
}

//----------------------------------------------------------------------------

DlgEdPage::DlgEdPage( const DlgEdPage& rPage )
    :SdrPage( rPage )
{
    pDlgEdForm = rPage.pDlgEdForm;
}

//----------------------------------------------------------------------------

DlgEdPage::~DlgEdPage()
{
    Clear();
}

//----------------------------------------------------------------------------

SdrPage* DlgEdPage::Clone() const
{
    return new DlgEdPage( *this );
}

//----------------------------------------------------------------------------

SdrObject* DlgEdPage::SetObjectOrdNum(ULONG nOldObjNum, ULONG nNewObjNum)
{
    SdrObject* pObj = SdrPage::SetObjectOrdNum( nOldObjNum, nNewObjNum );

    DlgEdHint aHint( DLGED_HINT_OBJORDERCHANGED );
    if ( pDlgEdForm )
    {
        DlgEditor* pDlgEditor = pDlgEdForm->GetDlgEditor();
        if ( pDlgEditor )
            pDlgEditor->Broadcast( aHint );
    }

    return pObj;
}

//----------------------------------------------------------------------------
