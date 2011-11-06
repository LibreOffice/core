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



#ifndef _EDITDBG_HXX
#define _EDITDBG_HXX

#include <svl/solar.hrc>
#include <tools/string.hxx>
#include <stdio.h>

class EditEngine;
class ParaPortion;
class EditUndoList;
class TextPortionList;
class SfxItemSet;
class SfxItemPool;
class SfxPoolItem;

ByteString  DbgOutItem( const SfxItemPool& rPool, const SfxPoolItem& rItem );
void        DbgOutItemSet( FILE* fp, const SfxItemSet& rSet, sal_Bool bSearchInParent, sal_Bool bShowALL );

class EditDbg
{
public:
    static void         ShowEditEngineData( EditEngine* pEditEngine, sal_Bool bInfoBox = sal_True );
    static void         ShowPortionData( ParaPortion* pPortion );
    static ByteString   GetPortionInfo( ParaPortion* pPPortion );
    static ByteString   GetTextPortionInfo( TextPortionList& rPortions );
    static ByteString   GetUndoDebStr( EditUndoList* pUndoList );
};


#endif // _EDITDBG_HXX
