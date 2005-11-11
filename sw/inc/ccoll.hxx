/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ccoll.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 13:14:42 $
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
#ifndef _CCOLL_HXX
#define _CCOLL_HXX

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _TOOLS_RESARY_HXX
#include <tools/resary.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

#ifndef INCLUDED_SW_CMDID_H
#include "cmdid.h"
#endif

#include <rtl/string.hxx>

class SwWrtShell;
class SwFmt;
class SwCollCondition;


//***********************************************************

struct CollName {
//      const char* pStr;
        ULONG nCnd;
        ULONG nSubCond;
    };

//***********************************************************

#define COND_COMMAND_COUNT 28

struct CommandStruct
{
    ULONG nCnd;
    ULONG nSubCond;
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
    SwCondCollItem(USHORT nWhich = FN_COND_COLL);
    ~SwCondCollItem();

                                TYPEINFO();

    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual int                 operator==( const SfxPoolItem& ) const;

    static const CommandStruct* GetCmds();

    const String&               GetStyle(USHORT nPos) const;
    void                        SetStyle( const String* pStyle, USHORT nPos);

};

//***********************************************************

//CHINA001 class SwCondCollPage : public SfxTabPage
//CHINA001 {
//CHINA001 FixedLine           aConditionFL;
//CHINA001 CheckBox         aConditionCB;
//CHINA001
//CHINA001 FixedText            aContextFT;
//CHINA001 FixedText            aUsedFT;
//CHINA001 SvTabListBox         aTbLinks;
//CHINA001
//CHINA001 FixedText            aStyleFT;
//CHINA001 ListBox              aStyleLB;
//CHINA001 ListBox              aFilterLB;
//CHINA001
//CHINA001 PushButton           aRemovePB;
//CHINA001 PushButton           aAssignPB;
//CHINA001
//CHINA001 String               sNoTmpl;
//CHINA001 ResStringArray       aStrArr;
//CHINA001
//CHINA001 SwWrtShell           &rSh;
//CHINA001 const CommandStruct*pCmds;
//CHINA001 SwFmt*               pFmt;
//CHINA001
//CHINA001 CollName*            pNms;
//CHINA001
//CHINA001 BOOL                 bNewTemplate;
//CHINA001
//CHINA001
//CHINA001 SwCondCollPage(Window *pParent, const SfxItemSet &rSet);
//CHINA001 virtual ~SwCondCollPage();
//CHINA001
//CHINA001 virtual int     DeactivatePage(SfxItemSet *pSet);
//CHINA001
//CHINA001 DECL_LINK( OnOffHdl, CheckBox* );
//CHINA001 DECL_LINK( AssignRemoveHdl, PushButton*);
//CHINA001 DECL_LINK( SelectHdl, ListBox* );
//CHINA001
//CHINA001 public:
//CHINA001
//CHINA001 static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
//CHINA001 static USHORT* GetRanges();
//CHINA001
//CHINA001 virtual BOOL FillItemSet(      SfxItemSet &rSet);
//CHINA001 virtual void Reset      (const SfxItemSet &rSet);
//CHINA001
//CHINA001 void SetCollection( SwFmt* pFormat, BOOL bNew );
//CHINA001 };



#endif

