/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <tools/debug.hxx>
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

bool SelectionController::TakeFormatPaintBrush( boost::shared_ptr< SfxItemSet >& /*rFormatSet*/  )
{
    return false;
}

bool SelectionController::ApplyFormatPaintBrush( SfxItemSet& /*rFormatSet*/, bool /*bNoCharacterFormats*/, bool /*bNoParagraphFormats*/ )
{
    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
