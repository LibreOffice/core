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
#ifndef DBAUI_WIZ_NAMEMATCHING_HXX
#define DBAUI_WIZ_NAMEMATCHING_HXX

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
        sal_Bool m_bReadOnly;
    protected:
        virtual void InitEntry(SvTreeListEntry* pEntry, const OUString& rStr, const Image& rImg1, const Image& rImg2, SvLBoxButtonKind eButtonKind);

    public:
        OColumnTreeBox( Window* pParent, const ResId& rResId );

        void FillListBox( const ODatabaseExport::TColumnVector& _rList);
        void SetReadOnly(sal_Bool _bRo=sal_True) { m_bReadOnly = _bRo; }
        virtual sal_Bool Select( SvTreeListEntry* pEntry, sal_Bool bSelect=sal_True );

    private:
        using OMarkableTreeListBox::Select;
    };

    // Wizard Page: OWizNameMatching
    // Name matching for data appending
    class OWizNameMatching : public OWizardPage
    {
        FixedText           m_FT_TABLE_LEFT;
        FixedText           m_FT_TABLE_RIGHT;
        OColumnTreeBox      m_CTRL_LEFT;    // left side
        OColumnTreeBox      m_CTRL_RIGHT;   // right side
        ImageButton         m_ibColumn_up;
        ImageButton         m_ibColumn_down;
        ImageButton         m_ibColumn_up_right;
        ImageButton         m_ibColumn_down_right;
        PushButton          m_pbAll;
        PushButton          m_pbNone;
        OUString            m_sSourceText;
        OUString            m_sDestText;

        DECL_LINK( ButtonClickHdl, Button * );
        DECL_LINK( RightButtonClickHdl, Button * );
        DECL_LINK( AllNoneClickHdl, Button * );
        DECL_LINK( TableListClickHdl, void* );
        DECL_LINK( TableListRightSelectHdl, void* );

    public:
        virtual void            Reset ( );
        virtual void            ActivatePage();
        virtual sal_Bool        LeavePage();
        virtual String          GetTitle() const ;

        OWizNameMatching(Window* pParent);
        virtual ~OWizNameMatching();

    };
}
#endif // DBAUI_WIZ_NAMEMATCHING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
