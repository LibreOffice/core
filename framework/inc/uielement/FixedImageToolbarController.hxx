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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_FIXEDIMAGETOOLBARCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_FIXEDIMAGETOOLBARCONTROLLER_HXX

#include <com/sun/star/frame/ControlCommand.hpp>

#include <uielement/complextoolbarcontroller.hxx>
#include <vcl/vclptr.hxx>

class ToolBox;

namespace framework
{
class FixedImageControl;

class FixedImageToolbarController final : public ComplexToolbarController
{
public:
    FixedImageToolbarController(const css::uno::Reference<css::uno::XComponentContext>& rxContext,
                                const css::uno::Reference<css::frame::XFrame>& rFrame,
                                ToolBox* pToolBar, ToolBoxItemId nID, const OUString& aCommand);

    // XComponent
    virtual void SAL_CALL dispose() override;

    void CheckAndUpdateImages();

private:
    virtual void executeControlCommand(const css::frame::ControlCommand& rControlCommand) override;

    DECL_LINK(MiscOptionsChanged, LinkParamNone*, void);

    VclPtr<FixedImageControl> m_pFixedImageControl;
    sal_Int16 m_eSymbolSize;
};
} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_FIXEDIMAGETOOLBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
