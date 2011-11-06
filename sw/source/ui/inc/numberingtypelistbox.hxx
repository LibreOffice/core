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


#ifndef _NUMBERINGTYPELISTBOX_HXX
#define _NUMBERINGTYPELISTBOX_HXX

#include <vcl/lstbox.hxx>
#include "swdllapi.h"

#define INSERT_NUM_TYPE_NO_NUMBERING                0x01
#define INSERT_NUM_TYPE_PAGE_STYLE_NUMBERING        0x02
#define INSERT_NUM_TYPE_BITMAP                      0x04
#define INSERT_NUM_TYPE_BULLET                      0x08
#define INSERT_NUM_EXTENDED_TYPES                   0x10

struct SwNumberingTypeListBox_Impl;

class SW_DLLPUBLIC SwNumberingTypeListBox : public ListBox
{
    SwNumberingTypeListBox_Impl* pImpl;

public:
    SwNumberingTypeListBox( Window* pWin, const ResId& rResId,
        sal_uInt16 nTypeFlags = INSERT_NUM_TYPE_PAGE_STYLE_NUMBERING|INSERT_NUM_TYPE_NO_NUMBERING|INSERT_NUM_EXTENDED_TYPES );
    ~SwNumberingTypeListBox();

    void        Reload(sal_uInt16 nTypeFlags);

    sal_Int16   GetSelectedNumberingType();
    sal_Bool    SelectNumberingType(sal_Int16 nType);
};


#endif

