/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ccoll.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 10:19:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include "cmdid.h"

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#include "swmodule.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "globals.hrc"
#include "helpid.h"


#ifndef _SFX_STYFITEM_HXX //autogen
#include <sfx2/styfitem.hxx>
#endif

#include "uitool.hxx"
#include "ccoll.hxx"
#include "fmtcol.hxx"
#include "hintids.hxx"
#include "docsh.hxx"
#include "docstyle.hxx"
#include "hints.hxx"

#include "chrdlg.hrc"
#include "ccoll.hrc"

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

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


TYPEINIT1_AUTOFACTORY(SwCondCollItem, SfxPoolItem)

/****************************************************************************
    Item fuer den Transport der Bedingungstabelle
****************************************************************************/


SwCondCollItem::SwCondCollItem(USHORT _nWhich ) :
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
    BOOL bReturn = TRUE;
    for(USHORT i = 0; i < COND_COMMAND_COUNT; i++)
        if(sStyles[i] != ((SwCondCollItem&)rItem).sStyles[i])
        {
            bReturn = FALSE;
            break;
        }

    return bReturn;
}

/****************************************************************************

****************************************************************************/


const String&   SwCondCollItem::GetStyle(USHORT nPos) const
{
#ifndef IRIX
    return nPos < COND_COMMAND_COUNT ? sStyles[nPos] : aEmptyStr;
#else
    return nPos < COND_COMMAND_COUNT ? (String)sStyles[nPos] : aEmptyStr;
#endif
}

/****************************************************************************

****************************************************************************/


void SwCondCollItem::SetStyle(const String* pStyle, USHORT nPos)
{
    if( nPos < COND_COMMAND_COUNT )
#ifndef IRIX
        sStyles[nPos] = pStyle ? *pStyle : aEmptyStr;
#else
        sStyles[nPos] = pStyle ? (String)*pStyle : aEmptyStr;
#endif
}


/****************************************************************************

****************************************************************************/


const CommandStruct* SwCondCollItem::GetCmds()
{
    return aCmds;
}




