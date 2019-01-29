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

#ifndef INCLUDED_SVTOOLS_GENERICTOOLBOXCONTROLLER_HXX
#define INCLUDED_SVTOOLS_GENERICTOOLBOXCONTROLLER_HXX

#include <svtools/svtdllapi.h>
#include <svtools/toolboxcontroller.hxx>
#include <vcl/toolbox.hxx>

namespace svt
{

class SVT_DLLPUBLIC GenericToolboxController : public svt::ToolboxController
{
    public:
        GenericToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                  const css::uno::Reference< css::frame::XFrame >& rFrame,
                                  ToolBox* pToolBox,
                                  sal_uInt16   nID,
                                  const OUString& aCommand );
        virtual ~GenericToolboxController() override;

        // XComponent
        virtual void SAL_CALL dispose() override;

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

         DECL_STATIC_LINK( GenericToolboxController, ExecuteHdl_Impl, void*, void );

    private:
        VclPtr<ToolBox>    m_pToolbox;
        sal_uInt16         m_nID;
};

}

#endif // INCLUDED_SVTOOLS_GENERICTOOLBOXCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
