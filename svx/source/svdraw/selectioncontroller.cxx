/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: selectioncontroller.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:48:21 $
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
#include "precompiled_svx.hxx"

#include <svx/selectioncontroller.hxx>

namespace sdr
{

bool SelectionController::onKeyInput(const KeyEvent& /*rKEvt*/, Window* /*pWin*/)
{
    return false;
}

bool SelectionController::onMouseButtonDown(const MouseEvent& /*rMEvt*/, Window* /*pWin*/)
{
    return false;
}

bool SelectionController::onMouseButtonUp(const MouseEvent& /*rMEvt*/, Window* /*pWin*/)
{
    return false;
}

bool SelectionController::onMouseMove(const MouseEvent& /*rMEvt*/, Window* /*pWin*/)
{
    return false;
}

void SelectionController::onSelectionHasChanged()
{
}

void SelectionController::GetState( SfxItemSet& /*rSet*/ )
{
}

void SelectionController::Execute( SfxRequest& /*rReq*/ )
{
}

bool SelectionController::DeleteMarked()
{
    return false;
}

bool SelectionController::GetAttributes(SfxItemSet& /*rTargetSet*/, bool /*bOnlyHardAttr*/) const
{
    return false;
}

bool SelectionController::SetAttributes(const SfxItemSet& /*rSet*/, bool /*bReplaceAll*/)
{
    return false;
}

bool SelectionController::GetStyleSheet( SfxStyleSheet* &/*rpStyleSheet*/ ) const
{
    return false;
}

bool SelectionController::SetStyleSheet( SfxStyleSheet* /*pStyleSheet*/, bool /*bDontRemoveHardAttr*/ )
{
    return false;
}

bool SelectionController::GetMarkedObjModel( SdrPage* /*pNewPage*/ )
{
    return false;
}

bool SelectionController::PasteObjModel( const SdrModel& /*rModel*/ )
{
    return false;
}

}
