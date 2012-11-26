/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
