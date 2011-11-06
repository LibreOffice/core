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

#include "svx/ActionDescriptionProvider.hxx"
#include "svx/svdglob.hxx"
#include "svx/svdstr.hrc"

::rtl::OUString ActionDescriptionProvider::createDescription( ActionType eActionType
                        , const ::rtl::OUString& rObjectName )
{
    sal_uInt16 nResID=0;
    switch( eActionType )
    {
    case INSERT:
        nResID=STR_UndoInsertObj;
        break;
    case DELETE:
        nResID= STR_EditDelete;
        break;
    case CUT:
        nResID= STR_ExchangeClpCut;
        break;
    case MOVE:
        nResID= STR_EditMove;
        break;
    case RESIZE:
        nResID= STR_EditResize;
        break;
    case ROTATE:
        nResID= STR_EditRotate;
        break;
    case TRANSFORM:
        nResID= STR_EditTransform;
        break;
    case FORMAT:
        nResID= STR_EditSetAttributes;
        break;
    case MOVE_TOTOP:
        nResID= STR_EditMovToTop;
        break;
    case MOVE_TOBOTTOM:
        nResID= STR_EditMovToBtm;
        break;
    case POS_SIZE:
        nResID = STR_EditPosSize;
        break;
    }
    if(!nResID)
        return rtl::OUString();

    XubString aStr(ImpGetResStr(nResID));
    XubString aName(rObjectName);
    aStr.SearchAndReplaceAscii("%1", aName);
    return rtl::OUString(aStr);
}
