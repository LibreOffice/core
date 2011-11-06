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


#ifndef _CONDEDTI_HXX
#define _CONDEDTI_HXX

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#include <svtools/transfer.hxx>
#include "swdllapi.h"

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC ConditionEdit : public Edit, public DropTargetHelper
{
    sal_Bool bBrackets, bEnableDrop;

    SW_DLLPRIVATE virtual sal_Int8  AcceptDrop( const AcceptDropEvent& rEvt );
    SW_DLLPRIVATE virtual sal_Int8  ExecuteDrop( const ExecuteDropEvent& rEvt );

public:
    ConditionEdit( Window* pParent, const ResId& rResId );

    inline void ShowBrackets(sal_Bool bShow)        { bBrackets = bShow; }

    inline void SetDropEnable( sal_Bool bFlag )     { bEnableDrop = bFlag; }
    inline sal_Bool IsDropEnable() const            { return bEnableDrop; }
};

#endif
















