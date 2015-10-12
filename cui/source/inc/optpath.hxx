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
#ifndef INCLUDED_CUI_SOURCE_INC_OPTPATH_HXX
#define INCLUDED_CUI_SOURCE_INC_OPTPATH_HXX

#include <sfx2/tabdlg.hxx>
#include <svtools/simptabl.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>

#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <svtools/dialogclosedlistener.hxx>

// forward ---------------------------------------------------------------

namespace svx
{
    class OptHeaderTabListBox;
}
struct OptPath_Impl;
class SvxPathTabPage;

// class SvxPathTabPage --------------------------------------------------

class SvxPathTabPage : public SfxTabPage
{
private:
    VclPtr<SvSimpleTableContainer> m_pPathCtrl;
    VclPtr<PushButton>         m_pStandardBtn;
    VclPtr<PushButton>         m_pPathBtn;

    VclPtr<svx::OptHeaderTabListBox> pPathBox;
    OptPath_Impl*               pImpl;

    css::uno::Reference< ::svt::DialogClosedListener > xDialogListener;
    css::uno::Reference< css::ui::dialogs::XFolderPicker2 > xFolderPicker;

    void        ChangeCurrentEntry( const OUString& _rFolder );

    DECL_LINK_TYPED( PathHdl_Impl, Button*, void);
    DECL_LINK_TYPED( DoubleClickPathHdl_Impl, SvTreeListBox*, bool);
    DECL_LINK_TYPED( StandardHdl_Impl, Button*, void);

    DECL_LINK_TYPED( PathSelect_Impl, SvTreeListBox*, void);
    DECL_LINK_TYPED( HeaderSelect_Impl, HeaderBar *, void );
    DECL_LINK_TYPED( HeaderEndDrag_Impl, HeaderBar *, void );

    DECL_LINK_TYPED( DialogClosedHdl, css::ui::dialogs::DialogClosedEvent*, void );

    void        GetPathList( sal_uInt16 _nPathHandle, OUString& _rInternalPath,
                             OUString& _rUserPath, OUString& _rWritablePath, bool& _rReadOnly );
    void        SetPathList( sal_uInt16 _nPathHandle,
                             const OUString& _rUserPath, const OUString& _rWritablePath );

public:
    SvxPathTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxPathTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        FillUserData() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
