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
#ifndef INCLUDED_CUI_SOURCE_INC_MULTIPAT_HXX
#define INCLUDED_CUI_SOURCE_INC_MULTIPAT_HXX

#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

#include "radiobtnbox.hxx"

// define ----------------------------------------------------------------

// different delimiter for Unix (:) and Windows (;)

#ifdef UNX
#define CLASSPATH_DELIMITER ':'
#else
#define CLASSPATH_DELIMITER ';'
#endif

class SvxMultiPathDialog : public ModalDialog
{
private:
    VclPtr<svx::SvxRadioButtonListBox> m_pRadioLB;
    VclPtr<PushButton>                 m_pAddBtn;
    VclPtr<PushButton>                 m_pDelBtn;

    DECL_LINK_TYPED(AddHdl_Impl, Button*, void);
    DECL_LINK_TYPED(DelHdl_Impl, Button*, void);
    DECL_LINK_TYPED(SelectHdl_Impl, SvTreeListBox*, void);
    DECL_LINK_TYPED(CheckHdl_Impl, SvTreeListBox*, void);

public:
    SvxMultiPathDialog(vcl::Window* pParent);
    virtual ~SvxMultiPathDialog();
    virtual void    dispose() override;

    OUString        GetPath() const;
    void            SetPath( const OUString& rPath );
};

class SvxPathSelectDialog : public ModalDialog
{
private:
    VclPtr<ListBox>                    m_pPathLB;
    VclPtr<PushButton>                 m_pAddBtn;
    VclPtr<PushButton>                 m_pDelBtn;

    DECL_LINK_TYPED(AddHdl_Impl, Button*, void);
    DECL_LINK_TYPED(DelHdl_Impl, Button*, void);
    DECL_LINK_TYPED(SelectHdl_Impl, ListBox&, void);

public:
    SvxPathSelectDialog(vcl::Window* pParent);
    virtual ~SvxPathSelectDialog();
    virtual void    dispose() override;

    OUString        GetPath() const;
    void            SetPath( const OUString& rPath );
};

#endif // INCLUDED_CUI_SOURCE_INC_MULTIPAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
