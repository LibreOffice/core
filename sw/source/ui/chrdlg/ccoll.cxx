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



#include "cmdid.h"
#include <tools/list.hxx>
#include "swmodule.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "globals.hrc"
#include "helpid.h"


#include <sfx2/styfitem.hxx>

#include "uitool.hxx"
#include "ccoll.hxx"
#include "fmtcol.hxx"
#include "hintids.hxx"
#include "docsh.hxx"
#include "docstyle.hxx"
#include "hints.hxx"

#include "chrdlg.hrc"
#include "ccoll.hrc"
#include <vcl/svapp.hxx>

#include <unomid.h>

// ******************************************************************

//!! order of entries has to be the same as in
//!! CommandStruct SwCondCollItem::aCmds[]

const char *aCommandContext[COND_COMMAND_COUNT] =
{
    "TableHeader",
    "Table",
    "Frame",
    "Section",
    "Footnote",
    "Endnote",
    "Header",
    "Footer",
    "OutlineLevel1",
    "OutlineLevel2",
    "OutlineLevel3",
    "OutlineLevel4",
    "OutlineLevel5",
    "OutlineLevel6",
    "OutlineLevel7",
    "OutlineLevel8",
    "OutlineLevel9",
    "OutlineLevel10",
    "NumberingLevel1",
    "NumberingLevel2",
    "NumberingLevel3",
    "NumberingLevel4",
    "NumberingLevel5",
    "NumberingLevel6",
    "NumberingLevel7",
    "NumberingLevel8",
    "NumberingLevel9",
    "NumberingLevel10"
};

sal_Int16 GetCommandContextIndex( const rtl::OUString &rContextName )
{
    sal_Int16 nRes = -1;
    for (sal_Int16 i = 0;  nRes == -1 && i < COND_COMMAND_COUNT;  ++i)
    {
        if (rContextName.equalsAscii( aCommandContext[i] ))
            nRes = i;
    }
    return nRes;
}

rtl::OUString GetCommandContextByIndex( sal_Int16 nIndex )
{
    rtl::OUString aRes;
    if (0 <= nIndex  &&  nIndex < COND_COMMAND_COUNT)
    {
        aRes = C2U( aCommandContext[ nIndex ] );
    }
    return aRes;
}

// Globals ******************************************************************


CommandStruct SwCondCollItem::aCmds[] =
{
    { PARA_IN_TABLEHEAD,    0 },
    { PARA_IN_TABLEBODY,    0 },
    { PARA_IN_FRAME,        0 },
    { PARA_IN_SECTION,      0 },
    { PARA_IN_FOOTENOTE,    0 },
    { PARA_IN_ENDNOTE,      0 },
    { PARA_IN_HEADER,       0 },
    { PARA_IN_FOOTER,       0 },
    { PARA_IN_OUTLINE,      0 },
    { PARA_IN_OUTLINE,      1 },
    { PARA_IN_OUTLINE,      2 },
    { PARA_IN_OUTLINE,      3 },
    { PARA_IN_OUTLINE,      4 },
    { PARA_IN_OUTLINE,      5 },
    { PARA_IN_OUTLINE,      6 },
    { PARA_IN_OUTLINE,      7 },
    { PARA_IN_OUTLINE,      8 },
    { PARA_IN_OUTLINE,      9 },
    { PARA_IN_LIST,         0 },
    { PARA_IN_LIST,         1 },
    { PARA_IN_LIST,         2 },
    { PARA_IN_LIST,         3 },
    { PARA_IN_LIST,         4 },
    { PARA_IN_LIST,         5 },
    { PARA_IN_LIST,         6 },
    { PARA_IN_LIST,         7 },
    { PARA_IN_LIST,         8 },
    { PARA_IN_LIST,         9 }
};

/****************************************************************************
    Item fuer den Transport der Bedingungstabelle
****************************************************************************/


SwCondCollItem::SwCondCollItem(sal_uInt16 _nWhich ) :
    SfxPoolItem(_nWhich)
{

}
/****************************************************************************

****************************************************************************/


SwCondCollItem::~SwCondCollItem()
{
}

/****************************************************************************

****************************************************************************/


SfxPoolItem*   SwCondCollItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwCondCollItem(*this);
}

/****************************************************************************

****************************************************************************/


int SwCondCollItem::operator==( const SfxPoolItem& rItem) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rItem), "unterschiedliche Typen" );
    sal_Bool bReturn = sal_True;
    for(sal_uInt16 i = 0; i < COND_COMMAND_COUNT; i++)
        if(sStyles[i] != ((SwCondCollItem&)rItem).sStyles[i])
        {
            bReturn = sal_False;
            break;
        }

    return bReturn;
}

/****************************************************************************

****************************************************************************/


const String&   SwCondCollItem::GetStyle(sal_uInt16 nPos) const
{
    return nPos < COND_COMMAND_COUNT ? sStyles[nPos] : aEmptyStr;
}

/****************************************************************************

****************************************************************************/


void SwCondCollItem::SetStyle(const String* pStyle, sal_uInt16 nPos)
{
    if( nPos < COND_COMMAND_COUNT )
        sStyles[nPos] = pStyle ? *pStyle : aEmptyStr;
}


/****************************************************************************

****************************************************************************/


const CommandStruct* SwCondCollItem::GetCmds()
{
    return aCmds;
}




