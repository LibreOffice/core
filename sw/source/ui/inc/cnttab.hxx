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


#ifndef _CNTTAB_HXX
#define _CNTTAB_HXX

#include <svx/stddlg.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>

#include "tox.hxx"
#include <tools/list.hxx>
#include <toxmgr.hxx>
#include <svx/checklbx.hxx>
#include <tools/resary.hxx>
#include <svtools/svtreebx.hxx>
#include <vcl/menubtn.hxx>
#include <svx/langbox.hxx>
#define TOX_PAGE_SELECT 1
#define TOX_PAGE_ENTRY  2
#define TOX_PAGE_STYLES 3

struct CurTOXType
{
    TOXTypes    eType;
    sal_uInt16      nIndex; //for TOX_USER only

    sal_Bool operator==(const CurTOXType aCmp)
       {
        return eType == aCmp.eType && nIndex == aCmp.nIndex;
       }
    sal_uInt16 GetFlatIndex() const;

    CurTOXType () : eType (TOX_INDEX), nIndex (0) {};

    CurTOXType (TOXTypes t, sal_uInt16 i) : eType (t), nIndex (i) {};
};

class SwOLENames : public Resource
{
    ResStringArray      aNamesAry;
public:
    SwOLENames(const ResId& rResId) :
        Resource(rResId),
        aNamesAry(ResId(1,*rResId.GetResMgr())){FreeResource();}

    ResStringArray&     GetNames() { return aNamesAry;}

};

#endif
