/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: structpg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:52:58 $
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

#ifndef SC_STRUCTPG_HXX
#define SC_STRUCTPG_HXX

#ifndef SC_FUNCUTL_HXX
#include "funcutl.hxx"
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"       // ScAddress
#endif

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

#ifndef _SV_TABPAGE_HXX //autogen
#include <vcl/tabpage.hxx>
#endif

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>

#endif

#ifndef _SV_TABCTRL_HXX //autogen
#include <vcl/tabctrl.hxx>
#endif

#ifndef SC_PARAWIN_HXX
#include "parawin.hxx"
#endif

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#ifndef SC_COMPILER_HXX
#include "compiler.hxx"
#endif

#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif


class ScViewData;
class ScFuncName_Impl;
class ScDocument;
class ScFuncDesc;

//============================================================================

#define STRUCT_END    1
#define STRUCT_FOLDER 2
#define STRUCT_ERROR  3

#define STRUCT_ERR_C1 1
#define STRUCT_ERR_C2 2

//============================================================================


class   ScStructListBox : public SvTreeListBox
{
private:

    BOOL            bActiveFlag;

protected:
                    virtual void MouseButtonDown( const MouseEvent& rMEvt );

public:

                    ScStructListBox(Window* pParent, const ResId& rResId );

    /** Inserts an entry with static image (no difference between collapsed/expanded). */
    SvLBoxEntry*    InsertStaticEntry(
                        const XubString& rText,
                        const Image& rEntryImg,
                        const Image& rEntryImgHC,
                        SvLBoxEntry* pParent = NULL,
                        ULONG nPos = LIST_APPEND,
                        ScToken* pToken = NULL );

    void            SetActiveFlag(BOOL bFlag=TRUE);
    BOOL            GetActiveFlag();
    void            GetFocus();
    void            LoseFocus();
};

//============================================================================

class ScStructPage : public TabPage
{
private:

    Link            aSelLink;

    FixedText       aFtStruct;
    ScStructListBox aTlbStruct;
    Image           maImgEnd;
    Image           maImgError;
    Image           maImgEndHC;
    Image           maImgErrorHC;

    ScToken*        pSelectedToken;

    DECL_LINK( SelectHdl, SvTreeListBox* );

protected:

    ScToken*        GetFunctionEntry(SvLBoxEntry* pEntry);

public:

                    ScStructPage( Window* pParent);

    SvTreeListBox*  GetStructPtr() {return &aTlbStruct;}

    void            ClearStruct();
    SvLBoxEntry*    InsertEntry(const XubString& rText, SvLBoxEntry* pParent,
                                USHORT nFlag,ULONG nPos=0,ScToken* pScToken=NULL);

    String          GetEntryText(SvLBoxEntry* pEntry);
    String          GetSelectedEntryText();
    ScToken*        GetSelectedToken();

    void            SetSelectionHdl( const Link& rLink ) { aSelLink = rLink; }
    const Link&     GetSelectionHdl() const { return aSelLink; }
};



#endif

