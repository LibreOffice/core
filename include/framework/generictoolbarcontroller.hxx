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

#include <framework/fwkdllapi.h>
#include <svtools/toolboxcontroller.hxx>
#include <vcl/toolbox.hxx>

namespace framework
{

class FWK_DLLPUBLIC GenericToolbarController final : public svt::ToolboxController
{
    public:
        GenericToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                  const css::uno::Reference< css::frame::XFrame >& rFrame,
                                  ToolBox* pToolBar,
                                  ToolBoxItemId nID,
                                  const OUString& aCommand );
        GenericToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                  const css::uno::Reference< css::frame::XFrame >& rFrame,
                                  weld::Toolbar& rToolBar,
                                  const OUString& aCommand );
        virtual ~GenericToolbarController() override;

        // XComponent
        virtual void SAL_CALL dispose() override;

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

        DECL_DLLPRIVATE_STATIC_LINK( GenericToolbarController, ExecuteHdl_Impl, void*, void );

        struct ExecuteInfo
        {
            css::uno::Reference< css::frame::XDispatch >     xDispatch;
            css::util::URL                                   aTargetURL;
            css::uno::Sequence< css::beans::PropertyValue >  aArgs;
        };

    private:
        VclPtr<ToolBox>     m_xToolbar;
        ToolBoxItemId       m_nID;
        bool                m_bEnumCommand : 1,
                            m_bMirrored : 1,
                            m_bMadeInvisible : 1;
        OUString            m_aEnumCommand;
};

class FWK_DLLPUBLIC ImageOrientationController final : public svt::ToolboxController
{
public:
    ImageOrientationController(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                               const css::uno::Reference<css::frame::XFrame>& rFrame,
                               const css::uno::Reference<css::awt::XWindow>& rParentWindow,
                               const OUString& rModuleName);

    // XComponent
    void SAL_CALL dispose() override;

    // XStatusListener
    void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    DECL_LINK(WindowEventListener, VclWindowEvent&, void);
private:
    Degree10 m_nRotationAngle;
    bool m_bMirrored;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
