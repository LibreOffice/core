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

#ifndef INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_PANELTABBARPEER_HXX
#define INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_PANELTABBARPEER_HXX

#include "svtaccessiblefactory.hxx"

#include <toolkit/awt/vclxwindow.hxx>


namespace svt
{


    class PanelTabBar;

    //= PanelTabBarPeer

    class PanelTabBarPeer : public VCLXWindow
    {
    public:
        explicit PanelTabBarPeer( PanelTabBar& i_rTabBar );

    protected:
        virtual ~PanelTabBarPeer();

        // VCLXWindow overridables
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > CreateAccessibleContext() override;

        // XComponent
        void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;

    private:
        AccessibleFactoryAccess m_aAccessibleFactory;
        VclPtr<PanelTabBar>     m_pTabBar;
    };


} // namespace svt


#endif // INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_PANELTABBARPEER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
