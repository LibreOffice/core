/*************************************************************************
 *
 *  $RCSfile: structpg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    Bitmap          aExpBmp;
    Bitmap          aCollBmp;
    Bitmap          aCloseBmp;
    Bitmap          aOpenBmp;
    Bitmap          aEndBmp;
    Bitmap          aErrorBmp;

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

    SvLBoxEntry*    InsertEntryWithError(USHORT nError,SvLBoxEntry* pParent,
                                        ULONG nPos=0);

    String          GetEntryText(SvLBoxEntry* pEntry);
    String          GetSelectedEntryText();
    ScToken*        GetSelectedToken();

    void            SetSelectionHdl( const Link& rLink ) { aSelLink = rLink; }
    const Link&     GetSelectionHdl() const { return aSelLink; }
};



#endif

