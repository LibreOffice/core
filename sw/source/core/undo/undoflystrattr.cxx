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


#include <undoflystrattr.hxx>
#include <frmfmt.hxx>

SwUndoFlyStrAttr::SwUndoFlyStrAttr( SwFlyFrmFmt& rFlyFrmFmt,
                                    const SwUndoId eUndoId,
                                    const OUString& sOldStr,
                                    const OUString& sNewStr )
    : SwUndo( eUndoId ),
      mrFlyFrmFmt( rFlyFrmFmt ),
      msOldStr( sOldStr ),
      msNewStr( sNewStr )
{
    OSL_ENSURE( eUndoId == UNDO_FLYFRMFMT_TITLE ||
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

    aResult.AddRule( UndoArg1, mrFlyFrmFmt.GetName() );

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
