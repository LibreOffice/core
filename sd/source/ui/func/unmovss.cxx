/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unmovss.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:41:25 $
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
    for(SdStyleSheetVector::iterator iter = maStyles.begin(); iter != maStyles.end(); iter++ )
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
        for(SdStyleSheetVector::iterator iter = maStyles.begin(); iter != maStyles.end(); iter++ )
        {
            pPool->Insert((*iter).get());
        }

        // now assign the childs again
        std::vector< SdStyleSheetVector >::iterator childlistiter( maListOfChildLists.begin() );
        for(SdStyleSheetVector::iterator iter = maStyles.begin(); iter != maStyles.end(); iter++, childlistiter++ )
        {
            String aParent((*iter)->GetName());
            for( SdStyleSheetVector::iterator childiter = (*childlistiter).begin(); childiter != (*childlistiter).end(); childiter++ )
            {
                (*childiter)->SetParent(aParent);
            }
        }
    }
    else
    {
        // remove the styles again from the pool
        for(SdStyleSheetVector::iterator iter = maStyles.begin(); iter != maStyles.end(); iter++ )
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


