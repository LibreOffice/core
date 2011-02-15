/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <svtools/svtreebx.hxx>
#include "formula/IFunctionDescription.hxx"
#include "ModuleHelper.hxx"

//============================================================================

//============================================================================
namespace formula
{

class IFormulaToken;
class   StructListBox : public SvTreeListBox
{
private:

    sal_Bool            bActiveFlag;

protected:
                    virtual void MouseButtonDown( const MouseEvent& rMEvt );

public:

                    StructListBox(Window* pParent, const ResId& rResId );

    /** Inserts an entry with static image (no difference between collapsed/expanded). */
    SvLBoxEntry*    InsertStaticEntry(
                        const XubString& rText,
                        const Image& rEntryImg,
                        const Image& rEntryImgHC,
                        SvLBoxEntry* pParent = NULL,
                        sal_uLong nPos = LIST_APPEND,
                        IFormulaToken* pToken = NULL );

    void            SetActiveFlag(sal_Bool bFlag=sal_True);
    sal_Bool            GetActiveFlag();
    void            GetFocus();
    void            LoseFocus();
};

//============================================================================

class StructPage : public TabPage
                    , public IStructHelper
{
private:
    OModuleClient   m_aModuleClient;
    Link            aSelLink;

    FixedText       aFtStruct;
    StructListBox   aTlbStruct;
    Image           maImgEnd;
    Image           maImgError;
    Image           maImgEndHC;
    Image           maImgErrorHC;

    IFormulaToken*  pSelectedToken;

    DECL_LINK( SelectHdl, SvTreeListBox* );

    using Window::GetParent;

protected:

    IFormulaToken*      GetFunctionEntry(SvLBoxEntry* pEntry);

public:

                    StructPage( Window* pParent);

    void            ClearStruct();
    virtual SvLBoxEntry*    InsertEntry(const XubString& rText, SvLBoxEntry* pParent,
                                sal_uInt16 nFlag,sal_uLong nPos=0,IFormulaToken* pScToken=NULL);

    virtual String          GetEntryText(SvLBoxEntry* pEntry) const;
    virtual SvLBoxEntry*    GetParent(SvLBoxEntry* pEntry) const;

    String          GetSelectedEntryText();
    IFormulaToken*  GetSelectedToken();

    void            SetSelectionHdl( const Link& rLink ) { aSelLink = rLink; }
    const Link&     GetSelectionHdl() const { return aSelLink; }
};

} // formula

#endif

