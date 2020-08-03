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
#pragma once

#include <memory>
#include <sfx2/tabdlg.hxx>

#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <svtools/dialogclosedlistener.hxx>

// forward ---------------------------------------------------------------
struct OptPath_Impl;
class SvxPathTabPage;

// class SvxPathTabPage --------------------------------------------------
class SvxPathTabPage : public SfxTabPage
{
private:
    std::unique_ptr<OptPath_Impl>               pImpl;

    rtl::Reference< ::svt::DialogClosedListener > xDialogListener;
    css::uno::Reference< css::ui::dialogs::XFolderPicker2 > xFolderPicker;

    std::unique_ptr<weld::Button> m_xStandardBtn;
    std::unique_ptr<weld::Button> m_xPathBtn;
    std::unique_ptr<weld::TreeView> m_xPathBox;

    void        ChangeCurrentEntry( const OUString& _rFolder );

    DECL_LINK(PathHdl_Impl, weld::Button&, void);
    DECL_LINK(DoubleClickPathHdl_Impl, weld::TreeView&, bool);
    DECL_LINK(StandardHdl_Impl, weld::Button&, void);

    DECL_LINK(PathSelect_Impl, weld::TreeView&, void);

    DECL_LINK(DialogClosedHdl, css::ui::dialogs::DialogClosedEvent*, void);

    void        GetPathList( sal_uInt16 _nPathHandle, OUString& _rInternalPath,
                             OUString& _rUserPath, OUString& _rWritablePath, bool& _rReadOnly );
    void        SetPathList( sal_uInt16 _nPathHandle,
                             const OUString& _rUserPath, const OUString& _rWritablePath );

public:
    SvxPathTabPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet );
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet );
    virtual ~SvxPathTabPage() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
