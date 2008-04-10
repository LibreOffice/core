/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: structpg.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_STRUCTPG_HXX
#define SC_STRUCTPG_HXX

#include "funcutl.hxx"
#include "global.hxx"       // ScAddress
#include <svtools/stdctrl.hxx>
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>

#endif
#include <vcl/tabctrl.hxx>
#include "parawin.hxx"
#include <svtools/svtreebx.hxx>
#include "compiler.hxx"
#include "cell.hxx"


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

