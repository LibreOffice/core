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
#include "precompiled_sw.hxx"

#include <undoflystrattr.hxx>
#include <frmfmt.hxx>

SwUndoFlyStrAttr::SwUndoFlyStrAttr( SwFlyFrmFmt& rFlyFrmFmt,
                                    const SwUndoId eUndoId,
                                    const String& sOldStr,
                                    const String& sNewStr )
    : SwUndo( eUndoId ),
      mrFlyFrmFmt( rFlyFrmFmt ),
      msOldStr( sOldStr ),
      msNewStr( sNewStr )
{
    ASSERT( eUndoId == UNDO_FLYFRMFMT_TITLE ||
            eUndoId == UNDO_FLYFRMFMT_DESCRIPTION,
            "<SwUndoFlyStrAttr::SwUndoFlyStrAttr(..)> - unexpected undo id --> Undo will not work" );
}

SwUndoFlyStrAttr::~SwUndoFlyStrAttr()
{
}

void SwUndoFlyStrAttr::UndoImpl(::sw::UndoRedoContext &)
{
    switch ( GetId() )
    {
        case UNDO_FLYFRMFMT_TITLE:
        {
            mrFlyFrmFmt.SetObjTitle( msOldStr, true );
        }
        break;
        case UNDO_FLYFRMFMT_DESCRIPTION:
        {
            mrFlyFrmFmt.SetObjDescription( msOldStr, true );
        }
        break;
        default:
        {
        }
    }
}

void SwUndoFlyStrAttr::RedoImpl(::sw::UndoRedoContext &)
{
    switch ( GetId() )
    {
        case UNDO_FLYFRMFMT_TITLE:
        {
            mrFlyFrmFmt.SetObjTitle( msNewStr, true );
        }
        break;
        case UNDO_FLYFRMFMT_DESCRIPTION:
        {
            mrFlyFrmFmt.SetObjDescription( msNewStr, true );
        }
        break;
        default:
        {
        }
    }
}

SwRewriter SwUndoFlyStrAttr::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule( UNDO_ARG1, mrFlyFrmFmt.GetName() );

    return aResult;
}
