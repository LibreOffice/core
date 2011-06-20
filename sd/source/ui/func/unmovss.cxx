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
#include "precompiled_sd.hxx"

#include "unmovss.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"

SdMoveStyleSheetsUndoAction::SdMoveStyleSheetsUndoAction( SdDrawDocument* pTheDoc, SdStyleSheetVector& rTheStyles, bool bInserted)
: SdUndoAction(pTheDoc)
, mbMySheets( !bInserted )
{
    maStyles.swap( rTheStyles );

    maListOfChildLists.resize( maStyles.size() );
    // Liste mit den Listen der StyleSheet-Kinder erstellen
    std::size_t i = 0;
    for(SdStyleSheetVector::iterator iter = maStyles.begin(); iter != maStyles.end(); ++iter )
    {
        maListOfChildLists[i++] = SdStyleSheetPool::CreateChildList( (*iter).get() );
    }
}

/*************************************************************************
|*
|* Undo()
|*
\************************************************************************/

void SdMoveStyleSheetsUndoAction::Undo()
{
    SfxStyleSheetBasePool* pPool  = mpDoc->GetStyleSheetPool();

    if (mbMySheets)
    {
        // the styles have to be inserted in the pool

        // first insert all styles to the pool
        for(SdStyleSheetVector::iterator iter = maStyles.begin(); iter != maStyles.end(); ++iter )
        {
            pPool->Insert((*iter).get());
        }

        // now assign the childs again
        std::vector< SdStyleSheetVector >::iterator childlistiter( maListOfChildLists.begin() );
        for(SdStyleSheetVector::iterator iter = maStyles.begin(); iter != maStyles.end(); ++iter, ++childlistiter )
        {
            String aParent((*iter)->GetName());
            for( SdStyleSheetVector::iterator childiter = (*childlistiter).begin(); childiter != (*childlistiter).end(); ++childiter )
            {
                (*childiter)->SetParent(aParent);
            }
        }
    }
    else
    {
        // remove the styles again from the pool
        for(SdStyleSheetVector::iterator iter = maStyles.begin(); iter != maStyles.end(); ++iter )
        {
            pPool->Remove((*iter).get());
        }
    }
    mbMySheets = !mbMySheets;
}

void SdMoveStyleSheetsUndoAction::Redo()
{
    Undo();
}

SdMoveStyleSheetsUndoAction::~SdMoveStyleSheetsUndoAction()
{
}

String SdMoveStyleSheetsUndoAction::GetComment() const
{
    return String();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
