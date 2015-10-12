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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_WNAMEMATCH_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_WNAMEMATCH_HXX

#include "WTabPage.hxx"
#include "marktree.hxx"
#include "DExport.hxx"
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include "WCopyTable.hxx"

namespace dbaui
{
    // columns are at root only no children
    class OColumnTreeBox : public OMarkableTreeListBox
    {
        bool m_bReadOnly;
    protected:
        virtual void InitEntry(SvTreeListEntry* pEntry, const OUString& rStr, const Image& rImg1, const Image& rImg2, SvLBoxButtonKind eButtonKind) override;

    public:
        OColumnTreeBox( vcl::Window* pParent, WinBits nBits = WB_BORDER );

        void FillListBox( const ODatabaseExport::TColumnVector& _rList);
        void SetReadOnly(bool _bRo=true) { m_bReadOnly = _bRo; }
        virtual bool Select( SvTreeListEntry* pEntry, bool bSelect=true ) override;

    private:
        using OMarkableTreeListBox::Select;
    };

    // Wizard Page: OWizNameMatching
    // Name matching for data appending
    class OWizNameMatching : public OWizardPage
    {
        VclPtr<FixedText>           m_pTABLE_LEFT;
        VclPtr<FixedText>           m_pTABLE_RIGHT;
        VclPtr<OColumnTreeBox>      m_pCTRL_LEFT;    // left side
        VclPtr<OColumnTreeBox>      m_pCTRL_RIGHT;   // right side
        VclPtr<PushButton>          m_pColumn_up;
        VclPtr<PushButton>          m_pColumn_down;
        VclPtr<PushButton>          m_pColumn_up_right;
        VclPtr<PushButton>          m_pColumn_down_right;
        VclPtr<PushButton>          m_pAll;
        VclPtr<PushButton>          m_pNone;
        OUString            m_sSourceText;
        OUString            m_sDestText;
        Image               m_aImgUp;
        Image               m_aImgDown;

        DECL_LINK_TYPED( ButtonClickHdl, Button *, void );
        DECL_LINK_TYPED( RightButtonClickHdl, Button *, void );
        DECL_LINK_TYPED( AllNoneClickHdl, Button *, void );
        DECL_LINK_TYPED( TableListClickHdl, SvTreeListBox*, void );
        DECL_LINK_TYPED( TableListRightSelectHdl, SvTreeListBox*, void );

    public:
        virtual void            Reset ( ) override;
        virtual void            ActivatePage() override;
        virtual bool            LeavePage() override;
        virtual OUString        GetTitle() const override ;

        OWizNameMatching(vcl::Window* pParent);
        virtual ~OWizNameMatching();
        virtual void dispose() override;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_WNAMEMATCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
