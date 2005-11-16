/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: popbox.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 13:54:11 $
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


#pragma hdrstop

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

#ifndef _POPBOX_HXX
#include <popbox.hxx>
#endif
#ifndef _NAVIPI_HXX
#include <navipi.hxx>
#endif


// --- class SwHelpToolBox ---------------------------------------------


SwHelpToolBox::SwHelpToolBox( SwNavigationPI* pParent, const ResId& rResId )
    : ToolBox( pParent, rResId ),
    DropTargetHelper( this )
{
}


void SwHelpToolBox::MouseButtonDown(const MouseEvent &rEvt)
{
        // Zuerst DoubleClick-Link auswerten
        // Dessen Returnwert entscheidet ueber andere Verarbeitung
        // Doppelclickhandler nur, wenn nicht auf einen Button geclickt wurde
    if(rEvt.GetButtons() == MOUSE_RIGHT &&
        0 == GetItemId(rEvt.GetPosPixel()))
    {
        aRightClickLink.Call((MouseEvent *)&rEvt);
    }
    else
        ToolBox::MouseButtonDown(rEvt);
}


long SwHelpToolBox::DoubleClick( ToolBox* pCaller )
{
        // kein Doppelklick auf einen Button
    if( 0 == pCaller->GetCurItemId() && aDoubleClickLink.Call(0) )
        return TRUE;
    return FALSE;
}

/*-----------------26.02.94 00:36-------------------
 dtor ueberladen
--------------------------------------------------*/


SwHelpToolBox::~SwHelpToolBox() {}

sal_Int8 SwHelpToolBox::AcceptDrop( const AcceptDropEvent& rEvt )
{
    return ((SwNavigationPI*)GetParent())->AcceptDrop( rEvt );
}

sal_Int8 SwHelpToolBox::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    return ((SwNavigationPI*)GetParent())->ExecuteDrop( rEvt );
}


