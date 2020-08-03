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

#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include "toolbar.hxx"
#include "formcontrolcontainer.hxx"
#include "bibshortcuthandler.hxx"

class BibDataManager;


namespace bib
{


    class BibGridwin;
    class BibBeamer final
            :public BibSplitWindow
            ,public FormControlContainer
    {
            css::uno::Reference< css::frame::XController >            m_xController;

            BibDataManager*         pDatMan;
            VclPtr<BibToolBar>      pToolBar;
            VclPtr<BibGridwin>      pGridWin;

            DECL_LINK( RecalcLayout_Impl, void*, void );

            void                    createToolBar();
            void                    createGridWin();

            // FormControlContainer ----------
            virtual css::uno::Reference< css::awt::XControlContainer >
                    getControlContainer() override;
        public:
            css::uno::Reference< css::frame::XDispatchProviderInterception >
                    getDispatchProviderInterception() const;

            BibBeamer(vcl::Window* pParent,BibDataManager* pDatMan );
            virtual ~BibBeamer() override;
            virtual void dispose() override;

            void    SetXController(const css::uno::Reference< css::frame::XController > &);

            virtual void            GetFocus() override;
    };


}   // namespace bib


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
