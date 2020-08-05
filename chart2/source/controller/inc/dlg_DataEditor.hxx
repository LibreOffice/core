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

#include <vcl/weld.hxx>

namespace com::sun::star::uno { class XComponentContext; }
namespace comphelper { template <class Tp, class Arg> class mem_fun1_t; }

namespace com::sun::star {
    namespace chart2 {
        class XChartDocument;
    }
}

namespace chart
{

class DataBrowser;

class DataEditor : public weld::GenericDialogController
{
public:
    DataEditor(weld::Window* pParent,
               const css::uno::Reference<css::chart2::XChartDocument> & xChartDoc,
               const css::uno::Reference<css::uno::XComponentContext> & xContext);
    virtual ~DataEditor() override;

    DECL_LINK(CloseHdl, weld::Button&, void);

    void SetReadOnly( bool bReadOnly );

private:
    bool                           m_bReadOnly;

    css::uno::Reference<css::chart2::XChartDocument> m_xChartDoc;
    css::uno::Reference<css::uno::XComponentContext> m_xContext;

    std::unique_ptr<weld::Toolbar> m_xTbxData;
    std::unique_ptr<weld::Button> m_xCloseBtn;
    std::unique_ptr<weld::Container> m_xTable;
    std::unique_ptr<weld::Container> m_xColumns;
    std::unique_ptr<weld::Container> m_xColors;
    css::uno::Reference<css::awt::XWindow> m_xTableCtrlParent;
    VclPtr<DataBrowser> m_xBrwData;

    /// handles actions of the toolbox
    DECL_LINK( ToolboxHdl, const OString&, void );
    /// is called, if the cursor of the table has moved
    DECL_LINK( BrowserCursorMovedHdl, DataBrowser*, void);
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
