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


