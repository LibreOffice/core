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

#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/weld.hxx>

namespace pcr
{
    //= TabOrderDialog
    class TabOrderDialog : public weld::GenericDialogController
    {
        css::uno::Reference< css::awt::XTabControllerModel >
                                    m_xTempModel;
        css::uno::Reference< css::awt::XTabControllerModel >
                                    m_xModel;
        css::uno::Reference< css::awt::XControlContainer >
                                    m_xControlContainer;
        css::uno::Reference< css::uno::XComponentContext >
                                    m_xORB;

        std::unique_ptr<weld::TreeView> m_xLB_Controls;
        std::unique_ptr<weld::Button> m_xPB_OK;
        std::unique_ptr<weld::Button> m_xPB_MoveUp;
        std::unique_ptr<weld::Button> m_xPB_MoveDown;
        std::unique_ptr<weld::Button> m_xPB_AutoOrder;

        DECL_LINK( ModelHasMoved, weld::TreeView&, void );
        DECL_LINK( MoveUpClickHdl, weld::Button&, void );
        DECL_LINK( MoveDownClickHdl, weld::Button&, void );
        DECL_LINK( AutoOrderClickHdl, weld::Button&, void );
        DECL_LINK( OKClickHdl, weld::Button&, void );

        void FillList();
        void MoveSelection(int nRelPos);

    public:
        TabOrderDialog(
            weld::Window* pParent,
            const css::uno::Reference< css::awt::XTabControllerModel >& _rxTabModel,
            const css::uno::Reference< css::awt::XControlContainer >& _rxControlCont,
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );

        virtual ~TabOrderDialog() override;

        void SetModified();
    };


}  // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
