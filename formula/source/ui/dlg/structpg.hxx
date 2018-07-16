/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FORMULA_SOURCE_UI_DLG_STRUCTPG_HXX
#define INCLUDED_FORMULA_SOURCE_UI_DLG_STRUCTPG_HXX

#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <svtools/treelistbox.hxx>
#include <formula/IFunctionDescription.hxx>

namespace formula
{

class FormulaToken;
class StructListBox : public SvTreeListBox
{
private:

    bool            bActiveFlag;

protected:
                    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

public:

                    StructListBox(vcl::Window* pParent, WinBits nBits );

    /** Inserts an entry with static image (no difference between collapsed/expanded). */
    SvTreeListEntry*    InsertStaticEntry(
                        const OUString& rText,
                        const Image& rEntryImg,
                        SvTreeListEntry* pParent,
                        sal_uLong nPos,
                        const FormulaToken* pToken );

    void            SetActiveFlag(bool bFlag);
    bool            GetActiveFlag() { return bActiveFlag;}
    void            GetFocus() override;
    void            LoseFocus() override;
};


class StructPage final : public TabPage
{
private:
    Link<StructPage&,void>  aSelLink;

    VclPtr<StructListBox>   m_pTlbStruct;
    Image           maImgEnd;
    Image           maImgError;

    const FormulaToken* pSelectedToken;

    DECL_LINK( SelectHdl, SvTreeListBox*, void );

    using Window::GetParent;

    const FormulaToken* GetFunctionEntry(SvTreeListEntry* pEntry);

public:

    explicit StructPage(vcl::Window* pParent);
    virtual         ~StructPage() override;
    virtual void    dispose() override;

    void            ClearStruct();
    SvTreeListEntry* InsertEntry(const OUString& rText, SvTreeListEntry* pParent,
                                sal_uInt16 nFlag, sal_uLong nPos, const FormulaToken* pScToken);

    OUString        GetEntryText(SvTreeListEntry* pEntry) const;

    void            SetSelectionHdl( const Link<StructPage&,void>& rLink ) { aSelLink = rLink; }

    StructListBox*  GetTlbStruct() const { return m_pTlbStruct; }
};

} // formula

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
