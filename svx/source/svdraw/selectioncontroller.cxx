/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svx/selectioncontroller.hxx>
#include <svx/svdobj.hxx>

namespace sdr
{

bool SelectionController::onKeyInput(const KeyEvent& /*rKEvt*/, vcl::Window* /*pWin*/)
{
    return false;
}

bool SelectionController::onMouseButtonDown(const MouseEvent& /*rMEvt*/, vcl::Window* /*pWin*/)
{
    return false;
}

bool SelectionController::onMouseButtonUp(const MouseEvent& /*rMEvt*/, vcl::Window* /*pWin*/)
{
    return false;
}

bool SelectionController::onMouseMove(const MouseEvent& /*rMEvt*/, vcl::Window* /*pWin*/)
{
    return false;
}

void SelectionController::onSelectionHasChanged()
{
}

void SelectionController::onSelectAll()
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

rtl::Reference<SdrObject> SelectionController::GetMarkedSdrObjClone( SdrModel& /*rTargetModel*/ )
{
    return nullptr;
}

bool SelectionController::PasteObjModel( const SdrModel& /*rModel*/ )
{
    return false;
}

bool SelectionController::ApplyFormatPaintBrush(SfxItemSet& /*rFormatSet*/, sal_Int16 /*nDepth*/,
                                                bool /*bNoCharacterFormats*/,
                                                bool /*bNoParagraphFormats*/)
{
    return false;
}

bool SelectionController::hasSelectedCells() const
{
    return false;
}

void SelectionController::getSelectedCells(table::CellPos& /*rFirstPos*/, table::CellPos& /*rLastPos*/)
{
}

bool SelectionController::setCursorLogicPosition(const Point& /*rPosition*/, bool /*bPoint*/)
{
    return false;
}


bool SelectionController::ChangeFontSize(bool /*bGrow*/, const FontList* /*pFontList*/)
{
    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
