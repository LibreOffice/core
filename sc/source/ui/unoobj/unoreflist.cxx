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
#include "precompiled_sc.hxx"


#include "unoreflist.hxx"
#include "document.hxx"

//------------------------------------------------------------------------

ScUnoRefList::ScUnoRefList()
{
}

ScUnoRefList::~ScUnoRefList()
{
}

void ScUnoRefList::Add( sal_Int64 nId, const ScRangeList& rOldRanges )
{
    aEntries.push_back( ScUnoRefEntry( nId, rOldRanges ) );
}

void ScUnoRefList::Undo( ScDocument* pDoc )
{
    std::list<ScUnoRefEntry>::const_iterator aEnd( aEntries.end() );
    for ( std::list<ScUnoRefEntry>::const_iterator aIter( aEntries.begin() );
          aIter != aEnd; ++aIter )
    {
        ScUnoRefUndoHint aHint( *aIter );
        pDoc->BroadcastUno( aHint );
    }
}

//------------------------------------------------------------------------

ScUnoRefUndoHint::ScUnoRefUndoHint( const ScUnoRefEntry& rRefEntry ) :
    aEntry( rRefEntry )
{
}

ScUnoRefUndoHint::~ScUnoRefUndoHint()
{
}

