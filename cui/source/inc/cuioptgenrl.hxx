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
#ifndef INCLUDED_CUI_SOURCE_INC_CUIOPTGENRL_HXX
#define INCLUDED_CUI_SOURCE_INC_CUIOPTGENRL_HXX

// include ---------------------------------------------------------------

#include <svx/optgenrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>

#include <vector>

// forward ---------------------------------------------------------------

class Edit;


// class SvxGeneralTabPage -----------------------------------------------

class SvxGeneralTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;
private:
    // the "Use data for document properties" checkbox
    VclPtr<CheckBox> m_pUseDataCB;
    VclPtr<VclContainer> m_pCryptoFrame;
    VclPtr<ListBox> m_pSigningKeyLB;
    VclPtr<ListBox> m_pEncryptionKeyLB;
    VclPtr<CheckBox> m_pEncryptToSelfCB;
    // rows
    struct Row;
    std::vector<std::shared_ptr<Row> > vRows;
    // fields
    struct Field;
    std::vector<std::shared_ptr<Field> > vFields;
    // "name" fields
    unsigned nNameRow;
    unsigned nShortNameField;

    DECL_LINK( ModifyHdl_Impl, Edit&, void );

    bool                GetData_Impl();
    void                SetData_Impl();

    void InitControls ();
    void InitCryptography();
    void SetLinks ();

protected:
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:
    SvxGeneralTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxGeneralTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif // INCLUDED_CUI_SOURCE_INC_CUIOPTGENRL_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
