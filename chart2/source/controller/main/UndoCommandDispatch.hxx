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

#include "CommandDispatch.hxx"

namespace com::sun::star::document { class XUndoManager; }
namespace com::sun::star::frame { class XModel; }

namespace chart
{

/** This is a CommandDispatch implementation for Undo and Redo.
 */
class UndoCommandDispatch : public CommandDispatch
{
public:
    explicit UndoCommandDispatch(
        const css::uno::Reference< css::uno::XComponentContext > & xContext,
        const css::uno::Reference< css::frame::XModel > & xModel );
    virtual ~UndoCommandDispatch() override;

    // late initialisation, especially for adding as listener
    virtual void initialize() override;

protected:
    // ____ XDispatch ____
    virtual void SAL_CALL dispatch(
        const css::util::URL& URL,
        const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) override;

    // ____ WeakComponentImplHelperBase ____
    /// is called when this is disposed
    virtual void SAL_CALL disposing() override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    virtual void fireStatusEvent(
        const OUString & rURL,
        const css::uno::Reference< css::frame::XStatusListener > & xSingleListener ) override;

private:
    css::uno::Reference< css::frame::XModel > m_xModel;
    css::uno::Reference< css::document::XUndoManager > m_xUndoManager;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
