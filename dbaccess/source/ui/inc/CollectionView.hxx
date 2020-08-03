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
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace dbaui
{
    /* this class allows to browse through the collection of forms and reports
    */
    class OCollectionView : public weld::GenericDialogController
    {
        css::uno::Reference< css::ucb::XContent>                  m_xContent;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        css::uno::Reference< css::ucb::XCommandEnvironment >      m_xCmdEnv;
        bool                   m_bCreateForm;

        std::unique_ptr<weld::Label> m_xFTCurrentPath;
        std::unique_ptr<weld::Button> m_xNewFolder;
        std::unique_ptr<weld::Button> m_xUp;
        std::unique_ptr<weld::TreeView> m_xView;
        std::unique_ptr<weld::Entry> m_xName;
        std::unique_ptr<weld::Button> m_xPB_OK;

        DECL_LINK(Up_Click, weld::Button&, void);
        DECL_LINK(NewFolder_Click, weld::Button&, void);
        DECL_LINK(Save_Click, weld::Button&, void);
        DECL_LINK(Dbl_Click_FileView, weld::TreeView&, bool);

        /// sets the fixedtext to the right content
        void initCurrentPath();

        void Initialize();
    public:
        OCollectionView(weld::Window * pParent,
                        const css::uno::Reference< css::ucb::XContent>& _xContent,
                        const OUString& _sDefaultName,
                        const css::uno::Reference< css::uno::XComponentContext >& _rxContext);
        virtual ~OCollectionView() override;
        const css::uno::Reference< css::ucb::XContent>& getSelectedFolder() const { return m_xContent;}
        OUString getName() const;
    };
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
