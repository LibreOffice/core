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
#include <sortedobjs.hxx>
#include <sortedobjsimpl.hxx>

SwSortedObjs::SwSortedObjs()
    : mpImpl( new SwSortedObjsImpl )
{
}

SwSortedObjs::~SwSortedObjs()
{
    delete mpImpl;
}

sal_uInt32 SwSortedObjs::Count() const
{
    return mpImpl->Count();
}

SwAnchoredObject* SwSortedObjs::operator[]( sal_uInt32 _nIndex ) const
{
    return (*mpImpl)[ _nIndex ];
}

bool SwSortedObjs::Insert( SwAnchoredObject& _rAnchoredObj )
{
    return mpImpl->Insert( _rAnchoredObj );
}

bool SwSortedObjs::Remove( SwAnchoredObject& _rAnchoredObj )
{
    return mpImpl->Remove( _rAnchoredObj );
}

bool SwSortedObjs::Contains( const SwAnchoredObject& _rAnchoredObj ) const
{
    return mpImpl->Contains( _rAnchoredObj );
}

bool SwSortedObjs::Update( SwAnchoredObject& _rAnchoredObj )
{
    return mpImpl->Update( _rAnchoredObj );
}

sal_uInt32 SwSortedObjs::ListPosOf( const SwAnchoredObject& _rAnchoredObj ) const
{
    return mpImpl->ListPosOf( _rAnchoredObj );
}
