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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_UNOADMIN_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_UNOADMIN_HXX

#include <svtools/genericunodialog.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>
#include "dsntypes.hxx"
#include "moduledbu.hxx"

class SfxItemSet;
class SfxItemPool;
class SfxPoolItem;

namespace dbaui
{

// ODatabaseAdministrationDialog
typedef ::svt::OGenericUnoDialog ODatabaseAdministrationDialogBase;
class ODatabaseAdministrationDialog
        :public ODatabaseAdministrationDialogBase
{
    OModuleClient           m_aModuleClient;
protected:
    SfxItemSet*             m_pDatasourceItems;     // item set for the dialog
    SfxItemPool*            m_pItemPool;            // item pool for the item set for the dialog
    SfxPoolItem**           m_pItemPoolDefaults;    // pool defaults
    ::dbaccess::ODsnTypeCollection*
                            m_pCollection;          // datasource type collection

    css::uno::Any           m_aInitialSelection;
    css::uno::Reference< css::sdbc::XConnection > m_xActiveConnection;

protected:
    ODatabaseAdministrationDialog(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);
    virtual ~ODatabaseAdministrationDialog();
protected:
// OGenericUnoDialog overridables
    virtual void destroyDialog() override;
    virtual void implInitialize(const css::uno::Any& _rValue) override;
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_UNOADMIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
