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


#ifndef _CCOLL_HXX
#define _CCOLL_HXX

#include <svl/poolitem.hxx>
#include <tools/string.hxx>
#include <sfx2/tabdlg.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <svtools/svtabbx.hxx>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <tools/resary.hxx>
#include "swdllapi.h"
#include "cmdid.h"

#include <rtl/string.hxx>

//***********************************************************

struct CollName {
//      const char* pStr;
        sal_uLong nCnd;
        sal_uLong nSubCond;
    };

//***********************************************************

#define COND_COMMAND_COUNT 28

struct CommandStruct
{
    sal_uLong nCnd;
    sal_uLong nSubCond;
};

//***********************************************************

sal_Int16       GetCommandContextIndex( const rtl::OUString &rContextName );
rtl::OUString   GetCommandContextByIndex( sal_Int16 nIndex );

//***********************************************************

class SW_DLLPUBLIC SwCondCollItem : public SfxPoolItem
{
    static CommandStruct        aCmds[COND_COMMAND_COUNT];

    String                      sStyles[COND_COMMAND_COUNT];

public:
    SwCondCollItem(sal_uInt16 nWhich = FN_COND_COLL);
    ~SwCondCollItem();

    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual int                 operator==( const SfxPoolItem& ) const;

    static const CommandStruct* GetCmds();

    const String&               GetStyle(sal_uInt16 nPos) const;
    void                        SetStyle( const String* pStyle, sal_uInt16 nPos);

};

#endif

