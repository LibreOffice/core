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
#ifndef INCLUDED_SVX_SOURCE_INC_TABWIN_HXX
#define INCLUDED_SVX_SOURCE_INC_TABWIN_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <svx/dbaexchange.hxx>
#include <com/sun/star/form/XForm.hpp>

#include <comphelper/propmultiplex.hxx>
#include <connectivity/dbtools.hxx>

class FmFormShell;
struct ColumnInfo;

class FmFieldWin : public SfxModelessDialogController
                 , public SfxControllerItem
                 , public ::comphelper::OPropertyChangeListener
{
    ::osl::Mutex        m_aMutex;
    std::unique_ptr<weld::TreeView> m_xListBox;
    std::vector<std::unique_ptr<ColumnInfo>> m_aListBoxData;
    ::dbtools::SharedConnection
                       m_aConnection;
    OUString    m_aDatabaseName,
                       m_aObjectName;
    sal_Int32          m_nObjectType;

    rtl::Reference<comphelper::OPropertyChangeMultiplexer>  m_xChangeListener;
    rtl::Reference<svx::OColumnTransferable> m_xHelper;

    void addToList(const css::uno::Reference<css::container::XNameAccess>& i_xColumns);

    DECL_LINK(RowActivatedHdl, weld::TreeView&, bool);
    DECL_LINK(DragBeginHdl, weld::TreeView&, bool);
public:
    FmFieldWin(SfxBindings *pBindings, SfxChildWindow *pMgr, weld::Window* pParent);

    virtual ~FmFieldWin() override;

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;

    void UpdateContent(FmFormShell const *);
    void UpdateContent(const css::uno::Reference< css::form::XForm > &);
    void FillInfo( SfxChildWinInfo& rInfo ) const override;

    const OUString& GetDatabaseName() const { return m_aDatabaseName; }
    const ::dbtools::SharedConnection& GetConnection() const { return m_aConnection; }
    const OUString& GetObjectName() const { return m_aObjectName; }
    sal_Int32 GetObjectType() const { return m_nObjectType; }

    bool    createSelectionControls( );

protected:
    // FmXChangeListener
    virtual void _propertyChanged(const css::beans::PropertyChangeEvent& evt) override;

protected:
    using SfxControllerItem::GetBindings;
};


class FmFieldWinMgr : public SfxChildWindow
{
public:
    FmFieldWinMgr(vcl::Window *pParent, sal_uInt16 nId,
        SfxBindings *pBindings, SfxChildWinInfo const *pInfo);
    SFX_DECL_CHILDWINDOW(FmFieldWinMgr);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
