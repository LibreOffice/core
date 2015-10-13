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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_EDITTOOLBARCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_EDITTOOLBARCONTROLLER_HXX

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>

#include <uielement/complextoolbarcontroller.hxx>
#include <vcl/edit.hxx>

class ToolBox;

namespace framework
{

class EditControl;

class EditToolbarController : public ComplexToolbarController

{
    public:
        EditToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                               const css::uno::Reference< css::frame::XFrame >& rFrame,
                               ToolBox* pToolBar,
                               sal_uInt16 nID,
                               sal_Int32 nWidth,
                               const OUString& aCommand );
        virtual ~EditToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) override;

        // called from EditControl
        void Modify();
        void KeyInput( const KeyEvent& rKEvt );
        void GetFocus();
        void LoseFocus();
        bool PreNotify( NotifyEvent& rNEvt );

    protected:
        virtual void executeControlCommand( const css::frame::ControlCommand& rControlCommand ) override;
        virtual css::uno::Sequence< css::beans::PropertyValue> getExecuteArgs(sal_Int16 KeyModifier) const override;

    private:
        VclPtr<EditControl>    m_pEditControl;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_EDITTOOLBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
